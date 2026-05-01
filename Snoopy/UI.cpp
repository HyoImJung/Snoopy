#include "GameUI.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <algorithm>

static std::string padTo(const std::string& s, int w) {
    if ((int)s.size() >= w) return s;
    return s + std::string(w - s.size(), ' ');
}

void GameUI::printLine(std::string symbol) const {
    for (int i = 0; i < UI_WIDTH; ++i) std::cout << symbol;
    std::cout << "\n";
}

std::string GameUI::hpBar(int hp, int maxHp, int barLen) const {
    int filled = (maxHp > 0) ? (hp * barLen / maxHp) : 0;
    filled = std::max(0, std::min(filled, barLen));
    std::string bar = "[";
    for (int i = 0; i < barLen; ++i) bar += (i < filled) ? "#" : "-";
    bar += "]";
    return bar;
}

void GameUI::drawInterface(const GameState& state) const {
    for (auto& l : renderInterfaceLines(state))
        std::cout << l << "\n";
}

// 오른쪽 패널: ALLY INFO / ENEMY INFO + Paladin / 적 12마리 스탯
std::vector<std::string> GameUI::renderInterfaceLines(const GameState& state) const {
    // 왼쪽(Paladin) 20자 | 오른쪽(Orc 목록) 22자
    const int LCOL = 20;
    const int RCOL = 22;
    const int W    = LCOL + 3 + RCOL; // 45
    std::string sep(W,  '=');
    std::string dash(W, '-');

    const auto& allies  = state.getAllies();
    const auto& enemies = state.getEnemies();

    auto row = [&](const std::string& l, const std::string& r) -> std::string {
        return padTo(l, LCOL) + "| " + r;
    };

    std::vector<std::string> lines;

    // ── 헤더 ──────────────────────────────────────────────────
    lines.push_back(sep);
    lines.push_back(row(" [ ALLY INFO ]", "[ ENEMY INFO ]"));
    lines.push_back(dash);

    // ── Paladin 스탯 (왼쪽) / 적 목록 (오른쪽) ────────────────
    // 왼쪽 Paladin 줄 목록
    std::vector<std::string> leftLines;
    leftLines.push_back(" <UNIT: Paladin>");
    leftLines.push_back(dash.substr(0, LCOL));
    if (!allies.empty()) {
        const auto& p = allies[0];
        leftLines.push_back("  ATK : " + std::to_string(p.atk));
        leftLines.push_back("  HP  : " + std::to_string(p.hp) + "/" + std::to_string(p.maxHp));
        leftLines.push_back("  AP  : " + std::to_string(state.getAp()) + "/" + std::to_string(state.getMaxAp()));
    }

    // 오른쪽 적 목록 줄
    std::vector<std::string> rightLines;
    rightLines.push_back("<ENEMY: Orc Warrior>");
    rightLines.push_back(std::string(RCOL, '-'));
    for (int i = 0; i < (int)enemies.size(); ++i) {
        const auto& e = enemies[i];
        std::string status = e.isAlive()
            ? "E" + std::to_string(i+1)
              + " HP:" + std::to_string(e.hp)
              + " ATK:" + std::to_string(e.atk)
            : "E" + std::to_string(i+1) + " [defeated]";
        rightLines.push_back(status);
    }

    // 두 열 합치기
    size_t rows = std::max(leftLines.size(), rightLines.size());
    for (size_t i = 0; i < rows; ++i) {
        std::string l = (i < leftLines.size())  ? leftLines[i]  : "";
        std::string r = (i < rightLines.size()) ? rightLines[i] : "";
        lines.push_back(padTo(l, LCOL) + "| " + r);
    }

    lines.push_back(sep);
    return lines;
}

// 왼쪽 맵 아래 출력: TOWER STATUS, REMAINING ENEMIES, Phase, Log, 조작법
std::vector<std::string> GameUI::renderCommandLines(const GameState& state) const {
    const int W = 64;
    std::string sep(W, '=');
    std::string dash(W, '-');
    int half = W / 2;

    const auto& allies  = state.getAllies();
    const auto& enemies = state.getEnemies();

    int aliveEnemies = 0;
    for (auto& e : enemies) if (e.isAlive()) aliveEnemies++;

    std::vector<std::string> lines;

    // TOWER STATUS | REMAINING ENEMIES
    lines.push_back(sep);
    lines.push_back(padTo(" <TOWER STATUS>", half)
                  + "| REMAINING ENEMIES: " + std::to_string(aliveEnemies));
    lines.push_back(padTo("  - HP: " + std::to_string(state.getTowerHp())
                         + "/" + std::to_string(state.getTowerMaxHp()), half)
                  + "| ----------------------------");
    lines.push_back(padTo("  - DP: " + std::to_string(state.getTowerDp()), half) + "|");
    lines.push_back(dash);

    // Phase & Log
    std::string phaseStr;
    switch (state.getPhase()) {
        case Phase::PLAYER_TURN: phaseStr = "[ Player Turn - Wave " + std::to_string(state.getWave()) + " ]"; break;
        case Phase::ENEMY_TURN:  phaseStr = "[ Enemy Turn... ]"; break;
        case Phase::WAVE_CLEAR:  phaseStr = "[ ** WAVE CLEAR! ** ]"; break;
        case Phase::GAME_OVER:   phaseStr = "[ !! GAME OVER !! ]"; break;
    }
    lines.push_back(" " + phaseStr);
    lines.push_back(" Log: " + state.getLastMessage());
    lines.push_back(sep);

    return lines;
}
