#include "Sound.h"
#include <cstring>
#include <cstdlib>
#include <stdio.h>
#include <process.h>

static char s_dir[MAX_PATH] = {};
float g_bgmVol = 1.0f;
float g_sfxVol = 0.8f;

void soundInit() {
    if (s_dir[0] != '\0') return;
    GetModuleFileNameA(NULL, s_dir, MAX_PATH);
    char* last = strrchr(s_dir, '\\');
    if (last) *(last + 1) = '\0';
}

void setBgmVolume(float vol) { g_bgmVol = vol < 0 ? 0 : vol > 1 ? 1 : vol; }
void setSfxVolume(float vol) { g_sfxVol = vol < 0 ? 0 : vol > 1 ? 1 : vol; }

void stopWav() { PlaySoundA(NULL, NULL, 0); }

void playWav(const char* filename, bool loop) {
    soundInit();
    char path[MAX_PATH];
    strcpy_s(path, s_dir);
    strcat_s(path, filename);
    DWORD flags = SND_FILENAME | SND_ASYNC | SND_NODEFAULT;
    if (loop) flags |= SND_LOOP;
    PlaySoundA(path, NULL, flags);
}

static DWORD readWav(const char* path, WAVEFORMATEX* wfx, BYTE** out) {
    HANDLE hf = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ,
                            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hf == INVALID_HANDLE_VALUE) return 0;
    DWORD fsz = GetFileSize(hf, NULL);
    BYTE* raw = (BYTE*)malloc(fsz);
    if (!raw) { CloseHandle(hf); return 0; }
    DWORD rd = 0; ReadFile(hf, raw, fsz, &rd, NULL); CloseHandle(hf);
    ZeroMemory(wfx, sizeof(WAVEFORMATEX));
    BYTE* pcm = NULL; DWORD pcmSz = 0;
    BYTE* p = raw+12, *e = raw+fsz;
    while (p+8 <= e) {
        DWORD ck=*(DWORD*)p, cksz=*(DWORD*)(p+4); p+=8;
        if (ck==0x20746d66u) {
            DWORD cp=cksz<sizeof(WAVEFORMATEX)?cksz:sizeof(WAVEFORMATEX);
            memcpy(wfx,p,cp); wfx->cbSize=0;
        } else if (ck==0x61746164u) { pcm=p; pcmSz=cksz; break; }
        p+=(cksz+1)&~1u;
    }
    if (!pcm||!pcmSz||!wfx->nSamplesPerSec) { free(raw); return 0; }
    *out=(BYTE*)malloc(pcmSz);
    if (!*out) { free(raw); return 0; }
    memcpy(*out,pcm,pcmSz);
    free(raw);
    return pcmSz;
}

static void scaleVol(BYTE* pcm, DWORD sz, float vol) {
    short* s=(short*)pcm; DWORD n=sz/2;
    for (DWORD i=0;i<n;i++) {
        int v=(int)(s[i]*vol);
        s[i]=(short)(v>32767?32767:v<-32768?-32768:v);
    }
}

struct SfxBuf { HWAVEOUT hwo; WAVEHDR hdr; BYTE* data; };

static unsigned __stdcall sfxClean(void* arg) {
    SfxBuf* b=(SfxBuf*)arg;
    while (!(b->hdr.dwFlags & WHDR_DONE)) Sleep(5);
    waveOutUnprepareHeader(b->hwo,&b->hdr,sizeof(WAVEHDR));
    waveOutClose(b->hwo);
    free(b->data); free(b);
    return 0;
}

void playSfx(const char* filename) {
    soundInit();
    char path[MAX_PATH]; strcpy_s(path,s_dir); strcat_s(path,filename);
    WAVEFORMATEX wfx; ZeroMemory(&wfx,sizeof(wfx));
    BYTE* pcm=NULL;
    DWORD sz=readWav(path,&wfx,&pcm);
    if (!sz) return;
    scaleVol(pcm,sz,g_sfxVol);
    HWAVEOUT hwo=NULL;
    if (waveOutOpen(&hwo,WAVE_MAPPER,&wfx,0,0,CALLBACK_NULL)!=MMSYSERR_NOERROR) {
        free(pcm); return;
    }
    SfxBuf* b=(SfxBuf*)malloc(sizeof(SfxBuf));
    if (!b) { waveOutClose(hwo); free(pcm); return; }
    b->hwo=hwo; b->data=pcm;
    ZeroMemory(&b->hdr,sizeof(WAVEHDR));
    b->hdr.lpData=(LPSTR)pcm; b->hdr.dwBufferLength=sz;
    waveOutPrepareHeader(hwo,&b->hdr,sizeof(WAVEHDR));
    waveOutWrite(hwo,&b->hdr,sizeof(WAVEHDR));
    HANDLE t=(HANDLE)_beginthreadex(NULL,0,sfxClean,b,0,NULL);
    if (t) CloseHandle(t);
}

void playHitSfx() {
    char n[32]; sprintf_s(n,"hit_%d.wav",(rand()%3)+1); playSfx(n);
}
void playExplodeSfx() {
    char n[32]; sprintf_s(n,"explode_%d.wav",(rand()%2)+1); playSfx(n);
}
