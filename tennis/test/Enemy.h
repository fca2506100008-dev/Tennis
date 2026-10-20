#pragma once
#include "Character.h"

// 難易度の定義
enum class Difficulty {
    EASY,
    NORMAL,
    HARD
};

class Enemy : public Character {
public:
    float speed;         // 移動速度
    float accuracy;      // コースの正確さ (0.0f: ランダム 〜 1.0f: 100%正確)
    int reactionDelay;   // 反応遅れフレーム数（打球判断の遅れ）
    Difficulty difficulty;

    Enemy(Vector3 startPos);

    // 難易度設定関数の追加
    void SetDifficulty(Difficulty diff);

    void Update(Ball& ball, float speedFactor = 1.0f) override;
};