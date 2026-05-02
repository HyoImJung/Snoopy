#include "GameState.h"
#include <algorithm>
#include <cmath>
#include <sstream>

GameState::GameState(GameMap& m)
    : map(m), phase(Phase::PLAYER_TURN), currentAllyIndex(-1),
      towerHp(50), towerMaxHp(50), towerDp(50),
      ap(50), maxAp(50), wave(1)
{
    allies.emplace_back("Knight", CharacterClass::KNIGHT,  9, 12, 50, 15, 1, 1, true);
    allies.emplace_back("Archer", CharacterClass::ARCHER, 10, 12, 35, 12, 3, 5, true);
    allies.emplace_back("Priest", CharacterClass::PRIEST, 11, 12, 30,  5, 2, 1, true);
    allies.emplace_back("Wizard", CharacterClass::MAGE,   12, 12, 25, 20, 4, 7, true);

    spawnEnemies();
    syncMapTiles();
    lastMessage = "Welcome! AP: 50/50. Select unit (1-4).";
}

void GameState::spawnEnemies() {
    enemies.clear();
    for (int i = 0; i < 6; ++i) {
        enemies.emplace_back("Orc" + std::to_string(i + 1),
            CharacterClass::ENEMY_ORC, i * 2, 0,
            45 + (wave * 5), 4 + wave, 1, 1, false);
    }
}

void GameState::setCurrentAllyIndex(int idx) {
    if (idx >= 0 && idx < (int)allies.size()) {
        if (!allies[idx].isAlive())
            lastMessage = allies[idx].name + " is dead.";
        else if (allies[idx].actedThisTurn)
            lastMessage = allies[idx].name + " already acted.";
        else {
            currentAllyIndex = idx;
            lastMessage = allies[idx].name + " selected.";
        }
    }
}

int GameState::manhattanDist(int x1, int y1, int x2, int y2) const {
    return std::abs(x1 - x2) + std::abs(y1 - y2);
}

bool GameState::isOccupied(int x, int y) const {
    for (int i = 0; i < (int)allies.size(); ++i)
        if (allies[i].isAlive() && allies[i].x == x && allies[i].y == y) return true;
    for (int i = 0; i < (int)enemies.size(); ++i)
        if (enemies[i].isAlive() && enemies[i].x == x && enemies[i].y == y) return true;
    return false;
}

void GameState::syncMapTiles() {
    for (int y = 0; y < 15; ++y)
        for (int x = 0; x < 15; ++x) {
            TileType t = map.getTileAt(x, y);
            if (t == TileType::HERO || t == TileType::ENEMY)
                map.setTile(x, y, TileType::EMPTY);
        }
    for (int i = 0; i < (int)allies.size(); ++i)
        if (allies[i].isAlive()) map.setTile(allies[i].x, allies[i].y, TileType::HERO);
    for (int i = 0; i < (int)enemies.size(); ++i)
        if (enemies[i].isAlive()) map.setTile(enemies[i].x, enemies[i].y, TileType::ENEMY);
}

bool GameState::tryMoveAlly(int idx, int dx, int dy) {
    if (idx < 0 || idx >= (int)allies.size()) return false;
    Character& c = allies[idx];
    if (!c.isAlive() || c.actedThisTurn) return false;
    if (ap < 1) { lastMessage = "Out of AP!"; return false; }

    if (c.cls == CharacterClass::PRIEST)
        if (dx != 0 && dy != 0) { lastMessage = "Priest cannot move diagonally!"; return false; }

    if (std::abs(dx) > 1 || std::abs(dy) > 1) return false;

    int nx = c.x + dx, ny = c.y + dy;
    if (nx < 0 || nx >= 15 || ny < 0 || ny >= 15) return false;
    if (map.getTileAt(nx, ny) != TileType::EMPTY || isOccupied(nx, ny)) return false;

    c.x = nx; c.y = ny;
    syncMapTiles();
    ap -= 1;
    lastMessage = c.name + " moved. AP: " + std::to_string(ap);
    return true;
}

