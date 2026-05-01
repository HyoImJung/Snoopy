#include "GameState.h"
#include <algorithm>
#include <cstdlib>
#include <sstream>

GameState::GameState(GameMap& m)
    : map(m), phase(Phase::PLAYER_TURN), currentAllyIndex(0),
      towerHp(50), towerMaxHp(50), towerDp(50), ap(10), maxAp(10), wave(1)
{
    allies.emplace_back("Paladin", CharacterClass::KNIGHT, 11, 12,  50, 15, 3, 1, true);

    enemies.emplace_back("Orc1",  CharacterClass::ENEMY_ORC, 0,  0, 45, 4, 2, 1, false);
    enemies.emplace_back("Orc2",  CharacterClass::ENEMY_ORC, 2,  0, 45, 4, 2, 1, false);
    enemies.emplace_back("Orc3",  CharacterClass::ENEMY_ORC, 4,  0, 45, 4, 2, 1, false);
    enemies.emplace_back("Orc4",  CharacterClass::ENEMY_ORC, 0,  2, 45, 4, 2, 1, false);
    enemies.emplace_back("Orc5",  CharacterClass::ENEMY_ORC, 2,  2, 45, 4, 2, 1, false);
    enemies.emplace_back("Orc6",  CharacterClass::ENEMY_ORC, 4,  2, 45, 4, 2, 1, false);
    enemies.emplace_back("Orc7",  CharacterClass::ENEMY_ORC, 0,  4, 45, 4, 2, 1, false);
    enemies.emplace_back("Orc8",  CharacterClass::ENEMY_ORC, 2,  4, 45, 4, 2, 1, false);
    enemies.emplace_back("Orc9",  CharacterClass::ENEMY_ORC, 0,  6, 45, 4, 2, 1, false);
    enemies.emplace_back("Orc10", CharacterClass::ENEMY_ORC, 2,  6, 45, 4, 2, 1, false);
    enemies.emplace_back("Orc11", CharacterClass::ENEMY_ORC, 0,  9, 45, 4, 2, 1, false);
    enemies.emplace_back("Orc12", CharacterClass::ENEMY_ORC, 2,  9, 45, 4, 2, 1, false);

    syncMapTiles();
}

int GameState::manhattanDist(int x1, int y1, int x2, int y2) const {
    return std::abs(x1 - x2) + std::abs(y1 - y2);
}

bool GameState::isOccupied(int x, int y) const {
    for (auto& a : allies)
        if (a.isAlive() && a.x == x && a.y == y) return true;
    for (auto& e : enemies)
        if (e.isAlive() && e.x == x && e.y == y) return true;
    return false;
}

// 아군 위치만 체크 (적 이동 시 사용 — 적끼리 겹침 허용)
bool GameState::isOccupiedByAlly(int x, int y) const {
    for (auto& a : allies)
        if (a.isAlive() && a.x == x && a.y == y) return true;
    return false;
}

void GameState::syncMapTiles() {
    for (int y = 0; y < 15; ++y)
        for (int x = 0; x < 15; ++x) {
            TileType t = map.getTileAt(x, y);
            if (t == TileType::HERO || t == TileType::ENEMY)
                map.setTile(x, y, TileType::EMPTY);
        }
    for (auto& a : allies)
        if (a.isAlive()) map.setTile(a.x, a.y, TileType::HERO);
    for (auto& e : enemies)
        if (e.isAlive()) map.setTile(e.x, e.y, TileType::ENEMY);
}

