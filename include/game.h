#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ============================================================
   CONSTANTS
   ============================================================ */
#define WINDOW_W        800
#define WINDOW_H        600
#define TILE_SIZE       48
#define MAP_COLS        30
#define MAP_ROWS        20
#define VIEWPORT_COLS   17          /* tiles visible horizontally */
#define VIEWPORT_ROWS   13          /* tiles visible vertically   */
#define MAX_SCORES      5
#define NAME_MAX_LEN    32
#define FPS             60

/* Platform types */
#define PLATFORM_NONE        0
#define PLATFORM_FIXED       1
#define PLATFORM_MOBILE      2
#define PLATFORM_DESTRUCTIBLE 3

/* Display modes */
#define MODE_MONO   0   /* single viewport */
#define MODE_MULTI  1   /* split screen (two viewports side by side) */

/* Level IDs */
#define LEVEL_1  1
#define LEVEL_2  2

/* ============================================================
   DATA STRUCTURES
   ============================================================ */

typedef struct {
    int type;           /* PLATFORM_* */
    int x, y;          /* tile position */
    int alive;          /* for destructible: 1=intact, 0=destroyed */
    int mobile_dir;     /* +1 right, -1 left (for mobile platforms) */
    float mobile_offset; /* current pixel offset */
} Platform;

typedef struct {
    int tiles[MAP_ROWS][MAP_COLS];       /* tile types */
    Platform platforms[64];              /* platforms list */
    int platform_count;
    int level;
} Map;

typedef struct {
    float x, y;         /* world position (pixels) */
    float vx, vy;
    int on_ground;
    int score;
    int lives;
} Player;

/* Camera / scroll state */
typedef struct {
    int cam_x, cam_y;   /* top-left world pixel of viewport */
} Camera;

/* Score entry */
typedef struct {
    char name[NAME_MAX_LEN];
    int  score;
} ScoreEntry;

/* Guide / HUD indicator window */
typedef struct {
    SDL_Rect  rect;
    char      title[64];
    char      lines[6][64];
    int       line_count;
    int       visible;
} GuideWindow;

/* Full game state */
typedef struct {
    SDL_Window   *window;
    SDL_Renderer *renderer;
    TTF_Font     *font_large;
    TTF_Font     *font_small;

    Map      map;
    Player   player;   /* Joueur 1 – ZQSD / flèches */
    Player   player2;  /* Joueur 2 – IJKL            */
    Camera   cam;      /* caméra Joueur 1 */
    Camera   cam2;     /* caméra Joueur 2 */

    int      display_mode;   /* MODE_MONO / MODE_MULTI */
    int      running;
    int      paused;
    int      show_guide;

    Uint32   start_ticks;    /* for afficherTemps */

    GuideWindow guide;

    /* Best scores sub-menu */
    int          scores_menu_active;
    ScoreEntry   scores[MAX_SCORES];
    int          score_count;
    char         input_name[NAME_MAX_LEN];
    int          input_len;
    int          waiting_name;   /* 1 = asking player for name */
} GameState;

/* ============================================================
   FUNCTION PROTOTYPES
   ============================================================ */

/* --- Initialisation --- */
int  game_init(GameState *g);
void game_cleanup(GameState *g);

/* --- Background / map --- */
void initBackground(Map *map, int level);
void afficherBackground(SDL_Renderer *r, const Map *map, const Camera *cam,
                        SDL_Rect viewport);

/* --- Platforms --- */
void initPlatforms(Map *map, int level);
void updatePlatforms(Map *map, float dt);
void afficherPlatforms(SDL_Renderer *r, const Map *map, const Camera *cam,
                       SDL_Rect viewport);

/* --- Display (mono / multi) --- */
void afficher(GameState *g, int mode);

/* --- Scroll --- */
void updateCamera(Camera *cam, const Player *p, int map_w_px, int map_h_px);

/* --- Time --- */
void afficherTemps(SDL_Renderer *r, TTF_Font *font, Uint32 start_ticks,
                   int x, int y);

/* --- Guide window --- */
void initGuide(GuideWindow *gw);
void afficherGuide(SDL_Renderer *r, TTF_Font *font, const GuideWindow *gw);

/* --- Scores sub-menu --- */
void initScores(GameState *g);
void loadScores(GameState *g, const char *filename);
void saveScores(GameState *g, const char *filename);
void addScore(GameState *g, const char *name, int score);
void afficherScoresMenu(GameState *g);

/* --- Events & update --- */
void handle_events(GameState *g);
void update(GameState *g, float dt);

/* --- Rendering helper --- */
void render_text(SDL_Renderer *r, TTF_Font *font, const char *text,
                 int x, int y, SDL_Color col);

#endif /* GAME_H */
