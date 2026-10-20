#include "Player.h"
#include <cmath>

Player::Player(Vector3 startPos)
    : Character(startPos, GetColor(50, 120, 255)) {
} // 青ウェア

void Player::Update(Ball& ball, float speedFactor) {
    float moveSpeed = 3.5f * speedFactor;

    if (CheckHitKey(KEY_INPUT_LEFT))  position.x -= moveSpeed;
    if (CheckHitKey(KEY_INPUT_RIGHT)) position.x += moveSpeed;
    if (CheckHitKey(KEY_INPUT_UP))    position.z += moveSpeed;
    if (CheckHitKey(KEY_INPUT_DOWN))  position.z -= moveSpeed;

    if (position.x < -120.0f) position.x = -120.0f;
    if (position.x > 120.0f) position.x = 120.0f;
    if (position.z < -300.0f) position.z = -300.0f;
    if (position.z > -20.0f) position.z = -20.0f;

    if (!ball.isMoving) return;

    // キー操作による球種判定
    bool keyFlat = CheckHitKey(KEY_INPUT_SPACE) != 0;
    bool keySpin = CheckHitKey(KEY_INPUT_Z) != 0;
    bool keySlice = CheckHitKey(KEY_INPUT_X) != 0;

    if (keyFlat || keySpin || keySlice) {
        float dx = ball.position.x - position.x;
        float dz = ball.position.z - position.z;
        float distSq = dx * dx + dz * dz;

        if (ball.velocity.z < 0.0f && distSq <= reachRadius * reachRadius && ball.position.y <= 50.0f) {
            float targetZ = 220.0f;
            float distanceZ = targetZ - position.z;

            float dirX = 0.0f;
            if (CheckHitKey(KEY_INPUT_LEFT))  dirX = -2.5f;
            if (CheckHitKey(KEY_INPUT_RIGHT)) dirX = 2.5f;

            // 選択キーによる球種の決定
            ShotType shot = ShotType::FLAT;
            if (keySpin)  shot = ShotType::TOPSPIN;
            if (keySlice) shot = ShotType::SLICE;

            ShotConfig config = GetShotConfig(shot);

            float baseVelZ = distanceZ * 0.024f * config.velZFactor;
            float baseVelX = (dx * 0.02f) + dirX;
            float baseVelY = (4.8f + (distanceZ * 0.002f)) * config.velYFactor;

            ball.Launch(ball.position, { baseVelX, baseVelY, baseVelZ }, shot);
        }
    }
}

void Player::DrawReachArea() const {
    const int DIV = 24;
    VERTEX3D vertex[DIV];
    WORD index[(DIV - 2) * 3];

    for (int i = 0; i < DIV; i++) {
        float angle = 2.0f * 3.14159265f * i / DIV;
        vertex[i].pos = VGet(position.x + cosf(angle) * reachRadius, 0.15f, position.z + sinf(angle) * reachRadius);
        vertex[i].norm = VGet(0.0f, 1.0f, 0.0f);
        vertex[i].dif = GetColorU8(0, 200, 255, 128);
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