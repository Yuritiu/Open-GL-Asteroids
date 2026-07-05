#include "MainGame.h"
#include "Camera.h"
#include "DLLManager.h"
#include <glm/gtc/type_ptr.hpp>  
#include <glm/gtc/constants.hpp>
#include <iostream>
#include <string>
#include <algorithm>
#include <SDL_ttf.h>
#include "CollisionSystem.h"
#include <ctime>

namespace
{
	constexpr size_t MODEL_MATRIX_OFFSET = 0;
	constexpr size_t VIEW_MATRIX_OFFSET = sizeof(glm::mat4);
	constexpr size_t PROJECTION_MATRIX_OFFSET = sizeof(glm::mat4) * 2;
}

MainGame::MainGame()
	: _gameDisplay("OpenGL Game", 1980, 1024), // Initialize the display wrapper
	_gameState(GameState::PLAY), counter(0.0f) 
{
	fixedTimeStep = 1.0f / getRefreshRate(); // Dynamically set refresh-based time step
}

MainGame::~MainGame()
{
	uiSystem.cleanup();

	delete player;
	player = nullptr;
}

void MainGame::run()
{
	initSystems(); 
	gameLoop();
}

void MainGame::initSystems() {
	lastFrameTime = SDL_GetTicks() / 1000.0f;
	deltaTime = 0.0f;
	accumulator = 0.0f;

	srand(static_cast<unsigned int>(time(nullptr)));
	
	loadMeshes();
	loadTextures();
	setupUBOs();
	loadShaders();
	setupCamera();
	loadPhysicsEngine();
	initPlayer(&playerMesh);
	spawnAsteroids(startingAsteroidCount);
	uiSystem.init("..\\res\\Pixel.ttf", 64);
}

void MainGame::initPlayer(Mesh *playerMesh) {
	playerTransform = Transform(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), (glm::vec3(0.2f, 0.2f, 0.2f)));
	player = new GameObject(playerMesh, &playerTransform, ShaderManager::getInstance().getShader("ADS").get());

	player->collisionRadius = 0.8f;
	player->isActive = true;
	
	player->type = GameObjectType::Player;
}

float MainGame::getRefreshRate() {
	SDL_DisplayMode mode;
	if (SDL_GetCurrentDisplayMode(0, &mode) == 0 && mode.refresh_rate > 0) {
		float rate = static_cast<float>(mode.refresh_rate);
		std::cout << "[DEBUG] Detected Refresh Rate: " << rate << " Hz" << std::endl;
		return rate;
	}
	std::cout << "[DEBUG] Could not detect refresh rate. Defaulting to 60 Hz." << std::endl;
	return 60.0f; // Fallback if query fails or returns 0
}

void MainGame::calculateDeltaTime() {
	float currentFrameTime = SDL_GetTicks() / 1000.0f; // Get current time in seconds
	deltaTime = currentFrameTime - lastFrameTime;
	lastFrameTime = currentFrameTime;

	// Prevent huge jumps
	if (deltaTime < 0.0f) {
		deltaTime = 0.0f;
	}
	if (deltaTime > 0.05f) {
		deltaTime = 0.05f; // cap at 50 ms
	}
}

void MainGame::loadPhysicsEngine() {
	if (!DLLManager::getInstance().loadDLL("PhysicsEngine.dll")) {
		std::cerr << "Failed to load PhysicsEngine.dll" << std::endl;
		return;
	}

	// Retrieve and call HelloWorld function from DLL (test linkage)
	typedef void (*HelloWorldFunc)();
	HelloWorldFunc HelloWorld = DLLManager::getInstance().getFunction<HelloWorldFunc>("PhysicsEngine.dll", "HelloWorld");

	if (HelloWorld) {
		HelloWorld(); // Call function from DLL
	}
	else {
		std::cerr << "Failed to retrieve HelloWorld function!" << std::endl;
	}

	// Retrieve physics functions using the template method
	setForwardDirection = DLLManager::getInstance().getFunction<void(*)(GameObject*, glm::vec3)>("PhysicsEngine.dll", "setForwardDirection");
	applyThrust = DLLManager::getInstance().getFunction<void(*)(GameObject*, float)>("PhysicsEngine.dll", "applyThrust");
	updatePhysics = DLLManager::getInstance().getFunction<void(*)(GameObject*, float)>("PhysicsEngine.dll", "updatePhysics");

	if (!setForwardDirection || !applyThrust || !updatePhysics) {
		std::cerr << "Failed to retrieve physics functions!" << std::endl;
	}
}


