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
#include <set>
#include <map>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#endif

static void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

static void wcout_line(const wchar_t* text) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD written;
    WriteConsoleW(h, text, (DWORD)wcslen(text), &written, NULL);
    WriteConsoleW(h, L"\n", 1, &written, NULL);
}

// ── DEFEAT screen ─────────────────────────────────────────────
static int g_defeatChoice = -1;

static void showDefeatScreen() {
    static const wchar_t* art[] = {
        L"  \u2588\u2588\u2588\u2588\u2588\u2588\u2557 \u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2557\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2557\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2557 \u2588\u2588\u2588\u2588\u2588\u2557 \u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2557",
        L"  \u2588\u2588\u2554\u2550\u2550\u2588\u2588\u2557\u2588\u2588\u2554\u2550\u2550\u2550\u2550\u255d\u2588\u2588\u2554\u2550\u2550\u2550\u2550\u255d\u2588\u2588\u2554\u2550\u2550\u2550\u2550\u255d\u2588\u2588\u2554\u2550\u2550\u2588\u2588\u2557\u255a\u2550\u2550\u2588\u2588\u2554\u2550\u2550\u255d",
        L"  \u2588\u2588\u2551  \u2588\u2588\u2551\u2588\u2588\u2588\u2588\u2588\u2557  \u2588\u2588\u2588\u2588\u2588\u2557  \u2588\u2588\u2588\u2588\u2588\u2557  \u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2551   \u2588\u2588\u2551   ",
        L"  \u2588\u2588\u2551  \u2588\u2588\u2551\u2588\u2588\u2554\u2550\u2550\u255d  \u2588\u2588\u2554\u2550\u2550\u255d  \u2588\u2588\u2554\u2550\u2550\u255d  \u2588\u2588\u2554\u2550\u2550\u2588\u2588\u2551   \u2588\u2588\u2551   ",
        L"  \u2588\u2588\u2588\u2588\u2588\u2588\u2554\u255d\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2557\u2588\u2588\u2551     \u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2557\u2588\u2588\u2551  \u2588\u2588\u2551   \u2588\u2588\u2551   ",
        L"  \u255a\u2550\u2550\u2550\u2550\u2550\u255d \u255a\u2550\u2550\u2550\u2550\u2550\u2550\u255d\u255a\u2550\u255d     \u255a\u2550\u2550\u2550\u2550\u2550\u2550\u255d\u255a\u2550\u255d  \u255a\u2550\u255d   \u255a\u2550\u255d   ",
    };
    const int ART_W = 49;
    const int ART_H = 6;

    static const wchar_t* menuItems[2] = {
        L" \ucc98\uc74c\ubd80\ud130 \ub2e4\uc2dc\ud558\uae30",
        L" \ub85c\ube44 \ud654\uba74\uc73c\ub85c \ub098\uac00\uae30"
    };
    static const wchar_t* confirmMsg[2] = {
        L"\ucc98\uc74c\ubd80\ud130 \ub2e4\uc2dc \uc2dc\uc791\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?",
        L"\ub85c\ube44 \ud654\uba74\uc73c\ub85c \ub098\uac00\uc2dc\uaca0\uc2b5\ub2c8\uae4c?"
    };

    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

    auto wpr = [&](int x, int y, const wchar_t* text) {
        COORD pos = { (SHORT)x, (SHORT)y };
        SetConsoleCursorPosition(h, pos);
        DWORD written;
        WriteConsoleW(h, text, (DWORD)wcslen(text), &written, NULL);
    };

    auto getSize = [&](int& cols, int& rows) {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(h, &csbi);
        cols = csbi.srWindow.Right  - csbi.srWindow.Left + 1;
        rows = csbi.srWindow.Bottom - csbi.srWindow.Top  + 1;
    };

    int selected = 0;

    auto draw = [&]() {
        system("cls");
        int cols, rows;
        getSize(cols, rows);
        int artX = (cols - ART_W) / 2;
        int artY = rows / 2 - 6;
        if (artX < 0) artX = 0;
        if (artY < 1) artY = 1;
        for (int i = 0; i < ART_H; i++) {
            // Red color (ANSI escape: \033[31m ... \033[0m)
            DWORD written;
            const wchar_t* red   = L"\033[31m";
            const wchar_t* reset = L"\033[0m";
            WriteConsoleW(h, red,   (DWORD)wcslen(red),   &written, NULL);
            wpr(artX, artY + i, art[i]);
            WriteConsoleW(h, reset, (DWORD)wcslen(reset), &written, NULL);
        }
        int menuX = cols / 2 - 12;
        int menuY = artY + ART_H + 2;
        for (int i = 0; i < 2; i++) {
            wpr(menuX, menuY + i * 2, selected == i ? L"> " : L"  ");
            DWORD written;
            WriteConsoleW(h, menuItems[i], (DWORD)wcslen(menuItems[i]), &written, NULL);
            wpr(menuX + 26, menuY + i * 2, selected == i ? L" <" : L"  ");
        }
        wpr(cols / 2 - 15, menuY + 6,
            L"[ Enter: \ud655\uc778 ]  [ \u2191\u2193: \uc774\ub3d9 ]");
    };

    draw();
    g_defeatChoice = -1;

    while (g_defeatChoice == -1) {
        wint_t c = _getwch();
        if (c == 0xE0 || c == 0) {
            wint_t arrow = _getwch();
            if (arrow == 72 && selected > 0) { selected--; draw(); }
            if (arrow == 80 && selected < 1)  { selected++; draw(); }
        }
        else if (c == 13) {
            int cols, rows;
            getSize(cols, rows);
            int cx = cols / 2, cy = rows / 2;
            // Fixed box width = 37 cols
            // Each confirm message padded to 37 cols internally
            static const wchar_t* popLines[2][2] = {
                // Restart: pad to 37 cols
                { L"\u2551  \ucc98\uc74c\ubd80\ud130 \ub2e4\uc2dc \uc2dc\uc791\ud558\uc2dc\uaca0\uc2b5\ub2c8\uae4c?    \u2551",
                  L"\u2551  Enter: \ud655\uc778   Backspace: \ucde8\uc18c      \u2551" },
                // Lobby: pad to 37 cols
                { L"\u2551  \ub85c\ube44 \ud654\uba74\uc73c\ub85c \ub098\uac00\uc2dc\uaca0\uc2b5\ub2c8\uae4c?      \u2551",
                  L"\u2551  Enter: \ud655\uc778   Backspace: \ucde8\uc18c      \u2551" }
            };
            wpr(cx - 19, cy - 1, L"\u2554\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2557");
            wpr(cx - 19, cy,     popLines[selected][0]);
            wpr(cx - 19, cy + 1, popLines[selected][1]);
            wpr(cx - 19, cy + 2, L"\u255a\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u255d");
            wint_t c2 = _getwch();
            if (c2 == 13) {
                g_defeatChoice = selected;
            } else {
                draw();
            }
        }
    }
}

