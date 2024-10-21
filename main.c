#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "raylib.h"

#define SCREEN_WIDTH		800
#define SCREEN_HEIGHT		800
#define PLAYER_RADIUS		10
#define BULLET_RADIUS		5
#define SPEED				10
#define BULLET_SPEED		15
#define MAX_ENNEMIES_COUNT	10

typedef struct {
	size_t x;
	size_t y;
} Player;

typedef struct {
	int x;
	int y;
	size_t dir_x;
	size_t dir_y;
	size_t from; 	// 0: shot by player | 1: shot by ennemies
} Bullet;

typedef struct {
	size_t count;
	Bullet *bullets;
} BulletArray;

typedef struct {
	size_t count;
	Player *ennemies;
} EnnemiesArray;

typedef struct {
	int w;
	int h;
	Vector2 position;
} ScreenInfo;

void init_player(Player *player)
{
	player->x = SCREEN_WIDTH / 2;
	player->y = SCREEN_HEIGHT / 2;
}

void destroy_bullet(BulletArray *bulletArray, size_t index)
{
	Bullet temp = bulletArray->bullets[index];
	bulletArray->bullets[index] = bulletArray->bullets[bulletArray->count - 1];
	bulletArray->bullets[bulletArray->count - 1] = temp;
	--bulletArray->count;
}

void destroy_ennemie(EnnemiesArray *ennemiesArray, size_t index)
{
	Player temp = ennemiesArray->ennemies[index];
	ennemiesArray->ennemies[index] = ennemiesArray->ennemies[ennemiesArray->count - 1];
	ennemiesArray->ennemies[ennemiesArray->count - 1] = temp;
	--ennemiesArray->count;
}

void shoot(BulletArray *bulletArray, Player *player, Vector2 *mouse_pos)
{
	int x_diff = ((int)mouse_pos->x - (int)player->x) / 20;
	int y_diff = ((int)mouse_pos->y - (int)player->y) / 20;
	
	++bulletArray->count;
	bulletArray->bullets = (Bullet *)realloc(bulletArray->bullets, bulletArray->count * sizeof(Bullet));
	bulletArray->bullets[bulletArray->count - 1].x = player->x;
	bulletArray->bullets[bulletArray->count - 1].y = player->y;
	bulletArray->bullets[bulletArray->count - 1].dir_x = x_diff;
	bulletArray->bullets[bulletArray->count - 1].dir_y = y_diff;
	bulletArray->bullets[bulletArray->count - 1].from = 0;
}

void draw_bullets(BulletArray *bulletArray, ScreenInfo *screen_info)
{
	for(size_t i = 0; i < bulletArray->count; ++i) {
		if(bulletArray->bullets[i].x > screen_info->w - BULLET_RADIUS ||
				bulletArray->bullets[i].y > screen_info->h - BULLET_RADIUS ||
					bulletArray->bullets[i].x < BULLET_RADIUS ||
						bulletArray->bullets[i].y < BULLET_RADIUS) {
			if(bulletArray->bullets[i].x > screen_info->w - BULLET_RADIUS) {
				screen_info->w += 10;
				SetWindowSize(screen_info->w, screen_info->h);
			} else if(bulletArray->bullets[i].y > screen_info->h - BULLET_RADIUS) {
				screen_info->h += 10;
				SetWindowSize(screen_info->w, screen_info->h);
			} else if(bulletArray->bullets[i].x < BULLET_RADIUS) {
				screen_info->w += 10;
				SetWindowSize(screen_info->w, screen_info->h);
				screen_info->position.x -= 10;
				SetWindowPosition(screen_info->position.x, screen_info->position.y);
			} else if(bulletArray->bullets[i].y < BULLET_RADIUS) {
				screen_info->h += 10;
				SetWindowSize(screen_info->w, screen_info->h);
				screen_info->position.y -= 10;
				SetWindowPosition(screen_info->position.x, screen_info->position.y);
			}
			destroy_bullet(bulletArray, i);
		}
		DrawCircle(bulletArray->bullets[i].x, bulletArray->bullets[i].y, BULLET_RADIUS, WHITE);
		bulletArray->bullets[i].x += bulletArray->bullets[i].dir_x;
		bulletArray->bullets[i].y += bulletArray->bullets[i].dir_y;
	}
}

void generate_ennemies(EnnemiesArray *ennemiesArray, ScreenInfo *screen_info)
{
	if(rand() % 50 == 1 && ennemiesArray->count < MAX_ENNEMIES_COUNT) {
		++ennemiesArray->count;
		ennemiesArray->ennemies = (Player*)realloc(ennemiesArray->ennemies, ennemiesArray->count * sizeof(Player));
		ennemiesArray->ennemies[ennemiesArray->count - 1].x = rand() % screen_info->w - PLAYER_RADIUS;
		ennemiesArray->ennemies[ennemiesArray->count - 1].y = rand() % screen_info->h - PLAYER_RADIUS;
	}
}

