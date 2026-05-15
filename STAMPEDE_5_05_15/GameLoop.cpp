#include "GameLoop.h"
#include "GameMap.h"
#include "GameState.h"
#include "GameUI.h"
#include "SaveLoad.h"
#include "Character.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#endif

// ---- Functions from original main.cpp ----

static void wcout_line(const wchar_t* text) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD written;
    WriteConsoleW(h, text, (DWORD)wcslen(text), &written, NULL);
    WriteConsoleW(h, L"\n", 1, &written, NULL);
}

static void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

static bool processInput(const std::string& raw, GameState& state) {
    if (raw.empty()) return true;
    if (raw == "q" || raw == "Q") return false;

    if (raw.length() == 1 && raw[0] >= '1' && raw[0] <= '4') {
        state.setCurrentAllyIndex(raw[0] - '1');
        return true;
    }

    if (raw == "z" || raw == "Z") {
        state.setLastMessage("Manual Turn End.");
        for (auto* a : state.getAllies()) if (a) a->actedThisTurn = true;
        state.endAllyAction();
        return true;
    }

    int idx = state.getCurrentAllyIndex();
    if (idx == -1) {
        state.setLastMessage("Please select a unit first (1-4)!");
        return true;
    }

    auto dirToDelta = [](char c, int& dx, int& dy) -> bool {
        if (c == 'w') { dx = 0; dy = -1; return true; }
        else if (c == 's') { dx = 0; dy = 1; return true; }
        else if (c == 'a') { dx = -1; dy = 0; return true; }
        else if (c == 'd') { dx = 1; dy = 0; return true; }
        return false;
    };

    Character* selectedAlly = state.getAllies()[idx];
    int movedCount = 0;
    int maxRange = selectedAlly->moveRange;

    for (size_t i = 0; i < raw.size(); ++i) {
        char c = raw[i];

        if (c == 'f' || c == 'F') {
            if (i + 1 < raw.size()) {
                char dir = raw[++i];
                state.tryAttackAlly(idx, dir);
            }
            continue;
        }

        int dx = 0, dy = 0;
        if (!dirToDelta(c, dx, dy)) continue;

        if (i + 1 < raw.size()) {
            int dx2 = 0, dy2 = 0;
            if (dirToDelta(raw[i + 1], dx2, dy2)) {
                if ((dx != 0 && dy2 != 0) || (dy != 0 && dx2 != 0)) {
                    dx += dx2; dy += dy2; i++;
                }
            }
        }

        if (movedCount < maxRange) {
            if (state.tryMoveAlly(idx, dx, dy)) movedCount++;
            else break;
        } else {
            state.setLastMessage("Movement limit reached!");
            break;
        }
    }
    return true;
}

// In-game exit confirm popup
// Returns: true=go to menu, false=keep playing
static bool confirmExit() {
    std::cout << "\n \xE2\x95\x94\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x97\n";
    std::cout << " \xE2\x95\x91  \xEB\xA9\x94\xEC\x9D\xB8 \xEB\xA9\x94\xEB\x89\xB4\xEB\xA1\x9C \xEB\x8F\x8C\xEC\x95\x84\xEA\xB0\x80\xEC\x8B\x9C\xEA\xB2\xA0\xEC\x8A\xB5\xEB\x8B\x88\xEA\xB9\x8C?   \xE2\x95\x91\n";
    std::cout << " \xE2\x95\x91  y: \xED\x99\x95\xEC\x9D\xB8   \xEA\xB7\xB8 \xEC\x99\xB8: \xEC\xB7\xA8\xEC\x86\x8C           \xE2\x95\x91\n";
    std::cout << " \xE2\x95\x9A\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x9D\n";
    std::cout << " >> ";
    std::string ans;
    std::getline(std::cin, ans);
    return (ans == "y" || ans == "Y");
}

// Main game loop (shared for new/load game)
static void gameLoop(GameMap& map, GameState& state, int saveSlot) {
    GameUI ui;

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

        // Add save hint
        std::cout << " [SAVE] p - Save to Slot " << (saveSlot + 1) << "   [MENU] b - back to menu\n";
        std::cout << "Command >> ";

        std::string input;
        std::getline(std::cin, input);

        // b key to go back to menu
        if (input == "b" || input == "B") {
            if (confirmExit()) break;
            continue;
        }

        // Save
        if (input == "p" || input == "P") {
            if (saveGame(state, saveSlot))
                state.setLastMessage("Saved to Slot " + std::to_string(saveSlot + 1) + "!");
            else
                state.setLastMessage("Save failed!");
            continue;
        }

        if (!processInput(input, state)) break;

        if (state.getTowerHp() <= 0) {
            clearScreen();
            std::cout << "GAME OVER: The Tower has fallen!\n";
            break;
        }
    }
}

// ---- Public entry functions ----

void runNewGame(int saveSlot) {
    GameMap   map(15, 15);
    GameState state(map);
    gameLoop(map, state, saveSlot);
}

void runLoadGame(int saveSlot) {
    GameMap   map(15, 15);
    GameState state(map);
    if (!loadGame(state, saveSlot)) {
        wcout_line(L"\ubd88\ub7ec\uc624\uae30 \uc2e4\ud328. \uc5d4\ud130\ub97c \ub204\ub974\uc138\uc694...");
        std::string dummy; std::getline(std::cin, dummy);
        return;
    }
    state.setLastMessage("Slot " + std::to_string(saveSlot + 1) + " loaded!");
    gameLoop(map, state, saveSlot);
}
