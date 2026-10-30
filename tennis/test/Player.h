#pragma once
#include "Character.h"
#include "Ball.h" // Ball& ball を参照するために追加

class Player : public Character {
public:
    // ★ 演出用：インパクトリングのパラメータ（挙動には一切影響しません）
    float effectRadius;
    float effectAlpha;

    Player(Vector3 startPos);
    void Update(Ball& ball, float speedFactor = 1.0f) override;
    void DrawReachArea() const;

    // ★ 演出用：打球インパクトエフェクトの描画関数
    void DrawImpactEffect() const;
};