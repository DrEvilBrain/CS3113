#include "Level1.h"

#define LEVEL1_OBJECT_COUNT 1
#define LEVEL1_ENEMY_COUNT 10
#define PLAYER_BULLET_COUNT 300
int BULLET_COUNT;

void Level1::Initialize()
{
    state.isLoading = false;
    state.nextScene = -1;
    BULLET_COUNT = 0;

    // set background to sky blue
    glClearColor(0.3137f, 0.6863f, 0.8941f, 1.0f);

    // Initialize Game Objects

    // Mustang model and texture
    GLuint mustangTextureID = Util::LoadTexture("Assets/planes/mustang.png");
    Mesh* mustangMesh = new Mesh();
    mustangMesh->LoadOBJ("Assets/planes/mustang.obj", 1);

    // Zero model and texture
    GLuint zeroTextureID = Util::LoadTexture("Assets/planes/zero.png");
    Mesh* zeroMesh = new Mesh();
    zeroMesh->LoadOBJ("Assets/planes/zero.obj", 1);

    // Floor cube model and texture
    GLuint floorTextureID = Util::LoadTexture("Assets/floor.jpg");
    Mesh* cubeMesh = new Mesh();
    cubeMesh->LoadOBJ("Assets/cube.obj", 100);

    // ***Initialize Player***
    state.player = new Entity();
    state.player->entityType = PLAYER;
    state.player->textureID = zeroTextureID;
    state.player->mesh = zeroMesh;
    state.player->position = glm::vec3(0, 5, 0);
    state.player->rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    state.player->scale = glm::vec3(0.1f, 0.1f, 0.1f);
    state.player->acceleration = glm::vec3(0, 0, 0);
    state.player->speed = 1.0f;

    // ***Initialize Objects***
    state.objects = new Entity[LEVEL1_OBJECT_COUNT];

    state.objects[0].textureID = floorTextureID;
    state.objects[0].mesh = cubeMesh;
    state.objects[0].position = glm::vec3(0, 0.25f, 0);
    state.objects[0].rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    state.objects[0].scale = glm::vec3(100, 0.5f, 100);
    state.objects[0].entityType = FLOOR;

    // ***Initialize Enemies***
    state.enemies = new Entity[LEVEL1_ENEMY_COUNT];

    for (int i = 0; i < LEVEL1_ENEMY_COUNT/2; i++)
    {
        state.enemies[i].textureID = zeroTextureID;
        state.enemies[i].mesh = zeroMesh;
        state.enemies[i].entityType = ENEMY;
        state.enemies[i].scale = glm::vec3(0.1f, 0.1f, 0.1f);
        state.enemies[i].position = glm::vec3(rand() % 100 - 50, 7, rand() % 100 - 50);
    }

    for (int i = LEVEL1_ENEMY_COUNT / 2; i < LEVEL1_ENEMY_COUNT; i++)
    {
        state.enemies[i].textureID = mustangTextureID;
        state.enemies[i].mesh = mustangMesh;
        state.enemies[i].entityType = ENEMY;
        state.enemies[i].scale = glm::vec3(0.1f, 0.1f, 0.1f);
        state.enemies[i].position = glm::vec3(rand() % 70 - 35, 3, rand() % 70 - 35);
    }

    // ***Initialize Bullets***
    state.bullets = new Entity[PLAYER_BULLET_COUNT];

    // ***Initialize UI Font***
    state.UIfont = Util::LoadTexture("Assets/font1.png");

    // ***Initiailize UI Icons***
    state.UIicon = Util::LoadTexture("Assets/crosshair.png");

    //state.winSFX = Mix_LoadWAV("Assets/Audio/success.wav");
    //state.loseSFX = Mix_LoadWAV("Assets/Audio/failure.wav");
    state.gunSFX = Mix_LoadWAV("Assets/Audio/gunfire.wav");
}

float bulletTimer = 0;

