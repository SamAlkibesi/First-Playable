/*
* Name: Ezekiel Evangelista, Cameron Lochray
* Date: April 2, 2023
*/

// Core Libraries
#include <crtdbg.h>
#include <iostream>
#include <Windows.h>
#include <SDL_Image.h>
#include <SDL.h> //allows us to use SDL library
#include <vector> //to make a container for the projectiles
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <string>

/*
* Use SDL to open a window and render some sprites at given locations and scales
*/

//global variables
constexpr float FPS = 60.0f;
constexpr float DELAY_TIME = 1000.0f / FPS; //target deltaTime in ms
const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 600;
float deltaTime = 1 / FPS; //time passing between frames in seconds

SDL_Window* pWindow = nullptr; //pointer to SDL_Window. It stores a menory location which we can use later.
SDL_Renderer* pRenderer = nullptr;
bool isGameRunning = true;
bool loseGame = false;

float enemySpawnDelay = 1.0f;
float enemySpawnTimer = 0.0f;
float enemyStartSpawnTimer = 1.0f;

namespace Fund
{
	//so getPosition() can return 2 values
	struct Vec2
	{
		float x = 0;
		float y = 0;
	};


	//declaring a struct or class declares a new type of object we can make. After this is declared, we can make Sprite variables that have all of the contained data fields, and functions
	struct Sprite
	{
	private:
		// public fields can be accessed from outside the struct or class
		SDL_Texture* pTexture;
		SDL_Rect src;
		SDL_Rect dst;

	public:

		double rotation = 0; //in degrees
		SDL_RendererFlip flipState = SDL_FLIP_NONE;
		Vec2 position; //where sprite is on screen

		//This is a constructor. this is a special type of function used when creating an object
		//The compiler knows it's a constructor because it has parentheses like a function, has the SAME NAME as the struct or class, and has no return. This one has no arguments. In that case, it's called the default constructor and is used to set default values.
		Sprite()
		{
			//std::cout << "Sprite default constructor\n";
			pTexture = nullptr;
			src = SDL_Rect{ 0,0,0,0 };
			dst = SDL_Rect{ 0,0,0,0 };
		}
		//UI Sprite From Text
		Sprite(TTF_Font* font, const char* text, SDL_Color color) : Sprite()
		{
			SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
			pTexture = SDL_CreateTextureFromSurface(pRenderer, surface);
			SDL_FreeSurface(surface);
			TTF_SizeText(font, text, &dst.w, &dst.h);
			src.w = dst.w;
			src.h = dst.h;
			
		}

		//constructors can have arguments as well, which is handy when we need to make them different
		Sprite(SDL_Renderer* renderer, const char* filePathToLoad)
		{
			//std::cout << "Sprite filepath constructor\n";
			src = SDL_Rect{ 0,0,0,0 };

			pTexture = IMG_LoadTexture(renderer, filePathToLoad); //load into our pTexture pointer
			if (pTexture == NULL)
			{
				std::cout << "Image failed to load: " << SDL_GetError() << std::endl;
			}
			SDL_QueryTexture(pTexture, NULL, NULL, &src.w, &src.h); //ask for the dimensions of the texture
			dst = SDL_Rect{ 0,0,src.w,src.h };
			//at this point, the width and the height of the texture should be placed at the memory addresses of src.w and src.h
		}

		//getters and setters

		//sets size for width and height
		void setSize(Vec2 sizeWidthHeight)
		{
			dst.w = sizeWidthHeight.x;
			dst.h = sizeWidthHeight.y;
		}

		//sets width and height
		void setSize(int w, int h)
		{
			dst.w = w;
			dst.h = h;
		}

		SDL_Rect GetRect() const //this function does not change anything
		{
			SDL_Rect returnValue = dst;
			returnValue.x = position.x;
			returnValue.y = position.y;
			return dst;
		}

		//return width and height
		Vec2 getSize() const
		{
			Vec2 returnVec = { dst.w,dst.h };
			return returnVec;
		}

		//return x and y position
		Vec2 getPosition()
		{
			Vec2 returnVec = { dst.x,dst.y };
			return returnVec;
		}


