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
#define SUBMIT 4

// 공통 함수
void gotoxy(int x, int y);
int  keyControl();
void init();

//	시작 메뉴 상수
const int MENU_X = 24;
const int MENU_Y = 12;
const int MENU_BRACKET = MENU_X + 13;

//	시작 메뉴 함수
void printOutlineStampede();
void drawMenuPoint(int index, bool selected);
int  startMenuDraw();
int  startPage();

//	세이브 메뉴 상수
const int SLOT_X = 5;
const int SLOT_START_Y = 8;
const int SLOT_GAP = 8;
const int SLOT_COUNT = 3;

// 세이브 메뉴 함수
void drawslot(int index, bool selected);
int  saveMenuDraw();
int  savePage();