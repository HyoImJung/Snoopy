#ifndef PRIEST_H
#define PRIEST_H

#include "Character.h"

class Priest : public Character {
public:
    // (see implementation)
    Priest(int px, int py);

    std::string getIcon() const override { return "P"; }

    // (see implementation)
    bool performAttack(char dir, class GameState& state) override;
};

#endif