#include <raylib.h>
#include "raymath.h"
#include <stdio.h>
#include <time.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define RANDOMSPEED 500
#define MAXPARTICLES 3000
#define PARTICLE_SIZE 2
#define MAX_TOUCH_POINTS 3

int SCREENWIDTH = 1200;
int SCREENHEIGHT = 1600;

#define FRICTION_FACTOR 10
#define FORCE_FACTOR 10
#define RADIUS_MAX 50
#define RADIUS_MIN .2

static unsigned long int next = 1;

int random_int(void)   // RAND_MAX assumed to be 32767
{
    next = next * 1103515245 + 12345;
    return (unsigned int)(next/65536) % 32768;
}

float random_float(void)
{
    return (float)random_int() / (float)(32767);
}

void set_seed(unsigned int seed) {
    next = seed;
}

int particle_radius[6] = { 100, 100, 100, 150, 150, 150 };

typedef enum
{
    P_RED,
    P_ORANGE,
    P_YELLOW,
    P_GREEN,
    P_BLUE,
    P_PURPLE,
} ParticleType;

typedef struct
{
    Vector2 pos;
    Vector2 vel;
    ParticleType type;
} Particle;

Color get_particle_color(Particle p)
{
    Color particle_colors[] = { RED, ORANGE, YELLOW, GREEN, BLUE, PURPLE };
    return particle_colors[p.type];
}

float force_matrix[6][6] =
{
    { 1.0, 0.2, 0.0, 0.0, 0.0, 0.0 },
    { 0.0, 1.0, 0.2, 0.0, 0.0, 0.0 },
    { 0.0, 0.0, 1.0, 0.2, 0.0, 0.0 },
    { 0.0, 0.0, 0.0, 1.0, 0.2, 0.0 },
    { 0.0, 0.0, 0.0, 0.0, 1.0, 0.2 },
    { 0.2, 0.0, 0.0, 0.0, 0.0, 1.0 },
};

void generate_from_seed(unsigned int seed)
{
    set_seed(seed);

    for (int i = 0; i < 6; i++)
        particle_radius[i] = (random_int() % (RADIUS_MAX / 2)) + (RADIUS_MAX / 2);
    for (int i = 0; i < 6; i++)
        for (int j = 0; j < 6; j++)
            force_matrix[i][j] = random_float() * 2.0 - 1.0;
}

float absolute(float input)
{
    return (input < 0) ? -input : input;
}

float calculate_force(float normalized_distance, float attraction_factor)
{
    const float beta = RADIUS_MIN;
    if (normalized_distance < beta)
        return normalized_distance / beta - 1;
    else if (beta < normalized_distance && normalized_distance < 1)
        return attraction_factor * (1 - absolute(2 * normalized_distance - 1 - beta) / (1 - beta));
    else
        return 0;
}

Particle generate_particle(Vector2 pos, Vector2 vel)
{
    Particle particle =
    {
        .pos = pos,
        .vel = vel,
        .type = random_int() % 6,
    };
    return particle;
}

Particle generate_random_particle()
{
    return generate_particle
    (
        (Vector2){ random_int() % SCREENWIDTH, random_int() % SCREENHEIGHT },
        (Vector2){ 0.0, 0.0 }
    );
}

Particle generate_particle_at_point(Vector2 pos)
{
    return generate_particle
    (
        (Vector2){ pos.x + (random_int() % 50 - 25), pos.y + (random_int() % 50 - 25) },
        (Vector2){ random_int() % (2*RANDOMSPEED) - RANDOMSPEED, random_int() % (2*RANDOMSPEED) - RANDOMSPEED }
    );
}

