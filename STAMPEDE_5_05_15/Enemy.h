#ifndef ENEMY_H
#define ENEMY_H

#include "Character.h"
#include <vector>
#include <string>

// GameState Ŭ  ˸ ( )
class GameState;

//   Ŭ
class Enemy : public Character {
public:
    Enemy(std::string n, int px, int py, int h, int a);

    std::string getIcon() const override { return "E"; }

    //    (ʿ  , ⼭ AI óϹǷ ⺻ )
    bool performAttack(char dir, GameState& state) override;
};

//  ü ϰ  ϴ  Ŭ
class EnemyManager {
public:
    // (see implementation)
    void spawnWave(int wave, std::vector<Character*>& enemies);

    //  AI ̵   óϴ Լ
    void processAITurn(std::vector<Character*>& enemies, GameState& state);
};

#endif