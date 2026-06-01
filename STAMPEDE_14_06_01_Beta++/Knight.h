#ifndef KNIGHT_H
#define KNIGHT_H

#include "Character.h"

class Knight : public Character {
public:
    // (see implementation)
    Knight(int px, int py);
        
    // (see implementation)
    std::string getIcon() const override { return "K"; }

    // (see implementation)
    bool performAttack(char dir, class GameState& state) override;
};

#endif