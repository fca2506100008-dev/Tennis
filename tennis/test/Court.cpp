#include "Court.h"
#include "DxLib.h"

void Court::Draw() const {
    // --- 1. コート面（緑）の描画 ---
    VERTEX3D vert[6];
    for (int i = 0; i < 6; ++i) {
        vert[i].norm = VGet(0.0f, 1.0f, 0.0f);
        vert[i].dif = GetColorU8(34, 139, 34, 255); // フォレストグリーン
        vert[i].spc = GetColorU8(0, 0, 0, 0);
        vert[i].u = 0.0f; vert[i].v = 0.0f;
    }

    // コート外側の緑領域も比例して大きく
    float marginX = COURT_WIDTH + 120.0f;
    float marginZ = COURT_LENGTH + 120.0f;

    VECTOR pTL = VGet(-marginX, 0.0f, marginZ);
    VECTOR pTR = VGet(marginX, 0.0f, marginZ);
    VECTOR pBR = VGet(marginX, 0.0f, -marginZ);
    VECTOR pBL = VGet(-marginX, 0.0f, -marginZ);

    vert[0].pos = pTL; vert[1].pos = pTR; vert[2].pos = pBR;
    vert[3].pos = pTL; vert[4].pos = pBR; vert[5].pos = pBL;

    SetUseCullingFlag(FALSE);
    DrawPolygon3D(vert, 2, DX_NONE_GRAPH, FALSE);
    SetUseCullingFlag(TRUE);

    // --- 2. 白ライン ---
    unsigned int white = GetColor(255, 255, 255);

    // 外枠（ダブルスライン & ベースライン）
    DrawLine3D(VGet(-COURT_WIDTH, 0.1f, -COURT_LENGTH), VGet(COURT_WIDTH, 0.1f, -COURT_LENGTH), white);
    DrawLine3D(VGet(-COURT_WIDTH, 0.1f, COURT_LENGTH), VGet(COURT_WIDTH, 0.1f, COURT_LENGTH), white);
    DrawLine3D(VGet(-COURT_WIDTH, 0.1f, -COURT_LENGTH), VGet(-COURT_WIDTH, 0.1f, COURT_LENGTH), white);
    DrawLine3D(VGet(COURT_WIDTH, 0.1f, -COURT_LENGTH), VGet(COURT_WIDTH, 0.1f, COURT_LENGTH), white);

    // シングルスライン
    DrawLine3D(VGet(-SINGLES_WIDTH, 0.1f, -COURT_LENGTH), VGet(-SINGLES_WIDTH, 0.1f, COURT_LENGTH), white);
    DrawLine3D(VGet(SINGLES_WIDTH, 0.1f, -COURT_LENGTH), VGet(SINGLES_WIDTH, 0.1f, COURT_LENGTH), white);

    // サービスライン
    DrawLine3D(VGet(-SINGLES_WIDTH, 0.1f, -SERVICE_LENGTH), VGet(SINGLES_WIDTH, 0.1f, -SERVICE_LENGTH), white);
    DrawLine3D(VGet(-SINGLES_WIDTH, 0.1f, SERVICE_LENGTH), VGet(SINGLES_WIDTH, 0.1f, SERVICE_LENGTH), white);

    // センターサービスライン
    DrawLine3D(VGet(0.0f, 0.1f, -SERVICE_LENGTH), VGet(0.0f, 0.1f, SERVICE_LENGTH), white);

    // センターマーク
    DrawLine3D(VGet(0.0f, 0.1f, -COURT_LENGTH), VGet(0.0f, 0.1f, -COURT_LENGTH + 18.0f), white);
    DrawLine3D(VGet(0.0f, 0.1f, COURT_LENGTH), VGet(0.0f, 0.1f, COURT_LENGTH - 18.0f), white);

    // --- 3. ネット ---
    unsigned int netColor = GetColor(200, 200, 200);
    unsigned int postColor = GetColor(80, 80, 80);

    DrawLine3D(VGet(-COURT_WIDTH - 3.0f, 0.0f, 0.0f), VGet(-COURT_WIDTH - 3.0f, NET_HEIGHT, 0.0f), postColor);
    DrawLine3D(VGet(COURT_WIDTH + 3.0f, 0.0f, 0.0f), VGet(COURT_WIDTH + 3.0f, NET_HEIGHT, 0.0f), postColor);
    DrawLine3D(VGet(-COURT_WIDTH - 3.0f, NET_HEIGHT, 0.0f), VGet(COURT_WIDTH + 3.0f, NET_HEIGHT, 0.0f), white);

    for (float y = 3.0f; y < NET_HEIGHT; y += 3.5f) {
        DrawLine3D(VGet(-COURT_WIDTH - 3.0f, y, 0.0f), VGet(COURT_WIDTH + 3.0f, y, 0.0f), netColor);
    }
    for (float x = -COURT_WIDTH; x <= COURT_WIDTH; x += 30.0f) {
        DrawLine3D(VGet(x, 0.0f, 0.0f), VGet(x, NET_HEIGHT, 0.0f), netColor);
    }
}