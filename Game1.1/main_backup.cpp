#include "raylib.h"

enum class GameState {MENU, PLAYING, PAUSED};
constexpr int SCREEN_WIDTH = 1000;
constexpr int SCREEN_HEIGHT = 600;
constexpr float PLAYER_SIZE = 30.0f;
constexpr float MOVE_SPEED = 300.0f;
constexpr float GRAVITY = 1000.0f;
constexpr float JUMP_SPEED = 600.0f;

struct Player {
	Vector2 position;
	Vector2 speed;
	Vector2 size;
	Color color;
	bool onGround;
	int jumpsleft;
	Vector2 checkpoint;
};

bool DrawButton(const char* text, int x, int y, int w, int h) {
	Rectangle buttonRect = { (float)x, (float)y, (float)w, (float)h };
	bool hover = CheckCollisionPointRec(GetMousePosition(), buttonRect);
	DrawRectangleRec(buttonRect, hover ? BEIGE : LIGHTGRAY);
	DrawRectangleLinesEx(buttonRect, 2.0f, WHITE);
	int fontSize = 22;
	int textWidth = MeasureText(text, fontSize);
	DrawText(text, x + w / 2 - textWidth / 2, y + h / 2 - fontSize / 2, fontSize, BLACK);
	return hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

void ResetGame(Player& player) {
	player.position = { 100.0f, 300.0f };
	player.speed = { 0.0f, 0.0f };
	player.size = { PLAYER_SIZE, PLAYER_SIZE };
	player.color = SKYBLUE;
	player.onGround = false;
	player.jumpsleft = 2;
	player.checkpoint = { 100.0f, 300.0f };
}

void UpdatePlayer(Player& player, float dt) {
	float dir = 0.0f;
	if (IsKeyDown(KEY_A)) dir -= 1.0f;
	if (IsKeyDown(KEY_D)) dir += 1.0f;

	bool jumpPressed = IsKeyPressed(KEY_SPACE);

	if (IsGamepadAvailable(0)) {
		float stickX = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
		const float DEADZONE = 0.2f;
		if (stickX < -DEADZONE) dir -= 1.0f;
		if (stickX > DEADZONE) dir += 1.0f;

		if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))
			jumpPressed = true;
	}
	player.speed.x = dir * MOVE_SPEED;
	player.speed.y += GRAVITY * dt;

	if (jumpPressed && player.jumpsleft > 0) {
		player.speed.y = -JUMP_SPEED;
		player.jumpsleft--;
	}
}

constexpr int TILE = 40;

const char* LEVEL[] = {
	"##################################################################################################################################################################################################################",
	"#................................................................................................................................................................................................................#",
	"#................................................................................................................................................................................................................#",
	"#................................................................................................................................................................................................................#",
	"#..............................................................................................................................................................................................#.................#",
	"#.......####............####.......................####...........................................................................#######......................................................#.................#",
	"#.................................................................................................................................#######......................................................#.................#",
	"#.................................................................................................................................#######..........................#####.......................#.................#",
	"#.................................................................................................................................#######......................................................#.................#",
	"#............####..............................####....................................#######.........##....#######..............#######........................................#######.......#.................#",
	"#..........................####........................................................................##.........................#######......................................................#.................#",
	"#......................................................................................................##.........................#######......................................................#.................#",
	"#.....................................................######...........................................##.........................#######......................................................#.................#",
	"#......................................................................................................##.........................#######......................................................#.................#",
	"############......########################......###########################                            ###########################################################################################################",
	"############......########################......###########################                            ###########################################################################################################",
};
constexpr int MAP_H = sizeof(LEVEL) / sizeof(LEVEL[0]);
int MAP_W = 0;

bool IsSolid(int tx, int ty) {
	if (ty < 0 || ty >= MAP_H) return false;
	if (tx < 0 || tx >= MAP_W) return true;
	return LEVEL[ty][tx] == '#';
}

void ResolveAxis(Player& p, bool horizontal) {
	int left = (int)(p.position.x / TILE);
	int right = (int)((p.position.x + p.size.x) / TILE);
	int top = (int)(p.position.y / TILE);
	int bottom = (int)((p.position.y + p.size.y) / TILE);

	for (int ty = top; ty <= bottom; ty++) {
		for (int tx = left; tx <= right; tx++) {
			if (!IsSolid(tx, ty)) continue;

			Rectangle tileRect = { (float)tx * TILE, (float)ty * TILE, (float)TILE, (float)TILE };
			Rectangle playerRect = { p.position.x, p.position.y, p.size.x, p.size.y };
			if (!CheckCollisionRecs(playerRect, tileRect)) continue;

			if (horizontal) {
				if (p.speed.x > 0)      p.position.x = tx * TILE - p.size.x;
				else if (p.speed.x < 0) p.position.x = (tx + 1) * TILE;
				p.speed.x = 0;
			}
			else {
				if (p.speed.y > 0) { p.position.y = ty * TILE - p.size.y; p.onGround = true; }
				else if (p.speed.y < 0) p.position.y = (ty + 1) * TILE;
				p.speed.y = 0;
			}
		}
	}
}

