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
                state.setLastMessage(e->name + " attacked " + a->name + "!");
                attacked = true;
                break;
            }
        }

        // 2. 공격하지 않았다면 타워를 향해 1칸 이동 시도
        if (!attacked) {
            // 우선순위 1: 전진 (y축 아래로 이동)
            if (e->y < towerY && state.canEnemyMoveTo(e->x, e->y + 1)) {
                e->y += 1;
            }
            // 우선순위 2: 타워가 오른쪽에 있다면 오른쪽 이동
            else if (e->x < towerX && state.canEnemyMoveTo(e->x + 1, e->y)) {
                e->x += 1;
            }
            // 우선순위 3: 타워가 왼쪽에 있다면 왼쪽 이동
            else if (e->x > towerX && state.canEnemyMoveTo(e->x - 1, e->y)) {
                e->x -= 1;
            }
            // 모든 방향이 지형이나 다른 유닛에 막혔다면 이번 턴은 대기(이동하지 않음)
        }
    }
}