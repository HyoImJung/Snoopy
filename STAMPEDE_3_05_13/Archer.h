#ifndef ARCHER_H
#define ARCHER_H

#include "Character.h"

class Archer : public Character {
public:
    // 생성자: 이름 "Archer", 사거리 5, 이동거리 3 등으로 설정
    Archer(int px, int py)
        : Character("Archer", CharacterClass::ARCHER, px, py, 35, 12, 3, 5, true) {}

    std::string getIcon() const override { return "A"; }

    // 궁수 전용 공격 로직 (직선 탐색 + 거리 보너스)
    bool performAttack(char dir, class GameState& state) override;
};

#endif