		//this draw function can be called on individual varuables of type Fund::Sprite, which will use their own variables to call SDL_RenderCopy. So, we can declare and draw new sprites with two lines:
		//Fund::Sprite myNewSprite = Sprite(pRenderer, "somefile.png");
		//myNewSprite.Draw(pRenderer);
		void Draw(SDL_Renderer* renderer)
		{
			dst.x = position.x;
			dst.y = position.y;
			SDL_RenderCopyEx(renderer, pTexture, &src, &dst, rotation, NULL, flipState);
		}

		void Cleanup()
		{
			SDL_DestroyTexture(pTexture);
			std::cout << SDL_GetError();
		}

	};
	class Blaster
	{
	public:
		Sprite sprite;
		Vec2 velocity;


		//move bullet
		void Update()
		{
			sprite.position.x += velocity.x * deltaTime;
			sprite.position.y += velocity.y * deltaTime;
		}
	};

	// Class to Move and Shoot Projectiles
	class Ship
	{
	private:
		float fireRepeatTimer = 0.0f;

	public:
		Sprite sprite;
		float moveSpeedPx = 500;
		float fireRepeatDelay = 0.5f;
		float shipHealth = 10;

		// Current Version only handles shooting up or down

		void RestShootCoodown()
		{
			fireRepeatTimer = fireRepeatDelay;
		}
		void Shoot(bool towardUp, std::vector<Fund::Blaster>& container, Fund::Vec2 velocity)
		{
			//create a new bullet 
			Fund::Sprite blasterSprite = Fund::Sprite(pRenderer, "../Assets/textures/blasterbolt.png");

			//start blaster at player position
			blasterSprite.position.x = sprite.getPosition().x;
			if (towardUp)
			{
				blasterSprite.position.x += sprite.getSize().x - (sprite.getSize().x / 1.5);
			}
			blasterSprite.position.y = sprite.getPosition().y + (sprite.getSize().y / 2) - (blasterSprite.getSize().y / 2);

			//Set up our blaster class instance
			Blaster blaster;
			blaster.sprite = blasterSprite;
			blaster.velocity = velocity;

			//add blaster to container
			container.push_back(blaster);

			//reset cooldown
			RestShootCoodown();
		}
		void Move(Vec2 input)
		{
			sprite.position.x += input.x * (moveSpeedPx * deltaTime);
			sprite.position.y += input.y * (moveSpeedPx * deltaTime);

		}
		void Update()
		{

			//tick down the time for the shooting cooldown
			fireRepeatTimer -= deltaTime;
		}
		bool CanShoot()
		{
			return(fireRepeatTimer <= 0.0f);
		}
		float GetHealth()
		{
			return(shipHealth);
		}
		void TakeHealth(float h)
		{
			shipHealth = shipHealth - h;
		}
		void SetHealth(float h)
		{
			shipHealth = h;
		}
	};
	//Part of AABB collision detection, returns true in bounds overlap
	bool AreBoundsOverlapping(int minA, int maxA, int minB, int maxB)
	{
		bool isOverlapping = false;
		if (maxA >= minB && maxA <= maxB) // check if max a is inside of B
		{
			isOverlapping = true;
		}
		if (minA <= maxB && minA >= minB) // check if min a is inside of B
		{
			isOverlapping = true;
		}
		return isOverlapping;
	}
	// Check collison between two sprites
	bool AreSpritesOverlapping(const Sprite& A, const Sprite& B)
	{
		// get bounds of each sprite on x and y
		int minAx, maxAx, minBx, maxBx;
		int minAy, maxAy, minBy, maxBy;

		SDL_Rect boundsA = A.GetRect();
		SDL_Rect boundsB = B.GetRect();

		SDL_bool isColliding = SDL_HasIntersection(&boundsA, &boundsB);
		return (bool)isColliding;

	}
}

//create new instances of struct Fund to load textures
Fund::Ship player;

// Enemy to copy
Fund::Sprite enemyOriginal;


Fund::Sprite background;
Fund::Sprite planet;
Fund::Sprite asteroid;
std::vector<Fund::Blaster> playerBlasterContainer; //std::vector is a class which allows dynamic size. This is a dynamic array of Fund::Sprite
std::vector<Fund::Ship> enemyContainer; //Contains Enemy Ships
std::vector<Fund::Blaster> enemyBlasterContainer; //Contains Enemy Projectiles


