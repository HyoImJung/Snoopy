#include "GameUI.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

// Is this wide char a "full-width" (2-column) glyph? (Hangul/CJK/fullwidth)
static bool isWideChar(wchar_t wc) {
    return (wc >= 0x1100 && wc <= 0x115F) || // Hangul Jamo
           (wc >= 0x2E80 && wc <= 0x303E) || // CJK radicals / punctuation
           (wc >= 0x3041 && wc <= 0x33FF) || // Kana / CJK symbols
           (wc >= 0x3400 && wc <= 0x4DBF) || // CJK Ext A
           (wc >= 0x4E00 && wc <= 0x9FFF) || // CJK Unified
           (wc >= 0xA000 && wc <= 0xA4CF) || // Yi
           (wc >= 0xAC00 && wc <= 0xD7A3) || // Hangul syllables
           (wc >= 0xF900 && wc <= 0xFAFF) || // CJK compat
           (wc >= 0xFF00 && wc <= 0xFF60) || // Fullwidth forms
           (wc >= 0xFFE0 && wc <= 0xFFE6);
}

// Display column width, independent of the string's byte encoding.
// ANSI escape sequences (ESC ... m) are stripped (count 0). The remaining
// text is converted to UTF-16 (UTF-8 first, CP949 fallback) and each glyph
// is counted as 2 columns if full-width (Hangul/CJK), otherwise 1.
static int displayWidth(const std::string& s) {
    // 1. Strip ANSI escape sequences
    std::string clean;
    clean.reserve(s.size());
    for (size_t i = 0; i < s.size(); ) {
        if ((unsigned char)s[i] == 0x1B) {
            while (i < s.size() && s[i] != 'm') ++i;
            if (i < s.size()) ++i;
        } else {
            clean += s[i++];
        }
    }
    if (clean.empty()) return 0;

    // 2. Convert to wide (UTF-8 strict, then CP949 fallback)
    std::wstring w;
    int wlen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
        clean.c_str(), (int)clean.size(), nullptr, 0);
    if (wlen > 0) {
        w.resize(wlen);
        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
            clean.c_str(), (int)clean.size(), &w[0], wlen);
    } else {
        wlen = MultiByteToWideChar(CP_ACP, 0,
            clean.c_str(), (int)clean.size(), nullptr, 0);
        if (wlen > 0) {
            w.resize(wlen);
            MultiByteToWideChar(CP_ACP, 0,
                clean.c_str(), (int)clean.size(), &w[0], wlen);
        }
    }

    // 3. Sum column widths
    int width = 0;
    for (wchar_t wc : w) width += isWideChar(wc) ? 2 : 1;
    return width;
}

// Pad a string to display-column width w (accounts for Hangul/ANSI codes).
static std::string pUI(const std::string& s, int w) {
    int dw = displayWidth(s);
    if (dw >= w) return s;
    return s + std::string(w - dw, ' ');
}

// UI.cpp 의 GameUI::renderInterfaceLines 함수 보강 개조


// ── Character ASCII art (Knight=0, Archer=1, Priest=2, Mage=3) ──
const char* GameUI::charArt[4][25] = {
  { // Knight
    "              ;",
    "                     =",
    "                 *   =",
    "              ; +*; :;",
    "             +    ;",
    "++=+++++    +             :",
    "   =;:: +; ;=          +  +",
    "     ;      ;           + +",
    "             :;  ;:  +    ;",
    "                ;++     : +",
    "          :    +***=:",
    "            +   =  +  :+",
    "               +",
    "                       +",
    "            * :    +   +",
    "               ;   +;",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
  },
  { // Archer
    "                  =",
    nullptr,
    "       ;  ;          =+",
    " ++=   ;;     ;       ;:",
    "   ==+      +          *",
    " ;       +=============;=+++",
    "      +",
    "       +::;:++         *",
    "        +++++++       +",
    "                    ;+",
    "                   ;",
    "      +  : +   +",
    "      +       :",
    "      = =     +",
    "      ; +   :::",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
  },
  { // Priest
    "           :+++++",
    nullptr,
    "          ;=    =;   +",
    "        +;         =",
    "      +==    =+    ==+;",
    "  =     ;+         ;     ;",
    " +    =+ ;        ;  ==   +",
    " =;:  =    ;    +    =   :;",
    "      +=++==+  ;====+=",
    "     ;+==+++;   ++*+:;+",
    "      ;=   ++     ++ ;",
    "      ;==+==    ======",
    "    =   ;+ +    =+: ;+ =",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
  },
  { // Mage
    "               ===",
    "              ;",
    ";;:  ;            +",
    "            +     ;=",
    nullptr,
    "           :        ;",
    "          ++        ;+",
    "        +  ;           :",
    "    :=",
    "    **             ; ;",
    "    =*  ;            ;",
    "    ;     =          +    ;",
    "                      :=:=:",
    "         +        :   =",
    "        ;         :    :",
    "        =              +",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
  },
};