bool GameState::tryAttackAlly(int idx, char dir) {
    if (idx < 0 || idx >= (int)allies.size()) return false;
    Character& c = allies[idx];
    if (!c.isAlive() || c.actedThisTurn) return false;

    int dx = 0, dy = 0;
    if      (dir == 'w') dy = -1;
    else if (dir == 's') dy =  1;
    else if (dir == 'a') dx = -1;
    else if (dir == 'd') dx =  1;
    else { lastMessage = "Invalid attack direction! Use w, a, s, d."; return false; }

    int minR = 1, maxR = 1;
    if      (c.cls == CharacterClass::ARCHER) { minR = 1; maxR = 5; }
    else if (c.cls == CharacterClass::MAGE)   { minR = 4; maxR = 7; }

    int targetX = -1, targetY = -1;
    bool found = false;
    for (int r = minR; r <= maxR; ++r) {
        int cx = c.x + dx * r, cy = c.y + dy * r;
        if (cx < 0 || cx >= 15 || cy < 0 || cy >= 15) break;
        for (int i = 0; i < (int)enemies.size(); ++i)
            if (enemies[i].isAlive() && enemies[i].x == cx && enemies[i].y == cy) {
                targetX = cx; targetY = cy; found = true; break;
            }
        if (found) break;
    }

    if (!found && c.cls == CharacterClass::PRIEST)
        if (tryHealPriest(idx, c.x + dx, c.y + dy)) return true;

    if (!found) { lastMessage = "No target in range for " + c.name + "."; return false; }

    bool hit = false;
    int baseAtk = c.atk;
    std::ostringstream oss;

    if (c.cls == CharacterClass::ARCHER) {
        int dist = manhattanDist(c.x, c.y, targetX, targetY);
        if (dist >= 4) { baseAtk = static_cast<int>(baseAtk * 1.5); oss << "[Long Shot!] "; }
        for (int i = 0; i < (int)enemies.size(); ++i)
            if (enemies[i].isAlive() && enemies[i].x == targetX && enemies[i].y == targetY) {
                enemies[i].hp -= baseAtk; hit = true;
                oss << c.name << " hit " << enemies[i].name << " for " << baseAtk << " HP.";
                break;
            }
    }
    else if (c.cls == CharacterClass::MAGE) {
        oss << "Wizard's Fireball exploded! ";
        for (int i = 0; i < (int)enemies.size(); ++i) {
            if (!enemies[i].isAlive()) continue;
            int diffX = std::abs(enemies[i].x - targetX);
            int diffY = std::abs(enemies[i].y - targetY);
            if (diffX <= 1 && diffY <= 1) {
                enemies[i].hp -= (diffX == 0 && diffY == 0)
                    ? baseAtk : static_cast<int>(baseAtk * 0.5);
                hit = true;
            }
        }
    }
    else {
        for (int i = 0; i < (int)enemies.size(); ++i)
            if (enemies[i].isAlive() && enemies[i].x == targetX && enemies[i].y == targetY) {
                enemies[i].hp -= baseAtk; hit = true;
                oss << c.name << " attacked " << enemies[i].name << ".";
                break;
            }
    }

    if (hit) {
        lastMessage = oss.str();
        c.actedThisTurn = true;
        currentAllyIndex = -1;
        checkWaveClear();
        if (phase != Phase::WAVE_CLEAR) endAllyAction();
        return true;
    }
    return false;
}

bool GameState::tryHealPriest(int idx, int tx, int ty) {
    for (int i = 0; i < (int)allies.size(); ++i) {
        if (allies[i].isAlive() && allies[i].x == tx && allies[i].y == ty) {
            allies[i].hp = std::min(allies[i].hp + 10, allies[i].maxHp);
            lastMessage = "Healed " + allies[i].name;
            allies[idx].actedThisTurn = true;
            currentAllyIndex = -1;
            endAllyAction();
            return true;
        }
    }
    return false;
}

