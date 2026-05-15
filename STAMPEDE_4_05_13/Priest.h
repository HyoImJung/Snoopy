#ifndef PRIEST_H
#define PRIEST_H

#include "Character.h"

class Priest : public Character {
public:
    // 생성자: 이름 "Priest", HP 30, ATK 5, 이동 2, 사거리 1
    Priest(int px, int py)
        : Character("Priest", CharacterClass::PRIEST, px, py, 30, 5, 2, 1, true) {}

    std::string getIcon() const override { return "P"; }

    // 사제 전용 로직 (방향에 따른 힐 또는 공격)
    bool performAttack(char dir, class GameState& state) override;
};

#endif