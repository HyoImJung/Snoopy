#include "Enemy.h"
#include "GameState.h"
#include <string>
#include <queue>
#include <vector>
#include <utility>

Enemy::Enemy(std::string n, int px, int py, int h, int a)
    : Character(n, CharacterClass::ENEMY_ORC, px, py, h, a, 1, 1, false) {}

bool Enemy::performAttack(char dir, GameState& state) {
    return false;
}

void EnemyManager::spawnWave(int wave, std::vector<Character*>& enemies) {
    for (int i = 0; i < 6; ++i) {
        enemies.push_back(new Enemy(
            "Orc" + std::to_string(i + 1),
            i * 2, 0,
            45 + (wave * 5),
            4 + wave));
    }
}

static std::pair<int,int> bfsNext(
    int sx, int sy, int gx, int gy,
    const GameState& state,
    const std::vector<std::pair<int,int> >& others)
{
    bool blocked[15][15] = {};
    for (int i = 0; i < (int)others.size(); ++i) {
        int bx = others[i].first;
        int by = others[i].second;
        if (bx >= 0 && bx < 15 && by >= 0 && by < 15)
            blocked[by][bx] = true;
    }

    bool vis[15][15] = {};
    int  parX[15][15];
    int  parY[15][15];
    for (int y = 0; y < 15; ++y)
        for (int x = 0; x < 15; ++x)
            parX[y][x] = parY[y][x] = -1;

    std::queue<std::pair<int,int> > q;
    vis[sy][sx] = true;
    q.push(std::make_pair(sx, sy));

    const int dx4[4] = { 0,  0, 1, -1 };
    const int dy4[4] = { 1, -1, 0,  0 };

    bool found = false;
    while (!q.empty() && !found) {
        std::pair<int,int> cur = q.front(); q.pop();
        int cx = cur.first;
        int cy = cur.second;
        for (int d = 0; d < 4; ++d) {
            int nx = cx + dx4[d];
            int ny = cy + dy4[d];
            if (nx < 0 || nx >= 15 || ny < 0 || ny >= 15) continue;
            if (vis[ny][nx]) continue;
            bool isGoal = (nx == gx && ny == gy);
            if (!isGoal && blocked[ny][nx])               continue;
            if (!isGoal && !state.canEnemyMoveTo(nx, ny)) continue;
            vis[ny][nx]  = true;
            parX[ny][nx] = cx;
            parY[ny][nx] = cy;
            q.push(std::make_pair(nx, ny));
            if (isGoal) { found = true; break; }
        }
    }

    if (!found) return std::make_pair(-1, -1);

    int cx = gx, cy = gy;
    while (true) {
        int px = parX[cy][cx];
        int py = parY[cy][cx];
        if (px == sx && py == sy) return std::make_pair(cx, cy);
        if (px == -1)             return std::make_pair(-1, -1);
        cx = px;
        cy = py;
    }
}

void EnemyManager::processAITurn(
    std::vector<Character*>& enemies, GameState& state)
{
    const int TWX = 7;
    const int TWY = 14;

    std::vector<std::pair<int,int> > epos;
    for (int i = 0; i < (int)enemies.size(); ++i) {
        if (enemies[i]->isAlive())
            epos.push_back(std::make_pair(enemies[i]->x, enemies[i]->y));
        else
            epos.push_back(std::make_pair(-1, -1));
    }

    for (int ei = 0; ei < (int)enemies.size(); ++ei) {
        Character* e = enemies[ei];
        if (!e->isAlive()) continue;

        bool acted = false;

        for (int ai = 0; ai < (int)state.getAllies().size(); ++ai) {
            Character* a = state.getAllies()[ai];
            if (a->isAlive() &&
                state.manhattanDist(e->x, e->y, a->x, a->y) == 1) {
                a->hp -= e->atk;
                state.addHitEffect(a->x, a->y, e->atk);
                state.setLastMessage(e->name + " attacked " + a->name + "!");
                acted = true;
                break;
            }
        }

        if (!acted &&
            state.manhattanDist(e->x, e->y, TWX, TWY) == 1) {
            state.damageTower(e->atk);
            state.setLastMessage(e->name + " attacked the Base!");
            acted = true;
        }

        if (acted) continue;

        int destX = TWX;
        int destY = TWY;
        int best  = 99999;
        for (int ai = 0; ai < (int)state.getAllies().size(); ++ai) {
            Character* a = state.getAllies()[ai];
            if (!a->isAlive()) continue;
            int d = state.manhattanDist(e->x, e->y, a->x, a->y);
            if (d < best) {
                best  = d;
                destX = a->x;
                destY = a->y;
            }
        }

        std::vector<std::pair<int,int> > otherPos;
        for (int j = 0; j < (int)epos.size(); ++j) {
            if (j != ei) otherPos.push_back(epos[j]);
        }

        std::pair<int,int> nxt =
            bfsNext(e->x, e->y, destX, destY, state, otherPos);

        int nx = nxt.first;
        int ny = nxt.second;

        if (nx == -1)                       continue;
        if (nx == destX && ny == destY)     continue;
        if (!state.canEnemyMoveTo(nx, ny))  continue;

        epos[ei] = std::make_pair(nx, ny);
        e->x = nx;
        e->y = ny;
    }
}