// Returns the ANSI color code for each hero class (same palette as the map)
static std::string heroColor(CharacterClass cls) {
    switch (cls) {
    case CharacterClass::KNIGHT: return "\033[0;90m";
    case CharacterClass::ARCHER: return "\033[0;32m";
    case CharacterClass::PRIEST: return "\033[0;93m";
    case CharacterClass::MAGE:   return "\033[0;35m";
    default: return "";
    }
}

// Returns the ANSI color code for each enemy class (same palette as the map)
static std::string enemyColor(CharacterClass cls) {
    switch (cls) {
    case CharacterClass::ENEMY_GOBLIN:     return "\033[0;92m"; // bright green
    case CharacterClass::ENEMY_SLIME:      return "\033[0;96m"; // bright cyan
    case CharacterClass::ENEMY_ORC:        return "\033[0;33m"; // yellow/brown
    case CharacterClass::ENEMY_UNDEAD:     return "\033[0;36m"; // dark cyan
    case CharacterClass::ENEMY_DEMON:      return "\033[0;91m"; // bright red
    case CharacterClass::ELITE_LICH_KING:  return "\033[0;95m"; // bright magenta
    case CharacterClass::ELITE_DEMON_KING: return "\033[0;31m"; // dark red
    case CharacterClass::BOSS_DRAGON:      return "\033[0;93m"; // bright yellow
    default: return "";
    }
}

