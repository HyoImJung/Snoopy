#include "GameMap.h"
#include "GameState.h"
#include <iostream>
#include <sstream>
#include <set>

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

    auto movable = state.getMovablePositions();
    std::set<std::pair<int, int>> movableSet(movable.begin(), movable.end());

    oss << "      ";
    for (int x = 0; x < width; ++x) { char b[8]; snprintf(b, 8, "%2d  ", x); oss << b; }
    lines.push_back(oss.str()); oss.str(""); oss.clear();

    for (int y = 0; y < height; ++y) {
        oss << (y == 0 ? u8"   \u250c" : u8"   \u251c");
        for (int x = 0; x < width; ++x)
            oss << u8"\u2500\u2500\u2500"
                << (x == width - 1 ? (y == 0 ? u8"\u2510" : u8"\u2524")
                                   : (y == 0 ? u8"\u252c" : u8"\u253c"));
        lines.push_back(oss.str()); oss.str(""); oss.clear();

        char b[8]; snprintf(b, 8, "%2d ", y); oss << b << u8"\u2502";
        for (int x = 0; x < width; ++x) {
            TileType t = grid[y][x];
            char icon = ' ';
            if (t == TileType::HERO)
                for (int i = 0; i < (int)state.getAllies().size(); ++i)
                    if (state.getAllies()[i].isAlive() &&
                        state.getAllies()[i].x == x &&
                        state.getAllies()[i].y == y)
                        icon = state.getAllies()[i].getIcon()[0];

            if (t == TileType::EMPTY && movableSet.count(std::make_pair(x, y)))
                oss << u8" * \u2502";
            else
                oss << " " << getTileIcon(t, icon) << u8" \u2502";
        }
        lines.push_back(oss.str()); oss.str(""); oss.clear();
    }

    oss << u8"   \u2514";
    for (int x = 0; x < width; ++x)
        oss << u8"\u2500\u2500\u2500" << (x == width - 1 ? u8"\u2518" : u8"\u2534");
    lines.push_back(oss.str());
    return lines;
}

void GameMap::setTile(int x, int y, TileType type) {
    if (x >= 0 && x < width && y >= 0 && y < height) grid[y][x] = type;
}

TileType GameMap::getTileAt(int x, int y) const {
    return (x >= 0 && x < width && y >= 0 && y < height) ? grid[y][x] : TileType::EMPTY;
}
