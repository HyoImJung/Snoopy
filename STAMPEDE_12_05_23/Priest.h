#ifndef PRIEST_H
#define PRIEST_H

#include "Character.h"

class Priest : public Character {
public:
    // (see implementation)
    Priest(int px, int py)
        : Character("Priest", CharacterClass::PRIEST, px, py, 30, 5, 2, 1, true) {}

    std::string getIcon() const override { return "P"; }

    // (see implementation)
    bool performAttack(char dir, class GameState& state) override;
};

#endif