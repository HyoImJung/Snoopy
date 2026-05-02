#include <iostream>
#include <string>
#include <vector>
#include <algorithm> // std::max를 위해 추가
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
        for (auto& a : state.getAllies_mutable()) {
            a.actedThisTurn = true;
        }
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

    for (size_t i = 0; i < raw.size(); ++i) {
        char c = raw[i];
        if (c == 'w' || c == 'a' || c == 's' || c == 'd') {
            if (movedCount >= maxRange) {
                state.setLastMessage(state.getAllies()[idx].name + " reached movement limit!");
                continue;
            }
            bool success = false;
            if (c == 'w')      success = state.tryMoveAlly(idx, 0, -1);
            else if (c == 's') success = state.tryMoveAlly(idx, 0, 1);
            else if (c == 'a') success = state.tryMoveAlly(idx, -1, 0);
            else if (c == 'd') success = state.tryMoveAlly(idx, 1, 0);

            if (success) movedCount++;
        }
        else if (c == 'f' || c == 'F') {
            if (i + 1 < raw.size()) {
                char dir = raw[++i];
                state.tryAttackAlly(idx, dir);
            }
        }
    }
    return true;
}

int main() {
    GameMap map(15, 15);
    GameState state(map);
    GameUI ui;

    while (true) {
        clearScreen();

        // 1. 맵 정보와 UI 정보를 줄 단위 벡터로 가져옵니다.
        std::vector<std::string> mapLines = map.renderLines(state);
            std::vector<std::string> uiLines = ui.renderInterfaceLines(state);

        // 2. 두 정보 중 더 긴 행 수를 기준으로 루프를 돕니다.
        size_t totalRows = std::max<size_t>(mapLines.size(), uiLines.size());

        for (size_t i = 0; i < totalRows; ++i) {
            // 왼쪽: 맵 출력
            if (i < mapLines.size()) {
                std::cout << mapLines[i];
            }
            else {
                // 맵 줄이 끝났으면 맵 너비만큼 공백 출력 (정렬 유지)
                if (!mapLines.empty()) std::cout << std::string(mapLines[0].length(), ' ');
            }

            // 간격 추가
            std::cout << "    ";

            // 오른쪽: UI 정보 출력
            if (i < uiLines.size()) {
                std::cout << uiLines[i];
            }

            std::cout << "\n";
        }

        // 3. 하단 명령어 가이드 및 로그는 전체 너비로 출력
        auto cmdLines = ui.renderCommandLines(state);
        std::cout << "\n";
        for (const auto& line : cmdLines) {
            std::cout << line << std::endl;
        }

        std::cout << "Command >> ";
        std::string input;
        std::getline(std::cin, input);

        if (!processInput(input, state)) break;

        if (state.getTowerHp() <= 0) {
            clearScreen();
            std::cout << "GAME OVER: The Tower has fallen!" << std::endl;
            break;
        }
    }

    return 0;
}