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
// Enemy.cpp 의 EnemyManager::processAITurn 함수 수정

// Enemy.cpp 의 EnemyManager::processAITurn 함수 전체 교체

void EnemyManager::processAITurn(std::vector<Character*>& enemies, GameState& state) {
    int towerX = 7;
    int towerY = 14;

    for (auto e : enemies) {
        if (!e->isAlive()) continue;

        // 1. 공격 인접 체크 (기존 유지)
        bool attacked = false;
        for (auto a : state.getAllies()) {
            if (a->isAlive() && state.manhattanDist(e->x, e->y, a->x, a->y) == 1) {
                a->hp -= e->atk;
                state.addHitEffect(a->x, a->y, e->atk);
                state.setLastMessage(e->name + " attacked " + a->name + "!");
                attacked = true;
                break;
            }
        }

        // 2. 타워 인접 시 타워 공격
        if (!attacked) {
            if (state.manhattanDist(e->x, e->y, towerX, towerY) == 1) {
                state.damageTower(e->atk);
                state.setLastMessage(e->name + " attacked the Base!");
                attacked = true;
            }
        }

        // 3. 공격하지 않았다면 타워를 향해 1칸 이동
        if (!attacked) {
            if (e->y < towerY && state.canEnemyMoveTo(e->x, e->y + 1)) {
                e->y += 1;
            }
            else if (e->x < towerX && state.canEnemyMoveTo(e->x + 1, e->y)) {
                e->x += 1;
            }
            else if (e->x > towerX && state.canEnemyMoveTo(e->x - 1, e->y)) {
                e->x -= 1;
            }
        }
    }
}