#ifndef MAGE_H
#define MAGE_H

#include "Character.h"

class Mage : public Character {
public:
    // 생성자: 이름 "Wizard", HP 25, ATK 20, 이동 4, 사거리 7
    Mage(int px, int py)
        : Character("Wizard", CharacterClass::MAGE, px, py, 25, 20, 4, 7, true) {}

    std::string getIcon() const override { return "W"; }

    // 마법사 전용 공격 로직 (원거리 탐색 + 3x3 스플래시)
    bool performAttack(char dir, class GameState& state) override;
};

#endif