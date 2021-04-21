#include "Level2.h"

#define LEVEL2_WIDTH 30
#define LEVEL2_HEIGHT 8

#define LEVEL2_ENEMY_COUNT 3

unsigned int level2_data[] =
{
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 3,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 0, 0, 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 0, 0, 3,
    3, 0, 0, 0, 0, 3, 3, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 3,
    3, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 1, 1, 0, 0, 0, 0, 0, 3,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 1, 1, 2, 2, 2, 2, 2, 2, 1, 1, 0, 0, 0, 3,
    3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 3
};

void Level2::Initialize()
{
    state.nextScene = -1;

    // set background to grey
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    // Initialize Game Objects

    // ***Initialize Player***
    state.player = new Entity();
    state.player->entityType = PLAYER;
    state.player->position = glm::vec3(2, -6, 0);
    state.player->movement = glm::vec3(0);
    state.player->acceleration = glm::vec3(0, -9.8f, 0);
    state.player->velocity = glm::vec3(0);
    state.player->speed = 3.0f;

    state.player->height = 1.0f;
    state.player->width = 0.5f;

    state.player->jumpPower = 6.0f;

    state.player->textureID = Util::LoadTexture("Assets/player.png");

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

    // ***Initialize Tileset***
    GLuint mapTextureID = Util::LoadTexture("Assets/tileset.png");
    state.map = new Map(LEVEL2_WIDTH, LEVEL2_HEIGHT, level2_data, mapTextureID, 1.0f, 4, 1);

    // ***Initialize Enemies***
    state.enemies = new Entity[LEVEL2_ENEMY_COUNT];
    GLuint enemyTextureID = Util::LoadTexture("Assets/enemy.png");

    // set all enemies to have entityType 'ENEMY', texture ID 'enemyTextureID', and width
    for (int i = 0; i < LEVEL2_ENEMY_COUNT; i++)
    {
        state.enemies[i].entityType = ENEMY;
        state.enemies[i].textureID = enemyTextureID;

        state.enemies[i].width = 0.5f;
    }

    // wait and go chaser
    state.enemies[0].position = glm::vec3(10, -1, 0);
    state.enemies[0].movement = glm::vec3(-1, 0, 0);
    state.enemies[0].speed = 1.0f;
    state.enemies[0].acceleration = glm::vec3(0, -9.8f, 0);
    state.enemies[0].aiType = WALKER;

    // ground patroller
    state.enemies[1].position = glm::vec3(19, -3, 0);
    state.enemies[1].speed = 1.0f;
    state.enemies[1].acceleration = glm::vec3(0, -9.8f, 0);
    state.enemies[1].aiType = WAITANDGO;
    state.enemies[1].aiState = IDLE;

    // flying sine wave patroller
    state.enemies[2].position = glm::vec3(12, -4, 0);
    state.enemies[2].initPosition = state.enemies[2].position;
    state.enemies[2].movement = glm::vec3(1, 0, 0);
    state.enemies[2].speed = 1.0f;
    state.enemies[2].direction = 0.05f;
    state.enemies[2].aiType = SINEFLYER;

    // ***Initialize UI Font***
    state.UIfont = Util::LoadTexture("Assets/font1.png");

    /*
    // ***Initialize Audio***
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    state.music = Mix_LoadMUS("Assets/Audio/crypto.mp3");
    Mix_PlayMusic(state.music, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2);
    */

    state.winSFX = Mix_LoadWAV("Assets/Audio/success.wav");
    state.loseSFX = Mix_LoadWAV("Assets/Audio/failure.wav");
}

void Level2::Update(float deltaTime)
{
    // update player
    state.player->Update(deltaTime, state.map, state.player, state.enemies, LEVEL2_ENEMY_COUNT);

    // update enemies
    for (int i = 0; i < LEVEL2_ENEMY_COUNT; i++)
    {
        state.enemies[i].Update(deltaTime, state.map, state.player, NULL, 0);
    }

    // check if player touched an enemy
    if (state.player->lastCollision == ENEMY && state.player->isActive)
    {
        Entity* enemy = state.player->lastCollisionEntity;

        enemy->CheckCollision(state.player);

        // player jumped on the enemy
        if (state.player->position.y > enemy->position.y + enemy->height * 0.9f)
        {
            enemy->isActive = false;
            state.player->lastCollision = PLAYER;

            state.player->velocity.y += 6.0;
        }
        // player got hit by the enemy
        else
        {
            state.lives--;

            // play sfx
            Mix_PlayChannel(-1, state.loseSFX, 0);

            // go back to start of the level after transition
            state.player->isActive = false;
            //state.playingEffect = FADEOUT;
            Initialize();
            //state.playingEffect = FADEIN;

            if (state.lives <= 0)
            {
                state.isGameOver = true;
            }
        }
    }

    // scene transition
    if (state.player->position.x >= LEVEL2_WIDTH - 2)
    {
        state.nextScene = 3;
    }
}

void Level2::Render(ShaderProgram* program)
{
    // render tilemap
    state.map->Render(program);

    // render enemies
    for (int i = 0; i < LEVEL2_ENEMY_COUNT; i++)
    {
        state.enemies[i].Render(program);
    }

    // render player
    state.player->Render(program);

    // render lives UI
    if (state.player->position.x > 5)
    {
        Util::DrawText(program, state.UIfont, "Lives: " + std::to_string(state.lives), 0.5f, -0.3f, glm::vec3(state.player->position.x - 4.75f, -0.25f, 0.0f));
    }
    else
    {
        Util::DrawText(program, state.UIfont, "Lives: " + std::to_string(state.lives), 0.5f, -0.3f, glm::vec3(0.25f, -0.25f, 0.0f));
    }

    // render level number UI
    if (state.player->position.x < 5)
    {
        Util::DrawText(program, state.UIfont, "1-2", 0.5f, -0.3f, glm::vec3(9.25f, -0.25f, 0.0f));
    }
    else
    {
        Util::DrawText(program, state.UIfont, "1-2", 0.5f, -0.3f, glm::vec3(4.25f + state.player->position.x, -0.25f, 0.0f));
    }

    // render UI text for when player loses all lives
    if (state.isGameOver && state.lives <= 0)
    {
        // end game with failure
        if (state.player->position.x < 5)
        {
            Util::DrawText(program, state.UIfont, "You Lose", 0.5f, -0.2f, glm::vec3(4, -3, 0.0f));
        }
        else
        {
            Util::DrawText(program, state.UIfont, "You Lose", 0.5f, -0.2f, glm::vec3(state.player->position.x - 1, -3, 0.0f));
        }
    }
}

