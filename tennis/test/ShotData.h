#pragma once

enum class ShotType {
    FLAT,    // フラット（標準）
    TOPSPIN, // トップスピン（急降下＆高くバウンド）
    SLICE    // スライス（低空＆低バウンド・滑る）
};

struct ShotConfig {
    float velYFactor;   // 打ち出し高さの倍率
    float velZFactor;   // 打ち出し速度の倍率
    float gravity;      // 重力加速度
    float bounceYRatio; // バウンド時のY速度保持率（高いほど跳ねる）
    float bounceZRatio; // バウンド時のZ速度保持率（高いほど前進維持）
};

// 球種ごとの設定値を取得する関数
inline ShotConfig GetShotConfig(ShotType type) {
    switch (type) {
    case ShotType::TOPSPIN:
        return { 1.25f, 1.15f, 0.50f, 0.85f, 0.90f };
    case ShotType::SLICE:
        return { 0.70f, 0.85f, 0.22f, 0.50f, 0.75f };
    case ShotType::FLAT:
    default:
        return { 1.00f, 1.00f, 0.35f, 0.70f, 0.85f };
    }
}