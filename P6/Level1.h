#include "Scene.h"

class Level1 : public Scene
{
public:
	void Initialize() override;
	void Update(float deltaTime) override;
	void Render(ShaderProgram* program, glm::mat4 viewMatrix, glm::mat4 modelMatrix, glm::mat4 projectionMatrix, glm::mat4 uiViewMatrix, glm::mat4 uiProjectionMatrix) override;
};
