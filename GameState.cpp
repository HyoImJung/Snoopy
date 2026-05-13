#include "GameState.h"
#include "Knight.h"
#include "Archer.h"
#include "Priest.h"
#include "Mage.h"
#include "Enemy.h"
#include <algorithm>
#include <cmath>
#include <sstream>

// 생성자: 초기 데이터 설정 및 유닛 생성
GameState::GameState(GameMap& m)
    : map(m), phase(Phase::PLAYER_TURN), currentAllyIndex(-1),
    towerHp(50), towerMaxHp(50), ap(50), maxAp(50), wave(1)
{
    // 다형성을 위해 각 클래스 객체를 동적 할당하여 관리
    allies.push_back(new Knight(9, 12));
    allies.push_back(new Archer(10, 12));
    allies.push_back(new Priest(11, 12));
    allies.push_back(new Mage(12, 12));

    // 첫 번째 웨이브 적 스폰 (EnemyManager 사용)
    enemyMgr.spawnWave(wave, enemies);

    syncMapTiles();
    lastMessage = "Welcome! AP: 50/50. Select unit (1-4).";
}

// 소멸자: 동적 할당된 메모리 해제
GameState::~GameState() {
    for (auto a : allies) delete a;
    for (auto e : enemies) delete e;
}

// 유닛 선택
void GameState::setCurrentAllyIndex(int idx) {
    if (idx >= 0 && idx < (int)allies.size()) {
        if (allies[idx]->isAlive()) {
            currentAllyIndex = idx;
            lastMessage = allies[idx]->name + " selected.";
        }
        else {
            lastMessage = "That unit is fallen.";
        }
    }
}

// 아군 이동 로직
bool GameState::tryMoveAlly(int idx, int dx, int dy) {
    if (idx < 0 || idx >= (int)allies.size()) return false;
    Character* a = allies[idx];

    if (!a->isAlive() || a->actedThisTurn) return false;

    // AP가 1 미만이면 이동 불가
    if (ap < 1) {
        lastMessage = "Not enough AP! (Required: 1)";
        return false;
    }

    int nx = a->x + dx;
    int ny = a->y + dy;

    // 맵 경계 체크
    if (nx < 0 || nx >= 15 || ny < 0 || ny >= 15) return false;

    // 지형 및 유닛 충돌 체크
    if (map.getTileAt(nx, ny) != TileType::EMPTY) return false;

    // 이동 처리
    a->x = nx;
    a->y = ny;

    ap -= 1;

    syncMapTiles();
    lastMessage = a->name + " moved. AP: " + std::to_string(ap);
    return true;
}

// 아군 공격 로직
bool GameState::tryAttackAlly(int idx, char dir) {
    if (idx < 0 || idx >= (int)allies.size()) return false;
    Character* a = allies[idx];

    if (!a->isAlive() || a->actedThisTurn) return false;

    // 각 클래스(기사, 궁수 등)에 구현된 performAttack 호출
    if (a->performAttack(dir, *this)) {
        a->actedThisTurn = true;
        syncMapTiles();
        checkWaveClear();
        return true;
    }

    lastMessage = "Attack missed or invalid direction.";
    return false;
}

// 이동 가능한 위치 계산
std::vector<std::pair<int, int>> GameState::getMovablePositions() const {
    std::vector<std::pair<int, int>> res;
    if (currentAllyIndex == -1 || currentAllyIndex >= (int)allies.size()) return res;

    Character* sel = allies[currentAllyIndex];
    if (!sel->isAlive() || sel->actedThisTurn || ap < 1) return res;

    // 방향 설정 (8방향)
    int dx[] = { 0,  0, -1, 1, -1,  1, -1, 1 };
    int dy[] = { -1, 1,  0, 0, -1, -1,  1, 1 };

    int maxDist = sel->moveRange; // 캐릭터 객체에 설정된 moveRange사용
    bool allowDiagonal = (sel->cls != CharacterClass::PRIEST); // 사제만 대각선 불가

    int dirCount = allowDiagonal ? 8 : 4;

    for (int d = 0; d < dirCount; ++d) {
        for (int dist = 1; dist <= maxDist; ++dist) {
            int nx = sel->x + (dx[d] * dist);
            int ny = sel->y + (dy[d] * dist);

            if (nx < 0 || nx >= 15 || ny < 0 || ny >= 15) break;
            if (map.getTileAt(nx, ny) != TileType::EMPTY || isOccupied(nx, ny)) break;

            res.push_back({ nx, ny });
        }
    }
    return res;
}

bool GameState::isOccupied(int x, int y) const {
    for (auto a : allies) {
        if (a->isAlive() && a->x == x && a->y == y) return true;
    }
    for (auto e : enemies) {
        if (e->isAlive() && e->x == x && e->y == y) return true;
    }
    return false;
}

// 맵 데이터와 유닛 위치 동기화
void GameState::syncMapTiles() {
    // 맵 초기화 (지형 제외 유닛 타일 제거)
    for (int y = 0; y < 15; ++y) {
        for (int x = 0; x < 15; ++x) {
            TileType t = map.getTileAt(x, y);
            if (t == TileType::HERO || t == TileType::ENEMY) {
                map.setTile(x, y, TileType::EMPTY);
            }
        }
    }

    // 아군 배치
    for (auto a : allies) {
        if (a->isAlive()) map.setTile(a->x, a->y, TileType::HERO);
    }
    // 적군 배치
    for (auto e : enemies) {
        if (e->isAlive()) map.setTile(e->x, e->y, TileType::ENEMY);
    }
}

// 아군 턴 종료 체크 및 적 턴 전환
void GameState::endAllyAction() {
    phase = Phase::ENEMY_TURN;
    lastMessage = "Enemy Turn Begins!";

    // 적 AI 처리 (EnemyManager에 위임)
    enemyMgr.processAITurn(enemies, *this);

    // 아군 상태 초기화 및 플레이어 턴으로 복귀
    for (auto a : allies) a->actedThisTurn = false;
    ap = std::min(maxAp, ap + 15); // 턴 종료 시 AP 회복
    phase = Phase::PLAYER_TURN;
    syncMapTiles();
}

// 웨이브 클리어 확인
void GameState::checkWaveClear() {
    bool anyEnemy = false;
    for (auto e : enemies) {
        if (e->isAlive()) {
            anyEnemy = true;
            break;
        }
    }

    if (!anyEnemy) {
        wave++;
        lastMessage = "Wave " + std::to_string(wave) + " Clear! New enemies spawn.";
        enemyMgr.spawnWave(wave, enemies);
        syncMapTiles();
    }
}

int GameState::manhattanDist(int x1, int y1, int x2, int y2) const {
    return std::abs(x1 - x2) + std::abs(y1 - y2);
}