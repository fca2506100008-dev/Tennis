#include "Ball.h"
#include "SoundManager.h"
#include <cmath>

Ball::Ball(Vector3 pos, Vector3 vel)
    : position(pos), velocity(vel), bounceCount(0), landingPos({ 0.0f, 0.0f, 0.0f }),
    hasLanded(false), isMoving(false), isTossing(false), currentShotType(ShotType::FLAT) {
}

void Ball::StartToss(Vector3 pos) {
    position = pos;
    velocity = { 0.0f, 6.0f, 0.0f };
    bounceCount = 0;
    hasLanded = false;
    isMoving = false;
    isTossing = true;
}

void Ball::Launch(Vector3 pos, Vector3 vel, ShotType shotType) {
    position = pos;
    velocity = vel;
    currentShotType = shotType;
    bounceCount = 0;
    hasLanded = false;
    landingPos = { 0.0f, 0.0f, 0.0f };
    isMoving = true;
    isTossing = false;

    // SE再生（打球時）
    SoundManager::GetInstance().Play(SE::HIT);
}

void Ball::Reset(Vector3 pos) {
    position = pos;
    velocity = { 0.0f, 0.0f, 0.0f };
    bounceCount = 0;
    hasLanded = false;
    landingPos = { 0.0f, 0.0f, 0.0f };
    isMoving = false;
    isTossing = false;
}

void Ball::Update(float speedFactor) {
    if (!isMoving && !isTossing) return;

    position.x += velocity.x * speedFactor;
    position.y += velocity.y * speedFactor;
    position.z += velocity.z * speedFactor;

    // 球種設定に応じた重力の適用
    ShotConfig config = GetShotConfig(currentShotType);
    velocity.y -= config.gravity * speedFactor;

    if (position.y <= 0.0f && velocity.y < 0.0f) {
        position.y = 0.0f;

        if (isTossing) {
            Reset(position);
            return;
        }

        if (!hasLanded) {
            landingPos = position;
            hasLanded = true;
        }

        // バウンド音の再生
        //SoundManager::GetInstance().Play(SE::BOUNCE);

        // 球種設定に応じた反発計算
        velocity.y = -velocity.y * config.bounceYRatio;
        velocity.x *= 0.85f;
        velocity.z *= config.bounceZRatio;

        bounceCount++;
    }
}

void Ball::Draw() const {
    DrawSphere3D(VGet(position.x, position.y + 4.0f, position.z), 4.0f, 10, GetColor(240, 240, 0), GetColor(255, 255, 255), TRUE);
}

void Ball::DrawShadow() const {
    const int DIV = 12;
    float radius = 5.0f;
    VERTEX3D vertex[DIV];
    WORD index[(DIV - 2) * 3];

    for (int i = 0; i < DIV; i++) {
        float angle = 2.0f * 3.14159265f * i / DIV;
        vertex[i].pos = VGet(position.x + cosf(angle) * radius, 0.1f, position.z + sinf(angle) * radius);
        vertex[i].norm = VGet(0.0f, 1.0f, 0.0f);
        vertex[i].dif = GetColorU8(30, 30, 30, 255);
        vertex[i].spc = GetColorU8(0, 0, 0, 0);
        vertex[i].u = 0.0f; vertex[i].v = 0.0f;
        vertex[i].su = 0.0f; vertex[i].sv = 0.0f;
    }

    int idx = 0;
    for (int i = 1; i < DIV - 1; i++) {
        index[idx++] = 0; index[idx++] = i; index[idx++] = i + 1;
    }

    DrawPolygonIndexed3D(vertex, DIV, index, DIV - 2, DX_NONE_GRAPH, FALSE);
}

void Ball::DrawLandingPoint() const {
    if (!hasLanded) return;

    const int DIV = 16;
    float radius = 8.0f;
    VERTEX3D vertex[DIV];
    WORD index[(DIV - 2) * 3];

    for (int i = 0; i < DIV; i++) {
        float angle = 2.0f * 3.14159265f * i / DIV;
        vertex[i].pos = VGet(landingPos.x + cosf(angle) * radius, 0.5f, landingPos.z + sinf(angle) * radius);
        vertex[i].norm = VGet(0.0f, 1.0f, 0.0f);
        vertex[i].dif = GetColorU8(255, 0, 0, 255);
        vertex[i].spc = GetColorU8(0, 0, 0, 0);
        vertex[i].u = 0.0f; vertex[i].v = 0.0f;
        vertex[i].su = 0.0f; vertex[i].sv = 0.0f;
    }

    int idx = 0;
    for (int i = 1; i < DIV - 1; i++) {
        index[idx++] = 0; index[idx++] = i; index[idx++] = i + 1;
    }

    DrawPolygonIndexed3D(vertex, DIV, index, DIV - 2, DX_NONE_GRAPH, FALSE);
}

void Ball::DrawPredictMarker() const {
    if (!isMoving || position.y <= 0.0f) return;

    Vector3 simPos = position;
    Vector3 simVel = velocity;
    ShotConfig config = GetShotConfig(currentShotType);

    for (int step = 0; step < 100; step++) {
        simPos.x += simVel.x;
        simPos.y += simVel.y;
        simPos.z += simVel.z;
        simVel.y -= config.gravity;

        if (simPos.y <= 0.0f) {
            simPos.y = 0.2f;
            DrawLine3D(VGet(position.x, position.y, position.z), VGet(simPos.x, 0.2f, simPos.z), GetColor(255, 200, 0));
            break;
        }
    }
}