void GameState::checkWaveClear() {
    bool allDead = true;
    for (int i = 0; i < (int)enemies.size(); ++i)
        if (enemies[i].isAlive()) { allDead = false; break; }
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
    for (int i = 0; i < (int)allies.size(); ++i)
        if (allies[i].isAlive() && !allies[i].actedThisTurn) return i;
    return -1;
}

std::vector<std::pair<int, int>> GameState::getMovablePositions() const {
    std::vector<std::pair<int, int>> result;
    int idx = currentAllyIndex;
    if (idx < 0 || idx >= (int)allies.size()) return result;
    const Character& c = allies[idx];
    if (!c.isAlive() || c.actedThisTurn) return result;
    if (ap < 1) return result;

    bool diagonal = (c.cls != CharacterClass::PRIEST);
    int range = c.moveRange;

    int dirX[4] = { 0,  0, -1, 1 };
    int dirY[4] = { -1, 1,  0, 0 };

    if (diagonal) {
        for (int ddx = -1; ddx <= 1; ++ddx) {
            for (int ddy = -1; ddy <= 1; ++ddy) {
                if (ddx == 0 && ddy == 0) continue;
                for (int r = 1; r <= range; ++r) {
                    int nx = c.x + ddx * r, ny = c.y + ddy * r;
                    if (nx < 0 || nx >= 15 || ny < 0 || ny >= 15) break;
                    if (map.getTileAt(nx, ny) == TileType::EMPTY && !isOccupied(nx, ny))
                        result.push_back(std::make_pair(nx, ny));
                    else break;
                }
            }
        }
    }
    else {
        for (int d = 0; d < 4; ++d) {
            for (int r = 1; r <= range; ++r) {
                int nx = c.x + dirX[d] * r, ny = c.y + dirY[d] * r;
                if (nx < 0 || nx >= 15 || ny < 0 || ny >= 15) break;
                if (map.getTileAt(nx, ny) == TileType::EMPTY && !isOccupied(nx, ny))
                    result.push_back(std::make_pair(nx, ny));
                else break;
            }
        }
    }
    return result;
}

void GameState::runEnemyTurn() {
    for (int i = 0; i < (int)enemies.size(); ++i) {
        Character& e = enemies[i];
        if (!e.isAlive()) continue;

        int tX = 7, tY = 14;
        bool acted = false;

        for (int j = 0; j < (int)allies.size(); ++j)
            if (allies[j].isAlive() && manhattanDist(e.x, e.y, allies[j].x, allies[j].y) == 1) {
                allies[j].hp -= e.atk; acted = true; break;
            }

        if (!acted && manhattanDist(e.x, e.y, tX, tY) <= 1) {
            towerHp -= e.atk; acted = true;
        }

        if (!acted) {
            int stepX[4] = { 0,  0, -1, 1 };
            int stepY[4] = { -1, 1,  0, 0 };
            int bX = e.x, bY = e.y;
            int mD = manhattanDist(e.x, e.y, tX, tY);
            for (int k = 0; k < 4; ++k) {
                int nx = e.x + stepX[k], ny = e.y + stepY[k];
                if (nx >= 0 && nx < 15 && ny >= 0 && ny < 15
                    && map.getTileAt(nx, ny) == TileType::EMPTY
                    && !isOccupied(nx, ny)) {
                    int dist = manhattanDist(nx, ny, tX, tY);
                    if (dist < mD) { mD = dist; bX = nx; bY = ny; }
                }
            }
            e.x = bX; e.y = bY;
        }
    }

    phase = Phase::PLAYER_TURN;
    for (int i = 0; i < (int)allies.size(); ++i) allies[i].actedThisTurn = false;
    currentAllyIndex = -1;
    syncMapTiles();
    if (towerHp <= 0) towerHp = 0;
}
