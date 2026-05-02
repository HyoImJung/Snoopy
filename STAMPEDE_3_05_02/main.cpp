#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "GameMap.h"
#include "GameState.h"
#include "GameUI.h"

#ifdef _WIN32
#include <windows.h>
#endif

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

bool processInput(const std::string& raw, GameState& state) {
    if (raw.empty()) return true;
    if (raw == "q" || raw == "Q") return false;

    if (raw.length() == 1 && raw[0] >= '1' && raw[0] <= '4') {
        state.setCurrentAllyIndex(raw[0] - '1');
        return true;
    }

    if (raw == "z" || raw == "Z") {
        state.setLastMessage("Manual Turn End. Enemy's turn begins.");
        for (auto& a : state.getAllies_mutable()) a.actedThisTurn = true;
        state.endAllyAction();
        return true;
    }

    int idx = state.getCurrentAllyIndex();
    if (idx == -1) {
        state.setLastMessage("Please select a unit first (1-4)!");
        return true;
    }

    int movedCount = 0;
    int maxRange = state.getAllies()[idx].moveRange;

    auto dirToDelta = [](char c, int& dx, int& dy) -> bool {
        if      (c == 'w') { dx =  0; dy = -1; return true; }
        else if (c == 's') { dx =  0; dy =  1; return true; }
        else if (c == 'a') { dx = -1; dy =  0; return true; }
        else if (c == 'd') { dx =  1; dy =  0; return true; }
        return false;
    };

    for (size_t i = 0; i < raw.size(); ++i) {
        char c = raw[i];

        if (c == 'f' || c == 'F') {
            if (i + 1 < raw.size()) { char dir = raw[++i]; state.tryAttackAlly(idx, dir); }
            continue;
        }

        int dx = 0, dy = 0;
        if (!dirToDelta(c, dx, dy)) continue;

        if (i + 1 < raw.size()) {
            int dx2 = 0, dy2 = 0;
            if (dirToDelta(raw[i + 1], dx2, dy2) && !(dx2 == dx && dy2 == dy)) {
                dx += dx2; dy += dy2;
                ++i;
            }
        }

        if (movedCount >= maxRange) {
            state.setLastMessage(state.getAllies()[idx].name + " reached movement limit!");
            continue;
        }

        if (state.tryMoveAlly(idx, dx, dy)) movedCount++;
    }
    return true;
}

int main() {
#ifdef _WIN32
    // Set console to UTF-8 so box-drawing characters render correctly
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    GameMap   map(15, 15);
    GameState state(map);
    GameUI    ui;

    while (true) {
        clearScreen();

        std::vector<std::string> mapLines = map.renderLines(state);
        std::vector<std::string> uiLines  = ui.renderInterfaceLines(state);

        size_t totalRows = std::max<size_t>(mapLines.size(), uiLines.size());
        for (size_t i = 0; i < totalRows; ++i) {
            if (i < mapLines.size()) std::cout << mapLines[i];
            else if (!mapLines.empty()) std::cout << std::string(mapLines[0].length(), ' ');
            std::cout << "    ";
            if (i < uiLines.size()) std::cout << uiLines[i];
            std::cout << "\n";
        }

        std::cout << "\n";
        for (const auto& line : ui.renderCommandLines(state)) std::cout << line << "\n";

        std::cout << "Command >> ";
        std::string input;
        std::getline(std::cin, input);

        if (!processInput(input, state)) break;

        if (state.getTowerHp() <= 0) {
            clearScreen();
            std::cout << "GAME OVER: The Tower has fallen!\n";
            break;
        }
    }

    return 0;
}