//audio files
Mix_Chunk* sfxShipHit;
Mix_Chunk* sfxPlayerShoot;
Mix_Music* bgmDefault;
int audioVolumeCurrent = MIX_MAX_VOLUME / 2;

//UI
TTF_Font* uiFont;
TTF_Font* uiLoseFont;
int scoreCurrent = 0;
Fund::Sprite uiSpriteScore;
Fund::Sprite uiSpriteHealth;
Fund::Sprite uiSpriteLose;

float shakeLevel = 0.0f; // betweeen 0 and 1
float shakeMagnitude = 20.0f;
float shakeDecay = 2.0f;
Fund::Vec2 scoreSpriteBasePosiiton = { 50,50 };
Fund::Vec2 healthSpriteBasePosiiton = { 800,50 };
Fund::Vec2 loseSpriteBasePosiiton = { 10,300 };

//Initialize SDL, open the window and set up renderer
bool Init()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		std::cout << "SDL Init failed: " << SDL_GetError() << std::endl;
		return false;
	}
	std::cout << "SDL Init success\n";

	//Create and assign our SDL_Window pointer
	pWindow = SDL_CreateWindow("Milestone 4", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);

	//not necessary to work but help figure out why it doens't work
	if (pWindow == NULL)
	{
		std::cout << "Window creation failed: " << SDL_GetError() << std::endl;
		return false;
	}
	else
	{
		std::cout << "Window creation success\n";
	}

	//Create and assign out SDL_Renderer pointer
	pRenderer = SDL_CreateRenderer(pWindow, -1, 0);

	if (pRenderer == NULL) //If CreateRenderer failed...
	{
		std::cout << "Renderer creation failed: " << SDL_GetError() << std::endl;
		return false;
	}
	else
	{
		std::cout << "Renderer creation success\n";
	}

	int  playbackFrequency = 22050;
	int chunkSize = 1024;
	if (Mix_OpenAudio(playbackFrequency, MIX_DEFAULT_FORMAT, 1, chunkSize) != 0)
	{
		std::cout << "Mix_OpenAudio failed: " << SDL_GetError() << std::endl;
		return false;
	}
	
	if (TTF_Init() != 0)
	{
		std::cout << "TTF Init creation failed: " << SDL_GetError() << std::endl;
		return false;
	}

	return true;
}

Mix_Chunk* LoadSound(const char* filePath)
{
	Mix_Chunk* sound = Mix_LoadWAV(filePath);
	if (sound == NULL)
	{
		std::cout << "Mix_LoadWAV failed to load file: " << filePath << "-" << SDL_GetError() << std::endl;
	}
	return sound;
}

void Load()
{
	//player textures
	char* fileToLoad = "../Assets/textures/fighter.png";

	player.sprite = Fund::Sprite(pRenderer, fileToLoad);

	Fund::Vec2 shipSize = player.sprite.getSize();
	int shipWidth = shipSize.x;
	int shipHeight = shipSize.y;

	//Describe location to paste to on the screen
	player.sprite.setSize(shipWidth, shipHeight);
	player.sprite.position = { (SCREEN_WIDTH / 2) - 50, 500 };

	//background texture
	background = Fund::Sprite(pRenderer, "../Assets/textures/stars.png");
	background.setSize(SCREEN_WIDTH, SCREEN_HEIGHT);

	//planet texture
	planet = Fund::Sprite(pRenderer, "../Assets/textures/ring-planet.png");
	planet.position = { 300, 400 };

	//asteroid textures
	asteroid = Fund::Sprite(pRenderer, "../Assets/textures/asteroid1.png");
	asteroid.setSize(50, 50);
	asteroid.position = { 700, 350 };

	//load audio files
	sfxPlayerShoot = LoadSound("../Assets/audio/blaster.mp3");
	bgmDefault = Mix_LoadMUS("../Assets/audio/bgm.mp3");
	sfxShipHit = LoadSound("../Assets/audio/hit.mp3");

	if (bgmDefault == NULL)
	{
		std::cout << "Mix_LoadMUS failed to load file: " << SDL_GetError() << std::endl;
	}

	//load font
	fileToLoad = "../Assets/fonts/Consolas.ttf";
	uiFont = TTF_OpenFont(fileToLoad, 24);
	if (uiFont == NULL)
	{
		std::cout << "Font failed to load: " << fileToLoad;
	}
	uiLoseFont = TTF_OpenFont(fileToLoad, 70);
	if (uiLoseFont == NULL)
	{
		std::cout << "Font failed to load: " << fileToLoad;
	}

	enemyOriginal = Fund::Sprite(pRenderer, "../Assets/textures/d7_small.png");
}

