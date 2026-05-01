#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "Character.h"
#include "GameMap.h"
#include <vector>
#include <string>

enum class Phase { PLAYER_TURN, ENEMY_TURN, WAVE_CLEAR, GAME_OVER };

class GameState {
private:
    GameMap& map;
    std::vector<Character> allies;
    std::vector<Character> enemies;
    Phase phase;
    int currentAllyIndex;
    int towerHp, towerMaxHp, towerDp;
    int ap, maxAp, wave;
    std::string lastMessage;

    void syncMapTiles();
    int  manhattanDist(int x1, int y1, int x2, int y2) const;
    bool isOccupied(int x, int y) const;
    void spawnEnemies();
    void checkWaveClear();

public:
    GameState(GameMap& m);

    void setCurrentAllyIndex(int idx);
    void setLastMessage(const std::string& msg) { lastMessage = msg; }

    bool tryMoveAlly(int idx, int dx, int dy);
    // 수정됨: 좌표 대신 방향(char dir)을 받음
    bool tryAttackAlly(int idx, char dir);
    bool tryHealPriest(int idx, int tx, int ty);
    void endAllyAction();
    void runEnemyTurn();
    int  nextActiveAllyIndex() const;

    const std::vector<Character>& getAllies()  const { return allies; }
    const std::vector<Character>& getEnemies() const { return enemies; }
    std::vector<Character>& getAllies_mutable() { return allies; }

    Phase getPhase()       const { return phase; }
    int getCurrentAllyIndex() const { return currentAllyIndex; }
    int getTowerHp()       const { return towerHp; }
    int getAp()            const { return ap; }
    int getMaxAp()         const { return maxAp; }
    int getWave()          const { return wave; }
    const std::string& getLastMessage() const { return lastMessage; }
};

#endif