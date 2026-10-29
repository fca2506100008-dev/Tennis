#include "DxLib.h"
#include "Ball.h"
#include "Player.h"
#include "Enemy.h"
#include "Court.h"
#include "SoundManager.h"
#include "ShotData.h"
#include "FpsManager.h"
#include <cmath>
#include <string>

#ifndef SINGLES_WIDTH
#define SINGLES_WIDTH 90.0f
#endif

#ifndef COURT_LENGTH
#define COURT_LENGTH 240.0f
#endif

#ifndef SERVICE_LINE_Z
#define SERVICE_LINE_Z 140.0f
#endif

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

// 縁取り文字列描画ヘルパー関数
void DrawBorderedString(int x, int y, const char* string, unsigned int color, unsigned int borderColor = GetColor(0, 0, 0)) {
    DrawString(x - 1, y, string, borderColor);
    DrawString(x + 1, y, string, borderColor);
    DrawString(x, y - 1, string, borderColor);
    DrawString(x, y + 1, string, borderColor);
    DrawString(x - 1, y - 1, string, borderColor);
    DrawString(x + 1, y - 1, string, borderColor);
    DrawString(x - 1, y + 1, string, borderColor);
    DrawString(x + 1, y + 1, string, borderColor);

    DrawString(x, y, string, color);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    ChangeWindowMode(TRUE);
    SetGraphMode(1280, 720, 32);

    SetWaitVSyncFlag(FALSE);

    if (DxLib_Init() == -1) return -1;
    SetDrawScreen(DX_SCREEN_BACK);

    SetBackgroundColor(30, 140, 50);
    SoundManager::GetInstance().Init();

    // ★ タイトル用画像とゲーム中用背景画像の読み込み
    int titleBgHandle = LoadGraph("title_bg.png.jpg");
    int gameBgHandle = LoadGraph("game_bg.png.jpg");

    FpsManager fpsManager;

    const Vector3 PLAYER_INIT_POS = { 0.0f, 0.0f, -240.0f };
    const Vector3 ENEMY_INIT_POS = { 0.0f, 0.0f,  240.0f };

    Player player(PLAYER_INIT_POS);
    Enemy enemy(ENEMY_INIT_POS);

    Ball ball(
        { player.position.x, player.position.y + 10.0f, player.position.z + 5.0f },
        { 0.0f, 0.0f, 0.0f }
    );
    Court court;

    bool enterKeyPrev = false;
    bool aKeyPrev = false;
    bool dKeyPrev = false;
    bool wKeyPrev = false;
    bool sKeyPrev = false;

    bool leftKeyPrev = false;
    bool rightKeyPrev = false;
    bool upKeyPrev = false;
    bool downKeyPrev = false;

    bool rKeyPrev = false;
    bool shiftKeyPrev = false;
    bool escKeyPrev = false;
    bool pKeyPrev = false;

    bool mouseLeftPrev = false;

    std::string resultMessage = "";
    int messageTimer = 0;

    int playerScore = 0;
    int enemyScore = 0;
    int playerGames = 0;
    int enemyGames = 0;

    const int targetGamesList[] = { 2, 5, 10 };
    int selectedGameIndex = 0;

    const char* diffNames[] = { "EASY", "NORMAL", "HARD" };
    const unsigned int diffColors[] = { GetColor(0, 255, 128), GetColor(255, 255, 0), GetColor(255, 80, 80) };
    int selectedDiffIndex = 1;

    const float speedList[] = { 1.0f, 1.5f, 2.0f, 0.5f };
    const char* speedNames[] = { "1.0x (NORMAL)", "1.5x (FAST)", "2.0x (SUPER FAST)", "0.5x (SLOW)" };
    const unsigned int speedColors[] = { GetColor(0, 255, 100), GetColor(255, 200, 0), GetColor(255, 80, 80), GetColor(100, 200, 255) };
    int selectedSpeedIndex = 0;

    bool isPlayerServer = true;
    int cpuServeTimer = 0;

    int faultCount = 0;
    bool isServeRally = false;

    GameState currentState = GameState::TITLE;
    std::string winnerText = "";
    int pauseMenuIndex = 0;

    SetCameraNearFar(5.0f, 1200.0f);

    auto ResetServePos = [&]() {
        int totalPoints = playerScore + enemyScore;
        float sideX = (totalPoints % 2 == 0) ? 45.0f : -45.0f;

        if (isPlayerServer) {
            player.position = { sideX, 0.0f, -240.0f };
            enemy.position = { -sideX, 0.0f, 240.0f };
            ball.Reset({ player.position.x, player.position.y + 10.0f, player.position.z + 5.0f });
        }
        else {
            enemy.position = { sideX, 0.0f, 240.0f };
            player.position = { -sideX, 0.0f, -240.0f };
            ball.Reset({ enemy.position.x, enemy.position.y + 10.0f, enemy.position.z - 5.0f });
        }
        cpuServeTimer = 0;
        };

    auto ResetAllGame = [&]() {
        playerScore = 0;
        enemyScore = 0;
        playerGames = 0;
        enemyGames = 0;
        isPlayerServer = true;
        faultCount = 0;
        isServeRally = false;
        ResetServePos();
        };

    auto AddPoint = [&](bool isPlayerPoint, const std::string& reason) {
        faultCount = 0;
        isServeRally = false;

        if (isPlayerPoint) {
            playerScore++;
            SoundManager::GetInstance().Play(SE::POINT);
        }
        else {
            enemyScore++;
            //SoundManager::GetInstance().Play(SE::OUT);
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

        ResetServePos();
        resultMessage = reason;
        messageTimer = 120;
        };

    auto IsInServiceBox = [&](Vector3 pos, bool serverIsPlayer, float serverX) -> bool {
        const float MARGIN = 3.0f;

        if (serverIsPlayer) {
            if (pos.z < -MARGIN || pos.z > SERVICE_LINE_Z + MARGIN) return false;
            if (serverX > 0.0f) return (pos.x >= -SINGLES_WIDTH - MARGIN && pos.x <= MARGIN);
            else                return (pos.x >= -MARGIN && pos.x <= SINGLES_WIDTH + MARGIN);
        }
        else {
            if (pos.z > MARGIN || pos.z < -SERVICE_LINE_Z - MARGIN) return false;
            if (serverX > 0.0f) return (pos.x >= -SINGLES_WIDTH - MARGIN && pos.x <= MARGIN);
            else                return (pos.x >= -MARGIN && pos.x <= SINGLES_WIDTH + MARGIN);
        }
        };

    while (ProcessMessage() == 0 && ClearDrawScreen() == 0) {
        fpsManager.Start();

        bool enterKey = CheckHitKey(KEY_INPUT_RETURN) != 0;
        bool aKey = CheckHitKey(KEY_INPUT_A) != 0;
        bool dKey = CheckHitKey(KEY_INPUT_D) != 0;
        bool wKey = CheckHitKey(KEY_INPUT_W) != 0;
        bool sKey = CheckHitKey(KEY_INPUT_S) != 0;

        bool leftKey = CheckHitKey(KEY_INPUT_LEFT) != 0;
        bool rightKey = CheckHitKey(KEY_INPUT_RIGHT) != 0;
        bool upKey = CheckHitKey(KEY_INPUT_UP) != 0;
        bool downKey = CheckHitKey(KEY_INPUT_DOWN) != 0;

        bool rKey = CheckHitKey(KEY_INPUT_R) != 0;
        bool shiftKey = (CheckHitKey(KEY_INPUT_LSHIFT) != 0) || (CheckHitKey(KEY_INPUT_RSHIFT) != 0);
        bool escKey = CheckHitKey(KEY_INPUT_ESCAPE) != 0;
        bool pKey = CheckHitKey(KEY_INPUT_P) != 0;

        int mouseInput = GetMouseInput();
        bool mouseLeft = (mouseInput & MOUSE_INPUT_LEFT) != 0;

        float currentSpeedFactor = speedList[selectedSpeedIndex] * 0.5f;

        if (currentState == GameState::TITLE) {
            if (leftKey && !leftKeyPrev)   selectedGameIndex = (selectedGameIndex + 2) % 3;
            if (rightKey && !rightKeyPrev) selectedGameIndex = (selectedGameIndex + 1) % 3;

            if (upKey && !upKeyPrev)     selectedDiffIndex = (selectedDiffIndex + 2) % 3;
            if (downKey && !downKeyPrev) selectedDiffIndex = (selectedDiffIndex + 1) % 3;

            if (shiftKey && !shiftKeyPrev) {
                selectedSpeedIndex = (selectedSpeedIndex + 1) % 4;
            }

            if (enterKey && !enterKeyPrev) {
                ResetAllGame();
                enemy.SetDifficulty(static_cast<Difficulty>(selectedDiffIndex));
                currentState = GameState::PLAY;
            }

            if (titleBgHandle != -1) {
                DrawExtendGraph(0, 0, 1280, 720, titleBgHandle, FALSE);
            }

            SetFontSize(64);
            DrawBorderedString(400, 100, "3D TENNIS GAME", GetColor(255, 255, 255));

            SetFontSize(20);
            DrawBorderedString(430, 210, "MATCH SETTING (Left / Right Key):", GetColor(200, 200, 255));
            std::string modeText = "<  " + std::to_string(targetGamesList[selectedGameIndex]) + " GAMES MATCH  >";
            SetFontSize(26);
            DrawBorderedString(450, 235, modeText.c_str(), GetColor(255, 255, 0));

            SetFontSize(20);
            DrawBorderedString(430, 290, "CPU DIFFICULTY (Up / Down Key):", GetColor(200, 200, 255));
            std::string diffText = "^  DIFFICULTY: " + std::string(diffNames[selectedDiffIndex]) + "  v";
            SetFontSize(26);
            DrawBorderedString(430, 315, diffText.c_str(), diffColors[selectedDiffIndex]);

            SetFontSize(20);
            DrawBorderedString(430, 370, "SPEED SETTING (SHIFT Key):", GetColor(200, 200, 255));
            std::string speedText = "[ SHIFT ] SPEED: " + std::string(speedNames[selectedSpeedIndex]);
            SetFontSize(26);
            DrawBorderedString(420, 395, speedText.c_str(), speedColors[selectedSpeedIndex]);

            SetFontSize(28);
            DrawBorderedString(460, 480, "PRESS ENTER TO START", GetColor(0, 255, 255));

            SetFontSize(16);
            DrawBorderedString(380, 560, "操作方法: WASD (移動) / SHIFT (速度) / Rキー (トス)", GetColor(200, 200, 200));
            DrawBorderedString(380, 585, "サーブ狙い: A / D キー  /  ショット: 左クリック", GetColor(255, 255, 100));
            DrawBorderedString(380, 610, "ESC or Pキー: ポーズ", GetColor(200, 200, 200));
        }
        else if (currentState == GameState::PLAY || currentState == GameState::PAUSE) {

            if ((escKey && !escKeyPrev) || (pKey && !pKeyPrev)) {
                if (currentState == GameState::PLAY) {
                    currentState = GameState::PAUSE;
                    pauseMenuIndex = 0;
                }
                else if (currentState == GameState::PAUSE) {
                    currentState = GameState::PLAY;
                }
            }

            if (currentState == GameState::PLAY) {
                if (!ball.isMoving) {
                    if (messageTimer <= 0) {
                        if (isPlayerServer) {
                            if (!ball.isTossing) {
                                ball.position = { player.position.x, player.position.y + 10.0f, player.position.z + 5.0f };

                                if (rKey && !rKeyPrev) {
                                    ball.StartToss({ player.position.x, player.position.y + 12.0f, player.position.z + 5.0f });
                                }
                            }
                            else {
                                bool shotInput = mouseLeft && !mouseLeftPrev;
                                if (shotInput && ball.position.y >= 20.0f) {
                                    ShotType sType = ShotType::FLAT;

                                    float targetX = (player.position.x > 0.0f) ? -35.0f : 35.0f;
                                    if (aKey) targetX -= 20.0f;
                                    if (dKey) targetX += 20.0f;

                                    float velX = (targetX - player.position.x) * 0.022f;

                                    ball.Launch(ball.position, { velX, 3.9f, 10.4f }, sType);
                                    isServeRally = true;
                                }
                            }
                        }
                        else {
                            if (!ball.isTossing) {
                                ball.position = { enemy.position.x, enemy.position.y + 10.0f, enemy.position.z - 5.0f };

                                cpuServeTimer++;
                                int targetWait = (int)(120 / (speedList[selectedSpeedIndex]));
                                if (cpuServeTimer >= targetWait || (enterKey && !enterKeyPrev)) {
                                    ball.StartToss({ enemy.position.x, enemy.position.y + 10.0f, enemy.position.z - 5.0f });
                                    cpuServeTimer = 0;
                                }
                            }
                            else {
                                if (ball.velocity.y <= 1.0f) {
                                    float targetX = (enemy.position.x > 0.0f) ? -35.0f : 35.0f;
                                    float velX = (targetX - enemy.position.x) * 0.022f;

                                    ball.Launch(ball.position, { velX, 3.9f, -10.4f }, ShotType::FLAT);
                                    isServeRally = true;
                                }
                            }
                        }
                    }
                }

                player.Update(ball, currentSpeedFactor);
                enemy.Update(ball, currentSpeedFactor);
                ball.Update(currentSpeedFactor);

                if (ball.isMoving && ball.hasLanded) {
                    if (isServeRally) {
                        float serverX = isPlayerServer ? player.position.x : enemy.position.x;
                        bool inBox = IsInServiceBox(ball.landingPos, isPlayerServer, serverX);

                        if (!inBox) {
                            faultCount++;
                            if (faultCount >= 2) {
                                bool playerPoint = !isPlayerServer;
                                AddPoint(playerPoint, "DOUBLE FAULT!");
                            }
                            else {
                                //SoundManager::GetInstance().Play(SE::OUT);
                                resultMessage = "FAULT!";
                                messageTimer = 180;
                                isServeRally = false;
                                ResetServePos();
                            }
                        }
                        else {
                            isServeRally = false;
                        }
                    }
                    else {
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
            }
            else if (currentState == GameState::PAUSE) {
                if ((wKey && !wKeyPrev) || (upKey && !upKeyPrev))         pauseMenuIndex = (pauseMenuIndex + 2) % 3;
                if ((sKey && !sKeyPrev) || (downKey && !downKeyPrev))     pauseMenuIndex = (pauseMenuIndex + 1) % 3;

                if (enterKey && !enterKeyPrev) {
                    if (pauseMenuIndex == 0)      currentState = GameState::PLAY;
                    else if (pauseMenuIndex == 1) { ResetAllGame(); currentState = GameState::PLAY; }
                    else if (pauseMenuIndex == 2) currentState = GameState::TITLE;
                }
            }

            // ★ 3D描画（コート・キャラ・ボール）を行う前に、ゲーム中背景画像を描画
            if (gameBgHandle != -1) {
                DrawExtendGraph(0, 0, 1280, 720, gameBgHandle, FALSE);
            }

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
                if (ball.isTossing) {
                    DrawString(10, 10, "A / D キー(コース狙い) + 左クリック: サーブ！", GetColor(255, 255, 0));
                }
                else {
                    std::string serveStr = "Rキー: トスを上げる (YOUR SERVE";
                    if (faultCount == 1) serveStr += " - 2nd SERVE)";
                    else serveStr += ")";
                    DrawString(10, 10, serveStr.c_str(), (faultCount == 1) ? GetColor(255, 100, 100) : GetColor(0, 255, 255));
                }
            }
            else {
                std::string serveStr = "CPU SERVE";
                if (faultCount == 1) serveStr += " (2nd SERVE)";
                DrawString(10, 10, serveStr.c_str(), GetColor(255, 150, 150));
            }

            std::string statusInfo = "[CPU: " + std::string(diffNames[selectedDiffIndex]) + "]";
            if (selectedSpeedIndex != 0) statusInfo += " [" + std::string(speedNames[selectedSpeedIndex]) + "]";
            DrawString(10, 30, statusInfo.c_str(), speedColors[selectedSpeedIndex]);
            DrawString(10, 50, "ESCキー / Pキー: ポーズ", GetColor(200, 200, 200));

            DrawFormatString(10, 690, GetColor(255, 255, 255), "FPS: %.1f", fpsManager.GetFps());

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
                unsigned int msgColor = (resultMessage == "FAULT!") ? GetColor(255, 200, 0) : GetColor(255, 50, 50);
                DrawString(480, 300, resultMessage.c_str(), msgColor);
                SetFontSize(16);
                messageTimer--;
            }

            if (currentState == GameState::PAUSE) {
                SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
                DrawBox(0, 0, 1280, 720, GetColor(0, 0, 0), TRUE);
                SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

                DrawBox(440, 180, 840, 540, GetColor(30, 30, 40), TRUE);
                DrawBox(440, 180, 840, 540, GetColor(200, 200, 255), FALSE);

                SetFontSize(48);
                DrawString(530, 210, "PAUSE", GetColor(255, 255, 255));

                SetFontSize(24);
                const char* menuItems[] = { "RESUME", "RESTART", "TITLE" };
                for (int i = 0; i < 3; i++) {
                    int yPos = 310 + i * 60;
                    if (i == pauseMenuIndex) {
                        std::string selText = "> " + std::string(menuItems[i]) + " <";
                        DrawString(520, yPos, selText.c_str(), GetColor(255, 255, 0));
                    }
                    else {
                        DrawString(540, yPos, menuItems[i], GetColor(180, 180, 180));
                    }
                }

                SetFontSize(16);
                DrawString(510, 490, "W/S または 十字キー: 選択  ENTER: 決定", GetColor(200, 200, 200));
            }
        }
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

        enterKeyPrev = enterKey;
        aKeyPrev = aKey;
        dKeyPrev = dKey;
        wKeyPrev = wKey;
        sKeyPrev = sKey;

        leftKeyPrev = leftKey;
        rightKeyPrev = rightKey;
        upKeyPrev = upKey;
        downKeyPrev = downKey;

        rKeyPrev = rKey;
        shiftKeyPrev = shiftKey;
        escKeyPrev = escKey;
        pKeyPrev = pKey;

        mouseLeftPrev = mouseLeft;

        ScreenFlip();

        fpsManager.Wait();
    }

    if (titleBgHandle != -1) DeleteGraph(titleBgHandle);
    if (gameBgHandle != -1)  DeleteGraph(gameBgHandle);

    DxLib_End();
    return 0;
}