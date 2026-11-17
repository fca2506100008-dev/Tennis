#pragma once
#include "Character.h"
#include "Ball.h"

class Player : public Character {
private:
    int effectAlpha = 0;
    float effectRadius = 0.0f;
    bool allowHoldShot = false; // 長押しショット許可フラグ（false: クリック必須 / true: 長押し可）

public:
    Player(Vector3 startPos);

    void Update(Ball& ball, float speedFactor);
    void DrawReachArea() const;
    void DrawImpactEffect() const;

    // 長押しショット設定の切り替え・取得用関数
    void SetAllowHoldShot(bool allow) { allowHoldShot = allow; }
    bool GetAllowHoldShot() const { return allowHoldShot; }
};