void MainGame::loadPhysicsEngineUnsafe() {
	DLLManager::getInstance().loadDLL("PhysicsEngine.dll"); // No check if successful

	using HelloWorldFunc = void(*)(); // functions stores, even if only local. 
	HelloWorldFunc HelloWorld = DLLManager::getInstance().getFunction<HelloWorldFunc>("PhysicsEngine.dll", "HelloWorld");

	(*HelloWorld)();  // Explicit dereferencing
	HelloWorld();  // Calls function directly with impicit derefernecing (no nullptr check)
}

// 🔹 Loads Meshes
void MainGame::loadMeshes() {
	playerMesh.loadModel("..\\res\\spaceship.obj");
	asteroidMesh.loadModel("..\\res\\Rock.obj");
	bulletMesh.loadModel("..\\res\\Ball.obj");
	particleMesh.loadModel("..\\res\\Ball.obj");
}

// 🔹 Loads Textures
void MainGame::loadTextures() {
	playerTexture.init("..\\res\\spaceshiptextura.png");
	texture.init("..\\res\\bricks.jpg");
}

// 🔹 Sets up UBOs for matrix data
void MainGame::setupUBOs() {
	UBOManager::getInstance().createUBO("Matrices", sizeof(glm::mat4) * 3, 0);

	// Initialize with identity matrices to avoid garbage data
	glm::mat4 identity = glm::mat4(1.0f);
	UBOManager::getInstance().updateUBOData("Matrices", 0, glm::value_ptr(identity), sizeof(glm::mat4));
	UBOManager::getInstance().updateUBOData("Matrices", sizeof(glm::mat4), glm::value_ptr(identity), sizeof(glm::mat4));
	UBOManager::getInstance().updateUBOData("Matrices", sizeof(glm::mat4) * 2, glm::value_ptr(identity), sizeof(glm::mat4));
}

// 🔹 Loads and sets up shaders
void MainGame::loadShaders() {
	ShaderManager::getInstance().loadShader("ADS", "..\\res\\ADS.vert", "..\\res\\ADS.frag");
	ShaderManager::getInstance().loadShader("UI", "..\\res\\UI.vert", "..\\res\\UI.frag");

	setActiveShader("ADS");

	// Bind UBO to Shader
	UBOManager::getInstance().bindUBOToShader("Matrices", ShaderManager::getInstance().getShader("ADS")->ID(), "Matrices");
}

// 🔹 Sets up Camera
void MainGame::setupCamera() {
	myCamera.initCamera(glm::vec3(0, 0, -25), 70.0f,
		(float)_gameDisplay.getWidth() / _gameDisplay.getHeight(), 0.01f, 1000.0f);
}

void MainGame::gameLoop() {
	while (_gameState != GameState::EXIT) 
	{
		calculateDeltaTime();
		processInput();

		if (!isGameOver)
		{
			handleMovementInput();

			if (player) {
				updatePhysics(player, deltaTime);
				wrapPlayer();
			}

			updateAsteroids(deltaTime);
			updateBullets(deltaTime);
			particleSystem.update(deltaTime);
			bulletTimer -= deltaTime;

			checkBulletAsteroidCollisions();
			checkPlayerAsteroidCollisions();

			checkWaveComplete();

			if (waitingForNextWave)
			{
				waveTimer -= deltaTime;

				if (waveTimer <= 0.0f)
				{
					startNextWave();
				}
			}
		}
		else
		{
			handleGameOverInput();
		}

		uiSystem.update(score, lives, currentWave, isGameOver);

		drawGame();
	}
}


