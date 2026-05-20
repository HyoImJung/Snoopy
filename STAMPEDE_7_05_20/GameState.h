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
    bool isSkillMenuOpen; // 스킬 선택 모드 플래그

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

    // Getter / Setter
    bool getIsSkillMenuOpen() const { return isSkillMenuOpen; }
    void setSkillMenuOpen(bool open) { isSkillMenuOpen = open; }

    // 스킬 관련 제어 함수
    void openSkillMenu();
    void closeSkillMenu();
    bool tryUseSkill(int skillIdx);

    // Enemy AI helpers
    bool canEnemyMoveTo(int nx, int ny) const {
        // 1. 맵 경계 체크
        if (nx < 0 || nx >= 15 || ny < 0 || ny >= 15) return false;

        // 2. 지형지물(산, 강, 타워 기지 등) 충돌 체크
        if (map.getTileAt(nx, ny) != TileType::EMPTY) return false;

        // 3. 다른 유닛(아군/적군)이 서 있는지 체크 (이미 구현된 isOccupied 활용)
        if (isOccupied(nx, ny)) return false;

        return true;
    }
    void damageTower(int dmg) { towerHp -= dmg; if (towerHp < 0) towerHp = 0; }

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
    int         getTowerMaxHp()       const { return towerMaxHp; }
    int         getTowerHp()          const { return towerHp; }
    int         getAp()               const { return ap; }
    int         getMaxAp()            const { return maxAp; }
    int         getWave()             const { return wave; }
    std::string getLastMessage()      const { return lastMessage; }
};

#endif