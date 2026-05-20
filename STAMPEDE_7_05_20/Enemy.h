#ifndef ENEMY_H
#define ENEMY_H

#include "Character.h"
#include <vector>
#include <string>

class GameState;

class Enemy : public Character {
public:
    Enemy(std::string n, int px, int py, int h, int a);

    std::string getIcon() const override { return "E"; }

    bool performAttack(char dir, GameState& state) override;
};

class EnemyManager {
public:
    void spawnWave(int wave, std::vector<Character*>& enemies);

    void processAITurn(std::vector<Character*>& enemies, GameState& state);
};

#endif