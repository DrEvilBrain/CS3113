#include <vector>

#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Entity.h"

#define PLATFORM_COUNT 4
#define OBSTACLE_COUNT 30

struct GameState
{
    Entity* player;
    Entity* platforms;
    Entity* obstacles;
    GLuint UIfont;
    bool isGameOver = false;
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
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Project 3: Paratrooper", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
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
    state.player->position = glm::vec3(0, 4.0f, 0);
    state.player->movement = glm::vec3(0);
    state.player->acceleration = glm::vec3(0, -1.0f, 0);
    state.player->velocity = glm::vec3(0);
    state.player->speed = 0.0f;

    state.player->height = 0.9f;
    state.player->width = 0.3f;

    // state.player->jumpPower = 1.2f;

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
    GLuint easyPlatformTextureID = LoadTexture("Assets/easyPlatform.png");
    GLuint mediumPlatformTextureID = LoadTexture("Assets/mediumPlatform.png");
    GLuint hardPlatformTextureID = LoadTexture("Assets/hardPlatform.png");

    // set all platforms to have entityType 'PLATFORM'
    for (int i = 0; i < PLATFORM_COUNT; i++)
    {
        state.platforms[i].entityType = PLATFORM;
    }

    // easy platforms
    state.platforms[0].textureID = easyPlatformTextureID;
    state.platforms[1].textureID = easyPlatformTextureID;

    state.platforms[0].position = glm::vec3(2.0f, -3.25f, 0.0f);
    state.platforms[1].position = glm::vec3(3.0f, -3.25f, 0.0f);

    // medium platforms
    state.platforms[2].textureID = mediumPlatformTextureID;
    state.platforms[2].position = glm::vec3(-3.0f, -3.25f, 0.0f);

    // hard platforms
    state.platforms[3].textureID = hardPlatformTextureID;
    state.platforms[3].position = glm::vec3(4.0f, -0.25f, 0.0f);

    // update the platforms once so we can render them
    for (int i = 0; i < PLATFORM_COUNT; i++)
    {
        state.platforms[i].Update(0, NULL, 0, NULL, 0);
    }

    // ***Initialize Obstacles***
    state.obstacles = new Entity[OBSTACLE_COUNT];
    GLuint obstacleTextureID = LoadTexture("Assets/wall.png");

    // set all obstacles to have entityType 'OBSTACLE' and the texture ID 'obstacleTextureID'
    for (int i = 0; i < OBSTACLE_COUNT; i++)
    {
        state.obstacles[i].entityType = OBSTACLE;
        state.obstacles[i].textureID = obstacleTextureID;
    }

    // floor row
    state.obstacles[0].position = glm::vec3(-5.0f, -3.25f, 0.0f);
    state.obstacles[1].position = glm::vec3(-4.0f, -3.25f, 0.0f);
    state.obstacles[2].position = glm::vec3(-2.0f, -3.25f, 0.0f);
    state.obstacles[3].position = glm::vec3(-1.0f, -3.25f, 0.0f);
    state.obstacles[4].position = glm::vec3(0.0f, -3.25f, 0.0f);
    state.obstacles[5].position = glm::vec3(1.0f, -3.25f, 0.0f);
    state.obstacles[6].position = glm::vec3(4.0f, -3.25f, 0.0f);
    state.obstacles[7].position = glm::vec3(5.0f, -3.25f, 0.0f);

    // left wall
    state.obstacles[8].position = glm::vec3(-5.0f, -2.25f, 0.0f);
    state.obstacles[9].position = glm::vec3(-5.0f, -1.25f, 0.0f);
    state.obstacles[10].position = glm::vec3(-5.0f, -0.25f, 0.0f);
    state.obstacles[11].position = glm::vec3(-5.0f, 0.75f, 0.0f);
    state.obstacles[12].position = glm::vec3(-5.0f, 1.75f, 0.0f);
    state.obstacles[13].position = glm::vec3(-5.0f, 2.75f, 0.0f);
    state.obstacles[14].position = glm::vec3(-5.0f, 3.75f, 0.0f);