int main(void)
{
    int particle_count = 0;
    Particle particles[MAXPARTICLES];
    float time_delta = 0.0;
    Vector2 touch_positions[MAX_TOUCH_POINTS] = { 0 };
    unsigned int seed = 5778;
    generate_from_seed(seed);

    InitWindow(SCREENWIDTH, SCREENHEIGHT, "Particle Simulation");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    int monitor = GetCurrentMonitor();
    SCREENHEIGHT = GetMonitorHeight(monitor);
    SCREENWIDTH = GetMonitorWidth(monitor);
    SetWindowSize(SCREENWIDTH, SCREENHEIGHT);
    SetWindowSize(SCREENWIDTH, SCREENHEIGHT);

    for (int i = 0; i < MAXPARTICLES / 2; i++)
    { 
        particles[particle_count] = generate_random_particle();
        particle_count++;       
    }

    while(!WindowShouldClose())
    {
        if (IsWindowResized())
        {
            SCREENHEIGHT = GetScreenHeight();
            SCREENWIDTH = GetScreenWidth();
            SetWindowSize(SCREENWIDTH, SCREENHEIGHT);
        }
        int touch_count = GetTouchPointCount();
        if (touch_count > MAX_TOUCH_POINTS)
            touch_count = MAX_TOUCH_POINTS;
        for (int i = 0; i < touch_count; i++)
            touch_positions[i] = GetTouchPosition(i);

        time_delta = GetFrameTime();

        // Create Particles using mouse
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && particle_count < MAXPARTICLES)
        {
            particles[particle_count] = generate_particle_at_point(GetMousePosition());
            particle_count++;
        }

        // Create Particles using touchscreen
        for (int i = 0; i < touch_count; i++)
        {
            if (particle_count >= MAXPARTICLES)
                break;
            if (touch_positions[i].x == 0 && touch_positions[i].y == 0)
                continue;

            particles[particle_count] = generate_particle_at_point(GetMousePosition());
            particle_count++;
        }


        for (int i = 0; i < particle_count; i++)
        {
            float total_force_x = 0.0;
            float total_force_y = 0.0;

            for (int j = 0; j < particle_count; j++)
            {
                if (i == j)
                    continue;
                
                Vector2 direction = Vector2Subtract(particles[j].pos, particles[i].pos);
                int max_radius = particle_radius[particles[j].type];

                if (direction.x > SCREENWIDTH - max_radius)
                    direction.x -= SCREENWIDTH;
                else if (direction.x < -(SCREENWIDTH - max_radius))
                    direction.x += SCREENWIDTH;

                if (direction.y > SCREENHEIGHT - max_radius)
                    direction.y -= SCREENHEIGHT;
                else if (direction.y < -(SCREENHEIGHT - max_radius))
                    direction.y += SCREENHEIGHT;

                float distance = Vector2LengthSqr(direction);

                if (distance > 0 && distance <max_radius * max_radius)
                {
                    distance = sqrt(distance);
                    float force = calculate_force(distance / max_radius, force_matrix[particles[i].type][particles[j].type]);
                    total_force_x += direction.x / distance * force * max_radius * FORCE_FACTOR;
                    total_force_y += (direction.y / distance * force) * max_radius * FORCE_FACTOR;
                }
            }

            float friction = pow(0.5, time_delta * FRICTION_FACTOR);
            particles[i].vel = Vector2Scale(particles[i].vel, friction);
            particles[i].vel.x += total_force_x * time_delta;
            particles[i].vel.y += total_force_y * time_delta;
        }


        // Update Particle Positions
        for (int i = 0; i < particle_count; i++)
        {
            particles[i].pos = Vector2Add(particles[i].pos, Vector2Scale(particles[i].vel, time_delta));
            particles[i].pos.x = Wrap(particles[i].pos.x, 0, SCREENWIDTH);
            particles[i].pos.y = Wrap(particles[i].pos.y, 0, SCREENHEIGHT);
        }

        BeginDrawing();

        ClearBackground(BLACK);
        for (int i = 0; i < particle_count; i++)
            DrawCircle(particles[i].pos.x, particles[i].pos.y, PARTICLE_SIZE, get_particle_color(particles[i]));

        char message[100];
        Color T_YELLOW = YELLOW;
        T_YELLOW.a = 225;
        DrawRectangle(0, 0, 155, 75, T_YELLOW);
        sprintf(message, "FPS: %d\nParticles: %04d\nSeed: %04d", GetFPS(), particle_count, seed);
        DrawText(message, 5, 5, 20, BLACK);

        GuiSetStyle(DEFAULT, TEXT_SIZE, 30);
        if ( GuiButton((Rectangle){ SCREENWIDTH - 155, SCREENHEIGHT - 55, 150, 50 }, "Reset") ){
            seed = random_int() % 10000;
            generate_from_seed(seed);
            particle_count = 0;
            for (int i = 0; i < MAXPARTICLES / 2; i++)
            { 
                particles[particle_count] = generate_random_particle();
                particle_count++;       
            }
        }

        //if ( GuiButton((Rectangle){ SCREENWIDTH - 300, SCREENHEIGHT - 200, 250, 75 }, "New Seed") ){
        //    seed = random_int() % 10000;
        //    generate_from_seed(seed);
        //}

        EndDrawing();
    }

    return 0;
}