#include <raylib.h>
#include "raymath.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#define DRAG_COEFFICIENT .01
#define FRICTION_COEFFICIENT 1
#define MAX_BALL_COUNT 50
#define GRAVITY_COEFFICIENT .4
const int screenWidth = 1200;
const int screenHeight = 800;

enum Mode {
    NORMAL,
    GRANNY_IN,
    EXTREME_IN,
    EXTREME,
    WON,
    LOST
};

enum Mode mode = NORMAL;

Color randomColor() {
    Color colors[] = {BLUE, RED, YELLOW, PURPLE, ORANGE, PINK};
    return colors[rand() % 6];
}

struct Ball {
    Vector2 pos;
    Vector2 vel;
    int radius;
    Color color;
};

typedef struct Ball Ball;

struct Paddle {
    Vector2 pos;
    int height;
    int width;
};

typedef struct Paddle Paddle;

double distance(Vector2 vec1, Vector2 vec2) {
    double xdiff = (vec2.x - vec1.x);
    double ydiff = (vec2.y - vec1.y);
    return sqrt(xdiff * xdiff + ydiff * ydiff);
}

void drawGranny(int x, int y, int scale) {
    Color B = BEIGE;
    Color BB = BLACK;
    Color BL = BLUE;
    Color R = RED;
    Color Y = YELLOW;
    Color N = BLANK; // background color
    Color G = GRAY;
    Color L = LIGHTGRAY;
    Color W = WHITE;
    Color drawing[26][16] =
    {
        {N, N, N, N, N, N, N, N, L, L, L, N, N, N, N, N},
        {N, N, N, N, N, L, L, L, L, L, L, L, L, L, N, N},
        {N, N, N, N, L, L, L, L, L, L, L, L, L, L, N, N},
        {N, N, N, N, L, L, L, L, L, L, L, L, L, L, L, N},
        {N, N, N, L, L, L, L, L, L, L, L, L, L, L, L, L},
        {N, N, N, N, L, L, L, L, B, B, B, B, L, L, L, N},
        {N, N, N, N, N, L, L, B, B, B, B, B, B, L, L, N},
        {N, N, N, N, N, L, B, W, BL, B, W, BL, B, N, N, N},
        {N, N, N, N, N, N, B, B, B, B, B, B, B, N, N, N},
        {N, N, N, N, R, R, B, B, B, B, B, B, B, N, N, N},
        {N, N, N, N, R, R, B, B, B, R, B, B, B, N, N, N},
        {N, N, N, R, R, R, B, B, B, B, B, B, N, N, N, N},
        {N, N, N, R, R, R, R, B, B, N, N, N, N, N, N, N},
        {N, N, R, R, R, R, R, R, R, N, N, N, N, N, N, N},
        {N, N, R, R, R, R, R, R, B, N, N, N, N, N, N, N},
        {N, R, R, R, R, R, R, R, B, N, N, N, N, N, N, N},
        {N, R, R, R, R, R, R, R, B, B, B, B, B, N, N, N},
        {R, R, R, R, R, R, R, R, N, N, N, B, B, G, G, N},
        {R, R, R, R, R, R, R, R, R, N, G, N, N, N, N, G},
        {R, R, R, R, R, R, R, R, R, N, G, N, N, N, N, G},
        {R, R, R, R, R, R, R, R, R, N, G, N, N, N, N, G},
        {Y, Y, Y, Y, Y, Y, Y, Y, Y, N, G, G, G, G, G, G},
        {N, B, N, N, B, N, N, N, N, N, G, N, N, N, N, G},
        {N, B, N, N, B, N, N, N, N, N, G, N, N, N, N, G},
        {N, B, N, N, B, N, N, N, N, N, G, N, N, N, N, G},
        {N, BB, BB, N, BB, BB, BB, N, N, N, BB, N, N, N, N, BB}
    };

    for (int i = 0; i < 16 * scale; i++)
        for (int j = 0; j < 26 * scale; j++)
        {
            DrawPixel(x + i, y + j, drawing[j / scale][i / scale]);
        }
}

