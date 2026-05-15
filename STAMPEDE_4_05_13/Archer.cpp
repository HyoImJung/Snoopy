#include "Archer.h"
#include "GameState.h"
#include <sstream>

bool Archer::performAttack(char dir, GameState& state) {
    int dx = 0, dy = 0;
    if (dir == 'w') dy = -1;
    else if (dir == 's') dy = 1;
    else if (dir == 'a') dx = -1;
    else if (dir == 'd') dx = 1;
    else return false;

    int targetX = -1, targetY = -1;
    Character* targetEnemy = nullptr;
    bool found = false;

    // 1. 사거리(1~5칸) 내의 가장 가까운 적을 직선상에서 탐색
    for (int r = 1; r <= atkRange; ++r) {
        int cx = x + (dx * r);
        int cy = y + (dy * r);

        if (cx < 0 || cx >= 15 || cy < 0 || cy >= 15) break;

        for (auto e : state.getEnemies()) {
            if (e->isAlive() && e->x == cx && e->y == cy) {
                targetX = cx;
                targetY = cy;
                targetEnemy = e;
                found = true;
                break;
            }
        }
        if (found) break;
    }

    // 2. 적을 찾았다면 데미지 계산 및 적용
    if (found && targetEnemy) {
        int finalAtk = atk;
        std::string bonusMsg = "";

        // 궁수 특성: 맨해튼 거리가 4 이상이면 데미지 1.5배
        int dist = state.manhattanDist(x, y, targetX, targetY);
        if (dist >= 4) {
            finalAtk = static_cast<int>(atk * 1.5);
            bonusMsg = "[Long Shot! 1.5x Damage] ";
        }

        targetEnemy->hp -= finalAtk;
        state.setLastMessage(bonusMsg + name + " shot " + targetEnemy->name + " for " + std::to_string(finalAtk) + " HP!");
        return true;
    }

    state.setLastMessage(name + " couldn't find any target in that direction.");
    return false;
}