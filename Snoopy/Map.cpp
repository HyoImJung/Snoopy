#include "GameMap.h"
#include <iostream>
#include <cstdio>

GameMap::GameMap(int w, int h) : width(w), height(h) {
    grid.assign(height, std::vector<TileType>(width, TileType::EMPTY));
    grid[height - 1][width / 2] = TileType::BASE;
    initTerrain();
}

void GameMap::initTerrain() {
    setTile(3, 3, TileType::MOUNTAIN);
    setTile(3, 4, TileType::MOUNTAIN);
    setTile(4, 4, TileType::MOUNTAIN);
    setTile(4, 5, TileType::MOUNTAIN);
    setTile(4, 6, TileType::MOUNTAIN);
    setTile(5, 6, TileType::MOUNTAIN);
    setTile(6, 7, TileType::MOUNTAIN);
    setTile(6, 8, TileType::MOUNTAIN);

    setTile(10, 2, TileType::RIVER);
    setTile(11, 2, TileType::RIVER);
    setTile(12, 3, TileType::RIVER);
    setTile(12, 4, TileType::RIVER);
    setTile(13, 5, TileType::RIVER);
}

std::string GameMap::getTileIcon(TileType type) const {
    switch (type) {
    case TileType::EMPTY:    return " ";
    case TileType::PATH:     return ".";
    case TileType::BASE:     return "B";
    case TileType::HERO:     return "H";
    case TileType::ENEMY:    return "E";
    case TileType::MOUNTAIN: return "^";
    case TileType::RIVER:    return "~";
    default: return " ";
    }
}

void GameMap::draw() const {
    std::cout << "     ";
    for (int x = 0; x < width; ++x) printf("%2d  ", x);
    std::cout << "\n   +";
    for (int x = 0; x < width; ++x) std::cout << "---" << (x == width - 1 ? "+" : "+");
    std::cout << "\n";

    for (int y = 0; y < height; ++y) {
        printf("%2d |", y);
        for (int x = 0; x < width; ++x) {
            std::cout << " " << getTileIcon(grid[y][x]) << " |";
        }
        std::cout << "\n";
        std::cout << "   +";
        for (int x = 0; x < width; ++x) std::cout << "---" << (x == width - 1 ? "+" : "+");
        std::cout << "\n";
    }
}

void GameMap::setTile(int x, int y, TileType type) {
    if (x >= 0 && x < width && y >= 0 && y < height)
        grid[y][x] = type;
}

TileType GameMap::getTileAt(int x, int y) const {
    if (x >= 0 && x < width && y >= 0 && y < height)
        return grid[y][x];
    return TileType::EMPTY;
}

std::vector<std::string> GameMap::renderLines() const {
    std::vector<std::string> lines;
    std::ostringstream oss;

    // header
    oss << "      ";
    for (int x = 0; x < width; ++x) {
        char buf[8];
        snprintf(buf, sizeof(buf), "%2d  ", x);
        oss << buf;
    }
    lines.push_back(oss.str()); oss.str(""); oss.clear();

    // top border
    oss << "   +";
    for (int x = 0; x < width; ++x) oss << "---" << (x == width-1 ? "+" : "+");
    lines.push_back(oss.str()); oss.str(""); oss.clear();

    for (int y = 0; y < height; ++y) {
        // row
        char buf[8]; snprintf(buf, sizeof(buf), "%2d |", y);
        oss << buf;
        for (int x = 0; x < width; ++x)
            oss << " " << getTileIcon(grid[y][x]) << " |";
        lines.push_back(oss.str()); oss.str(""); oss.clear();

        // row separator
        oss << "   +";
        for (int x = 0; x < width; ++x) oss << "---" << (x == width-1 ? "+" : "+");
        lines.push_back(oss.str()); oss.str(""); oss.clear();
    }
    return lines;
}
