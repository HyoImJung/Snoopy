#include "Knight.h"
#include "GameState.h"

bool Knight::performAttack(char dir, GameState& state) {
    int dx = 0, dy = 0;
    if (dir == 'w') dy = -1;
    else if (dir == 's') dy = 1;
    else if (dir == 'a') dx = -1;
    else if (dir == 'd') dx = 1;
    else return false;

    int tx = x + dx;
    int ty = y + dy;

    // (see implementation)
    if (tx < 0 || tx >= 15 || ty < 0 || ty >= 15) return false;

    // (see implementation)
    for (auto e : state.getEnemies()) {
        if (e->isAlive() && e->x == tx && e->y == ty) {
            e->hp -= atk; // (see implementation)
            state.setLastMessage(name + " hit " + e->name + " for " + std::to_string(atk) + " HP!");
            return true; // (see implementation)
        }
    }

    state.setLastMessage(name + " swung into the air...");
    return false; // (see implementation)
}