#include "Player.h"
#include "ShotData.h"
#include "DxLib.h"
#include <cmath>

Player::Player(Vector3 startPos)
    : Character(startPos, GetColor(50, 120, 255)) {
}

void Player::Update(Ball& ball, float speedFactor) {
    // SHIFTキー押下時のダッシュ制御（1.6倍）
    bool isShiftPressed = (CheckHitKey(KEY_INPUT_LSHIFT) != 0) || (CheckHitKey(KEY_INPUT_RSHIFT) != 0);
    float dashMultiplier = isShiftPressed ? 1.6f : 1.0f;
    float moveSpeed = 3.5f * dashMultiplier * speedFactor;

    // 自分がサーブを打つ前（ボールが未移動、かつ自分側のコートにいる）判定
    bool isMyServeWaiting = (!ball.isMoving && ball.position.z < 0.0f);

    // サーブ待機中のサイド記憶・ロック制御
    static bool serveFromRight = true;
    static bool isServingLastFrame = false;

    if (isMyServeWaiting) {
        // サーブ待機状態に入った最初の1フレームで、初期配置（position.x >= 0）から打つサイドを決定・ロック
        if (!isServingLastFrame) {
            serveFromRight = (position.x >= 0.0f);
        }

        // A / D キー（または十字キー左右）で左右移動
        if (CheckHitKey(KEY_INPUT_A) || CheckHitKey(KEY_INPUT_LEFT))  position.x -= moveSpeed;
        if (CheckHitKey(KEY_INPUT_D) || CheckHitKey(KEY_INPUT_RIGHT)) position.x += moveSpeed;

        // センターライン（x = 0.0f）跨ぎ防止ガード
        if (serveFromRight) {
            // 右サイド（デュースサイド）からのサーブ：センターラインより左（x < 0.0f）へ行けない
            if (position.x < 0.0f) position.x = 0.0f;
        }
        else {
            // 左サイド（アドバンテージサイド）からのサーブ：センターラインより右（x > 0.0f）へ行けない
            if (position.x > 0.0f) position.x = 0.0f;
        }
    }
    else {
        // ラリー中の左右移動
        if (CheckHitKey(KEY_INPUT_A) || CheckHitKey(KEY_INPUT_LEFT))  position.x -= moveSpeed;
        if (CheckHitKey(KEY_INPUT_D) || CheckHitKey(KEY_INPUT_RIGHT)) position.x += moveSpeed;

        // W / S キー（または十字キー上下）で前後移動（サーブ待機中は前後移動不可）
        if (CheckHitKey(KEY_INPUT_W) || CheckHitKey(KEY_INPUT_UP))   position.z += moveSpeed;
        if (CheckHitKey(KEY_INPUT_S) || CheckHitKey(KEY_INPUT_DOWN)) position.z -= moveSpeed;
    }

    // 前フレームのサーブ待機状態を更新
    isServingLastFrame = isMyServeWaiting;

    // プレイヤーの移動制限エリア（コート範囲内）
    if (position.x < -120.0f) position.x = -120.0f;
    if (position.x > 120.0f) position.x = 120.0f;
    if (position.z < -300.0f) position.z = -300.0f;
    if (position.z > -20.0f) position.z = -20.0f;

    if (!ball.isMoving) return;

    // マウス左クリックでショット実行（ラリー中）
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
            if (CheckHitKey(KEY_INPUT_A) || CheckHitKey(KEY_INPUT_LEFT))  dirX = -0.8f;
            if (CheckHitKey(KEY_INPUT_D) || CheckHitKey(KEY_INPUT_RIGHT)) dirX = 0.8f;

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

// LNK2019 未解決外部シンボル参照エラー対策（インパクトエフェクト描画関数の定義）
void Player::DrawImpactEffect() const {
    // ショットヒット時のエフェクト処理用（必要に応じて記述）
}