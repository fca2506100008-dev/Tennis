#include "DxLib.h"
#include "Ball.h"
#include "Player.h"
#include "Enemy.h"
#include "Court.h"
#include <cmath>
#include <string>

#ifndef SINGLES_WIDTH
#define SINGLES_WIDTH 90.0f
#endif

#ifndef COURT_LENGTH
#define COURT_LENGTH 240.0f
#endif

// ★ ゲーム状態（ポーズ画面 PAUSE を追加）
enum class GameState {
    TITLE,
    PLAY,
    PAUSE,
    RESULT
};

std::string GetScoreText(int pScore, int eScore) {
    if (pScore >= 3 && eScore >= 3) {
        if (pScore == eScore) return "40 - 40 (DEUCE)";
        if (pScore == eScore + 1) return "AD - 40";
        if (eScore == pScore + 1) return "40 - AD";
    }

    const char* scoreNames[] = { "0", "15", "30", "40" };
    std::string pStr = (pScore < 4) ? scoreNames[pScore] : "40";
    std::string eStr = (eScore < 4) ? scoreNames[eScore] : "40";

    return pStr + " - " + eStr;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    ChangeWindowMode(TRUE);
    SetGraphMode(1280, 720, 32);
    if (DxLib_Init() == -1) return -1;
    SetDrawScreen(DX_SCREEN_BACK);

    const Vector3 PLAYER_INIT_POS = { 0.0f, 0.0f, -240.0f };
    const Vector3 ENEMY_INIT_POS = { 0.0f, 0.0f,  240.0f };

    Player player(PLAYER_INIT_POS);
    Enemy enemy(ENEMY_INIT_POS);

    Ball ball(
        { player.position.x, player.position.y + 10.0f, player.position.z + 5.0f },
        { 0.0f, 0.0f, 0.0f }
    );
    Court court;

    bool rKeyPrev = false;
    bool enterKeyPrev = false;
    bool leftKeyPrev = false;
    bool rightKeyPrev = false;
    bool upKeyPrev = false;
    bool downKeyPrev = false;
    bool escKeyPrev = false;
    bool pKeyPrev = false;

    std::string resultMessage = "";
    int messageTimer = 0;

    int playerScore = 0;
    int enemyScore = 0;
    int playerGames = 0;
    int enemyGames = 0;

    const int targetGamesList[] = { 2, 5, 10 };
    int selectedGameIndex = 0;

    const float speedList[] = { 1.0f, 0.5f };
    int selectedSpeedIndex = 0;

    bool isPlayerServer = true;
    int cpuServeTimer = 0;

    GameState currentState = GameState::TITLE;
    std::string winnerText = "";

    // ★ ポーズメニュー用変数（0: 再開, 1: リスタート, 2: タイトルへ戻る）
    int pauseMenuIndex = 0;

    SetCameraNearFar(5.0f, 1200.0f);

    auto ResetAllGame = [&]() {
        playerScore = 0;
        enemyScore = 0;
        playerGames = 0;
        enemyGames = 0;
        isPlayerServer = true;
        cpuServeTimer = 0;
        player.position = PLAYER_INIT_POS;
        enemy.position = ENEMY_INIT_POS;
        ball.Reset({ player.position.x, player.position.y + 10.0f, player.position.z + 5.0f });
        };

    auto AddPoint = [&](bool isPlayerPoint, const std::string& reason) {
        if (isPlayerPoint) {
            playerScore++;
        }
        else {
            enemyScore++;
        }

        bool gameWon = false;
        if (playerScore >= 4 && playerScore >= enemyScore + 2) {
            playerGames++;
            gameWon = true;
        }
        else if (enemyScore >= 4 && enemyScore >= playerScore + 2) {
            enemyGames++;
            gameWon = true;
        }

        if (gameWon) {
            playerScore = 0;
            enemyScore = 0;

            isPlayerServer = !isPlayerServer;

            int targetGames = targetGamesList[selectedGameIndex];
            if (playerGames >= targetGames) {
                winnerText = "YOU WIN!";
                currentState = GameState::RESULT;
            }
            else if (enemyGames >= targetGames) {
                winnerText = "YOU LOSE...";
                currentState = GameState::RESULT;
            }
        }

        player.position = PLAYER_INIT_POS;
        enemy.position = ENEMY_INIT_POS;

        if (isPlayerServer) {
            ball.Reset({ player.position.x, player.position.y + 10.0f, player.position.z + 5.0f });
        }
        else {
            ball.Reset({ enemy.position.x, enemy.position.y + 10.0f, enemy.position.z - 5.0f });
        }

        cpuServeTimer = 0;
        resultMessage = reason;
        messageTimer = 120;
        };

    while (ProcessMessage() == 0 && ClearDrawScreen() == 0) {
        bool rKey = CheckHitKey(KEY_INPUT_R) != 0;
        bool enterKey = CheckHitKey(KEY_INPUT_RETURN) != 0;
        bool leftKey = CheckHitKey(KEY_INPUT_LEFT) != 0;
        bool rightKey = CheckHitKey(KEY_INPUT_RIGHT) != 0;
        bool upKey = CheckHitKey(KEY_INPUT_UP) != 0;
        bool downKey = CheckHitKey(KEY_INPUT_DOWN) != 0;
        bool escKey = CheckHitKey(KEY_INPUT_ESCAPE) != 0;
        bool pKey = CheckHitKey(KEY_INPUT_P) != 0;

        float currentSpeedFactor = speedList[selectedSpeedIndex];

        // ====================================================
        // ① タイトル画面の処理
        // ====================================================
        if (currentState == GameState::TITLE) {
            if (leftKey && !leftKeyPrev) {
                selectedGameIndex = (selectedGameIndex + 2) % 3;
            }
            if (rightKey && !rightKeyPrev) {
                selectedGameIndex = (selectedGameIndex + 1) % 3;
            }

            if ((upKey && !upKeyPrev) || (downKey && !downKeyPrev)) {
                selectedSpeedIndex = (selectedSpeedIndex + 1) % 2;
            }

            if (enterKey && !enterKeyPrev) {
                ResetAllGame();
                currentState = GameState::PLAY;
            }

            SetFontSize(64);
            DrawString(400, 140, "3D TENNIS GAME", GetColor(255, 255, 255));

            SetFontSize(20);
            DrawString(430, 270, "MATCH SETTING (Left/Right Key):", GetColor(200, 200, 255));
            std::string modeText = "<  " + std::to_string(targetGamesList[selectedGameIndex]) + " GAMES MATCH  >";
            SetFontSize(28);
            DrawString(450, 300, modeText.c_str(), GetColor(255, 255, 0));

            SetFontSize(20);
            DrawString(430, 370, "SPEED SETTING (Up/Down Key):", GetColor(200, 200, 255));
            std::string speedText = (selectedSpeedIndex == 0) ? "^  SPEED: 1.0x (NORMAL)  v" : "^  SPEED: 0.5x (SLOW)  v";
            SetFontSize(28);
            DrawString(440, 400, speedText.c_str(), (selectedSpeedIndex == 0) ? GetColor(0, 255, 100) : GetColor(255, 150, 0));

            SetFontSize(28);
            DrawString(460, 490, "PRESS ENTER TO START", GetColor(0, 255, 255));

            SetFontSize(16);
            DrawString(430, 580, "操作方法: 矢印キー(移動/設定選択) / Z or SPACE(打球)", GetColor(200, 200, 200));
            DrawString(430, 600, "Rキー: サーブ発射 / ESC or Pキー: ポーズ", GetColor(200, 200, 200));
        }

        // ====================================================
        // ② プレイ中 ＆ ③ ポーズ画面の処理
        // ====================================================
        else if (currentState == GameState::PLAY || currentState == GameState::PAUSE) {

            // ★ ESCキー または Pキー でポーズ切替
            if ((escKey && !escKeyPrev) || (pKey && !pKeyPrev)) {
                if (currentState == GameState::PLAY) {
                    currentState = GameState::PAUSE;
                    pauseMenuIndex = 0; // 初期選択は「RESUME」
                }
                else if (currentState == GameState::PAUSE) {
                    currentState = GameState::PLAY;
                }
            }

            // ----------------------------------------------------
            // プレイ中のみ更新処理を実行（ポーズ中は時間を止める）
            // ----------------------------------------------------
            if (currentState == GameState::PLAY) {
                if (!ball.isMoving) {
                    if (messageTimer <= 0) {
                        if (isPlayerServer) {
                            ball.position = { player.position.x, player.position.y + 10.0f, player.position.z + 5.0f };

                            if (rKey && !rKeyPrev) {
                                ball.Launch(
                                    { player.position.x, 20.0f, player.position.z + 10.0f },
                                    { 0.0f, 5.5f, 8.5f }
                                );
                            }
                        }
                        else {
                            ball.position = { enemy.position.x, enemy.position.y + 10.0f, enemy.position.z - 5.0f };

                            cpuServeTimer++;
                            int targetWait = (int)(60 / currentSpeedFactor);
                            if (cpuServeTimer >= targetWait || (enterKey && !enterKeyPrev)) {
                                ball.Launch(
                                    { enemy.position.x, 20.0f, enemy.position.z - 10.0f },
                                    { 0.0f, 5.5f, -8.5f }
                                );
                                cpuServeTimer = 0;
                            }
                        }
                    }
                }

                player.Update(ball, currentSpeedFactor);
                enemy.Update(ball, currentSpeedFactor);
                ball.Update(currentSpeedFactor);

                if (ball.isMoving && ball.hasLanded) {
                    bool isOutX = std::abs(ball.landingPos.x) > SINGLES_WIDTH;
                    bool isOutZ = std::abs(ball.landingPos.z) > COURT_LENGTH;

                    if (isOutX || isOutZ) {
                        bool playerPoint = (ball.landingPos.z < 0.0f);
                        AddPoint(playerPoint, "OUT!");
                    }
                    else if (ball.bounceCount >= 2) {
                        bool playerPoint = (ball.landingPos.z > 0.0f);
                        AddPoint(playerPoint, "2 BOUNCE!");
                    }
                }
            }
            // ----------------------------------------------------
            // ポーズ中のメニュー操作
            // ----------------------------------------------------
            else if (currentState == GameState::PAUSE) {
                if (upKey && !upKeyPrev) {
                    pauseMenuIndex = (pauseMenuIndex + 2) % 3; // 上へ
                }
                if (downKey && !downKeyPrev) {
                    pauseMenuIndex = (pauseMenuIndex + 1) % 3; // 下へ
                }

                if (enterKey && !enterKeyPrev) {
                    if (pauseMenuIndex == 0) {
                        // ① 再開 (RESUME)
                        currentState = GameState::PLAY;
                    }
                    else if (pauseMenuIndex == 1) {
                        // ② リスタート (RESTART)
                        ResetAllGame();
                        currentState = GameState::PLAY;
                    }
                    else if (pauseMenuIndex == 2) {
                        // ③ タイトル画面へ戻る (TITLE)
                        currentState = GameState::TITLE;
                    }
                }
            }

            // ----------------------------------------------------
            // 3Dオブジェクト・UI描画（プレイ中・ポーズ共通）
            // ----------------------------------------------------
            SetupCamera_Perspective(70.0f * 3.14159265f / 180.0f);
            SetCameraPositionAndTarget_UpVecY(
                VGet(0.0f, 210.0f, -380.0f),
                VGet(0.0f, 0.0f, -40.0f)
            );

            court.Draw();
            player.DrawReachArea();
            ball.DrawShadow();
            ball.DrawLandingPoint();
            ball.DrawPredictMarker();

            player.Draw();
            enemy.Draw();
            ball.Draw();

            if (isPlayerServer) {
                DrawString(10, 10, "Rキー: サーブ発射 (YOUR SERVE)", GetColor(0, 255, 255));
            }
            else {
                DrawString(10, 10, "CPU SERVE (自動またはEnterキーで発射)", GetColor(255, 150, 150));
            }

            std::string spdInfo = (selectedSpeedIndex == 0) ? "[SPEED: 1.0x]" : "[SPEED: 0.5x SLOW]";
            DrawString(10, 30, spdInfo.c_str(), (selectedSpeedIndex == 0) ? GetColor(200, 200, 200) : GetColor(255, 150, 0));
            DrawString(10, 50, "ESCキー / Pキー: ポーズ", GetColor(200, 200, 200));

            // スコアボード
            DrawBox(930, 10, 1260, 120, GetColor(0, 0, 0), TRUE);
            DrawBox(930, 10, 1260, 120, GetColor(255, 255, 255), FALSE);

            SetFontSize(16);
            std::string matchTypeStr = "(" + std::to_string(targetGamesList[selectedGameIndex]) + " GAMES MATCH)";
            DrawString(950, 15, matchTypeStr.c_str(), GetColor(180, 180, 255));

            SetFontSize(20);
            std::string gameStr = "GAMES  " + std::to_string(playerGames) + " - " + std::to_string(enemyGames);
            DrawString(950, 38, gameStr.c_str(), GetColor(255, 255, 0));

            std::string scoreStr = "POINT  " + GetScoreText(playerScore, enemyScore);
            DrawString(950, 65, scoreStr.c_str(), GetColor(255, 255, 255));

            std::string serverStr = isPlayerServer ? "SERVE: YOU" : "SERVE: CPU";
            DrawString(950, 92, serverStr.c_str(), isPlayerServer ? GetColor(0, 255, 255) : GetColor(255, 100, 100));

            SetFontSize(16);

            if (messageTimer > 0 && currentState == GameState::PLAY) {
                SetFontSize(48);
                DrawString(520, 300, resultMessage.c_str(), GetColor(255, 50, 50));
                SetFontSize(16);
                messageTimer--;
            }

            // ----------------------------------------------------
            // ★ ポーズ画面専用オーバレイUI描画
            // ----------------------------------------------------
            if (currentState == GameState::PAUSE) {
                // 暗い背景（黒の半透明ボックス）
                SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
                DrawBox(0, 0, 1280, 720, GetColor(0, 0, 0), TRUE);
                SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

                // ポーズウィンドウ枠
                DrawBox(440, 180, 840, 540, GetColor(30, 30, 40), TRUE);
                DrawBox(440, 180, 840, 540, GetColor(200, 200, 255), FALSE);

                SetFontSize(48);
                DrawString(530, 210, "PAUSE", GetColor(255, 255, 255));

                SetFontSize(24);
                const char* menuItems[] = { "RESUME", "RESTART", "TITLE" };
                for (int i = 0; i < 3; i++) {
                    int yPos = 310 + i * 60;
                    if (i == pauseMenuIndex) {
                        // 選択項目を黄色で強調 & カーソル表示
                        std::string selText = "> " + std::string(menuItems[i]) + " <";
                        DrawString(520, yPos, selText.c_str(), GetColor(255, 255, 0));
                    }
                    else {
                        DrawString(540, yPos, menuItems[i], GetColor(180, 180, 180));
                    }
                }

                SetFontSize(16);
                DrawString(510, 490, "↑/↓: 選択  ENTER: 決定", GetColor(200, 200, 200));
            }
        }

        // ====================================================
        // ④ 勝敗リザルト画面の処理
        // ====================================================
        else if (currentState == GameState::RESULT) {
            if (enterKey && !enterKeyPrev) {
                currentState = GameState::TITLE;
            }

            SetFontSize(64);
            unsigned int color = (winnerText == "YOU WIN!") ? GetColor(255, 215, 0) : GetColor(100, 100, 255);
            DrawString(460, 250, winnerText.c_str(), color);

            SetFontSize(28);
            DrawString(430, 420, "PRESS ENTER TO TITLE", GetColor(255, 255, 255));
            SetFontSize(16);
        }

        rKeyPrev = rKey;
        enterKeyPrev = enterKey;
        leftKeyPrev = leftKey;
        rightKeyPrev = rightKey;
        upKeyPrev = upKey;
        downKeyPrev = downKey;
        escKeyPrev = escKey;
        pKeyPrev = pKey;

        ScreenFlip();
    }

    DxLib_End();
    return 0;
}