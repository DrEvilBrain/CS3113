#include <vector>

#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Entity.h"

#define PLATFORM_COUNT 32
#define ENEMY_COUNT 3

struct GameState
{
    Entity* player;
    Entity* platforms;
    Entity* obstacles;
    Entity* enemies;
    GLuint UIfont;
    Mix_Music* music;
    Mix_Chunk* winSFX;
    Mix_Chunk* loseSFX;
    bool isGameOver = false;
    int lives = 1;
    int points = 0;
};

GameState state;

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

GLuint LoadTexture(const char* filePath)
{
    int w, h, n;
    unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);
    
    if (image == NULL)
    {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    stbi_image_free(image);
    return textureID;
}

void DrawText(ShaderProgram* program, GLuint fontTextureID, std::string text,
                float size, float spacing, glm::vec3 position)
{
    float width = 1.0f / 16.0f;
    float height = 1.0f / 16.0f;

    std::vector<float> vertices;
    std::vector<float> texCoords;

    for (int i = 0; i < text.size(); i++)
    {
        int index = (int)text[i];
        float offset = (size + spacing) * i;

        float u = (float)(index % 16) / 16.0f;
        float v = (float)(index / 16) / 16.0f;

        vertices.insert(vertices.end(), {
            offset + (-0.5f * size), 0.5f * size,
            offset + (-0.5f * size), -0.5f * size,
            offset + (0.5f * size), 0.5f * size,
            offset + (0.5f * size), -0.5f * size,
            offset + (0.5f * size), 0.5f * size,
            offset + (-0.5f * size), -0.5f * size,
         });

        texCoords.insert(texCoords.end(), {
            u, v,
            u, v + height,
            u + width, v,
            u + width, v + height,
            u + width, v,
            u, v + height,
         });
    }

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    program->SetModelMatrix(modelMatrix);

    glUseProgram(program->programID);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords.data());
    glEnableVertexAttribArray(program->texCoordAttribute);

    glBindTexture(GL_TEXTURE_2D, fontTextureID);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void Initialize()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    displayWindow = SDL_CreateWindow("Project 4: AI Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(0, 0, 640, 480);

    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");

    viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);

    float windowWidth = 5.0f;
    float windowHeight = 3.75f;
    float windowDepth = 1.0f;
    projectionMatrix = glm::ortho(-windowWidth, windowWidth, -windowHeight, windowHeight, -windowDepth, windowDepth);

    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);

    glUseProgram(program.programID);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialize Game Objects

    // ***Initialize Player***
    state.player = new Entity();
    state.player->entityType = PLAYER;
    state.player->position = glm::vec3(3, -2.25f, 0);
    state.player->movement = glm::vec3(0);
    state.player->acceleration = glm::vec3(0, -9.8f, 0);
    state.player->velocity = glm::vec3(0);
    state.player->speed = 3.0f;

    state.player->height = 1.0f;
    state.player->width = 0.5f;

    state.player->jumpPower = 7.0f;

    state.player->textureID = LoadTexture("Assets/player.png");

    /*
    state.player->animRight = new int[4]{ 3, 7, 11, 15 };
    state.player->animLeft = new int[4]{ 1, 5, 9, 13 };
    state.player->animUp = new int[4]{ 2, 6, 10, 14 };
    state.player->animDown = new int[4]{ 0, 4, 8, 12 };

    state.player->animIndices = state.player->animRight;
    state.player->animFrames = 4;
    state.player->animIndex = 0;
    state.player->animTime = 0;
    state.player->animCols = 4;
    state.player->animRows = 4;
    */

    // ***Initialize Platforms***
    state.platforms = new Entity[PLATFORM_COUNT];
    GLuint platformTextureID = LoadTexture("Assets/wall.png");

    // set all platforms to have entityType 'PLATFORM' and the texture ID 'platformTextureID'
    for (int i = 0; i < PLATFORM_COUNT; i++)
    {
        state.platforms[i].entityType = PLATFORM;
        state.platforms[i].textureID = platformTextureID;
    }

    // floor row
    state.platforms[0].position = glm::vec3(-5.0f, -3.25f, 0.0f);
    state.platforms[1].position = glm::vec3(-4.0f, -3.25f, 0.0f);
    state.platforms[2].position = glm::vec3(-3.0f, -3.25f, 0.0f);
    state.platforms[3].position = glm::vec3(-2.0f, -3.25f, 0.0f);
    state.platforms[4].position = glm::vec3(-1.0f, -3.25f, 0.0f);
    state.platforms[5].position = glm::vec3(0.0f, -3.25f, 0.0f);
    state.platforms[6].position = glm::vec3(1.0f, -3.25f, 0.0f);
    state.platforms[7].position = glm::vec3(2.0f, -3.25f, 0.0f);
    state.platforms[8].position = glm::vec3(3.0f, -3.25f, 0.0f);
    state.platforms[9].position = glm::vec3(4.0f, -3.25f, 0.0f);
    state.platforms[10].position = glm::vec3(5.0f, -3.25f, 0.0f);

    // left wall
    state.platforms[11].position = glm::vec3(-5.0f, -2.25f, 0.0f);
    state.platforms[12].position = glm::vec3(-5.0f, -1.25f, 0.0f);
    state.platforms[13].position = glm::vec3(-5.0f, -0.25f, 0.0f);
    state.platforms[14].position = glm::vec3(-5.0f, 0.75f, 0.0f);
    state.platforms[15].position = glm::vec3(-5.0f, 1.75f, 0.0f);
    state.platforms[16].position = glm::vec3(-5.0f, 2.75f, 0.0f);
    state.platforms[17].position = glm::vec3(-5.0f, 3.75f, 0.0f);

    // right wall
    state.platforms[18].position = glm::vec3(5.0f, -2.25f, 0.0f);
    state.platforms[19].position = glm::vec3(5.0f, -1.25f, 0.0f);
    state.platforms[20].position = glm::vec3(5.0f, -0.25f, 0.0f);
    state.platforms[21].position = glm::vec3(5.0f, 0.75f, 0.0f);
    state.platforms[22].position = glm::vec3(5.0f, 1.75f, 0.0f);
    state.platforms[23].position = glm::vec3(5.0f, 2.75f, 0.0f);
    state.platforms[24].position = glm::vec3(5.0f, 3.75f, 0.0f);

    // floating platform
    state.platforms[25].position = glm::vec3(0.0f, 0.75f, 0.0f);
    state.platforms[26].position = glm::vec3(1.0f, 0.75f, 0.0f);
    state.platforms[27].position = glm::vec3(2.0f, 0.75f, 0.0f);
    state.platforms[28].position = glm::vec3(3.0f, 0.75f, 0.0f);
    state.platforms[29].position = glm::vec3(4.0f, 0.75f, 0.0f);

    // stairs up to the floating platform
    state.platforms[30].position = glm::vec3(-1.0f, -0.25f, 0.0f);
    state.platforms[31].position = glm::vec3(-3.0f, -1.25f, 0.0f);
       
    // update the platforms once so we can render them
    for (int i = 0; i < PLATFORM_COUNT; i++)
    {
        state.platforms[i].Update(0, NULL, 0, NULL, NULL, 0);
    }

    // ***Initialize Enemies***
    state.enemies = new Entity[ENEMY_COUNT];
    GLuint enemyTextureID = LoadTexture("Assets/enemy.png");

    // set all enemies to have entityType 'ENEMY', texture ID 'enemyTextureID', and width
    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        state.enemies[i].entityType = ENEMY;
        state.enemies[i].textureID = enemyTextureID;
        
        state.enemies[i].width = 0.5f;
    }

    // wait and go chaser
    state.enemies[0].position = glm::vec3(-3, 0.75f, 0);
    state.enemies[0].movement = glm::vec3(-1, 0, 0);
    state.enemies[0].speed = 1.0f;
    state.enemies[0].acceleration = glm::vec3(0, -9.8f, 0);
    state.enemies[0].aiType = WALKER;

    // ground patroller
    state.enemies[1].position = glm::vec3(3, 1.75f, 0);
    state.enemies[1].speed = 1.0f;
    state.enemies[1].acceleration = glm::vec3(0, -9.8f, 0);
    state.enemies[1].aiType = WAITANDGO;
    state.enemies[1].aiState = IDLE;

    // flying sine wave patroller
    state.enemies[2].position = glm::vec3(0, 2.75f, 0);
    state.enemies[2].initPosition = state.enemies[2].position;
    state.enemies[2].movement = glm::vec3(1, 0, 0);
    state.enemies[2].speed = 1.0f;
    state.enemies[2].direction = 0.05f;
    state.enemies[2].aiType = SINEFLYER;

    // ***Initialize UI Font***
    state.UIfont = LoadTexture("Assets/font1.png");

    // ***Initialize Audio***
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    state.music = Mix_LoadMUS("Assets/Audio/crypto.mp3");
    Mix_PlayMusic(state.music, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2);

    state.winSFX = Mix_LoadWAV("Assets/Audio/success.wav");
    state.loseSFX = Mix_LoadWAV("Assets/Audio/failure.wav");
}

