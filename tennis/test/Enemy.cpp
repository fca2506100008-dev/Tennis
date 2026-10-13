#include "Enemy.h"
#include "DxLib.h"
#include <cmath>

// コンストラクタ
Enemy::Enemy(Vector3 startPos)
    : position(startPos), speed(2.2f) {
}

// 更新処理（思考・ポジショニング・ショット判定）
void Enemy::Update(Ball& ball, float speedFactor) {
    // サーブ前（ボールが静止中）は敵を動かさない
    if (!ball.isMoving) {
        return;
    }

    // 速度倍率を適用した移動速度
    float currentSpeed = speed * speedFactor;

    // ----------------------------------------------------
    // 1. 移動・ポジショニング処理
    // ----------------------------------------------------
    // ボールがCPU側（Z > 0）に向かって飛んできている場合、ボールの位置へ移動
    if (ball.velocity.z > 0.0f) {
        // 左右（X軸）の追従
        if (position.x < ball.position.x - 2.0f) position.x += currentSpeed;
        if (position.x > ball.position.x + 2.0f) position.x -= currentSpeed;

        // 前後（Z軸）の追従（ボールのZ座標に合わせて適切な深さへ移動）
        float targetZ = ball.position.z + 15.0f; // 少し後ろに構える
        if (targetZ > 240.0f) targetZ = 240.0f;  // コート奥の範囲制限
        if (targetZ < 40.0f)  targetZ = 40.0f;   // ネット前に出すぎない制限

        if (position.z < targetZ - 3.0f) position.z += currentSpeed;
        if (position.z > targetZ + 3.0f) position.z -= currentSpeed;
    }
    else {
        // 自分が打った後はコート中央奥（Z = 220.0f, X = 0.0f）の基本位置へ戻る
        if (position.x < -2.0f)  position.x += currentSpeed * 0.5f;
        if (position.x > 2.0f)  position.x -= currentSpeed * 0.5f;
        if (position.z < 220.0f) position.z += currentSpeed * 0.5f;
        if (position.z > 220.0f) position.z -= currentSpeed * 0.5f;
    }

    // ----------------------------------------------------
    // 2. 自動ショット・返球処理
    // ----------------------------------------------------
    // ボールとCPUの水平距離計算
    float dx = ball.position.x - position.x;
    float dz = ball.position.z - position.z;
    float distSq = dx * dx + dz * dz;

    // ボールが自分に向かってきており、かつ打球範囲内（距離約35以内・高さ40以下）に入ったら打ち返す
    if (ball.velocity.z > 0.0f && distSq < 1200.0f && ball.position.y < 40.0f) {
        // プレイヤー側（Z = -240.0f 付近）へ向けて打ち返す
        float targetZ = -220.0f;
        float distanceZ = position.z - targetZ;

        ball.velocity.z = -distanceZ * 0.024f;          // 手前方向へ
        ball.velocity.x = -dx * 0.03f;                 // プレイヤーの逆を突くように打ち分け
        ball.velocity.y = 4.5f + (distanceZ * 0.002f); // 弧を描く高さ

        // 打ち返したのでバウンド数と着地フラグをクリア
        ball.bounceCount = 0;
        ball.hasLanded = false;
    }
}

// 敵描画（人型キャラクター）
void Enemy::Draw() const {
    unsigned int bodyColor = GetColor(255, 60, 60);  // 赤のウエア
    unsigned int skinColor = GetColor(255, 220, 180); // 肌色
    unsigned int specular = GetColor(255, 255, 255);

    // 1. 頭（球体）
    DrawSphere3D(
        VGet(position.x, position.y + 22.0f, position.z),
        4.0f, 10, skinColor, specular, TRUE
    );

    // 2. 胴体（カプセル：胸〜腰）
    DrawCapsule3D(
        VGet(position.x, position.y + 12.0f, position.z),
        VGet(position.x, position.y + 18.0f, position.z),
        4.5f, 10, bodyColor, specular, TRUE
    );

    // 3. 脚（左右2本）
    DrawCapsule3D(
        VGet(position.x - 2.0f, position.y + 1.0f, position.z),
        VGet(position.x - 2.0f, position.y + 11.0f, position.z),
        2.0f, 8, bodyColor, specular, TRUE
    );
    DrawCapsule3D(
        VGet(position.x + 2.0f, position.y + 1.0f, position.z),
        VGet(position.x + 2.0f, position.y + 11.0f, position.z),
        2.0f, 8, bodyColor, specular, TRUE
    );

    // 4. 腕（左右2本）
    DrawCapsule3D(
        VGet(position.x - 5.0f, position.y + 10.0f, position.z),
        VGet(position.x - 5.0f, position.y + 17.0f, position.z),
        1.8f, 8, skinColor, specular, TRUE
    );
    DrawCapsule3D(
        VGet(position.x + 5.0f, position.y + 10.0f, position.z),
        VGet(position.x + 5.0f, position.y + 17.0f, position.z),
        1.8f, 8, skinColor, specular, TRUE
    );
}