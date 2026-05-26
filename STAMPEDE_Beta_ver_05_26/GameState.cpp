#include "GameState.h"
#include "Knight.h"
#include "Archer.h"
#include "Priest.h"
#include "Mage.h"
#include "Enemy.h"
#include "Elite.h"
#include "Boss.h" // ★ 보스 시스템 추가
#include <algorithm>
#include <cmath>
#include <sstream>
#include <utility>

GameState::GameState(GameMap& m)
    : map(m), phase(Phase::PLAYER_TURN), currentAllyIndex(-1),
    towerHp(50), towerMaxHp(50), ap(50), maxAp(50), wave(1), isSkillMenuOpen(false)
{
    allies.push_back(new Knight(9, 12));
    allies.push_back(new Archer(10, 12));
    allies.push_back(new Priest(11, 12));
    allies.push_back(new Mage(12, 12));

    enemyMgr.spawnWave(wave, enemies);
    syncMapTiles();
    lastMessage = "Welcome! AP: 50/50. Select unit (1-4).";
}

GameState::~GameState() {
    for (auto a : allies) delete a;
    for (auto e : enemies) delete e;
}

int GameState::manhattanDist(int x1, int y1, int x2, int y2) const {
    return std::abs(x1 - x2) + std::abs(y1 - y2);
}

