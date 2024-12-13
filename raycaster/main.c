#include <raylib.h>
#include "raymath.h"
#include <stdlib.h>
#include <time.h>

#define M_PI 3.14159265358979323846  /* pi */
#define SCREENWIDTH 1920
#define SCREENHEIGHT 720
#define MAPSIZE 720
#define BLOCKSIZE 36
#define RAYCASTPIXELWIDTH 8
#define VIEWANGLE 60.0 // in degrees

#define LIGHTBLUE (Color){ 135, 206, 235, 255 }

#define RAYNUMBER ( (SCREENWIDTH - MAPSIZE) / RAYCASTPIXELWIDTH )
#define MAPYOFFSET ( (SCREENHEIGHT - MAPSIZE) / 2 )
#define HALFVIEWANGLE (VIEWANGLE / 2)

enum Quadrant {
    UP_RIGHT,
    UP_LEFT,
    DOWN_LEFT,
    DOWN_RIGHT,
};

typedef enum Quadrant Quadrant;

const int map[MAPSIZE / BLOCKSIZE][MAPSIZE / BLOCKSIZE] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1},
    {1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

struct Player {
    Vector2 pos;
    Vector2 angle;
    int speed;
    int size;
};

typedef struct Player Player;

void drawMap() {
    for (int i = 0; i < MAPSIZE/BLOCKSIZE; i++)
        for (int j = 0; j < MAPSIZE/BLOCKSIZE; j++)
            DrawRectangle(
                BLOCKSIZE * j,
                MAPYOFFSET + BLOCKSIZE * i,
                BLOCKSIZE - 1,
                BLOCKSIZE - 1,
                map[i][j] ? GRAY : BLACK
            );
}

void drawPlayer(Player player) {
    DrawCircleV(player.pos, player.size, YELLOW);
    // DrawLineV(player.pos, Vector2Add(player.pos, Vector2Scale(player.angle, player.size * 3)), YELLOW);
}

bool isInMap(Vector2 pixel_pos) {
    return 0 < pixel_pos.x && pixel_pos.x < MAPSIZE
        && MAPYOFFSET < pixel_pos.y
        && pixel_pos.y < SCREENHEIGHT - MAPYOFFSET;
}

bool isBlock(Vector2 pixel_pos) {
    if (!isInMap(pixel_pos))
        return false;
    if (pixel_pos.x > MAPSIZE)
        return 0;
    int block_column = pixel_pos.x / BLOCKSIZE;
    int block_row = (pixel_pos.y - MAPYOFFSET) / BLOCKSIZE;
    return map[block_row][block_column] > 0;
}

bool isBlockFuzzy(Vector2 pixel_pos) {
    return (isBlock(Vector2Add(pixel_pos, (Vector2){+1,+1}))
         || isBlock(Vector2Add(pixel_pos, (Vector2){-1,+1}))
         || isBlock(Vector2Add(pixel_pos, (Vector2){+1,-1}))
         || isBlock(Vector2Add(pixel_pos, (Vector2){-1,-1})));
}

bool isInBlock(Player player) {
    Vector2 front_pixel = Vector2Add(player.pos, Vector2Scale(player.angle, 3 * player.size));
    return isBlock(front_pixel);
}

bool isInWindow(Vector2 pixel_pos) {
    return 0 < pixel_pos.x && pixel_pos.x < SCREENWIDTH
        && 0 < pixel_pos.y && pixel_pos.y < SCREENHEIGHT;
}

bool playerInBlock(Vector2 next_pos, int size) {
    Vector2 angle = Vector2Scale((Vector2){1, 0}, size);

    for (int i = 0; i < 360; i += 45)
        if (isBlock(Vector2Add(next_pos, Vector2Rotate(angle, DEG2RAD * i))))
            return true;
    
    return false;
}

Player movePlayer(Player player) {
    Vector2 next_pos = player.pos;
    if (IsKeyDown(KEY_UP))
        next_pos = Vector2Add(player.pos, Vector2Scale(player.angle, player.speed));
    else if (IsKeyDown(KEY_DOWN))
        next_pos = Vector2Add(player.pos, Vector2Scale(player.angle, -1 * player.speed));

    if (playerInBlock(next_pos, player.size))
        player.pos = player.pos;
    else
        player.pos = next_pos;

    if (IsKeyDown(KEY_RIGHT))
        player.angle = Vector2Rotate(player.angle, DEG2RAD * player.speed);
    else if (IsKeyDown(KEY_LEFT))
        player.angle = Vector2Rotate(player.angle, - DEG2RAD * player.speed);
    return player;
}

int roundToBlockSize(float number) {
    return BLOCKSIZE * ((int)(number + BLOCKSIZE / 2) / BLOCKSIZE);
}

// This will return a Vector3 of the form {end_point.x, end_point.y, wall_type}
Vector3 rayEndPoint(Player player, float angle_offset) {
    Vector2 angle = Vector2Rotate(player.angle, angle_offset);
    float absolute_angle = Vector2Angle(angle, (Vector2){1.0, 0.0});
    Vector2 first_vertical_wall = player.pos;
    Vector2 first_horizontal_wall = player.pos;
    Quadrant facing;

    if (0 <= absolute_angle && absolute_angle < M_PI / 2)
        facing = UP_RIGHT;
    else if (0 <= absolute_angle && absolute_angle < M_PI)
        facing = UP_LEFT;
    else if (- M_PI / 2 <= absolute_angle && absolute_angle < 0)
        facing = DOWN_RIGHT;
    else
        facing = DOWN_LEFT;

    if (facing == UP_LEFT || facing == DOWN_LEFT)
    {
        first_vertical_wall.x = player.pos.x - ((int)player.pos.x % BLOCKSIZE);
        first_vertical_wall.y = player.pos.y + tan(absolute_angle) * ((int)player.pos.x % BLOCKSIZE);
        first_vertical_wall = Vector2Add(first_vertical_wall, (Vector2){-1,0});
        Vector2 delta = (Vector2){ - BLOCKSIZE, tan(absolute_angle) * BLOCKSIZE};

        while (isInMap(first_vertical_wall) && !isBlockFuzzy(first_vertical_wall))
            first_vertical_wall = Vector2Add(first_vertical_wall, delta);
        first_vertical_wall = Vector2Add(first_vertical_wall, (Vector2){1,0});
    }
    else
    {
        first_vertical_wall.x = player.pos.x + (BLOCKSIZE - ((int)player.pos.x % BLOCKSIZE));
        first_vertical_wall.y = player.pos.y - tan(absolute_angle) * (BLOCKSIZE - ((int)player.pos.x % BLOCKSIZE));
        Vector2 delta = (Vector2){ BLOCKSIZE, - tan(absolute_angle) * BLOCKSIZE};

        while (isInMap(first_vertical_wall) && !isBlockFuzzy(first_vertical_wall))
            first_vertical_wall = Vector2Add(first_vertical_wall, delta);
        first_vertical_wall = Vector2Add(first_vertical_wall, (Vector2){-1,0});
    }

    if (facing == UP_LEFT || facing == UP_RIGHT)
    {
        first_horizontal_wall.y = player.pos.y - ((int)player.pos.y % BLOCKSIZE);
        first_horizontal_wall.x = player.pos.x - tan(absolute_angle - M_PI / 2) * ((int)player.pos.y % BLOCKSIZE);
        first_horizontal_wall = Vector2Add(first_horizontal_wall, (Vector2){0, -1});
        Vector2 delta = (Vector2){ - tan(absolute_angle - M_PI / 2) * BLOCKSIZE, - BLOCKSIZE};

        while (isInMap(first_horizontal_wall) && !isBlockFuzzy(first_horizontal_wall))
            first_horizontal_wall = Vector2Add(first_horizontal_wall, delta);
        first_horizontal_wall = Vector2Add(first_horizontal_wall, (Vector2){0, 1});
    }
    else
    {
        first_horizontal_wall.y = player.pos.y + (BLOCKSIZE - ((int)player.pos.y % BLOCKSIZE));
        first_horizontal_wall.x = player.pos.x + tan(absolute_angle - M_PI / 2) * (BLOCKSIZE - ((int)player.pos.y % BLOCKSIZE));
        Vector2 delta = (Vector2){ tan(absolute_angle - M_PI / 2) * BLOCKSIZE, BLOCKSIZE};

        while (isInMap(first_horizontal_wall) && !isBlockFuzzy(first_horizontal_wall))
            first_horizontal_wall = Vector2Add(first_horizontal_wall, delta);
        first_vertical_wall = Vector2Add(first_vertical_wall, (Vector2){0,-1});
    }

    // This is very hacky way to fix the inaccuracy of the algorithm. The algorithm should be rewritten from scratch.
    if (isBlock(Vector2Add(first_horizontal_wall, (Vector2){ (facing == UP_RIGHT || facing == DOWN_RIGHT) ? 1 : -1, -5.0}))
     && isBlock(Vector2Add(first_horizontal_wall, (Vector2){ (facing == UP_RIGHT || facing == DOWN_RIGHT) ? 1 : -1, +5.0})))
        return (Vector3){first_vertical_wall.x, first_vertical_wall.y, 0};

    if (isBlock(Vector2Add(first_vertical_wall, (Vector2){ -5.0, (facing == UP_LEFT || facing == UP_RIGHT) ? -1 : 2}))
     && isBlock(Vector2Add(first_vertical_wall, (Vector2){ +5.0, (facing == UP_LEFT || facing == UP_RIGHT) ? -1 : 2})))
        return (Vector3){first_horizontal_wall.x, first_horizontal_wall.y, 1};

    if (Vector2Distance(player.pos, first_horizontal_wall) < Vector2Distance(player.pos, first_vertical_wall))
        return (Vector3){first_horizontal_wall.x, first_horizontal_wall.y, 1};
    else
        return (Vector3){first_vertical_wall.x, first_vertical_wall.y, 0};
}

// Simple version, not in use
Vector2 rayEndPointSimple(Player player, float angle_offset) {
    Vector2 angle = Vector2Rotate(player.angle, angle_offset);
    Vector2 end_point = player.pos;
    while (!isBlock(end_point) && isInMap(end_point))
        end_point = Vector2Add(end_point, angle);
    return end_point;
}

void drawColumn(float block_distance, int view_column, Color color) {
    int view_width = (SCREENWIDTH - MAPSIZE);
    int view_center = MAPSIZE + (view_width / 2);
    int column_offset = view_column - (RAYNUMBER / 2);

    int column_height = SCREENHEIGHT / block_distance;
    int yOffSet = (SCREENHEIGHT - column_height) / 2;

    DrawRectangleGradientV(view_center + column_offset * RAYCASTPIXELWIDTH, 0, RAYCASTPIXELWIDTH, SCREENHEIGHT / 2, BLUE, LIGHTBLUE);
    DrawRectangleGradientV(view_center + column_offset * RAYCASTPIXELWIDTH, SCREENHEIGHT / 2, RAYCASTPIXELWIDTH, SCREENHEIGHT / 2, DARKBROWN, BROWN);
    DrawRectangle(view_center + column_offset * RAYCASTPIXELWIDTH, yOffSet, RAYCASTPIXELWIDTH, column_height, color);
}

bool horizontal_hit(Vector2 pixel_pos) {
    pixel_pos.y -= MAPYOFFSET;
    pixel_pos.x /= BLOCKSIZE;
    pixel_pos.y /= BLOCKSIZE;
    pixel_pos.x -= (int)pixel_pos.x;
    pixel_pos.y -= (int)pixel_pos.y;
    if ((pixel_pos.x < 0.02 || pixel_pos.x > 0.98)
    && !(pixel_pos.y < 0.02 || pixel_pos.y > 0.98))
        return true;
    return false;
}

int main() {
    srand(time(NULL));
    int frameCounter = 0;
    Player player;
    player.pos = (Vector2){MAPSIZE / 2, SCREENHEIGHT / 2};
    player.angle = (Vector2){0.0, -1.0};
    player.speed = 2;
    player.size = 8;
    Vector2 ray_end_point;
    bool ray_hit_horizontal;

    InitWindow(SCREENWIDTH, SCREENHEIGHT, "Raycaster");
    SetTargetFPS(60);

    while (WindowShouldClose() != true)
    {
        player = movePlayer(player);

        BeginDrawing();
        ClearBackground(BLACK);
        drawMap();
        drawPlayer(player);

        for (int view_column = 0; view_column < RAYNUMBER; view_column++)
        {
            float angle = - HALFVIEWANGLE + VIEWANGLE * view_column / RAYNUMBER;
            angle *= DEG2RAD;
            Vector3 result = rayEndPoint(player, angle);
            ray_end_point = (Vector2){ result.x, result.y };
            ray_hit_horizontal = result.z;
    
            float distance = Vector2Distance(player.pos, ray_end_point);
            distance *= cos(angle); // correct for fish eye effect
            float block_distance = (distance / BLOCKSIZE);
            
            if (view_column % 4 == 0)
                DrawLineV(Vector2Add(player.pos, Vector2Scale(Vector2Rotate(player.angle, angle), player.size)), ray_end_point, RED);

            if (ray_hit_horizontal)
                drawColumn(distance / BLOCKSIZE, view_column, (Color){0, Lerp(50, 225, Clamp(1.0f / block_distance, 0.0, 1.0)), 0, 255});
            else
                drawColumn(distance / BLOCKSIZE, view_column, (Color){0, 15 + Lerp(50, 225, Clamp(1.0f / block_distance, 0.0, 1.0)), 0, 255});
        }

        /* DrawFPS(10, 10); */
        EndDrawing();

        frameCounter++;
    }

    CloseWindow();
    return 0;
}
