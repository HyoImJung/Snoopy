#include "SaveLoad.h"
#include "Knight.h"
#include "Archer.h"
#include "Priest.h"
#include "Mage.h"
#include "Enemy.h"
#include <fstream>
#include <string>

std::string getSaveFilePath(int slot) {
    return "save_slot" + std::to_string(slot + 1) + ".dat";
}

bool hasSaveFile(int slot) {
    std::ifstream f(getSaveFilePath(slot));
    return f.good();
}

bool saveGame(const GameState& state, const GameMap& map, int slot) {
    std::ofstream f(getSaveFilePath(slot));
    if (!f.is_open()) return false;

    f << state.getWave() << " "
      << state.getAp()   << " "
      << state.getTowerHp() << "\n";

    const std::vector<Character*>& allies = state.getAllies();
    f << "ALLIES " << allies.size() << "\n";
    for (int i = 0; i < (int)allies.size(); i++) {
        Character* a = allies[i];
        f << static_cast<int>(a->cls) << " "
          << a->x << " " << a->y << " "
          << a->hp << " " << a->maxHp << " "
          << a->atk << " "
          << (a->actedThisTurn ? 1 : 0) << "\n";
    }

    const std::vector<Character*>& enemies = state.getEnemies();
    f << "ENEMIES " << enemies.size() << "\n";
    for (int i = 0; i < (int)enemies.size(); i++) {
        Character* e = enemies[i];
        f << e->x << " " << e->y << " "
          << e->hp << " " << e->maxHp << " "
          << e->atk << " "
          << (e->isAlive() ? 1 : 0) << "\n";
    }

    // GameMap을 직접 받아서 지형 저장 (const 문제 없음)
    map.saveTerrain(f);

    f.close();
    return true;
}

bool loadGame(GameState& state, GameMap& map, int slot) {
    std::ifstream f(getSaveFilePath(slot));
    if (!f.is_open()) return false;

    int wave, ap, towerHp;
    f >> wave >> ap >> towerHp;
    state.setLoadedData(wave, ap, towerHp);

    std::string token;
    int count;

    // Load allies
    f >> token >> count;
    std::vector<Character*>& allies = state.getAllies_mutable();
    for (int i = 0; i < (int)allies.size(); i++) delete allies[i];
    allies.clear();

    for (int i = 0; i < count; i++) {
        int cls, x, y, hp, maxHp, atk, acted;
        f >> cls >> x >> y >> hp >> maxHp >> atk >> acted;
        Character* c = 0;
        switch (static_cast<CharacterClass>(cls)) {
            case CharacterClass::KNIGHT: c = new Knight(x, y); break;
            case CharacterClass::ARCHER: c = new Archer(x, y); break;
            case CharacterClass::PRIEST: c = new Priest(x, y); break;
            case CharacterClass::MAGE:   c = new Mage(x, y);   break;
            default: continue;
        }
        c->hp = hp; c->maxHp = maxHp; c->atk = atk;
        c->actedThisTurn = (acted == 1);
        allies.push_back(c);
    }

    // Load enemies
    f >> token >> count;
    std::vector<Character*>& enemies = state.getEnemies_mutable();
    for (int i = 0; i < (int)enemies.size(); i++) delete enemies[i];
    enemies.clear();

    for (int i = 0; i < count; i++) {
        int x, y, hp, maxHp, atk, alive;
        f >> x >> y >> hp >> maxHp >> atk >> alive;
        Enemy* e = new Enemy("Orc" + std::to_string(i + 1), x, y, hp, atk);
        e->hp = hp; e->maxHp = maxHp;
        if (!alive) e->hp = 0;
        enemies.push_back(e);
    }

    // GameMap을 직접 받아서 지형 복원
    map.loadTerrain(f);

    f.close();
    state.syncMapTilesPublic();
    return true;
}