// ──────────────────────────────────────────────────────────────

static bool processInput(const std::string& raw, GameState& state) {
    if (raw.empty()) return true;
    if (raw == "q" || raw == "Q") return false;

    if (state.getIsSkillMenuOpen()) {
        if (raw == "b" || raw == "B") {
            state.closeSkillMenu();
            return true;
        }
        if (raw.length() == 1 && raw[0] >= '1' && raw[0] <= '3') {
            state.tryUseSkill(raw[0] - '1');
            return true;
        }
        state.setLastMessage("Invalid skill number! Press 1-3, or 'b' to cancel.");
        return true;
    }

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

    if (raw == "r" || raw == "R") {
        state.openSkillMenu();
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

static bool confirmExit() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    auto wpr = [&](int x, int y, const wchar_t* text) {
        COORD pos = { (SHORT)x, (SHORT)y };
        SetConsoleCursorPosition(h, pos);
        DWORD written;
        WriteConsoleW(h, text, (DWORD)wcslen(text), &written, NULL);
    };
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(h, &csbi);
    int cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    int cx = cols / 2, cy = rows / 2;
    wpr(cx - 17, cy - 1, L"\u2554\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2557");
    wpr(cx - 17, cy,     L"\u2551  \uba54\uc778 \uba54\ub274\ub85c \ub3cc\uc544\uac00\uc2dc\uaca0\uc2b5\ub2c8\uae4c?   \u2551");
    wpr(cx - 17, cy + 1, L"\u2551  y: \ud655\uc778   \uadf8 \uc678: \ucde8\uc18c           \u2551");
    wpr(cx - 17, cy + 2, L"\u255a\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u255d");
    wpr(cx - 4, cy + 3, L">> ");
    std::string ans;
    std::getline(std::cin, ans);
    return (ans == "y" || ans == "Y");
}

static void gameLoop(GameMap& map, GameState& state, int saveSlot) {
    GameUI ui;

    while (true) {
        clearScreen();

        std::vector<std::string> mapLines = map.renderLines(state);
        std::vector<std::string> uiLines  = ui.renderInterfaceLines(state);

        size_t totalRows = std::max<size_t>(mapLines.size(), uiLines.size());

        // Build a set of hit positions for quick lookup
        // Damage effect overlay
        {
            const auto effects = state.getHitEffects();
            state.clearHitEffects();

            // Merge effects at same (x,y): sum damage
            std::map<std::pair<int,int>, int> merged;
            for (const auto& ef : effects)
                merged[{ef.x, ef.y}] += ef.dmg;

            // Track modified byte ranges per line to avoid overlap
            // key=lineIdx, value=set of cellStart offsets already written
            std::map<int, std::set<int>> modifiedCells;

            for (const auto& kv : merged) {
                int ex = kv.first.first;
                int ey = kv.first.second;
                int dmg = kv.second;

                int lineIdx = 2 + ey * 2;
                if (lineIdx < 0 || lineIdx >= (int)mapLines.size()) continue;

                std::string& line = mapLines[lineIdx];
                int cellStart = 6 + ex * 6;
                if (cellStart + 3 > (int)line.size()) continue;
                if (modifiedCells[lineIdx].count(cellStart)) continue;

                std::string tag = "-" + std::to_string(dmg);
                std::string colored = "\033[41;97m";
                if ((int)tag.size() >= 3)
                    colored += tag.substr(0, 3);
                else
                    colored += tag + std::string(3 - tag.size(), ' ');
                colored += "\033[0m";

                // Adjust cellStart for previously inserted ANSI codes in same line
                // Each previous insertion adds (colored.size() - 3) extra bytes
                int extraBytes = 0;
                for (int prev : modifiedCells[lineIdx])
                    if (prev < cellStart) extraBytes += (int)colored.size() - 3;

                int adjustedStart = cellStart + extraBytes;
                if (adjustedStart + 3 > (int)line.size()) continue;

                line = line.substr(0, adjustedStart) + colored + line.substr(adjustedStart + 3);
                modifiedCells[lineIdx].insert(cellStart);
            }

            for (size_t i = 0; i < totalRows; ++i) {
                if (i < mapLines.size()) std::cout << mapLines[i];
                else if (!mapLines.empty()) std::cout << std::string(mapLines[0].length(), ' ');
                std::cout << "    ";
                if (i < uiLines.size()) std::cout << uiLines[i];
                std::cout << "\n";
            }

            if (!effects.empty()) {
                std::cout.flush();
                Sleep(300);
            }
        }

        std::cout << "\n";
        for (const auto& line : ui.renderCommandLines(state)) std::cout << line << "\n";
        std::cout << " [SAVE] p - Save to Slot " << (saveSlot + 1) << "   [MENU] b - back to menu\n";
        std::cout << "Command >> ";

        std::string input;
        std::getline(std::cin, input);

        if (input == "b" || input == "B") {
            if (confirmExit()) break;
            continue;
        }
        if (input == "p" || input == "P") {
            if (saveGame(state, saveSlot))
                state.setLastMessage("Saved to Slot " + std::to_string(saveSlot + 1) + "!");
            else
                state.setLastMessage("Save failed!");
            continue;
        }

        if (!processInput(input, state)) break;

        if (state.getTowerHp() <= 0) {
            showDefeatScreen();
            if (g_defeatChoice == 1) {
                break; // lobby
            } else {
                // restart
                GameMap newMap(15, 15);
                GameState newState(newMap);
                gameLoop(newMap, newState, saveSlot);
                break;
            }
        }
    }
}

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
