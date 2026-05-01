#include "GameState.h"
#include <algorithm>
#include <cmath>
#include <sstream>

GameState::GameState(GameMap& m)
    : map(m), phase(Phase::PLAYER_TURN), currentAllyIndex(-1),
    towerHp(50), towerMaxHp(50), towerDp(50),
    ap(50), maxAp(50), wave(1)
{
    allies.emplace_back("Knight", CharacterClass::KNIGHT, 11, 12, 50, 15, 1, 1, true);
    allies.emplace_back("Archer", CharacterClass::ARCHER, 10, 12, 35, 12, 2, 5, true);
    allies.emplace_back("Priest", CharacterClass::PRIEST, 12, 12, 30, 5, 3, 1, true);
    allies.emplace_back("Wizard", CharacterClass::MAGE, 9, 12, 25, 20, 1, 7, true);

    spawnEnemies();
    syncMapTiles();
    lastMessage = "Welcome! AP: 50/50. Select unit (1-4).";
}

void GameState::spawnEnemies() {
    enemies.clear();
    for (int i = 0; i < 6; ++i) {
        enemies.emplace_back("Orc" + std::to_string(i + 1),
            CharacterClass::ENEMY_ORC, i * 2, 0, 45 + (wave * 5), 4 + wave, 1, 1, false);
    }
}

void GameState::setCurrentAllyIndex(int idx) {
    if (idx >= 0 && idx < (int)allies.size()) {
        if (!allies[idx].isAlive()) lastMessage = allies[idx].name + " is dead.";
        else if (allies[idx].actedThisTurn) lastMessage = allies[idx].name + " already acted.";
        else { currentAllyIndex = idx; lastMessage = allies[idx].name + " selected."; }
    }
}

int GameState::manhattanDist(int x1, int y1, int x2, int y2) const {
    return std::abs(x1 - x2) + std::abs(y1 - y2);
}

bool GameState::isOccupied(int x, int y) const {
    for (auto& a : allies) if (a.isAlive() && a.x == x && a.y == y) return true;
    for (auto& e : enemies) if (e.isAlive() && e.x == x && e.y == y) return true;
    return false;
}

void GameState::syncMapTiles() {
    for (int y = 0; y < 15; ++y) {
        for (int x = 0; x < 15; ++x) {
            TileType t = map.getTileAt(x, y);
            if (t == TileType::HERO || t == TileType::ENEMY) map.setTile(x, y, TileType::EMPTY);
        }
    }
    for (auto& a : allies) if (a.isAlive()) map.setTile(a.x, a.y, TileType::HERO);
    for (auto& e : enemies) if (e.isAlive()) map.setTile(e.x, e.y, TileType::ENEMY);
}

bool GameState::tryMoveAlly(int idx, int dx, int dy) {
    if (idx < 0 || idx >= (int)allies.size()) return false;
    Character& c = allies[idx];
    if (!c.isAlive() || c.actedThisTurn) return false;
    if (ap < 1) { lastMessage = "Out of AP!"; return false; }

    int nx = c.x + dx, ny = c.y + dy;
    if (nx < 0 || nx >= 15 || ny < 0 || ny >= 15) return false;
    if (map.getTileAt(nx, ny) != TileType::EMPTY || isOccupied(nx, ny)) return false;

    if (c.cls == CharacterClass::PRIEST) {
        if (dx != 0 && dy != 0 || std::abs(dx + dy) > 3) return false;
    }
    else {
        if (std::max(std::abs(dx), std::abs(dy)) > c.moveRange) return false;
    }

    c.x = nx; c.y = ny;
    syncMapTiles();
    ap -= 1;
    lastMessage = c.name + " moved. AP: " + std::to_string(ap);
    return true;
}

