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

// -------------------------------------------------------
// Save format (text):
//   wave ap towerHp
//   ALLIES <count>
//   cls x y hp maxHp atk actedThisTurn
//   ENEMIES <count>
//   x y hp maxHp atk alive
// -------------------------------------------------------

bool saveGame(const GameState& state, int slot) {
    std::ofstream f(getSaveFilePath(slot));
    if (!f.is_open()) return false;

    f << state.getWave() << " "
      << state.getAp()   << " "
      << state.getTowerHp() << "\n";

    const auto& allies = state.getAllies();
    f << "ALLIES " << allies.size() << "\n";
    for (auto* a : allies) {
        f << static_cast<int>(a->cls) << " "
          << a->x << " " << a->y << " "
          << a->hp << " " << a->maxHp << " "
          << a->atk << " "
          << (a->actedThisTurn ? 1 : 0) << "\n";
    }

    const auto& enemies = state.getEnemies();
    f << "ENEMIES " << enemies.size() << "\n";
    for (auto* e : enemies) {
        f << e->x << " " << e->y << " "
          << e->hp << " " << e->maxHp << " "
          << e->atk << " "
          << (e->isAlive() ? 1 : 0) << "\n";
    }

    f.close();
    return true;
}

bool loadGame(GameState& state, int slot) {
    std::ifstream f(getSaveFilePath(slot));
    if (!f.is_open()) return false;

    int wave, ap, towerHp;
    f >> wave >> ap >> towerHp;
    state.setLoadedData(wave, ap, towerHp);

    std::string token;
    int count;

    // Load allies
    f >> token >> count;
    auto& allies = state.getAllies_mutable();
    for (auto* a : allies) delete a;
    allies.clear();

    for (int i = 0; i < count; ++i) {
        int cls, x, y, hp, maxHp, atk, acted;
        f >> cls >> x >> y >> hp >> maxHp >> atk >> acted;

        Character* c = nullptr;
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
    auto& enemies = state.getEnemies_mutable();
    for (auto* e : enemies) delete e;
    enemies.clear();

    for (int i = 0; i < count; ++i) {
        int x, y, hp, maxHp, atk, alive;
        f >> x >> y >> hp >> maxHp >> atk >> alive;

        Enemy* e = new Enemy("Orc" + std::to_string(i + 1), x, y, hp, atk);
        e->hp = hp; e->maxHp = maxHp;
        if (!alive) e->hp = 0;
        enemies.push_back(e);
    }

    f.close();
    state.syncMapTilesPublic();
    return true;
}