void MainGame::processInput()
{
	SDL_Event evnt;

	while (SDL_PollEvent(&evnt)) // Get and process events
	{
		switch (evnt.type)
		{
		case SDL_QUIT:
			_gameState = GameState::EXIT;
			break;
		}
	}
}

void MainGame::handleMovementInput()
{
	if (!player) return;
	if (isGameOver) return;

	const Uint8* keystates = SDL_GetKeyboardState(nullptr);

	if (keystates[SDL_SCANCODE_W]) {
		if (applyThrust) {
			applyThrust(player, 20.0f);
		}
	}

	if (keystates[SDL_SCANCODE_S]) {
		if (applyThrust) {
			applyThrust(player, -12.0f);
		}
	}

	glm::vec3 newRotation = *(player->transform->GetRot());
	float turnSpeed = glm::radians(180.0f);

	if (keystates[SDL_SCANCODE_A]) {
		newRotation.z -= turnSpeed * deltaTime;
	}

	if (keystates[SDL_SCANCODE_D]) {
		newRotation.z += turnSpeed * deltaTime;
	}

	if (keystates[SDL_SCANCODE_SPACE] && bulletTimer <= 0.0f) {
		fireBullet();
		bulletTimer = bulletCooldown;
	}

	player->transform->SetRot(newRotation);
}

void MainGame::handleGameOverInput()
{
	const Uint8* keystates = SDL_GetKeyboardState(nullptr);

	if (keystates[SDL_SCANCODE_R])
	{
		resetGame();
	}
}

void MainGame::setActiveShader(const std::string& shaderTag) {
	if (ShaderManager::getInstance().getShader(shaderTag)) {
		activeShaderTag = shaderTag;
		std::cout << "Shader switched to: " << shaderTag << std::endl;
	}
	else {
		std::cerr << "Error: Shader not found - " << shaderTag << std::endl;
	}
}

void MainGame::renderPlayer() {
	if (!player) {
		std::cerr << "Warning: Player pointer is null! Skipping render." << std::endl;
		return;
	}

	// Use the shader that was assigned to the player
	Shader* playerShader = player->shader;
	
	playerTexture.Bind(0);

	if (!playerShader) {
		std::cerr << "Error: Player has no assigned shader!" << std::endl;
		return;
	}

	playerShader->Bind();

	updateModelUBO(player->transform->GetModel());

	player->mesh->draw();
}


void MainGame::clearScreenBuffer()
{
	glClearColor(0.05f, 0.05f, 0.05f, 0.0f); // Set clear colour
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear colour and depth buffer - set colour to colour defined in glClearColor
}

void MainGame::drawGame() {
	clearScreenBuffer();

	updateCameraUBO();

	renderPlayer();
	renderAsteroids();
	renderBullets();
	renderParticles();

	renderUI();

	//renderGameObjects(); // Now handles full rendering
	_gameDisplay.swapBuffers();
}

void MainGame::wrapPlayer()
{
	if (!player || !player->transform) return;

	glm::vec3 pos = *(player->transform->GetPos());

	const float xLimit = 24.0f;
	const float yLimit = 14.0f;

	if (pos.x > xLimit) {
		pos.x = -xLimit;
	}
	else if (pos.x < -xLimit) {
		pos.x = xLimit;
	}

	if (pos.y > yLimit) {
		pos.y = -yLimit;
	}
	else if (pos.y < -yLimit) {
		pos.y = yLimit;
	}

	player->transform->SetPos(pos);
}


