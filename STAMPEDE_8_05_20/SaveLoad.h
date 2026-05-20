#pragma once

#include "GameState.h"
#include "GameMap.h"
#include <string>

// Returns save file path for slot index (0,1,2)
std::string getSaveFilePath(int slot);

// Check if save file exists for the slot
bool hasSaveFile(int slot);

// Save current game state to slot
bool saveGame(const GameState& state, int slot);

// Load game state from slot
bool loadGame(GameState& state, int slot);
