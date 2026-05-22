#include "GameMap.h"
#include "GameState.h"
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
    case TileType::HERO:     return std::string(1, unitIcon);
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

    std::vector<std::pair<int,int> > movable = state.getMovablePositions();
    std::set<std::pair<int,int> > movableSet(movable.begin(), movable.end());

    oss << "    ";
    for (int x = 0; x < width; ++x) {
        char b[8];
        snprintf(b, 8, "%2d  ", x);
        oss << b;
    }
    lines.push_back(oss.str()); oss.str(""); oss.clear();

    for (int y = 0; y < height; ++y) {
        oss << (y == 0 ? u8"   \u250c" : u8"   \u251c");
        for (int x = 0; x < width; ++x) {
            oss << u8"\u2500\u2500\u2500";
            if (x == width - 1)
                oss << (y == 0 ? u8"\u2510" : u8"\u2524");
            else
                oss << (y == 0 ? u8"\u252c" : u8"\u253c");
        }
        lines.push_back(oss.str()); oss.str(""); oss.clear();

        char b[8];
        snprintf(b, 8, "%2d ", y);
        oss << b << u8"\u2502";
        for (int x = 0; x < width; ++x) {
            TileType t = grid[y][x];
            char icon = ' ';
            if (t == TileType::HERO) {
                for (int i = 0; i < (int)state.getAllies().size(); ++i) {
                    if (state.getAllies()[i]->isAlive() &&
                        state.getAllies()[i]->x == x &&
                        state.getAllies()[i]->y == y) {
                        icon = state.getAllies()[i]->getIcon()[0];
                    }
                }
            }
            if (t == TileType::EMPTY && movableSet.count(std::make_pair(x, y)))
                oss << u8" * \u2502";
            else
                oss << " " << getTileIcon(t, icon) << u8" \u2502";
        }
        lines.push_back(oss.str()); oss.str(""); oss.clear();
    }

    oss << u8"   \u2514";
    for (int x = 0; x < width; ++x) {
        oss << u8"\u2500\u2500\u2500";
        oss << (x == width - 1 ? u8"\u2518" : u8"\u2534");
    }
    lines.push_back(oss.str());
    return lines;
}

void GameMap::setTile(int x, int y, TileType type) {
    if (x >= 0 && x < width && y >= 0 && y < height) grid[y][x] = type;
}

TileType GameMap::getTileAt(int x, int y) const {
    return (x >= 0 && x < width && y >= 0 && y < height) ? grid[y][x] : TileType::EMPTY;
}

