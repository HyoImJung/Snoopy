#include <iostream>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include "GameMap.h"
#include "GameUI.h"
#include "GameState.h"
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

static void enableANSI() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hOut, &mode);
    SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
}

static std::string padTo(const std::string& s, int w) {
    if ((int)s.size() >= w) return s;
    return s + std::string(w - s.size(), ' ');
}

bool processInput(const std::string& raw, GameState& state) {
    if (raw == "q" || raw == "Q") return false;
    if (state.getPhase() != Phase::PLAYER_TURN) return true;

    int idx = state.getCurrentAllyIndex();
    if (idx < 0) return true;

    for (int i = 0; i < (int)raw.size(); ) {
        char c = raw[i];
        if (c == 'f' || c == 'F') {
            int j = i + 1;
            while (j < (int)raw.size() && raw[j] == ' ') ++j;
            if (j < (int)raw.size()) {
                char dir = raw[j];
                const auto& p = state.getAllies()[idx];
                int tx = p.x, ty = p.y;
                if      (dir == 'w') ty -= 1;
                else if (dir == 's') ty += 1;
                else if (dir == 'a') tx -= 1;
                else if (dir == 'd') tx += 1;
                state.tryAttackAlly(idx, tx, ty);
                i = j + 1;
            } else { ++i; }
        }
        else if (c == 'w') { state.tryMoveAlly(idx, 0, -1); idx = state.getCurrentAllyIndex(); ++i; }
        else if (c == 's') { state.tryMoveAlly(idx, 0,  1); idx = state.getCurrentAllyIndex(); ++i; }
        else if (c == 'a') { state.tryMoveAlly(idx, -1, 0); idx = state.getCurrentAllyIndex(); ++i; }
        else if (c == 'd') { state.tryMoveAlly(idx,  1, 0); idx = state.getCurrentAllyIndex(); ++i; }
        else if (c == 'z') {
            auto& al = state.getAllies_mutable();
            if (idx >= 0 && idx < (int)al.size()) {
                al[idx].actedThisTurn = true;
                state.endAllyAction();
                idx = state.getCurrentAllyIndex();
            }
            ++i;
        }
        else { ++i; }
    }
    return true;
}

int main() {
    enableANSI();
    GameMap map(15, 15);
    GameState state(map);
    GameUI ui;

    int lastLines = 0;

    while (true) {
        if (lastLines > 0)
            std::cout << "\033[" << lastLines << "A\033[J";

        // ── 맵(왼쪽 64자) + 오른쪽 패널(ALLY/ENEMY INFO) side-by-side ──
        std::vector<std::string> mapLines    = map.renderLines();      // 32줄
        std::vector<std::string> rightPanel  = ui.renderInterfaceLines(state); // ~6줄

        int topRows = (int)std::max(mapLines.size(), rightPanel.size());
        for (int i = 0; i < topRows; ++i) {
            std::string l = (i < (int)mapLines.size())   ? mapLines[i]   : "";
            std::string r = (i < (int)rightPanel.size()) ? rightPanel[i] : "";
            std::cout << padTo(l, 64) << " " << r << "\n";
        }

        // ── 맵 아래: TOWER STATUS / Phase / Log / 조작법 (64자 너비) ──
        std::vector<std::string> bottomPanel = ui.renderCommandLines(state);
        for (auto& l : bottomPanel)
            std::cout << l << "\n";

        int totalLines = topRows + (int)bottomPanel.size() + 2;
        lastLines = totalLines;

        Phase phase = state.getPhase();
        if (phase == Phase::WAVE_CLEAR || phase == Phase::GAME_OVER) {
            std::cout << "\nPress Enter to exit...";
            std::cout.flush();
            std::cin.ignore();
            std::cin.get();
            break;
        }

        std::cout << "\n> ";
        std::cout.flush();

        std::string line;
        if (!std::getline(std::cin, line)) break;
        if (!processInput(line, state)) break;
    }

    return 0;
}
