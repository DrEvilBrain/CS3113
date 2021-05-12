#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1

#include <vector>
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#include "Util.h"
#include "Entity.h"
#include "Scene.h"
#include "MainMenu.h"
#include "Level1.h"

Scene* currentScene;
Scene* sceneList[2];

Effects* effects;

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;
glm::mat4 uiViewMatrix, uiProjectionMatrix;

void Render();

// handles switching bewteen scenes
void SwitchToScene(Scene* scene)
{
    currentScene = scene;
    currentScene->Initialize();
}

void Initialize()
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Project 6: First Flight", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 1280, 720);
    
    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");
    
    viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::perspective(glm::radians(45.0f), 1.777f, 0.1f, 100.0f);

    uiViewMatrix = glm::mat4(1.0);
    uiProjectionMatrix = glm::ortho(-6.4f, 6.4f, -3.6f, 3.6f, -1.0f, 1.0f);

    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    program.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    glUseProgram(program.programID);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);

    // ***Initialize Scenes***
    sceneList[0] = new MainMenu();
    sceneList[1] = new Level1();
    SwitchToScene(sceneList[0]);

    // ***Initialize Audio***
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    currentScene->state.music = Mix_LoadMUS("Assets/Audio/bgm.mp3");
    Mix_PlayMusic(currentScene->state.music, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME);

    // ***Initialize Effects***
    effects = new Effects(projectionMatrix, viewMatrix);
    effects->Start(NONE, 1);
}

bool mouseOneDown = false;

void ProcessInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                gameIsRunning = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                    // Start/End game
                    case SDLK_RETURN:
						// begin game when on the main menu
						if (currentScene == sceneList[0])
						{
                            currentScene->state.isLoading = true;
                            Render();

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
                break;

            // Check if Mouse 1 is held down. If so, fire machine guns
            case SDL_MOUSEBUTTONDOWN:
                mouseOneDown = true;
                currentScene->state.player->isFiring = true;
                break;

            // Stop firing machine guns when Mouse 1 is no longer held down
            case SDL_MOUSEBUTTONUP:
                mouseOneDown = false;
                currentScene->state.player->isFiring = false;
                break;
        }
    }

    const Uint8* keys = SDL_GetKeyboardState(NULL);
   
    // Complex realistic-ish flight controls that I don't know how to implement
    /*
    // Pitch Down
    if (keys[SDL_SCANCODE_W])
    {
        state.player->rotation.x += 0.35f;
    }
    // Pitch Up
    if (keys[SDL_SCANCODE_S])
    {
        state.player->rotation.x -= 0.35f;
    }
    // Yaw Left
    if (keys[SDL_SCANCODE_A])
    {
        state.player->rotation.y += 0.35f;
    }
    // Yaw Right
    if (keys[SDL_SCANCODE_D])
    {
        state.player->rotation.y -= 0.35f;
    }
    // Roll Left
    if (keys[SDL_SCANCODE_Q])
    {
        state.player->rotation.z += 0.35f;
    }
    // Roll Right
    if (keys[SDL_SCANCODE_E])
    {
        state.player->rotation.z -= 0.35f;
    }

    //state.player->acceleration.z = cos(glm::radians(state.player->rotation.y));
    //state.player->acceleration.x = sin(glm::radians(state.player->rotation.y));

    // Accelerate
    if (keys[SDL_SCANCODE_SPACE])
    {
        playerThrust += 0.01f;
    }
    // Decelerate
    if (keys[SDL_SCANCODE_LCTRL])
    {
        playerThrust -= 0.01f;
    }

    state.player->acceleration = glm::radians(glm::normalize(state.player->rotation)) * playerThrust;

    if (state.player->acceleration.z > 0.2f) state.player->acceleration.z = 0.2f;
    if (state.player->acceleration.z < -0.2f) state.player->acceleration.z = -0.2f;
    if (state.player->acceleration.x > 0.2f) state.player->acceleration.x = 0.2f;
    if (state.player->acceleration.x < -0.2f) state.player->acceleration.x = -0.2f;
    */

    // Simplified controls that are more like a first person shooter (no x-axis rotation)
    // Turn Left
    if (keys[SDL_SCANCODE_A])
    {
        currentScene->state.player->rotation.y += 0.25f;
    }
    // Turn Right
    if (keys[SDL_SCANCODE_D])
    {
        currentScene->state.player->rotation.y -= 0.25f;
    }
    
    //currentScene->state.player->velocity.x = 0;
    currentScene->state.player->velocity.y = 0;
    //currentScene->state.player->velocity.z = 0;

    // Accelerate
    if (keys[SDL_SCANCODE_W])
    {
        //currentScene->state.player->acceleration.z = cos(glm::radians(currentScene->state.player->rotation.y)) * -0.5f;
        //currentScene->state.player->acceleration.x = sin(glm::radians(currentScene->state.player->rotation.y)) * -0.5f;
        currentScene->state.player->thrust += 0.07f;
    }
    // Decelerate
    if (keys[SDL_SCANCODE_S] && currentScene->state.player->thrust > 0.1f)
    {
        //currentScene->state.player->acceleration.z = cos(glm::radians(currentScene->state.player->rotation.y)) * 0.5f;
        //currentScene->state.player->acceleration.x = sin(glm::radians(currentScene->state.player->rotation.y)) * 0.5f;
        currentScene->state.player->thrust -= 0.07f;
    }

    // Ascend
    if (keys[SDL_SCANCODE_SPACE])
    {
        currentScene->state.player->velocity.y = 0.25f;
    }
    // Descend
    if (keys[SDL_SCANCODE_LCTRL])
    {
        currentScene->state.player->velocity.y = -0.25f;
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

    viewMatrix = glm::mat4(1.0f);
    viewMatrix = glm::rotate(viewMatrix, glm::radians(currentScene->state.player->rotation.y), glm::vec3(0, -1.0f, 0));
    viewMatrix = glm::translate(viewMatrix, -currentScene->state.player->position);
}

void Render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program.programID);
    
    // render the scene
    currentScene->Render(&program, viewMatrix, modelMatrix, projectionMatrix, uiViewMatrix, uiProjectionMatrix);

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