void GameMap::randomizeTerrain(int wave,
    const std::vector<std::pair<int,int> >& occupied)
{
    for (int ry = 0; ry < height; ry++) {
        for (int rx = 0; rx < width; rx++) {
            TileType t = grid[ry][rx];
            if (t == TileType::MOUNTAIN || t == TileType::RIVER)
                grid[ry][rx] = TileType::EMPTY;
        }
    }

    std::set<std::pair<int,int> > blocked;
    for (int i = 0; i < (int)occupied.size(); i++) {
        blocked.insert(occupied[i]);
    }
    blocked.insert(std::make_pair(width / 2, height - 1));
    for (int x = 0; x < width; x++) {
        blocked.insert(std::make_pair(x, 0));
    }
    for (int i = 0; i < (int)occupied.size(); i++) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                int nx = occupied[i].first + dx;
                int ny = occupied[i].second + dy;
                if (nx >= 0 && nx < width && ny >= 0 && ny < height)
                    blocked.insert(std::make_pair(nx, ny));
            }
        }
    }

    // BFS connectivity check lambda replaced with inline function approach
    // We check connectivity inline below after each cluster placement

    std::mt19937 rng(std::random_device{}());
    int clusters = std::min(4 + (wave - 1) * 6, 40);
    int maxSize  = std::min(3 + (wave - 1) * 2, 12);
    int minSize  = std::min(2 + (wave - 1), maxSize - 1);
    if (minSize < 2) minSize = 2;

    int riverThresh = (wave >= 2) ? 4 : 3;
    std::uniform_int_distribution<int> typeRoll(0, 9);
    std::uniform_int_distribution<int> xRoll(0, width - 1);
    std::uniform_int_distribution<int> yRoll(1, height - 3);
    std::uniform_int_distribution<int> sizeRoll(minSize, maxSize);
    std::uniform_int_distribution<int> dirRoll(0, 3);
    int dx4[4] = { 0, 0, 1, -1 };
    int dy4[4] = { 1, -1, 0,  0 };

    for (int c = 0; c < clusters; c++) {
        TileType ttype = (typeRoll(rng) < riverThresh)
            ? TileType::MOUNTAIN : TileType::RIVER;
        int size = sizeRoll(rng);

        int attempts = 0, sx = -1, sy = -1;
        while (attempts < 80) {
            attempts++;
            int tx = xRoll(rng), ty = yRoll(rng);
            if (blocked.count(std::make_pair(tx, ty)) == 0 &&
                grid[ty][tx] == TileType::EMPTY) {
                sx = tx; sy = ty; break;
            }
        }
        if (sx == -1) continue;

        std::vector<std::pair<int,int> > cluster;
        cluster.push_back(std::make_pair(sx, sy));
        int cx = sx, cy = sy;
        int preferDir = dirRoll(rng);
        for (int s = 1; s < size; s++) {
            bool placed = false;
            int startDir = (ttype == TileType::RIVER) ? preferDir : dirRoll(rng);
            for (int dd = 0; dd < 4; dd++) {
                int d = (startDir + dd) % 4;
                int nx = cx + dx4[d], ny = cy + dy4[d];
                if (nx < 0 || nx >= width || ny < 0 || ny >= height) continue;
                if (blocked.count(std::make_pair(nx, ny))) continue;
                if (grid[ny][nx] != TileType::EMPTY) continue;
                cluster.push_back(std::make_pair(nx, ny));
                cx = nx; cy = ny; placed = true; break;
            }
            if (!placed) break;
        }

        for (int i = 0; i < (int)cluster.size(); i++)
            grid[cluster[i].second][cluster[i].first] = ttype;

        // BFS connectivity check
        bool vis[15][15] = {};
        std::queue<std::pair<int,int> > q;
        int bx = width / 2, by = height - 1;
        for (int px = 0; px < width; px++) {
            if (grid[0][px] == TileType::EMPTY || grid[0][px] == TileType::BASE) {
                vis[0][px] = true;
                q.push(std::make_pair(px, 0));
            }
        }
        while (!q.empty()) {
            std::pair<int,int> cur = q.front(); q.pop();
            int qx = cur.first, qy = cur.second;
            for (int d = 0; d < 4; d++) {
                int nx = qx + dx4[d], ny = qy + dy4[d];
                if (nx < 0 || nx >= width || ny < 0 || ny >= height) continue;
                if (vis[ny][nx]) continue;
                TileType t = grid[ny][nx];
                if (t == TileType::MOUNTAIN || t == TileType::RIVER) continue;
                vis[ny][nx] = true;
                q.push(std::make_pair(nx, ny));
            }
        }
        if (!vis[by][bx]) {
            for (int i = 0; i < (int)cluster.size(); i++)
                grid[cluster[i].second][cluster[i].first] = TileType::EMPTY;
        }
    }
}

void GameMap::saveTerrain(std::ostream& out) const {
    out << "TERRAIN\n";
    for (int sy = 0; sy < height; sy++) {
        for (int sx = 0; sx < width; sx++) {
            TileType t = grid[sy][sx];
            if (t == TileType::MOUNTAIN || t == TileType::RIVER) {
                out << sx << " " << sy << " "
                    << static_cast<int>(t) << "\n";
            }
        }
    }
    out << "END_TERRAIN\n";
}

void GameMap::loadTerrain(std::istream& in) {
    for (int ly = 0; ly < height; ly++) {
        for (int lx = 0; lx < width; lx++) {
            TileType t = grid[ly][lx];
            if (t == TileType::MOUNTAIN || t == TileType::RIVER)
                grid[ly][lx] = TileType::EMPTY;
        }
    }

    // >> 방식으로 TERRAIN 토큰 탐색 (>> 후 스트림 위치와 호환)
    std::string token;
    bool found = false;
    while (in >> token) {
        if (token == "TERRAIN") { found = true; break; }
    }
    if (!found) return;

    while (in >> token) {
        if (token == "END_TERRAIN") break;
        int tx = 0, ty = 0, tt = 0;
        // token은 x 좌표
        tx = std::stoi(token);
        if (!(in >> ty >> tt)) break;
        if (tx >= 0 && tx < width && ty >= 0 && ty < height)
            grid[ty][tx] = static_cast<TileType>(tt);
    }
}
