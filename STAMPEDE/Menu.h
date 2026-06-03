#pragma once

#include <cstdio>
#include <iostream>
#include <windows.h>
#include <conio.h>
#include <vector>
#include <string>

#define UP     0
#define DOWN   1
#define LEFT   2
#define RIGHT  3
#define SUBMIT     4
#define BACK       5
#define DELETE_KEY 6

// Common functions
void gotoxy(int x, int y);
int  keyControl();
void init();

// Sound effect functions
void sfxMove();    // Cursor move
void sfxSubmit();  // Confirm (Enter)
void sfxBack();    // Back (Backspace)
void sfxDelete();  // Delete (Delete key)

//	Start menu constants
const int MENU_X = 24;
const int MENU_Y = 12;
const int MENU_BRACKET = MENU_X + 13;

//	Start menu functions
void printOutlineStampede();
void drawMenuPoint(int index, bool selected);
int  startMenuDraw();
int  startPage();

//	Save menu constants
const int SLOT_X = 5;
const int SLOT_START_Y = 8;
const int SLOT_GAP = 8;
const int SLOT_COUNT = 3;

// Save menu functions
void drawslot(int index, bool selected, int mode);
int  saveMenuDraw(int mode);
int  savePage();   // For new game (no delete)
int  loadPage();   // For continue (with delete)
