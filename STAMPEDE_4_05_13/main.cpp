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

    // 1. 유닛 선택 처리 (1-4)
    if (raw.length() == 1 && raw[0] >= '1' && raw[0] <= '4') {
        state.setCurrentAllyIndex(raw[0] - '1');
        return true;
    }

    // 2. 턴 강제 종료 처리 (z)
    if (raw == "z" || raw == "Z") {
        state.setLastMessage("Manual Turn End.");
        for (auto* a : state.getAllies()) if (a) a->actedThisTurn = true;
        state.endAllyAction();
        return true;
    }

    // 3. 현재 선택된 유닛 확인
    int idx = state.getCurrentAllyIndex();
    if (idx == -1) {
        state.setLastMessage("Please select a unit first (1-4)!");
        return true;
    }

    // --- [중요] dirToDelta 람다 함수를 호출부보다 '위쪽'에 정의합니다 ---
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

    // 4. 입력 문자열 순차 처리 (wawawawa 등)
    for (size_t i = 0; i < raw.size(); ++i) {
        char c = raw[i];

        // 공격(f) 처리
        if (c == 'f' || c == 'F') {
            if (i + 1 < raw.size()) {
                char dir = raw[++i];
                state.tryAttackAlly(idx, dir);
            }
            continue;
        }

        int dx = 0, dy = 0;
        // 이제 위에서 정의했으므로 dirToDelta를 인식할 수 있습니다.
        if (!dirToDelta(c, dx, dy)) continue;

        // 대각선 판정: 다음 문자도 방향키이고 현재와 수직이면 합침
        if (i + 1 < raw.size()) {
            int dx2 = 0, dy2 = 0;
            if (dirToDelta(raw[i + 1], dx2, dy2)) {
                if ((dx != 0 && dy2 != 0) || (dy != 0 && dx2 != 0)) {
                    dx += dx2;
                    dy += dy2;
                    i++; // 두 글자를 소모
                }
            }
        }

        // 이동 실행 (법사 등 최대 range까지)
        if (movedCount < maxRange) {
            if (state.tryMoveAlly(idx, dx, dy)) {
                movedCount++;
            }
            else {
                break; // 장애물 등에 막힘
            }
        }
        else {
            state.setLastMessage("Movement limit reached!");
            break;
        }
    }
    return true;
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    GameMap   map(15, 15);
    GameState state(map);
    GameUI    ui;

    while (true) {
        clearScreen();

        std::vector<std::string> mapLines = map.renderLines(state);
        std::vector<std::string> uiLines = ui.renderInterfaceLines(state);

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