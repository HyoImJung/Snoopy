#include "GameUI.h"
#include <iostream>
#include <iomanip>

void GameUI::printLine(std::string symbol) const {
    for (int i = 0; i < UI_WIDTH; ++i) std::cout << symbol;
    std::cout << std::endl;
}

void GameUI::drawInterface() const {
    int half = UI_WIDTH / 2;
    printLine("=");
    std::cout << std::left << std::setw(half) << " [ ALLY INFO ]" << "|" << " [ ENEMY INFO ]" << std::endl;
    printLine("-");

    // 타워 및 적 정보 출력 로직
    std::cout << std::left << std::setw(half) << " <TOWER STATUS>" << "|" << " REMAINING ENEMIES: 12" << std::endl;
    std::cout << std::left << std::setw(half) << "  - HP: 100/100" << "|" << " ----------------------------" << std::endl;
    std::cout << std::left << std::setw(half) << "  - DP: 50" << "|" << std::endl;

    printLine("-");
    std::cout << std::left << std::setw(half) << " <UNIT: Paladin>" << "|" << " <ENEMY: Orc Warrior>" << std::endl;
    std::cout << std::left << std::setw(half) << "  - ATK: 15 / AP: 3" << "|" << "  - HP: 45 / ATK: 8" << std::endl;
    printLine("=");
}