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

    // (see implementation)
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

    // (see implementation)
    if (found) {
        state.setLastMessage("Wizard's Fireball exploded at (" + std::to_string(targetX) + ", " + std::to_string(targetY) + ")!");

        bool hitAny = false;
        for (auto e : state.getEnemies()) {
            if (!e->isAlive()) continue;

            // (see implementation)
            int diffX = std::abs(e->x - targetX);
            int diffY = std::abs(e->y - targetY);

            if (diffX <= 1 && diffY <= 1) {
                if (diffX == 0 && diffY == 0) {
                    e->hp -= atk; // (see implementation)
                }
                else {
                    e->hp -= static_cast<int>(atk * 0.5); // (see implementation)
                }
                hitAny = true;
            }
        }
        return hitAny;
    }

    state.setLastMessage("No target in magic range (4-7 blocks).");
    return false;
}