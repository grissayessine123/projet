/* platforms.c – gestion des plateformes (fixe, mobile, destructible) */
#include "game.h"

/* ------------------------------------------------------------------ */
void initPlatforms(Map *map, int level)
{
    map->platform_count = 0;

    if (level == LEVEL_1) {
        /* Mobile platform 1 */
        Platform *p = &map->platforms[map->platform_count++];
        p->type        = PLATFORM_MOBILE;
        p->x           = 3;   /* tile column start */
        p->y           = 15;
        p->alive       = 1;
        p->mobile_dir  = 1;
        p->mobile_offset = 0;

        /* Mobile platform 2 */
        p = &map->platforms[map->platform_count++];
        p->type        = PLATFORM_MOBILE;
        p->x           = 16;
        p->y           = 13;
        p->alive       = 1;
        p->mobile_dir  = -1;
        p->mobile_offset = 0;

    } else {
        /* Level 2: three mobile platforms */
        int starts[][2] = {{1,16},{10,14},{20,12}};
        int dirs[]      = {1, -1, 1};
        for (int i = 0; i < 3; i++) {
            Platform *p    = &map->platforms[map->platform_count++];
            p->type        = PLATFORM_MOBILE;
            p->x           = starts[i][0];
            p->y           = starts[i][1];
            p->alive       = 1;
            p->mobile_dir  = dirs[i];
            p->mobile_offset = 0;
        }
    }
}

/* ------------------------------------------------------------------ */
void updatePlatforms(Map *map, float dt)
{
    float speed = 60.0f;  /* pixels/sec */

    for (int i = 0; i < map->platform_count; i++) {
        Platform *p = &map->platforms[i];
        if (!p->alive) continue;
        if (p->type != PLATFORM_MOBILE) continue;

        p->mobile_offset += p->mobile_dir * speed * dt;

        /* Bounce within 3-tile range */
        if (p->mobile_offset >  3 * TILE_SIZE) { p->mobile_offset =  3 * TILE_SIZE; p->mobile_dir = -1; }
        if (p->mobile_offset < -3 * TILE_SIZE) { p->mobile_offset = -3 * TILE_SIZE; p->mobile_dir =  1; }
    }
}

/* ------------------------------------------------------------------ */
void afficherPlatforms(SDL_Renderer *r, const Map *map, const Camera *cam,
                       SDL_Rect vp)
{
    (void)vp; /* viewport already set by caller */

    for (int i = 0; i < map->platform_count; i++) {
        const Platform *p = &map->platforms[i];
        if (!p->alive) continue;

        int world_x = p->x * TILE_SIZE + (int)p->mobile_offset;
        int world_y = p->y * TILE_SIZE;
        int sx = world_x - cam->cam_x;
        int sy = world_y - cam->cam_y;

        SDL_Rect dst = { sx, sy, TILE_SIZE * 3, TILE_SIZE / 2 };

        /* Blue for mobile */
        SDL_SetRenderDrawColor(r, 60, 180, 255, 255);
        SDL_RenderFillRect(r, &dst);
        SDL_SetRenderDrawColor(r, 20, 80, 180, 255);
        SDL_RenderDrawRect(r, &dst);
    }
}

/* ------------------------------------------------------------------ */
/* Destroy a destructible tile at world pixel (wx, wy)               */
/* ------------------------------------------------------------------ */
void destroyTileAt(Map *map, int wx, int wy)
{
    int col = wx / TILE_SIZE;
    int row = wy / TILE_SIZE;
    if (row < 0 || row >= MAP_ROWS || col < 0 || col >= MAP_COLS) return;
    if (map->tiles[row][col] == PLATFORM_DESTRUCTIBLE)
        map->tiles[row][col] = PLATFORM_NONE;
}
