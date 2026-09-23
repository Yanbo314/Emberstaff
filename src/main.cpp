#include "raylib.h"
#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <algorithm>

enum class GameState { MENU, PLAYING, PAUSED, EDITOR, WIN };
enum class Weapon    { Staff = 0, Sword = 1 };
enum class AnimState { Walk, JumpRise, JumpApex, JumpFall, Roll, Attack, Hurt };

constexpr int   SCREEN_WIDTH  = 1000;
constexpr int   SCREEN_HEIGHT = 600;
constexpr int   TILE          = 40;

constexpr float PLAYER_SIZE   = 30.0f;
constexpr float MOVE_SPEED    = 300.0f;
constexpr float GRAVITY       = 1000.0f;
constexpr float JUMP_SPEED    = 600.0f;

constexpr float ACCELERATION  = 2600.0f;
constexpr float AIR_ACCEL     = 1400.0f;
constexpr float FRICTION      = 3200.0f;
constexpr float AIR_FRICTION  = 700.0f;
constexpr float COYOTE_TIME   = 0.10f;
constexpr float JUMP_BUFFER   = 0.10f;
constexpr float FALL_MULT     = 1.7f;
constexpr float LOW_JUMP_CUT  = 0.45f;
constexpr float MAX_FALL      = 1500.0f;
constexpr float AIR_JUMP_LOCKOUT = 0.10f;

constexpr const char* GAME_TITLE = "Emberstaff";

constexpr const char* CHAR_DIR   = "assets/character/";
constexpr const char* BG_DIR     = "assets/background/";
constexpr const char* SFX_DIR    = "assets/sfx/";
constexpr const char* LEVEL_FILE = "levels/level.txt";

constexpr float CHAR_DRAW_SIZE     = 64.0f;
constexpr float PICKUP_DRAW_SIZE   = 32.0f;
constexpr float FIREBALL_DRAW_SIZE = 28.0f;
constexpr float FX_HEAL_DRAW_SIZE  = 22.0f;
constexpr float FX_SPARK_DRAW_SIZE = 32.0f;

constexpr float WALK_FRAME_TIME         = 0.11f;
constexpr float ROLL_FRAME_TIME         = 0.09f;
constexpr float ATTACK_FRAME_TIME       = 0.13f;
constexpr float HURT_FRAME_TIME         = 0.14f;
constexpr float PICKUP_FRAME_TIME       = 0.08f;
constexpr float FIREBALL_FRAME_TIME     = 0.08f;
constexpr float ENEMY_MOVE_FRAME_TIME   = 0.12f;
constexpr float ENEMY_ATTACK_FRAME_TIME = 0.12f;
constexpr float ENEMY_HURT_FRAME_TIME   = 0.12f;
constexpr float ENEMY_DEATH_FRAME_TIME  = 0.13f;

constexpr float APEX_THRESHOLD      = 80.0f;
constexpr float WALK_ANIM_MIN_SPEED = 15.0f;
constexpr float ROLL_DURATION       = 4 * ROLL_FRAME_TIME;
constexpr float HURT_DURATION       = 2 * HURT_FRAME_TIME;
constexpr float WALK_BOB_PX         = 2.0f;
constexpr float IDLE_BOB_PX         = 1.0f;
constexpr float IDLE_BOB_PERIOD     = 0.6f;
constexpr float LAND_DIP_PX         = 2.0f;
constexpr float LAND_DIP_TIME       = 0.09f;

constexpr int   PLAYER_MAX_HP       = 5;
constexpr float INVINCIBLE_TIME     = 1.0f;
constexpr float BLINK_INTERVAL      = 0.1f;
constexpr float KNOCKBACK_X         = 350.0f;
constexpr float KNOCKBACK_Y         = 300.0f;
constexpr float ATTACK_DURATION     = 3 * ATTACK_FRAME_TIME;
constexpr float ATTACK_COOLDOWN     = 0.45f;
constexpr float ATTACK_HOVER_TIME   = 0.2f;
constexpr float ATTACK_HOVER_DAMP   = 0.15f;
constexpr float SWORD_RANGE         = 46.0f;
constexpr int   SWORD_DAMAGE        = 2;
constexpr int   FIREBALL_DAMAGE     = 1;
constexpr float FIREBALL_SPEED      = 520.0f;
constexpr float FIREBALL_LIFETIME   = 1.2f;
constexpr float FIREBALL_HITBOX     = 22.0f;

constexpr int   PLAYER_MAX_MANA     = 3;
constexpr float MANA_REGEN_TIME     = 2.5f;
constexpr float ROLL_SPEED_MULT     = 1.4f;
constexpr float ROLL_COOLDOWN      = 0.55f;

constexpr float PICKUP_HITBOX        = 26.0f;
constexpr float BOOST_DURATION       = 5.0f;
constexpr float BOOST_MULT           = 1.5f;
constexpr float BOOST_TINT_INTERVAL  = 0.1f;
constexpr Color BOOST_TINT           = { 185, 40, 40, 255 };
constexpr float HEALTH_RESPAWN_TIME  = 10.0f;
constexpr float HEAL_FX_LIFETIME     = 2.0f;
constexpr float SPARKLE_FX_LIFETIME  = 0.25f;

constexpr int   ENEMY_HP              = 3;
constexpr float ENEMY_W               = 40.0f;
constexpr float ENEMY_H               = 32.0f;
constexpr float ENEMY_DRAW_SIZE       = 96.0f;
constexpr float ENEMY_HOP_SPEED_X     = 90.0f;
constexpr float ENEMY_HOP_SPEED_Y     = 380.0f;
constexpr float ENEMY_HOP_INTERVAL    = 0.9f;
constexpr float ENEMY_AGGRO_RANGE_X   = 170.0f;
constexpr float ENEMY_AGGRO_RANGE_Y   = 60.0f;
constexpr float ENEMY_LUNGE_SPEED_X   = 300.0f;
constexpr float ENEMY_LUNGE_SPEED_Y   = 250.0f;
constexpr float ENEMY_ATTACK_COOLDOWN = 1.2f;
constexpr float ENEMY_ATTACK_WINDUP   = 0.30f;
constexpr float ENEMY_ATTACK_TOTAL    = ENEMY_ATTACK_WINDUP + 3 * ENEMY_ATTACK_FRAME_TIME;
constexpr float ENEMY_KNOCKBACK       = 320.0f;
constexpr float ENEMY_KNOCKBACK_POP   = 180.0f;
constexpr float ENEMY_HURT_FRICTION   = 480.0f;
constexpr float ENEMY_HURT_DURATION   = 2 * ENEMY_HURT_FRAME_TIME;
constexpr float ENEMY_POST_HURT_DELAY = 0.5f;
constexpr float PARALLAX_FACTOR = 0.2f;
constexpr char TILE_WALL   = '#';
constexpr char TILE_STONE  = 'R';
constexpr char TILE_EMPTY  = '.';
constexpr char TILE_SPAWN  = 'S';
constexpr char TILE_COIN   = 'C';
constexpr char TILE_HEALTH = 'H';
constexpr char TILE_ENERGY = 'B';
constexpr char TILE_ENEMY  = 'E';
constexpr char TILE_FLAG   = 'F';
constexpr char TILE_GOAL   = 'G';

constexpr float FLAG_DRAW_SIZE  = 64.0f;
constexpr float FLAG_FRAME_TIME = 0.15f;
constexpr float GOAL_DRAW_W     = 64.0f;
constexpr float GOAL_DRAW_H     = 96.0f;

std::string gContentRoot = "./";

std::string LocateContentRoot() {
	const std::string exeDir = GetApplicationDirectory();
	std::string candidates[5] = { "./", exeDir, exeDir + "../", exeDir + "../../", exeDir + "../../../" };
	for (const std::string& root : candidates)
		if (DirectoryExists((root + "assets").c_str())) return root;
	TraceLog(LOG_WARNING, "CONTENT: no 'assets' folder next to the working directory or the executable; using placeholder graphics");
	return "./";
}

std::string ContentPath(const char* dir, const char* name = "") {
	return gContentRoot + dir + name;
}

struct SpriteSheet {
	Texture2D tex{};
	int  frames = 1;
	bool ok = false;
	float footPadSrc = 0.0f; 
	float FrameW() const { return (float)tex.width / (float)frames; }
};

struct GameAssets {
	SpriteSheet skyBg[3];
	SpriteSheet walk[2]; 
	SpriteSheet jump[2];
	SpriteSheet attack[2];
	SpriteSheet hurt[2];
	SpriteSheet roll;
	SpriteSheet fireball;
	SpriteSheet coin, health, energy;
	SpriteSheet enemyMove, enemyAttack, enemyHurt, enemyDeath;
	SpriteSheet fxHeal, fxSparkle;
	SpriteSheet tileGrass, tileDirt, tileStoneA, tileStoneB;
	SpriteSheet flag, goal;
};

GameAssets gAssets;

SpriteSheet LoadSheet(const char* dir, const char* name, int frames) {
	SpriteSheet s;
	s.frames = frames;
	Image img = LoadImage(ContentPath(dir, name).c_str());
	if (img.data == nullptr) {
		TraceLog(LOG_WARNING, "Missing sprite: %s", name);
		return s;
	}
	Color* px = LoadImageColors(img);
	int lowestOpaque = -1;
	for (int y = img.height - 1; y >= 0 && lowestOpaque < 0; y--)
		for (int x = 0; x < img.width; x++)
			if (px[y * img.width + x].a > 8) { lowestOpaque = y; break; }
	UnloadImageColors(px);
	s.footPadSrc = (lowestOpaque >= 0) ? (float)(img.height - 1 - lowestOpaque) : 0.0f;
	s.tex = LoadTextureFromImage(img);
	UnloadImage(img);
	s.ok = (s.tex.id > 0);
	return s;
}