void MainGame::spawnAsteroids(int count)
{
	for (int i = 0; i < count; i++)
	{
		std::string name = "asteroid_" + std::to_string(asteroidID++);
		
		glm::vec3 playerPos = *(player->transform->GetPos());

		glm::vec3 pos;
		float minDistanceFromPlayer = 6.0f;

		do
		{
			float x = -16.0f + static_cast<float>(rand()) / RAND_MAX * 32.0f;
			float y = -9.0f + static_cast<float>(rand()) / RAND_MAX * 18.0f;

			pos = glm::vec3(x, y, 0.0f);

		} while (glm::length(pos - playerPos) < minDistanceFromPlayer);
		
		glm::vec3 rot(0.0f, 0.0f, 0.0f);
		glm::vec3 scale(1.5f, 1.5f, 1.5f);

		TransformManager::getInstance().addTransform(
			name,
			Transform(pos, rot, scale)
		);

		Transform* asteroidTransform = &TransformManager::getInstance().getTransform(name);
		Shader* asteroidShader = ShaderManager::getInstance().getShader("ADS").get();

		GameObject asteroid(&asteroidMesh, asteroidTransform, asteroidShader);

		float vx = -2.0f + static_cast<float>(rand()) / RAND_MAX * 4.0f;
		float vy = -2.0f + static_cast<float>(rand()) / RAND_MAX * 4.0f;

		asteroid.transformTag = name;

		asteroid.type = GameObjectType::Asteroid;

		asteroid.velocity = glm::vec3(vx, vy, 0.0f);

		asteroid.asteroidLevel = 2;
		asteroid.collisionRadius = 1.5f;
		asteroid.isActive = true;
		
		asteroids.push_back(asteroid);
	}
}

void MainGame::updateAsteroids(float deltaTime)
{
	const float xLimit = 25.0f;
	const float yLimit = 15.0f;

	for (auto& asteroid : asteroids)
	{
		if (!asteroid.isActive) continue;

		if (!asteroid.transform) continue;

		glm::vec3 pos = *(asteroid.transform->GetPos());
		pos += asteroid.velocity * deltaTime;

		if (pos.x > xLimit) pos.x = -xLimit;
		else if (pos.x < -xLimit) pos.x = xLimit;

		if (pos.y > yLimit) pos.y = -yLimit;
		else if (pos.y < -yLimit) pos.y = yLimit;

		asteroid.transform->SetPos(pos);

		glm::vec3 rot = *(asteroid.transform->GetRot());
		rot.z += glm::radians(30.0f) * deltaTime;
		asteroid.transform->SetRot(rot);
	}
}

void MainGame::renderAsteroids()
{
	for (auto& asteroid : asteroids)
	{
		if (!asteroid.isActive) continue;

		if (!asteroid.mesh || !asteroid.transform || !asteroid.shader) continue;

		asteroid.shader->Bind();
		
		texture.Bind(0);

		updateModelUBO(asteroid.transform->GetModel());

		asteroid.mesh->draw();
	}
}

void MainGame::fireBullet()
{
	if (!player || !player->transform) return;

	glm::vec3 playerPos = *(player->transform->GetPos());
	glm::vec3 playerRot = *(player->transform->GetRot());

	float angle = -playerRot.z + glm::pi<float>();

	glm::vec3 direction;
	direction.x = sin(-angle);
	direction.y = -cos(angle);
	direction.z = 0.0f;

	direction = glm::normalize(direction);

	std::string name = "bullet_" + std::to_string(bulletID++);

	TransformManager::getInstance().addTransform(
		name,
		Transform(playerPos, glm::vec3(0.0f), glm::vec3(0.2f))
	);

	Transform* bulletTransform = &TransformManager::getInstance().getTransform(name);
	Shader* bulletShader = ShaderManager::getInstance().getShader("ADS").get();

	GameObject bullet(&bulletMesh, bulletTransform, bulletShader);

	float bulletSpeed = 15.0f;
	bullet.velocity = direction * bulletSpeed;

	bullet.collisionRadius = 0.2f;
	bullet.isActive = true;

	bullet.transformTag = name;

	bullet.type = GameObjectType::Bullet;

	bullets.push_back(bullet);
}

