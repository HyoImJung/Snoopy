#include "Enemy.h"
#include "GameState.h"
#include <string>

// Enemy 생성자 구현
Enemy::Enemy(std::string n, int px, int py, int h, int a)
    : Character(n, CharacterClass::ENEMY_ORC, px, py, h, a, 1, 1, false) {}

bool Enemy::performAttack(char dir, GameState& state) {
    // 적은 수동 입력(f+방향)으로 공격하지 않으므로 false 반환
    return false;
}

// EnemyManager: 적 생성 로직
void EnemyManager::spawnWave(int wave, std::vector<Character*>& enemies) {
    // 기존에 존재하던 적 메모리 해제 (필요 시)
    // GameState 소멸자에서 처리하므로 여기서는 새로 추가만 담당
    for (int i = 0; i < 6; ++i) {
        enemies.push_back(new Enemy("Orc" + std::to_string(i + 1),
            i * 2, 0,           // X: 가로 배치, Y: 맵 최상단
            45 + (wave * 5),    // 웨이브당 체력 증가
            4 + wave));         // 웨이브당 공격력 증가
    }
}

// EnemyManager: 적 AI 로직 (기존 GameState에 있던 내용을 옮겨옴)
void EnemyManager::processAITurn(std::vector<Character*>& enemies, GameState& state) {
    int towerX = 7; // 타워 위치 예시
    int towerY = 14;

    for (auto e : enemies) {
        if (!e->isAlive()) continue;

        // 1. 주변에 아군이 있으면 공격
        bool attacked = false;
        for (auto a : state.getAllies()) {
            if (a->isAlive() && state.manhattanDist(e->x, e->y, a->x, a->y) == 1) {
                a->hp -= e->atk;
                state.setLastMessage(e->name + " attacked " + a->name + "!");
                attacked = true;
                break;
            }
        }

        // 2. 타워 근처면 타워 공격 (타워 체력 깎는 함수 호출 필요)
        // (참고: GameState에 타워 체력 감소 로직이 있다면 여기 추가)

        // 3. 공격하지 않았다면 타워 방향으로 한 칸 전진
        if (!attacked) {
            if (e->y < towerY) e->y += 1;
            else if (e->x < towerX) e->x += 1;
            else if (e->x > towerX) e->x -= 1;
        }
    }
}