void LoadAssets() {
	gAssets.skyBg[0]    = LoadSheet(BG_DIR, "sky_bg_1.png", 1);
	gAssets.skyBg[1]    = LoadSheet(BG_DIR, "sky_bg_2.png", 1);
	gAssets.skyBg[2]    = LoadSheet(BG_DIR, "sky_bg_3.png", 1);
	gAssets.walk[0]     = LoadSheet(CHAR_DIR, "mage_walk_staff.png", 6);
	gAssets.walk[1]     = LoadSheet(CHAR_DIR, "mage_walk_sword.png", 6);
	gAssets.jump[0]     = LoadSheet(CHAR_DIR, "mage_jump_staff.png", 3);
	gAssets.jump[1]     = LoadSheet(CHAR_DIR, "mage_jump_sword.png", 3);
	gAssets.attack[0]   = LoadSheet(CHAR_DIR, "mage_attack_staff.png", 6);
	gAssets.attack[1]   = LoadSheet(CHAR_DIR, "mage_attack_sword.png", 6);
	gAssets.hurt[0]     = LoadSheet(CHAR_DIR, "mage_hurt_staff.png", 2);
	gAssets.hurt[1]     = LoadSheet(CHAR_DIR, "mage_hurt_sword.png", 2);
	gAssets.roll        = LoadSheet(CHAR_DIR, "mage_roll.png", 4);
	gAssets.fireball    = LoadSheet(CHAR_DIR, "fireball.png", 3);
	gAssets.coin        = LoadSheet(CHAR_DIR, "coin_spin.png", 8);
	gAssets.health      = LoadSheet(CHAR_DIR, "health_potion_spin.png", 8);
	gAssets.energy      = LoadSheet(CHAR_DIR, "energy_bottle_spin.png", 8);
	gAssets.enemyMove   = LoadSheet(CHAR_DIR, "enemy_move.png", 4);
	gAssets.enemyAttack = LoadSheet(CHAR_DIR, "enemy_attack.png", 3);
	gAssets.enemyHurt   = LoadSheet(CHAR_DIR, "enemy_hurt.png", 2);
	gAssets.enemyDeath  = LoadSheet(CHAR_DIR, "enemy_death.png", 5);
	gAssets.fxHeal      = LoadSheet(CHAR_DIR, "fx_heal_plus.png", 1);
	gAssets.fxSparkle   = LoadSheet(CHAR_DIR, "fx_coin_sparkle.png", 4);
	gAssets.tileGrass   = LoadSheet(BG_DIR, "tile_grass_top.png", 1);
	gAssets.tileDirt    = LoadSheet(BG_DIR, "tile_dirt.png", 1);
	gAssets.tileStoneA  = LoadSheet(BG_DIR, "tile_stone_a.png", 1);
	gAssets.tileStoneB  = LoadSheet(BG_DIR, "tile_stone_b.png", 1);
	gAssets.flag        = LoadSheet(CHAR_DIR, "checkpoint_flag.png", 4);
	gAssets.goal        = LoadSheet(CHAR_DIR, "goal_door.png", 1);
}

struct GameSounds {
	Sound jump, doubleJump, roll, sword, fireball, noMana;
	Sound enemyHit, enemyDie, hurt;
	Sound coin, potion, energy, checkpoint;
	Sound win, death;
	Music music;
	bool  musicOk = false;
};

GameSounds gSounds;
Sound LoadSfx(const char* name, float volume) {
	Sound s{};
	const std::string path = ContentPath(SFX_DIR, name);
	if (!FileExists(path.c_str())) {
		TraceLog(LOG_WARNING, "Missing sfx: %s", name);
		return s;
	}
	s = LoadSound(path.c_str());
	if (IsSoundValid(s)) SetSoundVolume(s, volume);
	return s;
}

void PlaySfx(const Sound& s) { if (s.frameCount > 0) PlaySound(s); }

void LoadSounds() {
	gSounds.jump       = LoadSfx("jump.wav",        0.45f);
	gSounds.doubleJump = LoadSfx("double_jump.wav", 0.45f);
	gSounds.roll       = LoadSfx("roll.wav",        0.50f);
	gSounds.sword      = LoadSfx("sword.wav",       0.50f);
	gSounds.fireball   = LoadSfx("fireball.wav",    0.45f);
	gSounds.noMana     = LoadSfx("no_mana.wav",     0.50f);
	gSounds.enemyHit   = LoadSfx("enemy_hit.wav",   0.55f);
	gSounds.enemyDie   = LoadSfx("enemy_die.wav",   0.55f);
	gSounds.hurt       = LoadSfx("hurt.wav",        0.60f);
	gSounds.coin       = LoadSfx("coin.wav",        0.40f);
	gSounds.potion     = LoadSfx("potion.wav",      0.50f);
	gSounds.energy     = LoadSfx("energy.wav",      0.50f);
	gSounds.checkpoint = LoadSfx("checkpoint.wav",  0.55f);
	gSounds.win        = LoadSfx("win.wav",         0.60f);
	gSounds.death      = LoadSfx("death.wav",       0.60f);

	const std::string musicPath = ContentPath(SFX_DIR, "music.wav");
	if (FileExists(musicPath.c_str())) {
		gSounds.music = LoadMusicStream(musicPath.c_str());
		gSounds.musicOk = IsMusicValid(gSounds.music);
		if (gSounds.musicOk) {
			gSounds.music.looping = true;
			SetMusicVolume(gSounds.music, 0.35f);
			PlayMusicStream(gSounds.music);
		}
	}
}

void UnloadSounds() {
	Sound* all[] = { &gSounds.jump, &gSounds.doubleJump, &gSounds.roll, &gSounds.sword,
	                 &gSounds.fireball, &gSounds.noMana, &gSounds.enemyHit, &gSounds.enemyDie,
	                 &gSounds.hurt, &gSounds.coin, &gSounds.potion, &gSounds.energy,
	                 &gSounds.checkpoint, &gSounds.win, &gSounds.death };
	for (Sound* s : all) if (s->frameCount > 0) UnloadSound(*s);
	if (gSounds.musicOk) UnloadMusicStream(gSounds.music);
}

void UnloadSheet(SpriteSheet& s) { if (s.ok) UnloadTexture(s.tex); s.ok = false; }

void UnloadAssets() {
	for (int i = 0; i < 3; i++) UnloadSheet(gAssets.skyBg[i]);
	for (int i = 0; i < 2; i++) {
		UnloadSheet(gAssets.walk[i]);  UnloadSheet(gAssets.jump[i]);
		UnloadSheet(gAssets.attack[i]); UnloadSheet(gAssets.hurt[i]);
	}
	UnloadSheet(gAssets.roll);      UnloadSheet(gAssets.fireball);
	UnloadSheet(gAssets.coin);      UnloadSheet(gAssets.health);   UnloadSheet(gAssets.energy);
	UnloadSheet(gAssets.enemyMove); UnloadSheet(gAssets.enemyAttack);
	UnloadSheet(gAssets.enemyHurt); UnloadSheet(gAssets.enemyDeath);
	UnloadSheet(gAssets.fxHeal);    UnloadSheet(gAssets.fxSparkle);
	UnloadSheet(gAssets.tileGrass); UnloadSheet(gAssets.tileDirt);
	UnloadSheet(gAssets.tileStoneA); UnloadSheet(gAssets.tileStoneB);
	UnloadSheet(gAssets.flag);      UnloadSheet(gAssets.goal);
}

void DrawSheetFrame(const SpriteSheet& s, int frame, float centerX, float bottomY,
                    float destW, float destH, int facing, Color tint) {
	if (!s.ok) return;
	if (frame < 0) frame = 0;
	if (frame >= s.frames) frame = s.frames - 1;
	float fw = s.FrameW();
	Rectangle src = { fw * frame, 0.0f, (facing >= 0 ? fw : -fw), (float)s.tex.height };
	float padDst = s.footPadSrc * (destH / (float)s.tex.height);
	Rectangle dst = { centerX - destW * 0.5f, bottomY - destH + padDst, destW, destH };
	DrawTexturePro(s.tex, src, dst, { 0.0f, 0.0f }, 0.0f, tint);
}

struct Player {
	Vector2 position;
	Vector2 speed;
	Vector2 size;
	Color   color;
	bool    onGround;
	int     jumpsleft;
	Vector2 checkpoint;
	int     facing;
	float   coyote;
	float   jumpBuffer;
	float   jumpLockout;
	bool    jumpCutable;
	float   landTimer;
	int     hp;
	int     coins;
	Weapon  weapon;
	float   boostTimer;
	float   hurtTimer;
	float   invincibleTimer;
	float   attackTimer;
	float   attackCooldown;
	bool    attackHit;
	bool    fireballShot;
	bool    attackAir;
	float   rollTimer;
	int     mana;
	float   manaRegenTimer;
	bool    dodging;
	float   rollCooldown;
	int     deaths;
	float   playTime;
	int     totalCoins;
	int     checkpointTx;
	int     checkpointTy;
	AnimState anim;
	float   animTimer;
	int     animFrame;
};

enum class PickupType { Coin, Health, Energy };

struct Pickup {
	PickupType type;
	Vector2 center;
	float   animTimer;
	bool    dormant;
	float   respawnTimer;
	bool    remove;
};

enum class EnemyState { Move, Attack, Hurt, Dying };

struct Enemy {
	Vector2 pos;
	Vector2 vel;
	Vector2 size;
	int     facing;
	EnemyState state;
	float   stateTimer;
	float   animTimer;
	int     animFrame;
	float   hopTimer;
	float   attackCooldown;
	bool    lunged;
	int     hp;
	bool    onGround;
	bool    remove;
};

struct Fireball {
	Vector2 pos;
	float   vx;
	float   life;
	float   animTimer;
	bool    remove;
};

enum class FxType { HealPlus, CoinSparkle };

struct Fx {
	FxType  type;
	Vector2 pos;
	Vector2 vel;
	float   life;
	float   maxLife;
};

std::vector<std::string> gMap;
int MAP_W = 0;
int MAP_H = 0;