void MainGame::updateBullets(float deltaTime)
{
	const float xLimit = 24.0f;
	const float yLimit = 14.0f;

	for (auto& bullet : bullets)
	{
		if (!bullet.isActive) continue;

		if (!bullet.transform) continue;

		glm::vec3 pos = *(bullet.transform->GetPos());
		pos += bullet.velocity * deltaTime;

		bullet.transform->SetPos(pos);
	}

	bullets.erase(
		std::remove_if(bullets.begin(), bullets.end(),
			[xLimit, yLimit](GameObject& bullet)
			{
				if (!bullet.transform) return true;

				glm::vec3 pos = *(bullet.transform->GetPos());

				return pos.x > xLimit || pos.x < -xLimit ||
					pos.y > yLimit || pos.y < -yLimit;
			}),
		bullets.end()
	);
}

void MainGame::renderBullets()
{
	for (auto& bullet : bullets)
	{
		if (!bullet.isActive) continue;

		if (!bullet.mesh || !bullet.transform || !bullet.shader) continue;

		bullet.shader->Bind();
		
		texture.Bind(0);

		updateModelUBO(bullet.transform->GetModel());

		bullet.mesh->draw();
	}
}

void MainGame::checkBulletAsteroidCollisions()
{
	std::vector<GameObject> newAsteroids;

	for (size_t bulletIndex = 0; bulletIndex < bullets.size(); bulletIndex++)
	{
		if (!bullets[bulletIndex].isActive) continue;

		for (size_t asteroidIndex = 0; asteroidIndex < asteroids.size(); asteroidIndex++)
		{
			if (!asteroids[asteroidIndex].isActive) continue;

			if (CollisionSystem::checkFullCollision(bullets[bulletIndex], asteroids[asteroidIndex])) 
			{
				handleBulletAsteroidCollision(bullets[bulletIndex], asteroids[asteroidIndex], newAsteroids);
				break;
			}
		}
	}

	removeInactiveBullets();

	removeInactiveAsteroids();

	asteroids.insert(asteroids.end(), newAsteroids.begin(), newAsteroids.end());
}

void MainGame::checkPlayerAsteroidCollisions()
{
	if (!player || isGameOver) return;

	for (auto& asteroid : asteroids)
	{
		if (!asteroid.isActive) continue;

		if (CollisionSystem::checkFullCollision(*player, asteroid)) 
		{
			handlePlayerAsteroidCollision(asteroid);
			break;
		}
	}

	asteroids.erase(
		std::remove_if(asteroids.begin(), asteroids.end(),
			[](GameObject& asteroid)
			{
				return !asteroid.isActive;
			}),
		asteroids.end()
	);
}

void MainGame::checkWaveComplete()
{
	if (waitingForNextWave) return;

	if (asteroids.empty())
	{
		waitingForNextWave = true;
		waveTimer = waveSpawnDelay;

		std::cout << "Wave cleared!" << std::endl;
	}
}

void MainGame::startNextWave()
{
	currentWave++;

	bullets.clear();

	int asteroidCount = startingAsteroidCount + currentWave - 1;

	std::cout << "Starting wave " << currentWave
		<< " with " << asteroidCount << " asteroids." << std::endl;

	spawnAsteroids(asteroidCount);

	waitingForNextWave = false;
}

void MainGame::resetGame()
{
	score = 0;
	lives = 3;
	isGameOver = false;

	player->velocity = glm::vec3(0.0f);
	player->acceleration = glm::vec3(0.0f);
	
	currentWave = 1;
	waitingForNextWave = false;
	waveTimer = 0.0f;

	for (auto& bullet : bullets)
	{
		if (!bullet.transformTag.empty())
		{
			TransformManager::getInstance().removeTransform(bullet.transformTag);
		}
	}

	for (auto& asteroid : asteroids)
	{
		if (!asteroid.transformTag.empty())
		{
			TransformManager::getInstance().removeTransform(asteroid.transformTag);
		}
	}

	bullets.clear();
	asteroids.clear();

	if (player && player->transform)
	{
		player->transform->SetPos(glm::vec3(0.0f, 0.0f, 0.0f));
		player->transform->SetRot(glm::vec3(0.0f, 0.0f, 0.0f));
		player->velocity = glm::vec3(0.0f);
	}

	spawnAsteroids(startingAsteroidCount);

	std::cout << "Game reset." << std::endl;
}

