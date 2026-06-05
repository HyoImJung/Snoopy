#include "SaveLoad.h"
#include "Knight.h"
#include "Archer.h"
#include "Priest.h"
#include "Mage.h"
#include "Enemy.h"
#include <fstream>
#include <string>

// ★ [추가] Enemy.cpp에 만들어둔 팩토리 함수를 외부(extern)에서 끌어옵니다!
extern Character* createEnemyInstance(int cls, int x, int y);

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
        << state.getAp() << " "
        << state.getMaxAp() << " "
        << state.getTowerHp() << "\n";

    const std::vector<Character*>& allies = state.getAllies();
    f << "ALLIES " << allies.size() << "\n";
    for (int i = 0; i < (int)allies.size(); i++) {
        Character* a = allies[i];
        f << static_cast<int>(a->cls) << " "
            << a->x << " " << a->y << " "
            << a->hp << " " << a->maxHp << " "
            << a->atk << " " << a->def << " "
            << (a->actedThisTurn ? 1 : 0) << "\n";
    }

    const std::vector<Character*>& enemies = state.getEnemies();
    f << "ENEMIES " << enemies.size() << "\n";
    for (int i = 0; i < (int)enemies.size(); i++) {
        Character* e = enemies[i];
        // ★ [수정] 적군도 아군처럼 직업(cls) 번호를 먼저 저장하도록 개편!
        f << static_cast<int>(e->cls) << " "
            << e->x << " " << e->y << " "
            << e->hp << " " << e->maxHp << " "
            << e->atk << " "
            << (e->isAlive() ? 1 : 0) << "\n";
    }

    // 보물상자 저장
    const std::vector<std::pair<int,int>>& treasures = state.getTreasures();
    f << "TREASURES " << treasures.size() << "\n";
    for (auto& t : treasures) f << t.first << " " << t.second << "\n";

    // GameMap을 직접 받아서 지형 저장
    map.saveTerrain(f);

    f.close();
    return true;
}

bool loadGame(GameState& state, GameMap& map, int slot) {
    std::ifstream f(getSaveFilePath(slot));
    if (!f.is_open()) return false;

    int wave, ap, maxAp, towerHp;
    f >> wave >> ap >> maxAp >> towerHp;
    state.setLoadedData(wave, ap, towerHp);
    state.setMaxAp(maxAp);

    std::string token;
    int count;

    // Load allies
    f >> token >> count;
    std::vector<Character*>& allies = state.getAllies_mutable();
    for (int i = 0; i < (int)allies.size(); i++) delete allies[i];
    allies.clear();

    for (int i = 0; i < count; i++) {
        int cls, x, y, hp, maxHp, atk, def, acted;
        f >> cls >> x >> y >> hp >> maxHp >> atk >> def >> acted;
        Character* c = 0;
        switch (static_cast<CharacterClass>(cls)) {
        case CharacterClass::KNIGHT: c = new Knight(x, y); break;
        case CharacterClass::ARCHER: c = new Archer(x, y); break;
        case CharacterClass::PRIEST: c = new Priest(x, y); break;
        case CharacterClass::MAGE:   c = new Mage(x, y);   break;
        default: continue;
        }
        c->hp = hp; c->maxHp = maxHp; c->atk = atk; c->def = def;
        c->actedThisTurn = (acted == 1);
        allies.push_back(c);
    }

    // Load enemies
    f >> token >> count;
    std::vector<Character*>& enemies = state.getEnemies_mutable();
    for (int i = 0; i < (int)enemies.size(); i++) delete enemies[i];
    enemies.clear();

    for (int i = 0; i < count; i++) {
        int cls, x, y, hp, maxHp, atk, alive;

        // ★ [수정] 파일에서 저장해둔 적군의 직업(cls) 번호를 같이 읽어옵니다!
        f >> cls >> x >> y >> hp >> maxHp >> atk >> alive;

        // ★ [핵심] 팩토리 함수를 호출하여 완벽하게 일치하는 몬스터/보스로 되살려냅니다.
        Character* e = createEnemyInstance(cls, x, y);
        e->hp = hp;
        e->maxHp = maxHp;

        // 웨이브 스케일링이 적용된 공격력이면 그걸 유지하도록 덮어씌움
        e->atk = atk;

        if (!alive) e->hp = 0;
        enemies.push_back(e);
    }

    // 보물상자 복원 (구버전 세이브 호환: TREASURES 블록이 없으면 건너뜀)
    std::vector<std::pair<int,int>>& treasures = state.getTreasures_mutable();
    treasures.clear();
    f >> token;
    if (token == "TREASURES") {
        f >> count;
        for (int i = 0; i < count; i++) {
            int tx, ty;
            f >> tx >> ty;
            treasures.push_back({ tx, ty });
        }
        // 다음은 지형 블록
        map.loadTerrain(f);
    }
    else {
        // 구버전 세이브: 방금 읽은 토큰이 "TERRAIN" → 지형 항목들이 이어짐
        map.loadTerrain(f);
    }

    f.close();
    state.syncMapTilesPublic();
    return true;
}