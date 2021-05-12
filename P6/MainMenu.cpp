#include "MainMenu.h"

Entity logo;

void MainMenu::Initialize()
{
    // initialize the main menu
    state.player = new Entity(); // necessary to prevent a segfault
    state.lives = 1; // set how many lives the player has
    state.isLoading = false;
    state.isGameOver = false;

    // black background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // ***Initialize UI Font***
    state.UIfont = Util::LoadTexture("Assets/font1.png");

    // ***Initialize Title Screen Logo***
    logo.textureID = Util::LoadTexture("Assets/logo.png");
}

void MainMenu::Update(float deltaTime)
{
    // update the main menu
}

void MainMenu::Render(ShaderProgram* program, glm::mat4 viewMatrix, glm::mat4 modelMatrix, glm::mat4 projectionMatrix, glm::mat4 uiViewMatrix, glm::mat4 uiProjectionMatrix)
{
    program->SetProjectionMatrix(uiProjectionMatrix);
    program->SetViewMatrix(uiViewMatrix);

    // render menu splash screen
    Util::DrawIcon(program, logo.textureID, glm::vec3(0, 1, 0), glm::vec3(12.0f, 4.25f, 1.0f));

    // render text
    if (state.isLoading)
    {
        Util::DrawText(program, state.UIfont, "LOADING", 0.5f, -0.2f, glm::vec3(-1.0f, -2.5f, 0));
    }
    else
    {
        Util::DrawText(program, state.UIfont, "PRESS ENTER TO START", 0.5f, -0.2f, glm::vec3(-3, -2.5f, 0));
    }
}

