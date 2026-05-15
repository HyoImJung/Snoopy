#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "GameMap.h"
#include "Character.h" // Character struct/class definition
#include "Enemy.h" // Enemy manager
#include <vector>
#include <string>

// Defines current game phase
enum class Phase { PLAYER_TURN, ENEMY_TURN, WAVE_CLEAR, GAME_OVER };

class GameState {
private:
    GameMap& map;                   // Game map reference
    std::vector<Character*> allies;  // Ally pointer vector (polymorphism)
    std::vector<Character*> enemies; // Enemy pointer vector

    EnemyManager enemyMgr;          // Enemy spawn & AI manager

    Phase phase;                    // Current game phase
    int currentAllyIndex;           // Currently selected ally index

    int towerHp, towerMaxHp;        // Tower HP
    int ap, maxAp;                  // Player action points
    int wave;                       // Current wave number
    std::string lastMessage;        // Message shown in bottom UI
    bool isOccupied(int x, int y) const;

    // Internal utility functions
    void syncMapTiles();            // Sync unit positions to map tiles
    void checkWaveClear();          // Check if all enemies are defeated

public:
    // Constructor & destructor (free dynamic memory in destructor)
    GameState(GameMap& m);
    ~GameState();

    // State setters & message management
    void setCurrentAllyIndex(int idx);
    void setLastMessage(const std::string& msg) { lastMessage = msg; }

    // Core game logic
    bool tryMoveAlly(int idx, int dx, int dy);   // Try to move ally
    bool tryAttackAlly(int idx, char dir);       // Try to attack with ally
    void endAllyAction();                        // Handle end of ally turn
    int  manhattanDist(int x1, int y1, int x2, int y2) const;

    // Helper functions for UI and map rendering
    std::vector<std::pair<int, int>> getMovablePositions() const;

    // Getter functions
    const std::vector<Character*>& getAllies()  const { return allies; }
    const std::vector<Character*>& getEnemies() const { return enemies; }

    // Non-const accessors for external modification
    std::vector<Character*>& getAllies_mutable()  { return allies; }
    std::vector<Character*>& getEnemies_mutable() { return enemies; }

    // For save/load
    void setLoadedData(int w, int a, int th) { wave = w; ap = a; towerHp = th; }
    void syncMapTilesPublic() { syncMapTiles(); }

    Phase       getPhase()            const { return phase; }
    int         getCurrentAllyIndex() const { return currentAllyIndex; }
    int         getTowerHp()          const { return towerHp; }
    int         getAp()               const { return ap; }
    int         getMaxAp()            const { return maxAp; }
    int         getWave()             const { return wave; }
    std::string getLastMessage()      const { return lastMessage; }
};

#endif