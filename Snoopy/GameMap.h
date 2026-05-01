#ifndef GAMEMAP_H
#define GAMEMAP_H

#include <vector>
#include <string>
#include <sstream>

enum class TileType {
    EMPTY,
    PATH,
    BASE,
    HERO,
    ENEMY,
    MOUNTAIN,
    RIVER
};

class GameMap {
private:
    int width;
    int height;
    std::vector<std::vector<TileType>> grid;

    void initTerrain();

public:
    GameMap(int w = 15, int h = 15);
    void draw() const;
    std::vector<std::string> renderLines() const;
    void setTile(int x, int y, TileType type);
    TileType getTileAt(int x, int y) const;
    std::string getTileIcon(TileType type) const;
};

#endif
