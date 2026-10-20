#pragma once
#include "DxLib.h"
#include "Vector3.h"
#include "Ball.h"

class Character {
public:
    Vector3 position;
    float reachRadius;
    unsigned int bodyColor;

    Character(Vector3 startPos, unsigned int color)
        : position(startPos), reachRadius(45.0f), bodyColor(color) {
    }

    virtual ~Character() = default;
    virtual void Update(Ball& ball, float speedFactor = 1.0f) = 0;

    // プレイヤー・敵共通の人型3Dモデル描画
    void Draw() const {
        unsigned int skinColor = GetColor(255, 220, 180);
        unsigned int specular = GetColor(255, 255, 255);

        // 1. 頭
        DrawSphere3D(VGet(position.x, position.y + 22.0f, position.z), 4.0f, 10, skinColor, specular, TRUE);
        // 2. 胴体
        DrawCapsule3D(VGet(position.x, position.y + 12.0f, position.z), VGet(position.x, position.y + 18.0f, position.z), 4.5f, 10, bodyColor, specular, TRUE);
        // 3. 脚（左右）
        DrawCapsule3D(VGet(position.x - 2.0f, position.y + 1.0f, position.z), VGet(position.x - 2.0f, position.y + 11.0f, position.z), 2.0f, 8, bodyColor, specular, TRUE);
        DrawCapsule3D(VGet(position.x + 2.0f, position.y + 1.0f, position.z), VGet(position.x + 2.0f, position.y + 11.0f, position.z), 2.0f, 8, bodyColor, specular, TRUE);
        // 4. 腕（左右）
        DrawCapsule3D(VGet(position.x - 5.0f, position.y + 10.0f, position.z), VGet(position.x - 5.0f, position.y + 17.0f, position.z), 1.8f, 8, skinColor, specular, TRUE);
        DrawCapsule3D(VGet(position.x + 5.0f, position.y + 10.0f, position.z), VGet(position.x + 5.0f, position.y + 17.0f, position.z), 1.8f, 8, skinColor, specular, TRUE);
    }
};