void GameState::setCurrentAllyIndex(int idx) {
    if (idx >= 0 && idx < (int)allies.size()) {
        if (allies[idx]->isAlive()) {
            currentAllyIndex = idx;

            // ★ 기절 및 유혹 상태일 때 선택 시 UI 피드백 문자열 분기
            std::string statusStr = "";
            if (allies[idx]->isStunned) {
                statusStr = " (Stunned)";
            }
            // ★ [수정] isCharmed 대신 charmedBy 포인터가 비어있지 않은지(nullptr이 아닌지) 검사합니다.
            else if (allies[idx]->charmedBy != nullptr) {
                statusStr = " (Charmed)";
            }

            lastMessage = allies[idx]->name + statusStr + " selected.";
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

    // 1. 최소 요구 AP 검사 (AP가 1보다 작으면 이동 불가)
    if (ap < 1) {
        lastMessage = "Not enough AP! (Required: 1)";
        return false;
    }

    int nx = a->x + dx;
    int ny = a->y + dy;

    // 2. 맵 경계선 검사
    if (nx < 0 || nx >= 15 || ny < 0 || ny >= 15) return false;

    // 3. 지형 및 유닛 충돌 검사
    if (map.getTileAt(nx, ny) != TileType::EMPTY) return false;

    // ★ [추가] 감전 시 이동 불가!
    if (a->isShocked) {
        lastMessage = a->name + " is SHOCKED and cannot move!";
        return false;
    }

    // 4. 버프 확인 및 AP 차감
    // 무료 이동 버프가 있다면 횟수만 차감하고 AP 소모는 건너뜀
    if (a->freeMoveCells > 0) {
        a->freeMoveCells--;
    }
    else {
        // 일반 이동 시 AP 1 소모
        ap -= 1;
    }

    // 5. 실제 이동 처리 및 맵 동기화
    a->x = nx;
    a->y = ny;
    syncMapTiles();
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


    // ★ [추가] 탈진 상태라면 스킬 AP 요구량이 10 증가합니다!
    int finalApCost = caster->isExhausted ? targetSkill->apCost + 10 : targetSkill->apCost;

    // 행동력(AP) 검사
    if (ap < finalApCost) {
        lastMessage = "Not enough AP! (Required: " + std::to_string(finalApCost) + (caster->isExhausted ? " [Exhausted penalty]" : "") + ")";
        return false;
    }

    // 스킬 실행
    if (targetSkill->execute(*caster, *this)) {
        ap -= finalApCost; // ★ 증가된 코스트로 차감
        isSkillMenuOpen = false; // 스킬을 쓰면 스킬창은 무조건 닫힙니다.

        // ★ [핵심] 스킬이 턴을 소모하는 타입일 때만 행동 완료 및 유닛 선택 해제!
        if (targetSkill->endsTurn) {
            caster->actedThisTurn = true;
            currentAllyIndex = -1; // 선택 해제
        }
        // 버프 스킬(endsTurn == false)일 경우에는 actedThisTurn이 바뀌지 않고 유닛 선택도 유지됩니다!

        syncMapTilesPublic();
        checkWaveClear();

        // ★ [버그 수정] 기존 코드에 있던 endAllyAction(); 강제 호출을 삭제했습니다.
        // 스킬 하나 썼다고 다른 아군들의 턴까지 통째로 날아가며 적 턴이 되는 문제를 방지합니다.

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


// ★ 다중 타일 보스 충돌 판정 (isOccupying 기반)
bool GameState::isOccupied(int x, int y) const {
    for (auto a : allies) if (a->isAlive() && a->isOccupying(x, y)) return true;
    for (auto e : enemies) if (e->isAlive() && e->isOccupying(x, y)) return true;
    return false;
}

// ★ 맵 동기화 (다중 타일 보스 'D'와 역린 'R' 표시 지원)
void GameState::syncMapTiles() {
    for (int y = 0; y < 15; ++y) {
        for (int x = 0; x < 15; ++x) {
            TileType t = map.getTileAt(x, y);
            if (t == TileType::HERO || t == TileType::ENEMY) map.setTile(x, y, TileType::EMPTY);
        }
    }
    for (auto a : allies) if (a->isAlive()) map.setTile(a->x, a->y, TileType::HERO);
    for (auto e : enemies) {
        if (e->isAlive()) {
            // 보스 등 다중 타일 객체의 모든 위치에 ENEMY 마킹
            for (int y = 0; y < 15; y++) for (int x = 0; x < 15; x++)
                if (e->isOccupying(x, y)) map.setTile(x, y, TileType::ENEMY);
        }
    }
}

void GameState::endAllyAction() {
    phase = Phase::ENEMY_TURN;
    lastMessage = "Enemy Turn Begins!";
    enemyMgr.processAITurn(enemies, *this);

    // ★ 턴 종료 시 상태 리셋 및 기절 해제 로직
    for (auto a : allies) {
        if (a->isAlive()) {

            // 이번 턴에 쓰지 않은 일회성 버프들은 턴이 넘어가면 초기화
            a->freeMoveCells = 0;
            a->multiHitCount = 1;

            // ★ [추가] 적의 공격 턴이 끝나고 돌아왔으므로, 남은 보호막은 소멸합니다.
            a->shieldHp = 0;

            for (auto a : allies) {
                if (a->isAlive()) {
                    a->freeMoveCells = 0;
                    a->multiHitCount = 1;
                    a->shieldHp = 0;
                    if (a->isShocked) a->isShocked = false;

                    // ★ [추가] 유혹당한 상태라면 악마를 향해 강제로 걸어갑니다!
            if (a->charmedBy) {
                if (a->charmedBy->isAlive()) {
                    int tx = a->charmedBy->x;
                    int ty = a->charmedBy->y;

                    bool moved = false; // 이동 성공 여부 체크
                    
                    // X축으로 먼저 이동 시도
                    if (a->x < tx && canEnemyMoveTo(a->x + 1, a->y)) { a->x++; moved = true; }
                    if (!moved && a->x > tx && canEnemyMoveTo(a->x - 1, a->y)) { a->x--; moved = true; }

                    // X축 이동에 실패했거나(장애물), 이미 X 좌표가 같다면 Y축 이동 시도
                    if (!moved && a->y < ty && canEnemyMoveTo(a->x, a->y + 1)) { a->y++; moved = true; }
                    if (!moved && a->y > ty && canEnemyMoveTo(a->x, a->y - 1)) { a->y--; moved = true; }
                }

                a->actedThisTurn = true; // 이동 후 강제로 턴 종료!
                a->charmedBy = nullptr;  // 1턴 후 유혹 해제
                a->isStunned = false;    // 기절보다 유혹이 우선시됨

            }
            else if (a->isStunned) {
                a->actedThisTurn = true;
                a->isStunned = false;
            }
            else {
                a->actedThisTurn = false;
            }

                    // ★ [수정] 기절(Stun)과 유혹(Charm) 모두 1턴 행동 불가로 처리
                    if (a->isStunned) {
                        a->actedThisTurn = true;
                        a->isStunned = false;
                    }
                    else {
                        a->actedThisTurn = false;
                    }
                }
            }
        }
    }
    ap = std::min(maxAp, ap + 15);
    phase = Phase::PLAYER_TURN;
    syncMapTiles();
}

void GameState::checkWaveClear() {
    bool anyEnemy = false;
    for (auto e : enemies) if (e->isAlive()) { anyEnemy = true; break; }

    if (!anyEnemy) {
        // ★ 4웨이브(보스) 클리어 시 승리 조건
        if (wave == 4) {
            phase = Phase::GAME_OVER;
            lastMessage = "VICTORY!! The Dragon has fallen!";
            return;
        }

        for (auto e : enemies) delete e;
        enemies.clear();
        wave++;
        lastMessage = "Wave " + std::to_string(wave - 1) + " Clear!";
        enemyMgr.spawnWave(wave, enemies);
        syncMapTiles();
    }
}

// =======================================================
// [세이브/로드용 팩토리 함수] 
// SaveLoad.cpp에서 몬스터 종류(cls)를 넘겨주면 알맞은 객체로 복원합니다.
// =======================================================
Character* createEnemyInstance(int cls, int x, int y) {
    switch (static_cast<CharacterClass>(cls)) {
    case CharacterClass::ENEMY_SLIME: return new Slime(x, y);
    case CharacterClass::ENEMY_GOBLIN: return new Goblin(x, y);
    case CharacterClass::ENEMY_ORC: return new Orc(x, y);
    case CharacterClass::ENEMY_UNDEAD: return new Undead(x, y);
    case CharacterClass::ENEMY_DEMON: return new Demon(x, y);
    case CharacterClass::ELITE_LICH_KING: return new LichKing(x, y);
    case CharacterClass::ELITE_DEMON_KING: return new DemonKing(x, y);
    case CharacterClass::BOSS_DRAGON: {
        DragonBoss* boss = new DragonBoss();
        boss->x = x;
        boss->y = y;
        return boss;
    }
    default: return new Enemy("Unknown", x, y, 10, 1); // 예비용
    }
}