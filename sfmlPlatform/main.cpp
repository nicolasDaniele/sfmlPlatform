#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include "Hero.h"
#include "Enemy.h"
#include "Rocket.h"

sf::Vector2f viewSize(1024, 768);
sf::VideoMode vm(viewSize.x, viewSize.y);
sf::RenderWindow window(vm, "Platform Game", sf::Style::Default);

sf::Vector2f playerPosition;
bool playermoving = false;

sf::Texture skyTxture;
sf::Sprite skySprite;

//sf::Texture bgTxture;
//sf::Sprite bgSprite;

Hero hero;
std::vector<Enemy*> enemies;
std::vector<Rocket*> rockets;

float currentTime;
float prevTime = 0.0f;
int score = 0;
bool gameover = true;

// Text
sf::Font headingFont;
sf::Text headingText;
sf::Font scoreFont;
sf::Text scoreText;
sf::Text tutorialText;

// Audio
sf::Music bgMusic;
sf::SoundBuffer fireBuffer;
sf::SoundBuffer hitBuffer;
sf::Sound fireSound(fireBuffer);
sf::Sound hitSound(hitBuffer);

// Function prototypes
void spawnEnemy();
void shoot();
bool checkCollision(sf::Sprite sprite1, sf::Sprite sprite2);
void reset();

void init()
{
	skyTxture.loadFromFile("Assets/Sprites/BG/sky.png");
	skySprite.setTexture(skyTxture);
	skySprite.scale(2.5f, 2.5f);

	//bgTxture.loadFromFile("");
	//bgSprite.setTexture(skyTxture);

	hero.init("Assets/Sprites/Hero/idle.png", 11, 2.0f,
		sf::Vector2f(viewSize.x * 0.25f, viewSize.y * 0.5f), 200);

	// Text
	// Title
	headingFont.loadFromFile("Assets/Fonts/horror.ttf");
	headingText.setFont(headingFont);
	headingText.setString("Ninja Frog Killer");
	headingText.setCharacterSize(84);
	headingText.setFillColor(sf::Color::Black);
	sf::FloatRect headingBounds = headingText.getLocalBounds();
	headingText.setOrigin(headingBounds.width / 2, headingBounds.height / 2);
	headingText.setPosition(sf::Vector2f(viewSize.x * 0.5f,
				viewSize.y * 0.10f));
	// Score
	scoreFont.loadFromFile("Assets/Fonts/handmadefont.ttf");
	scoreText.setFont(scoreFont);
	scoreText.setString(" Score: 0");
	scoreText.setCharacterSize(45);
	scoreText.setFillColor(sf::Color::Black);
	sf::FloatRect scoreBounds = scoreText.getLocalBounds();
	scoreText.setOrigin(scoreBounds.width / 2, scoreBounds.height / 2);
	scoreText.setPosition(sf::Vector2f(viewSize.x * 0.5f,
		viewSize.y * 0.10f));
	// Tutorial
	tutorialText.setFont(headingFont);
	tutorialText.setString("Press SpaceBar to shoot and start game\nUp arrow to jump");
	tutorialText.setCharacterSize(35);
	tutorialText.setFillColor(sf::Color::Black);
	sf::FloatRect tutorialBounds = tutorialText.getLocalBounds();
	tutorialText.setOrigin(tutorialBounds.width / 2, tutorialBounds.height / 2);
	tutorialText.setPosition(sf::Vector2f(viewSize.x * 0.5f,
		viewSize.y * 0.30f));

	// Audio
	bgMusic.openFromFile("Assets/Audio/Music/happy.ogg");
	bgMusic.play();
	hitBuffer.loadFromFile("Assets/Audio/SFX/die.ogg");
	fireBuffer.loadFromFile("Assets/Audio/SFX/shoot.ogg");

	srand((int)time(0));
}

void updateInput()
{
	sf::Event event;

	while (window.pollEvent(event))
	{
		if (event.type == sf::Event::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::Up)
			{
				hero.jump(750.0f);
			}
			else if (event.key.code == sf::Keyboard::Space)
			{
				if (gameover)
				{
					gameover = false;
					reset();
				}
				else
				{
					shoot();
				}
			}
		}
		// Close window
		if (event.key.code == sf::Keyboard::Escape || 
			event.type == sf::Event::Closed)
		{
			window.close();
		}
	}
}

