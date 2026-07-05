#pragma once
#include <SDL\SDL.h>
#include <GL/glew.h>
#include <vector>
#include <windows.h>  // Required for LoadLibrary & GetProcAddress
#include <string>
#include "ShaderManager.h"
#include "TransformManager.h"
#include "UBOManager.h"
#include "Mesh.h"
#include "Texture.h"
#include "transform.h"
#include "DisplayFacade.h" 
#include "GameObject.h"
#include <SDL_ttf.h>
#include "ParticleSystem.h"
#include "UISystem.h"

enum class GameState{PLAY, EXIT};



class MainGame
{
public:
	MainGame();
	~MainGame();

	void run();

private:

	void setActiveShader(const std::string& shaderTag);
	void initSystems();
	void processInput();
	void gameLoop();
	void drawGame();
	void clearScreenBuffer();
	void calculateDeltaTime();
	void loadMeshes();
	void loadTextures();
	void setupUBOs();
	void loadShaders();
	void setupCamera();
	void renderPlayer();
	void loadPhysicsEngine();
	void loadPhysicsEngineUnsafe();
	void initPlayer(Mesh* playerMesh);
	float getRefreshRate();
	void handleMovementInput();
	void wrapPlayer();
	void spawnAsteroids(int count);
	void updateAsteroids(float deltaTime);
	void renderAsteroids();
	void fireBullet();
	void updateBullets(float deltaTime);
	void renderBullets();
	void checkBulletAsteroidCollisions();
	void checkWaveComplete();
	void startNextWave();
	void checkPlayerAsteroidCollisions();
	void handleGameOverInput();
	void resetGame();
	void handleBulletAsteroidCollision(GameObject& bullet, GameObject& asteroid, std::vector<GameObject>& newAsteroids);
	void handlePlayerAsteroidCollision(GameObject& asteroid);
	void damagePlayer();
	void updateCameraUBO();
	void updateModelUBO(const glm::mat4& model);
	void removeInactiveBullets();
	void removeInactiveAsteroids();

	void renderUI();
	void renderTextQuad(float x, float y, float width, float height);

	void renderParticles();


	DisplayFacade _gameDisplay;
	GameState _gameState;
	Mesh susanna;
	Mesh playerMesh;
	Mesh asteroidMesh;
	Mesh bulletMesh;
	Camera myCamera;
	Texture texture; 
	Texture playerTexture;
	Transform transform;
	Transform playerTransform;
	
	ParticleSystem particleSystem;
	Mesh particleMesh;

	UISystem uiSystem;

	float counter;

	int asteroidID = 0;
	float deltaTime = 0.0f;
	float lastFrameTime = 0.0f;
	float accumulator = 0.0f;
	mutable float fixedTimeStep; // 60 physics updates per second
	float bulletCooldown = 0.2f;
	float bulletTimer = 0.0f;
	int bulletID = 0;

	int currentWave = 1;
	int startingAsteroidCount = 5;

	float waveSpawnDelay = 2.0f;
	float waveTimer = 0.0f;
	bool waitingForNextWave = false;

	int score = 0;
	int lives = 3;
	bool isGameOver = false;

	GameObject* player; // Store a pointer to the player object
	std::vector<GameObject> gameObjects; // Store all game objects (for future automation)
	std::string activeShaderTag; // Track the active shader
	std::vector<GameObject> asteroids;
	std::vector<GameObject> bullets;

	// Function pointers for physics engine functions
	void (*setForwardDirection)(GameObject*, glm::vec3) = nullptr;
	void (*applyThrust)(GameObject*, float) = nullptr;
	void (*updatePhysics)(GameObject*, float) = nullptr;

	// Function pointer for HelloWorld test function
	void (*HelloWorld)() = nullptr;
};