constexpr const char* DEFAULT_LEVEL[] = {
	"##################################################################################################################################################################################################################",
	"#................................................................................................................................................................................................................#",
	"#................................................................................................................................................................................................................#",
	"#.........................C......................................................................................................................................................................................#",
	"#........................E..........................................................................................................E..E.......................................................#.................#",
	"#.......####............####.......................####...........................................................................#######.................................................###..#.................#",
	"#......................................C..........................................................................................#######.............................E........................#.................#",
	"#.............C...................................................................................................................#######..........................#####.......................#.................#",
	"#..............E..................................E....................................................EE.......E.................#######............................................E.........#.................#",
	"#............####...........E..................####....................................#######.........##....#######..............#######........................................#######.......#.................#",
	"#..........................####.................................................###....................##.........................#######......................................................#.......C.........#",
	"#..S.....................................................E.............................................##.........................#######......................................................#.................#",
	"#.....................................................######.....................H.....................##.........................#######......................................................#.....C.....C.....#",
	"#........E...........E...EE...F....C..E..............E.........F...B.......................E.....E.E...##.....H......F.....E......#######........E......E..F.......E.........E.............F...#...E....G........#",
	"###########.......########################......###########################.............##########################################################################################################################",
	"############......########################......###########################.............##########################################################################################################################",
};

char GetTile(int tx, int ty) {
	if (ty < 0 || ty >= MAP_H || tx < 0 || tx >= MAP_W) return TILE_EMPTY;
	return gMap[ty][tx];
}

void SetTile(int tx, int ty, char c) {
	if (ty < 0 || ty >= MAP_H || tx < 0 || tx >= MAP_W) return;
	gMap[ty][tx] = c;
}

bool IsSolid(int tx, int ty) {
	if (ty < 0 || ty >= MAP_H) return false;
	if (tx < 0 || tx >= MAP_W) return true;
	char c = gMap[ty][tx];
	return c == TILE_WALL || c == TILE_STONE;
}

Vector2 FindSpawn() {
	for (int ty = 0; ty < MAP_H; ty++)
		for (int tx = 0; tx < MAP_W; tx++)
			if (gMap[ty][tx] == TILE_SPAWN)
				return { tx * (float)TILE + (TILE - PLAYER_SIZE) / 2.0f, ty * (float)TILE };
	return { 100.0f, 100.0f };
}

constexpr int DEFAULT_LEVEL_H = (int)(sizeof(DEFAULT_LEVEL) / sizeof(DEFAULT_LEVEL[0]));

constexpr int ConstStrLen(const char* s) {
	int n = 0;
	while (s[n] != '\0') n++;
	return n;
}

constexpr int DefaultLevelWidth() {
	int w = 0;
	for (int i = 0; i < DEFAULT_LEVEL_H; i++) {
		int len = ConstStrLen(DEFAULT_LEVEL[i]);
		if (len > w) w = len;
	}
	return w;
}
constexpr int DEFAULT_LEVEL_W = DefaultLevelWidth();

void LoadDefaultMap() {
	gMap.clear();
	for (int i = 0; i < DEFAULT_LEVEL_H; i++) {
		std::string row = DEFAULT_LEVEL[i];
		row.resize(DEFAULT_LEVEL_W, TILE_EMPTY);
		gMap.push_back(row);
	}
	MAP_H = DEFAULT_LEVEL_H;
	MAP_W = DEFAULT_LEVEL_W;
}

void SaveMap() {
	std::ofstream f(ContentPath(LEVEL_FILE));
	if (!f) return;
	for (const auto& row : gMap) f << row << "\n";
}

bool LoadMap() {
	std::ifstream f(ContentPath(LEVEL_FILE));
	if (!f) return false;
	std::vector<std::string> rows;
	std::string line;
	int w = 0;
	while (std::getline(f, line)) {
		while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) line.pop_back();
		if ((int)line.size() > w) w = (int)line.size();
		rows.push_back(line);
	}
	if (rows.empty()) return false;
	for (auto& r : rows) r.resize(w, TILE_EMPTY);
	gMap = rows;
	MAP_H = (int)gMap.size();
	MAP_W = w;
	return true;
}

void AddColumn()    { for (auto& r : gMap) r.push_back(TILE_EMPTY); MAP_W++; }
void RemoveColumn() { if (MAP_W > 1) { for (auto& r : gMap) r.pop_back(); MAP_W--; } }
void AddRow()       { gMap.push_back(std::string(MAP_W, TILE_EMPTY)); MAP_H++; }
void RemoveRow()    { if (MAP_H > 1) { gMap.pop_back(); MAP_H--; } }

float MoveToward(float current, float target, float maxDelta) {
	if (fabsf(target - current) <= maxDelta) return target;
	return current + (target > current ? maxDelta : -maxDelta);
}

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

void SpawnEntities(std::vector<Pickup>& pickups, std::vector<Enemy>& enemies) {
	pickups.clear();
	enemies.clear();
	for (int ty = 0; ty < MAP_H; ty++) {
		for (int tx = 0; tx < MAP_W; tx++) {
			char c = gMap[ty][tx];
			Vector2 center = { tx * (float)TILE + TILE * 0.5f, ty * (float)TILE + TILE * 0.5f };
			if (c == TILE_COIN || c == TILE_HEALTH || c == TILE_ENERGY) {
				Pickup pk{};
				pk.type = (c == TILE_COIN) ? PickupType::Coin
				        : (c == TILE_HEALTH) ? PickupType::Health : PickupType::Energy;
				pk.center = center;
				pk.animTimer = (float)((tx + ty) % 4) * PICKUP_FRAME_TIME;
				pickups.push_back(pk);
			}
			else if (c == TILE_ENEMY) {
				Enemy e{};
				e.size = { ENEMY_W, ENEMY_H };
				e.pos = { tx * (float)TILE + (TILE - ENEMY_W) / 2.0f, ty * (float)TILE + TILE - ENEMY_H };
				e.facing = -1;
				e.state = EnemyState::Move;
				e.hp = ENEMY_HP;
				e.hopTimer = (float)(tx % 3) * 0.3f;
				enemies.push_back(e);
			}
		}
	}
}

void RespawnEnemies(std::vector<Enemy>& enemies) {
	enemies.clear();
	for (int ty = 0; ty < MAP_H; ty++) {
		for (int tx = 0; tx < MAP_W; tx++) {
			if (gMap[ty][tx] != TILE_ENEMY) continue;
			Enemy e{};
			e.size = { ENEMY_W, ENEMY_H };
			e.pos = { tx * (float)TILE + (TILE - ENEMY_W) / 2.0f, ty * (float)TILE + TILE - ENEMY_H };
			e.facing = -1;
			e.state = EnemyState::Move;
			e.hp = ENEMY_HP;
			e.hopTimer = (float)(tx % 3) * 0.3f;
			enemies.push_back(e);
		}
	}
}

int CountCoinTiles() {
	int n = 0;
	for (int ty = 0; ty < MAP_H; ty++)
		for (int tx = 0; tx < MAP_W; tx++)
			if (gMap[ty][tx] == TILE_COIN) n++;
	return n;
}

Vector2 CheckpointPos(const Player& p) {
	if (p.checkpointTx >= 0)
		return { p.checkpointTx * (float)TILE + (TILE - PLAYER_SIZE) / 2.0f,
		         p.checkpointTy * (float)TILE + (TILE - PLAYER_SIZE) };
	return FindSpawn();
}

void ResetGame(Player& player, std::vector<Pickup>& pickups, std::vector<Enemy>& enemies,
               std::vector<Fireball>& fireballs, std::vector<Fx>& fxs) {
	Vector2 spawn = FindSpawn();
	player.position = spawn;
	player.speed = { 0.0f, 0.0f };
	player.size = { PLAYER_SIZE, PLAYER_SIZE };
	player.color = SKYBLUE;
	player.onGround = false;
	player.jumpsleft = 2;
	player.checkpoint = spawn;
	player.facing = 1;
	player.coyote = 0.0f;
	player.jumpBuffer = 0.0f;
	player.jumpLockout = 0.0f;
	player.jumpCutable = false;
	player.landTimer = 0.0f;
	player.hp = PLAYER_MAX_HP;
	player.coins = 0;
	player.weapon = Weapon::Staff;
	player.boostTimer = 0.0f;
	player.hurtTimer = 0.0f;
	player.invincibleTimer = 0.0f;
	player.attackTimer = 0.0f;
	player.attackCooldown = 0.0f;
	player.attackHit = false;
	player.fireballShot = false;
	player.attackAir = false;
	player.rollTimer = 0.0f;
	player.mana = PLAYER_MAX_MANA;
	player.manaRegenTimer = 0.0f;
	player.dodging = false;
	player.rollCooldown = 0.0f;
	player.deaths = 0;
	player.playTime = 0.0f;
	player.checkpointTx = -1;
	player.checkpointTy = -1;
	player.anim = AnimState::Walk;
	player.animTimer = 0.0f;
	player.animFrame = 0;
	SpawnEntities(pickups, enemies);
	player.totalCoins = CountCoinTiles();
	fireballs.clear();
	fxs.clear();
}

void RefreshFromMap(Player& player, std::vector<Pickup>& pickups, std::vector<Enemy>& enemies,
                    std::vector<Fireball>& fireballs, std::vector<Fx>& fxs) {
	if (player.checkpointTx >= 0 && GetTile(player.checkpointTx, player.checkpointTy) != TILE_FLAG) {
		player.checkpointTx = -1;
		player.checkpointTy = -1;
	}
	player.checkpoint = CheckpointPos(player);
	player.position = player.checkpoint;
	player.speed = { 0.0f, 0.0f };
	player.hurtTimer = 0.0f;
	player.attackTimer = 0.0f;
	player.rollTimer = 0.0f;
	player.dodging = false;
	player.invincibleTimer = INVINCIBLE_TIME;
	SpawnEntities(pickups, enemies);
	player.totalCoins = player.coins + CountCoinTiles();
	fireballs.clear();
	fxs.clear();
}

void ResolveAxisBody(Vector2& pos, Vector2& vel, Vector2 size, bool horizontal,
                     bool& onGround, bool& landed) {
	int left   = (int)(pos.x / TILE);
	int right  = (int)((pos.x + size.x) / TILE);
	int top    = (int)(pos.y / TILE);
	int bottom = (int)((pos.y + size.y) / TILE);

	for (int ty = top; ty <= bottom; ty++) {
		for (int tx = left; tx <= right; tx++) {
			if (!IsSolid(tx, ty)) continue;

			Rectangle tileRect = { (float)tx * TILE, (float)ty * TILE, (float)TILE, (float)TILE };
			Rectangle bodyRect = { pos.x, pos.y, size.x, size.y };
			if (!CheckCollisionRecs(bodyRect, tileRect)) continue;

			if (horizontal) {
				if (vel.x > 0)      pos.x = tx * TILE - size.x;
				else if (vel.x < 0) pos.x = (tx + 1) * TILE;
				vel.x = 0;
			}
			else {
				if (vel.y > 0) {
					pos.y = ty * TILE - size.y;
					if (!onGround) landed = true;
					onGround = true;
				}
				else if (vel.y < 0) pos.y = (ty + 1) * TILE;
				vel.y = 0;
			}
		}
	}
}

