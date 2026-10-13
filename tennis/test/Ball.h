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

    Ball(Vector3 pos, Vector3 vel);
    void Launch(Vector3 pos, Vector3 vel);
    void Reset(Vector3 pos);
    void Update(float speedFactor = 1.0f); // Åö speedFactor à¯êîÇí«â¡
    void Draw() const;
    void DrawShadow() const;
    void DrawLandingPoint() const;
    void DrawPredictMarker() const;
};