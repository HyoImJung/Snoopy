#ifndef KNIGHT_H
#define KNIGHT_H

#include "Character.h"

class Knight : public Character {
public:
    // 생성자: 이름, 직업, 위치, HP, ATK, 이동거리, 사거리, 진영 설정
    Knight(int px, int py)
        : Character("Knight", CharacterClass::KNIGHT, px, py, 50, 15, 1, 1, true) {}

    // 기사 고유의 아이콘 반환
    std::string getIcon() const override { return "K"; }

    // 기사 고유의 공격 로직 (상하좌우 1칸)
    bool performAttack(char dir, class GameState& state) override;
};

#endif