void Start()
{
	Mix_Volume(-1, audioVolumeCurrent);
	Mix_PlayMusic(bgmDefault, -1);
}

//input variables
bool isUpPressed = false;
bool isDownPressed = false;
bool isLeftPressed = false;
bool isRightPressed = false;
bool isShootPressed = false;

void Restart()
{
	loseGame = false;
	player.SetHealth(10);

	enemyContainer.clear();
	enemyBlasterContainer.clear();
}
void LoseGame()
{
	std::string loseGameString = "You Lose!! Press R to Restart";
	SDL_Color color = { 255,255,255,255 };
	uiSpriteLose = Fund::Sprite(uiLoseFont, loseGameString.c_str(), color);


	uiSpriteLose.position.x = loseSpriteBasePosiiton.x;
	uiSpriteLose.position.y = loseSpriteBasePosiiton.y;

	loseGame = true;
}
void Input()
{
	SDL_Event event; //event data polled each time
	while (SDL_PollEvent(&event)) //polled until all events are handeled
	{
		//decide what to do with this event
		switch (event.type)
		{
		case(SDL_KEYDOWN):
		{
			SDL_Scancode key = event.key.keysym.scancode;
			switch (key)
			{
			case(SDL_SCANCODE_W):
			{
				isUpPressed = true;
				break;
			}

			case(SDL_SCANCODE_S):
			{
				isDownPressed = true;
				break;
			}

			case(SDL_SCANCODE_A):
			{
				isLeftPressed = true;
				break;
			}

			case(SDL_SCANCODE_D):
			{
				isRightPressed = true;
				break;
			}

			case(SDL_SCANCODE_SPACE):
			{
				isShootPressed = true;
				break;
			}
			case(SDL_SCANCODE_R):
			{
				Restart();
				break;
			}
			break;
			}
			break;
		}
		case(SDL_KEYUP):
		{
			SDL_Scancode key = event.key.keysym.scancode;
			switch (key)
			{
			case(SDL_SCANCODE_W):
			{
				isUpPressed = false;
				break;
			}

			case(SDL_SCANCODE_S):
			{
				isDownPressed = false;
				break;
			}

			case(SDL_SCANCODE_A):
			{
				isLeftPressed = false;
				break;
			}

			case(SDL_SCANCODE_D):
			{
				isRightPressed = false;
				break;
			}

			case(SDL_SCANCODE_SPACE):
			{
				isShootPressed = false;
				break;
			}
			case(SDL_SCANCODE_EQUALS):
			{
				//increase volume
				audioVolumeCurrent = min(audioVolumeCurrent + 10, MIX_MAX_VOLUME);
				Mix_Volume(-1, audioVolumeCurrent);
				std::cout << "Volume: " << audioVolumeCurrent << std::endl;
				break;
			}
			case(SDL_SCANCODE_MINUS):
			{
				//decrease volume
				audioVolumeCurrent = max(audioVolumeCurrent - 10, 0);
				Mix_Volume(-1, audioVolumeCurrent);
				std::cout << "Volume: " << audioVolumeCurrent << std::endl;
				break;
			}
			}
			break;
		}

		}
	}
}
void AddScore(int toAdd)
{
	scoreCurrent += toAdd;
	shakeLevel = min(1, shakeLevel + 0.5f);

}
void TakeHealth(int damage)
{
	player.TakeHealth(1);
	if (player.GetHealth() <= 0)
	{
		LoseGame();
	}
}
void SpawnEnemy()
{
	//enemy textures
	Fund::Sprite enemy = enemyOriginal;
	
	//Spawning at Random Position
	enemy.position =
	{
		(float)(rand() % (SCREEN_WIDTH - (int)enemy.getSize().x)),
		 (0)
	};
	enemy.flipState = SDL_FLIP_HORIZONTAL;
	enemy.rotation = 270.0;

	Fund::Ship enemy1;
	enemy1.sprite = enemy;

	enemy1.fireRepeatDelay = 1.5;
	enemy1.moveSpeedPx = 50;
	enemy1.shipHealth = 1;
	enemy1.RestShootCoodown();
	//Add to enemy
	enemyContainer.push_back(enemy1);
	//reset timer
	enemySpawnTimer = enemySpawnDelay;
}
//Returns true if sprite is overlapping screen
bool IsOffScreen(Fund::Sprite sprite)
{
	bool isOffLeft = sprite.position.x + sprite.getSize().x < 0;
	bool isOffRight = sprite.position.x > SCREEN_WIDTH;
	bool isOffTop = sprite.position.y + sprite.getSize().y < 0 - sprite.getSize().y;
	bool isOffBottom = sprite.position.y > SCREEN_HEIGHT;
	return (isOffLeft || isOffRight || isOffTop || isOffBottom);
}
void UpdatePlayer()
{
	//moves the sprites
	Fund::Vec2 inputVector;
	if (isUpPressed)
	{
		inputVector.y = -1;
		if (player.sprite.position.y < 0)
		{
			player.sprite.position.y = 0;
		}
	}

	if (isDownPressed)
	{
		inputVector.y = 1;
		const int lowestPointScreen = SCREEN_HEIGHT - player.sprite.getSize().y;
		if (player.sprite.position.y > lowestPointScreen)
		{
			player.sprite.position.y = lowestPointScreen;
		}
	}
	if (isLeftPressed)
	{
		inputVector.x = -1;
		if (player.sprite.position.x < 0)
		{
			player.sprite.position.x = 0;
		}
	}
	if (isRightPressed)
	{
		inputVector.x = 1;
		const int leftmostPointScreen = SCREEN_WIDTH - player.sprite.getSize().x;
		if (player.sprite.position.x > leftmostPointScreen)
		{
			player.sprite.position.x = leftmostPointScreen;
		}
	}

	//if shooting and our shooting is off cooldown
	if (isShootPressed && player.CanShoot())
	{
		bool toUp = true;
		Fund::Vec2 velocity{ 0,-1000 };
		// Pass Blaster Container by referance to add blasts to the container specifically
		player.Shoot(toUp, playerBlasterContainer, velocity);

		//play shooting sound
		Mix_PlayChannel(-1, sfxPlayerShoot, 0);
	}

	player.Move(inputVector);
	player.Update();
	//std::cout << player.sprite.position.y << std::endl;
}
void CollisionDetection()
{
	for (std::vector<Fund::Blaster>::iterator it = enemyBlasterContainer.begin(); it != enemyBlasterContainer.end();)
	{
		Fund::Sprite& enemyBlaster = it->sprite;
		if (Fund::AreSpritesOverlapping(player.sprite, enemyBlaster))
		{
			std::cout << "Player was Hit" << std::endl;
			TakeHealth(1);

			//sound when player gets hit
			Mix_PlayChannel(-1, sfxShipHit, 0);

			//remove this element from the container
			it = enemyBlasterContainer.erase(it);
		}
		if (it != enemyBlasterContainer.end())
		{
			it++;
		}
	}
	for (std::vector<Fund::Blaster>::iterator blasterIterator = playerBlasterContainer.begin(); blasterIterator != playerBlasterContainer.end();)
	{
		for (std::vector<Fund::Ship>::iterator enemyIterator = enemyContainer.begin(); enemyIterator != enemyContainer.end();)
		{
			//Test for collision between player blaster and enemy
			if (Fund::AreSpritesOverlapping(blasterIterator->sprite, enemyIterator->sprite))
			{
				//destroy player projectile
				blasterIterator = playerBlasterContainer.erase(blasterIterator);
				//destroy enemy
				enemyIterator = enemyContainer.erase(enemyIterator);

				AddScore(100);
				//enemy gets hit
				Mix_PlayChannel(-1, sfxShipHit, 0);

				//if we destroy stop comparing
				if (blasterIterator == playerBlasterContainer.end())
				{
					break; // leave for loop
				}
			}
			if (enemyIterator != enemyContainer.end())
			{
				enemyIterator++;
			}
		}
		if (blasterIterator != playerBlasterContainer.end())
		{
			blasterIterator++;
		}
	}
}
void RemoveOffscreenSprites()
{
	for (auto blasterIterator = playerBlasterContainer.begin(); blasterIterator != playerBlasterContainer.end(); blasterIterator++)
	{
		if (IsOffScreen(blasterIterator->sprite))
		{
			blasterIterator->sprite.Cleanup();
			blasterIterator = playerBlasterContainer.erase(blasterIterator);
			if (blasterIterator == playerBlasterContainer.end())
			{
				break;
			}
		}
		
	}
	for (auto enemyBlasterIterator = enemyBlasterContainer.begin(); enemyBlasterIterator != enemyBlasterContainer.end(); enemyBlasterIterator++)
	{
		if (IsOffScreen(enemyBlasterIterator->sprite))
		{
			enemyBlasterIterator->sprite.Cleanup();
			enemyBlasterIterator = enemyBlasterContainer.erase(enemyBlasterIterator);
			if (enemyBlasterIterator == enemyBlasterContainer.end())
			{
				break;
			}
		}

	}
	for (auto enemyIterator = enemyContainer.begin(); enemyIterator != enemyContainer.end(); enemyIterator++)
	{
		if (IsOffScreen(enemyIterator->sprite))
		{
			enemyIterator = enemyContainer.erase(enemyIterator);
			if (enemyIterator == enemyContainer.end())
			{
				break;
			}
		}
	
	}
}
void Update()
{
	if (loseGame == false)
	{
		UpdatePlayer();//moves the sprites
		CollisionDetection(); //Detects collsions
	}
	

	Fund::Vec2 inputVector;


	//Update blasters across the screen
	for (int i = 0; i < playerBlasterContainer.size(); i++)
	{

		playerBlasterContainer[i].Update();
	}
	//Update enemy blasters across the screen
	for (int i = 0; i < enemyBlasterContainer.size(); i++)
	{

		enemyBlasterContainer[i].Update();
	}
	//Update enemy ships
	for (int i = 0; i < enemyContainer.size(); i++)
	{
		//Reference to enemy at index I
		Fund::Ship& enemy = enemyContainer[i];

		enemy.Move({ 0, 1 });
		enemy.Update();
		if (enemy.CanShoot())
		{
			bool towardUp = false;
			Fund::Vec2 velocity = { 0,200 };
			enemy.Shoot(towardUp, enemyBlasterContainer, velocity);
		}
	}
	//Spawn enemies on timer
	if (loseGame == false)
	{
		if (enemySpawnTimer <= 0)
		{
			SpawnEnemy();
		}
		else
		{
			enemySpawnTimer -= deltaTime;
		}
	}

	//moves background
	planet.position.y += 0.7;
	if (planet.position.y >= SCREEN_HEIGHT)
	{
		planet.position.y = -200;
	}
	//moves background
	background.position.y += 0.5;
	if (background.position.y >= SCREEN_HEIGHT)
	{
		background.position.y = -SCREEN_HEIGHT;

	}
	//moves background
	asteroid.position.y += 1;
	if (asteroid.position.y >= SCREEN_HEIGHT)
	{
		asteroid.position.y = -SCREEN_HEIGHT;

	}
	RemoveOffscreenSprites();
}

