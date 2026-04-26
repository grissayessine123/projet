/* background.c – initialiser et afficher le background (Level1/Level2)
   Platforms: fixe, mobile, destructible
*/
#include "game.h"

/* ------------------------------------------------------------------ */
/* Colour palette for tiles                                            */
/* ------------------------------------------------------------------ */
static void tile_color(SDL_Renderer *r, int type)
{
    switch (type) {
        case PLATFORM_NONE:
            SDL_SetRenderDrawColor(r, 30, 30, 60, 255);   /* dark sky */
            break;
        case PLATFORM_FIXED:
            SDL_SetRenderDrawColor(r, 100, 180, 80, 255); /* green */
            break;
        case PLATFORM_MOBILE:
            SDL_SetRenderDrawColor(r, 80, 140, 220, 255); /* blue */
            break;
        case PLATFORM_DESTRUCTIBLE:
            SDL_SetRenderDrawColor(r, 200, 120, 60, 255); /* orange */
            break;
        default:
            SDL_SetRenderDrawColor(r, 60, 60, 60, 255);
    }
}

/* ------------------------------------------------------------------ */
/* initBackground – fill tile map for the requested level             */
/* ------------------------------------------------------------------ */
void initBackground(Map *map, int level)
{
    memset(map, 0, sizeof(*map));
    map->level = level;

    /* Default: air everywhere */
    for (int r = 0; r < MAP_ROWS; r++)
        for (int c = 0; c < MAP_COLS; c++)
            map->tiles[r][c] = PLATFORM_NONE;

    if (level == LEVEL_1) {
        /* --- Level 1: flat floor + a few fixed blocks --- */
        /* Ground row */
        for (int c = 0; c < MAP_COLS; c++)
            map->tiles[MAP_ROWS - 1][c] = PLATFORM_FIXED;

        /* Some fixed platforms */
        for (int c = 4; c <= 7; c++)  map->tiles[12][c] = PLATFORM_FIXED;
        for (int c = 10; c <= 14; c++) map->tiles[10][c] = PLATFORM_FIXED;
        for (int c = 18; c <= 22; c++) map->tiles[8][c]  = PLATFORM_FIXED;
        for (int c = 24; c <= 27; c++) map->tiles[14][c] = PLATFORM_FIXED;

        /* Destructible blocks */
        for (int c = 8; c <= 9; c++)   map->tiles[13][c] = PLATFORM_DESTRUCTIBLE;
        for (int c = 15; c <= 17; c++) map->tiles[11][c] = PLATFORM_DESTRUCTIBLE;

    } else {
        /* --- Level 2: more complex, gaps in floor --- */
        /* Floor with gaps */
        for (int c = 0; c < MAP_COLS; c++) {
            if (c < 5 || (c > 7 && c < 13) || (c > 15 && c < 20) ||
                (c > 22 && c < 28))
                map->tiles[MAP_ROWS - 1][c] = PLATFORM_FIXED;
        }
        /* Stepped platforms */
        for (int c = 2; c <= 5; c++)   map->tiles[14][c] = PLATFORM_FIXED;
        for (int c = 6; c <= 9; c++)   map->tiles[12][c] = PLATFORM_FIXED;
        for (int c = 10; c <= 13; c++) map->tiles[10][c] = PLATFORM_FIXED;
        for (int c = 14; c <= 17; c++) map->tiles[8][c]  = PLATFORM_FIXED;
        for (int c = 18; c <= 21; c++) map->tiles[6][c]  = PLATFORM_FIXED;

        /* Destructible ceiling-ish blocks */
        for (int c = 3; c <= 6; c++)   map->tiles[9][c]  = PLATFORM_DESTRUCTIBLE;
        for (int c = 20; c <= 23; c++) map->tiles[11][c] = PLATFORM_DESTRUCTIBLE;
    }

    /* Initialise platforms (mobile ones) */
    initPlatforms(map, level);
}

/* ------------------------------------------------------------------ */
/* afficherBackground – draw visible tiles                             */
/* ------------------------------------------------------------------ */
void afficherBackground(SDL_Renderer *r, const Map *map, const Camera *cam,
                        SDL_Rect vp)
{
    /* Clip to viewport */
    SDL_RenderSetViewport(r, &vp);

    int start_col = cam->cam_x / TILE_SIZE;
    int start_row = cam->cam_y / TILE_SIZE;
    int end_col   = start_col + VIEWPORT_COLS + 2;
    int end_row   = start_row + VIEWPORT_ROWS + 2;
    if (end_col > MAP_COLS) end_col = MAP_COLS;
    if (end_row > MAP_ROWS) end_row = MAP_ROWS;

    /* Sky background – FillRect au lieu de RenderClear !
       RenderClear ignore le viewport et efface tout l'écran. */
    int bg_r = (map->level == LEVEL_1) ? 30 : 10;
    int bg_g = (map->level == LEVEL_1) ? 30 : 20;
    int bg_b = (map->level == LEVEL_1) ? 60 : 40;
    SDL_SetRenderDrawColor(r, bg_r, bg_g, bg_b, 255);
    SDL_Rect full_vp = { 0, 0, vp.w, vp.h };   /* coordonnées locales au viewport */
    SDL_RenderFillRect(r, &full_vp);

    /* Draw tiles */
    for (int row = start_row; row < end_row; row++) {
        for (int col = start_col; col < end_col; col++) {
            int type = map->tiles[row][col];
            if (type == PLATFORM_NONE) continue;

            SDL_Rect dst = {
                col * TILE_SIZE - cam->cam_x,
                row * TILE_SIZE - cam->cam_y,
                TILE_SIZE,
                TILE_SIZE
            };
            tile_color(r, type);
            SDL_RenderFillRect(r, &dst);

            /* Grid outline */
            SDL_SetRenderDrawColor(r, 0, 0, 0, 80);
            SDL_RenderDrawRect(r, &dst);
        }
    }

    /* Draw mobile platforms on top */
    afficherPlatforms(r, map, cam, vp);

    SDL_RenderSetViewport(r, NULL);
}
