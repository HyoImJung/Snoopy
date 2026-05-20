#include "Priest.h"
#include "GameState.h"
#include <algorithm>

bool Priest::performAttack(char dir, GameState& state) {
    int dx = 0, dy = 0;
    if (dir == 'w') dy = -1;
    else if (dir == 's') dy = 1;
    else if (dir == 'a') dx = -1;
    else if (dir == 'd') dx = 1;
    else return false;

    int tx = x + dx;
    int ty = y + dy;

    //   üũ
    if (tx < 0 || tx >= 15 || ty < 0 || ty >= 15) return false;

    // (see implementation)
    for (auto a : state.getAllies()) {
        if (a->isAlive() && a->x == tx && a->y == ty) {
            if (a->hp >= a->maxHp) {
                state.setLastMessage(a->name + " is already at full HP!");
                return false; // ̹ ǮǸ ൿ  ϰ
            }
            int healAmt = 15; //  
            a->hp = std::min(a->hp + healAmt, a->maxHp);
            state.setLastMessage(name + " healed " + a->name + " for " + std::to_string(healAmt) + " HP!");
            return true;
        }
    }

    // (see implementation)
    for (auto e : state.getEnemies()) {
        if (e->isAlive() && e->x == tx && e->y == ty) {
            e->hp -= atk;
            state.setLastMessage(name + " cast Holy Light on " + e->name + "!");
            return true;
        }
    }

    state.setLastMessage("Nothing there to heal or attack.");
    return false;
}