void Draw()
{
	if (loseGame == false)
	{
		//changes background color
		SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 0);

		// refreshes the frame so ship doesn't smear when it moves
		SDL_RenderClear(pRenderer);
		background.Draw(pRenderer);
		planet.Draw(pRenderer);
		asteroid.Draw(pRenderer);
		player.sprite.Draw(pRenderer);

		//draw all blasters on the screen
		for (int i = 0; i < playerBlasterContainer.size(); i++)
		{
			playerBlasterContainer[i].sprite.Draw(pRenderer);
		}

		//draw enemy blasters
		for (int i = 0; i < enemyBlasterContainer.size(); i++)
		{
			enemyBlasterContainer[i].sprite.Draw(pRenderer);
		}
		//draw all enemies
		for (int i = 0; i < enemyContainer.size(); i++)
		{
			enemyContainer[i].sprite.Draw(pRenderer);
		}

		std::string scoreString = "Score: " + std::to_string(scoreCurrent);
		SDL_Color color = { 255,255,255,255 };

		uiSpriteScore = Fund::Sprite(uiFont, scoreString.c_str(), color);

		std::string healthString = "Player Health: " + std::to_string(player.GetHealth());

		uiSpriteHealth = Fund::Sprite(uiFont, healthString.c_str(), color);

		// RIP takeoff o7
		Fund::Vec2 offset =
		{
			shakeLevel * shakeMagnitude * (float)(rand() % 10000) * 0.0001,
			shakeLevel * shakeMagnitude * (float)(rand() % 10000) * 0.0001
		};

		uiSpriteScore.position.x = scoreSpriteBasePosiiton.x + offset.x;
		uiSpriteScore.position.y = scoreSpriteBasePosiiton.y + offset.y;

		uiSpriteHealth.position.x = healthSpriteBasePosiiton.x + offset.x;
		uiSpriteHealth.position.y = healthSpriteBasePosiiton.y + offset.y;

		uiSpriteScore.Draw(pRenderer);
		uiSpriteHealth.Draw(pRenderer);

		shakeLevel = max(0, shakeLevel - deltaTime * shakeDecay);
		//show the hidden space we were drawing to called the BackBuffer. 
		//For more information why we use this, look up Double Buffering
		SDL_RenderPresent(pRenderer);
	}
	else
	{
		//changes background color
		SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 0);
		// refreshes the frame so ship doesn't smear when it moves
		SDL_RenderClear(pRenderer);
		background.Draw(pRenderer);
		planet.Draw(pRenderer);
		uiSpriteLose.Draw(pRenderer);
		SDL_RenderPresent(pRenderer);
	}


}


