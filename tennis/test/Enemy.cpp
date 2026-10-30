#include "Enemy.h"
#include "ShotData.h"
#include <cmath>
#include <cstdlib>

Enemy::Enemy(Vector3 startPos)
    : Character(startPos, GetColor(255, 60, 60)) {
    SetDifficulty(Difficulty::NORMAL);
}

// 全難易度の能力値を一括引き上げ
void Enemy::SetDifficulty(Difficulty diff) {
    difficulty = diff;
    switch (diff) {
    case Difficulty::EASY:
        speed = 2.0f;         // 遅すぎないしっかりした足取り
        accuracy = 0.70f;     // コート内にしっかりコントロール
        reactionDelay = 5;
        break;

    case Difficulty::NORMAL:
        speed = 2.8f;         // プレイヤー並みの移動速度
        accuracy = 0.95f;     // オープンコートへ的確に打ち分ける
        reactionDelay = 2;
        break;

    case Difficulty::HARD:
        speed = 3.6f;         // 俊敏にボールに追いつく
        accuracy = 1.10f;     // サイドラインぎりぎりを射抜く
        reactionDelay = 0;
        break;

    case Difficulty::SUPER_HARD: // 爆速・超精度・予測打球
        speed = 4.4f;         // プレイヤーを凌駕する爆速移動
        accuracy = 1.35f;     // コート最深部・角（コーナー）をピンポイントで狙う
        reactionDelay = 0;
        break;
    }
}

void Enemy::Update(Ball& ball, float speedFactor) {
    float currentSpeed = speed * speedFactor;

    // 1. 移動処理
    if (ball.isMoving && ball.velocity.z > 0.0f) {
        // ボールが自分に向かって飛んできている場合の追従
        if (position.x < ball.position.x - 2.0f) position.x += currentSpeed;
        if (position.x > ball.position.x + 2.0f) position.x -= currentSpeed;

        float targetZ = ball.position.z + 15.0f;
        if (targetZ > 240.0f) targetZ = 240.0f;
        if (targetZ < 40.0f)  targetZ = 40.0f;

        if (position.z < targetZ - 3.0f) position.z += currentSpeed;
        if (position.z > targetZ + 3.0f) position.z -= currentSpeed;
    }
    else if (ball.isMoving && ball.velocity.z <= 0.0f) {
        // ラリー中（相手コートへ飛んでいる最中）のみホームポジションへ移動（中央に完全固定せず現在サイドを維持）
        float homeX = (position.x >= 0.0f) ? 35.0f : -35.0f;
        if (position.x < homeX - 2.0f)      position.x += currentSpeed * 0.5f;
        else if (position.x > homeX + 2.0f) position.x -= currentSpeed * 0.5f;

        if (position.z < 220.0f) position.z += currentSpeed * 0.5f;
        else if (position.z > 220.0f) position.z -= currentSpeed * 0.5f;
    }
    // ※ ball.isMoving が false の時（サーブ待機中など）は、位置調整を行わず ResetServePos で設定された初期位置を維持する

    // 移動制限（CPU側コート）
    if (position.x < -120.0f) position.x = -120.0f;
    if (position.x > 120.0f) position.x = 120.0f;
    if (position.z < 20.0f) position.z = 20.0f;
    if (position.z > 300.0f) position.z = 300.0f;

    if (!ball.isMoving) return;

    // 2. 打球・コース精度の計算
    float dx = ball.position.x - position.x;
    float dz = ball.position.z - position.z;
    float distSq = dx * dx + dz * dz;

    if (ball.velocity.z > 0.0f && distSq < 1400.0f && ball.position.y < 45.0f) {

        // --- 狙う目標X座標の算出 ---
        float idealTargetX = 0.0f;

        if (difficulty == Difficulty::SUPER_HARD) {
            // 超激むず：コーナー最深部（±85.0f）を厳しく攻める
            idealTargetX = (ball.position.x >= 0.0f) ? -85.0f : 85.0f;
        }
        else if (difficulty == Difficulty::HARD) {
            idealTargetX = (ball.position.x >= 0.0f) ? -78.0f : 78.0f;
        }
        else {
            if (ball.position.x > 20.0f)       idealTargetX = -70.0f;
            else if (ball.position.x < -20.0f) idealTargetX = 70.0f;
            else                               idealTargetX = (std::rand() % 2 == 0) ? 65.0f : -65.0f;
        }

        // 精度（accuracy）を乗算
        float actualTargetX = idealTargetX * accuracy;

        // --- 球種選択の思考制御 ---
        ShotType cpuShot = ShotType::FLAT;

        if (difficulty == Difficulty::SUPER_HARD) {
            // 状況判断に応じたコース配球
            if (ball.position.z > -100.0f)        cpuShot = ShotType::TOPSPIN; // 伸びる深いロブ
            else if (position.z < 120.0f)        cpuShot = ShotType::SLICE;   // 鋭い低空ドロップ
            else                                 cpuShot = ShotType::FLAT;    // 高速フラット
        }
        else if (difficulty == Difficulty::HARD || difficulty == Difficulty::NORMAL) {
            if (ball.position.z > -120.0f)       cpuShot = ShotType::TOPSPIN;
            else if (position.z < 100.0f)        cpuShot = ShotType::SLICE;
        }

        ShotConfig config = GetShotConfig(cpuShot);

        float targetZ = -220.0f;
        float distanceZ = position.z - targetZ;

        float velX = (actualTargetX - position.x) * 0.023f;
        float velZ = -distanceZ * 0.024f * config.velZFactor;
        float velY = (4.5f + (distanceZ * 0.002f)) * config.velYFactor;

        ball.Launch(ball.position, { velX, velY, velZ }, cpuShot);
    }
}