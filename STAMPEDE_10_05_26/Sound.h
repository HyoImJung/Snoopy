#pragma once
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

void soundInit();
void stopWav();
void playWav(const char* filename, bool loop);
void playSfx(const char* filename);
void playHitSfx();
void playExplodeSfx();
void setBgmVolume(float vol);
void setSfxVolume(float vol);
extern float g_bgmVol;
extern float g_sfxVol;
