#pragma once
#include "DxLib.h"

enum class SE {
    HIT,     // 打球音
    BOUNCE,  // バウンド音
    POINT,   // 得点音
    OUT      // アウト音
};

class SoundManager {
private:
    int handles[4];
    SoundManager(); // シングルトンパターン

public:
    static SoundManager& GetInstance();
    void Init();
    void Play(SE se);
};