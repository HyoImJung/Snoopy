#include "GameUI.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>

static std::string pUI(const std::string& s, int w) {
    if ((int)s.size() >= w) return s;
    return s + std::string(w - s.size(), ' ');
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
    lns.push_back(" [ SELECTED UNIT INFO ]");
    lns.push_back(dash);

    if (curIdx == -1) {
        lns.push_back("    PLEASE SELECT A UNIT (Key 1-4)");
        lns.push_back("");
    }
    else {
        const auto* sel = allies[curIdx];
        std::string status = sel->actedThisTurn ? "[DONE]" : "[READY]";
        lns.push_back(" NAME  : " + sel->name + " " + status);
        lns.push_back(" HP    : " + std::to_string(sel->hp) + " / " + std::to_string(sel->maxHp));
        lns.push_back(" ATK   : " + std::to_string(sel->atk) + "  RANGE: " + std::to_string(sel->atkRange));

        // -----------------------------------------------------------
        // ★ [동적 UI 업그레이드] 스킬 메뉴가 열렸을 때 스킬 목록 나열
        // -----------------------------------------------------------
        if (state.getIsSkillMenuOpen()) {
            lns.push_back(dash);
            lns.push_back(" [ AVAILABLE SKILLS ]");
            auto skillNames = sel->getSkillNames();
            for (size_t i = 0; i < skillNames.size(); ++i) {
                lns.push_back("  " + std::to_string(i + 1) + ". " + skillNames[i]);
            }
        }
    }

    lns.push_back(dash);
    lns.push_back(" [ TOWER & ENEMIES STATUS ]");
    lns.push_back(" Wave: " + std::to_string(state.getWave()));
    lns.push_back(dash);

    // 타워 HP 및 적 정보 출력용 람다
    std::string towerStr = "Tower HP: " + std::to_string(state.getTowerHp()) + "/" + std::to_string(state.getTowerMaxHp());
    std::string apStr = "Player AP: " + std::to_string(state.getAp()) + "/" + std::to_string(state.getMaxAp());

    auto getEnemyStr = [&](int i) -> std::string {
        if (i >= 0 && i < (int)enemies.size()) {
            const auto* e = enemies[i];
            return "E" + std::to_string(i + 1) + " " + (e->isAlive() ? "HP:" + std::to_string(e->hp) : "Dead");
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
            std::string status = (!a->isAlive()) ? "Dead" : (a->actedThisTurn ? "Done" : "Ready");
            leftContent = prefix + a->name.substr(0, 1) + ": " + status;
        }
        lns.push_back(pUI(leftContent, LEFT_W) + colSep + getEnemyStr(i + 2));
    }

    // -----------------------------------------------------------
    // ★ [동적 UI 업그레이드] 최하단 스킬 설명란 생성 및 소멸 영역
    // -----------------------------------------------------------
    if (curIdx != -1 && state.getIsSkillMenuOpen()) {
        lns.push_back(sep);
        lns.push_back(" [ SKILL DESCRIPTIONS ]");
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
    lns.push_back(" [SELECT] 1:K 2:A 3:P 4:W | [MOVE] w,a,s,d | [ATK] f<dir>");
    lns.push_back(" [TURN END] z - End your turn manually");
    lns.push_back(" Log: " + state.getLastMessage());
    lns.push_back(sep);

    return lns;
}