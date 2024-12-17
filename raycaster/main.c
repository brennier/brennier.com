#include <raylib.h>
#include "raymath.h"

#define M_PI 3.14159265358979323846  /* pi */
#define SCREENWIDTH 1920
#define SCREENHEIGHT 720
#define MAPSIZE 720
#define BLOCKSIZE 36
#define RAYCASTPIXELWIDTH 4
#define VIEWANGLE 60.0 // in degrees
#define TEXTURESIZE 64

#define LIGHTBLUE (Color){ 135, 206, 235, 255 }

#define RAYNUMBER ( (SCREENWIDTH - MAPSIZE) / RAYCASTPIXELWIDTH )
#define MAPYOFFSET ( (SCREENHEIGHT - MAPSIZE) / 2 )
#define HALFVIEWANGLE (VIEWANGLE / 2)

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

void draw_map() {
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

void draw_player(Player player) {
    DrawCircleV(player.pos, player.size, YELLOW);
    // DrawLineV(player.pos, Vector2Add(player.pos, Vector2Scale(player.angle, player.size * 3)), YELLOW);
}

bool isInMap(Vector2 pixel_pos) {
    return 0 < pixel_pos.x && pixel_pos.x < MAPSIZE
        && MAPYOFFSET < pixel_pos.y
        && pixel_pos.y < SCREENHEIGHT - MAPYOFFSET;
}

bool is_block(Vector2 pixel_pos) {
    if (!isInMap(pixel_pos))
        return false;
    if (pixel_pos.x > MAPSIZE)
        return false;
    int block_column = pixel_pos.x / BLOCKSIZE;
    int block_row = (pixel_pos.y - MAPYOFFSET) / BLOCKSIZE;
    return map[block_row][block_column] > 0;
}

bool isInBlock(Player player) {
    Vector2 front_pixel = Vector2Add(player.pos, Vector2Scale(player.angle, 3 * player.size));
    return is_block(front_pixel);
}

bool isInWindow(Vector2 pixel_pos) {
    return 0 < pixel_pos.x && pixel_pos.x < SCREENWIDTH
        && 0 < pixel_pos.y && pixel_pos.y < SCREENHEIGHT;
}

bool playerInBlock(Vector2 next_pos, int size) {
    Vector2 angle = Vector2Scale((Vector2){1, 0}, size);

    for (int i = 0; i < 360; i += 45)
        if (is_block(Vector2Add(next_pos, Vector2Rotate(angle, DEG2RAD * i))))
            return true;
    
    return false;
}

Player movePlayer(Player player, float time_delta) {
    Vector2 next_pos = player.pos;
    float speed = player.speed * time_delta * 60.0f;
    if (IsKeyDown(KEY_UP))
        next_pos = Vector2Add(player.pos, Vector2Scale(player.angle, speed));
    else if (IsKeyDown(KEY_DOWN))
        next_pos = Vector2Add(player.pos, Vector2Scale(player.angle, -1 * speed));

    if (playerInBlock(next_pos, player.size))
        player.pos = player.pos;
    else
        player.pos = next_pos;

    if (IsKeyDown(KEY_RIGHT))
        player.angle = Vector2Rotate(player.angle, DEG2RAD * speed);
    else if (IsKeyDown(KEY_LEFT))
        player.angle = Vector2Rotate(player.angle, - DEG2RAD * speed);
    return player;
}

// This will return a Vector4 of the form {end_point.x, end_point.y, wall_type, texture_column_position}
Vector4 rayEndPoint(Player player, float angle_offset) {
    Vector2 angle = Vector2Rotate(player.angle, angle_offset);
    Vector2 first_vertical_wall = player.pos;
    Vector2 first_horizontal_wall = player.pos;

    // The amount to add if x increases by one BLOCKSIZE
    Vector2 y_step = (Vector2){ BLOCKSIZE, BLOCKSIZE * (angle.y / angle.x) };
    // The amount to increase if y increases by one BLOCKSIZE
    Vector2 x_step = (Vector2){ BLOCKSIZE * (angle.x / angle.y), BLOCKSIZE };

    // Set the previous and next block-aligned coordinates
    int prev_aligned_x = floor(player.pos.x / BLOCKSIZE) * BLOCKSIZE;
    int prev_aligned_y = floor(player.pos.y / BLOCKSIZE) * BLOCKSIZE;
    int next_aligned_x = BLOCKSIZE + prev_aligned_x;
    int next_aligned_y = BLOCKSIZE + prev_aligned_y;

    // Distance to the next alignment
    float x_dist = 0.0f;
    float y_dist = 0.0f;

    // Find the first vertical wall
    if (angle.x > 0)
        x_dist = next_aligned_x - player.pos.x;
    else
    {
        y_step = Vector2Scale(y_step, -1.0f);
        x_dist = player.pos.x - prev_aligned_x;
        x_dist += 0.001;
    }

    if (angle.y > 0)
        y_dist = next_aligned_y - player.pos.y;
    else
    {
        x_step = Vector2Scale(x_step, -1.0f);
        y_dist = player.pos.y - prev_aligned_y;
        y_dist += 0.001;
    }

    first_vertical_wall = Vector2Add(player.pos, Vector2Scale(y_step, x_dist / BLOCKSIZE));
    first_horizontal_wall = Vector2Add(player.pos, Vector2Scale(x_step, y_dist / BLOCKSIZE));

    while (isInMap(first_vertical_wall) && !is_block(first_vertical_wall))
        first_vertical_wall = Vector2Add(first_vertical_wall, y_step);
    while (isInMap(first_horizontal_wall) && !is_block(first_horizontal_wall))
        first_horizontal_wall = Vector2Add(first_horizontal_wall, x_step);

    float texture_column_vertical = ((int)first_vertical_wall.y % BLOCKSIZE);
    texture_column_vertical *= (float)TEXTURESIZE / BLOCKSIZE;
    float texture_column_horizontal = ((int)first_horizontal_wall.x % BLOCKSIZE);
    texture_column_horizontal *= (float)TEXTURESIZE / BLOCKSIZE;

    if (Vector2DistanceSqr(player.pos, first_horizontal_wall) < Vector2DistanceSqr(player.pos, first_vertical_wall))
        return (Vector4){first_horizontal_wall.x, first_horizontal_wall.y, 1, texture_column_horizontal};
    else
        return (Vector4){first_vertical_wall.x, first_vertical_wall.y, 0, texture_column_vertical};
}

// Simple version, not in use
Vector2 rayEndPointSimple(Player player, float angle_offset) {
    Vector2 angle = Vector2Rotate(player.angle, angle_offset);
    Vector2 end_point = player.pos;
    while (!is_block(end_point) && isInMap(end_point))
        end_point = Vector2Add(end_point, angle);
    return end_point;
}

void drawColumn(float block_distance, int view_column, float brightness, int texture_column, Texture2D texture) {
    int view_width = (SCREENWIDTH - MAPSIZE);
    int view_center = MAPSIZE + (view_width / 2);
    int column_offset = view_column - (RAYNUMBER / 2);

    int column_height = SCREENHEIGHT / block_distance;
    int yOffSet = (SCREENHEIGHT - column_height) / 2;

    DrawRectangleGradientV(view_center + column_offset * RAYCASTPIXELWIDTH, 0, RAYCASTPIXELWIDTH, SCREENHEIGHT / 2, BLUE, LIGHTBLUE);
    DrawRectangleGradientV(view_center + column_offset * RAYCASTPIXELWIDTH, SCREENHEIGHT / 2, RAYCASTPIXELWIDTH, SCREENHEIGHT / 2, (Color){76, 35, 26, 255}, BROWN);

    //DrawRectangle(view_center + column_offset * RAYCASTPIXELWIDTH, yOffSet, RAYCASTPIXELWIDTH, column_height, ColorBrightness(GREEN, brightness));
    DrawTexturePro
    (
        texture,
        (Rectangle){texture_column, 0, 1, TEXTURESIZE}, //{texture_column, 0, TEXTURESIZE, RAYCASTPIXELWIDTH},
        (Rectangle){view_center + column_offset * RAYCASTPIXELWIDTH, yOffSet, RAYCASTPIXELWIDTH, column_height},
        (Vector2){0.0, 0.0},
        0.0,
        ColorBrightness(WHITE, brightness)
    );
}

Texture2D load_texture(char filename[])
{
    Image image = LoadImage(filename);
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);
    return texture;
}

