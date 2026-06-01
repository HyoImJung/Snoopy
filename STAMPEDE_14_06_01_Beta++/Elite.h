#ifndef ELITE_H
#define ELITE_H

#include "Character.h"
#include "GameState.h"

// =========================================================
// 엘리트 1: 리치왕 (Lich King)
// =========================================================
class LichKing : public Character {
private:
    int turnCounter;
public:
    LichKing(int px, int py);
    std::string getIcon() const override { return "L"; }
    bool performAttack(char dir, GameState& state) override { return false; }

    bool isUndead() const override { return true; }
    bool resistsPurification() const override { return true; } // 패시브: 정화 저항

    // 턴을 소모하는 고유 스킬을 썼다면 true 반환
    bool processEliteTurn(GameState& state);
};

// =========================================================
// 엘리트 2: 악마왕 (Demon King)
// =========================================================
class DemonKing : public Character {
private:
    int turnCounter;
public:
    DemonKing(int px, int py);
    std::string getIcon() const override { return "V"; } // 일반 악마(d)와 구분하기 위해 K 사용
    bool performAttack(char dir, GameState& state) override { return false; }

    // 패시브: 악마들의 왕 (스킬 데미지 50% 경감)
    bool takeSkillDamage(int dmg, int tx = -1, int ty = -1) override;

    bool processEliteTurn(GameState& state);
};

#endif