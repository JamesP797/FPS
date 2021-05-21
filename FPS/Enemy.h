#ifndef ENEMY_H
#define ENEMY_H
#include "Model.h"
#define NUM_PARTICLES 40

struct Particle {
	glm::vec3 Position, Velocity, Color, RotationDirection;
	float RotationAmount, aliveTime, duration;
};

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
	bool render;
	bool dropping;
	bool tracking;
	Particle particles[NUM_PARTICLES];
	bool gettingShot;

	Enemy(glm::vec3 initialPos, string modelPath, float initialYaw, unsigned int enemyTexture, unsigned int enemyTextureSpec)
	{
		health = 100;
		obj = new Model(modelPath);
		position = initialPos;
		yaw = initialYaw;
		texture = enemyTexture;
		textureSpec = enemyTextureSpec;
		front = glm::vec3(0.0f, 0.0f, 1.0f);
		MovementSpeed = 1.0f;
		alive = true;
		render = false;
		dropping = false;
		tracking = false;
		gettingShot = false;

		// setup particles
		srand(time(NULL));
		for (int i = 0; i < NUM_PARTICLES; i++)
		{
			Particle particle;
			particle.Position = this->position + glm::normalize(glm::vec3((float)rand() / (float)RAND_MAX - 0.5f, (float)rand() / (float)RAND_MAX - 0.5f, (float)rand() / (float)RAND_MAX - 0.5f)) / 6.0f;
			particle.Velocity = glm::vec3((float)rand() / (float)RAND_MAX / 10.0f, (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX / 10.0f);
			particle.RotationDirection = glm::normalize(glm::vec3((float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX));
			particle.Color = glm::vec3(1.0f, 1.0f, 0.0f);
			particle.RotationAmount = rand() % 360;
			particle.aliveTime = 0;
			particle.duration = (float)rand() / (float)RAND_MAX * 2;
			particles[i] = particle;
		}
	}


	void move(float deltaTime, glm::vec3 playerPos)
	{
		// turn
		updateVectors();

		// move towards front
		float velocity = this->MovementSpeed * deltaTime;
		if (this->dropping) 
		{
			// move down
			this->position += (glm::vec3(0.0f, -1.0f, 0.0f) * velocity);
			if (this->position.y < 0.0f)
			{
				dropping = false;
				tracking = true;
			}
		}
		else if (this->tracking && this->alive)
		{
			this->position += (glm::normalize(playerPos - this->position) * velocity);
			glm::vec3 frontNorm = glm::vec3(0.0f, 0.0f, 1.0f);
			glm::vec3 toPlayerNorm = glm::normalize(playerPos - this->position);
			this->yaw = glm::acos(glm::dot(frontNorm, toPlayerNorm));
		}
		else if (!this->alive)
		{
			this->position.y -= deltaTime;
			if (this->position.y < -5)
			{
				render = false;
			}
		}
	}

	void draw(Shader shader, Shader* lightSourceShader, unsigned int particleVAO)
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
		model = glm::rotate(model, this->yaw, glm::vec3(0.0f, 1.0f, 0.0f));
		shader.setMat4("model", model);
		this->obj->Draw(shader);

		lightSourceShader->use();
		for (int i = 0; i < NUM_PARTICLES; i++)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, particles[i].Position);
			model = glm::rotate(model, glm::radians(particles[i].RotationAmount), particles[i].RotationDirection);
			model = glm::scale(model, glm::vec3(0.02f, 0.02f, 0.02f));
			lightSourceShader->setMat4("model", model);
			lightSourceShader->setVec3("lightColor", particles[i].Color);
			glBindVertexArray(particleVAO);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
		shader.use();
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

	// returns true if we died
	bool takeDamage()
	{
		health--;
		if (health <= 0) {
			alive = false;
			return true;
		}
		return false;
	}

	void debug()
	{
		printf("%f %f %f\n", position.x, position.y, position.z);
	}

	void updateParticles(float deltaTime)
	{
		for (int i = 0; i < NUM_PARTICLES; i++)
		{
			particles[i].Position += particles[i].Velocity * deltaTime;
			particles[i].aliveTime += deltaTime;
			particles[i].Color.g = -particles[i].aliveTime / particles[i].duration + 1;
			if (particles[i].aliveTime > particles[i].duration)
			{
				if (gettingShot) {
					particles[i].Position = this->position + glm::normalize(glm::vec3((float)rand() / (float)RAND_MAX - 0.5f, (float)rand() / (float)RAND_MAX - 0.5f, (float)rand() / (float)RAND_MAX - 0.5f)) / 6.0f;
					particles[i].Velocity = glm::vec3((float)rand() / (float)RAND_MAX / 10.0f, (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX / 10.0f);
					particles[i].aliveTime = 0;
					particles[i].RotationDirection = glm::normalize(glm::vec3((float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX));
					particles[i].Color = glm::vec3(1.0f, 1.0f, 0.0f);
					particles[i].RotationAmount = rand() % 360;
				}
				else {
					particles[i].Position = glm::vec3(-100.0f, -100.0f, -100.0f);
				}
			}
		}
	}
};


#endif

