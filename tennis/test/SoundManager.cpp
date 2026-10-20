#include "SoundManager.h"

SoundManager::SoundManager() {
    for (int i = 0; i < 4; ++i) handles[i] = -1;
}

SoundManager& SoundManager::GetInstance() {
    static SoundManager instance;
    return instance;
}

void SoundManager::Init() {
    handles[(int)SE::HIT] = LoadSoundMem("soundhit.mp3");
    /*handles[(int)SE::BOUNCE] = LoadSoundMem("sound/bounce.wav");
    handles[(int)SE::POINT] = LoadSoundMem("sound/point.wav");
    handles[(int)SE::OUT] = LoadSoundMem("sound/out.wav");*/
}

void SoundManager::Play(SE se) {
    int idx = static_cast<int>(se);
    if (idx >= 0 && idx < 4 && handles[idx] != -1) {
        PlaySoundMem(handles[idx], DX_PLAYTYPE_BACK);
    }
}