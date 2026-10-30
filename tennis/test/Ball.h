#pragma once
#include "DxLib.h"
#include "Vector3.h"
#include "ShotData.h"

class Ball {
public:
    Vector3 position;
    Vector3 velocity;
    int bounceCount;
    Vector3 landingPos;
    bool hasLanded;
    bool isMoving;
    bool isTossing;
    ShotType currentShotType;

    Ball(Vector3 pos, Vector3 vel);
    void StartToss(Vector3 pos);
    void Launch(Vector3 pos, Vector3 vel, ShotType shotType = ShotType::FLAT);
    void Reset(Vector3 pos);
    void Update(float speedFactor = 1.0f);
    void Draw() const;
    void DrawShadow() const;
    void DrawLandingPoint() const;
    void DrawPredictMarker() const;

    // ★ 演出用描画（物理演算・変数構造には影響を与えない追加関数）
    void DrawTimingRing() const;
};