    // right wall
    state.obstacles[15].position = glm::vec3(5.0f, -2.25f, 0.0f);
    state.obstacles[16].position = glm::vec3(5.0f, -1.25f, 0.0f);
    state.obstacles[17].position = glm::vec3(5.0f, -0.25f, 0.0f);
    state.obstacles[18].position = glm::vec3(5.0f, 0.75f, 0.0f);
    state.obstacles[19].position = glm::vec3(5.0f, 1.75f, 0.0f);
    state.obstacles[20].position = glm::vec3(5.0f, 2.75f, 0.0f);
    state.obstacles[21].position = glm::vec3(5.0f, 3.75f, 0.0f);

    // floating obstacle above the easy platform and surrounding the hard platform
    state.obstacles[22].position = glm::vec3(3.0f, -0.25f, 0.0f);
    state.obstacles[23].position = glm::vec3(2.0f, 0.75f, 0.0f);
    state.obstacles[24].position = glm::vec3(2.0f, 3.75f, 0.0f);
   
    // floating block to the left of the medium platform
    state.obstacles[25].position = glm::vec3(-4.0f, -1.25f, 0.0f);

    // vertical wall to the right of the medium landing platform
    state.obstacles[26].position = glm::vec3(-2.0f, 0.75f, 0.0f);
    state.obstacles[27].position = glm::vec3(-2.0f, -0.25f, 0.0f);
    state.obstacles[28].position = glm::vec3(-2.0f, -1.25f, 0.0f);
    
    // wall at the bottom middle of the screen
    state.obstacles[29].position = glm::vec3(0.0f, -2.25f, 0.0f);
       
    // update the obstacles once so we can render them
    for (int i = 0; i < OBSTACLE_COUNT; i++)
    {
        state.obstacles[i].Update(0, NULL, 0, NULL, 0);
    }

    // ***Initialize UI Font***
    state.UIfont = LoadTexture("Assets/font1.png");
}

// Controls:
// Left Arrow = Accelerate to the Left
// Right Arrow = Accelerate to the Right
// Spacebar = Restart when in a game over screen
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
                        // Restart the game
                        if (state.isGameOver)
                        {
                            state.player->position = glm::vec3(0, 4.0f, 0);
                            state.player->movement = glm::vec3(0);
                            state.player->acceleration = glm::vec3(0, -1.0f, 0);
                            state.player->velocity = glm::vec3(0);
                            state.player->speed = 0.0f;
                            state.player->lastCollision = PLAYER;
                            state.player->textureID = LoadTexture("Assets/player.png");
                            state.isGameOver = false;
                        }
                        break;
                }
                break; // SDL_KEYDOWN
        }
    }
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    // accelerate left
    if (keys[SDL_SCANCODE_LEFT] && !state.isGameOver)
    {
        state.player->acceleration.x += -0.1f;
        state.player->animIndices = state.player->animLeft;
    }
    // accelerate right
    else if (keys[SDL_SCANCODE_RIGHT] && !state.isGameOver)
    {
        state.player->acceleration.x += 0.1f;
        state.player->animIndices = state.player->animRight;
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
            state.player->Update(FIXED_TIMESTEP, state.platforms, PLATFORM_COUNT, state.obstacles, OBSTACLE_COUNT);

            // check if player landed on a platform or an obstacle
            if (state.player->lastCollision == PLATFORM || state.player->lastCollision == OBSTACLE)
            {
                // trigger game over
                state.isGameOver = true;

                // show landed sprite if the player has collided at the bottom
                if (state.player->collidedBottom)
                {
                    state.player->textureID = LoadTexture("Assets/player_landed.png");
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

    // render obstacles
    for (int i = 0; i < OBSTACLE_COUNT; i++)
    {
        state.obstacles[i].Render(&program);
    }

    // render player
    state.player->Render(&program);
    
    // render UI text for when the player lands on a landing platform
    if (state.isGameOver && state.player->lastCollision == PLATFORM)
    {
        // end game with victory
        DrawText(&program, state.UIfont, "Mission Successful!", 0.5f, -0.2f, glm::vec3(-2.75f, 2.0f, 0.0f));
    }
    
    // render UI text for when the player lands on an obstacle
    if (state.isGameOver && state.player->lastCollision == OBSTACLE)
    {
        // end game with failure
        DrawText(&program, state.UIfont, "  Mission Failed!  ", 0.5f, -0.2f, glm::vec3(-2.75f, 2.0f, 0.0f));
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
