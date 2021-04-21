#include "Entity.h"

Entity::Entity()
{
    position = glm::vec3(0);
    initPosition = position;
    movement = glm::vec3(0);
    acceleration = glm::vec3(0);
    velocity = glm::vec3(0);
    speed = 0;

    width = 1.0f;
    height = 1.0f;
    
    modelMatrix = glm::mat4(1.0f);
}

bool Entity::CheckCollision(Entity* other)
{
    if (!isActive || !other->isActive)
    {
        return false;
    }

    float xdist = fabs(position.x - other->position.x) - ((width + other->width) / 2.0f);
    float ydist = fabs(position.y - other->position.y) - ((height + other->height) / 2.0f);

    if (xdist < 0 && ydist < 0)
    {
        lastCollision = other->entityType;
        lastCollisionEntity = other;
        return true;
    }
    else
    {
        return false;
    }
}

void Entity::CheckCollisionsX(Entity* objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity* object = &objects[i];

        if (CheckCollision(object))
        {
            float xdist = fabs(position.x - object->position.x);
            float penetrationX = fabs(xdist - (width / 2.0f) - (object->width / 2.0f));
            // moving right
            if (velocity.x > 0)
            {
                position.x -= penetrationX;
                velocity.x = 0;
                collidedRight = true;
            }
            // moving left
            else if (velocity.x < 0)
            {
                position.x += penetrationX;
                velocity.x = 0;
                collidedLeft = true;
            }
        }
    }
}

void Entity::CheckCollisionsY(Entity* objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity* object = &objects[i];
 
        if (CheckCollision(object))
        {
            float ydist = fabs(position.y - object->position.y);
            float penetrationY = fabs(ydist - (height / 2.0f) - (object->height / 2.0f));
            // moving up
            if (velocity.y > 0)
            {
                position.y -= penetrationY;
                velocity.y = 0;
                collidedTop = true;
                object->collidedBottom = true;
            }
            // moving down
            else if (velocity.y < 0)
            {
                position.y += penetrationY;
                velocity.y = 0;
                collidedBottom = true;
                object->collidedTop = true;
            }
        }
    }
}

void Entity::CheckCollisionsX(Map* map)
{
    // Probes for tiles
    glm::vec3 left = glm::vec3(position.x - (width / 2), position.y, position.z);
    glm::vec3 right = glm::vec3(position.x + (width / 2), position.y, position.z);

    float penetration_x = 0;
    float penetration_y = 0;

    if (map->IsSolid(left, &penetration_x, &penetration_y) && velocity.x < 0)
    {
        position.x += penetration_x;
        velocity.x = 0;
        collidedLeft = true;
    }

    if (map->IsSolid(right, &penetration_x, &penetration_y) && velocity.x > 0)
    {
        position.x -= penetration_x;
        velocity.x = 0;
        collidedRight = true;
    }
}

void Entity::CheckCollisionsY(Map* map)
{
    // Probes for tiles
    glm::vec3 top = glm::vec3(position.x, position.y + (height / 2), position.z);
    glm::vec3 top_left = glm::vec3(position.x - (width / 2), position.y + (height / 2), position.z);
    glm::vec3 top_right = glm::vec3(position.x + (width / 2), position.y + (height / 2), position.z);

    glm::vec3 bottom = glm::vec3(position.x, position.y - (height / 2), position.z);
    glm::vec3 bottom_left = glm::vec3(position.x - (width / 2), position.y - (height / 2), position.z);
    glm::vec3 bottom_right = glm::vec3(position.x + (width / 2), position.y - (height / 2), position.z);

    float penetration_x = 0;
    float penetration_y = 0;

    if (map->IsSolid(top, &penetration_x, &penetration_y) && velocity.y > 0)
    {
        position.y -= penetration_y;
        velocity.y = 0;
        collidedTop = true;
    }
    else if (map->IsSolid(top_left, &penetration_x, &penetration_y) && velocity.y > 0)
    {
        position.y -= penetration_y;
        velocity.y = 0;
        collidedTop = true;
    }
    else if (map->IsSolid(top_right, &penetration_x, &penetration_y) && velocity.y > 0)
    {
        position.y -= penetration_y;
        velocity.y = 0;
        collidedTop = true;
    }

    if (map->IsSolid(bottom, &penetration_x, &penetration_y) && velocity.y < 0)
    {
        position.y += penetration_y;
        velocity.y = 0;
        collidedBottom = true;
    }
    else if (map->IsSolid(bottom_left, &penetration_x, &penetration_y) && velocity.y < 0)
    {
        position.y += penetration_y;
        velocity.y = 0;
        collidedBottom = true;
    }
    else if (map->IsSolid(bottom_right, &penetration_x, &penetration_y) && velocity.y < 0)
    {
        position.y += penetration_y;
        velocity.y = 0;
        collidedBottom = true;
    }
}

