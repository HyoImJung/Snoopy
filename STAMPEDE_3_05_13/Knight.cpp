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

    // 맵 밖으로 나가는지 체크
    if (tx < 0 || tx >= 15 || ty < 0 || ty >= 15) return false;

    // 해당 위치에 있는 적 탐색
    for (auto e : state.getEnemies()) {
        if (e->isAlive() && e->x == tx && e->y == ty) {
            e->hp -= atk; // 기사의 공격력만큼 대미지
            state.setLastMessage(name + " hit " + e->name + " for " + std::to_string(atk) + " HP!");
            return true; // 공격 성공
        }
    }

    state.setLastMessage(name + " swung into the air...");
    return false; // 공격 대상 없음
}