void update(float dt)
{
	hero.update(dt);
	currentTime += dt;

	// Spawn enemies
	if (currentTime >=prevTime + 1.125f)
	{
		spawnEnemy();
		prevTime = currentTime;
	}

	// Update enemies
	for (int i = 0; i < enemies.size(); i++)
	{
		Enemy *enemy = enemies[i];
		enemy->update(dt);

		if (enemy->getSprite().getPosition().x < 0)
		{
			enemies.erase(enemies.begin() + i);
			delete(enemy);
			gameover = true;
		}
	}

	// Update rockets
	for (int i = 0; i < rockets.size(); i++)
	{
		Rocket* rocket = rockets[i];
		rocket->update(dt);

		if (rocket->getSprite().getPosition().x > viewSize.x)
		{
			rockets.erase(rockets.begin() + i);
			delete(rocket);
		}
	}

	// Check collision saw-enemy
	for(int i = 0; i < rockets.size(); i++)
	{
		for (int j = 0; j < enemies.size(); j++)
		{
			Rocket* rocket = rockets[i];
			Enemy* enemy = enemies[i];

			if (checkCollision(rocket->getSprite(), enemy->getSprite()))
			{
				hitSound.play();

				score++;
				std::string finalScore = "Score " + std::to_string(score);
				scoreText.setString(finalScore);
				sf::FloatRect scoreBounds = scoreText.getLocalBounds();
				scoreText.setOrigin(scoreBounds.width / 2, scoreBounds. height /2);
				scoreText.setPosition(sf::Vector2f(viewSize.x * 0.5f,
								viewSize.y * 0.10f));

				rockets.erase(rockets.begin() + i);
				enemies.erase(enemies.begin() + j);

				delete(rocket);
				delete(enemy);
			}
		}
	}
}

void draw()
{
	window.draw(skySprite);
	//window.draw(bgSprite);
	window.draw(hero.getSprite());

	if (gameover)
	{
		window.draw(headingText);
		window.draw(tutorialText);
	}
	else
	{
		window.draw(scoreText);
	}

	// Draw enemies
	for (Enemy *enemy : enemies)
	{
		window.draw(enemy->getSprite());
	}

	// Draw rockets
	for (Rocket* rocket : rockets)
	{
		window.draw(rocket->getSprite());
	}
}


int main()
{
	sf::Clock clock;
	window.setFramerateLimit(60);

	init();

	while (window.isOpen())
	{
		updateInput();

		// Update game
		sf::Time dt = clock.restart();
		if (!gameover)
		{
			update(dt.asSeconds());
		}

		// Draw game
		window.clear(sf::Color::Red);
		draw();
		window.display();
	}

	return 0;
}

void spawnEnemy()
{
	int randLoc = rand() % 3;
	sf::Vector2f enemyPos;
	float speed;

	switch (randLoc)
	{
		case 0:
			enemyPos = sf::Vector2f(viewSize.x, viewSize.y * 0.75f);
			speed = -400;
			break;
		case 1:
			enemyPos = sf::Vector2f(viewSize.x, viewSize.y * 0.60f);
			speed = -550;
			break;
		case 2:
			enemyPos = sf::Vector2f(viewSize.x, viewSize.y * 0.40f);
			speed = -650;
			break;
		default: 
			printf("Incorrect value \n");
			return;
	}

	Enemy* enemy = new Enemy();
	enemy->init("Assets/Sprites/Enemy/enemy.png", enemyPos, speed);

	enemies.push_back(enemy);
}

void shoot()
{
	Rocket* rocket = new Rocket();
	rocket->init("Assets/Sprites/Rocket/saw.png",
		hero.getSprite().getPosition(), 400.0f, 250.0f);
	rockets.push_back(rocket);

	fireSound.play();
}

bool checkCollision(sf::Sprite sprite1, sf::Sprite sprite2)
{
	sf::FloatRect shape1 = sprite1.getGlobalBounds();
	sf::FloatRect shape2 = sprite2.getGlobalBounds();

	if (shape1.intersects(shape2))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void reset()
{
	score = 0; 
	currentTime = 0.0f;
	prevTime = 0.0f;
	scoreText.setString("SCore: 0");

	for (Enemy * enemy : enemies)
	{
		delete(enemy);
	}
	for (Rocket* rocket : rockets)
	{
		delete(rocket);
	}

	enemies.clear();
	rockets.clear();
}