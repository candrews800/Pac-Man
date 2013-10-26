// Pacman by Clinton Andrews

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "SDL_ttf.h"
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include <time.h>

// Screen consists of grid of 28 x 36 tiles
// Tiles are 20 x 20 pixels
const int SCREEN_WIDTH = 560;
const int SCREEN_HEIGHT = 720;
const int SCREEN_BPP = 32;
const int FRAME_RATE = 60;
const int GRID_X = 28;
const int GRID_Y = 36;

std::ofstream logger("log.txt");

void log( std::string message)
{
	logger << message << std::endl;
	logger.flush();
}
void log( char x)
{
	logger<<x<<std::endl;
}
std::string convertInt(int number)
{
	std::stringstream ss;
	ss<<number;
	return ss.str();
}
std::string convertDouble(double number)
{
	std::stringstream ss;
	ss<<number;
	return ss.str();
}

const int PACMAN_SPEED = 3;
const int GHOST_SPEED = 3;

bool ready_screen = true;
double pacman_speed_modifier = 1;
double ghost_speed_modifier = 1;
double ghost_tunnel_modifier = 1;
int elroy_dots_left_one = 0;
double elroy_modifier_one = 1;
int elroy_dots_left_two = 0;
double elroy_modifier_two = 1;
double pacman_fright_modifier = 1;
double fright_modifier = 1;
int fright_time = 0;
int number_of_flashes = 0;
bool fright_mode = false;
int bonus_text_time = 0;
int bonus_start_time = 0;
int bonus = 0;
int bonus_x = 0;
int bonus_y = 0;
int ghosts_eaten = 0;
bool pacman_is_dead = false;

TTF_Font *font = NULL;
TTF_Font *bonus_font = NULL;
SDL_Color text_color = {255,255,255};
SDL_Color bonus_text_color = {30,144,255};

SDL_Surface *screen = NULL;
SDL_Surface *player_one_text = NULL;
SDL_Surface *current_score_text = NULL;
SDL_Surface *high_score_text = NULL;
SDL_Surface *highscore_text = NULL;
SDL_Surface *pacman_lives_left = NULL;
SDL_Surface *level_text = NULL;
SDL_Surface *fruit = NULL;
SDL_Surface *ready_text = NULL;
SDL_Surface *bonus_text = NULL;
SDL_Surface *cherry = NULL;
SDL_Surface *strawberry = NULL;
SDL_Surface *peach = NULL;
SDL_Surface *apple = NULL;
SDL_Surface *grapes = NULL;
SDL_Surface *galaxian = NULL;
SDL_Surface *bell = NULL;
SDL_Surface *key = NULL;


Mix_Chunk *waka = NULL;
Mix_Chunk *ate_fruit = NULL;
Mix_Chunk *start_music = NULL;
Mix_Chunk *fright_sound = NULL;
Mix_Chunk *eating_ghost = NULL;
Mix_Chunk *ghost_chase = NULL;
Mix_Chunk *ghost_death = NULL;
Mix_Chunk *pacman_death = NULL;
Mix_Chunk *credit = NULL;
Mix_Chunk *extra_life = NULL;

bool initialize();
SDL_Surface *loadImage(std::string file_path);
void applySurface(int x, int y, SDL_Surface *source, SDL_Surface *destination);
void displayGrid(int time);
void displayPacman(int time);
void displayGhosts(int time);
void loadLevel(int level);
void checkDot();
void checkFruit();
void checkPowerUp();
void checkGhost();
void displayScores();
void displayFruit();
void displayBonus();
void frightMode();
void displayReadyScreen();
void pacmanInit();
void redInit();
void blueInit();
void orangeInit();
void pinkInit();
void setFruit();
bool isScatterMode();
void levelReset();
void setNextLevel();
void newLevel();
void setfrightMode();
void ghostHouse();
void ghostAudio();
void checkPacman();
void gameOver();

class Tile{
public:
	int x, y;
	int w, h;
	bool is_powerup;
	bool is_open;
	bool is_occupied;
	bool has_dot;
	bool has_fruit;
	bool is_tunnel;
	bool up_allowed;
	SDL_Surface *image;
	
	void Tile::load(int a, int b, std::string image_file, bool open, bool power);
	int Tile::centerX();
	int Tile::centerY();
};
void Tile::load(int a, int b, std::string image_file, bool open, bool power)
{
	x = a;
	y = b;
	w = 20;
	h = 20;

	image = loadImage(image_file);

	is_open = open;
	is_powerup = power;
	has_dot = false;
	up_allowed = true;
	is_tunnel = false;
	has_fruit = false;
}
int Tile::centerX()
{
	return x + (w/2);
}
int Tile::centerY()
{
	return y + (h/2);
}
Tile grid[GRID_X][GRID_Y];

class GameObject
{
public:
	double x, y;
	int w, h;
	double xVel;
	double yVel;
	int direction;
	int frame;
	int current_tile_x;
	int current_tile_y;
	int next_tile_x;
	int next_tile_y;
	int ghost_type;
	bool frightened;
	bool is_active;
	bool animate;
	bool is_dead;
	SDL_Surface *image;