bool MoveAndCollideBody(Vector2& pos, Vector2& vel, Vector2 size, bool& onGround, float dt) {
	bool landed = false;
	pos.x += vel.x * dt;
	ResolveAxisBody(pos, vel, size, true, onGround, landed);
	pos.y += vel.y * dt;
	onGround = false;
	ResolveAxisBody(pos, vel, size, false, onGround, landed);
	return landed;
}

void MoveAndCollide(Player& p, float dt) {
	if (MoveAndCollideBody(p.position, p.speed, p.size, p.onGround, dt))
		p.landTimer = LAND_DIP_TIME;
}

void SpawnHealFx(std::vector<Fx>& fxs, Vector2 center) {
	for (int i = 0; i < 4; i++) {
		Fx f{};
		f.type = FxType::HealPlus;
		f.pos = { center.x + (i - 1.5f) * 12.0f, center.y - i * 5.0f };
		f.vel = { (float)GetRandomValue(-15, 15), -45.0f };
		f.life = f.maxLife = HEAL_FX_LIFETIME;
		fxs.push_back(f);
	}
}

void SpawnSparkleFx(std::vector<Fx>& fxs, Vector2 center) {
	Fx f{};
	f.type = FxType::CoinSparkle;
	f.pos = center;
	f.vel = { 0.0f, 0.0f };
	f.life = f.maxLife = SPARKLE_FX_LIFETIME;
	fxs.push_back(f);
}

void SpawnFireball(std::vector<Fireball>& fireballs, const Player& p) {
	Fireball fb{};
	fb.pos = { p.position.x + p.size.x * 0.5f + p.facing * (p.size.x * 0.5f + 12.0f),
	           p.position.y + p.size.y * 0.45f };
	fb.vx = p.facing * FIREBALL_SPEED;
	fb.life = FIREBALL_LIFETIME;
	fireballs.push_back(fb);
}

void UpdatePlayer(Player& player, std::vector<Fireball>& fireballs, float dt) {
	player.hurtTimer       -= dt;
	player.invincibleTimer -= dt;
	player.boostTimer      -= dt;
	player.attackCooldown  -= dt;
	player.rollCooldown    -= dt;
	if (player.attackTimer > 0.0f) player.attackTimer -= dt;
	if (player.rollTimer   > 0.0f) player.rollTimer   -= dt;
	if (player.rollTimer  <= 0.0f) player.dodging = false;

	if (player.mana < PLAYER_MAX_MANA) {
		player.manaRegenTimer += dt;
		if (player.manaRegenTimer >= MANA_REGEN_TIME) {
			player.mana++;
			player.manaRegenTimer = 0.0f;
		}
	} else {
		player.manaRegenTimer = 0.0f;
	}

	bool controlLocked = (player.hurtTimer > 0.0f);
	float dir = 0.0f;
	bool jumpPressed = false, jumpReleased = false, attackPressed = false, rollPressed = false;

	if (!controlLocked) {
		if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  dir -= 1.0f;
		if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) dir += 1.0f;
		jumpPressed   = IsKeyPressed(KEY_SPACE);
		jumpReleased  = IsKeyReleased(KEY_SPACE);
		attackPressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_J);
		rollPressed   = IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_K);
		bool switchWeapon = IsKeyPressed(KEY_Q);

		if (IsGamepadAvailable(0)) {
			float stickX = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
			const float DEADZONE = 0.2f;
			if (stickX < -DEADZONE) dir -= 1.0f;
			if (stickX > DEADZONE)  dir += 1.0f;
			if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))  jumpPressed = true;
			if (IsGamepadButtonReleased(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) jumpReleased = true;
			if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT))  attackPressed = true;
			if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_UP))    switchWeapon = true;
			if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) rollPressed = true;
		}

		if (switchWeapon && player.attackTimer <= 0.0f)
			player.weapon = (player.weapon == Weapon::Staff) ? Weapon::Sword : Weapon::Staff;
	}

	if (rollPressed && player.onGround && player.rollTimer <= 0.0f
	    && player.rollCooldown <= 0.0f && player.attackTimer <= 0.0f) {
		player.rollTimer = ROLL_DURATION;
		player.rollCooldown = ROLL_DURATION + ROLL_COOLDOWN;
		player.dodging = true;
		PlaySfx(gSounds.roll);
	}

	if (player.rollTimer <= 0.0f) {
		if (dir < 0.0f) player.facing = -1;
		else if (dir > 0.0f) player.facing = 1;
	}

	float moveSpeed = MOVE_SPEED * (player.boostTimer > 0.0f ? BOOST_MULT : 1.0f);
	float target = dir * moveSpeed;
	float rate;
	if (dir != 0.0f) rate = player.onGround ? ACCELERATION : AIR_ACCEL;
	else             rate = player.onGround ? FRICTION     : AIR_FRICTION;
	player.speed.x = MoveToward(player.speed.x, target, rate * dt);

	if (player.rollTimer > 0.0f)
		player.speed.x = player.facing * moveSpeed * (player.dodging ? ROLL_SPEED_MULT : 1.0f);

	if (player.onGround) player.coyote = COYOTE_TIME;
	else                 player.coyote -= dt;

	if (jumpPressed) player.jumpBuffer = JUMP_BUFFER;
	else             player.jumpBuffer -= dt;

	player.jumpLockout -= dt;
	bool canGroundJump = (player.coyote > 0.0f);
	bool canAirJump = (player.jumpsleft > 0 && player.jumpLockout <= 0.0f);
	if (player.jumpBuffer > 0.0f && (canGroundJump || canAirJump)) {
		player.speed.y = -JUMP_SPEED;
		if (canGroundJump) {
			player.jumpsleft = 1;
			player.jumpCutable = true;
			PlaySfx(gSounds.jump);
		} else {
			player.jumpsleft--;
			player.rollTimer = ROLL_DURATION;
			player.dodging = false;
			player.attackTimer = 0.0f;
			player.jumpCutable = false;
			PlaySfx(gSounds.doubleJump);
		}
		player.coyote = 0.0f;
		player.jumpBuffer = 0.0f;
		player.jumpLockout = AIR_JUMP_LOCKOUT;
	}

	if (jumpReleased && player.speed.y < 0.0f && player.jumpCutable)
		player.speed.y *= LOW_JUMP_CUT;

	if (attackPressed && player.attackCooldown <= 0.0f && player.attackTimer <= 0.0f
	    && player.rollTimer <= 0.0f) {
		if (player.weapon == Weapon::Staff && player.mana <= 0) {
			PlaySfx(gSounds.noMana);
		} else {
			player.attackTimer = ATTACK_DURATION;
			player.attackCooldown = ATTACK_COOLDOWN;
			player.attackHit = false;
			player.fireballShot = false;
			player.attackAir = !player.onGround;
			if (player.weapon == Weapon::Staff) player.mana--;
			else PlaySfx(gSounds.sword);
		}
	}
	if (player.attackTimer > 0.0f && player.weapon == Weapon::Staff && !player.fireballShot
	    && (ATTACK_DURATION - player.attackTimer) >= ATTACK_FRAME_TIME) {
		SpawnFireball(fireballs, player);
		player.fireballShot = true;
		PlaySfx(gSounds.fireball);
	}

	float g = (player.speed.y > 0.0f) ? GRAVITY * FALL_MULT : GRAVITY;
	player.speed.y += g * dt;
	if (player.attackAir && player.attackTimer > ATTACK_DURATION - ATTACK_HOVER_TIME
	    && player.speed.y > 0.0f)
		player.speed.y *= ATTACK_HOVER_DAMP;
	if (player.speed.y > MAX_FALL) player.speed.y = MAX_FALL;

	if (player.landTimer > 0.0f) player.landTimer -= dt;
}

void UpdatePlayerAnim(Player& p, float dt) {
	AnimState next;
	if (p.hurtTimer > 0.0f)        next = AnimState::Hurt;
	else if (p.attackTimer > 0.0f) next = AnimState::Attack;
	else if (p.rollTimer > 0.0f)   next = AnimState::Roll;
	else if (!p.onGround) {
		if (p.speed.y < -APEX_THRESHOLD)     next = AnimState::JumpRise;
		else if (p.speed.y > APEX_THRESHOLD) next = AnimState::JumpFall;
		else                                 next = AnimState::JumpApex;
	}
	else next = AnimState::Walk;

	if (next != p.anim) { p.anim = next; p.animTimer = 0.0f; p.animFrame = 0; }
	p.animTimer += dt;

	switch (p.anim) {
	case AnimState::Walk:
		if (fabsf(p.speed.x) > WALK_ANIM_MIN_SPEED)
			p.animFrame = (int)(p.animTimer / WALK_FRAME_TIME) % 6;
		else p.animFrame = 0;
		break;
	case AnimState::JumpRise: p.animFrame = 0; break;
	case AnimState::JumpApex: p.animFrame = 1; break;
	case AnimState::JumpFall: p.animFrame = 2; break;
	case AnimState::Roll:
		p.animFrame = (int)(p.animTimer / ROLL_FRAME_TIME);
		if (p.animFrame > 3) p.animFrame = 3;
		break;
	case AnimState::Attack: {
		int local = (int)((ATTACK_DURATION - p.attackTimer) / ATTACK_FRAME_TIME);
		if (local > 2) local = 2;
		p.animFrame = local + (p.onGround ? 0 : 3);
		break;
	}
	case AnimState::Hurt:
		p.animFrame = (p.animTimer < HURT_FRAME_TIME) ? 0 : 1;
		break;
	}
}