// Controls:
// Left Arrow = Move Left
// Right Arrow = Move Right
// Spacebar = Jump
// R = Restart
void ProcessInput()
{
    state.player->movement = glm::vec3(0);
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                gameIsRunning = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                    case SDLK_SPACE:
                        // Handle jumping
                        if (state.player->collidedBottom)
                        {
                            state.player->jump = true;
                        }
                        break;
                }
                break; // SDL_KEYDOWN
        }
    }
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    // move left
    if (keys[SDL_SCANCODE_LEFT] && !state.isGameOver)
    {
        state.player->movement.x += -1.0f;
        state.player->animIndices = state.player->animLeft;
    }
    // move right
    else if (keys[SDL_SCANCODE_RIGHT] && !state.isGameOver)
    {
        state.player->movement.x += 1.0f;
        state.player->animIndices = state.player->animRight;
    }
    // restart the game
    else if (keys[SDL_SCANCODE_R] && state.isGameOver)
    {
        state.player->position = glm::vec3(3, -2.25f, 0);
        state.player->movement = glm::vec3(0);
        state.player->acceleration = glm::vec3(0, -9.8f, 0);
        state.player->velocity = glm::vec3(0);
        state.player->lastCollision = PLAYER;
        state.lives = 0;
        state.points = 0;
        state.isGameOver = false;

        // reset enemy positions
        // wait and go chaser
        state.enemies[0].position = glm::vec3(-3, 0.75f, 0);
        state.enemies[0].movement = glm::vec3(-1, 0, 0);
        state.enemies[0].speed = 1.0f;
        state.enemies[0].aiType = WALKER;

        // ground patroller
        state.enemies[1].position = glm::vec3(3, 1.75f, 0);
        state.enemies[1].speed = 1.0f;
        state.enemies[1].aiType = WAITANDGO;
        state.enemies[1].aiState = IDLE;

        // flying sine wave patroller
        state.enemies[2].position = glm::vec3(0, 2.75f, 0);
        state.enemies[2].initPosition = state.enemies[2].position;
        state.enemies[2].movement = glm::vec3(1, 0, 0);
        state.enemies[2].speed = 1.0f;
        state.enemies[2].direction = 0.05f;
        state.enemies[2].aiType = SINEFLYER;

        for (int i = 0; i < ENEMY_COUNT; i++)
        {
            state.enemies[i].isActive = true;
        }
    }
    
    if (glm::length(state.player->movement) > 1.0f)
    {
        state.player->movement = glm::normalize(state.player->movement);
    }
}