	GameObject::GameObject();
	void GameObject::toGrid(int grid_x, int grid_y);
	void GameObject::move();
	void GameObject::ghostMove(int target_x, int target_y);
	void GameObject::moveFromGhostHouse();
	int GameObject::getX();
	int GameObject::getY();
	int GameObject::nextX();
	int GameObject::nextY();
	void GameObject::handleInput(SDL_Event event);
	int GameObject::targetX(bool scatter_mode, int pacman_current_x, int pacman_current_y, int pacman_current_direction, int red_x, int red_y);
	int GameObject::targetY(bool scatter_mode, int pacman_current_x, int pacman_current_y, int pacman_current_direction, int red_x, int red_y);
};
GameObject::GameObject()
{
	h = 32;
	w = 32;
	xVel = -PACMAN_SPEED;
	direction = 4;
	frame = 0;
	animate = false;
	is_dead = false;
}
void GameObject::toGrid(int grid_x, int grid_y)
{
	x = grid[grid_x][grid_y].centerX();
	y = grid[grid_x][grid_y].centerY();
}
void GameObject::move()
{
	// Portal
	if(x-w/2<0)
	{
		x = SCREEN_WIDTH-w/2;
	}
	else if(x+w/2>SCREEN_WIDTH)
	{
		x = w/2;
	}

	// Moving Left
	if(direction == 4)
	{
		if(fright_mode == false)
			x += xVel * pacman_speed_modifier;
		else if(fright_mode)
			x += xVel * pacman_fright_modifier;
		if(grid[getX()-1][getY()].is_open == false && x < grid[getX()][getY()].centerX())
		{
			x = grid[getX()][getY()].centerX();
			xVel = 0;
		}

		if(yVel > 0 && grid[getX()][getY()+1].is_open == true)
		{
			x = grid[getX()][getY()].centerX();
			xVel = 0;
			direction = 3;
		}

		else if(yVel < 0 && grid[getX()][getY()-1].is_open == true)
		{
			x = grid[getX()][getY()].centerX();
			xVel = 0;
			direction = 1;
		}

		else if(xVel > 0)
		{
			direction = 2;
		}
	}

	// Moving Right
	else if(direction == 2)
	{
		if(fright_mode == false)
			x += xVel * pacman_speed_modifier;
		else if(fright_mode)
			x += xVel * pacman_fright_modifier;
		if(grid[getX()+1][getY()].is_open == false && x > grid[getX()][getY()].centerX())
		{
			x = grid[getX()][getY()].centerX();
			xVel = 0;
		}

		if(yVel > 0 && grid[getX()][getY()+1].is_open == true)
		{
			x = grid[getX()][getY()].centerX();
			xVel = 0;
			direction = 3;
		}

		else if(yVel < 0 && grid[getX()][getY()-1].is_open == true)
		{
			x = grid[getX()][getY()].centerX();
			xVel = 0;
			direction = 1;
		}

		else if(xVel < 0)
		{
			direction = 4;
		}
	}

	// Moving Up
	else if(direction == 1)
	{
		if(fright_mode == false)
			y += yVel * pacman_speed_modifier;
		else if(fright_mode)
			y += yVel * pacman_fright_modifier;
		if(grid[getX()][getY()-1].is_open == false && y < grid[getX()][getY()].centerY())
		{
			y = grid[getX()][getY()].centerY();
			yVel = 0;
		}

		if(xVel > 0 && grid[getX()+1][getY()].is_open == true)
		{
			y = grid[getX()][getY()].centerY();
			yVel = 0;
			direction = 2;
		}

		else if(xVel < 0 && grid[getX()-1][getY()].is_open == true)
		{
			y = grid[getX()][getY()].centerY();
			yVel = 0;
			direction = 4;
		}

		else if(yVel > 0)
		{
			direction = 3;
		}
	}

	// Moving Down
	else if(direction == 3)
	{
		if(fright_mode == false)
			y += yVel * pacman_speed_modifier;
		else if(fright_mode)
			y += yVel * pacman_fright_modifier;
		if(grid[getX()][getY()+1].is_open == false && y > grid[getX()][getY()].centerY())
		{
			y = grid[getX()][getY()].centerY();
			yVel = 0;
		}

		if(xVel > 0 && grid[getX()+1][getY()].is_open == true)
		{
			y = grid[getX()][getY()].centerY();
			yVel = 0;
			direction = 2;
		}

		else if(xVel < 0 && grid[getX()-1][getY()].is_open == true)
		{
			y = grid[getX()][getY()].centerY();
			yVel = 0;
			direction = 4;
		}

		else if(yVel < 0)
		{
			direction = 1;
		}
	}
}
void GameObject::ghostMove(int target_x, int target_y)
{
	if(target_x < 0)
	{
		target_x = 0;
	}
	else if(target_x>27)
	{
		target_x = 27;
	}
	if(target_y < 0)
	{
		target_y = 0;
	}
	else if(target_y>35)
	{
		target_y=35;
	}

	if(x-w/2<0)
	{
		toGrid(26,17);
		next_tile_x=20;
		next_tile_y=17;
	}
	else if(x+w/2>SCREEN_WIDTH)
	{
		toGrid(1,17);
		next_tile_x=1;
		next_tile_y=17;
	}

	current_tile_x = getX();
	current_tile_y = getY();

	bool ghost_centered = false;

	if(abs(x - grid[next_tile_x][next_tile_y].centerX()) <= GHOST_SPEED && abs(y - grid[next_tile_x][next_tile_y].centerY()) <= GHOST_SPEED)
	{
		x = grid[next_tile_x][next_tile_y].centerX();
		y = grid[next_tile_x][next_tile_y].centerY();

		ghost_centered = true;

		double distance = 100000;
		int i = 0;
		int j = 0;
		switch(direction)
		{
		// Up
		case 1:
			if(grid[current_tile_x][current_tile_y-1].is_open && grid[current_tile_x][current_tile_y].up_allowed)
			{
				distance = sqrt(pow((double)current_tile_x-target_x, 2) + pow((double)(current_tile_y-1)-target_y, 2));
				log(convertDouble(distance).c_str());
				direction = 1;
				next_tile_y = current_tile_y - 1;
				i++;
			}
			if(grid[current_tile_x+1][current_tile_y].is_open)
			{
				double dist = sqrt(pow((double)(current_tile_x+1)-target_x, 2) + pow((double)(current_tile_y)-target_y, 2));
				if(dist<distance)
				{
					distance = dist;
					log(convertDouble(distance).c_str());
					direction = 2;
					next_tile_x = current_tile_x + 1;
									
					j++;
					if(i==1)
					{
						i=0;
						next_tile_y++;
					}
				}
			}

			if(grid[current_tile_x-1][current_tile_y].is_open)
			{
				double dist = sqrt(pow((double)(current_tile_x-1)-target_x, 2) + pow((double)(current_tile_y)-target_y, 2));
				if(dist<distance)
				{
					distance = dist;
					log(convertDouble(distance).c_str());
					direction = 4;
					next_tile_x = current_tile_x - 1;
					if(i==1)
						next_tile_y++;
				}

			}

			break;
		// Right
		case 2:
			if(grid[current_tile_x][current_tile_y-1].is_open && grid[current_tile_x][current_tile_y].up_allowed)
			{
				distance = sqrt(pow((double)current_tile_x-target_x, 2) + pow((double)(current_tile_y-1)-target_y, 2));
				log(convertDouble(distance).c_str());
				direction = 1;
				next_tile_y = current_tile_y - 1;
				i++;
			}

			if(grid[current_tile_x+1][current_tile_y].is_open)
			{
				double dist = sqrt(pow((double)(current_tile_x+1)-target_x, 2) + pow((double)(current_tile_y)-target_y, 2));
				if(dist<distance)
				{
					distance = dist;
					log(convertDouble(distance).c_str());
					direction = 2;
					next_tile_x =  current_tile_x + 1;
					j++;
					if(i==1)
					{
						i=0;
						next_tile_y++;
					}
				}
			}

			if(grid[current_tile_x][current_tile_y+1].is_open)
			{
				double dist = sqrt(pow((double)(current_tile_x)-target_x, 2) + pow((double)(current_tile_y+1)-target_y, 2));
				if(dist<distance)
				{
					distance = dist;
					log(convertDouble(distance).c_str());
					direction = 3;
					next_tile_y = current_tile_y + 1;
					if(j==1)
						next_tile_x--;
				}
			}
			break;
		// Down
		case 3:
			if(grid[current_tile_x][current_tile_y+1].is_open)
			{
				distance = sqrt(pow((double)(current_tile_x-target_x), 2) + pow((double)(current_tile_y+1)-target_y, 2));
				log(convertDouble(distance).c_str());
				direction = 3;
				next_tile_y = current_tile_y + 1;
				i++;
			}

			if(grid[current_tile_x+1][current_tile_y].is_open)
			{
				double dist = sqrt(pow((double)(current_tile_x+1)-target_x, 2) + pow((double)(current_tile_y-target_y), 2));
				if(dist<distance)
				{
					distance = dist;
					log(convertDouble(distance).c_str());
					direction = 2;
					next_tile_x = current_tile_x + 1;
					if(i==1)
					{
						i=0;
						next_tile_y--;
					}
				}
			}

			if(grid[current_tile_x-1][current_tile_y].is_open)
			{
				double dist = sqrt(pow((double)(current_tile_x-1-target_x), 2) + pow((double)(current_tile_y-target_y), 2));
				if(dist<distance)
				{
					distance = dist;
					log(convertDouble(distance).c_str());
					direction = 4;
					next_tile_x = current_tile_x - 1;
					if(i==1)
						next_tile_y--;
				}
			}
			break;
		// Left
		case 4:
			if(grid[current_tile_x][current_tile_y-1].is_open && grid[current_tile_x][current_tile_y].up_allowed)
			{
				distance = sqrt(pow((double)current_tile_x-target_x, 2) + pow((double)(current_tile_y-1)-target_y, 2))+.1;
				log(convertDouble(distance).c_str());
				direction = 1;
				next_tile_y = current_tile_y - 1;
				i++;
			}

			if(grid[current_tile_x][current_tile_y+1].is_open)
			{
				double dist = sqrt(pow((double)(current_tile_x)-target_x, 2) + pow((double)(current_tile_y+1-target_y), 2));
				if(dist<distance)
				{
					distance = dist;
					log(convertDouble(distance).c_str());
					direction = 3;
					next_tile_y = current_tile_y + 1;
					j++;
				}
			}

			if(grid[current_tile_x-1][current_tile_y].is_open)
			{
				double dist = sqrt(pow((double)(current_tile_x-1)-target_x, 2) + pow((double)(current_tile_y)-target_y, 2));
				if(dist<distance)
				{
					distance = dist;
					log(convertDouble(distance).c_str());
					direction = 4;
					next_tile_x = current_tile_x - 1;
					if(i==1 || j==1)
						next_tile_y = current_tile_y;

				}
			}
		}
	}

	if(is_dead == true && x < grid[14][14].centerX() && x > grid[13][14].centerX() && y > grid[14][13].centerY()-2 && y < grid[14][20].centerY() )
	{
		x = grid[14][17].centerX() - 10;
		direction = 3;
		if(y>grid[14][18].centerY()-10)
		{
			toGrid(14,17);
			x -= 10;
			direction = 0;
			frightened = false;
			is_active = false;
			animate = false;
			is_dead = false;
		}		
	}
	if(ghost_centered == false){

		double xSpeed = xVel * ghost_speed_modifier;
		double ySpeed = yVel * ghost_speed_modifier;

		if(grid[current_tile_x][current_tile_y].is_tunnel)
		{
			xSpeed = xVel * ghost_tunnel_modifier;
			ySpeed = yVel * ghost_tunnel_modifier;
		}
		if(frightened && is_active)
		{
			xSpeed = xVel * fright_modifier;
			ySpeed = yVel * fright_modifier;
		}
		if(is_dead)
		{
			xSpeed = 2*xVel;
			ySpeed = 2*yVel;
		}
		switch(direction)
		{
		// Move Up
		case 1:
			y -= ySpeed;
			break;
		// Right
		case 2:
			x += xSpeed;
			break;
		// Down
		case 3:
			y += ySpeed;
			break;
		// Left
		case 4:
			x -= xSpeed;
			break;
		}
	}
}
void GameObject::moveFromGhostHouse()
{
	if(animate == false && y != grid[14][14].centerY()-10)
	{
		if(x < (grid[14][17].centerX() - 10))
		{
			x += 0.5;
			direction = 2;
		}
		else if(x > grid[14][17].centerX() - 10)
		{
			x -= 0.5;
			direction = 4;
		}
		else
		{
			y -= 0.5;
			direction = 1;
		}
	}
	else
	{
		switch(direction)
		{
		case 0:
			direction = 1;
			break;
		case 1:
			y -= 1;
			if(y < 335)
				direction = 3;
			break;
		case 3:
			y += 1;
			if(y > 365)
				direction = 1;
			break;
		}

	}
}
int GameObject::getX()
{
	return x / 20;
}
int GameObject::getY()
{
	return y / 20;
}
int GameObject::nextX()
{
	if(direction==2)
		return getX() + 1;
	else if(direction==4)
		return getX() - 1;
	else 
		return getX();
}
int GameObject::nextY()
{
	if(direction==1)
		return getY() - 1;
	else if(direction==3)
		return getY() + 1;
	else
		return getY();
}
void GameObject::handleInput(SDL_Event event)
{
	// 1 Top, 2 Right, 3 Bottom, 4 Left
	switch(event.type)
	{
	case SDL_KEYDOWN:
		if(event.key.keysym.sym == SDLK_LEFT)
		{
			xVel = -1 * PACMAN_SPEED;
		}
		else if(event.key.keysym.sym == SDLK_RIGHT)
		{
			xVel = PACMAN_SPEED;
		}
		else if(event.key.keysym.sym == SDLK_UP)
		{
			yVel = -1 * PACMAN_SPEED;
		}
		else if(event.key.keysym.sym == SDLK_DOWN)
		{
			yVel = PACMAN_SPEED;
		}
		break;
	}
}
int GameObject::targetX(bool scatter_mode, int pacman_current_x, int pacman_current_y, int pacman_current_direction, int red_x, int red_y)
{
	if(frightened)
		return rand() % 28;
	if(is_dead)
		return 14;
	switch(ghost_type)
	{
	// Red
	case 1:
		if(scatter_mode)
		{
			return 25;
		}
		return pacman_current_x;
		break;
	// Pink
	case 2:
		if(scatter_mode)
		{
			return 2;
		}
		switch(pacman_current_direction)
		{
		case 1:
			return pacman_current_x - 4;
			break;
		case 2:
			return pacman_current_x + 4;
			break;
		case 3:
			return pacman_current_x;
			break;
		case 4:
			return pacman_current_x - 4;
			break;
		}
		break;
	// Blue
	case 3:
		if(scatter_mode)
		{
			return 27;
		}
		return 2*pacman_current_x - red_x;
		break;
	// Orange
	case 4:
		if(scatter_mode)
		{
			return 0;
		}
		double distance = sqrt(pow((double)getX() - pacman_current_x, 2) + pow((double)getY() - pacman_current_y, 2));
		if(distance > 8)
			return pacman_current_x;
		else
			return 0;
		break;
	}
}
int GameObject::targetY(bool scatter_mode, int pacman_current_x, int pacman_current_y, int pacman_current_direction, int red_x, int red_y)
{
	if(frightened)
		return rand() % 36;
	if(is_dead)
		return 14;
	switch(ghost_type)
	{
	// Red
	case 1:
		if(scatter_mode)
		{
			return 0;
		}
		return pacman_current_y;

		break;
	// Pink
	case 2:
		if(scatter_mode)
		{
			return 0;
		}
		switch(pacman_current_direction)
		{
		case 1:
			return pacman_current_y - 4;
			break;
		case 2:
			return pacman_current_y;
			break;
		case 3:
			return pacman_current_y + 4;
			break;
		case 4:
			return pacman_current_y;
			break;
		}
		break;
	// Blue
	case 3:
		if(scatter_mode)
		{
			return 34;
		}
		return 2*pacman_current_y - red_y;

		break;
	// Orange
	case 4:
		if(scatter_mode)
		{
			return 34;
		}
		double distance = sqrt(pow((double)getX() - pacman_current_x, 2) + pow((double)getY() - pacman_current_y, 2));
		if(distance > 8)
			return pacman_current_y;
		else
			return 34;
		break;
	}
}
GameObject pacman;
GameObject red;
GameObject pink;
GameObject blue;
GameObject alien;
GameObject orange;

