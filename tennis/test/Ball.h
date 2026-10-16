#pragma once
#include "DxLib.h"
#include "Vector3.h"

class Ball {
public:
    Vector3 position;
    Vector3 velocity;
    int bounceCount;
    Vector3 landingPos;
    bool hasLanded;
    bool isMoving;
    bool isTossing; // トス中フラグ

    Ball(Vector3 pos, Vector3 vel);
    void StartToss(Vector3 pos);                   // トス開始
    void Launch(Vector3 pos, Vector3 vel);         // サーブ・ショット発射
    void Reset(Vector3 pos);                       // 位置リセット
    void Update(float speedFactor = 1.0f);          // 物理・座標更新（速度倍率対応）
    void Draw() const;                             // ボール描画
    void DrawShadow() const;                       // 影描画
    void DrawLandingPoint() const;                 // 着地点マーカー描画
    void DrawPredictMarker() const;                // 落下予測線描画
};