#define FIXED_TIMESTEP 0.0166666f
float lastTicks = 0;
float accumulator = 0.0f;

void Update()
{
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;
    
    if (!state.isGameOver)
    {
        deltaTime += accumulator;
        if (deltaTime < FIXED_TIMESTEP)
        {
            accumulator = deltaTime;
            return;
        }

        // Update. Notice it's FIXED_TIMESTEP. Not deltaTime
        while (deltaTime >= FIXED_TIMESTEP)
        {
            // update player
            state.player->Update(FIXED_TIMESTEP, state.platforms, PLATFORM_COUNT, state.player, state.enemies, ENEMY_COUNT);

            // update enemies
            for (int i = 0; i < ENEMY_COUNT; i++)
            {
                state.enemies[i].Update(FIXED_TIMESTEP, state.platforms, PLATFORM_COUNT, state.player, NULL, 0);
            }

            // check if player touched an enemy
            if (state.player->lastCollision == ENEMY)
            {
                Entity* enemy = state.player->lastCollisionEntity;

                enemy->CheckCollision(state.player);

                // player jumped on the enemy
                if (state.player->position.y > enemy->position.y + enemy->height * 0.9f)
                {
                    enemy->isActive = false;
                    state.player->lastCollision = PLAYER;
                    state.points++;

                    state.player->velocity.y += 6.0;

                    if (state.points >= 3)
                    {
                        state.isGameOver = true;

                        // play sfx
                        Mix_PlayChannel(-1, state.winSFX, 0);
                    }
                }
                // player got hit by the enemy
                else
                {
                    state.lives--;
                    state.isGameOver = true;

                    // play sfx
                    Mix_PlayChannel(-1, state.loseSFX, 0);
                }
            }

            deltaTime -= FIXED_TIMESTEP;
        }

        accumulator = deltaTime;
    }
}

void Render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // render platforms
    for (int i = 0; i < PLATFORM_COUNT; i++)
    {
        state.platforms[i].Render(&program);
    }

    // render enemies
    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        state.enemies[i].Render(&program);
    }

    // render player
    state.player->Render(&program);
    
    // render UI text for when the player when the player clears all enemies
    if (state.isGameOver && state.lives > 0)
    {
        // end game with victory
        DrawText(&program, state.UIfont, "You Win!", 0.5f, -0.2f, glm::vec3(-1.0f, 2.0f, 0.0f));
    }
    
    // render UI text for when the player touches an enemy and loses a life
    if (state.isGameOver && state.lives <= 0)
    {
        // end game with failure
        DrawText(&program, state.UIfont, "You Lose", 0.5f, -0.2f, glm::vec3(-1.0f, 2.0f, 0.0f));
    }

    SDL_GL_SwapWindow(displayWindow);
}

void Shutdown()
{
    SDL_Quit();
}

int main(int argc, char* argv[])
{
    Initialize();
    
    while (gameIsRunning)
    {
        ProcessInput();
        Update();
        Render();
    }
    
    Shutdown();
    return 0;
}
