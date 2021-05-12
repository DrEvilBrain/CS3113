#pragma once
#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_mixer.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#include "Effects.h"
#include "Util.h"
#include "Entity.h"

struct GameState
{
    Entity* player;
    Entity* objects;
    Entity* enemies;
    Entity* bullets;

    GLuint UIfont;
    GLuint UIicon;

    Mix_Music* music;
    Mix_Chunk* winSFX;
    Mix_Chunk* loseSFX;
    Mix_Chunk* gunSFX;

    EffectType playingEffect;

    bool isLoading = false;
    bool isGameOver = false;
    int lives;

    int nextScene;
};

class Scene
{
public:
	GameState state;
	virtual void Initialize() = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Render(ShaderProgram* program, glm::mat4 viewMatrix, glm::mat4 modelMatrix, glm::mat4 projectionMatrix, glm::mat4 uiViewMatrix, glm::mat4 uiProjectionMatrix) = 0;
};