bool GameState::tryMoveAlly(int idx, int dx, int dy) {
    if (idx < 0 || idx >= (int)allies.size()) return false;
    Character& c = allies[idx];
    if (!c.isAlive() || c.actedThisTurn) return false;

    int nx = c.x + dx;
    int ny = c.y + dy;

    if (nx < 0 || nx >= 15 || ny < 0 || ny >= 15) {
        lastMessage = "Cannot move outside the map.";
        return false;
    }
    TileType t = map.getTileAt(nx, ny);
    if (t == TileType::MOUNTAIN || t == TileType::BASE) {
        lastMessage = "Cannot move to that tile.";
        return false;
    }
    if (std::abs(dx) + std::abs(dy) > 1) {
        lastMessage = "Can only move one tile at a time.";
        return false;
    }
    if (isOccupied(nx, ny)) {
        lastMessage = "Tile is occupied.";
        return false;
    }

    map.setTile(c.x, c.y, TileType::EMPTY);
    c.x = nx; c.y = ny;
    map.setTile(c.x, c.y, TileType::HERO);
    ap -= 5;
    lastMessage = std::string(c.name) + " moved. AP: " + std::to_string(ap) + "/" + std::to_string(maxAp);
    // AP 소진 시 자동으로 턴 종료
    if (ap <= 0) {
        c.actedThisTurn = true;
        endAllyAction();
    }
    return true;
}

bool GameState::tryAttackAlly(int idx, int tx, int ty) {
    if (idx < 0 || idx >= (int)allies.size()) return false;
    Character& c = allies[idx];
    if (!c.isAlive()) return false;
    if (c.cls == CharacterClass::PRIEST) {
        lastMessage = "Priest cannot attack. Use 'h' to heal.";
        return false;
    }

    int dist = manhattanDist(c.x, c.y, tx, ty);
    if (dist > c.atkRange) {
        lastMessage = "Out of attack range. (Range: " + std::to_string(c.atkRange) + ")";
        return false;
    }

    for (auto& e : enemies) {
        if (e.isAlive() && e.x == tx && e.y == ty) {
            e.hp -= c.atk;
            std::ostringstream oss;
            oss << c.name << " -> " << e.name << ": " << c.atk << " damage.";
            if (!e.isAlive()) {
                map.setTile(e.x, e.y, TileType::EMPTY);
                oss << " [Defeated!]";
            }
            lastMessage = oss.str();
            c.actedThisTurn = true;
            endAllyAction();
            return true;
        }
    }
    lastMessage = "No enemy at that position.";
    return false;
}

bool GameState::tryHealPriest(int idx, int tx, int ty) {
    if (idx < 0 || idx >= (int)allies.size()) return false;
    Character& c = allies[idx];
    if (c.cls != CharacterClass::PRIEST) {
        lastMessage = "Only Priest can heal.";
        return false;
    }
    int dist = manhattanDist(c.x, c.y, tx, ty);
    if (dist > c.atkRange) {
        lastMessage = "Out of heal range.";
        return false;
    }
    for (auto& a : allies) {
        if (a.isAlive() && a.x == tx && a.y == ty) {
            int heal = 4;
            a.hp = std::min(a.hp + heal, a.maxHp);
            lastMessage = "Priest healed " + std::string(a.name) + " for " + std::to_string(heal) + ".";
            c.actedThisTurn = true;
            endAllyAction();
            return true;
        }
    }
    lastMessage = "No ally at that position.";
    return false;
}

void GameState::endAllyAction() {
    int next = nextActiveAllyIndex();
    if (next == -1) {
        phase = Phase::ENEMY_TURN;
        runEnemyTurn();
    } else {
        currentAllyIndex = next;
    }
}

int GameState::nextActiveAllyIndex() const {
    for (int i = 0; i < (int)allies.size(); ++i)
        if (allies[i].isAlive() && !allies[i].actedThisTurn)
            return i;
    return -1;
}

