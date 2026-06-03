#include "GameMap.h"
#include "GameState.h"
#include "Boss.h"
#include <iostream>
#include <sstream>
#include <set>
#include <random>
#include <algorithm>
#include <queue>
#include <string>

GameMap::GameMap(int w, int h) : width(w), height(h) {
    grid.assign(height, std::vector<TileType>(width, TileType::EMPTY));
    grid[height - 1][width / 2] = TileType::BASE;
    initTerrain();
}

void GameMap::initTerrain() {
    int mt[] = { 3,3, 3,4, 4,4, 4,5, 4,6, 5,6, 6,7, 6,8 };
    for (int i = 0; i < 16; i += 2) setTile(mt[i], mt[i + 1], TileType::MOUNTAIN);
    int rv[] = { 10,2, 11,2, 12,3, 12,4, 13,5 };
    for (int i = 0; i < 10; i += 2) setTile(rv[i], rv[i + 1], TileType::RIVER);
}

std::string GameMap::getTileIcon(TileType type, char unitIcon) const {
    switch (type) {
    case TileType::EMPTY:    return " ";
    case TileType::BASE:     return "B";
    case TileType::ENEMY:    return "E";
    case TileType::MOUNTAIN: return "^";
    case TileType::RIVER:    return "~";
    default: return " ";
    }
}

std::vector<std::string> GameMap::renderLines(const GameState& state) const {
    std::vector<std::string> lines;
    std::ostringstream oss;

    auto movable = state.getMovablePositions();
    std::set<std::pair<int, int>> movableSet(movable.begin(), movable.end());

    auto attackable = state.getAttackablePositions();
    std::set<std::pair<int, int>> attackableSet(attackable.begin(), attackable.end());

    oss << "    ";
    for (int x = 0; x < width; ++x) { char b[8]; snprintf(b, 8, "%2d  ", x); oss << b; }
    lines.push_back(oss.str()); oss.str(""); oss.clear();

    for (int y = 0; y < height; ++y) {
        oss << (y == 0 ? u8"   \u250c" : u8"   \u251c");
        for (int x = 0; x < width; ++x)
            oss << u8"\u2500\u2500\u2500" << (x == width - 1 ? (y == 0 ? u8"\u2510" : u8"\u2524") : (y == 0 ? u8"\u252c" : u8"\u253c"));
        lines.push_back(oss.str()); oss.str(""); oss.clear();

        char b[8]; snprintf(b, 8, "%2d ", y); oss << b << u8"\u2502";
        for (int x = 0; x < width; ++x) {
            TileType t = grid[y][x];
            char icon = ' ';

            // 1. 아군 렌더링
            if (t == TileType::HERO) {
                for (auto hero : state.getAllies()) {
                    if (hero->isAlive() && hero->x == x && hero->y == y) icon = hero->getIcon()[0];
                }
                oss << " " << icon << u8" \u2502";
            }
            // 2. 적군(보스 포함) 렌더링
            else if (t == TileType::ENEMY) {
                for (auto e : state.getEnemies()) {
                    if (e->isAlive() && e->isOccupying(x, y)) {
                        if (e->cls == CharacterClass::BOSS_DRAGON) {
                            auto boss = dynamic_cast<DragonBoss*>(e);
                            icon = (boss && boss->isWeakPointTile(x, y)) ? 'R' : 'D';
                        }
                        else {
                            icon = e->getIcon()[0];
                        }
                        break;
                    }
                }
                oss << " " << icon << u8" \u2502";
            }
            // 3. 보물상자 (이동/공격 표시보다 우선)
            else if (t == TileType::EMPTY && state.isTreasureAt(x, y)) {
                oss << u8" $ │";
            }
            // 4. 이동 범위(*) / 공격 범위(*)
            else if (t == TileType::EMPTY && movableSet.count({ x, y })) {
                oss << u8" * \u2502";
            }
            else if (t == TileType::EMPTY && attackableSet.count({ x, y })) {
                oss << u8" * \u2502";
            }
            else {
                oss << " " << getTileIcon(t, ' ') << u8" \u2502";
            }
        }
        lines.push_back(oss.str()); oss.str(""); oss.clear();
    }

    oss << u8"   \u2514";
    for (int x = 0; x < width; ++x) oss << u8"\u2500\u2500\u2500" << (x == width - 1 ? u8"\u2518" : u8"\u2534");
    lines.push_back(oss.str());
    return lines;
}