bool HasLineOfSight(Vector2 a, Vector2 b) {
	float dx = b.x - a.x, dy = b.y - a.y;
	float dist = sqrtf(dx * dx + dy * dy);
	int steps = (int)(dist / (TILE * 0.5f)) + 1;
	for (int i = 1; i < steps; i++) {
		float t = (float)i / (float)steps;
		int tx = (int)((a.x + dx * t) / TILE);
		int ty = (int)((a.y + dy * t) / TILE);
		if (IsSolid(tx, ty)) return false;
	}
	return true;
}

void UpdateEnemy(Enemy& e, const Player& player, float dt) {
	e.animTimer += dt;

	if (e.state == EnemyState::Dying) {
		e.vel.x = 0.0f;
		e.animFrame = (int)(e.animTimer / ENEMY_DEATH_FRAME_TIME);
		if (e.animTimer >= 5 * ENEMY_DEATH_FRAME_TIME) e.remove = true;
		e.vel.y += GRAVITY * dt;
		if (e.vel.y > MAX_FALL) e.vel.y = MAX_FALL;
		MoveAndCollideBody(e.pos, e.vel, e.size, e.onGround, dt);
		return;
	}

	if (e.state == EnemyState::Hurt) {
		e.stateTimer -= dt;
		e.animFrame = (e.animTimer < ENEMY_HURT_FRAME_TIME) ? 0 : 1;
		e.vel.x = MoveToward(e.vel.x, 0.0f, ENEMY_HURT_FRICTION * dt);
		if (e.stateTimer <= 0.0f) {
			e.state = EnemyState::Move;
			e.animTimer = 0.0f;
			if (e.attackCooldown < ENEMY_POST_HURT_DELAY) e.attackCooldown = ENEMY_POST_HURT_DELAY;
		}
	}
	else if (e.state == EnemyState::Attack) {
		e.stateTimer -= dt;
		if (e.animTimer < ENEMY_ATTACK_WINDUP) {
			e.animFrame = 0;
			e.vel.x = MoveToward(e.vel.x, 0.0f, FRICTION * dt);
		} else {
			if (!e.lunged) {
				e.vel.x = e.facing * ENEMY_LUNGE_SPEED_X;
				e.vel.y = -ENEMY_LUNGE_SPEED_Y;
				e.lunged = true;
			}
			e.animFrame = (e.animTimer < ENEMY_ATTACK_WINDUP + ENEMY_ATTACK_FRAME_TIME) ? 1 : 2;
		}
		if (e.stateTimer <= 0.0f) {
			e.state = EnemyState::Move;
			e.animTimer = 0.0f;
			e.attackCooldown = ENEMY_ATTACK_COOLDOWN;
			e.hopTimer = ENEMY_HOP_INTERVAL * 0.5f;
		}
	}
	else {
		e.attackCooldown -= dt;
		e.animFrame = (int)(e.animTimer / ENEMY_MOVE_FRAME_TIME) % 4;

		if (e.onGround) {
			e.vel.x = MoveToward(e.vel.x, 0.0f, FRICTION * dt);
			e.hopTimer -= dt;

			float pdx = (player.position.x + player.size.x * 0.5f) - (e.pos.x + e.size.x * 0.5f);
			float pdy = (player.position.y + player.size.y * 0.5f) - (e.pos.y + e.size.y * 0.5f);

			Vector2 eCenter = { e.pos.x + e.size.x * 0.5f, e.pos.y + e.size.y * 0.5f };
			Vector2 pCenter = { player.position.x + player.size.x * 0.5f,
			                    player.position.y + player.size.y * 0.5f };
			if (player.hp > 0 && e.attackCooldown <= 0.0f
			    && fabsf(pdx) < ENEMY_AGGRO_RANGE_X && fabsf(pdy) < ENEMY_AGGRO_RANGE_Y
			    && HasLineOfSight(eCenter, pCenter)) {
				e.state = EnemyState::Attack;
				e.animTimer = 0.0f;
				e.stateTimer = ENEMY_ATTACK_TOTAL;
				e.facing = (pdx >= 0.0f) ? 1 : -1;
				e.lunged = false;
			}
			else if (e.hopTimer <= 0.0f) {
				int aheadX = (int)((e.pos.x + (e.facing > 0 ? e.size.x + 4.0f : -4.0f)) / TILE);
				int bodyY  = (int)((e.pos.y + e.size.y * 0.5f) / TILE);
				int footY  = (int)((e.pos.y + e.size.y + 4.0f) / TILE);
				if (IsSolid(aheadX, bodyY) || !IsSolid(aheadX, footY)) e.facing = -e.facing;
				e.vel.x = e.facing * ENEMY_HOP_SPEED_X;
				e.vel.y = -ENEMY_HOP_SPEED_Y;
				e.hopTimer = ENEMY_HOP_INTERVAL;
			}
		}
	}

	e.vel.y += GRAVITY * dt;
	if (e.vel.y > MAX_FALL) e.vel.y = MAX_FALL;
	MoveAndCollideBody(e.pos, e.vel, e.size, e.onGround, dt);
}

void DamageEnemy(Enemy& e, int dmg, int fromDir) {
	e.hp -= dmg;
	e.animTimer = 0.0f;
	e.animFrame = 0;
	if (e.hp <= 0) {
		e.state = EnemyState::Dying;
		e.vel = { 0.0f, 0.0f };
		PlaySfx(gSounds.enemyDie);
	} else {
		e.state = EnemyState::Hurt;
		e.stateTimer = ENEMY_HURT_DURATION;
		e.vel.x = fromDir * ENEMY_KNOCKBACK;
		e.vel.y = -ENEMY_KNOCKBACK_POP;
		PlaySfx(gSounds.enemyHit);
	}
}

void UpdatePickups(std::vector<Pickup>& pickups, float dt) {
	for (auto& pk : pickups) {
		pk.animTimer += dt;
		if (pk.dormant) {
			pk.respawnTimer -= dt;
			if (pk.respawnTimer <= 0.0f) pk.dormant = false;
		}
	}
}

void UpdateFireballs(std::vector<Fireball>& fireballs, float dt) {
	for (auto& fb : fireballs) {
		fb.pos.x += fb.vx * dt;
		fb.animTimer += dt;
		fb.life -= dt;
		if (fb.life <= 0.0f) fb.remove = true;
		if (IsSolid((int)(fb.pos.x / TILE), (int)(fb.pos.y / TILE))) fb.remove = true;
	}
}

void UpdateFxs(std::vector<Fx>& fxs, float dt) {
	for (auto& f : fxs) {
		f.pos.x += f.vel.x * dt;
		f.pos.y += f.vel.y * dt;
		f.life -= dt;
	}
}

void HandleInteractions(Player& p, std::vector<Pickup>& pickups, std::vector<Enemy>& enemies,
                        std::vector<Fireball>& fireballs, std::vector<Fx>& fxs) {
	Rectangle pr = { p.position.x, p.position.y, p.size.x, p.size.y };

	for (auto& pk : pickups) {
		if (pk.remove || pk.dormant) continue;
		Rectangle r = { pk.center.x - PICKUP_HITBOX * 0.5f, pk.center.y - PICKUP_HITBOX * 0.5f,
		                PICKUP_HITBOX, PICKUP_HITBOX };
		if (!CheckCollisionRecs(pr, r)) continue;

		switch (pk.type) {
		case PickupType::Coin:
			p.coins++;
			pk.remove = true;
			SpawnSparkleFx(fxs, pk.center);
			PlaySfx(gSounds.coin);
			break;
		case PickupType::Health:
			if (p.hp >= PLAYER_MAX_HP) break;
			p.hp++;
			pk.dormant = true;
			pk.respawnTimer = HEALTH_RESPAWN_TIME;
			SpawnHealFx(fxs, { pr.x + pr.width * 0.5f, pr.y + pr.height * 0.5f });
			PlaySfx(gSounds.potion);
			break;
		case PickupType::Energy:
			p.boostTimer = BOOST_DURATION;
			p.mana = PLAYER_MAX_MANA;
			p.manaRegenTimer = 0.0f;
			pk.remove = true;
			PlaySfx(gSounds.energy);
			break;
		}
	}

	if (p.attackTimer > 0.0f && p.weapon == Weapon::Sword && !p.attackHit) {
		float elapsed = ATTACK_DURATION - p.attackTimer;
		int frame = (int)(elapsed / ATTACK_FRAME_TIME);
		if (frame == 1) {
			Rectangle hit = { p.facing > 0 ? pr.x + pr.width : pr.x - SWORD_RANGE,
			                  pr.y - 6.0f, SWORD_RANGE, pr.height + 12.0f };
			for (auto& e : enemies) {
				if (e.remove || e.state == EnemyState::Dying) continue;
				Rectangle er = { e.pos.x, e.pos.y, e.size.x, e.size.y };
				if (CheckCollisionRecs(hit, er)) {
					DamageEnemy(e, SWORD_DAMAGE, p.facing);
					p.attackHit = true;
				}
			}
		}
	}

	for (auto& fb : fireballs) {
		if (fb.remove) continue;
		Rectangle fr = { fb.pos.x - FIREBALL_HITBOX * 0.5f, fb.pos.y - FIREBALL_HITBOX * 0.5f,
		                 FIREBALL_HITBOX, FIREBALL_HITBOX };
		for (auto& e : enemies) {
			if (e.remove || e.state == EnemyState::Dying) continue;
			Rectangle er = { e.pos.x, e.pos.y, e.size.x, e.size.y };
			if (CheckCollisionRecs(fr, er)) {
				DamageEnemy(e, FIREBALL_DAMAGE, (fb.vx > 0.0f) ? 1 : -1);
				fb.remove = true;
				break;
			}
		}
	}

	if (p.invincibleTimer <= 0.0f && p.hp > 0 && !p.dodging) {
		for (const auto& e : enemies) {
			if (e.remove || e.state == EnemyState::Dying || e.state == EnemyState::Hurt) continue;
			Rectangle er = { e.pos.x, e.pos.y, e.size.x, e.size.y };
			if (!CheckCollisionRecs(pr, er)) continue;

			p.hp--;
			p.hurtTimer = HURT_DURATION;
			p.invincibleTimer = INVINCIBLE_TIME;
			p.attackTimer = 0.0f;
			p.rollTimer = 0.0f;
			p.dodging = false;
			int away = (pr.x + pr.width * 0.5f < e.pos.x + e.size.x * 0.5f) ? -1 : 1;
			p.speed.x = away * KNOCKBACK_X;
			p.speed.y = -KNOCKBACK_Y;
			PlaySfx(gSounds.hurt);
			break;
		}
	}
}

