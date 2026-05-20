#include "GameState.h"
#include "Knight.h"
#include "Archer.h"
#include "Priest.h"
#include "Mage.h"
#include "Enemy.h"
#include <algorithm>
#include <cmath>
#include <sstream>

// Constructor: initialize data and create units
GameState::GameState(GameMap& m)
    : map(m), phase(Phase::PLAYER_TURN), currentAllyIndex(-1),
    towerHp(50), towerMaxHp(50), ap(50), maxAp(50), wave(1)
{
    // Dynamically allocate each class object for polymorphism
    allies.push_back(new Knight(9, 12));
    allies.push_back(new Archer(10, 12));
    allies.push_back(new Priest(11, 12));
    allies.push_back(new Mage(12, 12));

    // Spawn first wave enemies via EnemyManager
    enemyMgr.spawnWave(wave, enemies);

    syncMapTiles();
    lastMessage = "Welcome! AP: 50/50. Select unit (1-4).";
}

// Destructor: free dynamically allocated memory
GameState::~GameState() {
    for (auto a : allies) delete a;
    for (auto e : enemies) delete e;
}

// Select unit
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

// Ally movement logic
bool GameState::tryMoveAlly(int idx, int dx, int dy) {
    if (idx < 0 || idx >= (int)allies.size()) return false;
    Character* a = allies[idx];

    if (!a->isAlive() || a->actedThisTurn) return false;

    // Cannot move if AP < 1
    if (ap < 1) {
        lastMessage = "Not enough AP! (Required: 1)";
        return false;
    }

    int nx = a->x + dx;
    int ny = a->y + dy;

    // Map boundary check
    if (nx < 0 || nx >= 15 || ny < 0 || ny >= 15) return false;

    // Terrain and unit collision check
    if (map.getTileAt(nx, ny) != TileType::EMPTY) return false;

    // Process movement
    a->x = nx;
    a->y = ny;

    ap -= 1;

    syncMapTiles();
    lastMessage = a->name + " moved. AP: " + std::to_string(ap);
    return true;
}

// Ally attack logic
bool GameState::tryAttackAlly(int idx, char dir) {
    if (idx < 0 || idx >= (int)allies.size()) return false;
    Character* a = allies[idx];

    if (!a->isAlive() || a->actedThisTurn) return false;

    // Call performAttack implemented in each class (Knight, Archer, etc.)
    if (a->performAttack(dir, *this)) {
        a->actedThisTurn = true;
        syncMapTiles();
        checkWaveClear();
        return true;
    }

    lastMessage = "Attack missed or invalid direction.";
    return false;
}

// 1. 스킬 메뉴 열기
void GameState::openSkillMenu() {
    // 유닛이 선택되어 있고, 아직 이번 턴에 행동하지 않은 경우에만 스킬 메뉴를 열 수 있도록 제한
    if (currentAllyIndex != -1 && !allies[currentAllyIndex]->actedThisTurn) {
        isSkillMenuOpen = true;
        lastMessage = allies[currentAllyIndex]->name + " - Select Skill (1-3) or [B]ack";
    }
    else {
        lastMessage = "Please select a ready unit first!";
    }
}

// 2. 스킬 메뉴 닫기
void GameState::closeSkillMenu() {
    isSkillMenuOpen = false;
    lastMessage = "Skill menu closed.";
}

// 3. 스킬 사용 시도 (이전에 설계한 다형성 기반 코드)
bool GameState::tryUseSkill(int skillIdx) {
    if (currentAllyIndex == -1 || !isSkillMenuOpen) return false;
    Character* caster = allies[currentAllyIndex];

    if (skillIdx < 0 || skillIdx >= (int)caster->skills.size()) return false;
    Skill* targetSkill = caster->skills[skillIdx];

    // 행동력(AP) 검사
    if (ap < targetSkill->apCost) {
        lastMessage = "Not enough Player AP! (Required: " + std::to_string(targetSkill->apCost) + ")";
        return false;
    }

    // 스킬 다형성 실행
    if (targetSkill->execute(*caster, *this)) {
        ap -= targetSkill->apCost; // 스킬 코스트만큼 플레이어 AP 차감

        // 사용 성공 후 상태 정리
        isSkillMenuOpen = false;
        caster->actedThisTurn = true;
        currentAllyIndex = -1;

        syncMapTilesPublic(); // 맵 타일 동기화 함수 호출
        checkWaveClear();
        if (phase != Phase::WAVE_CLEAR) endAllyAction();
        return true;
    }

    return false;
}

// Calculate movable positions
std::vector<std::pair<int, int>> GameState::getMovablePositions() const {
    std::vector<std::pair<int, int>> res;
    if (currentAllyIndex == -1 || currentAllyIndex >= (int)allies.size()) return res;

    Character* sel = allies[currentAllyIndex];
    if (!sel->isAlive() || sel->actedThisTurn || ap < 1) return res;

    // Direction setup (8 directions)
    int dx[] = { 0,  0, -1, 1, -1,  1, -1, 1 };
    int dy[] = { -1, 1,  0, 0, -1, -1,  1, 1 };

    int maxDist = sel->moveRange; // Use moveRange set on character object
    bool allowDiagonal = (sel->cls != CharacterClass::PRIEST); // Priest cannot move diagonally

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

// Sync map data with unit positions
void GameState::syncMapTiles() {
    // Reset map (remove unit tiles, keep terrain)
    for (int y = 0; y < 15; ++y) {
        for (int x = 0; x < 15; ++x) {
            TileType t = map.getTileAt(x, y);
            if (t == TileType::HERO || t == TileType::ENEMY) {
                map.setTile(x, y, TileType::EMPTY);
            }
        }
    }

    // Place allies
    for (auto a : allies) {
        if (a->isAlive()) map.setTile(a->x, a->y, TileType::HERO);
    }
    // Place enemies
    for (auto e : enemies) {
        if (e->isAlive()) map.setTile(e->x, e->y, TileType::ENEMY);
    }
}

// Check ally turn end and switch to enemy turn
void GameState::endAllyAction() {
    phase = Phase::ENEMY_TURN;
    lastMessage = "Enemy Turn Begins!";

    // Process enemy AI (delegated to EnemyManager)
    enemyMgr.processAITurn(enemies, *this);

    // Reset ally state and return to player turn
    for (auto a : allies) a->actedThisTurn = false;
    ap = std::min(maxAp, ap + 15); // Restore AP on turn end
    phase = Phase::PLAYER_TURN;
    syncMapTiles();
}

// Check wave clear
void GameState::checkWaveClear() {
    bool anyEnemy = false;
    for (auto e : enemies) {
        if (e->isAlive()) {
            anyEnemy = true;
            break;
        }
    }

    if (!anyEnemy) {
        for (auto e : enemies) delete e;
        enemies.clear();
        
        wave++;
        lastMessage = "Wave " + std::to_string(wave) + " Clear! New enemies spawn.";
        enemyMgr.spawnWave(wave, enemies);
        syncMapTiles();
    }
}

int GameState::manhattanDist(int x1, int y1, int x2, int y2) const {
    return std::abs(x1 - x2) + std::abs(y1 - y2);
}
