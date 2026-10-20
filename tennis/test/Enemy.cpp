#include "Enemy.h"
#include "ShotData.h"
#include <cmath>
#include <cstdlib>

Enemy::Enemy(Vector3 startPos)
    : Character(startPos, GetColor(255, 60, 60)) {
    SetDifficulty(Difficulty::NORMAL); // デフォルトはNORMAL
}

// 難易度に応じてパラメータを一括変更
void Enemy::SetDifficulty(Difficulty diff) {
    difficulty = diff;
    switch (diff) {
    case Difficulty::EASY:
        speed = 1.4f;         // 足が遅い
        accuracy = 0.5f;      // 狙いが曖昧（中央寄りになりやすい）
        reactionDelay = 15;   // 反応がワンテンポ遅れる
        break;

    case Difficulty::NORMAL:
        speed = 2.2f;         // 標準的な移動速度
        accuracy = 0.85f;     // 狙ったコースに概ね打てる
        reactionDelay = 5;    // 少しだけ反応にディレイ
        break;

    case Difficulty::HARD:
        speed = 3.0f;         // かなり速い
        accuracy = 1.0f;      // 精密にライン際を狙う
        reactionDelay = 0;    // 即座に最適な打球を判断
        break;
    }
}

void Enemy::Update(Ball& ball, float speedFactor) {
    if (!ball.isMoving) return;

    float currentSpeed = speed * speedFactor;

    // 1. 移動処理（EASYの場合はボールの反応を少し鈍くする）
    if (ball.velocity.z > 0.0f) {
        // EASYモード時：ボールがネットを越えてからしか動かない制限などを付けるのも有効
        bool canMove = (difficulty != Difficulty::EASY) || (ball.position.z > 0.0f);

        if (canMove) {
            if (position.x < ball.position.x - 2.0f) position.x += currentSpeed;
            if (position.x > ball.position.x + 2.0f) position.x -= currentSpeed;

            float targetZ = ball.position.z + 15.0f;
            if (targetZ > 240.0f) targetZ = 240.0f;
            if (targetZ < 40.0f)  targetZ = 40.0f;

            if (position.z < targetZ - 3.0f) position.z += currentSpeed;
            if (position.z > targetZ + 3.0f) position.z -= currentSpeed;
        }
    }
    else {
        // ホームポジション復帰
        if (position.x < -2.0f)  position.x += currentSpeed * 0.5f;
        if (position.x > 2.0f)   position.x -= currentSpeed * 0.5f;
        if (position.z < 220.0f) position.z += currentSpeed * 0.5f;
        if (position.z > 220.0f) position.z -= currentSpeed * 0.5f;
    }

    // 2. 打球・コース精度の計算
    float dx = ball.position.x - position.x;
    float dz = ball.position.z - position.z;
    float distSq = dx * dx + dz * dz;

    if (ball.velocity.z > 0.0f && distSq < 1200.0f && ball.position.y < 40.0f) {

        // --- 狙う目標X座標の算出 ---
        float idealTargetX = 0.0f;
        if (ball.position.x > 30.0f)      idealTargetX = -75.0f; // オープンコート（左）
        else if (ball.position.x < -30.0f) idealTargetX = 75.0f;  // オープンコート（右）
        else                              idealTargetX = (std::rand() % 2 == 0) ? 60.0f : -60.0f;

        // ★ 精度（accuracy）によるブレの付与
        // accuracyが低いほど、理想の目標から中央（0.0f）寄りにブレる
        float actualTargetX = idealTargetX * accuracy;

        // EASYならさらにランダムなバラつき（±15.0f）を加えて甘い球にする
        if (difficulty == Difficulty::EASY) {
            float offset = (std::rand() % 30 - 15);
            actualTargetX += offset;
        }

        // --- 球種選択の思考制御 ---
        ShotType cpuShot = ShotType::FLAT;

        // HARDのみ完璧な球種使い分けを行う
        if (difficulty == Difficulty::HARD) {
            if (ball.position.z > -120.0f)       cpuShot = ShotType::TOPSPIN; // ロブ
            else if (position.z < 100.0f)        cpuShot = ShotType::SLICE;   // 低空ドロップ
        }
        else if (difficulty == Difficulty::NORMAL) {
            if (std::rand() % 100 < 30) cpuShot = ShotType::SLICE;
        }

        ShotConfig config = GetShotConfig(cpuShot);

        float targetZ = -220.0f;
        float distanceZ = position.z - targetZ;

        float velX = (actualTargetX - position.x) * 0.022f;
        float velZ = -distanceZ * 0.024f * config.velZFactor;
        float velY = (4.5f + (distanceZ * 0.002f)) * config.velYFactor;

        ball.Launch(ball.position, { velX, velY, velZ }, cpuShot);
    }
}