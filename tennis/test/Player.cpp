#include "Player.h"
#include "DxLib.h"
#include <cmath>

// コンストラクタ
Player::Player(Vector3 startPos)
    : position(startPos), reachRadius(45.0f) {
}

// 更新処理（キー移動 ＆ スイング・打球判定）
void Player::Update(Ball& ball, float speedFactor) {
    // 速度倍率を適用した移動速度
    float moveSpeed = 3.5f * speedFactor;

    // 矢印キーでプレイヤー移動
    if (CheckHitKey(KEY_INPUT_LEFT))  position.x -= moveSpeed;
    if (CheckHitKey(KEY_INPUT_RIGHT)) position.x += moveSpeed;
    if (CheckHitKey(KEY_INPUT_UP))    position.z += moveSpeed;
    if (CheckHitKey(KEY_INPUT_DOWN))  position.z -= moveSpeed;

    // 移動範囲の制限（コート手前エリア）
    if (position.x < -120.0f) position.x = -120.0f;
    if (position.x > 120.0f) position.x = 120.0f;
    if (position.z < -300.0f) position.z = -300.0f;
    if (position.z > -20.0f) position.z = -20.0f;

    // ボールが飛行中（isMoving == true）のみ打球判定を行う
    if (!ball.isMoving) return;

    // SPACEキー または Zキーでショット
    bool shotKey = (CheckHitKey(KEY_INPUT_SPACE) != 0) || (CheckHitKey(KEY_INPUT_Z) != 0);

    if (shotKey) {
        // プレイヤーとボールの水平距離を計算
        float dx = ball.position.x - position.x;
        float dz = ball.position.z - position.z;
        float distSq = dx * dx + dz * dz;

        // ボールが自分に向かって飛んできており、打球エリア内＆一定の高さ以下にある場合
        if (ball.velocity.z < 0.0f && distSq <= reachRadius * reachRadius && ball.position.y <= 50.0f) {
            // 相手コート奥（Z = 220.0f 付近）へ向けて打ち返す
            float targetZ = 220.0f;
            float distanceZ = targetZ - position.z;

            // 左右キーの入力方向によって打ち分け（コース打ち）
            float dirX = 0.0f;
            if (CheckHitKey(KEY_INPUT_LEFT))  dirX = -2.5f;
            if (CheckHitKey(KEY_INPUT_RIGHT)) dirX = 2.5f;

            ball.velocity.z = distanceZ * 0.024f;            // 奥方向への推進力
            ball.velocity.x = (dx * 0.02f) + dirX;           // 左右コース
            ball.velocity.y = 4.8f + (distanceZ * 0.002f);   // 弧を描く高さ

            // 打ち返したためバウンド数と着地フラグをクリア
            ball.bounceCount = 0;
            ball.hasLanded = false;
        }
    }
}

// プレイヤー描画（人型キャラクター）
void Player::Draw() const {
    unsigned int bodyColor = GetColor(50, 120, 255); // 青のウエア
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

// 足元の打球可能エリア（円）の描画
void Player::DrawReachArea() const {
    const int DIV = 24;
    VERTEX3D vertex[DIV];
    WORD index[(DIV - 2) * 3];

    for (int i = 0; i < DIV; i++) {
        float angle = 2.0f * 3.14159265f * i / DIV;
        vertex[i].pos = VGet(
            position.x + cosf(angle) * reachRadius,
            0.15f, // 地面直上
            position.z + sinf(angle) * reachRadius
        );
        vertex[i].norm = VGet(0.0f, 1.0f, 0.0f);
        vertex[i].dif = GetColorU8(0, 200, 255, 128);
        vertex[i].spc = GetColorU8(0, 0, 0, 0);
        vertex[i].u = 0.0f; vertex[i].v = 0.0f;
        vertex[i].su = 0.0f; vertex[i].sv = 0.0f;
    }

    int idx = 0;
    for (int i = 1; i < DIV - 1; i++) {
        index[idx++] = 0;
        index[idx++] = i;
        index[idx++] = i + 1;
    }

    DrawPolygonIndexed3D(vertex, DIV, index, DIV - 2, DX_NONE_GRAPH, FALSE);
}