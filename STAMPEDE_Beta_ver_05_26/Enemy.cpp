#include "Enemy.h"
#include "GameState.h"
#include "Sound.h"
#include "Boss.h" 
#include "Elite.h" // 엘리트 몬스터 생성을 위한 헤더
#include <cstdlib>
#include <string>
#include <queue>
#include <vector>
#include <utility>

// 일반 Enemy
Enemy::Enemy(std::string n, int px, int py, int h, int a)
    : Character(n, CharacterClass::ENEMY_ORC, px, py, h, a, 1, 1, false) {}

bool Enemy::performAttack(char dir, GameState& state) { return false; }

// Slime 구현
Slime::Slime(int px, int py) : Character("Slime", CharacterClass::ENEMY_SLIME, px, py, 10, 2, 1, 1, false), hasSplit(false) {}
bool Slime::performAttack(char dir, GameState& state) { return false; }
bool Slime::takeDamage(int dmg, int tx, int ty) {
    bool isDead = Character::takeDamage(dmg, tx, ty);
    if (hp <= 0 && !hasSplit) {
        hp = maxHp / 2;
        hasSplit = true;
    }
    return isDead;
}

// Goblin 구현
Goblin::Goblin(int px, int py) : Character("Goblin", CharacterClass::ENEMY_GOBLIN, px, py, 8, 4, 1, 1, false) {}
bool Goblin::performAttack(char dir, GameState& state) { return true; }

// Orc 구현
Orc::Orc(int px, int py) : Character("Orc", CharacterClass::ENEMY_ORC, px, py, 20, 3, 1, 1, false) {}
bool Orc::performAttack(char dir, GameState& state) { return false; }

// Undead 구현
Undead::Undead(int px, int py) : Character("Undead", CharacterClass::ENEMY_UNDEAD, px, py, 15, 3, 1, 1, false) {}
bool Undead::performAttack(char dir, GameState& state) { return false; }

// Demon 구현
Demon::Demon(int px, int py) : Character("Demon", CharacterClass::ENEMY_DEMON, px, py, 30, 6, 1, 1, false) {}
bool Demon::performAttack(char dir, GameState& state) { return true; }


// =======================================================
// [스폰 매니저] 웨이브별 지정 몬스터 생성 (수정본)
// =======================================================
void EnemyManager::spawnWave(int wave, std::vector<Character*>& enemies) {
    // 1웨이브: 고블린 2, 슬라임 2, 오크 2 (총 6마리 가로 정렬 배치)
    if (wave == 1) {
        enemies.push_back(new Goblin(2, 0));
        enemies.push_back(new Goblin(4, 0));
        enemies.push_back(new Slime(6, 0));
        enemies.push_back(new Slime(8, 0));
        enemies.push_back(new Orc(10, 0));
        enemies.push_back(new Orc(12, 0));
    }
    // 2웨이브: 리치킹 1, 주변 언데드 2 (총 3마리 배치)
    else if (wave == 2) {
        enemies.push_back(new LichKing(7, 1));  // 리치킹 중앙 배치
        enemies.push_back(new Undead(5, 1));    // 좌측 호위 언데드
        enemies.push_back(new Undead(9, 1));    // 우측 호위 언데드
    }
    // 3웨이브: 악마왕(Vaal 'V') 1, 주변 악마 4 (총 5마리 배치)
    else if (wave == 3) {
        enemies.push_back(new DemonKing(7, 1)); // 악마왕 중앙 배치
        enemies.push_back(new Demon(4, 1));     // 악마들 빽빽하게 가로 정렬
        enemies.push_back(new Demon(6, 1));
        enemies.push_back(new Demon(8, 1));
        enemies.push_back(new Demon(10, 1));
    }
    // 4웨이브: 최종 보스 드래곤 단독 스폰
    else if (wave == 4) {
        enemies.push_back(new DragonBoss());
    }
}

