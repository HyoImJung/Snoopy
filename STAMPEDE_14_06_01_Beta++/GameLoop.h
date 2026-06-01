#pragma once
#include "GameMap.h"
#include "GameState.h"

// Start new game (slot used for saving)
void runNewGame(int saveSlot);

// Load saved game
void runLoadGame(int saveSlot);
