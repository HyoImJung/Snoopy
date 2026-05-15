#include "Enemy.h"
#include "GameState.h"
#include <string>

// (see implementation)
Enemy::Enemy(std::string n, int px, int py, int h, int a)
    : Character(n, CharacterClass::ENEMY_ORC, px, py, h, a, 1, 1, false) {}

bool Enemy::performAttack(char dir, GameState& state) {
    // (see implementation)
    return false;
}

// (see implementation)
void EnemyManager::spawnWave(int wave, std::vector<Character*>& enemies) {
    // (see implementation)
    // (see implementation)
    for (int i = 0; i < 6; ++i) {
        enemies.push_back(new Enemy("Orc" + std::to_string(i + 1),
            i * 2, 0,           // (see implementation)
            45 + (wave * 5),    // (see implementation)
            4 + wave));         // (see implementation)
    }
}

// (see implementation)
void EnemyManager::processAITurn(std::vector<Character*>& enemies, GameState& state) {
    int towerX = 7; // (see implementation)
    int towerY = 14;

    for (auto e : enemies) {
        if (!e->isAlive()) continue;

        // (see implementation)
        bool attacked = false;
        for (auto a : state.getAllies()) {
            if (a->isAlive() && state.manhattanDist(e->x, e->y, a->x, a->y) == 1) {
                a->hp -= e->atk;
                state.setLastMessage(e->name + " attacked " + a->name + "!");
                attacked = true;
                break;
            }
        }

        // (see implementation)
        // (see implementation)

        // (see implementation)
        if (!attacked) {
            if (e->y < towerY) e->y += 1;
            else if (e->x < towerX) e->x += 1;
            else if (e->x > towerX) e->x -= 1;
        }
    }
}