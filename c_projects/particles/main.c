#include <raylib.h>
#include "raymath.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define SCREENHEIGHT 900
#define SCREENWIDTH  1600
#define RANDOMSPEED 500
#define MAXPARTICLES 2000
#define MAX_TOUCH_POINTS 3

typedef struct
{
    float pos_x[MAXPARTICLES];
    float pos_y[MAXPARTICLES];
    float vel_x[MAXPARTICLES];
    float vel_y[MAXPARTICLES];
    Color color[MAXPARTICLES];
    float size[MAXPARTICLES];
    int number;
} Particles;

int main(void)
{
    int particle_number = 0;
    float particles_pos_x[MAXPARTICLES];
    float particles_pos_y[MAXPARTICLES];
    float particles_vel_x[MAXPARTICLES];
    float particles_vel_y[MAXPARTICLES];
    Color particles_color[MAXPARTICLES];
    float particles_size[MAXPARTICLES];
    float time_delta = 0.0;
    Vector2 touch_positions[MAX_TOUCH_POINTS] = { 0 };

    srand((unsigned int)time(NULL));
    InitWindow(SCREENWIDTH, SCREENHEIGHT, "Particle Simulation");

    while(!WindowShouldClose())
    {
        int touch_count = GetTouchPointCount();
        if (touch_count > MAX_TOUCH_POINTS)
            touch_count = MAX_TOUCH_POINTS;
        for (int i = 0; i < touch_count; i++)
            touch_positions[i] = GetTouchPosition(i);

        time_delta = GetFrameTime();

        // Create Particles
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && particle_number < MAXPARTICLES)
        {
            Color colors[] = { PURPLE, RED, ORANGE, YELLOW, GREEN, BLUE, PINK };
            particles_pos_x[particle_number] = GetMouseX() + (rand() % 50 - 25);
            particles_pos_y[particle_number] = GetMouseY() + (rand() % 50 - 25);
            particles_vel_x[particle_number] = rand() % (2*RANDOMSPEED) - RANDOMSPEED;
            particles_vel_y[particle_number] = rand() % (2*RANDOMSPEED) - RANDOMSPEED;
            particles_color[particle_number] = colors[rand() % (sizeof(colors)/sizeof(colors[0]))];
            particles_size[particle_number] = (rand() % 5) + 5;
            particle_number++;
        }

        // Create Particles
        for (int i = 0; i < touch_count; i++)
        {
            if (particle_number > MAXPARTICLES)
                break;
            if (touch_positions[i].x == 0 && touch_positions[i].y == 0)
                continue;

            Color colors[] = { PURPLE, RED, ORANGE, YELLOW, GREEN, BLUE, PINK };
            particles_pos_x[particle_number] = touch_positions[i].x + (rand() % 50 - 25);
            particles_pos_y[particle_number] = touch_positions[i].y + (rand() % 50 - 25);
            particles_vel_x[particle_number] = rand() % (2*RANDOMSPEED) - RANDOMSPEED;
            particles_vel_y[particle_number] = rand() % (2*RANDOMSPEED) - RANDOMSPEED;
            particles_color[particle_number] = colors[rand() % (sizeof(colors)/sizeof(colors[0]))];
            particles_size[particle_number] = (rand() % 5) + 5;
            particle_number++;
        }

        // Update Particle Positions
        for (int i = 0; i < particle_number; i++)
        {
            if (particles_pos_x[i] - particles_size[i] < 0 || particles_pos_x[i] + particles_size[i] > SCREENWIDTH)
            {
                particles_pos_x[i] = Clamp(particles_pos_x[i], particles_size[i], SCREENWIDTH - particles_size[i]);
                particles_vel_x[i] *= -1;
            }
            if (particles_pos_y[i] - particles_size[i] < 0 || particles_pos_y[i] + particles_size[i] > SCREENHEIGHT)
            {
                particles_pos_y[i] = Clamp(particles_pos_y[i], particles_size[i], SCREENHEIGHT - particles_size[i]);
                particles_vel_y[i] *= -1;
            }
            particles_pos_x[i] += particles_vel_x[i] * time_delta;
            particles_pos_y[i] += particles_vel_y[i] * time_delta;
        }

        // Particle collision
        for (int i = 0; i < particle_number; i++)
            for (int j = i+1; j < particle_number; j++)
            {
                Vector2 this_pos = (Vector2){ particles_pos_x[i], particles_pos_y[i] };
                Vector2 this_vel = (Vector2){ particles_vel_x[i], particles_vel_y[i] };
                Vector2 other_pos = (Vector2){ particles_pos_x[j], particles_pos_y[j] };
                Vector2 other_vel = (Vector2){ particles_vel_x[j], particles_vel_y[j] };
                float push_distance = (particles_size[i] + particles_size[j]) - Vector2Distance(this_pos, other_pos);

                if (push_distance < 0)
                    continue;
                
                Vector2 normal = Vector2Normalize(Vector2Subtract(this_pos, other_pos));
                Vector2 this_parallel_velocity  = Vector2Scale(normal, Vector2DotProduct(this_vel, normal));
                Vector2 other_parallel_velocity = Vector2Scale(normal, Vector2DotProduct(other_vel, normal));

                this_vel = Vector2Subtract(this_vel, this_parallel_velocity);
                other_vel = Vector2Subtract(other_vel, other_parallel_velocity);

                this_vel = Vector2Add(this_vel, other_parallel_velocity);
                other_vel = Vector2Add(other_vel, this_parallel_velocity);

                particles_vel_x[i] = this_vel.x;
                particles_vel_y[i] = this_vel.y;
                particles_vel_x[j] = other_vel.x;
                particles_vel_y[j] = other_vel.y;

                push_distance /= 2.0f;
                this_pos  = Vector2Add(this_pos, Vector2Scale(normal, push_distance));
                other_pos = Vector2Add(other_pos, Vector2Scale(normal, - push_distance));

                particles_pos_x[i] = this_pos.x;
                particles_pos_y[i] = this_pos.y;
                particles_pos_x[j] = other_pos.x;
                particles_pos_y[j] = other_pos.y;
            }

        BeginDrawing();

        ClearBackground(BLACK);
        for (int i = 0; i < particle_number; i++)
            DrawCircle(particles_pos_x[i], particles_pos_y[i], particles_size[i], particles_color[i]);

        char message[100];
        Color T_YELLOW = YELLOW;
        T_YELLOW.a = 225;
        DrawRectangle(0, 0, 195, 65, T_YELLOW);
        sprintf(message, "FPS: %d\nParticles: %04d", GetFPS(), particle_number);
        DrawText(message, 10, 10, 24, BLACK);
        EndDrawing();
    }

    return 0;
}