Ball spawnBall(Color color) {
    Ball ball;
    ball.color = color;
    ball.radius = 20;
    ball.pos = (Vector2){screenWidth / 2, screenHeight / 2};
    ball.vel = (Vector2){ rand() % 10 - 5, rand() % 10 - 5};
    while (ball.vel.x == 0)
        ball.vel.x = rand() % 10 - 5;
    ball.vel = Vector2Scale(Vector2Normalize(ball.vel), 10.0);
    return ball;
}

void drawField() {
    ClearBackground(GREEN);
    DrawCircle(screenWidth / 2, screenHeight / 2, 150.0f, WHITE);
    DrawCircle(screenWidth / 2, screenHeight / 2, 148.0f, GREEN);
    DrawRectangle(screenWidth / 2 - 1, 0, 2, screenHeight, WHITE);
}

void drawPaddles(Paddle playerPaddle, Paddle computerPaddle) {
    DrawRectangle(playerPaddle.pos.x - playerPaddle.width / 2, playerPaddle.pos.y - playerPaddle.height / 2, playerPaddle.width, playerPaddle.height, WHITE);
    DrawRectangle(computerPaddle.pos.x - computerPaddle.width / 2, computerPaddle.pos.y - computerPaddle.height / 2, computerPaddle.width, computerPaddle.height, WHITE);
}

void drawBalls(Ball ballArray[], int ballCount) {
    for (int i = 0; i < ballCount; i++)
        DrawCircle(ballArray[i].pos.x, ballArray[i].pos.y, ballArray[i].radius, ballArray[i].color);
}

void grannyInAnimation(int frameCounter) {
    if (IsKeyDown(KEY_SPACE) && frameCounter > 360)
    {
        mode = EXTREME_IN;
        return;
    }
    BeginDrawing();
    drawField();
    if (frameCounter < 180)
        drawGranny(screenWidth / 4 + 50, frameCounter / 2.0f * 3, 4);
    else
    {
        drawGranny(screenWidth / 4 + 50, 90 * 3, 4);
        DrawText("Don't do it! My husband died playing extreme pong!", screenWidth / 15, 90 * 3 - 60, 32, PURPLE);
    }

    if (frameCounter > 360) {
        DrawText("This old woman obviously has no idea what she's talking about!\nPress [SPACE] to ignore her and enjoy pong TO THE EXTREME!", screenWidth / 15, 5 * screenHeight / 6, 32, RED);
    }
    EndDrawing();
}