void Entity::AI(Map* map, Entity* player)
{
    switch (aiType)
    {
        case WALKER:
            AIWalker();
            break;

        case WAITANDGO:
            AIWaitAndGo(player);
            break;

        case SINEFLYER:
            AISineFlyer();
            break;
    }

    CheckCollisionsY(player, 1);
    CheckCollisionsX(player, 1);
}

void Entity::AIWalker()
{
    aiState = WALKING;

    // change x-axis direction
    if (lastCollision == PLATFORM && (collidedRight || collidedLeft))
    {
        movement.x *= -1;
    }
}

void Entity::AIWaitAndGo(Entity* player)
{
    switch (aiState)
    {
        case IDLE:
            if (glm::distance(position, player->position) < 3.0f)
            {
                aiState = WALKING;
            }
            break;

        case WALKING:
            if (player->position.x < position.x)
            {
                movement = glm::vec3(-1, 0, 0);
            }
            else
            {
                movement = glm::vec3(1, 0, 0);
            }
            break;

        case ATTACKING:
            break;
    }
}

void Entity::AISineFlyer()
{
    aiState = WALKING;

    // change x-axis direction
    if (lastCollision == PLATFORM && (collidedRight || collidedLeft))
    {
        movement.x *= -1;
    }

    // move in the y-axis
    movement.y += direction;

    float upperLimit = initPosition.y + 0.05f;
    float lowerLimit = initPosition.y - 0.05f;

    if (position.y >= upperLimit)
    {
        direction = -0.01f;
    }
    if (position.y <= lowerLimit)
    {
        direction = 0.01f;
    }

    velocity.y = movement.y * speed;
}

void Entity::Update(float deltaTime, Map* map, Entity* player, Entity* enemies, int enemyCount)
{
    // don't update non-active entities
    if (!isActive)
    {
        return;
    }

    if (entityType == ENEMY)
    {
        AI(map, player);
    }

    collidedTop = false;
    collidedBottom = false;
    collidedLeft = false;
    collidedRight = false;

    // animate the entity when it is moving
    if (animIndices != NULL)
    {
        if (glm::length(movement) != 0)
        {
            animTime += deltaTime;

            if (animTime >= 0.25f)
            {
                animTime = 0.0f;
                animIndex++;
                if (animIndex >= animFrames)
                {
                    animIndex = 0;
                }
            }
        }
        else
        {
            animIndex = 0;
        }
    }

    // handle an entity jumping
    if (jump)
    {
        jump = false;

        velocity.y += jumpPower;
    }

    // handle movement
    velocity.x = movement.x * speed;
    velocity += acceleration * deltaTime;

    // move on the y-axis
    position.y += velocity.y * deltaTime;
    // check for collisions and fix the y-axis position if necessary
    CheckCollisionsY(map);
    CheckCollisionsY(enemies, enemyCount);

    // move on the x-axis
    position.x += velocity.x * deltaTime;
    // check for collisions and fix the x-axis position if necessary
    CheckCollisionsX(map);
    CheckCollisionsX(enemies, enemyCount);

    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
}

void Entity::DrawSpriteFromTextureAtlas(ShaderProgram *program, GLuint textureID, int index)
{
    float u = (float)(index % animCols) / (float)animCols;
    float v = (float)(index / animCols) / (float)animRows;
    
    float width = 1.0f / (float)animCols;
    float height = 1.0f / (float)animRows;
    
    float texCoords[] = { u, v + height, u + width, v + height, u + width, v,
        u, v + height, u + width, v, u, v};
    
    float vertices[]  = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void Entity::Render(ShaderProgram *program)
{
    if (!isActive)
    {
        return;
    }

    program->SetModelMatrix(modelMatrix);
    
    if (animIndices != NULL)
    {
        DrawSpriteFromTextureAtlas(program, textureID, animIndices[animIndex]);
        return;
    }
    
    float vertices[]  = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}
