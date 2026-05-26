#include "GameMap.h"
#include "GameState.h"
#include "Boss.h"
#include <iostream>
#include <sstream>
#include <set>
#include <random>
#include <algorithm>
#include <queue>
#include <string>

GameMap::GameMap(int w, int h) : width(w), height(h) {
    grid.assign(height, std::vector<TileType>(width, TileType::EMPTY));
    grid[height - 1][width / 2] = TileType::BASE;
    initTerrain();
}

void GameMap::initTerrain() {
    int mt[] = { 3,3, 3,4, 4,4, 4,5, 4,6, 5,6, 6,7, 6,8 };
    for (int i = 0; i < 16; i += 2) setTile(mt[i], mt[i + 1], TileType::MOUNTAIN);
    int rv[] = { 10,2, 11,2, 12,3, 12,4, 13,5 };
    for (int i = 0; i < 10; i += 2) setTile(rv[i], rv[i + 1], TileType::RIVER);
}

std::string GameMap::getTileIcon(TileType type, char unitIcon) const {
    switch (type) {
    case TileType::EMPTY:    return " ";
    case TileType::BASE:     return "B";
    case TileType::ENEMY:    return "E";
    case TileType::MOUNTAIN: return "^";
    case TileType::RIVER:    return "~";
    default: return " ";
    }
}

std::vector<std::string> GameMap::renderLines(const GameState& state) const {
    std::vector<std::string> lines;
    std::ostringstream oss;

    auto movable = state.getMovablePositions();
    std::set<std::pair<int, int>> movableSet(movable.begin(), movable.end());

    oss << "    ";
    for (int x = 0; x < width; ++x) { char b[8]; snprintf(b, 8, "%2d  ", x); oss << b; }
    lines.push_back(oss.str()); oss.str(""); oss.clear();

    for (int y = 0; y < height; ++y) {
        oss << (y == 0 ? u8"   \u250c" : u8"   \u251c");
        for (int x = 0; x < width; ++x)
            oss << u8"\u2500\u2500\u2500" << (x == width - 1 ? (y == 0 ? u8"\u2510" : u8"\u2524") : (y == 0 ? u8"\u252c" : u8"\u253c"));
        lines.push_back(oss.str()); oss.str(""); oss.clear();

        char b[8]; snprintf(b, 8, "%2d ", y); oss << b << u8"\u2502";
        for (int x = 0; x < width; ++x) {
            TileType t = grid[y][x];
            char icon = ' ';

            // 1. 아군 렌더링
            if (t == TileType::HERO) {
                for (auto hero : state.getAllies()) {
                    if (hero->isAlive() && hero->x == x && hero->y == y) icon = hero->getIcon()[0];
                }
                oss << " " << icon << u8" \u2502";
            }
            // 2. 적군(보스 포함) 렌더링
            else if (t == TileType::ENEMY) {
                for (auto e : state.getEnemies()) {
                    if (e->isAlive() && e->isOccupying(x, y)) {
                        if (e->cls == CharacterClass::BOSS_DRAGON) {
                            auto boss = dynamic_cast<DragonBoss*>(e);
                            icon = (boss && boss->isWeakPointTile(x, y)) ? 'R' : 'D';
                        }
                        else {
                            icon = e->getIcon()[0];
                        }
                        break;
                    }
                }
                oss << " " << icon << u8" \u2502";
            }
            // 3. 지형 및 이동 가능 표시
            else if (t == TileType::EMPTY && movableSet.count({ x, y })) {
                oss << u8" * \u2502";
            }
            else {
                oss << " " << getTileIcon(t, ' ') << u8" \u2502";
            }
        }
        lines.push_back(oss.str()); oss.str(""); oss.clear();
    }

    oss << u8"   \u2514";
    for (int x = 0; x < width; ++x) oss << u8"\u2500\u2500\u2500" << (x == width - 1 ? u8"\u2518" : u8"\u2534");
    lines.push_back(oss.str());
    return lines;
}

// [기능 추가] 절차적 맵 생성 및 BFS 검증
void GameMap::randomizeTerrain(int wave, const std::vector<std::pair<int, int>>& occupied) {
    // 기존 랜덤 지형 초기화 생략 및 구현 (생략된 부분은 위 코드의 로직 그대로 사용 가능)
    // ... (이 부분은 제공하신 첫 번째 코드의 randomizeTerrain 로직을 그대로 붙여넣으시면 됩니다)
}

// [기능 추가] 맵 저장 및 불러오기
void GameMap::saveTerrain(std::ostream& out) const {
    out << "TERRAIN\n";
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (grid[y][x] == TileType::MOUNTAIN || grid[y][x] == TileType::RIVER)
                out << x << " " << y << " " << static_cast<int>(grid[y][x]) << "\n";
        }
    }
    out << "END_TERRAIN\n";
}

void GameMap::loadTerrain(std::istream& in) {
    // ... (제공하신 첫 번째 코드의 loadTerrain 로직을 그대로 사용)
}

void GameMap::setTile(int x, int y, TileType type) { if (x >= 0 && x < width && y >= 0 && y < height) grid[y][x] = type; }
TileType GameMap::getTileAt(int x, int y) const { return (x >= 0 && x < width && y >= 0 && y < height) ? grid[y][x] : TileType::EMPTY; }