void MoveAndCollide(Player& p, float dt) {
	p.position.x += p.speed.x * dt;
	ResolveAxis(p, true);

	p.position.y += p.speed.y * dt;
	p.onGround = false;
	ResolveAxis(p, false);
}

void UpdateCamera(Camera2D& cam, Player& p) {
	cam.target.x = p.position.x + p.size.x / 2.0f;
	cam.target.y = p.position.y + p.size.y / 2.0f;

	float halfW = SCREEN_WIDTH / 2.0f;
	float halfH = SCREEN_HEIGHT / 2.0f;
	float mapW = MAP_W * (float)TILE;
	float mapH = MAP_H * (float)TILE;

	if (mapW <= SCREEN_WIDTH) cam.target.x = mapW / 2.0f;
	else {
		if (cam.target.x < halfW)        cam.target.x = halfW;
		if (cam.target.x > mapW - halfW) cam.target.x = mapW - halfW;
	}

	if (mapH <= SCREEN_HEIGHT) cam.target.y = mapH / 2.0f;
	else {
		if (cam.target.y < halfH)        cam.target.y = halfH;
		if (cam.target.y > mapH - halfH) cam.target.y = mapH - halfH;
	}
}

void DrawWorld(const Player& player, Camera2D camera) {
	BeginMode2D(camera);
	for (int ty = 0; ty < MAP_H; ty++) {
		for (int tx = 0; tx < MAP_W; tx++) {
			if (LEVEL[ty][tx] == '#') {
				DrawRectangle(tx * TILE, ty * TILE, TILE, TILE, DARKBLUE);
			}
		}
	}
	DrawRectangleV(player.position, player.size, player.color);
	EndMode2D();
}

int main() {
	InitWindow(1000, 600, "My Game");
	SetTargetFPS(60);
	SetExitKey(KEY_NULL);

	MAP_W = TextLength(LEVEL[0]);

	Camera2D camera = { 0 };
	camera.offset = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
	camera.zoom = 1.0f;

	GameState state = GameState::MENU;
	Player player;
	ResetGame(player);

	while (!WindowShouldClose()) {
		float dt = GetFrameTime();

		BeginDrawing();
		ClearBackground(DARKGRAY);

		if (state == GameState::MENU) {
			const char* title = "MY GAME";
			DrawText(title, SCREEN_WIDTH / 2 - MeasureText(title, 60) / 2, 120, 60, RAYWHITE);
			int btnW = 320, btnX = SCREEN_WIDTH / 2 - btnW / 2;
			if (DrawButton("Start New Game", btnX, 280, btnW, 60)) {
				ResetGame(player);
				state = GameState::PLAYING;
			}
			if (DrawButton("Continue", btnX, 370, btnW, 60)) {
				state = GameState::PLAYING;
			}
		}
		else if (state == GameState::PLAYING) {
			if (IsKeyPressed(KEY_ESCAPE)) state = GameState::PAUSED;

			UpdatePlayer(player, dt);
			MoveAndCollide(player, dt);

			if (player.onGround) {
				player.jumpsleft = 2;
				player.checkpoint = player.position;
			}
			if (player.position.y > MAP_H * TILE) {
				player.position = player.checkpoint;
				player.speed = { 0.0f, 0.0f };
			}
			UpdateCamera(camera, player);

			DrawWorld(player, camera);
			DrawText("Playing  (ESC pause later)", 20, 20, 22, RAYWHITE);
		}
		else if (state == GameState::PAUSED) {
			DrawWorld(player, camera);

			DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.5f));

			const char* t = "PAUSED";
			DrawText(t, SCREEN_WIDTH / 2 - MeasureText(t, 60) / 2, 120, 60, RAYWHITE);

			int btnW = 320, btnX = SCREEN_WIDTH / 2 - btnW / 2;
			if (DrawButton("Resume", btnX, 280, btnW, 60) || IsKeyPressed(KEY_ESCAPE)) {
				state = GameState::PLAYING;
			}
			if (DrawButton("Back to Menu", btnX, 370, btnW, 60)) {
				state = GameState::MENU;
			}
		}
		EndDrawing();
	}
	CloseWindow();
	return 0;
}