// 수정된 원거리 공격 탐색 로직
bool GameState::tryAttackAlly(int idx, char dir) {
    if (idx < 0 || idx >= (int)allies.size()) return false;
    Character& c = allies[idx];
    if (!c.isAlive() || c.actedThisTurn) return false;

    // 1. 공격 방향 설정
    int dx = 0, dy = 0;
    if (dir == 'w') dy = -1;
    else if (dir == 's') dy = 1;
    else if (dir == 'a') dx = -1;
    else if (dir == 'd') dx = 1;
    else {
        lastMessage = "Invalid attack direction! Use w, a, s, d.";
        return false;
    }

    // 2. 직업별 사거리 정의
    int minR = 1, maxR = 1;
    if (c.cls == CharacterClass::ARCHER) { minR = 1; maxR = 5; }
    else if (c.cls == CharacterClass::MAGE) { minR = 4; maxR = 7; }
    // 기사(KNIGHT)와 사제(PRIEST)는 기본값 1을 사용합니다.

    // 3. 타겟 탐색 (해당 방향으로 사거리 내 가장 가까운 적 찾기)
    int targetX = -1, targetY = -1;
    bool found = false;

    for (int r = minR; r <= maxR; ++r) {
        int cx = c.x + (dx * r);
        int cy = c.y + (dy * r);

        if (cx < 0 || cx >= 15 || cy < 0 || cy >= 15) break;

        for (auto& e : enemies) {
            if (e.isAlive() && e.x == cx && e.y == cy) {
                targetX = cx;
                targetY = cy;
                found = true;
                break;
            }
        }
        if (found) break;
    }

    // 4. 사제(PRIEST) 예외 처리: 적이 없으면 해당 방향 아군 힐 시도
    if (!found && c.cls == CharacterClass::PRIEST) {
        if (tryHealPriest(idx, c.x + dx, c.y + dy)) return true;
    }

    if (!found) {
        lastMessage = "No target in range for " + c.name + ".";
        return false;
    }

    // 5. 데미지 계산 및 특성 적용[cite: 2]
    bool hit = false;
    int baseAtk = c.atk;
    std::ostringstream oss;

    // --- 궁수 특성: 거리가 4칸 이상이면 데미지 1.5배[cite: 2] ---
    if (c.cls == CharacterClass::ARCHER) {
        int dist = manhattanDist(c.x, c.y, targetX, targetY);
        if (dist >= 4) {
            baseAtk = static_cast<int>(baseAtk * 1.5);
            oss << "[Long Shot!] ";
        }
        // 단일 대상 공격
        for (auto& e : enemies) {
            if (e.isAlive() && e.x == targetX && e.y == targetY) {
                e.hp -= baseAtk;
                hit = true;
                oss << c.name << " hit " << e.name << " for " << baseAtk << " HP.";
                break;
            }
        }
    }
    // --- 마법사 특성: 타격점 중심 3x3 범위 스플래시 (중심 100%, 주변 50%)[cite: 2] ---
    else if (c.cls == CharacterClass::MAGE) {
        oss << "Wizard's Fireball exploded! ";
        for (auto& e : enemies) {
            if (!e.isAlive()) continue;

            int diffX = std::abs(e.x - targetX);
            int diffY = std::abs(e.y - targetY);

            // 3x3 범위 체크 (x, y 차이가 각각 1 이하)[cite: 2]
            if (diffX <= 1 && diffY <= 1) {
                if (diffX == 0 && diffY == 0) {
                    e.hp -= baseAtk; // 직격 대미지
                }
                else {
                    e.hp -= static_cast<int>(baseAtk * 0.5); // 주변 스플래시 대미지[cite: 2]
                }
                hit = true;
            }
        }
    }
    // --- 기본 공격 (기사 등) ---
    else {
        for (auto& e : enemies) {
            if (e.isAlive() && e.x == targetX && e.y == targetY) {
                e.hp -= baseAtk;
                hit = true;
                oss << c.name << " attacked " << e.name << ".";
                break;
            }
        }
    }

    // 6. 공격 성공 시 후처리
    if (hit) {
        lastMessage = oss.str();
        c.actedThisTurn = true;
        currentAllyIndex = -1;

        checkWaveClear(); // 적 전멸 여부 확인 및 AP 회복[cite: 2]

        if (phase != Phase::WAVE_CLEAR) {
            endAllyAction(); // 다음 아군 선택 혹은 적 턴으로 전환[cite: 2]
        }
        return true;
    }

    return false;
}

bool GameState::tryHealPriest(int idx, int tx, int ty) {
    for (auto& a : allies) {
        if (a.isAlive() && a.x == tx && a.y == ty) {
            a.hp = std::min(a.hp + 10, a.maxHp);
            lastMessage = "Healed " + a.name;
            allies[idx].actedThisTurn = true; currentAllyIndex = -1;
            endAllyAction(); return true;
        }
    }
    return false;
}

void GameState::checkWaveClear() {
    bool allDead = true;
    for (const auto& e : enemies) if (e.isAlive()) { allDead = false; break; }
    if (allDead) {
        wave++; ap = maxAp; spawnEnemies(); syncMapTiles();
        lastMessage = "WAVE CLEAR! AP Refilled.";
    }
}

void GameState::endAllyAction() {
    int next = nextActiveAllyIndex();
    if (next == -1) { phase = Phase::ENEMY_TURN; runEnemyTurn(); }
    else currentAllyIndex = next;
}

int GameState::nextActiveAllyIndex() const {
    for (int i = 0; i < (int)allies.size(); ++i) {
        if (allies[i].isAlive() && !allies[i].actedThisTurn) return i;
    }
    return -1;
}

void GameState::runEnemyTurn() {
    for (auto& e : enemies) {
        if (!e.isAlive()) continue;
        int tX = 7, tY = 14;
        bool acted = false;

        for (auto& a : allies) {
            if (a.isAlive() && manhattanDist(e.x, e.y, a.x, a.y) == 1) {
                a.hp -= e.atk; acted = true; break;
            }
        }
        if (!acted && manhattanDist(e.x, e.y, tX, tY) <= 1) {
            towerHp -= e.atk; acted = true;
        }
        if (!acted) {
            int dx[] = { 0,0,-1,1 }, dy[] = { -1,1,0,0 }, bX = e.x, bY = e.y, mD = manhattanDist(e.x, e.y, tX, tY);
            for (int i = 0; i < 4; ++i) {
                int nx = e.x + dx[i], ny = e.y + dy[i];
                if (nx >= 0 && nx < 15 && ny >= 0 && ny < 15 && map.getTileAt(nx, ny) == TileType::EMPTY && !isOccupied(nx, ny)) {
                    int d = manhattanDist(nx, ny, tX, tY);
                    if (d < mD) { mD = d; bX = nx; bY = ny; }
                }
            }
            e.x = bX; e.y = bY;
        }
    }
    phase = Phase::PLAYER_TURN;
    for (auto& a : allies) a.actedThisTurn = false;
    currentAllyIndex = -1; syncMapTiles();
    if (towerHp <= 0) towerHp = 0;
}