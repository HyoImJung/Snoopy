#ifndef MAGE_H
#define MAGE_H

#include "Character.h"

class Mage : public Character {
public:
    // (see implementation)
    Mage(int px, int py);

    std::string getIcon() const override { return "M"; }

    // (see implementation)
    bool performAttack(char dir, class GameState& state) override;
};

#endif