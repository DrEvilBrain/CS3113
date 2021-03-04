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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

SDL_Window* displayWindow;
bool gameIsRunning = true;
bool gameStarted = false;
bool gameOver = true;
float windowWidth = 10.0f;
float windowHeight = 7.5f;
float windowDepth = 1.0f;

ShaderProgram program;
glm::mat4 viewMatrix, projectionMatrix;
glm::mat4 playerOneMatrix, playerTwoMatrix, ballMatrix;

// 1P paddle starting position and movement
glm::vec3 playerOne_position = glm::vec3(-(windowWidth - 2.0f), 0, 0);
glm::vec3 playerOne_movement = glm::vec3(0, 0, 0);

// 2P paddle starting position and movement
glm::vec3 playerTwo_position = glm::vec3(windowWidth - 2.0f, 0, 0);
glm::vec3 playerTwo_movement = glm::vec3(0, 0, 0);

// ball starting position and movement
glm::vec3 ball_position = glm::vec3(0, 0, 0);
glm::vec3 ball_movement = glm::vec3(0, 0, 0);

GLuint playerOneTextureID;
GLuint playerTwoTextureID;
GLuint ballTextureID;

float paddleWidth = 0.5f;
float paddleHeight = 3.0f;

float player_speed = 15.0f;
float ball_speed = 6.0f;

// loads textures onto a model
GLuint LoadTexture(const char* filePath) {
    int w, h, n;
    unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);

    if (image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(image);
    return textureID;
}

// initializes the game with our camera and game object data
void Initialize() {
    // initialize random seed for starting ball serves
    srand(time(0));

    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Project 2: Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

    #ifdef _WINDOWS
        glewInit();
    #endif

    glViewport(0, 0, 640, 480);

    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");

    viewMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-windowWidth, windowWidth, -windowHeight, windowHeight, -windowDepth, windowDepth);

    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);

    glUseProgram(program.programID);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_BLEND);

    // Good setting for transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Game Object Data
    playerOneMatrix = glm::mat4(1.0f);
    playerTwoMatrix = glm::mat4(1.0f);
    ballMatrix = glm::mat4(1.0f);

    playerOneTextureID = LoadTexture("Assets/1P.png");
    playerTwoTextureID = LoadTexture("Assets/2P.png");
    ballTextureID = LoadTexture("Assets/ball.png");
}

enum Wall
{
    Up = 0,
    Right = 1,
    Down = 2,
    Left = 3
};

// checks if the object is colliding with the window's walls
bool IsScreenColliding(glm::vec3 object_position, glm::mat4 objectModelMatrix, Wall wallToCheck) {
    float widthAvg = objectModelMatrix[0].x / 2.0f;
    float heightAvg = objectModelMatrix[1].y / 2.0f;

    float xdist = fabs(fabs(object_position.x) - fabs(windowWidth)) - widthAvg;
    float ydist = fabs(fabs(object_position.y) - fabs(windowHeight)) - heightAvg;

    if (xdist < 0)
    {
        if ((wallToCheck == Right && object_position.x > 0) || (wallToCheck == Left && object_position.x < 0))
        {
            return true;
        }
    }

    if (ydist < 0)
    {
        if ((wallToCheck == Up && object_position.y > 0) || (wallToCheck == Down && object_position.y < 0))
        {
            return true;
        }
    }

    return false;
}

// checks if two models are box to box colliding
bool IsBoxColliding(glm::vec3 objectOne_position, glm::mat4 objectOneMatrix,
                    glm::vec3 objectTwo_position, glm::mat4 objectTwoMatrix) {
    float widthAvg = (objectOneMatrix[0].x + objectTwoMatrix[0].x) / 2.0f;
    float heightAvg = (objectOneMatrix[1].y + objectTwoMatrix[1].y) / 2.0f;

    float xdist = fabs(objectTwo_position.x - objectOne_position.x) - widthAvg;
    float ydist = fabs(objectTwo_position.y - objectOne_position.y) - heightAvg;

    if (xdist < 0 && ydist < 0)
    {
        return true;
    }
    
    return false;
}

