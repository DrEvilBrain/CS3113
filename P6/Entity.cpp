#include "Entity.h"

Entity::Entity()
{
    position = glm::vec3(0);
    acceleration = glm::vec3(0);
    velocity = glm::vec3(0);
    rotation = glm::vec3(0);
    scale = glm::vec3(1);

    modelMatrix = glm::mat4(1.0f);
    
    billboard = false;

    width = 1.0f;
    height = 1.0f;
    depth = 1.0f;

    speed = 1.0f;
}

void Entity::Update(float deltaTime, Entity* player, Entity* objects, int objectCount, Entity* enemies, int enemyCount, Entity* bullets, int bulletCount)
{
    glm::vec3 previousPosition = position;

    if (billboard)
    {
        float directionX = position.x - player->position.x;
        float directionZ = position.z - player->position.z;
        rotation.y = glm::degrees(atan2f(directionX, directionZ));

        velocity.z = cos(glm::radians(rotation.y)) * -speed;
        velocity.x = sin(glm::radians(rotation.y)) * -speed;
    }
    
    // simple movement for all non-player entities
    if (entityType != PLAYER)
    {
        velocity += acceleration * deltaTime;
        position += velocity * deltaTime;
    }
    
    if (entityType == PLAYER)
    {
        // Calculate forward direction
        forward = glm::vec3(-sin(glm::radians(rotation.y)), 0, -cos(glm::radians(rotation.y)));

        // Player movement
        if (thrust > 0)
        {
            thrust -= 3 * deltaTime;
        }
        acceleration = forward * thrust * deltaTime;
        velocity += acceleration * deltaTime;

        if (velocity.x > 5) velocity.x = 5;
        if (velocity.x < -5) velocity.x = -5;
        if (velocity.z > 5) velocity.z = 5;
        if (velocity.z < -5) velocity.z = -5;

        position += velocity * deltaTime;

        // Collision detection for the player with enemies
        for (int i = 0; i < enemyCount; i++)
        {
            if (CheckCollision(&enemies[i]))
            {
                lives--;
                position = previousPosition;
                break;
            }
        }

        // Crashed into the ground
        if (position.y < 0.5f)
        {
            lives--;
        }
    }
    
    if (entityType == ENEMY)
    {
        rotation.y += 30 * deltaTime;

        // Collision detection with player bullets
        for (int i = 0; i < bulletCount; i++)
        {
            if (CheckCollision(&bullets[i]) && isActive)
            {
                isActive = false;
                player->shotDown++;
            }
        }
    }

    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::scale(modelMatrix, scale);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
}

void Entity::Render(ShaderProgram *program)
{
    program->SetModelMatrix(modelMatrix);
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    if (!isActive)
    {
        return;
    }

    if (billboard)
    {
        DrawBillboard(program);
    }
    else
    {
        mesh->Render(program);
    }
}

void Entity::DrawBillboard(ShaderProgram* program)
{
    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

bool Entity::CheckCollision(Entity* other)
{
    float xdist = fabs(position.x - other->position.x) - ((width + other->width) / 2.0f);
    float ydist = fabs(position.y - other->position.y) - ((height + other->height) / 2.0f);
    float zdist = fabs(position.z - other->position.z) - ((depth + other->depth) / 2.0f);

    if (xdist < 0 && ydist < 0 && zdist < 0) return true;
    return false;
}