bool UpdateTileTriggers(Player& p, std::vector<Fx>& fxs) {
	int left   = (int)(p.position.x / TILE);
	int right  = (int)((p.position.x + p.size.x) / TILE);
	int top    = (int)(p.position.y / TILE);
	int bottom = (int)((p.position.y + p.size.y) / TILE);

	for (int ty = top; ty <= bottom; ty++) {
		for (int tx = left; tx <= right; tx++) {
			char c = GetTile(tx, ty);
			if (c == TILE_GOAL) return true;
			if (c == TILE_FLAG && (tx != p.checkpointTx || ty != p.checkpointTy)) {
				p.checkpointTx = tx;
				p.checkpointTy = ty;
				p.checkpoint = CheckpointPos(p);
				SpawnHealFx(fxs, { tx * (float)TILE + TILE * 0.5f, ty * (float)TILE + TILE * 0.5f });
				PlaySfx(gSounds.checkpoint);
			}
		}
	}
	return false;
}

void CleanupEntities(std::vector<Pickup>& pickups, std::vector<Enemy>& enemies,
                     std::vector<Fireball>& fireballs, std::vector<Fx>& fxs) {
	pickups.erase(std::remove_if(pickups.begin(), pickups.end(),
		[](const Pickup& p) { return p.remove; }), pickups.end());
	enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
		[](const Enemy& e) { return e.remove; }), enemies.end());
	fireballs.erase(std::remove_if(fireballs.begin(), fireballs.end(),
		[](const Fireball& f) { return f.remove; }), fireballs.end());
	fxs.erase(std::remove_if(fxs.begin(), fxs.end(),
		[](const Fx& f) { return f.life <= 0.0f; }), fxs.end());
}

void UpdateCamera(Camera2D& cam, Player& p) {
	cam.zoom = 1.0f;
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

void DrawParallaxBackground(const Camera2D& cam) {
	if (!gAssets.skyBg[0].ok) {
		DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SKYBLUE);
		return;
	}
	float texW = (float)gAssets.skyBg[0].tex.width;
	float texH = (float)gAssets.skyBg[0].tex.height;
	float scale = (float)SCREEN_HEIGHT / texH;
	float w = texW * scale;

	float scrolled = (cam.target.x - SCREEN_WIDTH * 0.5f) * PARALLAX_FACTOR;
	int first = (int)floorf(scrolled / w);
	float startX = (float)first * w - scrolled;

	for (int i = 0; ; i++) {
		float x = startX + i * w;
		if (x >= SCREEN_WIDTH) break;
		const SpriteSheet& s = gAssets.skyBg[((first + i) % 3 + 3) % 3];
		DrawTexturePro(s.tex, { 0, 0, texW, texH },
		               { x, 0, w + 1.0f, (float)SCREEN_HEIGHT }, { 0, 0 }, 0.0f, WHITE);
	}
}

void DrawTileTexture(const SpriteSheet& s, int tx, int ty) {
	Rectangle src = { 0.0f, 0.0f, (float)s.tex.width, (float)s.tex.height };
	Rectangle dst = { (float)tx * TILE, (float)ty * TILE, (float)TILE, (float)TILE };
	DrawTexturePro(s.tex, src, dst, { 0.0f, 0.0f }, 0.0f, WHITE);
}

