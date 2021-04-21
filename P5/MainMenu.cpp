#include "MainMenu.h"

Entity logo;

void MainMenu::Initialize()
{
    // initialize the main menu
    state.player = new Entity(); // necessary to prevent a segfault
    state.lives = 3; // set how many lives the player has
    state.isGameOver = false;

    // black background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // ***Initialize UI Font***
    state.UIfont = Util::LoadTexture("Assets/font1.png");

    // ***Initialize Title Screen Logo***
    logo.textureID = Util::LoadTexture("Assets/logo.png");
    logo.modelMatrix = glm::mat4(1.0f);
    logo.modelMatrix = glm::translate(logo.modelMatrix, glm::vec3(5, -3, 0.0f));
    logo.modelMatrix = glm::scale(logo.modelMatrix, glm::vec3(9.0f, 4.5f, 1.0f));
}

void MainMenu::Update(float deltaTime)
{
    // update the main menu
}

void MainMenu::Render(ShaderProgram* program)
{
    // render menu splash screen
    logo.Render(program);

    // render text
    Util::DrawText(program, state.UIfont, "PRESS ENTER TO START", 0.5f, -0.2f, glm::vec3(2.25f, -6, 0.0f));
}

