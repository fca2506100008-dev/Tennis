#include "Player.h"
#include "ShotData.h"
#include "DxLib.h"
#include <cmath>

Player::Player(Vector3 startPos)
    : Character(startPos, GetColor(50, 120, 255)) {
}

void Player::Update(Ball& ball, float speedFactor) {
    float moveSpeed = 3.5f * speedFactor;

    // 自分がサーブを打つ前（ボール停止中かつボールが自分側にある）かどうかの判定
    bool isMyServeWaiting = (!ball.isMoving && ball.position.z < 0.0f);

    // A / D キーで左右移動（サーブ前後を問わず可能）
    if (CheckHitKey(KEY_INPUT_A)) position.x -= moveSpeed;
    if (CheckHitKey(KEY_INPUT_D)) position.x += moveSpeed;

    // W / S キーで前後移動（自分のサーブ待機中のみ前後移動をロック）
    if (!isMyServeWaiting) {
        if (CheckHitKey(KEY_INPUT_W)) position.z += moveSpeed;
        if (CheckHitKey(KEY_INPUT_S)) position.z -= moveSpeed;
    }

    // プレイヤーの移動制限エリア
    if (position.x < -120.0f) position.x = -120.0f;
    if (position.x > 120.0f) position.x = 120.0f;
    if (position.z < -300.0f) position.z = -300.0f;
    if (position.z > -20.0f) position.z = -20.0f;

    if (!ball.isMoving) return;

    // マウス左クリックでショット実行
    bool mouseLeft = (GetMouseInput() & MOUSE_INPUT_LEFT) != 0;

    if (mouseLeft) {
        float dx = ball.position.x - position.x;
        float dz = ball.position.z - position.z;
        float distSq = dx * dx + dz * dz;

        if (ball.velocity.z < 0.0f && distSq <= reachRadius * reachRadius && ball.position.y <= 50.0f) {
            float targetZ = 220.0f;
            float distanceZ = targetZ - position.z;

            // A / D キーで狙う左右コースの補正
            float dirX = 0.0f;
            if (CheckHitKey(KEY_INPUT_A)) dirX = -0.8f;
            if (CheckHitKey(KEY_INPUT_D)) dirX = 0.8f;

            ShotType shot = ShotType::FLAT;
            ShotConfig config = GetShotConfig(shot);

            float baseVelZ = distanceZ * 0.024f * config.velZFactor;
            float baseVelY = (4.8f + (distanceZ * 0.002f)) * config.velYFactor;
            float baseVelX = (dx * 0.010f) + dirX;

            const float MAX_VEL_X = 1.4f;
            if (baseVelX > MAX_VEL_X) baseVelX = MAX_VEL_X;
            if (baseVelX < -MAX_VEL_X) baseVelX = -MAX_VEL_X;

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
        vertex[i].pos = VGet(
            position.x + cosf(angle) * reachRadius,
            0.15f,
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