std::vector<std::string> GameUI::renderInterfaceLines(const GameState& state) const {
    std::vector<std::string> lns;
    const int TOTAL_W = 50;
    const int LEFT_W = 24;

    std::string sep(TOTAL_W, '=');
    std::string dash(TOTAL_W, '-');
    std::string colSep = " | ";

    int curIdx = state.getCurrentAllyIndex();
    const auto& allies = state.getAllies();
    const auto& enemies = state.getEnemies();

    lns.push_back(sep);
    lns.push_back(" [ 선택된 유닛 정보 ]");
    lns.push_back(dash);

    if (curIdx == -1) {
        lns.push_back("    유닛을 선택하세요 (숫자 1-4)");
        lns.push_back("");
    }
    else {
        const auto* sel = allies[curIdx];
        std::string status = sel->actedThisTurn ? "[행동완료]" : "[대기중]";
        lns.push_back(" 이름  : " + heroColor(sel->cls) + sel->name + "\033[0m" + " " + status);
        lns.push_back(" 체력  : " + std::to_string(sel->hp) + " / " + std::to_string(sel->maxHp));
        lns.push_back(" 공격력: " + std::to_string(sel->atk) + "  사거리: " + std::to_string(sel->atkRange));

        // -----------------------------------------------------------
        // ★ [동적 UI 업그레이드] 스킬 메뉴가 열렸을 때 스킬 목록 나열
        // -----------------------------------------------------------
        if (state.getIsSkillMenuOpen()) {
            lns.push_back(dash);
            lns.push_back(" [ 사용 가능한 스킬 ]");
            auto skillNames = sel->getSkillNames();
            for (size_t i = 0; i < skillNames.size(); ++i) {
                lns.push_back("  " + std::to_string(i + 1) + ". " + skillNames[i]);
            }
        }
    }

    lns.push_back(dash);
    lns.push_back(" [ 타워 & 적 상태 ]");
    lns.push_back(" 웨이브: " + std::to_string(state.getWave()));
    lns.push_back(dash);

    // 타워 HP 및 적 정보 출력용 람다
    std::string towerStr = "타워 HP: " + std::to_string(state.getTowerHp()) + "/" + std::to_string(state.getTowerMaxHp());
    std::string apStr = "플레이어 AP: " + std::to_string(state.getAp()) + "/" + std::to_string(state.getMaxAp());

    auto getEnemyStr = [&](int i) -> std::string {
        if (i >= 0 && i < (int)enemies.size()) {
            const auto* e = enemies[i];
            std::string col = enemyColor(e->cls);
            std::string icon = e->getIcon();
            std::string info = e->isAlive() ? "HP:" + std::to_string(e->hp) : "사망";
            return "(" + col + icon + "\033[0m" + ") " + col + e->name + "\033[0m" + " " + info;
        }
        return "";
    };

    lns.push_back(pUI(towerStr, LEFT_W) + colSep + getEnemyStr(0));
    lns.push_back(pUI(apStr, LEFT_W) + colSep + getEnemyStr(1));

    int maxRows = std::max((int)allies.size(), (int)enemies.size() - 2);
    maxRows = std::max(maxRows, 0);
    for (int i = 0; i < maxRows; ++i) {
        std::string leftContent = "";
        if (i < (int)allies.size()) {
            const auto* a = allies[i];
            std::string prefix = (curIdx == i) ? ">" : " ";
            std::string status = (!a->isAlive()) ? "사망" : (a->actedThisTurn ? "완료" : "대기");
            std::string icon = a->getIcon();
            std::string col = heroColor(a->cls);
            // Compute padding from display width, then apply color to icon + name
            std::string plain   = prefix + "(" + icon + ") " + a->name + ": " + status;
            std::string colored = prefix + "(" + col + icon + "\033[0m" + ") " + col + a->name + "\033[0m" + ": " + status;
            int pad = LEFT_W - displayWidth(plain);
            leftContent = colored + (pad > 0 ? std::string(pad, ' ') : "");
        }
        if (leftContent.empty()) leftContent = std::string(LEFT_W, ' ');
        lns.push_back(leftContent + colSep + getEnemyStr(i + 2));
    }

    // -----------------------------------------------------------
    // ★ [동적 UI 업그레이드] 최하단 스킬 설명란 생성 및 소멸 영역
    // -----------------------------------------------------------
    if (curIdx != -1 && state.getIsSkillMenuOpen()) {
        lns.push_back(sep);
        lns.push_back(" [ 스킬 설명 ]");
        lns.push_back(dash);

        const auto* sel = allies[curIdx];
        // 1번부터 3번까지의 스킬의 짤막한 핵심 정보를 리스트 형태로 출력합니다.
        for (int i = 0; i < 3; ++i) {
            lns.push_back(" " + std::to_string(i + 1) + ": " + sel->getSkillDescription(i));
        }
    }

    lns.push_back(sep);
    return lns;
}

std::vector<std::string> GameUI::renderCommandLines(const GameState& state) const {
    std::vector<std::string> lns;
    const int W = 66;
    std::string sep(W, '=');

    lns.push_back(sep);
    lns.push_back(" [선택] 1:기사 2:궁수 3:사제 4:법사 | [이동] w,a,s,d | [공격] f+방향");
    if (state.getIsSkillMenuOpen()) {
        lns.push_back(" [스킬] 1~3: 스킬 선택 | [취소] b - 스킬 취소");
    } else {
        lns.push_back(" [스킬] r - 스킬 사용 | [취소] b - 취소 | [턴 종료] z");
    }
    lns.push_back(" 기록: " + state.getLastMessage());
    lns.push_back(sep);

    return lns;
}