void MainGame::handleBulletAsteroidCollision(GameObject& bullet, GameObject& asteroid, std::vector<GameObject>& newAsteroids)
{
	bullet.isActive = false;

	int oldLevel = asteroid.asteroidLevel;
	glm::vec3 oldPosition = *(asteroid.transform->GetPos());


	int particleAmount = 15;

	asteroid.isActive = false;

	if (oldLevel == 2)
	{
		score += 100;
		particleAmount = 40;
	}
	else if (oldLevel == 1)
	{
		score += 200;
		particleAmount = 25;
	}
	else
	{
		score += 300;
		particleAmount = 15;
	}

	particleSystem.spawnExplosion(oldPosition, particleAmount);

	if (oldLevel > 0)
	{
		int newLevel = oldLevel - 1;

		for (int i = 0; i < 2; i++)
		{
			std::string name = "asteroid_" + std::to_string(asteroidID++);

			float scaleValue = 1.0f;
			float radius = 1.0f;

			if (newLevel == 1)
			{
				scaleValue = 0.8f;
				radius = 0.8f;
			}
			else
			{
				scaleValue = 0.45f;
				radius = 0.45f;
			}

			glm::vec3 offset = (i == 0)
				? glm::vec3(0.7f, 0.7f, 0.0f)
				: glm::vec3(-0.7f, -0.7f, 0.0f);

			TransformManager::getInstance().addTransform(
				name,
				Transform(oldPosition + offset, glm::vec3(0.0f), glm::vec3(scaleValue))
			);

			Transform* newTransform = &TransformManager::getInstance().getTransform(name);
			Shader* shader = ShaderManager::getInstance().getShader("ADS").get();

			GameObject splitAsteroid(&asteroidMesh, newTransform, shader);

			float angle = static_cast<float>(rand()) / RAND_MAX * glm::two_pi<float>();
			float speed = 2.0f + static_cast<float>(rand()) / RAND_MAX * 2.0f;

			splitAsteroid.velocity = glm::vec3(
				cos(angle) * speed,
				sin(angle) * speed,
				0.0f
			);

			splitAsteroid.transformTag = name;
			
			splitAsteroid.type = GameObjectType::Asteroid;
			
			splitAsteroid.asteroidLevel = newLevel;
			splitAsteroid.collisionRadius = radius;
			splitAsteroid.isActive = true;

			newAsteroids.push_back(splitAsteroid);
		}
	}	
}

void MainGame::handlePlayerAsteroidCollision(GameObject& asteroid)
{
	asteroid.isActive = false;

	damagePlayer();
}

void MainGame::damagePlayer()
{
	lives--;

	if (player && player->transform)
	{
		player->transform->SetPos(glm::vec3(0.0f, 0.0f, 0.0f));
		player->transform->SetRot(glm::vec3(0.0f, 0.0f, 0.0f));
		player->velocity = glm::vec3(0.0f);
		player->acceleration = glm::vec3(0.0f);
	}

	if (lives <= 0)
	{
		isGameOver = true;
	}
}

void MainGame::updateCameraUBO()
{
	glm::mat4 view = myCamera.getView();
	glm::mat4 projection = myCamera.getProjection();

	UBOManager::getInstance().updateUBOData(
		"Matrices",
		VIEW_MATRIX_OFFSET,
		glm::value_ptr(view),
		sizeof(glm::mat4)
	);

	UBOManager::getInstance().updateUBOData(
		"Matrices",
		PROJECTION_MATRIX_OFFSET,
		glm::value_ptr(projection),
		sizeof(glm::mat4)
	);
}

