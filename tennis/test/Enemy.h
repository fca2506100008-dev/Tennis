#pragma once
#include "DxLib.h"
#include "Vector3.h"
#include "Ball.h"

class Enemy {
public:
    Vector3 position;
    float speed;

    Enemy(Vector3 startPos);
    void Update(Ball& ball, float speedFactor = 1.0f);
    void Draw() const;
};