#ifndef COURT_H
#define COURT_H

// ----------------------------------------------------
// コート寸法定数（中心からの半分の距離）
// ----------------------------------------------------
const float COURT_LENGTH = 240.0f; // 全長 480 (Z: -240 〜 +240)
const float COURT_WIDTH = 120.0f; // ダブルス全幅 240 (X: -120 〜 +120)
const float SINGLES_WIDTH = 90.0f;  // ★シングルス全幅 180 (X: -90 〜 +90)
const float SERVICE_LENGTH = 120.0f; // サービスライン (Z: -120, +120)
const float NET_HEIGHT = 15.0f;  // ネットの高さ

class Court {
public:
    void Draw() const;
};

#endif // COURT_H