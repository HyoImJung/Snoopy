#ifndef ARCHER_H
#define ARCHER_H

#include "Character.h"

class Archer : public Character {
public:
    // (see implementation)
    Archer(int px, int py)
        : Character("Archer", CharacterClass::ARCHER, px, py, 35, 12, 3, 5, true) {}

    std::string getIcon() const override { return "A"; }

    // (see implementation)
    bool performAttack(char dir, class GameState& state) override;
};

#endif