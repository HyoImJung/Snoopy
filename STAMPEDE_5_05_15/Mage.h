#ifndef MAGE_H
#define MAGE_H

#include "Character.h"

class Mage : public Character {
public:
    // (see implementation)
    Mage(int px, int py)
        : Character("Wizard", CharacterClass::MAGE, px, py, 25, 20, 4, 7, true) {}

    std::string getIcon() const override { return "W"; }

    // (see implementation)
    bool performAttack(char dir, class GameState& state) override;
};

#endif