#ifndef GAMEMAP_H
#define GAMEMAP_H

#include <vector>
#include <string>
#include <iostream>

enum class TileType { EMPTY, PATH, BASE, HERO, ENEMY, MOUNTAIN, RIVER };

class GameMap {
private:
    int width, height;
    std::vector<std::vector<TileType> > grid;
    void initTerrain();

public:
    GameMap(int w = 15, int h = 15);
    std::vector<std::string> renderLines(const class GameState& state) const;
    void setTile(int x, int y, TileType type);
    TileType getTileAt(int x, int y) const;
    std::string getTileIcon(TileType type, char unitIcon = ' ') const;

    // Randomize terrain for a new wave.
    // wave: current wave number (higher = more obstacles).
    // occupied: list of (x,y) positions that must not become terrain
    //           (ally positions, base tile, enemy spawn row).
    void randomizeTerrain(int wave,
                          const std::vector<std::pair<int,int> >& occupied);

    // Terrain persistence: save/load only MOUNTAIN and RIVER tiles.
    // Format written to stream: "TERRAIN\n x y type\n ... END_TERRAIN\n"
    void saveTerrain(std::ostream& out) const;
    void loadTerrain(std::istream& in);
};

#endif
