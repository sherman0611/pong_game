#include <raylib.h>
#include <iostream>

using namespace std;

enum GameState {
    TITLE,
    GAME1P,
    GAME2P
};

const int screenWidth = 1280;
const int screenHeight = 800;

class Ball {
    public:
        float x, y, radius, speedX, speedY;

        float resetDelayTimer = 0.5f;
        float resetDelayDuration = 0.5f;

        void Draw() {
            DrawCircle(x, y, radius, WHITE);
        }

        void Update() {
            // countdown
            if (resetDelayTimer > 0.0f) {
                resetDelayTimer -= GetFrameTime();
                return;
            }

            // move ball
            x += speedX;
            y += speedY;

            // bounce ball at border
            if (y + radius >= GetScreenHeight() || y - radius <= 0) {
                speedY *= -1;
            }
        }

        void Reset() {
            x = screenWidth / 2;
            y = screenHeight / 2;

            int speedChoices[2] = {-1, 1};
            speedX *= speedChoices[GetRandomValue(0, 1)];
            speedY *= speedChoices[GetRandomValue(0, 1)];

            resetDelayTimer = resetDelayDuration;
        }
};

class Paddle {
    protected:
        void StopAtBorder() {
            if (y <= 0) {
                y = 0;
            }
            if (y + height >= GetScreenHeight()) {
                y = GetScreenHeight() - height;
            }
        }

    public:
        float x, y, width, height, speed;

        void Draw() {
            DrawRectangleRounded(Rectangle{x, y, width, height}, 0.8, 0, WHITE);
        }

        void Update(KeyboardKey up, KeyboardKey down) {
            if (IsKeyDown(up)) {
                y = y - speed;
            }
            if (IsKeyDown(down)) {
                y = y + speed;
            }
            StopAtBorder();
        }
};

class CpuPaddle: public Paddle {
    public:
        void Update(float ballY) {
            if (y + height/2 > ballY) {
                y -= speed;
            }
            if (y + height/2 <= ballY) {
                y += speed;
            }
            StopAtBorder();
        }
};

class Button {
    public:
        Rectangle bounds;
        Color color;
        string text;

        Button(float width, float height, Color color, string text, float x = 0, float y = 0)
            : bounds({x, y, width, height}), color(color), text(text) {}

        bool IsPressed() {
            return CheckCollisionPointRec(GetMousePosition(), bounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
        }

        void Draw() {
            DrawRectangleRec(bounds, color);
            DrawText(text.c_str(), static_cast<int>(bounds.x + bounds.width / 2 - MeasureText(text.c_str(), 20) / 2),
                    static_cast<int>(bounds.y + bounds.height / 2 - 10), 20, BLACK);
        }
};

int player1Score = 0;
int player2Score = 0;
GameState gameState = TITLE;

Ball ball;
Paddle player1, player2;
CpuPaddle cpu;

Button startButton1P(300, 60, LIGHTGRAY, "Single Player", screenWidth / 2 - 150, screenHeight * 0.4);
Button startButton2P(300, 60, LIGHTGRAY, "Multiplayer", screenWidth / 2 - 150, screenHeight * 0.5);
Button exitButton(300, 60, LIGHTGRAY, "Exit Game", screenWidth / 2 - 150, screenHeight * 0.6);

void DrawTitleScreen() {
    const char *titleText = "Pong Game";

    float titleX = screenWidth / 2 - MeasureText(titleText, 60) / 2;
    float titleY = screenHeight / 4;
    DrawText(titleText, titleX, titleY, 60, WHITE);

    startButton1P.Draw();
    startButton2P.Draw();
    exitButton.Draw();
}

void Logic() {
    // player wins
    if (ball.x - ball.radius <= 0) {
        player1Score++;
        ball.Reset();
    }
    if (ball.x + ball.radius >= GetScreenWidth()) {
        player2Score++;
        ball.Reset();
    }

    // check collusion
    if (CheckCollisionCircleRec(Vector2{ball.x, ball.y}, ball.radius, Rectangle{player1.x, player1.y, player1.width, player1.height})) {
        ball.speedX *= -1;
    }
    if (gameState == GAME1P) {
        if (CheckCollisionCircleRec(Vector2{ball.x, ball.y}, ball.radius, Rectangle{cpu.x, cpu.y, cpu.width, cpu.height})) {
            ball.speedX *= -1;
        }
    }
    if (gameState == GAME2P) {
        if (CheckCollisionCircleRec(Vector2{ball.x, ball.y}, ball.radius, Rectangle{player2.x, player2.y, player2.width, player2.height})) {
            ball.speedX *= -1;
        }
    }
}

int main() {
    InitWindow(screenWidth, screenHeight, "Pong");
    SetTargetFPS(60);

    ball.x = screenWidth / 2;
    ball.y = screenHeight / 2;
    ball.radius = 20;
    int speedChoices[2] = {-10, 10};
    ball.speedX = speedChoices[GetRandomValue(0, 1)];
    ball.speedY = speedChoices[GetRandomValue(0, 1)];

    float paddleWidth = 25;
    float paddleHeight = 170;
    
    player1.width = paddleWidth;
    player1.height = paddleHeight;
    player1.x = screenWidth - player1.width - 10;
    player1.y = screenHeight / 2 - player1.height / 2;
    player1.speed = 9;

    cpu.width = paddleWidth;
    cpu.height = paddleHeight;
    cpu.x = 10;
    cpu.y = screenHeight / 2 - cpu.height / 2;
    cpu.speed = 8;

    player2.width = paddleWidth;
    player2.height = paddleHeight;
    player2.x = 10;
    player2.y = screenHeight / 2 - player2.height / 2;
    player2.speed = 9;

    while (WindowShouldClose() == false) {
        BeginDrawing();

        if (gameState == TITLE) {
            DrawTitleScreen();
            if (startButton1P.IsPressed()) {
                gameState = GAME1P;
            }
            if (startButton2P.IsPressed()) {
                gameState = GAME2P;
            }
            if (exitButton.IsPressed()) {
                CloseWindow();
                return 0;
            }
        } else {
            ball.Update();
            player1.Update(KEY_UP, KEY_DOWN);
            if (gameState == GAME1P) {
                cpu.Update(ball.y);
            } else if (gameState == GAME2P) {
                player2.Update(KEY_W, KEY_S);
            }

            Logic();

            // render screen
            ClearBackground(BLACK);
            ball.Draw();
            player1.Draw();
            if (gameState == GAME1P) {
                cpu.Draw();
            } else if (gameState == GAME2P) {
                player2.Draw();
            }
            DrawLine(screenWidth / 2, 0, screenWidth / 2, screenHeight, WHITE);
            DrawText(TextFormat("%i", player2Score), screenWidth / 4 - 20, 20, 80, WHITE);
            DrawText(TextFormat("%i", player1Score), 3 * screenWidth / 4 - 20, 20, 80, WHITE);
        }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}