void Level1::Update(float deltaTime)
{
    if (state.isGameOver)
    {
        return;
    }

    bulletTimer += deltaTime;

    // update player
    state.player->Update(deltaTime, state.player, state.objects, LEVEL1_OBJECT_COUNT, state.enemies, LEVEL1_ENEMY_COUNT, NULL, NULL);

    // Fire guns
    if (state.player->isFiring && state.player->ammo > 0)
    {
        state.player->ammo -= 15 * deltaTime;

        if (state.player->ammo < 0)
        {
            state.player->ammo = 0;
        }
        else
        {
            // play sfx
            Mix_PlayChannel(0, state.gunSFX, 0);
        }

        // spawn the bullet (timer prevents bullets from being spawned too quickly)
        if (bulletTimer > 0.5f)
        {
            bulletTimer = 0.0f;

            BULLET_COUNT += 1;
            state.bullets[BULLET_COUNT - 1].position = state.player->position;
            state.bullets[BULLET_COUNT - 1].forward = glm::vec3(state.player->forward);
            state.bullets[BULLET_COUNT - 1].rotation = glm::vec3(state.player->rotation);
            state.bullets[BULLET_COUNT - 1].scale = glm::vec3(0.025f, 0.025f, 0.025f);
            state.bullets[BULLET_COUNT - 1].entityType = BULLET;

            GLuint floorTextureID = Util::LoadTexture("Assets/bullet.jpg");
            Mesh* cubeMesh = new Mesh();
            cubeMesh->LoadOBJ("Assets/cube.obj", 1);
            state.bullets[BULLET_COUNT - 1].textureID = floorTextureID;
            state.bullets[BULLET_COUNT - 1].mesh = cubeMesh;
        } 
    }
    // if no longer firing
    else
    {
        Mix_HaltChannel(0);
    }

    // update objects
    for (int i = 0; i < LEVEL1_OBJECT_COUNT; i++)
    {
        state.objects[i].Update(deltaTime, state.player, NULL, NULL, NULL, NULL, NULL, NULL);
    }

    // update enemies
    for (int i = 0; i < LEVEL1_ENEMY_COUNT; i++)
    {
        state.enemies[i].Update(deltaTime, state.player, NULL, NULL, NULL, NULL, state.bullets, BULLET_COUNT);
    }

    // update bullets
    for (int i = 0; i < BULLET_COUNT; i++)
    {
        state.bullets[i].velocity = glm::vec3(state.bullets[i].forward.x * 10, 0, state.bullets[i].forward.z * 10);
        state.bullets[i].Update(deltaTime, NULL, state.objects, LEVEL1_OBJECT_COUNT, state.enemies, LEVEL1_ENEMY_COUNT, NULL, NULL);
    }

    // trigger game over when player has no lives left or shoots down all enemies
    if (state.player->lives <= 0 || state.player->shotDown >= LEVEL1_ENEMY_COUNT)
    {
        state.isGameOver = true;
    }

    // scene transition
    /*
    if (state.player->position.x >= LEVEL1_WIDTH - 2)
    {
        state.nextScene = 0;
    }
    */
}

void Level1::Render(ShaderProgram* program, glm::mat4 viewMatrix, glm::mat4 modelMatrix, glm::mat4 projectionMatrix, glm::mat4 uiViewMatrix, glm::mat4 uiProjectionMatrix)
{
    // render all 3D stuff
    program->SetProjectionMatrix(projectionMatrix);
    program->SetViewMatrix(viewMatrix);

    //state.player->Render(program);

    for (int i = 0; i < LEVEL1_OBJECT_COUNT; i++)
    {
        state.objects[i].Render(program);
    }

    for (int i = 0; i < LEVEL1_ENEMY_COUNT; i++)
    {
        state.enemies[i].Render(program);
    }

    for (int i = 0; i < BULLET_COUNT; i++)
    {
        state.bullets[i].Render(program);
    }

    // render all 2D stuff now
    program->SetProjectionMatrix(uiProjectionMatrix);
    program->SetViewMatrix(uiViewMatrix);

    // ammo counter
    Util::DrawText(program, state.UIfont, "Ammo: " + std::to_string((int)state.player->ammo), 0.5, -0.25f, glm::vec3(-6, 3.2, 0));

    // objective counter
    Util::DrawText(program, state.UIfont, std::to_string(state.player->shotDown) + "/" + std::to_string(LEVEL1_ENEMY_COUNT) + " enemy planes shot down", 0.5, -0.25f, glm::vec3(-3, 3.2, 0));

    // acceleration counter
    Util::DrawText(program, state.UIfont, "Acceleration: " + std::to_string((int)state.player->thrust), 0.5, -0.25f, glm::vec3(-6, -3.2, 0));

    // crosshair
    Util::DrawIcon(program, state.UIicon, glm::vec3(0, 0, 0), glm::vec3(1));

    // game over text
    if (state.isGameOver && state.player->shotDown < LEVEL1_ENEMY_COUNT)
    {
        Util::DrawText(program, state.UIfont, "You crashed! Mission failed!", 0.5, -0.25f, glm::vec3(-3.0f, 0, 0));
    }

    // victory text
    if (state.isGameOver && state.player->shotDown >= LEVEL1_ENEMY_COUNT)
    {
        Util::DrawText(program, state.UIfont, "Bingo! Mission success!", 0.5, -0.25f, glm::vec3(-2.75f, 0, 0));
    }
}

