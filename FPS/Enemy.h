#ifndef ENEMY_H
#define ENEMY_H
#include "Model.h"

class Enemy
{
public:
	glm::vec3 position;
	float yaw;
	glm::vec3 front;
	int health;
	Model* obj;
	unsigned int texture;
	unsigned int textureSpec;
	float MovementSpeed;
	bool alive;

	Enemy(glm::vec3 initialPos, string modelPath, float initialYaw, unsigned int enemyTexture, unsigned int enemyTextureSpec)
	{
		health = 10;
		obj = new Model(modelPath);
		position = initialPos;
		yaw = initialYaw;
		texture = enemyTexture;
		textureSpec = enemyTextureSpec;
		front = glm::vec3(0.0f, 0.0f, 1.0f);
		MovementSpeed = 1.0f;
		alive = true;
	}


	void move(float deltaTime)
	{
		// turn
		this->yaw += deltaTime * 100;
		updateVectors();

		// move towards front
		float velocity = this->MovementSpeed * deltaTime;
		this->position += (this->front * velocity);
		this->position.y = glm::sin(glfwGetTime());
	}

	void draw(Shader shader)
	{
		// use rusty texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textureSpec);

		// render enemy
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(this->position));
		model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
		model = glm::rotate(model, glm::radians(this->yaw), glm::vec3(0.0f, 1.0f, 0.0f));
		shader.setMat4("model", model);
		this->obj->Draw(shader);
	}

	void updateVectors()
	{
		// calculate the new Front vector
		glm::vec3 frontTemp;
		frontTemp.x = cos(glm::radians(this->yaw)) * cos(glm::radians(0.0f));
		frontTemp.y = sin(glm::radians(0.0f));
		frontTemp.z = sin(glm::radians(this->yaw)) * cos(glm::radians(0.0f));
		this->front = glm::normalize(frontTemp);
	}

	bool playerLooking(glm::vec3 cameraPos, glm::vec3 cameraFront)
	{
		// The point where we expect the enemy to be
		glm::vec3 point = cameraPos + cameraFront * glm::distance(cameraPos, position);

		return glm::abs(point.x - position.x) < 0.5f && glm::abs(point.y - position.y) < 0.5f && glm::abs(point.z - position.z) < 0.5f;
	}

	void takeDamage()
	{
		health--;
		if (health <= 0)
			alive = false;
	}

	void debug()
	{
		printf("%f %f %f\n", position.x, position.y, position.z);
	}
};


#endif

