#include "Ball.h"
#include "DxLib.h"
#include <cmath>

// コンストラクタ
Ball::Ball(Vector3 pos, Vector3 vel)
    : position(pos), velocity(vel), bounceCount(0), landingPos({ 0.0f, 0.0f, 0.0f }),
    hasLanded(false), isMoving(false), isTossing(false) {
}

// ★ トス開始（真上にボールを投げ上げる）
void Ball::StartToss(Vector3 pos) {
    position = pos;
    velocity = { 0.0f, 6.0f, 0.0f }; // 上方向へ初速を与える
    bounceCount = 0;
    hasLanded = false;
    isMoving = false;
    isTossing = true;
}

// ★ サーブ・ショット発射
void Ball::Launch(Vector3 pos, Vector3 vel) {
    position = pos;
    velocity = vel;
    bounceCount = 0;
    hasLanded = false;
    landingPos = { 0.0f, 0.0f, 0.0f };
    isMoving = true;
    isTossing = false; // サーブ打球完了
}

// ★ ボール位置のリセット（得点後やトスやり直し時）
void Ball::Reset(Vector3 pos) {
    position = pos;
    velocity = { 0.0f, 0.0f, 0.0f };
    bounceCount = 0;
    hasLanded = false;
    landingPos = { 0.0f, 0.0f, 0.0f };
    isMoving = false;
    isTossing = false;
}

// ★ 物理・座標更新（速度倍率 speedFactor に対応）
void Ball::Update(float speedFactor) {
    // 静止中の場合は物理計算を行わない
    if (!isMoving && !isTossing) return;

    // 速度倍率を掛け合わせて位置を更新
    position.x += velocity.x * speedFactor;
    position.y += velocity.y * speedFactor;
    position.z += velocity.z * speedFactor;

    // 重力（速度倍率を適用）
    velocity.y -= 0.35f * speedFactor;

    // 地面（Y = 0.0f）への着地・バウンド処理
    if (position.y <= 0.0f && velocity.y < 0.0f) {
        position.y = 0.0f;

        // トス中に打たずに地面まで落ちてしまった場合（空振り・やり直し）
        if (isTossing) {
            Reset(position);
            return;
        }

        // 最初の着地点を記録（アウト・イン判定用）
        if (!hasLanded) {
            landingPos = position;
            hasLanded = true;
        }

        // 跳ね返り（Y速度反転 ＆ 摩擦による減速）
        velocity.y = -velocity.y * 0.7f;
        velocity.x *= 0.85f;
        velocity.z *= 0.85f;

        bounceCount++;
    }
}

// ボール本体の描画（黄色の球体）
void Ball::Draw() const {
    DrawSphere3D(
        VGet(position.x, position.y + 4.0f, position.z),
        4.0f, 10,
        GetColor(240, 240, 0), GetColor(255, 255, 255), TRUE
    );
}

// 地面に落ちる丸い影の描画
void Ball::DrawShadow() const {
    const int DIV = 12;
    float radius = 5.0f;
    VERTEX3D vertex[DIV];
    WORD index[(DIV - 2) * 3];

    for (int i = 0; i < DIV; i++) {
        float angle = 2.0f * 3.14159265f * i / DIV;
        vertex[i].pos = VGet(
            position.x + cosf(angle) * radius,
            0.1f, // 地面よりごくわずかに上
            position.z + sinf(angle) * radius
        );
        vertex[i].norm = VGet(0.0f, 1.0f, 0.0f);
        vertex[i].dif = GetColorU8(30, 30, 30, 255);
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

// ワンバウンドした位置を示す赤い円形マーカーの描画
void Ball::DrawLandingPoint() const {
    if (!hasLanded) return;

    const int DIV = 16;
    float radius = 8.0f;
    VERTEX3D vertex[DIV];
    WORD index[(DIV - 2) * 3];

    for (int i = 0; i < DIV; i++) {
        float angle = 2.0f * 3.14159265f * i / DIV;
        vertex[i].pos = VGet(
            landingPos.x + cosf(angle) * radius,
            0.5f,
            landingPos.z + sinf(angle) * radius
        );
        vertex[i].norm = VGet(0.0f, 1.0f, 0.0f);
        vertex[i].dif = GetColorU8(255, 0, 0, 255);
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

// 次にバウンドする予測位置を示す黄色の直線ガイドを描画
void Ball::DrawPredictMarker() const {
    if (!isMoving || position.y <= 0.0f) return;

    Vector3 simPos = position;
    Vector3 simVel = velocity;
    float gravity = 0.35f;

    // 未来の軌道を100ステップ簡易シミュレーション
    for (int step = 0; step < 100; step++) {
        simPos.x += simVel.x;
        simPos.y += simVel.y;
        simPos.z += simVel.z;
        simVel.y -= gravity;

        if (simPos.y <= 0.0f) {
            simPos.y = 0.2f;

            // 現在位置から予想バウンド地点までの直線を描画
            DrawLine3D(
                VGet(position.x, position.y, position.z),
                VGet(simPos.x, 0.2f, simPos.z),
                GetColor(255, 200, 0)
            );
            break;
        }
    }
}