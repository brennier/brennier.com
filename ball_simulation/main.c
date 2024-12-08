#include <raylib.h>
#include "raymath.h"
#include <stdlib.h>
#include <stdio.h> 
#include <math.h>

#define DRAG_COEFFICIENT .01
#define FRICTION_COEFFICIENT 1
#define MAX_BALL_COUNT 100
#define GRAVITY_COEFFICIENT .4
const int screenWidth = 1200;
const int screenHeight = 800;

int max(int a, int b)
{
    return (a > b) ? a : b;
}

int min(int a, int b)
{
    return (a > b) ? a : b;
}

struct Ball {
    Vector2 pos;
    Vector2 vel;
    Vector2 acc;
    int radius;
    Color color;
};

typedef struct Ball Ball;

Ball init_ball() {
  Ball const b = {
    .pos = (Vector2){0.0, 0.0},
    .vel = (Vector2){0.0, 0.0},
    .acc = (Vector2){0.0, GRAVITY_COEFFICIENT},
    .radius = 20,
    .color = WHITE
    };
  return b;
}

Ball ballUpdatePhysics(Ball ball) {
    ball.vel = Vector2Add(ball.vel, ball.acc);
    ball.pos = Vector2Add(ball.pos, ball.vel);
    ball.vel = Vector2Scale(ball.vel, 1.0 - DRAG_COEFFICIENT);
    return ball;
}

Ball ballClampToScreen(Ball ball) {
    ball.pos = (Vector2){
        Clamp(ball.pos.x, ball.radius, screenWidth - ball.radius),
        Clamp(ball.pos.y, ball.radius, screenHeight - ball.radius)
    };
    return ball;
}

double distance(Vector2 vec1, Vector2 vec2) {
    double xdiff = (vec2.x - vec1.x);
    double ydiff = (vec2.y - vec1.y);
    return sqrt(xdiff * xdiff + ydiff * ydiff);
}

int main() {
    srand(0);
    char message[200];

    Ball ballArray[MAX_BALL_COUNT];
    int ballCount = 0;

    struct Ball ball1 = init_ball();
    ball1.pos = (Vector2){screenWidth / 2, screenHeight / 2};
    ball1.color = RED;
    ballArray[0] = ball1;
    ballCount++;

    int inputDelay = 15;
    InitWindow(screenWidth, screenHeight, "Basic ball program");
    SetTargetFPS(60);

    while (WindowShouldClose() != true)
    {
        if (IsKeyDown(KEY_SPACE) && inputDelay == 0)
        {
            for (int i = 0; i < ballCount; i++)
                ballArray[i].acc = (Vector2){ ballArray[i].acc.y, - ballArray[i].acc.x};
            //ball1.acc = Vector2Rotate(ball1.acc, -3.14 / 2);
            inputDelay = 15;
        }
        else if (inputDelay > 0)
        {
            inputDelay -= 1;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && ballCount < MAX_BALL_COUNT)
        {
            Color ballColors[] = {WHITE, BLUE, RED, YELLOW, PURPLE, ORANGE, PINK, BROWN};
            Ball newBall = init_ball();
            newBall.pos = GetMousePosition();
            newBall.radius = (rand() % 40) + 10;
            newBall.color = ballColors[rand() % 8];
            newBall.acc = ballArray[0].acc;
            ballArray[ballCount] = newBall;
            ballCount++;
        }

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

        for (int i = 0; i < ballCount; i++)
            ballArray[i] = ballUpdatePhysics(ballArray[i]);

        for (int i = 0; i < ballCount; i++)
        {
            if (ballArray[i].pos.x + ballArray[i].radius > screenWidth || ballArray[i].pos.x - ballArray[i].radius < 0)
                ballArray[i].vel = Vector2Scale(Vector2Reflect(ballArray[i].vel, (Vector2){ 1, 0 }), FRICTION_COEFFICIENT);
            if (ballArray[i].pos.y + ballArray[i].radius > screenHeight || ballArray[i].pos.y - ballArray[i].radius < 0)
                ballArray[i].vel = Vector2Scale(Vector2Reflect(ballArray[i].vel, (Vector2){ 0, 1 }), FRICTION_COEFFICIENT);
        }

        for (int i = 0; i < ballCount; i++)
            ballArray[i] = ballClampToScreen(ballArray[i]);

        BeginDrawing();
        ClearBackground(GREEN);

        for (int i = 0; i < ballCount; i++)
            DrawCircle(ballArray[i].pos.x, ballArray[i].pos.y, ballArray[i].radius, ballArray[i].color);

        if (ballArray[0].acc.y > 0 && ballArray[0].acc.x == 0)
        {
            DrawRectangle(50, 50, 50, 100, BLACK);
            DrawTriangle(
                (Vector2){125, 150},
                (Vector2){25, 150},
                (Vector2){75, 200},
                BLACK
            );
        }
        else if (ballArray[0].acc.y < 0 && ballArray[0].acc.x == 0)
        {
            DrawRectangle(50, 100, 50, 100, BLACK);
            DrawTriangle(
                (Vector2){25, 100},
                (Vector2){125, 100},
                (Vector2){75, 50},
                BLACK
            );
        }
        else if (ballArray[0].acc.y == 0 && ballArray[0].acc.x > 0)
        {
            DrawRectangle(50, 50, 100, 50, BLACK);
            DrawTriangle(
                (Vector2){150, 25},
                (Vector2){150, 125},
                (Vector2){200, 75},
                BLACK
            );
        }
        else
        {
            DrawRectangle(100, 50, 100, 50, BLACK);
            
            DrawTriangle(
                (Vector2){100, 125},
                (Vector2){100, 25},
                (Vector2){50, 75},
                BLACK
            );
        }

        sprintf(message, "There are %.2d balls", ballCount);
        DrawText(message, 20, screenHeight - 20, 20, BLACK);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}