int main() {
    Player player;
    player.pos = (Vector2){MAPSIZE / 2, SCREENHEIGHT / 2};
    player.angle = (Vector2){0.0, -1.0};
    player.speed = 2;
    player.size = 8;
    float time_delta;

    // Cache calls to cosine for fish eye correcting each ray
    static float cosine_cache[RAYNUMBER];
    for (int ray_index = 0; ray_index < RAYNUMBER; ray_index++)
    {
        float angle = - HALFVIEWANGLE + VIEWANGLE * ray_index / RAYNUMBER;
        angle *= DEG2RAD;
        cosine_cache[ray_index] = cos(angle);
    }

    InitWindow(SCREENWIDTH, SCREENHEIGHT, "Raycaster");

    Texture2D brick_texture = load_texture("brick_texture.png");

    while (WindowShouldClose() != true)
    {
        time_delta = GetFrameTime();
        player = movePlayer(player, time_delta);

        BeginDrawing();
        ClearBackground(BLACK);
        draw_map();
        draw_player(player);

        for (int view_column = 0; view_column < RAYNUMBER; view_column++)
        {
            float angle = - HALFVIEWANGLE + VIEWANGLE * view_column / RAYNUMBER;
            angle *= DEG2RAD;
            Vector4 result = rayEndPoint(player, angle);
            Vector2 ray_end_point = (Vector2){ result.x, result.y };
            bool ray_hit_horizontal = result.z;
            int texture_column = result.w;
    
            float distance = Vector2Distance(player.pos, ray_end_point);
            distance *= cosine_cache[view_column]; // correct for fish eye effect
            float block_distance = (distance / BLOCKSIZE);
            float brightness = Lerp(-0.9, 0.10, Clamp(1.0f / sqrt(block_distance), 0.0, 1.0));
            
            if (view_column % 4 == 0)
                DrawLineV(Vector2Add(player.pos, Vector2Scale(Vector2Rotate(player.angle, angle), player.size)), ray_end_point, RED);

            if (ray_hit_horizontal)
                drawColumn(block_distance, view_column, brightness + 0.1, texture_column, brick_texture);
            else
                drawColumn(block_distance, view_column, brightness, texture_column, brick_texture);
        }

        DrawRectangle(SCREENWIDTH - 105, 10, 90, 20, WHITE);
        DrawFPS(SCREENWIDTH - 100, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}