// [기능 추가] 절차적 맵 생성 및 BFS 검증
void GameMap::randomizeTerrain(int wave, const std::vector<std::pair<int, int>>& occupied) {
    // 1. 기존 지형(산, 강) 초기화
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            if (grid[y][x] == TileType::MOUNTAIN || grid[y][x] == TileType::RIVER)
                grid[y][x] = TileType::EMPTY;

    // 2. 점유 불가 타일 셋 구성
    std::set<std::pair<int,int>> occSet(occupied.begin(), occupied.end());

    // 3. 웨이브가 높을수록 장애물이 체감되게 증가 (최대치 제한)
    int mountainCount = std::min(4 + wave * 5, 30);
    int riverCount    = std::min(2 + wave * 4, 22);

    std::mt19937 rng(std::random_device{}());

    auto tryPlace = [&](TileType type, int count) {
        int attempts = count * 20;
        int placed = 0;
        while (placed < count && attempts-- > 0) {
            int x = rng() % width;
            int y = 1 + rng() % (height - 3); // 행 0(적 스폰), 행 13~14(기지) 보호
            if (grid[y][x] != TileType::EMPTY) continue;
            if (occSet.count({x, y})) continue;
            grid[y][x] = type;
            ++placed;
        }
    };

    tryPlace(TileType::MOUNTAIN, mountainCount);
    tryPlace(TileType::RIVER,    riverCount);

    // 4. BFS로 y=0 → y=14 경로 보장
    auto hasPath = [&]() -> bool {
        bool visited[15][15] = {};
        std::queue<std::pair<int,int>> q;
        int dx[] = {0,0,-1,1};
        int dy[] = {-1,1,0,0};
        for (int x = 0; x < width; ++x) {
            if (grid[0][x] != TileType::MOUNTAIN && grid[0][x] != TileType::RIVER) {
                visited[0][x] = true;
                q.push({x, 0});
            }
        }
        while (!q.empty()) {
            std::pair<int,int> cur = q.front(); q.pop();
            int cx = cur.first, cy = cur.second;
            if (cy == height - 1) return true;
            for (int d = 0; d < 4; ++d) {
                int nx = cx + dx[d], ny = cy + dy[d];
                if (nx < 0 || nx >= width || ny < 0 || ny >= height) continue;
                if (visited[ny][nx]) continue;
                if (grid[ny][nx] == TileType::MOUNTAIN || grid[ny][nx] == TileType::RIVER) continue;
                visited[ny][nx] = true;
                q.push({nx, ny});
            }
        }
        return false;
    };

    // 5. 경로가 막히면 장애물을 하나씩 제거해 경로 확보
    while (!hasPath()) {
        bool removed = false;
        for (int y = 1; y < height - 1 && !removed; ++y) {
            for (int x = 0; x < width && !removed; ++x) {
                if (grid[y][x] == TileType::MOUNTAIN || grid[y][x] == TileType::RIVER) {
                    grid[y][x] = TileType::EMPTY;
                    removed = true;
                }
            }
        }
        if (!removed) break;
    }
}

// [기능 추가] 맵 저장 및 불러오기
void GameMap::saveTerrain(std::ostream& out) const {
    out << "TERRAIN\n";
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (grid[y][x] == TileType::MOUNTAIN || grid[y][x] == TileType::RIVER)
                out << x << " " << y << " " << static_cast<int>(grid[y][x]) << "\n";
        }
    }
    out << "END_TERRAIN\n";
}

void GameMap::loadTerrain(std::istream& in) {
    // 기존 지형 초기화
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            if (grid[y][x] == TileType::MOUNTAIN || grid[y][x] == TileType::RIVER)
                grid[y][x] = TileType::EMPTY;

    std::string token;
    while (in >> token) {
        if (token == "END_TERRAIN") break;
        if (token == "TERRAIN") continue;
        int x = std::stoi(token);
        int y, t;
        in >> y >> t;
        TileType type = static_cast<TileType>(t);
        if (type == TileType::MOUNTAIN || type == TileType::RIVER)
            setTile(x, y, type);
    }
}

void GameMap::setTile(int x, int y, TileType type) { if (x >= 0 && x < width && y >= 0 && y < height) grid[y][x] = type; }
TileType GameMap::getTileAt(int x, int y) const { return (x >= 0 && x < width && y >= 0 && y < height) ? grid[y][x] : TileType::EMPTY; }