int main() {
    srand(time(NULL));

    Ball ballArray[MAX_BALL_COUNT];
    char message[200];
    int ballCount = 0;
    int playerScore = 0;
    int computerScore = 0;

    Paddle playerPaddle;
    playerPaddle.height = 150;
    playerPaddle.width = 40;
    playerPaddle.pos = (Vector2){ 40, screenHeight / 2 };

    Paddle computerPaddle;
    computerPaddle.height = 150;
    computerPaddle.width = 40;
    computerPaddle.pos = (Vector2){ screenWidth - 50, screenHeight / 2 };

    ballArray[0] = spawnBall(WHITE);
    ballCount++;

    InitWindow(screenWidth, screenHeight, "Pong");
    SetTargetFPS(60);

    for (int frameCounter = 0; WindowShouldClose() != true; frameCounter++)
    {
        switch (mode) {
            case GRANNY_IN:
                grannyInAnimation(frameCounter);
                continue;
            case EXTREME_IN:
                if (frameCounter > 180) 
                {
                    frameCounter = 0;
                }
                else if (frameCounter < 60)
                {
                    playerPaddle.pos.y = screenHeight / 2;
                    computerPaddle.pos.y = screenHeight / 2;
                    playerPaddle.height = 150 + 300 * frameCounter / 60.0f;
                    computerPaddle.height = 150 + 300 * frameCounter / 60.0f;
                }
                else if (frameCounter == 60)
                {
                    ballArray[0] = spawnBall(randomColor());
                    ballArray[0].vel = (Vector2){ -1, -0.5 };
                    ballArray[0].vel = Vector2Scale(Vector2Normalize(ballArray[0].vel), 10.0);
                    ballArray[1] = spawnBall(randomColor());
                    ballCount = 2;
                }
                else if (frameCounter < 180)
                {
                    ;
                }
                else
                {
                    mode = EXTREME;
                    frameCounter = 0;
                }
                break;
            case NORMAL: case EXTREME: case WON: case LOST:
                break;
        }

        if (mode != LOST && playerScore >= 20)
            mode = WON;
        else if (mode != WON && computerScore >= 20)
            mode = LOST;

        if (IsKeyDown(KEY_SPACE) && mode == NORMAL && frameCounter > 15 * 60)
        {
            mode = GRANNY_IN;
            ballCount = 0;
            frameCounter = 0;
            continue;
        }

        if (mode == EXTREME && ballCount < MAX_BALL_COUNT && frameCounter % 120 == 0) {
            ballArray[ballCount] = spawnBall(randomColor());
            ballCount++;
        }

        if (IsKeyDown(KEY_DOWN))
            playerPaddle.pos.y += 7;
        else if (IsKeyDown(KEY_UP))
            playerPaddle.pos.y -= 7;

        /* COLLISION BETWEEEN BALL AND PADDLE*/

        for (int i = 0; i < ballCount; i++) {
            // Find the closest point to the circle within the rectangle
            Vector2 closestPointPlayerPaddle = (Vector2){
                Clamp(ballArray[i].pos.x, playerPaddle.pos.x - playerPaddle.width / 2, playerPaddle.pos.x + playerPaddle.width / 2),
                Clamp(ballArray[i].pos.y, playerPaddle.pos.y - playerPaddle.height / 2, playerPaddle.pos.y + playerPaddle.height / 2),
            };

            // A collision has occurred
            if (distance(closestPointPlayerPaddle, ballArray[i].pos) < ballArray[i].radius)
            {
                ballArray[i].pos = Vector2Add(closestPointPlayerPaddle, Vector2Scale(Vector2Normalize(Vector2Subtract(ballArray[i].pos, closestPointPlayerPaddle)), ballArray[i].radius));
                ballArray[i].vel = Vector2Reflect(ballArray[i].vel, Vector2Normalize(Vector2Subtract(ballArray[i].pos, closestPointPlayerPaddle)));
                if (0 < ballArray[i].vel.x && ballArray[i].vel.x < 3)
                    ballArray[i].vel.x = 3;
            }

            // Find the closest point to the circle within the rectangle
            Vector2 closestPointComputerPaddle = (Vector2){
                Clamp(ballArray[i].pos.x, computerPaddle.pos.x - computerPaddle.width / 2, computerPaddle.pos.x + computerPaddle.width / 2),
                Clamp(ballArray[i].pos.y, computerPaddle.pos.y - computerPaddle.height / 2, computerPaddle.pos.y + computerPaddle.height / 2),
            };

            // A collision has occurred
            if (distance(closestPointComputerPaddle, ballArray[i].pos) < ballArray[i].radius)
            {
                ballArray[i].pos = Vector2Add(closestPointComputerPaddle, Vector2Scale(Vector2Normalize(Vector2Subtract(ballArray[i].pos, closestPointComputerPaddle)), ballArray[i].radius));
                ballArray[i].vel = Vector2Reflect(ballArray[i].vel, Vector2Normalize(Vector2Subtract(ballArray[i].pos, closestPointComputerPaddle)));
                if (-3 < ballArray[i].vel.x && ballArray[i].vel.x < 0)
                    ballArray[i].vel.x = -3;
            }
        }

        /* END COLLISION BETWEEEN BALL AND PADDLE*/

        /* COLLISION BETWEEN BALLs */

        for (int i = 0; i < ballCount; i++)
            for (int j = i; j < ballCount; j++)
                if (distance(ballArray[i].pos, ballArray[j].pos) < ballArray[i].radius + ballArray[j].radius) {
                    ballArray[i].vel = Vector2Scale(Vector2Reflect(ballArray[i].vel, Vector2Normalize(Vector2Subtract(ballArray[j].pos, ballArray[i].pos))), FRICTION_COEFFICIENT);
                    ballArray[j].vel = Vector2Scale(Vector2Reflect(ballArray[j].vel, Vector2Normalize(Vector2Subtract(ballArray[i].pos, ballArray[j].pos))), FRICTION_COEFFICIENT);
                    float push_distance = ballArray[i].radius + ballArray[j].radius - distance(ballArray[i].pos, ballArray[j].pos);
                    push_distance /= 2.0f;
                    ballArray[i].pos = Vector2Add(ballArray[i].pos, Vector2Scale(Vector2Normalize(Vector2Subtract(ballArray[i].pos, ballArray[j].pos)), push_distance));
                    ballArray[j].pos = Vector2Add(ballArray[j].pos, Vector2Scale(Vector2Normalize(Vector2Subtract(ballArray[j].pos, ballArray[i].pos)), push_distance));
                }

        /* END COLLISION BETWEEN BALLs */

        // AI with the paddle
        {
            float averageBallPosition = 0.0f;
            int numOfBalls = 0;

            for (int i = 0; i < ballCount; i++)
            {
                if (ballArray[i].pos.x > screenWidth / 2)
                {
                    averageBallPosition += ballArray[i].pos.y;
                    numOfBalls++;
                }
            }

            averageBallPosition /= (float)numOfBalls;

            if (numOfBalls == 0)
                ;
            else if (averageBallPosition < computerPaddle.pos.y - 10)
                computerPaddle.pos.y -= 7;
            else if (averageBallPosition > computerPaddle.pos.y + 10)
                computerPaddle.pos.y += 7;
        }

        for (int i = 0; i < ballCount; i++)
            ballArray[i].pos = Vector2Add(ballArray[i].pos, ballArray[i].vel);

        // Collision between ball and walls
        for (int i = 0; i < ballCount; i++)
        {
            if (ballArray[i].pos.y + ballArray[i].radius > screenHeight || ballArray[i].pos.y - ballArray[i].radius < 0)
                ballArray[i].vel.y *= -1.0f;
            if (ballArray[i].pos.x + ballArray[i].radius > screenWidth)
            {
                ballArray[i] = spawnBall(ballArray[i].color);
                playerScore++;
            }
            else if (ballArray[i].pos.x - ballArray[i].radius < 0)
            {
                ballArray[i] = spawnBall(ballArray[i].color);
                computerScore++;
            }
        }

        for (int i = 0; i < ballCount; i++)
            ballArray[i].pos.y = Clamp(ballArray[i].pos.y, ballArray[i].radius, screenHeight - ballArray[i].radius);

        playerPaddle.pos.y = Clamp(playerPaddle.pos.y, playerPaddle.height / 2, screenHeight - playerPaddle.height / 2);
        computerPaddle.pos.y = Clamp(computerPaddle.pos.y,  computerPaddle.height / 2, screenHeight - computerPaddle.height / 2);

        BeginDrawing();

        drawField();
        drawPaddles(playerPaddle, computerPaddle);
        drawBalls(ballArray, ballCount);

        DrawText("The first person to score 20 points wins", 20, 10, 20, WHITE);

        if (mode == EXTREME || mode == EXTREME_IN) {
            int xOffset = rand() % 10 - 5;
            int yOffset = rand() % 10 - 5;
            DrawText("EXTREME PONG!", screenWidth / 4 + xOffset, screenHeight / 5 + yOffset, 64, randomColor());
        } else if (mode == WON) {
            DrawText("You won! :)", screenWidth / 2 - 125, screenHeight / 5, 64, WHITE);
        } else if (mode == LOST) {
            DrawText("You lost! :(", screenWidth / 2 - 125, screenHeight / 5, 64, WHITE);
        } else if (mode == NORMAL && frameCounter > 15 * 60) {
            DrawText("Press [SPACE] to play EXTREME pong (if you dare!)", 20, screenHeight - 30, 20, RED);
        }

        if (mode == EXTREME_IN && frameCounter < 78) {
            drawGranny(screenWidth / 4 + 50, 90 * 3, 4);
        } else if (mode == EXTREME_IN && frameCounter < 180) {
            drawGranny(screenWidth / 4 + 50 - 20*(frameCounter - 78), 90 * 3 - 20*(frameCounter - 78), 4);
        }

        sprintf(message, "Your score: %d", playerScore);
        DrawText(message, screenWidth / 8, screenHeight / 8, 32, BLACK);
        sprintf(message, "Computer's score: %d", computerScore);
        DrawText(message, 5 * (screenWidth / 8), screenHeight / 8, 32, BLACK);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