// =======================================================
// [팀원 로직] BFS 최단 거리 이동 (유지)
// =======================================================
static std::pair<int, int> bfsNext(int sx, int sy, int gx, int gy, const GameState& state, const std::vector<std::pair<int, int> >& others) {
    bool blocked[15][15] = {};
    for (auto o : others) if (o.first >= 0 && o.first < 15 && o.second >= 0 && o.second < 15) blocked[o.second][o.first] = true;
    int dist[15][15];
    for (int i = 0; i < 15; ++i) for (int j = 0; j < 15; ++j) dist[i][j] = 99999;
    std::queue<std::pair<int, int>> q;
    q.push({ gx, gy }); dist[gy][gx] = 0;
    int dx[] = { 0, 0, -1, 1 }; int dy[] = { -1, 1, 0, 0 };

    while (!q.empty()) {
        auto curr = q.front(); q.pop();
        for (int i = 0; i < 4; ++i) {
            int nx = curr.first + dx[i], ny = curr.second + dy[i];
            if (nx >= 0 && nx < 15 && ny >= 0 && ny < 15) {
                TileType t = state.getMap().getTileAt(nx, ny);
                if (t != TileType::MOUNTAIN && t != TileType::RIVER && !blocked[ny][nx] && dist[ny][nx] > dist[curr.second][curr.first] + 1) {
                    dist[ny][nx] = dist[curr.second][curr.first] + 1;
                    q.push({ nx, ny });
                }
            }
        }
    }
    int bestD = 99999; std::pair<int, int> bestStep = { -1, -1 };
    for (int i = 0; i < 4; ++i) {
        int nx = sx + dx[i], ny = sy + dy[i];
        if (nx >= 0 && nx < 15 && ny >= 0 && ny < 15) {
            TileType t = state.getMap().getTileAt(nx, ny);
            if (t != TileType::MOUNTAIN && t != TileType::RIVER && !blocked[ny][nx] && dist[ny][nx] < bestD) {
                bestD = dist[ny][nx]; bestStep = { nx, ny };
            }
        }
    }
    return bestStep;
}