void draw_ennemies(EnnemiesArray *ennemiesArray, ScreenInfo *screen_info, Texture2D *ennemy_texture)
{
	for(size_t i = 0; i < ennemiesArray->count; ++i) {
		if(ennemiesArray->ennemies[i].x > screen_info->w ||
				ennemiesArray->ennemies[i].x < PLAYER_RADIUS ||
					ennemiesArray->ennemies[i].y > screen_info->h ||
						ennemiesArray->ennemies[i].y < PLAYER_RADIUS)
			destroy_ennemie(ennemiesArray, i);
		//DrawCircle(ennemiesArray->ennemies[i].x, ennemiesArray->ennemies[i].y, PLAYER_RADIUS, GREEN);
		DrawTextureEx(*ennemy_texture, (Vector2){ ennemiesArray->ennemies[i].x, ennemiesArray->ennemies[i].y }, 50, 0.1, WHITE);
	}
}

void check_collision(BulletArray *bulletArray, EnnemiesArray *ennemiesArray, size_t *score)
{
	for(size_t i = 0; i < bulletArray->count; ++i) {
		for(size_t j = 0; j < ennemiesArray->count; ++j) {
			if(CheckCollisionCircles(
						(Vector2){ bulletArray->bullets[i].x, bulletArray->bullets[i].y }, 
							BULLET_RADIUS,
								(Vector2){ ennemiesArray->ennemies[j].x, ennemiesArray->ennemies[j].y }, 
									PLAYER_RADIUS)) {
				destroy_bullet(bulletArray, i);
				destroy_ennemie(ennemiesArray, j);
				++(*score);
			}
		}
	}
}

int main(void)
{
	srand(time(NULL));
	
	BulletArray bulletArray = {0};
	Player player = {0};
	EnnemiesArray ennemiesArray = {0};
	ScreenInfo screen_info = {
		.w = SCREEN_WIDTH,
		.h = SCREEN_HEIGHT
	};
	init_player(&player);

	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Space Invader");
	
	Vector2 mouse_pos = {0};
	screen_info.position = GetWindowPosition();

	size_t score = 0;

	Image ship = LoadImage("./assets/ship.png");
	Image ennemy_image = LoadImage("./assets/ennemy.png");

	Texture2D ship_texture = LoadTextureFromImage(ship);
	Texture2D ennemy_texture = LoadTextureFromImage(ennemy_image);

	UnloadImage(ship);
	UnloadImage(ennemy_image);

	float adj = 0;
	float opp = 0;
	float hyp = 0;
	float res = 0;
	
	while(!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(BLACK);
			// Get mouse infos
			mouse_pos = GetMousePosition();
			DrawCircle(mouse_pos.x, mouse_pos.y, BULLET_RADIUS, WHITE);
			//DrawLine(player.x, player.y, mouse_pos.x, mouse_pos.y, WHITE);
			
			// Key event
			if(IsKeyDown(KEY_W) && player.y > PLAYER_RADIUS) player.y -= 10;
			if(IsKeyDown(KEY_S) && player.y < screen_info.h - PLAYER_RADIUS) player.y += 10;
			if(IsKeyDown(KEY_A) && player.x > PLAYER_RADIUS) player.x -= 10;
			if(IsKeyDown(KEY_D) && player.x < screen_info.w - PLAYER_RADIUS) player.x += 10;
			if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) shoot(&bulletArray, &player, &mouse_pos);
			
			// Check if player overflow the window
			if(player.y > screen_info.h) player.y -= PLAYER_RADIUS * 2;
			if(player.x > screen_info.w) player.x -= PLAYER_RADIUS * 2;
			
			// Draw player
			//DrawCircle(player.x, player.y, PLAYER_RADIUS, WHITE);
			
			adj = (float)player.x - (float)mouse_pos.x;
			opp = (float)player.y - (float)mouse_pos.y;
			hyp = sqrt(adj*adj + opp*opp);
			res = ((atan(opp/adj) * 180) / 3.141519) - 5;
			
			if(mouse_pos.x < player.x) res += 180;
			DrawTextureEx(ship_texture, (Vector2){ player.x, player.y }, res, 0.1, WHITE);

			// Draw bullets
			draw_bullets(&bulletArray, &screen_info);
			
			// Draw and generate ennemies
			generate_ennemies(&ennemiesArray, &screen_info);
			draw_ennemies(&ennemiesArray, &screen_info, &ennemy_texture);
			
			// Check bullets collisions with ennemies
			check_collision(&bulletArray, &ennemiesArray, &score);
			
			screen_info.w -= 0.0000000000001;
			screen_info.h -= 0.0000000000001;
			SetWindowSize(screen_info.w, screen_info.h);
		
			DrawText(TextFormat("Score: %zu", score), 10, screen_info.h - 25, 20, WHITE);
			
			WaitTime(0.03);
		EndDrawing();
	}
	free(bulletArray.bullets);
	free(ennemiesArray.ennemies);
	return 0;
}