// handles game input
//
// 1P controls:
// - W is Up
// - S is Down
// 
// 2P controls:
// - Up Arrow is Up
// - Down Arrow is Down
void ProcessInput() {
    // reset player movement
    playerOne_movement = glm::vec3(0);
    playerTwo_movement = glm::vec3(0);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                gameIsRunning = false;
                break;

            case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				case SDLK_SPACE:
					// Start/restart the game only when the game isn't running
                    if (!gameStarted)
                    {
                        ball_position = glm::vec3(0);

                        int i = rand() % 4;
                        
                        if (i == 0)
                        {
                            ball_movement.x = 1.0f;
                            ball_movement.y = 1.0f;
                        }
                        if (i == 1)
                        {
                            ball_movement.x = 1.0f;
                            ball_movement.y = -1.0f;
                        }
                        if (i == 2)
                        {
                            ball_movement.x = -1.0f;
                            ball_movement.y = 1.0f;
                        }
                        if (i == 3)
                        {
                            ball_movement.x = -1.0f;
                            ball_movement.y = -1.0f;
                        }
                        
                        ball_speed = 6.0f;
                        gameStarted = true;
                        gameOver = false;
                        ballTextureID = LoadTexture("Assets/ball.png");
                    }
					break;
				}
                break; // SDL_KEYDOWN
        }
    }

    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    // 1P paddle move up
    if (keys[SDL_SCANCODE_W]) {
        if (!IsScreenColliding(playerOne_position, playerOneMatrix, Up) && !gameOver)
        {
            playerOne_movement.y = 1.0f;
        }
    }
    // 1P paddle move down
    if (keys[SDL_SCANCODE_S]) {
        if (!IsScreenColliding(playerOne_position, playerOneMatrix, Down) && !gameOver)
        {
            playerOne_movement.y = -1.0f;
        }
    }
    // 2P paddle move up
    if (keys[SDL_SCANCODE_UP]) {
        if (!IsScreenColliding(playerTwo_position, playerTwoMatrix, Up) && !gameOver)
        {
            playerTwo_movement.y = 1.0f;
        }
    }
    // 2P paddle move down
    if (keys[SDL_SCANCODE_DOWN]) {
        if (!IsScreenColliding(playerTwo_position, playerTwoMatrix, Down) && !gameOver)
        {
            playerTwo_movement.y = -1.0f;
        }
    }
}

float lastTicks = 0.0f;

// runs every frame
void Update() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
	float deltaTime = ticks - lastTicks;
	lastTicks = ticks;

    // make the ball bounce off whenever it collides with the top and bottom walls
    if (IsScreenColliding(ball_position, ballMatrix, Up) && gameStarted)
    {
        ball_movement.y = -1.0f;
    }
    if (IsScreenColliding(ball_position, ballMatrix, Down) && gameStarted)
    {
        ball_movement.y = 1.0f;
    }
    // make the ball bounce off whenever it collides with either player paddles
    if (IsBoxColliding(ball_position, ballMatrix, playerOne_position, playerOneMatrix) && gameStarted)
    {
        ball_movement.x = 1.0f;
        ball_speed += 0.25f;
    }
    if (IsBoxColliding(ball_position, ballMatrix, playerTwo_position, playerTwoMatrix) && gameStarted)
    {
        ball_movement.x = -1.0f;
        ball_speed += 0.25f;
    }
    // make the ball stop moving once it collides with the right or left walls
    if (IsScreenColliding(ball_position, ballMatrix, Right) && gameStarted)
    {
        ball_movement.x = 0;
        ball_movement.y = 0;

        gameOver = true;
        gameStarted = false;
        ballTextureID = LoadTexture("Assets/ball_game_over.png");
    }
    if (IsScreenColliding(ball_position, ballMatrix, Left) && gameStarted)
    {
        ball_movement.x = 0;
        ball_movement.y = 0;

        gameOver = true;
        gameStarted = false;
        ballTextureID = LoadTexture("Assets/ball_game_over.png");
    }

	// Add (direction * units per second * elapsed time)
	playerOne_position += playerOne_movement * player_speed * deltaTime;
    playerTwo_position += playerTwo_movement * player_speed * deltaTime;
    ball_position += ball_movement * ball_speed * deltaTime;

    // move 1P paddle
    playerOneMatrix = glm::mat4(1.0f);
    playerOneMatrix = glm::translate(playerOneMatrix, playerOne_position);
    
    // move 2P paddle
    playerTwoMatrix = glm::mat4(1.0f);
    playerTwoMatrix = glm::translate(playerTwoMatrix, playerTwo_position);

    // scale the paddles
    playerOneMatrix = glm::scale(playerOneMatrix, glm::vec3(paddleWidth, paddleHeight, 1.0f));
    playerTwoMatrix = glm::scale(playerTwoMatrix, glm::vec3(paddleWidth, paddleHeight, 1.0f));

    // move ball
    ballMatrix = glm::mat4(1.0f);
    ballMatrix = glm::translate(ballMatrix, ball_position);
}

// renders our models
void Render() {
    glClear(GL_COLOR_BUFFER_BIT);

    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);

    // render 1P paddle
    program.SetModelMatrix(playerOneMatrix);
    glBindTexture(GL_TEXTURE_2D, playerOneTextureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // render 2P paddle
    program.SetModelMatrix(playerTwoMatrix);
    glBindTexture(GL_TEXTURE_2D, playerTwoTextureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // render ball
    program.SetModelMatrix(ballMatrix);
    glBindTexture(GL_TEXTURE_2D, ballTextureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);

    SDL_GL_SwapWindow(displayWindow);
}

// end the game
void Shutdown() {
    SDL_Quit();
}

// calls other functions to start, update, and end the game
int main(int argc, char* argv[]) {
    Initialize();

    while (gameIsRunning) {
        ProcessInput();
        Update();
        Render();
    }

    Shutdown();
    return 0;
}