// =======================================================
// [적군 AI 턴 처리 로직 - 오버플로우 방지 유지]
// =======================================================
void EnemyManager::processAITurn(std::vector<Character*>& enemies, GameState& state) {
    int TWX = 7, TWY = 14;

    std::vector<std::pair<int, int>> epos(enemies.size(), { -1, -1 });
    for (int i = 0; i < (int)enemies.size(); ++i) {
        if (enemies[i]->isAlive()) {
            epos[i] = { enemies[i]->x, enemies[i]->y };
        }
    }

    for (int ei = 0; ei < (int)enemies.size(); ++ei) {
        Character* e = enemies[ei];
        if (!e->isAlive()) continue;

        if (e->cls == CharacterClass::BOSS_DRAGON) {
            DragonBoss* boss = dynamic_cast<DragonBoss*>(e);
            if (boss) boss->processBossTurn(state);
            continue;
        }

        // 엘리트 몬스터 스킬 턴 처리
        if (e->cls == CharacterClass::ELITE_LICH_KING || e->cls == CharacterClass::ELITE_DEMON_KING) {
            bool usedSkill = false;
            if (e->cls == CharacterClass::ELITE_LICH_KING)
                usedSkill = dynamic_cast<LichKing*>(e)->processEliteTurn(state);
            else if (e->cls == CharacterClass::ELITE_DEMON_KING)
                usedSkill = dynamic_cast<DemonKing*>(e)->processEliteTurn(state);

            if (usedSkill) {
                e->tauntedBy = nullptr;
                continue;
            }
        }

        // 상태이상 기절/감전/유혹 체크
        if (e->isStunned || e->isShocked || e->charmedBy != nullptr) {
            e->isStunned = false;
            e->isShocked = false;
            e->tauntedBy = nullptr;
            continue;
        }

        // [패시브] 악마 - 오만
        if (e->cls == CharacterClass::ENEMY_DEMON) {
            Demon* demon = dynamic_cast<Demon*>(e);
            demon->demonTurnCounter++;
            if (demon->demonTurnCounter % 2 != 0) {
                bool charmed = false;
                for (auto a : state.getAllies()) {
                    // 유혹 사거리 체크 (3칸)
                    if (a->isAlive() && state.manhattanDist(e->x, e->y, a->x, a->y) <= 3) {
                        // applyCharm에 demon(자신)을 전달
                        if (a->applyCharm(demon)) {
                            state.setLastMessage(demon->name + " CHARMED " + a->name + "!");
                            charmed = true;
                        }
                    }
                }
                if (charmed) {
                    playHitSfx();
                }
                // 유혹을 시도했다면 이동/공격을 하지 않고 턴 종료
                continue;
                }
            }

        int destX = TWX, destY = TWY;
        bool acted = false;

        // [패시브] 오크 - 파괴의지
        if (e->cls == CharacterClass::ENEMY_ORC) {
            if (state.manhattanDist(e->x, e->y, TWX, TWY) == 1) {
                state.damageTower(e->atk);
                playHitSfx();
                state.setLastMessage(e->name + " brutally attacked the Base!");
                acted = true;
            }
            destX = TWX; destY = TWY;
        }
        else {
            // [일반 공격 페이즈]
            for (auto a : state.getAllies()) {
                if (a->isAlive() && state.manhattanDist(e->x, e->y, a->x, a->y) == 1) {
                    if (e->tauntedBy && e->tauntedBy != a) continue;

                    int finalAtk = e->atk;
                    // [패시브] 언데드 - 크리티컬
                    if (e->cls == CharacterClass::ENEMY_UNDEAD && (rand() % 100 < 25)) {
                        finalAtk *= 2;
                        state.setLastMessage("CRITICAL! " + e->name + " fearlessly struck " + a->name + "!");
                    }
                    else {
                        state.setLastMessage(e->name + " attacked " + a->name + "!");
                    }

                    a->takeDamage(finalAtk);
                    playHitSfx();
                    acted = true;
                    break;
                }
            }

            if (!acted && !e->tauntedBy && state.manhattanDist(e->x, e->y, TWX, TWY) == 1) {
                state.damageTower(e->atk);
                playHitSfx();
                state.setLastMessage(e->name + " attacked the Base!");
                acted = true;
            }

            if (acted) {
                e->tauntedBy = nullptr;
                continue;
            }

            // [타겟 선정 페이즈]
            if (e->tauntedBy && e->tauntedBy->isAlive()) {
                destX = e->tauntedBy->x;
                destY = e->tauntedBy->y;
            }
            // [패시브] 고블린 - 교활
            else if (e->cls == CharacterClass::ENEMY_GOBLIN) {
                int bestDist = 9999;
                for (auto a : state.getAllies()) {
                    if (a->isAlive() && (a->cls == CharacterClass::PRIEST || a->cls == CharacterClass::MAGE)) {
                        int d = state.manhattanDist(e->x, e->y, a->x, a->y);
                        if (d < bestDist) { bestDist = d; destX = a->x; destY = a->y; }
                    }
                }
                if (bestDist == 9999) {
                    for (auto a : state.getAllies()) {
                        if (a->isAlive()) {
                            int d = state.manhattanDist(e->x, e->y, a->x, a->y);
                            if (d < bestDist) { bestDist = d; destX = a->x; destY = a->y; }
                        }
                    }
                }
            }
            else {
                int bestDist = 9999;
                for (auto a : state.getAllies()) {
                    if (a->isAlive()) {
                        int d = state.manhattanDist(e->x, e->y, a->x, a->y);
                        if (d < bestDist) { bestDist = d; destX = a->x; destY = a->y; }
                    }
                }
            }
        }

        // [이동 페이즈]
        std::vector<std::pair<int, int> > otherPos;
        for (int j = 0; j < (int)epos.size(); ++j) {
            if (j != ei && epos[j].first != -1) {
                otherPos.push_back(epos[j]);
            }
        }

        std::pair<int, int> nxt = bfsNext(e->x, e->y, destX, destY, state, otherPos);
        if (nxt.first != -1) {
            e->x = nxt.first;
            e->y = nxt.second;
            epos[ei] = nxt;
        }

        e->tauntedBy = nullptr;
    }
}