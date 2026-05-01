#ifndef GAMEMAP_H
#define GAMEMAP_H
#include <vector>
#include <string>

enum class TileType { EMPTY, PATH, BASE, HERO, ENEMY, MOUNTAIN, RIVER };

class GameMap {
private:
    int width, height;
    std::vector<std::vector<TileType>> grid;
    void initTerrain();
public:
    GameMap(int w = 15, int h = 15);
    std::vector<std::string> renderLines(const class GameState& state) const; // state 인자 추가[cite: 1]
    void setTile(int x, int y, TileType type);
    TileType getTileAt(int x, int y) const;
    std::string getTileIcon(TileType type, char unitIcon = ' ') const;
};
#endif