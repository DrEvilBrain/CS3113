#pragma once
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
#include "Mesh.h"

enum EntityType
{
    PLAYER,
    ENEMY,
    FLOOR,
    BULLET
};

class Entity
{
public:
    EntityType entityType;
    
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    glm::vec3 rotation;
    glm::vec3 scale;

    glm::vec3 forward;
    float thrust;
    
    float speed;
    bool billboard;
    float width;
    float height;
    float depth;
    
    GLuint textureID;
    Mesh* mesh;
    
    glm::mat4 modelMatrix;

    bool isFiring = false;
    bool isActive = true;

    float ammo = 1000;
    int lives = 1;
    int shotDown = 0;

    Entity();
    
    void Update(float deltaTime, Entity* player, Entity* objects, int objectCount, Entity* enemies, int enemyCount, Entity* bullets, int bulletCount);
    void Render(ShaderProgram *program);
    void DrawBillboard(ShaderProgram* program);
    bool CheckCollision(Entity* other);
};