void DrawTiles(bool editorView, const Camera2D& cam, int activeFlagTx, int activeFlagTy) {
	Vector2 tl = GetScreenToWorld2D({ 0.0f, 0.0f }, cam);
	Vector2 br = GetScreenToWorld2D({ (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT }, cam);
	int tx0 = (int)floorf(tl.x / TILE) - 1; if (tx0 < 0) tx0 = 0;
	int ty0 = (int)floorf(tl.y / TILE) - 1; if (ty0 < 0) ty0 = 0;
	int tx1 = (int)floorf(br.x / TILE) + 2; if (tx1 > MAP_W) tx1 = MAP_W;
	int ty1 = (int)floorf(br.y / TILE) + 2; if (ty1 > MAP_H) ty1 = MAP_H;

	for (int ty = ty0; ty < ty1; ty++) {
		for (int tx = tx0; tx < tx1; tx++) {
			char c = gMap[ty][tx];
			if (c == TILE_WALL) {
				bool covered = IsSolid(tx, ty - 1);
				const SpriteSheet& s = covered ? gAssets.tileDirt : gAssets.tileGrass;
				if (s.ok) DrawTileTexture(s, tx, ty);
				else DrawRectangle(tx * TILE, ty * TILE, TILE, TILE, covered ? BROWN : DARKGREEN);
				continue;
			}
			if (c == TILE_STONE) {
				const SpriteSheet& s = (ty % 2 == 0) ? gAssets.tileStoneA : gAssets.tileStoneB;
				if (s.ok) DrawTileTexture(s, tx, ty);
				else DrawRectangle(tx * TILE, ty * TILE, TILE, TILE, GRAY);
				continue;
			}

			float cx = tx * (float)TILE + TILE * 0.5f;
			float bottom = ty * (float)TILE + TILE;

			if (c == TILE_FLAG) {
				bool active = (tx == activeFlagTx && ty == activeFlagTy);
				int frame = active ? (int)(GetTime() / FLAG_FRAME_TIME) % 4 : 0;
				if (active)
					DrawCircleGradient({ cx, bottom - TILE * 0.5f }, 34.0f,
					                   Fade(GOLD, 0.35f), Fade(GOLD, 0.0f));
				if (gAssets.flag.ok)
					DrawSheetFrame(gAssets.flag, frame, cx, bottom, FLAG_DRAW_SIZE, FLAG_DRAW_SIZE, 1, WHITE);
				else {
					DrawRectangle((int)cx - 2, (int)bottom - 36, 4, 36, DARKBROWN);
					DrawTriangle({ cx + 2, bottom - 36 }, { cx + 2, bottom - 22 },
					             { cx + 20, bottom - 29 }, active ? GOLD : RED);
				}
				continue;
			}
			if (c == TILE_GOAL) {
				float pulse = 0.22f + 0.12f * sinf((float)GetTime() * 3.0f);
				DrawCircleGradient({ cx, bottom - GOAL_DRAW_H * 0.45f }, 58.0f,
				                   Fade(GOLD, pulse), Fade(GOLD, 0.0f));
				if (gAssets.goal.ok)
					DrawSheetFrame(gAssets.goal, 0, cx, bottom, GOAL_DRAW_W, GOAL_DRAW_H, 1, WHITE);
				else {
					DrawRectangle((int)(cx - 20), (int)(bottom - 64), 40, 64, DARKPURPLE);
					DrawRectangleLines((int)(cx - 20), (int)(bottom - 64), 40, 64, GOLD);
				}
				continue;
			}
			if (!editorView) continue;

			if (c == TILE_SPAWN)
				DrawRectangleLinesEx({ tx * (float)TILE + 4, ty * (float)TILE + 4, TILE - 8.0f, TILE - 8.0f }, 2.0f, GREEN);
			else if (c == TILE_COIN)
				DrawSheetFrame(gAssets.coin, 0, cx, bottom - 4.0f, PICKUP_DRAW_SIZE, PICKUP_DRAW_SIZE, 1, WHITE);
			else if (c == TILE_HEALTH)
				DrawSheetFrame(gAssets.health, 0, cx, bottom - 4.0f, PICKUP_DRAW_SIZE, PICKUP_DRAW_SIZE, 1, WHITE);
			else if (c == TILE_ENERGY)
				DrawSheetFrame(gAssets.energy, 0, cx, bottom - 4.0f, PICKUP_DRAW_SIZE, PICKUP_DRAW_SIZE, 1, WHITE);
			else if (c == TILE_ENEMY)
				DrawSheetFrame(gAssets.enemyMove, 0, cx, bottom, 48.0f, 48.0f, 1, WHITE);
		}
	}
}

void DrawPickups(const std::vector<Pickup>& pickups) {
	for (const auto& pk : pickups) {
		if (pk.remove || pk.dormant) continue;
		const SpriteSheet* s =
			(pk.type == PickupType::Coin) ? &gAssets.coin :
			(pk.type == PickupType::Health) ? &gAssets.health : &gAssets.energy;
		int frame = (int)(pk.animTimer / PICKUP_FRAME_TIME) % 8;
		if (s->ok) {
			DrawSheetFrame(*s, frame, pk.center.x, pk.center.y + PICKUP_DRAW_SIZE * 0.5f,
			               PICKUP_DRAW_SIZE, PICKUP_DRAW_SIZE, 1, WHITE);
		} else {
			Color fb = (pk.type == PickupType::Coin) ? GOLD :
			           (pk.type == PickupType::Health) ? GREEN : RED;
			DrawCircleV(pk.center, 10.0f, fb);
		}
	}
}

void DrawEnemies(const std::vector<Enemy>& enemies) {
	for (const auto& e : enemies) {
		if (e.remove) continue;
		const SpriteSheet* s = nullptr;
		switch (e.state) {
		case EnemyState::Move:   s = &gAssets.enemyMove;   break;
		case EnemyState::Attack: s = &gAssets.enemyAttack; break;
		case EnemyState::Hurt:   s = &gAssets.enemyHurt;   break;
		case EnemyState::Dying:  s = &gAssets.enemyDeath;  break;
		}
		float cx = e.pos.x + e.size.x * 0.5f;
		float bottom = e.pos.y + e.size.y;
		if (s && s->ok)
			DrawSheetFrame(*s, e.animFrame, cx, bottom, ENEMY_DRAW_SIZE, ENEMY_DRAW_SIZE, e.facing, WHITE);
		else
			DrawRectangle((int)e.pos.x, (int)e.pos.y, (int)e.size.x, (int)e.size.y, LIME);
	}
}

void DrawFireballs(const std::vector<Fireball>& fireballs) {
	for (const auto& fb : fireballs) {
		if (fb.remove) continue;
		int frame = (int)(fb.animTimer / FIREBALL_FRAME_TIME) % 3;
		int facing = (fb.vx >= 0.0f) ? 1 : -1;
		if (gAssets.fireball.ok)
			DrawSheetFrame(gAssets.fireball, frame, fb.pos.x, fb.pos.y + FIREBALL_DRAW_SIZE * 0.5f,
			               FIREBALL_DRAW_SIZE, FIREBALL_DRAW_SIZE, facing, WHITE);
		else
			DrawCircleV(fb.pos, 8.0f, ORANGE);
	}
}

void DrawFxs(const std::vector<Fx>& fxs) {
	for (const auto& f : fxs) {
		if (f.life <= 0.0f) continue;
		if (f.type == FxType::HealPlus) {
			Color tint = Fade(WHITE, f.life / f.maxLife);
			if (gAssets.fxHeal.ok)
				DrawSheetFrame(gAssets.fxHeal, 0, f.pos.x, f.pos.y + FX_HEAL_DRAW_SIZE * 0.5f,
				               FX_HEAL_DRAW_SIZE, FX_HEAL_DRAW_SIZE, 1, tint);
			else {
				Color c = Fade(GREEN, f.life / f.maxLife);
				DrawRectangle((int)f.pos.x - 2, (int)f.pos.y - 7, 4, 14, c);
				DrawRectangle((int)f.pos.x - 7, (int)f.pos.y - 2, 14, 4, c);
			}
		}
		else {
			int frame = (int)((1.0f - f.life / f.maxLife) * 4.0f);
			if (frame > 3) frame = 3;
			if (gAssets.fxSparkle.ok)
				DrawSheetFrame(gAssets.fxSparkle, frame, f.pos.x, f.pos.y + FX_SPARK_DRAW_SIZE * 0.5f,
				               FX_SPARK_DRAW_SIZE, FX_SPARK_DRAW_SIZE, 1, WHITE);
			else
				DrawCircleLines((int)f.pos.x, (int)f.pos.y, 6.0f + 10.0f * (1.0f - f.life / f.maxLife), GOLD);
		}
	}
}

void DrawPlayer(const Player& p) {
	if (p.invincibleTimer > 0.0f && p.hurtTimer <= 0.0f
	    && fmodf(p.invincibleTimer, 2.0f * BLINK_INTERVAL) < BLINK_INTERVAL)
		return;

	Color tint = WHITE;
	if (p.boostTimer > 0.0f && fmodf(p.boostTimer, 2.0f * BOOST_TINT_INTERVAL) < BOOST_TINT_INTERVAL)
		tint = BOOST_TINT;

	int w = (int)p.weapon;
	const SpriteSheet* sheet = nullptr;
	switch (p.anim) {
	case AnimState::Walk:     sheet = &gAssets.walk[w];   break;
	case AnimState::JumpRise:
	case AnimState::JumpApex:
	case AnimState::JumpFall: sheet = &gAssets.jump[w];   break;
	case AnimState::Roll:     sheet = &gAssets.roll;      break;
	case AnimState::Attack:   sheet = &gAssets.attack[w]; break;
	case AnimState::Hurt:     sheet = &gAssets.hurt[w];   break;
	}

	float offsetY = 0.0f;
	if (p.anim == AnimState::Walk) {
		if (fabsf(p.speed.x) > WALK_ANIM_MIN_SPEED) {
			float stepTime = 3.0f * WALK_FRAME_TIME;
			offsetY = -WALK_BOB_PX * fabsf(sinf(PI * p.animTimer / stepTime));
		} else {
			offsetY = ((int)(p.animTimer / IDLE_BOB_PERIOD) % 2 == 1) ? IDLE_BOB_PX : 0.0f;
		}
	}
	if (p.landTimer > 0.0f) offsetY += LAND_DIP_PX;

	float cx = p.position.x + p.size.x * 0.5f;
	float bottom = p.position.y + p.size.y + offsetY;

	if (sheet && sheet->ok) {
		DrawSheetFrame(*sheet, p.animFrame, cx, bottom, CHAR_DRAW_SIZE, CHAR_DRAW_SIZE, p.facing, tint);
	} else {
		float x = p.position.x;
		float y = p.position.y + offsetY;
		DrawRectangle((int)x, (int)y, (int)p.size.x, (int)p.size.y, p.color);
		float eyeX = (p.facing > 0) ? x + p.size.x - 8 : x + 3;
		DrawRectangle((int)eyeX, (int)(y + 6), 5, 5, WHITE);
	}
}

void DrawHud(const Player& p) {
	for (int i = 0; i < PLAYER_MAX_HP; i++)
		DrawRectangle(20 + i * 28, 20, 24, 20, i < p.hp ? RED : Fade(BLACK, 0.45f));

	for (int i = 0; i < PLAYER_MAX_MANA; i++) {
		DrawRectangle(20 + i * 22, 46, 18, 12, i < p.mana ? SKYBLUE : Fade(BLACK, 0.45f));
		if (i == p.mana && p.mana < PLAYER_MAX_MANA) {
			float t = p.manaRegenTimer / MANA_REGEN_TIME;
			DrawRectangle(20 + i * 22, 46, (int)(18.0f * t), 12, Fade(SKYBLUE, 0.45f));
		}
	}

	if (gAssets.coin.ok) DrawSheetFrame(gAssets.coin, 0, 32, 104, 26, 26, 1, WHITE);
	else                 DrawCircle(32, 92, 10, GOLD);
	DrawText(TextFormat("x %d / %d", p.coins, p.totalCoins), 50, 82, 22, RAYWHITE);

	DrawText(TextFormat("Weapon: %s  (Q)", (p.weapon == Weapon::Staff) ? "STAFF" : "SWORD"),
	         20, 114, 18, RAYWHITE);
	if (p.boostTimer > 0.0f)
		DrawText(TextFormat("SPEED UP %.1f", p.boostTimer), 20, 136, 18, BOOST_TINT);

	int mins = (int)(p.playTime / 60.0f);
	int secs = (int)p.playTime % 60;
	DrawText(TextFormat("TIME %02d:%02d   DEATHS %d", mins, secs, p.deaths),
	         SCREEN_WIDTH - 420, 44, 18, Fade(RAYWHITE, 0.7f));
}

void DrawPlayingScene(const Player& player, const std::vector<Pickup>& pickups,
                      const std::vector<Enemy>& enemies, const std::vector<Fireball>& fireballs,
                      const std::vector<Fx>& fxs, const Camera2D& camera) {
	DrawParallaxBackground(camera);
	BeginMode2D(camera);
	DrawTiles(false, camera, player.checkpointTx, player.checkpointTy);
	DrawPickups(pickups);
	DrawEnemies(enemies);
	DrawFireballs(fireballs);
	DrawPlayer(player);
	DrawFxs(fxs);
	EndMode2D();
}

struct Editor {
	Camera2D cam;
	char brush;
	bool initialized;
	bool dirty;
};

void EditorInit(Editor& ed) {
	ed.cam = {};
	ed.cam.offset = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
	ed.cam.zoom = 1.0f;
	Vector2 spawn = FindSpawn();
	ed.cam.target = { spawn.x, spawn.y };
	ed.brush = TILE_WALL;
	ed.initialized = true;
	ed.dirty = false;
}

const char* BrushName(char b) {
	switch (b) {
	case TILE_WALL:   return "WALL";
	case TILE_SPAWN:  return "SPAWN";
	case TILE_COIN:   return "COIN";
	case TILE_HEALTH: return "POTION";
	case TILE_ENERGY: return "ENERGY";
	case TILE_ENEMY:  return "SLIME";
	case TILE_STONE:  return "STONE";
	case TILE_FLAG:   return "FLAG";
	case TILE_GOAL:   return "GOAL";
	default:          return "ERASE";
	}
}

void UpdateEditor(Editor& ed, float dt) {

	float panSpeed = 500.0f / ed.cam.zoom;
	if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  ed.cam.target.x -= panSpeed * dt;
	if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) ed.cam.target.x += panSpeed * dt;
	if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    ed.cam.target.y -= panSpeed * dt;
	if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  ed.cam.target.y += panSpeed * dt;

	float wheel = GetMouseWheelMove();
	if (wheel != 0.0f) {
		ed.cam.zoom += wheel * 0.1f;
		if (ed.cam.zoom < 0.2f) ed.cam.zoom = 0.2f;
		if (ed.cam.zoom > 3.0f) ed.cam.zoom = 3.0f;
	}

	if (IsKeyPressed(KEY_ONE))   ed.brush = TILE_WALL;
	if (IsKeyPressed(KEY_TWO))   ed.brush = TILE_EMPTY;
	if (IsKeyPressed(KEY_THREE)) ed.brush = TILE_SPAWN;
	if (IsKeyPressed(KEY_FOUR))  ed.brush = TILE_COIN;
	if (IsKeyPressed(KEY_FIVE))  ed.brush = TILE_HEALTH;
	if (IsKeyPressed(KEY_SIX))   ed.brush = TILE_ENERGY;
	if (IsKeyPressed(KEY_SEVEN)) ed.brush = TILE_ENEMY;
	if (IsKeyPressed(KEY_EIGHT)) ed.brush = TILE_STONE;
	if (IsKeyPressed(KEY_NINE))  ed.brush = TILE_FLAG;
	if (IsKeyPressed(KEY_ZERO))  ed.brush = TILE_GOAL;

	if (IsKeyPressed(KEY_RIGHT_BRACKET)) { AddColumn();    ed.dirty = true; }
	if (IsKeyPressed(KEY_LEFT_BRACKET))  { RemoveColumn(); ed.dirty = true; }
	if (IsKeyPressed(KEY_APOSTROPHE))    { AddRow();       ed.dirty = true; }
	if (IsKeyPressed(KEY_SEMICOLON))     { RemoveRow();    ed.dirty = true; }

	bool overHud = GetMousePosition().y < 70.0f;
	Vector2 world = GetScreenToWorld2D(GetMousePosition(), ed.cam);
	int tx = (int)floorf(world.x / TILE);
	int ty = (int)floorf(world.y / TILE);
	if (!overHud && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
		if (ed.brush == TILE_SPAWN) {
			for (int y = 0; y < MAP_H; y++)
				for (int x = 0; x < MAP_W; x++)
					if (gMap[y][x] == TILE_SPAWN) gMap[y][x] = TILE_EMPTY;
		}
		SetTile(tx, ty, ed.brush);
		ed.dirty = true;
	}
	if (!overHud && IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) { SetTile(tx, ty, TILE_EMPTY); ed.dirty = true; }

	if (IsKeyPressed(KEY_F5)) SaveMap();
	if (IsKeyPressed(KEY_F9)) { if (!LoadMap()) LoadDefaultMap(); EditorInit(ed); ed.dirty = true; }
}

