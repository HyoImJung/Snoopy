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
#include <cstdlib>

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
    spawnTreasure(); // 1웨이브 보물상자 배치
    lastMessage = "환영합니다! AP: 50/50. 유닛을 선택하세요 (1-4).";
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
                statusStr = " (기절)";
            }
            // ★ [수정] isCharmed 대신 charmedBy 포인터가 비어있지 않은지(nullptr이 아닌지) 검사합니다.
            else if (allies[idx]->charmedBy != nullptr) {
                statusStr = " (매혹)";
            }

            lastMessage = allies[idx]->name + statusStr + " 선택됨.";
        }
        else {
            lastMessage = "쓰러진 유닛입니다.";
        }
    }
}

// Ally movement logic
bool GameState::tryMoveAlly(int idx, int dx, int dy) {
    if (idx < 0 || idx >= (int)allies.size()) return false;
    Character* a = allies[idx];

    if (!a->isAlive()) return false;
    if (a->actedThisTurn) {
        lastMessage = a->name + "은(는) 이번 턴에 이미 행동했다! (다른 유닛 선택 또는 z)";
        return false;
    }

    // ★ [신속 이동] 3칸을 모두 쓰면 더 이상 이동 불가, 제자리 공격만 가능
    if (a->swiftMoveActive && a->freeMoveCells == 0) {
        lastMessage = a->name + "은(는) 신속 이동을 마쳤다. 이제 제자리 공격만 가능하다!";
        return false;
    }

    // 1. 최소 요구 AP 검사 (무료 이동 중이 아니면 AP 1 이상 필요)
    if (a->freeMoveCells == 0 && ap < 1) {
        lastMessage = "AP가 부족하다! (필요: 1)";
        return false;
    }

    int nx = a->x + dx;
    int ny = a->y + dy;

    // 2. 맵 경계선 검사
    if (nx < 0 || nx >= 15 || ny < 0 || ny >= 15) {
        lastMessage = "맵 밖으로는 이동할 수 없다!";
        return false;
    }

    // 3. 지형 및 유닛 충돌 검사
    if (map.getTileAt(nx, ny) != TileType::EMPTY) {
        lastMessage = "길이 막혀 있다!";
        return false;
    }

    // ★ [추가] 감전 시 이동 불가!
    if (a->isShocked) {
        lastMessage = a->name + "은(는) 감전되어 이동할 수 없다!";
        return false;
    }

    // 4. 버프 확인 및 AP 차감
    // 무료 이동(신속 이동) 횟수가 있으면 횟수만 차감하고 AP 소모는 건너뜀
    if (a->freeMoveCells > 0) {
        a->freeMoveCells--;
        if (a->swiftMoveActive) {
            if (a->freeMoveCells > 0)
                lastMessage = a->name + " 남은 이동: " + std::to_string(a->freeMoveCells) + "칸";
            else
                lastMessage = a->name + " 이동 완료! 이제 제자리 공격만 가능하다.";
        }
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

// 분열 대기 슬라임 처리: splitPending 플래그가 서 있는 슬라임 옆에 새 슬라임을 소환
void GameState::processPendingSlimeSplits() {
    int dx[] = { 0, 0, -1, 1 };
    int dy[] = { -1, 1, 0, 0 };

    std::vector<Character*> newSlimes;

    for (auto e : enemies) {
        Slime* slime = dynamic_cast<Slime*>(e);
        if (!slime || !slime->splitPending) continue;
        slime->splitPending = false;

        // 상하좌우 중 비어있는 첫 번째 칸에 새 슬라임 소환
        for (int i = 0; i < 4; ++i) {
            int nx = slime->x + dx[i];
            int ny = slime->y + dy[i];
            if (canEnemyMoveTo(nx, ny)) {
                Slime* baby = new Slime(nx, ny);
                baby->hp      = baby->maxHp / 2; // 5 HP
                baby->hasSplit = true;            // 분열 슬라임은 재분열 불가
                newSlimes.push_back(baby);
                lastMessage = slime->name + "이(가) 분열했다! 옆에 새로운 슬라임이 나타났다!";
                break;
            }
        }
    }

    for (auto s : newSlimes) enemies.push_back(s);
    if (!newSlimes.empty()) syncMapTiles();
}

// 겹친 아군을 멀리 날려보낸다.
// (1) 드래곤(보스)의 7x7 영역과 겹친 아군, (2) 다른 아군과 같은 칸에 겹친 아군 모두 처리.
void GameState::repelAlliesFromBoss() {
    Character* boss = nullptr;
    for (auto e : enemies) {
        if (e->isAlive() && e->cls == CharacterClass::BOSS_DRAGON) { boss = e; break; }
    }

    const int cx = 7, cy = 7; // 드래곤 중심 (보스가 없으면 맵 중앙 기준 방향)

    // 착지 가능한 칸인지 판정 (지형/점유 기반 — 맵 동기화 상태와 무관하게 정확)
    auto validLanding = [&](int tx, int ty, Character* self) -> bool {
        if (tx < 0 || tx >= 15 || ty < 0 || ty >= 15) return false;
        TileType t = map.getTileAt(tx, ty);
        if (t == TileType::MOUNTAIN || t == TileType::RIVER || t == TileType::BASE) return false;
        if (boss && boss->isOccupying(tx, ty)) return false;
        for (auto a : allies) if (a != self && a->isAlive() && a->x == tx && a->y == ty) return false;
        for (auto e : enemies) if (e->isAlive() && e->isOccupying(tx, ty)) return false;
        return true;
    };

    bool flung = false;

    for (size_t i = 0; i < allies.size(); ++i) {
        Character* a = allies[i];
        if (!a->isAlive()) continue;

        // 겹침 판정: 보스 영역 안 OR 앞선 아군과 같은 칸
        bool onBoss = (boss && boss->isOccupying(a->x, a->y));
        bool onAlly = false;
        for (size_t j = 0; j < i; ++j)
            if (allies[j]->isAlive() && allies[j]->x == a->x && allies[j]->y == a->y) { onAlly = true; break; }
        if (!onBoss && !onAlly) continue;

        // 중심에서 바깥으로 향하는 방향 (정중앙/겹침이면 아래로)
        int ddx = (a->x == cx) ? 0 : (a->x > cx ? 1 : -1);
        int ddy = (a->y == cy) ? 0 : (a->y > cy ? 1 : -1);
        if (ddx == 0 && ddy == 0) ddy = 1;

        bool placed = false;

        // 1. 밀려나는 방향 직선상에서 가장 먼 착지 칸 (저 멀리 날아감)
        for (int dist = 14; dist >= 1 && !placed; --dist) {
            int tx = a->x + ddx * dist;
            int ty = a->y + ddy * dist;
            if (validLanding(tx, ty, a)) { a->x = tx; a->y = ty; placed = true; }
        }

        // 2. 실패 시: 중심에서 가장 먼 착지 칸으로 이동
        if (!placed) {
            int bestDist = -1, bx = a->x, by = a->y;
            for (int yy = 0; yy < 15; ++yy)
                for (int xx = 0; xx < 15; ++xx)
                    if (validLanding(xx, yy, a)) {
                        int d = std::abs(xx - cx) + std::abs(yy - cy);
                        if (d > bestDist) { bestDist = d; bx = xx; by = yy; }
                    }
            if (bestDist >= 0) { a->x = bx; a->y = by; placed = true; }
        }

        if (placed) flung = true;
    }

    if (flung) {
        lastMessage = "겹친 영웅들이 저 멀리 날아갔다!";
        syncMapTiles();
    }
}

// 해당 좌표에 보물상자가 있는지
bool GameState::isTreasureAt(int x, int y) const {
    for (auto& t : treasures)
        if (t.first == x && t.second == y) return true;
    return false;
}

// 현재 선택된 아군이 보물상자에 인접해 있는지
bool GameState::selectedAllyNearTreasure() const {
    if (currentAllyIndex < 0 || currentAllyIndex >= (int)allies.size()) return false;
    Character* a = allies[currentAllyIndex];
    if (!a->isAlive()) return false;
    int dx[] = { 0, 0, -1, 1 };
    int dy[] = { -1, 1, 0, 0 };
    for (int i = 0; i < 4; ++i)
        if (isTreasureAt(a->x + dx[i], a->y + dy[i])) return true;
    return false;
}

// 빈 칸 한 곳에 보물상자 1개 배치 (지형·유닛·기지·기존 상자 피해서)
void GameState::spawnTreasure() {
    std::vector<std::pair<int,int>> candidates;
    for (int y = 0; y < 15; ++y) {
        for (int x = 0; x < 15; ++x) {
            if (map.getTileAt(x, y) != TileType::EMPTY) continue; // 산/강/타워 제외
            if (isOccupied(x, y)) continue;                       // 유닛 칸 제외
            if (isTreasureAt(x, y)) continue;                     // 기존 상자 제외
            if (y <= 1 || y >= 13) continue;                      // 적 스폰행/아군 시작·기지 근처 제외
            candidates.push_back({ x, y });
        }
    }
    if (candidates.empty()) return;
    int idx = rand() % candidates.size();
    treasures.push_back(candidates[idx]);
}

// 선택한 아군의 인접 칸에 있는 보물상자를 열어 무작위 버프 적용
bool GameState::tryOpenTreasure(int idx) {
    if (idx < 0 || idx >= (int)allies.size()) return false;
    Character* a = allies[idx];
    if (!a->isAlive()) return false;

    // 기절/감전/매혹 상태에서는 상자와 상호작용 불가
    if (a->isStunned || a->isShocked || a->charmedBy != nullptr) {
        lastMessage = a->name + "은(는) 행동 불능 상태라 보물상자를 열 수 없다!";
        return false;
    }

    // 상하좌우 인접 칸에서 상자 탐색
    int dx[] = { 0, 0, -1, 1 };
    int dy[] = { -1, 1, 0, 0 };
    int found = -1;
    for (int i = 0; i < 4; ++i) {
        int nx = a->x + dx[i], ny = a->y + dy[i];
        for (int t = 0; t < (int)treasures.size(); ++t) {
            if (treasures[t].first == nx && treasures[t].second == ny) { found = t; break; }
        }
        if (found != -1) break;
    }
    if (found == -1) {
        lastMessage = "인접한 보물상자가 없다!";
        return false;
    }

    // 보물상자 개봉에 AP 20 소비
    if (ap < 20) {
        lastMessage = "보물상자를 열려면 AP 20이 필요하다! (현재 AP " + std::to_string(ap) + ")";
        return false;
    }
    ap -= 20;

    // 무작위 버프 적용 (모두 영구 지속)
    int r = rand() % 4;
    switch (r) {
    case 0: {
        // 체력: 최대 체력과 현재 체력을 함께 영구 증가 (기존 한계를 넘어섬: 50 -> 70)
        int up = 20;
        a->maxHp += up;
        a->hp += up;
        lastMessage = a->name + "의 최대 체력이 영구히 +" + std::to_string(up) + " 증가했다! (보물상자)";
        break;
    }
    case 1: {
        // 공격력 영구 증가
        int up = 5;
        a->atk += up;
        lastMessage = a->name + "의 공격력이 영구히 +" + std::to_string(up) + " 증가했다! (보물상자)";
        break;
    }
    case 2: {
        // 최대 AP와 현재 AP를 함께 영구 증가
        int up = 20;
        maxAp += up;
        ap += up;
        lastMessage = a->name + "이(가) 보물상자에서 최대 AP +" + std::to_string(up) + " 을(를) 얻었다!";
        break;
    }
    default: {
        // 방어력 영구 증가 (받는 피해 감소, 지속)
        int up = 3;
        a->def += up;
        lastMessage = a->name + "의 방어력이 영구히 +" + std::to_string(up) + " 증가했다! (보물상자)";
        break;
    }
    }

    treasures.erase(treasures.begin() + found); // 상자 소멸
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
        processPendingSlimeSplits();
        syncMapTiles();
        checkWaveClear();
        return true;
    }

    lastMessage = "공격이 빗나갔거나 잘못된 방향이다.";
    return false;
}

// 1. 스킬 메뉴 열기
void GameState::openSkillMenu() {
    // 유닛이 선택되어 있고, 아직 이번 턴에 행동하지 않은 경우에만 스킬 메뉴를 열 수 있도록 제한
    if (currentAllyIndex != -1 && !allies[currentAllyIndex]->actedThisTurn) {
        // ★ [신속 이동] 사용 중에는 다른 스킬을 쓸 수 없음 (이동 후 공격만 가능)
        if (allies[currentAllyIndex]->swiftMoveActive) {
            lastMessage = "신속 이동 중에는 스킬을 쓸 수 없다. 제자리 공격만 가능!";
            return;
        }
        isSkillMenuOpen = true;
        lastMessage = "";
    }
    else {
        lastMessage = "먼저 행동 가능한 유닛을 선택하세요!";
    }
}

// 2. 스킬 메뉴 닫기
void GameState::closeSkillMenu() {
    isSkillMenuOpen = false;
    lastMessage = "스킬 메뉴를 닫았다.";
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
        lastMessage = "AP가 부족하다! (필요: " + std::to_string(finalApCost) + (caster->isExhausted ? " [탈진 페널티]" : "") + ")";
        return false;
    }

    // 스킬 실행
    if (targetSkill->execute(*caster, *this)) {
        ap -= finalApCost; // ★ 증가된 코스트로 차감
        isSkillMenuOpen = false; // 스킬을 쓰면 스킬창은 무조건 닫힙니다.
        processPendingSlimeSplits();

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
    if (!sel->isAlive() || sel->actedThisTurn) return res;

    // ★ [신속 이동] 3칸을 모두 쓰면 이동 범위 표시 안 함 (공격만 가능)
    if (sel->swiftMoveActive && sel->freeMoveCells == 0) return res;

    // 무료 이동(신속 이동)이 남아있지 않은데 AP가 부족하면 이동 불가
    if (sel->freeMoveCells == 0 && ap < 1) return res;

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


// Calculate attackable positions from current selected ally's position
// Knight/Priest : 4-directional, range 1
// Archer        : 4-directional straight line, range up to atkRange
// Mage          : 4-directional straight line, range exactly atkRange (minimum range = 4)
std::vector<std::pair<int, int>> GameState::getAttackablePositions() const {
    std::vector<std::pair<int, int>> res;
    if (currentAllyIndex == -1 || currentAllyIndex >= (int)allies.size()) return res;

    Character* sel = allies[currentAllyIndex];
    if (!sel->isAlive() || sel->actedThisTurn) return res;

    int dx[] = { 0, 0, -1, 1 };
    int dy[] = { -1, 1, 0, 0 };

    // Mage has a minimum effective range of 4 (skips r=1,2,3 for normal enemies)
    int minRange = (sel->cls == CharacterClass::MAGE) ? 4 : 1;

    // Priest's atkRange stat is 3 but performAttack only reaches 1 adjacent tile
    int effectiveRange = (sel->cls == CharacterClass::PRIEST) ? 1 : sel->atkRange;

    for (int d = 0; d < 4; ++d) {
        for (int r = minRange; r <= effectiveRange; ++r) {
            int nx = sel->x + dx[d] * r;
            int ny = sel->y + dy[d] * r;
            if (nx < 0 || nx >= 15 || ny < 0 || ny >= 15) break;
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
    lastMessage = "적의 턴이 시작된다!";

    // 지난 플레이어 턴 동안 표시된 기절/매혹은 1턴 행동 불가를 마쳤으므로 여기서 해제
    // (플래그를 플레이어 턴 내내 유지해 패널/맵에 상태를 표시하기 위함)
    for (auto a : allies) {
        if (a->isAlive()) { a->isStunned = false; a->charmedBy = nullptr; }
    }

    enemyMgr.processAITurn(enemies, *this);

    // ★ 턴 종료 시 상태 리셋 및 기절/유혹 처리
    for (auto a : allies) {
        if (!a->isAlive()) continue;

        // 일회성 버프 초기화 (보호막은 적 턴이 끝나면 소멸)
        a->freeMoveCells = 0;
        a->multiHitCount = 1;
        a->swiftMoveActive = false;
        a->shieldHp = 0;
        if (a->isShocked) a->isShocked = false;

        if (a->charmedBy) {
            // 유혹: 악마를 향해 강제로 한 칸 이동 후, 이번 턴 행동 불가
            if (a->charmedBy->isAlive()) {
                int tx = a->charmedBy->x;
                int ty = a->charmedBy->y;
                bool moved = false;
                if (a->x < tx && canEnemyMoveTo(a->x + 1, a->y)) { a->x++; moved = true; }
                if (!moved && a->x > tx && canEnemyMoveTo(a->x - 1, a->y)) { a->x--; moved = true; }
                if (!moved && a->y < ty && canEnemyMoveTo(a->x, a->y + 1)) { a->y++; moved = true; }
                if (!moved && a->y > ty && canEnemyMoveTo(a->x, a->y - 1)) { a->y--; moved = true; }
            }
            a->actedThisTurn = true;  // 1턴 행동 불가 (charmedBy는 표시용으로 유지)
            a->isStunned = false;     // 매혹이 기절보다 우선
        }
        else if (a->isStunned) {
            // 기절: 이번 턴 행동 불가 (isStunned는 표시용으로 유지)
            a->actedThisTurn = true;
        }
        else {
            a->actedThisTurn = false; // 정상: 새 턴에 행동 가능
        }
    }
    ap = std::min(maxAp, ap + 15);
    phase = Phase::PLAYER_TURN;
    repelAlliesFromBoss(); // 적 턴 이후 보스와 겹친 아군이 있으면 날려보냄
    syncMapTiles();
}

void GameState::checkWaveClear() {
    bool anyEnemy = false;
    for (auto e : enemies) if (e->isAlive()) { anyEnemy = true; break; }

    if (!anyEnemy) {
        // ★ 4웨이브(보스) 클리어 시 승리 조건
        if (wave == 4) {
            phase = Phase::GAME_OVER;
            lastMessage = "승리!! 드래곤이 쓰러졌다!";
            return;
        }

        for (auto e : enemies) delete e;
        enemies.clear();
        // 삭제된 적을 가리키던 아군의 매혹/기절 상태 정리 (dangling 방지)
        for (auto a : allies) { a->charmedBy = nullptr; a->isStunned = false; }
        wave++;
        lastMessage = std::to_string(wave - 1) + " 웨이브 클리어!";

        // 새 웨이브마다 지형 랜덤 재생성 (아군 위치 + 기지 타일 보호)
        std::vector<std::pair<int,int>> occupied;
        for (auto a : allies) if (a->isAlive()) occupied.push_back({a->x, a->y});
        occupied.push_back({7, 14}); // 기지 타일
        map.randomizeTerrain(wave, occupied);

        enemyMgr.spawnWave(wave, enemies);
        syncMapTiles(); // 보스/아군 위치를 맵에 먼저 반영 (빈 칸 판정 정확도 확보)

        // 보스 스폰 시 7x7 영역과 겹친 아군을 멀리 날려보냄
        repelAlliesFromBoss();

        spawnTreasure(); // 새 웨이브마다 보물상자 배치
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
    default: return new Enemy("알수없음", x, y, 10, 1); // 예비용
    }
}