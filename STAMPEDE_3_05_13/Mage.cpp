#include "Mage.h"
#include "GameState.h"
#include <algorithm>
#include <cmath>

bool Mage::performAttack(char dir, GameState& state) {
    int dx = 0, dy = 0;
    if (dir == 'w') dy = -1;
    else if (dir == 's') dy = 1;
    else if (dir == 'a') dx = -1;
    else if (dir == 'd') dx = 1;
    else return false;

    int targetX = -1, targetY = -1;
    bool found = false;

    // 1. 최소 사거리 4칸 ~ 최대 사거리 7칸 사이의 적 탐색
    for (int r = 4; r <= atkRange; ++r) {
        int cx = x + (dx * r);
        int cy = y + (dy * r);

        if (cx < 0 || cx >= 15 || cy < 0 || cy >= 15) break;

        for (auto e : state.getEnemies()) {
            if (e->isAlive() && e->x == cx && e->y == cy) {
                targetX = cx;
                targetY = cy;
                found = true;
                break;
            }
        }
        if (found) break;
    }

    // 2. 타격 지점을 찾았다면 3x3 스플래시 대미지 적용
    if (found) {
        state.setLastMessage("Wizard's Fireball exploded at (" + std::to_string(targetX) + ", " + std::to_string(targetY) + ")!");

        bool hitAny = false;
        for (auto e : state.getEnemies()) {
            if (!e->isAlive()) continue;

            // 타격 지점으로부터의 거리 계산 (x, y 차이가 각각 1 이하이면 3x3 범위)
            int diffX = std::abs(e->x - targetX);
            int diffY = std::abs(e->y - targetY);

            if (diffX <= 1 && diffY <= 1) {
                if (diffX == 0 && diffY == 0) {
                    e->hp -= atk; // 중심부는 100% 대미지
                }
                else {
                    e->hp -= static_cast<int>(atk * 0.5); // 주변 8칸은 50% 대미지
                }
                hitAny = true;
            }
        }
        return hitAny;
    }

    state.setLastMessage("No target in magic range (4-7 blocks).");
    return false;
}