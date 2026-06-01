#ifndef ARCHER_H
#define ARCHER_H

#include "Character.h"

class Archer : public Character {
public:
    // (see implementation)
    Archer(int px, int py);

    std::string getIcon() const override { return "A"; }

    // (see implementation)
    bool performAttack(char dir, class GameState& state) override;
};

#endif