void GameState::runEnemyTurn() {
    std::ostringstream log;
    int towerX = 7, towerY = 14;

    for (auto& e : enemies) {
        if (!e.isAlive()) continue;

        // ── 1. 인접 아군 공격 ──────────────────────────────────────
        bool attacked = false;
        for (auto& a : allies) {
            if (!a.isAlive()) continue;
            if (manhattanDist(e.x, e.y, a.x, a.y) <= e.atkRange) {
                a.hp -= e.atk;
                log << e.name << " atk " << a.name << "(" << e.atk << "). ";
                if (!a.isAlive()) {
                    map.setTile(a.x, a.y, TileType::EMPTY);
                    log << "[defeated] ";
                }
                attacked = true;
                break;
            }
        }
        if (attacked) continue;

        // ── 2. 타워 인접 시 공격 (겹쳐서 때리기 허용) ──────────────
        if (manhattanDist(e.x, e.y, towerX, towerY) <= 1) {
            towerHp -= e.atk;
            log << e.name << " atk tower(" << e.atk << "). ";
            if (towerHp <= 0) {
                towerHp = 0;
                phase = Phase::GAME_OVER;
                lastMessage = "Tower destroyed! GAME OVER";
                return;
            }
            continue;
        }

        // ── 3. 팔라딘 사정거리+1 이내면 도망 ──────────────────────
        int dirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
        bool fleeing = false;
        if (!allies.empty() && allies[0].isAlive()) {
            const auto& p = allies[0];
            int threat = p.atkRange * 2;
            if (manhattanDist(e.x, e.y, p.x, p.y) <= threat) {
                // 팔라딘에서 가장 멀어지는 칸으로 이동
                int bestX = e.x, bestY = e.y;
                int bestDist = manhattanDist(e.x, e.y, p.x, p.y);
                for (auto& d : dirs) {
                    int nx = e.x + d[0];
                    int ny = e.y + d[1];
                    if (nx < 0 || nx >= 15 || ny < 0 || ny >= 15) continue;
                    TileType t = map.getTileAt(nx, ny);
                    if (t == TileType::MOUNTAIN || t == TileType::BASE) continue;
                    if (isOccupied(nx, ny)) continue;  // 도망 중엔 적끼리도 겹치지 않음
                    int dist = manhattanDist(nx, ny, p.x, p.y);
                    if (dist > bestDist) {
                        bestDist = dist;
                        bestX = nx; bestY = ny;
                    }
                }
                if (bestX != e.x || bestY != e.y) {
                    map.setTile(e.x, e.y, TileType::EMPTY);
                    e.x = bestX; e.y = bestY;
                    map.setTile(e.x, e.y, TileType::ENEMY);
                    fleeing = true;
                }
            }
        }
        if (fleeing) continue;

        // ── 4. 타워를 향해 이동 (타워 인접 칸만 겹침 허용) ─────────
        int bestX = e.x, bestY = e.y;
        int bestDist = manhattanDist(e.x, e.y, towerX, towerY);
        for (auto& d : dirs) {
            int nx = e.x + d[0];
            int ny = e.y + d[1];
            if (nx < 0 || nx >= 15 || ny < 0 || ny >= 15) continue;
            TileType t = map.getTileAt(nx, ny);
            if (t == TileType::MOUNTAIN || t == TileType::BASE) continue;
            // 타워 인접 칸(B 바로 앞)이면 적끼리 겹침 허용, 그 외엔 불허
            bool nextToTower = (manhattanDist(nx, ny, towerX, towerY) <= 1);
            if (nextToTower) {
                if (isOccupiedByAlly(nx, ny)) continue;
            } else {
                if (isOccupied(nx, ny)) continue;
            }
            int dist = manhattanDist(nx, ny, towerX, towerY);
            if (dist < bestDist) {
                bestDist = dist;
                bestX = nx; bestY = ny;
            }
        }
        if (bestX != e.x || bestY != e.y) {
            map.setTile(e.x, e.y, TileType::EMPTY);
            e.x = bestX; e.y = bestY;
            map.setTile(e.x, e.y, TileType::ENEMY);
        }
    }

    bool anyAlive = false;
    for (auto& e : enemies) if (e.isAlive()) { anyAlive = true; break; }
    if (!anyAlive) {
        phase = Phase::WAVE_CLEAR;
        lastMessage = "Wave 1 Clear! All enemies defeated!";
        return;
    }

    bool allyAlive = false;
    for (auto& a : allies) if (a.isAlive()) { allyAlive = true; break; }
    if (!allyAlive) {
        phase = Phase::GAME_OVER;
        lastMessage = "All Heroes defeated! GAME OVER";
        return;
    }

    phase = Phase::PLAYER_TURN;
    for (auto& a : allies) a.actedThisTurn = false;
    ap = maxAp;
    currentAllyIndex = nextActiveAllyIndex();

    if (log.str().empty()) lastMessage = "Enemy turn ended.";
    else lastMessage = log.str();
}
