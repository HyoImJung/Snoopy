#pragma once

#include "GameState.h"
#include "GameMap.h"
#include <string>

std::string getSaveFilePath(int slot);
bool hasSaveFile(int slot);

// map을 직접 받아서 const_cast/getMap 우회 없이 saveTerrain 호출
bool saveGame(const GameState& state, const GameMap& map, int slot);
bool loadGame(GameState& state, GameMap& map, int slot);