void DrawEditor(const Editor& ed) {
	BeginMode2D(ed.cam);
	DrawTiles(true, ed.cam, -1, -1);

	Color grid = Fade(GRAY, 0.35f);
	for (int x = 0; x <= MAP_W; x++)
		DrawLine(x * TILE, 0, x * TILE, MAP_H * TILE, grid);
	for (int y = 0; y <= MAP_H; y++)
		DrawLine(0, y * TILE, MAP_W * TILE, y * TILE, grid);

	DrawRectangleLinesEx({ 0, 0, (float)MAP_W * TILE, (float)MAP_H * TILE }, 2.0f, ORANGE);

	Vector2 world = GetScreenToWorld2D(GetMousePosition(), ed.cam);
	int tx = (int)floorf(world.x / TILE);
	int ty = (int)floorf(world.y / TILE);
	if (tx >= 0 && tx < MAP_W && ty >= 0 && ty < MAP_H)
		DrawRectangleLinesEx({ tx * (float)TILE, ty * (float)TILE, (float)TILE, (float)TILE }, 2.0f, YELLOW);

	EndMode2D();

	DrawRectangle(0, 0, SCREEN_WIDTH, 70, Fade(BLACK, 0.6f));
	DrawText(TextFormat("EDITOR  |  Brush: %s   Zoom: %.1f   Size: %dx%d", BrushName(ed.brush), ed.cam.zoom, MAP_W, MAP_H), 12, 10, 20, RAYWHITE);
	DrawText("LMB paint RMB erase | 1 ground 2 erase 3 spawn 4 coin 5 potion 6 energy 7 slime 8 stone 9 flag 0 goal | [ ] cols ; ' rows | F5 save F9 load | F1 play", 12, 40, 16, LIGHTGRAY);
}

int main() {
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE);
	SetWindowMinSize(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
	SetTargetFPS(60);
	SetExitKey(KEY_NULL);

	gContentRoot = LocateContentRoot();
	TraceLog(LOG_INFO, "CONTENT: root resolved to '%s'", gContentRoot.c_str());

	InitAudioDevice();
	LoadAssets();
	LoadSounds();

	RenderTexture2D target = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
	SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

	if (!LoadMap()) LoadDefaultMap();

	Camera2D camera{};
	camera.offset = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
	camera.zoom = 1.0f;

	GameState state = GameState::MENU;
	Player player;
	std::vector<Pickup>   pickups;
	std::vector<Enemy>    enemies;
	std::vector<Fireball> fireballs;
	std::vector<Fx>       fxs;
	ResetGame(player, pickups, enemies, fireballs, fxs);

	Editor editor;
	editor.initialized = false;
	editor.dirty = false;
	bool quitRequested = false;

	while (!WindowShouldClose() && !quitRequested) {
		float dt = GetFrameTime();

		if (gSounds.musicOk) UpdateMusicStream(gSounds.music);

		if ((IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) && IsKeyPressed(KEY_ENTER))
			ToggleBorderlessWindowed();

		float scale = fminf((float)GetScreenWidth() / SCREEN_WIDTH,
		                    (float)GetScreenHeight() / SCREEN_HEIGHT);
		float offX = (GetScreenWidth()  - SCREEN_WIDTH  * scale) * 0.5f;
		float offY = (GetScreenHeight() - SCREEN_HEIGHT * scale) * 0.5f;
		SetMouseOffset((int)-offX, (int)-offY);
		SetMouseScale(1.0f / scale, 1.0f / scale);

		BeginTextureMode(target);
		ClearBackground(DARKGRAY);

		if (state == GameState::MENU) {
			DrawText(GAME_TITLE, SCREEN_WIDTH / 2 - MeasureText(GAME_TITLE, 60) / 2, 80, 60, RAYWHITE);
			int btnW = 320, btnX = SCREEN_WIDTH / 2 - btnW / 2;
			if (DrawButton("Start New Game", btnX, 210, btnW, 55)) {
				ResetGame(player, pickups, enemies, fireballs, fxs);
				editor.dirty = false;
				state = GameState::PLAYING;
			}
			if (DrawButton("Continue", btnX, 275, btnW, 55)) {
				if (editor.dirty) {
					RefreshFromMap(player, pickups, enemies, fireballs, fxs);
					editor.dirty = false;
				}
				state = GameState::PLAYING;
			}
			if (DrawButton("Map Editor", btnX, 340, btnW, 55)) {
				if (!editor.initialized) EditorInit(editor);
				state = GameState::EDITOR;
			}
			if (DrawButton("Quit", btnX, 405, btnW, 55)) {
				quitRequested = true;
			}
			DrawText("Tip: press F1 in game to open the editor | Alt+Enter fullscreen", SCREEN_WIDTH / 2 - MeasureText("Tip: press F1 in game to open the editor | Alt+Enter fullscreen", 18) / 2, 490, 18, GRAY);
		}
		else if (state == GameState::PLAYING) {
			if (IsKeyPressed(KEY_ESCAPE)) state = GameState::PAUSED;
			if (IsKeyPressed(KEY_F1)) {
				if (!editor.initialized) EditorInit(editor);
				state = GameState::EDITOR;
			}

			player.playTime += dt;

			UpdatePlayer(player, fireballs, dt);
			MoveAndCollide(player, dt);
			UpdatePlayerAnim(player, dt);

			if (player.onGround) player.jumpsleft = 2;

			if (player.position.y > MAP_H * TILE) {
				player.position = player.checkpoint;
				player.speed = { 0.0f, 0.0f };
				player.hp--;
				player.invincibleTimer = INVINCIBLE_TIME;
				if (player.hp > 0) PlaySfx(gSounds.hurt);
			}
			if (player.hp <= 0) {
				player.deaths++;
				player.position = player.checkpoint;
				player.speed = { 0.0f, 0.0f };
				player.hp = PLAYER_MAX_HP;
				player.mana = PLAYER_MAX_MANA;
				player.boostTimer = 0.0f;
				player.hurtTimer = 0.0f;
				player.attackTimer = 0.0f;
				player.rollTimer = 0.0f;
				player.dodging = false;
				player.invincibleTimer = INVINCIBLE_TIME;
				RespawnEnemies(enemies);
				fireballs.clear();
				PlaySfx(gSounds.death);
			}

			UpdatePickups(pickups, dt);
			for (auto& e : enemies) UpdateEnemy(e, player, dt);
			UpdateFireballs(fireballs, dt);
			UpdateFxs(fxs, dt);

			HandleInteractions(player, pickups, enemies, fireballs, fxs);
			if (UpdateTileTriggers(player, fxs)) {
				state = GameState::WIN;
				PlaySfx(gSounds.win);
			}
			CleanupEntities(pickups, enemies, fireballs, fxs);

			UpdateCamera(camera, player);

			DrawPlayingScene(player, pickups, enemies, fireballs, fxs, camera);
			DrawHud(player);
			DrawText("ESC pause  F1 editor  Q weapon  LMB/J attack  Shift roll", SCREEN_WIDTH - 480, 20, 18, Fade(RAYWHITE, 0.7f));
		}
		else if (state == GameState::PAUSED) {
			DrawPlayingScene(player, pickups, enemies, fireballs, fxs, camera);
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
		else if (state == GameState::WIN) {
			DrawPlayingScene(player, pickups, enemies, fireballs, fxs, camera);
			DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.55f));

			const char* t = "LEVEL CLEAR!";
			DrawText(t, SCREEN_WIDTH / 2 - MeasureText(t, 60) / 2, 100, 60, GOLD);

			int mins = (int)(player.playTime / 60.0f);
			int secs = (int)player.playTime % 60;
			const char* stats = TextFormat("TIME %02d:%02d    COINS %d / %d    DEATHS %d",
			                               mins, secs, player.coins, player.totalCoins, player.deaths);
			DrawText(stats, SCREEN_WIDTH / 2 - MeasureText(stats, 26) / 2, 210, 26, RAYWHITE);
			if (player.coins >= player.totalCoins && player.totalCoins > 0) {
				const char* all = "ALL COINS COLLECTED!";
				DrawText(all, SCREEN_WIDTH / 2 - MeasureText(all, 22) / 2, 250, 22, GOLD);
			}

			int btnW = 320, btnX = SCREEN_WIDTH / 2 - btnW / 2;
			if (DrawButton("Play Again", btnX, 320, btnW, 60)) {
				ResetGame(player, pickups, enemies, fireballs, fxs);
				editor.dirty = false;
				state = GameState::PLAYING;
			}
			if (DrawButton("Back to Menu", btnX, 400, btnW, 60)) {
				ResetGame(player, pickups, enemies, fireballs, fxs);
				editor.dirty = false;
				state = GameState::MENU;
			}
		}
		else if (state == GameState::EDITOR) {
			if (IsKeyPressed(KEY_F1)) {
				if (editor.dirty) {
					RefreshFromMap(player, pickups, enemies, fireballs, fxs);
					editor.dirty = false;
				}
				state = GameState::PLAYING;
			}
			if (IsKeyPressed(KEY_ESCAPE)) state = GameState::MENU;

			UpdateEditor(editor, dt);
			DrawEditor(editor);
		}

		EndTextureMode();

		BeginDrawing();
		ClearBackground(BLACK);
		DrawTexturePro(target.texture,
		               { 0.0f, 0.0f, (float)target.texture.width, -(float)target.texture.height },
		               { offX, offY, SCREEN_WIDTH * scale, SCREEN_HEIGHT * scale },
		               { 0.0f, 0.0f }, 0.0f, WHITE);
		EndDrawing();
	}

	UnloadRenderTexture(target);
	UnloadSounds();
	UnloadAssets();
	CloseAudioDevice();
	CloseWindow();
	return 0;
}
