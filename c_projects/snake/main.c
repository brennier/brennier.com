#include <raylib.h>
#include "raymath.h"
#include <stdlib.h>
#include <stdio.h> 
#include <time.h>

#define MAX_SNAKE_LENGTH 100
#define LIGHTGREEN (Color){ 120, 255, 120, 255 }
const int screenWidth = 800;
const int screenHeight = 800;

enum mode {
    NORMAL,
    SLOWMO,
};

enum Directions {
    RIGHT,
    LEFT,
    UP,
    DOWN
};

struct Snake {
    Vector2 positions[MAX_SNAKE_LENGTH];
    enum Directions direction;
    int length;
};

typedef struct Snake Snake;

void drawSnake(Snake snake) {
    for (int i = 0; i < snake.length; i++)
    {
        DrawRectangle(snake.positions[i].x * 16, snake.positions[i].y * 16, 16, 16, WHITE);
        // Draw the outline around the rectangle
        DrawRectangle(snake.positions[i].x * 16, snake.positions[i].y * 16, 16, 2, LIGHTGREEN);
        DrawRectangle(snake.positions[i].x * 16, snake.positions[i].y * 16, 2, 16, LIGHTGREEN);
        DrawRectangle(snake.positions[i].x * 16 + 14, snake.positions[i].y * 16, 2, 16, LIGHTGREEN);
        DrawRectangle(snake.positions[i].x * 16, snake.positions[i].y * 16 + 14, 16, 2, LIGHTGREEN);
    }
}

void updateSnake(Snake* snake) {
    for (int i = snake->length - 1; i >= 0; i--)
        snake->positions[i+1] = snake->positions[i];
    switch (snake->direction) {
        case UP:
            snake->positions[0] = Vector2Add(snake->positions[1], (Vector2){0,-1});
            break;
        case DOWN:
            snake->positions[0] = Vector2Add(snake->positions[1], (Vector2){0,1});
            break;
        case LEFT:
            snake->positions[0] = Vector2Add(snake->positions[1], (Vector2){-1,0});
            break;
        case RIGHT:
            snake->positions[0] = Vector2Add(snake->positions[1], (Vector2){1,0});
            break;
    }
}

Vector2 snakeHead(Snake snake) {
    return snake.positions[0];
}

int snakeIntersects(Snake snake) {
    for (int i = 1; i < snake.length; i++)
        if (Vector2Equals(snake.positions[0], snake.positions[i]))
            return true;
    return false;
}

int main() {
    srand(time(NULL));
    enum mode mode = NORMAL;
    char message[200];
    int frameCounter = 0;
    int powerup = 100;
    Vector2 apple = (Vector2){ rand() % 50, rand() % 40 + 10 };
    Snake snake;
    snake.positions[0] = (Vector2){25, 25};
    snake.direction = UP;
    snake.length = 1;

    InitWindow(screenWidth, screenHeight, "Snake");
    SetTargetFPS(60);

    while (WindowShouldClose() != true)
    {

        if (IsKeyDown(KEY_UP) && snake.direction != DOWN)
            snake.direction = UP;
        else if (IsKeyDown(KEY_DOWN) && snake.direction != UP)
            snake.direction = DOWN;
        else if (IsKeyDown(KEY_LEFT) && snake.direction != RIGHT)
            snake.direction = LEFT;
        else if (IsKeyDown(KEY_RIGHT) && snake.direction != LEFT)
            snake.direction = RIGHT;  

        if (IsKeyDown(KEY_SPACE) && powerup > 0)
            mode = SLOWMO;
        else
            mode = NORMAL;

        if (mode == SLOWMO && frameCounter % 8 == 0)
        {
            updateSnake(&snake);
            powerup -= 2;
        }
        
        if (mode == NORMAL && frameCounter % 2 == 0)
        {
            updateSnake(&snake);
            if (powerup < 100)
                powerup++;
        }

        if (snakeIntersects(snake))
            break;
        
        if (Vector2Equals(snakeHead(snake), apple))
        {
            apple = (Vector2){ rand() % 50, rand() % 40 + 10 };
            snake.length++;
        }

        // Check if snake head is out of bounds
        if (screenWidth / 16 <= snakeHead(snake).x || snakeHead(snake).x < 0)
            break;
        else if (screenHeight / 16 <= snakeHead(snake).y || snakeHead(snake).y < 0)
            break;

        BeginDrawing();
        ClearBackground(GREEN);
        drawSnake(snake);
        DrawRectangle(apple.x * 16, apple.y * 16, 16, 16, RED);
        DrawRectangle(20, 20, 510, 30, LIGHTGREEN);
        DrawRectangle(25, 25, 5*powerup, 20, ORANGE);
        DrawText("Press [Space] to slow down time", 20, 70, 24, BLACK);
        sprintf(message, "Score: %d", snake.length);
        DrawText(message, 20, screenHeight - 32, 32, BLACK);
        EndDrawing();

        frameCounter++;
    }

    // Shift the snake back one
    for (int i = 0; i < snake.length; i++)
        snake.positions[i] = snake.positions[i+1];

    while (WindowShouldClose() != true)
    {
        BeginDrawing();
        ClearBackground(GREEN);
        drawSnake(snake);
        sprintf(message, "Final score: %03d", snake.length);
        DrawText(message, screenWidth / 2 - 16*16, screenHeight / 2, 64, BLACK);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}