#ifndef ELITE_H
#define ELITE_H

#include "Character.h"
#include "GameState.h"

// =========================================================

// =========================================================
class LichKing : public Character {
private:
    int turnCounter;
public:
    LichKing(int px, int py);
    std::string getIcon() const override { return "L"; }
    bool performAttack(char dir, GameState& state) override { return false; }

    bool isUndead() const override { return true; }
    bool resistsPurification() const override { return true; }


    bool processEliteTurn(GameState& state);
};

// =========================================================

// =========================================================
class DemonKing : public Character {
private:
    int turnCounter;
public:
    DemonKing(int px, int py);
    std::string getIcon() const override { return "V"; }
    bool performAttack(char dir, GameState& state) override { return false; }


    bool takeSkillDamage(int dmg, int tx = -1, int ty = -1) override;

    bool processEliteTurn(GameState& state);
};

#endif
