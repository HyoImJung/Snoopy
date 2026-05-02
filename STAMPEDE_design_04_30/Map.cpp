#include "GameMap.h"
#include <iostream>

GameMap::GameMap(int w, int h) : width(w), height(h) {
    grid.assign(height, std::vector<TileType>(width, TileType::EMPTY));

    // 1. 기본 본진 배치
    grid[height - 1][width / 2] = TileType::BASE;

    // 2. 지형 초기화 호출
    initTerrain();
}

void GameMap::initTerrain() {
    // 산맥 배치 (사진의 빨간 표시 경로: 7개)
    setTile(3, 3, TileType::MOUNTAIN);
    setTile(3, 4, TileType::MOUNTAIN);
    setTile(4, 4, TileType::MOUNTAIN);
    setTile(4, 5, TileType::MOUNTAIN);
    setTile(4, 6, TileType::MOUNTAIN);
    setTile(5, 6, TileType::MOUNTAIN);
    setTile(6, 7, TileType::MOUNTAIN);
    setTile(6, 8, TileType::MOUNTAIN);

    // 강 배치 (예시: 맵 우측 상단에 5개 배치)
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
    case TileType::MOUNTAIN: return "^"; // 산
    case TileType::RIVER:    return "~"; // 강
    default: return " ";
    }
}

void GameMap::draw() const {
    // (기존 draw 로직과 동일 - 생략 없이 유지)
    std::cout << "     ";
    for (int x = 0; x < width; ++x) printf("%2d  ", x);
    std::cout << "\n   ┌";
    for (int x = 0; x < width; ++x) std::cout << "───" << (x == width - 1 ? "┐" : "┬");
    std::cout << "\n";

    for (int y = 0; y < height; ++y) {
        printf("%2d │", y);
        for (int x = 0; x < width; ++x) {
            std::cout << " " << getTileIcon(grid[y][x]) << " │";
        }
        std::cout << "\n";
        if (y < height - 1) {
            std::cout << "   ├";
            for (int x = 0; x < width; ++x) std::cout << "───" << (x == width - 1 ? "┤" : "┼");
            std::cout << "\n";
        }
    }
    std::cout << "   └";
    for (int x = 0; x < width; ++x) std::cout << "───" << (x == width - 1 ? "┘" : "┴");
    std::cout << "\n";
}

void GameMap::setTile(int x, int y, TileType type) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        grid[y][x] = type;
    }
}