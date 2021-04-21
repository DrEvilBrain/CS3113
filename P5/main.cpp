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

#include "Effects.h"
#include "Util.h"
#include "Entity.h"
#include "Map.h"
#include "Scene.h"
#include "MainMenu.h"
#include "Level1.h"
#include "Level2.h"
#include "Level3.h"

Scene* currentScene;
Scene* sceneList[4];

Effects* effects;

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

// handles switching bewteen scenes
void SwitchToScene(Scene* scene)
{
    currentScene = scene;
    currentScene->Initialize();
}

void Initialize()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    displayWindow = SDL_CreateWindow("Project 5: Operation Intrude", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
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

    // ***Initialize Scenes***
    sceneList[0] = new MainMenu();
    sceneList[1] = new Level1();
    sceneList[2] = new Level2();
    sceneList[3] = new Level3();
    SwitchToScene(sceneList[0]);

    // ***Initialize Audio***
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    currentScene->state.music = Mix_LoadMUS("Assets/Audio/crypto.mp3");
    Mix_PlayMusic(currentScene->state.music, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2);

    // ***Initialize Effects***
    effects = new Effects(projectionMatrix, viewMatrix);
    effects->Start(NONE, 1);
}

// Controls:
// Left Arrow = Move Left
// Right Arrow = Move Right
// Spacebar = Jump
// Enter = Start/End Game
void ProcessInput()
{
    currentScene->state.player->movement = glm::vec3(0);
    
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
                        if (currentScene->state.player->collidedBottom)
                        {
                            currentScene->state.player->jump = true;
                        }
                    break;

                    case SDLK_RETURN:
                        // begin game when on the main menu
                        if (currentScene == sceneList[0])
                        {
                            SwitchToScene(sceneList[1]);
                            currentScene->state.lives = sceneList[0]->state.lives;
                        }
                        // go back to main menu when game is over
                        else if (currentScene->state.isGameOver)
                        {
                            currentScene->state.isGameOver = false;
                            SwitchToScene(sceneList[0]);
                        }
                        break;
                }
                break; // SDL_KEYDOWN
        }
    }
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    // move left
    if (keys[SDL_SCANCODE_LEFT] && !currentScene->state.isGameOver)
    {
        currentScene->state.player->movement.x += -1.0f;
        currentScene->state.player->animIndices = currentScene->state.player->animLeft;
    }
    // move right
    else if (keys[SDL_SCANCODE_RIGHT] && !currentScene->state.isGameOver)
    {
        currentScene->state.player->movement.x += 1.0f;
        currentScene->state.player->animIndices = currentScene->state.player->animRight;
    }
    
    if (glm::length(currentScene->state.player->movement) > 1.0f)
    {
        currentScene->state.player->movement = glm::normalize(currentScene->state.player->movement);
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
    
    if (!currentScene->state.isGameOver)
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
            currentScene->Update(FIXED_TIMESTEP);
            effects->Update(FIXED_TIMESTEP);

            deltaTime -= FIXED_TIMESTEP;
        }

        accumulator = deltaTime;
    }

    // camera scrolling
    viewMatrix = glm::mat4(1.0f);

    if (currentScene->state.player->position.x > 5)
    {
        viewMatrix = glm::translate(viewMatrix, glm::vec3(-(currentScene->state.player->position.x), 3.75, 0));
    }
    else
    {
        viewMatrix = glm::translate(viewMatrix, glm::vec3(-5, 3.75, 0));
    }

    // handle changing effects
    if (currentScene->state.playingEffect != NONE)
    {
        effects->Start(currentScene->state.playingEffect, 1);
    }
}

void Render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(program.programID);

    // update the viewmatrix for camera scrolling
    program.SetViewMatrix(viewMatrix);

    // render the scene
    currentScene->Render(&program);

    // render effects
    effects->Render();

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

        if (currentScene->state.nextScene >= 0)
        {
            // make life count persist between scene transitions
            sceneList[currentScene->state.nextScene]->state.lives = currentScene->state.lives;

            // go to next scene
            SwitchToScene(sceneList[currentScene->state.nextScene]);
        }

        Render();
    }
    
    Shutdown();
    return 0;
}
