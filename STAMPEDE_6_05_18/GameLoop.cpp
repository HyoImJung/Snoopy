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

    // ----------------------------------------------------
    // [분기 A] 이미 스킬 메뉴가 열려있는 상태일 때의 입력 처리
    // ----------------------------------------------------
    if (state.getIsSkillMenuOpen()) {
        if (raw == "b" || raw == "B") {
            state.closeSkillMenu(); // 스킬 창 닫기
            return true;
        }

        if (raw.length() == 1 && raw[0] >= '1' && raw[0] <= '3') {
            int skillIdx = raw[0] - '1';
            state.tryUseSkill(skillIdx); // 스킬 발동 시도
            return true;
        }

        state.setLastMessage("Invalid skill number! Press 1-3, or 'b' to cancel.");
        return true;
    }

    // ----------------------------------------------------
    // [분기 B] 일반Tactical 모드일 때의 입력 처리
    // ----------------------------------------------------

    // 유닛 선택 (1-4)
    if (raw.length() == 1 && raw[0] >= '1' && raw[0] <= '4') {
        state.setCurrentAllyIndex(raw[0] - '1');
        return true;
    }

    // 턴 종료 (z)
    if (raw == "z" || raw == "Z") {
        state.setLastMessage("Manual Turn End.");
        for (auto* a : state.getAllies()) if (a) a->actedThisTurn = true;
        state.endAllyAction();
        return true;
    }

    // ★ 이 부분이 기존의 이동/공격 루프보다 '위쪽'에 명시되어 있어야 합니다!
    if (raw == "r" || raw == "R") {
        state.openSkillMenu(); // GameState의 스킬창 오픈 함수 트리거 호출
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
        clearScreen(); // 화면을 깨끗이 지우고 시작

        // 1. 현재 GameState의 상태(스킬창 오픈 여부 포함)를 기반으로 라인들을 렌더링
        std::vector<std::string> mapLines = map.renderLines(state);
        std::vector<std::string> uiLines = ui.renderInterfaceLines(state); // <- 여기서 동적 UI가 반영됩니다.

        // 2. 맵과 우측 UI 정보창을 가로로 결합하여 출력
        size_t totalRows = std::max<size_t>(mapLines.size(), uiLines.size());
        for (size_t i = 0; i < totalRows; ++i) {
            if (i < mapLines.size()) std::cout << mapLines[i];
            else if (!mapLines.empty()) std::cout << std::string(mapLines[0].length(), ' ');

            std::cout << "    "; // 맵과 UI 사이 공백

            if (i < uiLines.size()) std::cout << uiLines[i];
            std::cout << "\n";
        }

        std::cout << "\n";
        // 하단 커맨드 가이드 가로선 및 로그 출력
        for (const auto& line : ui.renderCommandLines(state)) std::cout << line << "\n";

        std::cout << " [SAVE] p - Save to Slot " << (saveSlot + 1) << "   [MENU] b - back to menu\n";
        std::cout << "Command >> ";

        std::string input;
        std::getline(std::cin, input);

        // 메뉴 이동 및 저장 기능
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

        // 3. 핵심: 입력을 처리하고 나면 다음 루프에서 수정된 UI가 반영되어야 합니다.
        if (!processInput(input, state)) break;

        // 게임 오버 체크
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
