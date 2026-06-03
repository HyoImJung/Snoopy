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


class Slime : public Character {
public:
    bool hasSplit;
    bool splitPending; // signals GameState to spawn a new Slime at an adjacent tile
    Slime(int px, int py);
    std::string getIcon() const override { return "S"; }
    bool performAttack(char dir, GameState& state) override;
    bool takeDamage(int dmg, int tx = -1, int ty = -1) override;
};

class Goblin : public Character {
public:
    Goblin(int px, int py);
    std::string getIcon() const override { return "G"; }
    bool performAttack(char dir, GameState& state) override;
};

class Orc : public Character {
public:
    Orc(int px, int py);
    std::string getIcon() const override { return "O"; }
    bool performAttack(char dir, GameState& state) override;
};

class Undead : public Character {
public:
    Undead(int px, int py);
    std::string getIcon() const override { return "U"; }
    bool performAttack(char dir, GameState& state) override;
    bool isUndead() const override { return true; }
};

class Demon : public Character {
public:
    int demonTurnCounter;
    Demon(int px, int py);
    std::string getIcon() const override { return "d"; }
    bool performAttack(char dir, GameState& state) override;
};

class EnemyManager {
public:
    void spawnWave(int wave, std::vector<Character*>& enemies);
    void processAITurn(std::vector<Character*>& enemies, GameState& state);
};

#endif
