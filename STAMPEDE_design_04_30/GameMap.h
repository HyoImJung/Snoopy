#ifndef GAMEMAP_H
#define GAMEMAP_H

#include <vector>
#include <string>

enum class TileType {
    EMPTY,
    PATH,
    BASE,
    HERO,
    ENEMY,
    MOUNTAIN, // ^ 표시
    RIVER      // ~ 표시
};

class GameMap {
private:
    int width;
    int height;
    std::vector<std::vector<TileType>> grid;

    void initTerrain(); // 지형 초기화 함수

public:
    GameMap(int w = 15, int h = 15);
    void draw() const;
    void setTile(int x, int y, TileType type);
    std::string getTileIcon(TileType type) const;
};

#endif