int score = 0;
int lives_left = 2;
int level = 1;
int dots = 0;
int bonus_points = 0;
int lastPellet = 0;
int fruitTimer = 0;
bool active_fruit = false;
bool first_fruit = false;
bool second_fruit = false;
bool waka_sound_is_on = false;
int game_time = 0;
int delay_time = 0;
int delay_start_time = 0;
int fright_start_time = 0;
bool new_game = true;
bool bonus_life_remaining = true;

int main(int argc, char* args[])
{    
	bool quit = false;

	// Initialize SDL
	if(initialize() == false)
	{
		return 1;
	}

	long int last_tick = SDL_GetTicks();

	// Game Loop
	while (quit == false)
	{
		// User Input
		// Check if User quits
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_QUIT:
					quit = true;
					break;
				default:
					pacman.handleInput(event);
					break;
			}

		}


		// Game Logic
		if(lives_left < 0)
		{
			new_game = true;
			delay_time = 5000;
		}
		if(score > 5000 && bonus_life_remaining)
		{
			lives_left++;
			Mix_PlayChannel(0,extra_life,0);
			bonus_life_remaining = false;
		}

		if(delay_start_time + delay_time < SDL_GetTicks())
		{
			ready_screen = false;
			Mix_Resume(0);
			Mix_Resume(1);
			Mix_Resume(2);
			Mix_Resume(3);
			Mix_Resume(5);
			delay_time = 0;
			game_time = SDL_GetTicks();
			setNextLevel();
			checkDot();
			checkPowerUp();
			checkFruit();
			checkGhost();
			checkPacman();
			setFruit();
			frightMode();
			pacman.move();
			ghostHouse();
			ghostAudio();
			gameOver();
			if(pink.is_active)
				pink.ghostMove(pink.targetX(isScatterMode(),pacman.getX(),pacman.getY(),pacman.direction, red.getX(), red.getY()),pink.targetY(isScatterMode(),pacman.getX(),pacman.getY(),pacman.direction, red.getX(), red.getY()));
			if(red.is_active)
				red.ghostMove(red.targetX(isScatterMode(),pacman.getX(),pacman.getY(),pacman.direction, red.getX(), red.getY()),red.targetY(isScatterMode(),pacman.getX(),pacman.getY(),pacman.direction, red.getX(), red.getY()));
			if(orange.is_active)
				orange.ghostMove(orange.targetX(isScatterMode(),pacman.getX(),pacman.getY(),pacman.direction, red.getX(), red.getY()),orange.targetY(isScatterMode(),pacman.getX(),pacman.getY(),pacman.direction, red.getX(), red.getY()));
			if(blue.is_active)
				blue.ghostMove(blue.targetX(isScatterMode(),pacman.getX(),pacman.getY(),pacman.direction, red.getX(), red.getY()),blue.targetY(isScatterMode(),pacman.getX(),pacman.getY(),pacman.direction, red.getX(), red.getY()));
		}

		// Display
		displayGrid(SDL_GetTicks());
		displayScores();
		displayGhosts(SDL_GetTicks());
		displayPacman(SDL_GetTicks());
		displayFruit();
		displayReadyScreen();
		displayBonus();
		SDL_Flip(screen);

		// Cap Frame Rate
		long int delta = SDL_GetTicks() - last_tick;
		if( delta < 1000 / FRAME_RATE)
		{
			SDL_Delay( (1000/FRAME_RATE) - delta);
		}
		last_tick = SDL_GetTicks();
	}

	// Quit SDL
	SDL_Quit();
	return 0;  
}