void MainGame::updateModelUBO(const glm::mat4& model)
{
	UBOManager::getInstance().updateUBOData(
		"Matrices",
		MODEL_MATRIX_OFFSET,
		glm::value_ptr(model),
		sizeof(glm::mat4)
	);
}

void MainGame::removeInactiveBullets()
{
	bullets.erase(
		std::remove_if(bullets.begin(), bullets.end(),
			[](GameObject& bullet)
			{
				if (!bullet.isActive)
				{
					if (!bullet.transformTag.empty())
					{
						TransformManager::getInstance().removeTransform(bullet.transformTag);
					}

					return true;
				}

				return false;
			}),
		bullets.end()
	);
}

void MainGame::removeInactiveAsteroids()
{
	asteroids.erase(
		std::remove_if(asteroids.begin(), asteroids.end(),
			[](GameObject& asteroid)
			{
				if (!asteroid.isActive)
				{
					if (!asteroid.transformTag.empty())
					{
						TransformManager::getInstance().removeTransform(asteroid.transformTag);
					}

					return true;
				}

				return false;
			}),
		asteroids.end()
	);
}

void MainGame::renderTextQuad(float x, float y, float width, float height)
{
	if (uiSystem.getTextTexture() == 0)
	{
		return;
	}

	float vertices[] =
	{
		// position        // uv
		x,         y,          0.0f, 0.0f,
		x + width, y,          1.0f, 0.0f,
		x + width, y - height, 1.0f, 1.0f,

		x,         y,          0.0f, 0.0f,
		x + width, y - height, 1.0f, 1.0f,
		x,         y - height, 0.0f, 1.0f
	};

	GLuint vao;
	GLuint vbo;

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, uiSystem.getTextTexture());

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,
		2,
		GL_FLOAT,
		GL_FALSE,
		4 * sizeof(float),
		(void*)0
	);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,
		2,
		GL_FLOAT,
		GL_FALSE,
		4 * sizeof(float),
		(void*)(2 * sizeof(float))
	);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisable(GL_BLEND);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glBindVertexArray(0);

	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}

void MainGame::renderUI()
{
	GLuint uiTextTexture = uiSystem.getTextTexture();
	int uiTextWidth = uiSystem.getTextWidth();
	int uiTextHeight = uiSystem.getTextHeight();

	if (uiTextTexture == 0 || uiTextWidth <= 0 || uiTextHeight <= 0)
	{
		return;
	}

	Shader* uiShader = ShaderManager::getInstance().getShader("UI").get();

	if (!uiShader)
	{
		return;
	}

	uiShader->Bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, uiTextTexture);

	GLint texLocation = glGetUniformLocation(uiShader->ID(), "uiTexture");
	glUniform1i(texLocation, 0);

	float textureAspect = static_cast<float>(uiTextWidth) / static_cast<float>(uiTextHeight);

	// Use the aspect value that worked for your project
	float screenAspect = 1980.0f / 480.0f;


	if (isGameOver)
	{
		float height = 0.35f;
		float width = height * screenAspect * 0.5f;

		renderTextQuad(-1 * 0.5f, 0.25f, width, height);
	}
	else
	{
		float height = 0.08f;
		float width = height * screenAspect * 1.2;

		renderTextQuad(-0.95f, 0.90f, width, height);
	}
}

void MainGame::renderParticles()
{
	Shader* shader = ShaderManager::getInstance().getShader("ADS").get();

	if (!shader) return;

	shader->Bind();

	for (auto& particle : particleSystem.getParticles())
	{
		float lifeRatio = particle.lifetime / particle.maxLifetime;
		float size = particle.size * lifeRatio;

		Transform particleTransform(
			particle.position,
			glm::vec3(0.0f),
			glm::vec3(size)
		);

		updateModelUBO(particleTransform.GetModel());

		particleMesh.draw();
	}
}