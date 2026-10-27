#ifndef FPS_MANAGER_H
#define FPS_MANAGER_H

#include "DxLib.h"

class FpsManager {
private:
    int mStartTime;            // 処理開始時間
    int mCount;                // カウンタ
    float mFps;                // 計算した現在のFPS
    const int TARGET_FPS = 120; // 目標FPS

public:
    FpsManager() : mStartTime(0), mCount(0), mFps(0.0f) {}

    // フレーム開始時に呼ぶ
    void Start() {
        if (mCount == 0) {
            mStartTime = GetNowCount();
        }
    }

    // フレーム終了時に呼ぶ（60FPSに同期させるようにウェイトを入れる）
    void Wait() {
        int tookTime = GetNowCount() - mStartTime;
        int waitTime = mCount * 1000 / TARGET_FPS - tookTime;

        if (waitTime > 0) {
            Sleep(waitTime);
        }

        mCount++;
        if (mCount == TARGET_FPS) {
            int now = GetNowCount();
            mFps = 1000.0f / ((now - mStartTime) / (float)TARGET_FPS);
            mCount = 0;
            mStartTime = now;
        }
    }

    float GetFps() const { return mFps; }
};

#endif // FPS_MANAGER_H