bool initialize(){
	srand ( time(NULL) );

	// Start all SDL Systems
	if(SDL_Init(SDL_INIT_EVERYTHING) == -1)
	{
		return false;
	}

	// Initialize the screen
	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE);
	if(screen == NULL)
	{
		return false;
	}

	if( TTF_Init() == -1 )
    {
        return false;    
    }

	// Initialize SDL_mixer
    if( Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 4096 ) == -1 )
    {
        return false;    
    }

	Mix_AllocateChannels(6);

	font = TTF_OpenFont("automati.ttf", 20);
	bonus_font = TTF_OpenFont("automati.ttf", 10);

	waka = Mix_LoadWAV("audio/waka.wav");
	ate_fruit = Mix_LoadWAV("audio/fruit.wav");
	start_music = Mix_LoadWAV("audio/start.wav");
	fright_sound = Mix_LoadWAV("audio/fright.wav");
	eating_ghost = Mix_LoadWAV("audio/eating_ghost.wav");
	ghost_death = Mix_LoadWAV("audio/ghost_return.wav");
	pacman_death = Mix_LoadWAV("audio/pacman_death.wav");
	extra_life = Mix_LoadWAV("audio/extra_life.wav");
	credit = Mix_LoadWAV("audio/credit.wav");

	
	key=loadImage("images/fruit/key.bmp");
	bell=loadImage("images/fruit/bell.bmp");
	galaxian=loadImage("images/fruit/galaxian.bmp");
	grapes=loadImage("images/fruit/grapes.bmp");
	apple=loadImage("images/fruit/apple.bmp");
	peach=loadImage("images/fruit/peach.bmp");
	strawberry=loadImage("images/fruit/strawberry.bmp");
	cherry=loadImage("images/fruit/cherries.bmp");

	// Set Caption
	SDL_WM_SetCaption("Pacman by Clinton Andrews", NULL);
	return true;
}
SDL_Surface *loadImage(std::string file_path)
{
	SDL_Surface *image = NULL;

	image = IMG_Load(file_path.c_str());

	return image;
}
void applySurface(int x, int y, SDL_Surface *source, SDL_Surface *destination)
{
	SDL_Rect coords;
	coords.x = x;
	coords.y = y;

	SDL_BlitSurface(source, NULL, screen, &coords);
}
void displayGrid(int time)
{
	for(int j=0;j<GRID_Y;j++)
	{
		for(int i=0;i<GRID_X;i++)
		{
			if(grid[i][j].is_powerup == true && (time/500) % 3 > 0)
				grid[i][j].image = loadImage("images/powerup.bmp");
			else if(grid[i][j].is_powerup == true && (time/500) % 3 == 0)
			{
				grid[i][j].image = loadImage("images/blank.bmp");
			}
			applySurface(grid[i][j].x, grid[i][j].y, grid[i][j].image, screen);
		}
	}
}
void loadLevel(int level)
{
	// Load Specific Level Variables
	dots = 244;

	int bonus_points_by_level[13] = { 100, 300, 500, 500, 700, 700, 1000, 1000, 2000, 2000, 3000, 3000, 5000 };
	if(level>12)
		bonus_points = bonus_points_by_level[12];
	else
		bonus_points = bonus_points_by_level[level-1];

	if(level==1)
		pacman_speed_modifier=0.8;
	else if(level>20 || level==2 || level==3 || level == 4)
		pacman_speed_modifier=0.9;
	else
		pacman_speed_modifier=1;

	if(level>4)
	{
		ghost_speed_modifier=0.95;
		ghost_tunnel_modifier = 0.5;
		elroy_modifier_one = 1;
		elroy_modifier_two = 1.05;
		pacman_fright_modifier = 1;
		fright_modifier = 0.6;

	}
	else if(level>1)
	{
		ghost_speed_modifier=0.85;
		ghost_tunnel_modifier = 0.45;
		elroy_modifier_one = .9;
		elroy_modifier_two = .95;
		pacman_fright_modifier = 0.95;
		fright_modifier = 0.55;
	}
	else
	{
		ghost_speed_modifier=0.75;
		ghost_tunnel_modifier = 0.4;
		elroy_modifier_one = .8;
		elroy_modifier_two = .85;
		pacman_fright_modifier = 0.9;
		fright_modifier = 0.5;
	}

	if(level>8)
		ghost_chase = Mix_LoadWAV("audio/ghost_5.wav");
	else if(level>5)
		ghost_chase = Mix_LoadWAV("audio/ghost_4.wav");
	else if(level>4)
		ghost_chase = Mix_LoadWAV("audio/ghost_3.wav");
	else if(level>1)
		ghost_chase = Mix_LoadWAV("audio/ghost_2.wav");
	else
		ghost_chase = Mix_LoadWAV("audio/ghost_1.wav");

	int elroy_dots_left_one_by_level[19] = {20, 30, 40, 40, 40, 50, 50, 50, 60, 60, 60, 80, 80, 80, 100, 100, 100, 100, 120};
	int elroy_dots_left_two_by_level[19] = {10, 15, 20, 20, 20, 25, 25, 25, 30, 30, 30, 40, 40, 40, 50, 50, 50, 50, 60};
	int fright_time_by_level[19] = {6, 5, 4, 3, 2, 5, 2, 2, 1, 5, 2, 1, 1, 3, 1, 1, 0, 1, 0};
	int number_of_flashes_by_level[19] = {5, 5, 5, 5, 5, 5, 5, 5, 3, 5, 5, 3, 3, 5, 3, 3, 0, 3, 0};

	if(level>18)
	{
		elroy_dots_left_one = elroy_dots_left_one_by_level[18];
		elroy_dots_left_two = elroy_dots_left_two_by_level[18];
		fright_time = fright_time_by_level[18];
		number_of_flashes = number_of_flashes_by_level[18];
	}
	else
	{
		elroy_dots_left_one = elroy_dots_left_one_by_level[level-1];
		elroy_dots_left_two = elroy_dots_left_two_by_level[level-1];
		fright_time = fright_time_by_level[level-1];
		number_of_flashes = number_of_flashes_by_level[level-1];
	}

	std::string file_name = "levels/1.txt";

	std::ifstream file;
	file.open(file_name.c_str());

	for(int j=0;j<GRID_Y;j++)
	{
		for(int i=0;i<GRID_X;i++)
		{
			char type;
			file>>type;
			switch(type)
			{
			case 'o':
				grid[i][j].load(i*20,j*20,"images/blank.bmp",true,false);
				break;
			case '*':
				grid[i][j].load(i*20,j*20,"images/dot.bmp",true,false);
				grid[i][j].has_dot = true;
				break;
			case '+':
				grid[i][j].load(i*20,j*20,"images/powerup.bmp",true,true);
				break;
			case 'a':
				grid[i][j].load(i*20,j*20,"images/left double vertical.bmp",false,false);
				break;
			case 'd':
				grid[i][j].load(i*20,j*20,"images/right double vertical.bmp",false,false);
				break;
			case 'v':
				grid[i][j].load(i*20,j*20,"images/vertical.bmp",false,false);
				break;
			case 'h':
				grid[i][j].load(i*20,j*20,"images/horizontal.bmp",false,false);
				break;
			case 'w':
				grid[i][j].load(i*20,j*20,"images/top double horizontal.bmp",false,false);
				break;
			case 'x':
				grid[i][j].load(i*20,j*20,"images/bottom double horizontal.bmp",false,false);
				break;
			case '7':
				grid[i][j].load(i*20,j*20,"images/top left.bmp",false,false);
				break;
			case '9':
				grid[i][j].load(i*20,j*20,"images/top right.bmp",false,false);
				break;
			case '1':
				grid[i][j].load(i*20,j*20,"images/bottom left.bmp",false,false);
				break;
			case '3':
				grid[i][j].load(i*20,j*20,"images/bottom right.bmp",false,false);
				break;
			case 'q':
				grid[i][j].load(i*20,j*20,"images/double top left.bmp",false,false);
				break;
			case 'e':
				grid[i][j].load(i*20,j*20,"images/double top right.bmp",false,false);
				break;
			case 'z':
				grid[i][j].load(i*20,j*20,"images/double bottom left.bmp",false,false);
				break;
			case 'c':
				grid[i][j].load(i*20,j*20,"images/double bottom right.bmp",false,false);
				break;
			case '!':
				grid[i][j].load(i*20,j*20,"images/special top left.bmp",false,false);
				break;
			case '@':
				grid[i][j].load(i*20,j*20,"images/special top right.bmp",false,false);
				break;
			case '#':
				grid[i][j].load(i*20,j*20,"images/special left top.bmp",false,false);
				break;
			case '%':
				grid[i][j].load(i*20,j*20,"images/special left bottom.bmp",false,false);
				break;
			case '$':
				grid[i][j].load(i*20,j*20,"images/special right top.bmp",false,false);
				break;
			case '^':
				grid[i][j].load(i*20,j*20,"images/special right bottom.bmp",false,false);
				break;
			case '<':
				grid[i][j].load(i*20,j*20,"images/square top left.bmp",false,false);
				break;
			case '>':
				grid[i][j].load(i*20,j*20,"images/square top right.bmp",false,false);
				break;
			case ',':
				grid[i][j].load(i*20,j*20,"images/square bottom left.bmp",false,false);
				break;
			case '.':
				grid[i][j].load(i*20,j*20,"images/square bottom right.bmp",false,false);
				break;
			case '_':
				grid[i][j].load(i*20,j*20,"images/ghost door.bmp",false,false);
				break;
			case '(':
				grid[i][j].load(i*20,j*20,"images/small top left.bmp",false,false);
				break;
			case ')':
				grid[i][j].load(i*20,j*20,"images/small top right.bmp",false,false);
				break;
			case '{':
				grid[i][j].load(i*20,j*20,"images/small bottom left.bmp",false,false);
				break;
			case '}':
				grid[i][j].load(i*20,j*20,"images/small bottom right.bmp",false,false);
				break;
			default:
				grid[i][j].load(i*20,j*20,"images/blank.bmp",false,false);
				break;
			}
		}
	}

	grid[12][26].up_allowed = false;
	grid[13][26].up_allowed = false;
	grid[14][26].up_allowed = false;
	grid[15][26].up_allowed = false;
	grid[16][26].up_allowed = false;
	grid[17][26].up_allowed = false;

	grid[12][14].up_allowed = false;
	grid[13][14].up_allowed = false;
	grid[14][14].up_allowed = false;
	grid[15][14].up_allowed = false;
	grid[16][14].up_allowed = false;
	grid[17][14].up_allowed = false;

	grid[22][17].is_tunnel = true;
	grid[23][17].is_tunnel = true;
	grid[24][17].is_tunnel = true;
	grid[25][17].is_tunnel = true;
	grid[26][17].is_tunnel = true;
	grid[27][17].is_tunnel = true;

	grid[0][17].is_tunnel = true;
	grid[1][17].is_tunnel = true;
	grid[2][17].is_tunnel = true;
	grid[3][17].is_tunnel = true;
	grid[4][17].is_tunnel = true;
	grid[5][17].is_tunnel = true;
}
void displayPacman(int time)
{
	if(pacman.frame == 0 && (time/100) % 2 == 0 && (pacman.xVel != 0 || pacman.yVel != 0))
	{
		pacman.frame++;
		pacman.image = loadImage("images/pacman closed.bmp");
	}
	else if(pacman.frame == 1 && (time/100) % 2 == 1 )
	{
		pacman.frame = 0;
		switch(pacman.direction)
		{
		case 1:
			pacman.image = loadImage("images/pacman open top.bmp");
			break;
		case 2:
			pacman.image = loadImage("images/pacman open right.bmp");
			break;
		case 3:
			pacman.image = loadImage("images/pacman open bottom.bmp");
			break;
		case 4:
			pacman.image = loadImage("images/pacman open left.bmp");
			break;
		}
	}
	if(delay_time>0)
		pacman.image = loadImage("images/pacman closed.bmp");

	if(delay_start_time + delay_time > SDL_GetTicks() && fright_mode)
		pacman.image = loadImage("images/blank.bmp");

	if(pacman_is_dead)
	{
		if(delay_start_time + 2000 < SDL_GetTicks())
		{
			pacman.image = loadImage("images/pacman closed.bmp");
			pacman_is_dead = false;
			lives_left--;
			levelReset();
		}
		else if(delay_start_time + 1800 < SDL_GetTicks())
			pacman.image = loadImage("images/death/6.bmp");
		else if(delay_start_time + 1500 < SDL_GetTicks())
			pacman.image = loadImage("images/death/5.bmp");
		else if(delay_start_time + 1200 < SDL_GetTicks())
			pacman.image = loadImage("images/death/4.bmp");
		else if(delay_start_time + 900 < SDL_GetTicks())
			pacman.image = loadImage("images/death/3.bmp");
		else if(delay_start_time + 600 < SDL_GetTicks())
			pacman.image = loadImage("images/death/2.bmp");
		else if(delay_start_time + 300 < SDL_GetTicks())
			pacman.image = loadImage("images/death/1.bmp");

	}
	applySurface(pacman.x - pacman.w/2 + 0.5, pacman.y - pacman.h/2 + 0.5, pacman.image, screen);
}
void checkPacman()
{
	if((pacman.getX() == red.getX() && pacman.getY() == red.getY() && red.is_active && red.frightened == false && red.is_dead == false) || (blue.is_dead == false &&pacman.getX() == blue.getX() && pacman.getY() == blue.getY() && blue.is_active && blue.frightened == false) || 
	   (pink.is_dead == false&&pacman.getX() == pink.getX() && pacman.getY() == pink.getY() && pink.is_active && pink.frightened == false) ||(orange.is_dead == false &&pacman.getX() == orange.getX() && pacman.getY() == orange.getY() && orange.is_active && orange.frightened == false))
	{
		delay_start_time = SDL_GetTicks();
		delay_time = 3000;
		Mix_HaltChannel(-1);
		Mix_PlayChannel(0,pacman_death,0);
		pacman_is_dead = true;
		fright_mode = false;
	}
}
void checkDot()
{
	if(grid[pacman.getX()][pacman.getY()].has_dot == true)
	{
		grid[pacman.getX()][pacman.getY()].has_dot = false;
		grid[pacman.getX()][pacman.getY()].image = loadImage("images/blank.bmp");
		score += 10;
		dots--;
		if(waka_sound_is_on == false)
		{
			waka_sound_is_on = true;
			Mix_PlayChannel(1, waka, 0);
		}
		lastPellet = SDL_GetTicks();
	}

	if(grid[pacman.nextX()][pacman.nextY()].has_dot == false && grid[pacman.nextX()][pacman.nextY()].is_powerup == false && lastPellet + 150 < SDL_GetTicks())
	{
		// Halt Waka Sound
		Mix_HaltChannel(1);
		waka_sound_is_on = false;
	}
}
void checkFruit()
{
	if(grid[pacman.getX()][pacman.getY()].has_fruit == true)
	{
		active_fruit = false;
		grid[14][20].has_fruit=false;
		grid[pacman.getX()][pacman.getY()].image = loadImage("images/blank.bmp");
		score += bonus_points;
		Mix_PlayChannel(4, ate_fruit, 0);
		bonus = bonus_points;
		bonus_text_time = 1000;
		bonus_start_time = SDL_GetTicks();
		bonus_x = grid[14][20].x-15;
		bonus_y = grid[14][20].y;
	}
}
void checkPowerUp()
{
	if(grid[pacman.getX()][pacman.getY()].is_powerup == true)
	{
		grid[pacman.getX()][pacman.getY()].is_powerup = false;
		grid[pacman.getX()][pacman.getY()].image = loadImage("images/blank.bmp");
		score += 40;
		dots--;
		setfrightMode();
	}
}
void checkGhost()
{
	if(pacman.getX() == red.getX() && pacman.getY() == red.getY() && red.frightened == true)
	{
		red.is_dead = true;
		red.frightened = false;
		switch(ghosts_eaten)
		{
		case 0:
			ghosts_eaten++;
			bonus = 200;
			bonus_text_time = 1000;
			bonus_start_time = SDL_GetTicks();
			bonus_x = grid[red.getX()][red.getY()].x-10;
			bonus_y = grid[red.getX()][red.getY()].y+5;
			delay_start_time = SDL_GetTicks();
			delay_time = 1000;
			Mix_Pause(0);
			Mix_Pause(1);
			Mix_Pause(2);
			Mix_Pause(3);
			Mix_Pause(5);
			Mix_PlayChannel(4, eating_ghost, 0);
			break;
		case 1:
			ghosts_eaten++;
			bonus = 400;
			bonus_text_time = 1000;
			bonus_start_time = SDL_GetTicks();
			bonus_x = grid[red.getX()][red.getY()].x-10;
			bonus_y = grid[red.getX()][red.getY()].y+5;
			delay_start_time = SDL_GetTicks();
			delay_time = 1000;
			Mix_Pause(0);
			Mix_Pause(1);
			Mix_Pause(2);
			Mix_Pause(3);
			Mix_Pause(5);
			Mix_PlayChannel(4, eating_ghost, 0);
			break;
		case 2:
			ghosts_eaten++;
			bonus = 800;
			bonus_text_time = 1000;
			bonus_start_time = SDL_GetTicks();
			bonus_x = grid[red.getX()][red.getY()].x-10;
			bonus_y = grid[red.getX()][red.getY()].y+5;
			delay_start_time = SDL_GetTicks();
			delay_time = 1000;
			Mix_Pause(0);
			Mix_Pause(1);
			Mix_Pause(2);
			Mix_Pause(3);
			Mix_Pause(5);
			Mix_PlayChannel(4, eating_ghost, 0);
			break;
		case 3:
			ghosts_eaten++;
			bonus = 1600;
			bonus_text_time = 1000;
			bonus_start_time = SDL_GetTicks();
			bonus_x = grid[red.getX()][red.getY()].x-10;
			bonus_y = grid[red.getX()][red.getY()].y+5;
			delay_start_time = SDL_GetTicks();
			delay_time = 1000;
			Mix_Pause(0);
			Mix_Pause(1);
			Mix_Pause(2);
			Mix_Pause(3);
			Mix_Pause(5);
			Mix_PlayChannel(4, eating_ghost, 0);
			break;
		}
	}
	if(pacman.getX() == pink.getX() && pacman.getY() == pink.getY() && pink.frightened == true)
	{
		pink.is_dead = true;
		pink.frightened = false;
		switch(ghosts_eaten)
		{
		case 0:
			ghosts_eaten++;
			bonus = 200;
			bonus_text_time = 1000;
			bonus_start_time = SDL_GetTicks();
			bonus_x = grid[pink.getX()][pink.getY()].x-10;
			bonus_y = grid[pink.getX()][pink.getY()].y+5;
			delay_start_time = SDL_GetTicks();
			delay_time = 1000;
			Mix_Pause(0);
			Mix_Pause(1);
			Mix_Pause(2);
			Mix_Pause(3);
			Mix_Pause(5);
			Mix_PlayChannel(4, eating_ghost, 0);
			break;
		case 1:
			ghosts_eaten++;
			bonus = 400;
			bonus_text_time = 1000;
			bonus_start_time = SDL_GetTicks();
			bonus_x = grid[pink.getX()][pink.getY()].x-10;
			bonus_y = grid[pink.getX()][pink.getY()].y+5;
			delay_start_time = SDL_GetTicks();
			delay_time = 1000;
			Mix_Pause(0);
			Mix_Pause(1);
			Mix_Pause(2);
			Mix_Pause(3);
			Mix_Pause(5);
			Mix_PlayChannel(4, eating_ghost, 0);
			break;
		case 2:
			ghosts_eaten++;
			bonus = 800;
			bonus_text_time = 1000;
			bonus_start_time = SDL_GetTicks();
			bonus_x = grid[pink.getX()][pink.getY()].x-10;
			bonus_y = grid[pink.getX()][pink.getY()].y+5;
			delay_start_time = SDL_GetTicks();
			delay_time = 1000;
			Mix_Pause(0);
			Mix_Pause(1);
			Mix_Pause(2);
			Mix_Pause(3);
			Mix_Pause(5);
			Mix_PlayChannel(4, eating_ghost, 0);
			break;
		case 3:
			ghosts_eaten++;
			bonus = 1600;
			bonus_text_time = 1000;
			bonus_start_time = SDL_GetTicks();
			bonus_x = grid[pink.getX()][pink.getY()].x-10;
			bonus_y = grid[pink.getX()][pink.getY()].y+5;
			delay_start_time = SDL_GetTicks();
			delay_time = 1000;
			Mix_Pause(0);
			Mix_Pause(1);
			Mix_Pause(2);
			Mix_Pause(3);
			Mix_Pause(5);
			Mix_PlayChannel(4, eating_ghost, 0);
			break;
		}
	}
	if(pacman.getX() == orange.getX() && pacman.getY() == orange.getY() && orange.frightened == true)
	{
		orange.is_dead = true;
		orange.frightened = false;
		switch(ghosts_eaten)
		{
		case 0:
			ghosts_eaten++;
			bonus = 200;
			bonus_text_time = 1000;
			bonus_start_time = SDL_GetTicks();
			bonus_x = grid[orange.getX()][orange.getY()].x-10;
			bonus_y = grid[orange.getX()][orange.getY()].y+5;
			delay_start_time = SDL_GetTicks();
			delay_time = 1000;
			Mix_Pause(0);
			Mix_Pause(1);
			Mix_Pause(2);
			Mix_Pause(3);
			Mix_Pause(5);
			Mix_PlayChannel(4, eating_ghost, 0);
			break;
		case 1:
			ghosts_eaten++;
			bonus = 400;
			bonus_text_time = 1000;
			bonus_start_time = SDL_GetTicks();
			bonus_x = grid[orange.getX()][orange.getY()].x-10;
			bonus_y = grid[orange.getX()][orange.getY()].y+5;
			delay_start_time = SDL_GetTicks();
			delay_time = 1000;
			Mix_Pause(0);
			Mix_Pause(1);
			Mix_Pause(2);
			Mix_Pause(3);
			Mix_Pause(5);
			Mix_PlayChannel(4, eating_ghost, 0);
			break;
		case 2:
			ghosts_eaten++;
			bonus = 800;
			bonus_text_time = 1000;
			bonus_start_time = SDL_GetTicks();
			bonus_x = grid[orange.getX()][orange.getY()].x-10;
			bonus_y = grid[orange.getX()][orange.getY()].y+5;
			delay_start_time = SDL_GetTicks();
			delay_time = 1000;
			Mix_Pause(0);
			Mix_Pause(1);
			Mix_Pause(2);
			Mix_Pause(3);
			Mix_Pause(5);
			Mix_PlayChannel(4, eating_ghost, 0);
			break;
		case 3:
			ghosts_eaten++;
			bonus = 1600;
			bonus_text_time = 1000;
			bonus_start_time = SDL_GetTicks();
			bonus_x = grid[orange.getX()][orange.getY()].x-10;
			bonus_y = grid[orange.getX()][orange.getY()].y+5;
			delay_start_time = SDL_GetTicks();
			delay_time = 1000;
			Mix_Pause(0);
			Mix_Pause(1);
			Mix_Pause(2);
			Mix_Pause(3);
			Mix_Pause(5);
			Mix_PlayChannel(4, eating_ghost, 0);
			break;
		}

	}
	if(pacman.getX() == blue.getX() && pacman.getY() == blue.getY() && blue.frightened == true)
	{
		blue.is_dead = true;
		blue.frightened = false;
		switch(ghosts_eaten)
		{
		case 0:
			ghosts_eaten++;
			bonus = 200;
			bonus_text_time = 1000;
			bonus_start_time = SDL_GetTicks();
			bonus_x = grid[blue.getX()][blue.getY()].x-10;
			bonus_y = grid[blue.getX()][blue.getY()].y+5;
			delay_start_time = SDL_GetTicks();
			delay_time = 1000;
			Mix_Pause(0);
			Mix_Pause(1);
			Mix_Pause(2);
			Mix_Pause(3);
			Mix_Pause(5);
			Mix_PlayChannel(4, eating_ghost, 0);
			break;
		case 1:
			ghosts_eaten++;
			bonus = 400;
			bonus_text_time = 1000;
			bonus_start_time = SDL_GetTicks();
			bonus_x = grid[blue.getX()][blue.getY()].x-10;
			bonus_y = grid[blue.getX()][blue.getY()].y+5;
			delay_start_time = SDL_GetTicks();
			delay_time = 1000;
			Mix_Pause(0);
			Mix_Pause(1);
			Mix_Pause(2);
			Mix_Pause(3);
			Mix_Pause(5);
			Mix_PlayChannel(4, eating_ghost, 0);
			break;
		case 2:
			ghosts_eaten++;
			bonus = 800;
			bonus_text_time = 1000;
			bonus_start_time = SDL_GetTicks();
			bonus_x = grid[blue.getX()][blue.getY()].x-10;
			bonus_y = grid[blue.getX()][blue.getY()].y+5;
			delay_start_time = SDL_GetTicks();
			delay_time = 1000;
			Mix_Pause(0);
			Mix_Pause(1);
			Mix_Pause(2);
			Mix_Pause(3);
			Mix_Pause(5);
			Mix_PlayChannel(4, eating_ghost, 0);
			break;
		case 3:
			ghosts_eaten++;
			bonus = 1600;
			bonus_text_time = 1000;
			bonus_start_time = SDL_GetTicks();
			bonus_x = grid[blue.getX()][blue.getY()].x-10;
			bonus_y = grid[blue.getX()][blue.getY()].y+5;
			delay_start_time = SDL_GetTicks();
			delay_time = 1000;
			Mix_Pause(0);
			Mix_Pause(1);
			Mix_Pause(2);
			Mix_Pause(3);
			Mix_Pause(5);
			Mix_PlayChannel(4, eating_ghost, 0);
			break;
		}
	}
}
void displayScores()
{
	// Display Lives Left
	pacman_lives_left = loadImage("images/pacman open left.bmp");
	for(int i=0;i<lives_left;i++)
	{
		applySurface(50*i+10, 685, pacman_lives_left, screen);
	}

	//Display Current Score + Text
	player_one_text = TTF_RenderText_Solid(font,"SCORE" ,text_color);
	applySurface(25, 5, player_one_text, screen);

	current_score_text = TTF_RenderText_Solid(font,convertInt(score).c_str(),text_color);
	applySurface(45, 25, current_score_text, screen);

	high_score_text = TTF_RenderText_Solid(font,"HIGH SCORE", text_color);
	applySurface(200,5,high_score_text,screen);

	// Display High Score
	std::string highscore_file = "highscore.txt";
	std::ifstream load;
	load.open(highscore_file.c_str());

	std::string shigh_score;
	load>> shigh_score;
	load.close();

	int highscore = atoi(shigh_score.c_str());

	if(score > highscore)
		highscore=score;

	std::ofstream save;
	save.open(highscore_file.c_str());
	save << highscore;

	highscore_text = TTF_RenderText_Solid(font,(convertInt(highscore)).c_str(),text_color);
	applySurface(250,25,highscore_text, screen);

	// Display Level
	level_text = TTF_RenderText_Solid(font,"LEVEL",text_color);
	applySurface(450,5,level_text,screen);

	level_text = TTF_RenderText_Solid(font,convertInt(level).c_str(),text_color);
	applySurface(485,25,level_text,screen);

}
void displayGhosts(int time)
{
	switch(red.direction)
	{
	case 1:
		red.image=loadImage("images/ghosts/red up.bmp");
		break;
	case 2:
		red.image=loadImage("images/ghosts/red right.bmp");
		break;
	case 3:
		red.image=loadImage("images/ghosts/red down.bmp");
		break;
	case 4:
		red.image=loadImage("images/ghosts/red left.bmp");
		break;
	}

	switch(blue.direction)
	{
	case 1:
		blue.image=loadImage("images/ghosts/blue up.bmp");
		break;
	case 2:
		blue.image=loadImage("images/ghosts/blue right.bmp");
		break;
	case 3:
		blue.image=loadImage("images/ghosts/blue down.bmp");
		break;
	case 4:
		blue.image=loadImage("images/ghosts/blue left.bmp");
		break;
	}
	switch(orange.direction)
	{
	case 1:
		orange.image=loadImage("images/ghosts/orange up.bmp");
		break;
	case 2:
		orange.image=loadImage("images/ghosts/orange right.bmp");
		break;
	case 3:
		orange.image=loadImage("images/ghosts/orange down.bmp");
		break;
	case 4:
		orange.image=loadImage("images/ghosts/orange left.bmp");
		break;
	}
	switch(pink.direction)
	{
	case 1:
		pink.image=loadImage("images/ghosts/pink up.bmp");
		break;
	case 2:
		pink.image=loadImage("images/ghosts/pink right.bmp");
		break;
	case 3:
		pink.image=loadImage("images/ghosts/pink down.bmp");
		break;
	case 4:
		pink.image=loadImage("images/ghosts/pink left.bmp");
		break;
	}

	if(red.frightened && int((time-fright_start_time) / ((double)number_of_flashes/2/(double)fright_time*1000)) %2 == 0)
	{
		red.image=loadImage("images/ghosts/fright blue.bmp");
	}
	else if(red.frightened && int((time-fright_start_time) / ((double)number_of_flashes/2/(double)fright_time*1000)) %2 == 1)
	{
		red.image=loadImage("images/ghosts/fright white.bmp");
	}

	if(blue.frightened && int((time-fright_start_time) / ((double)number_of_flashes/2/(double)fright_time*1000)) %2 == 0)
	{
		blue.image=loadImage("images/ghosts/fright blue.bmp");
	}
	else if(blue.frightened && int((time-fright_start_time) / ((double)number_of_flashes/2/(double)fright_time*1000)) %2 == 1)
	{
		blue.image=loadImage("images/ghosts/fright white.bmp");
	}

	if(pink.frightened && int((time-fright_start_time) / ((double)number_of_flashes/2/(double)fright_time*1000)) %2 == 0)
	{
		pink.image=loadImage("images/ghosts/fright blue.bmp");
	}
	else if(pink.frightened && int((time-fright_start_time) / ((double)number_of_flashes/2/(double)fright_time*1000)) %2 == 1)
	{
		pink.image=loadImage("images/ghosts/fright white.bmp");
	}

	if(orange.frightened && int((time-fright_start_time) / ((double)number_of_flashes/2/(double)fright_time*1000)) %2 == 0)
	{
		orange.image=loadImage("images/ghosts/fright blue.bmp");
	}
	else if(orange.frightened && int((time-fright_start_time) / ((double)number_of_flashes/2/(double)fright_time*1000)) %2 == 1)
	{
		orange.image=loadImage("images/ghosts/fright white.bmp");
	}

	if(red.is_dead)
	{
		switch(red.direction)
		{
		case 1:
			red.image=loadImage("images/ghosts/dead up.bmp");
			break;
		case 2:
			red.image=loadImage("images/ghosts/dead right.bmp");
			break;
		case 3:
			red.image=loadImage("images/ghosts/dead down.bmp");
			break;
		case 4:
			red.image=loadImage("images/ghosts/dead left.bmp");
			break;
		}
		if(delay_start_time + delay_time > SDL_GetTicks())
			red.image = loadImage("images/blank.bmp");
	}
	if(pink.is_dead)
	{
		switch(pink.direction)
		{
		case 1:
			pink.image=loadImage("images/ghosts/dead up.bmp");
			break;
		case 2:
			pink.image=loadImage("images/ghosts/dead right.bmp");
			break;
		case 3:
			pink.image=loadImage("images/ghosts/dead down.bmp");
			break;
		case 4:
			pink.image=loadImage("images/ghosts/dead left.bmp");
			break;
		}
		if(delay_start_time + delay_time > SDL_GetTicks())
			pink.image = loadImage("images/blank.bmp");
	}
	if(blue.is_dead)
	{
		switch(blue.direction)
		{
		case 1:
			blue.image=loadImage("images/ghosts/dead up.bmp");
			break;
		case 2:
			blue.image=loadImage("images/ghosts/dead right.bmp");
			break;
		case 3:
			blue.image=loadImage("images/ghosts/dead down.bmp");
			break;
		case 4:
			blue.image=loadImage("images/ghosts/dead left.bmp");
			break;
		}
		if(delay_start_time + delay_time > SDL_GetTicks())
			blue.image = loadImage("images/blank.bmp");
	}
	if(orange.is_dead)
	{
		switch(orange.direction)
		{
		case 1:
			orange.image=loadImage("images/ghosts/dead up.bmp");
			break;
		case 2:
			orange.image=loadImage("images/ghosts/dead right.bmp");
			break;
		case 3:
			orange.image=loadImage("images/ghosts/dead down.bmp");
			break;
		case 4:
			orange.image=loadImage("images/ghosts/dead left.bmp");
			break;
		}
		if(delay_start_time + delay_time > SDL_GetTicks())
			orange.image = loadImage("images/blank.bmp");
	}


	applySurface(blue.x - blue.w/2 + 0.5,blue.y - blue.h/2 + 0.5,blue.image,screen);
	applySurface(red.x - red.w/2 + 0.5,red.y - red.h/2 + 0.5,red.image,screen);
	applySurface(pink.x - pink.w/2 + 0.5,pink.y - pink.h/2 + 0.5,pink.image,screen);
	applySurface(orange.x - orange.w/2 + 0.5,orange.y - orange.h/2 + 0.5,orange.image,screen);
}
void displayFruit()
{
	if(level>12)
	{
		fruit = loadImage("images/fruit/key.bmp");
		applySurface(240,685,key,screen);
		applySurface(280,685,bell,screen);
		applySurface(320,685,galaxian,screen);
		applySurface(360,685,grapes,screen);
		applySurface(400,685,apple,screen);
		applySurface(440,685,peach,screen);
		applySurface(480,685,strawberry,screen);
		applySurface(520,685,cherry,screen);
	}
	else if(level>10)
	{
		fruit = loadImage("images/fruit/bell.bmp");
		applySurface(280,685,bell,screen);
		applySurface(320,685,galaxian,screen);
		applySurface(360,685,grapes,screen);
		applySurface(400,685,apple,screen);
		applySurface(440,685,peach,screen);
		applySurface(480,685,strawberry,screen);
		applySurface(520,685,cherry,screen);
	}
	else if(level>8)
	{
		fruit = loadImage("images/fruit/galaxian.bmp");
		applySurface(320,685,galaxian,screen);
		applySurface(360,685,grapes,screen);
		applySurface(400,685,apple,screen);
		applySurface(440,685,peach,screen);
		applySurface(480,685,strawberry,screen);
		applySurface(520,685,cherry,screen);
	}
	else if(level>6)
	{
		fruit = loadImage("images/fruit/grapes.bmp");
		applySurface(360,685,grapes,screen);
		applySurface(400,685,apple,screen);
		applySurface(440,685,peach,screen);
		applySurface(480,685,strawberry,screen);
		applySurface(520,685,cherry,screen);
	}
	else if(level>4)
	{
		fruit = loadImage("images/fruit/apple.bmp");
		applySurface(400,685,apple,screen);
		applySurface(440,685,peach,screen);
		applySurface(480,685,strawberry,screen);
		applySurface(520,685,cherry,screen);
	}
	else if(level>2)
	{
		fruit = loadImage("images/fruit/peach.bmp");
		applySurface(440,685,peach,screen);
		applySurface(480,685,strawberry,screen);
		applySurface(520,685,cherry,screen);
	}
	else if(level==2)
	{
		fruit = loadImage("images/fruit/strawberry.bmp");
		applySurface(480,685,strawberry,screen);
		applySurface(520,685,cherry,screen);
	}
	else
	{
		fruit = loadImage("images/fruit/cherries.bmp");
		applySurface(520,685,cherry,screen);
	}

	if(active_fruit)
	{
		applySurface(grid[14][20].x-15,grid[14][20].y-5,fruit,screen);
	}
}
void pacmanInit()
{
	pacman.toGrid(14,26);
	pacman.x-=10;
	pacman.xVel = -PACMAN_SPEED;
	pacman.direction = 4;
	pacman.frame = 0;
}
void redInit()
{
	red.xVel = GHOST_SPEED;
	red.yVel = GHOST_SPEED;
	red.image = loadImage("images/ghosts/red center.bmp");
	red.toGrid(14,14);
	red.next_tile_x = 13;
	red.next_tile_y = 14;
	red.x-=10;
	red.direction = 4;
	red.ghost_type = 1;
	red.frightened = false;
	red.is_active = true;
	red.is_dead = false;
}
void orangeInit()
{
	orange.xVel = GHOST_SPEED;
	orange.yVel = GHOST_SPEED;
	orange.image = loadImage("images/ghosts/orange center.bmp");
	orange.toGrid(16,17);
	orange.x -= 10;
	orange.direction = 0;
	orange.ghost_type = 4;
	orange.frightened = false;
	orange.is_active = false;
	orange.animate = true;
	orange.is_dead = false;
}
void blueInit()
{
	blue.xVel = GHOST_SPEED;
	blue.yVel = GHOST_SPEED;
	blue.image = loadImage("images/ghosts/blue center.bmp");
	blue.toGrid(12,17);
	blue.x -= 10;
	blue.direction = 0;
	blue.ghost_type = 3;
	blue.frightened = false;
	blue.is_active = false;
	blue.animate = true;
	blue.is_dead = false;
}
void pinkInit()
{
	pink.xVel = GHOST_SPEED;
	pink.yVel = GHOST_SPEED;
	pink.image = loadImage("images/ghosts/pink center.bmp");
	pink.toGrid(14,17);
	pink.x -= 10;
	pink.direction = 0;
	pink.ghost_type = 2;
	pink.frightened = false;
	pink.is_active = false;
	pink.animate = false;
	pink.is_dead = false;
}
void setFruit()
{
	if(active_fruit == false && (dots == 170 || dots == 70) && !first_fruit)
	{
		active_fruit = true;
		grid[14][20].has_fruit=true;
		fruitTimer = SDL_GetTicks();
	}

	if(active_fruit == false && (dots == 170 || dots == 70) && first_fruit && !second_fruit)
	{
		active_fruit = true;
		grid[14][20].has_fruit=true;
		fruitTimer = SDL_GetTicks();
	}

	if(active_fruit == true && SDL_GetTicks() > fruitTimer + 9500 + ghosts_eaten*1000)
	{
		active_fruit = false;
		grid[14][20].has_fruit=false;
		fruitTimer = SDL_GetTicks();
	}
}
bool isScatterMode()
{
	if(level>4)
	{
		if(game_time/1000>1093)
			return false;
		else if(game_time/1000>1092)
			return true;
		else if(game_time/1000>55)
			return false;
		else if(game_time/1000>50)
			return true;
		else if(game_time/1000>30)
			return false;
		else if(game_time/1000>25)
			return true;
		else if(game_time/1000>5)
			return false;
		else 
			return true;
	}
	else if(level>1)
	{
		if(game_time/1000>1093)
			return false;
		else if(game_time/1000>1092)
			return true;
		else if(game_time/1000>59)
			return false;
		else if(game_time/1000>54)
			return true;
		else if(game_time/1000>34)
			return false;
		else if(game_time/1000>27)
			return true;
		else if(game_time/1000>7)
			return false;
		else 
			return true;
	}
	else
	{
		if(game_time/1000>84)
			return false;
		else if(game_time/1000>79)
			return true;
		else if(game_time/1000>59)
			return false;
		else if(game_time/1000>54)
			return true;
		else if(game_time/1000>34)
			return false;
		else if(game_time/1000>27)
			return true;
		else if(game_time/1000>7)
			return false;
		else 
			return true;
	}
}
void levelReset()
{
	pacmanInit();
	pinkInit();
	blueInit();
	redInit();
	orangeInit();
}
void displayReadyScreen()
{
	if(ready_screen)
	{
		ready_text = loadImage("images/ready.bmp");
		applySurface(230,395,ready_text,screen);
	}
	if(lives_left < 0 )
	{
		ready_text = loadImage("images/game_over.bmp");
		applySurface(230,395,ready_text,screen);
	}
}
void setNextLevel()
{
	if(dots == 0 && new_game == false)
	{
		level++;
		loadLevel(level);
		levelReset();
		game_time = SDL_GetTicks();
		newLevel();
	}
}
void newLevel()
{
	Mix_HaltChannel(-1);
	Mix_PlayChannel(-1, start_music, 0);
	delay_start_time = SDL_GetTicks();
	delay_time = 5000;
	ready_screen = true;
	ghosts_eaten = 0;
}
void setfrightMode()
{
	fright_mode = true;
	if(red.is_active == true)
		red.frightened = true;
	if(blue.is_active == true)
		blue.frightened = true;
	if(orange.is_active == true)
		orange.frightened = true;
	if(pink.is_active == true)
		pink.frightened = true;
	fright_start_time = SDL_GetTicks();

	fright_mode = true;

	if(red.frightened)
	{
		switch(red.direction)
		{
		case 1:
			red.direction = 3;
			red.next_tile_y += 1;
			break;
		case 2:
			red.direction = 4;
			red.next_tile_x -= 1;
			break;
		case 3:
			red.direction = 1;
			red.next_tile_y -= 1;
			break;
		case 4:
			red.direction = 2;
			red.next_tile_x += 1;
			break;
		}
	}
	if(blue.frightened)
	{
		switch(blue.direction)
		{
		case 1:
			blue.direction = 3;
			blue.next_tile_y += 1;
			break;
		case 2:
			blue.direction = 4;
			blue.next_tile_x -= 1;
			break;
		case 3:
			blue.direction = 1;
			blue.next_tile_y -= 1;
			break;
		case 4:
			blue.direction = 2;
			blue.next_tile_x += 1;
			break;
		}
	}
	if(orange.frightened)
	{
		switch(orange.direction)
		{
		case 1:
			orange.direction = 3;
			orange.next_tile_y += 1;
			break;
		case 2:
			orange.direction = 4;
			orange.next_tile_x -= 1;
			break;
		case 3:
			orange.direction = 1;
			orange.next_tile_y -= 1;
			break;
		case 4:
			orange.direction = 2;
			orange.next_tile_x += 1;
			break;
		}
	}
	if(pink.frightened)
	{
		switch(pink.direction)
		{
		case 1:
			pink.direction = 3;
			pink.next_tile_y += 1;
			break;
		case 2:
			pink.direction = 4;
			pink.next_tile_x -= 1;
			break;
		case 3:
			pink.direction = 1;
			pink.next_tile_y -= 1;
			break;
		case 4:
			pink.direction = 2;
			pink.next_tile_x += 1;
			break;
		}
	}
}
void frightMode()
{
	if(SDL_GetTicks() - fright_time*1000 - ghosts_eaten*1000> fright_start_time || ghosts_eaten == 4)
	{
		ghosts_eaten = 0;
		red.frightened = false;
		orange.frightened = false;
		pink.frightened = false;
		blue.frightened = false;
		
		fright_mode = false;
	}
}
void ghostHouse()
{
	if(blue.is_active == false && blue.is_dead == false)
	{
		blue.moveFromGhostHouse();
		if(blue.y < grid[14][14].centerY())
			blue.y=grid[14][14].centerY();

	}
	if(orange.is_active == false && orange.is_dead == false)
	{
		orange.moveFromGhostHouse();
		if(orange.y < grid[14][14].centerY())
			orange.y=grid[14][14].centerY();
	}

	if(red.is_active == false && dots < 244)
	{
		red.moveFromGhostHouse();

		if(red.y==grid[14][14].centerY())
		{
			if(pacman.x > 280)
			{
				red.next_tile_x = 15;
				red.next_tile_y = 14;
				red.direction = 2;
			}

			else
			{
				red.next_tile_x = 13;
				red.next_tile_y = 14;
				red.direction = 4;
			}
			red.is_active = true;
		}
	}

	if(pink.is_active == false && dots < 244)
	{
		pink.moveFromGhostHouse();

		if(pink.y==grid[14][14].centerY())
		{
			if(pacman.x > 280)
			{
				pink.next_tile_x = 15;
				pink.next_tile_y = 14;
				pink.direction = 2;
			}

			else
			{
				pink.next_tile_x = 13;
				pink.next_tile_y = 14;
				pink.direction = 4;
			}
			pink.is_active = true;
		}
	}

	else if(blue.is_active == false && ((dots < 214 && level == 1) || (dots < 244 && level > 1)))
	{
		blue.animate = false;

		if(blue.y==grid[14][14].centerY())
		{
			if(pacman.x > 280)
			{
				blue.next_tile_x = 15;
				blue.next_tile_y = 14;
				blue.direction = 2;
			}

			else
			{
				blue.next_tile_x = 13;
				blue.next_tile_y = 14;
				blue.direction = 4;
			}
			blue.is_active = true;
		}
	}

	else if(orange.is_active == false && ((dots < 184 && level == 1) || (dots < 194 && level == 2) || (dots < 244 && level > 2)))
	{
		orange.animate = false;

		if(orange.y==grid[14][14].centerY())
		{
			if(pacman.x > 280)
			{
				orange.next_tile_x = 15;
				orange.next_tile_y = 14;
				orange.direction = 2;
			}

			else
			{
				orange.next_tile_x = 13;
				orange.next_tile_y = 14;
				orange.direction = 4;
			}
			orange.is_active = true;
		}
	}

}
void displayBonus()
{
	if(bonus_text_time > 0)
	{
		bonus_text = TTF_RenderText_Solid(bonus_font,convertInt(bonus).c_str(),bonus_text_color);
		applySurface(bonus_x,bonus_y,bonus_text,screen);
	}
	if(SDL_GetTicks() > bonus_start_time + bonus_text_time)
	{
		bonus_text_time = 0;
	}
}
void ghostAudio()
{
	if(Mix_Playing(3) == 0 && (red.is_dead || blue.is_dead || pink.is_dead || orange.is_dead) && delay_start_time + delay_time < SDL_GetTicks())
	{
		Mix_PlayChannel(3, ghost_death, 0);
	}
	else if(!(red.is_dead || blue.is_dead || pink.is_dead || orange.is_dead))
	{
		Mix_HaltChannel(3);
	}
	if(fright_mode == false && Mix_Playing(2) == 0 && delay_start_time + delay_time < SDL_GetTicks())
	{
		Mix_HaltChannel(5);
		Mix_PlayChannel(2, ghost_chase , 0);
	}
	else if(fright_mode == true && Mix_Playing(5) == 0 && delay_start_time + delay_time < SDL_GetTicks())
	{
		Mix_HaltChannel(2);
		Mix_PlayChannel(5, fright_sound, 0);
	}
	if(new_game == true)
		Mix_HaltChannel(-1);
}
void gameOver()
{
	if(new_game == true)
	{
		new_game = false;

		lives_left = 2;

		loadLevel(1);
		levelReset();
		newLevel();

		redInit();
		orangeInit();
		pinkInit();
		blueInit();

		score = 0;
		level = 1;
		bonus_life_remaining = true;
	}
}