void Cleanup()
{
	background.Cleanup();
	player.sprite.Cleanup();
	//iterate through all sprites and call cleanup
	for (auto blaster : playerBlasterContainer)
	{
		blaster.sprite.Cleanup();
	}
	for (auto blaster : enemyBlasterContainer)
	{
		blaster.sprite.Cleanup();
	}
	for (auto enemy : enemyContainer)
	{
		enemy.sprite.Cleanup();
	}

	player.sprite.Cleanup();
	TTF_Quit();
	Mix_FreeChunk(sfxPlayerShoot);
	Mix_FreeMusic(bgmDefault);
	Mix_CloseAudio();
	SDL_DestroyWindow(pWindow);
	SDL_DestroyRenderer(pRenderer);
	SDL_Quit();
}


/**
 * \brief Program Entry Point
 */
int main(int argc, char* args[])
{

	// show and position the application console
	AllocConsole();
	auto console = freopen("CON", "w", stdout);
	const auto window_handle = GetConsoleWindow();
	MoveWindow(window_handle, 100, 700, 800, 200, TRUE);

	// Display Main SDL Window
	isGameRunning = Init();

	Load();

	Start();

	enemySpawnTimer = enemyStartSpawnTimer;

	// Main Game Loop
	while (isGameRunning)
	{
		const auto frame_start = static_cast<float>(SDL_GetTicks());
		//if statement?
		Input();//take player input

		Update();//update game state (presumably based on other conditions and input)

		Draw();//draw to screen to show new game state to player

		//figure out how long we need to wait for the next frame timing
		//current time - time at start of frame = time elapsed during this frame
		 
		if (const float frame_time = static_cast<float>(SDL_GetTicks()) - frame_start;
			frame_time < DELAY_TIME)
		{
			SDL_Delay(static_cast<int>(DELAY_TIME - frame_time));
		}

		// delta time
		deltaTime = (static_cast<float>(SDL_GetTicks()) - frame_start) / 1000.0f;


	}

	return 0;
}
