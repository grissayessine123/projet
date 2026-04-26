/* events.c – gestion des evenements et mise a jour du jeu */
#include "game.h"

/* Forward declaration of destroyTileAt from platforms.c */
void destroyTileAt(Map *map, int wx, int wy);

/* ------------------------------------------------------------------ */
void handle_events(GameState *g)
{
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {

        /* -------- Text input for score name entry -------- */
        if (g->waiting_name) {
            if (ev.type == SDL_TEXTINPUT) {
                int len = (int)strlen(ev.text.text);
                if (g->input_len + len < NAME_MAX_LEN - 1) {
                    strcat(g->input_name, ev.text.text);
                    g->input_len += len;
                }
            }
            if (ev.type == SDL_KEYDOWN) {
                if (ev.key.keysym.sym == SDLK_BACKSPACE && g->input_len > 0) {
                    g->input_name[--g->input_len] = '\0';
                }
                if (ev.key.keysym.sym == SDLK_RETURN && g->input_len > 0) {
                    addScore(g, g->input_name, g->player.score);
                    g->waiting_name = 0;
                    SDL_StopTextInput();
                }
                if (ev.key.keysym.sym == SDLK_ESCAPE) {
                    g->waiting_name = 0;
                    SDL_StopTextInput();
                }
            }
            if (ev.type == SDL_QUIT) g->running = 0;
            continue;
        }

        /* -------- Normal events -------- */
        switch (ev.type) {
            case SDL_QUIT:
                g->running = 0;
                break;

            case SDL_KEYDOWN:
                switch (ev.key.keysym.sym) {

                    case SDLK_ESCAPE:
                        g->running = 0;
                        break;

                    /* Guide toggle */
                    case SDLK_g:
                        g->show_guide     = !g->show_guide;
                        g->guide.visible  = g->show_guide;
                        break;

                    /* Display mode toggle */
                    case SDLK_m:
                        g->display_mode = (g->display_mode == MODE_MONO)
                                          ? MODE_MULTI : MODE_MONO;
                        break;

                    /* Scores sub-menu */
                    case SDLK_s:
                        if (!g->scores_menu_active) {
                            g->scores_menu_active = 1;
                        } else {
                            /* If menu open and not waiting for name,
                               propose to save current score */
                            if (!g->waiting_name && g->player.score > 0) {
                                g->waiting_name = 1;
                                g->input_len    = 0;
                                memset(g->input_name, 0, sizeof(g->input_name));
                                SDL_StartTextInput();
                            } else {
                                g->scores_menu_active = 0;
                            }
                        }
                        break;

                    /* Level switch */
                    case SDLK_l:
                    {
                        int next = (g->map.level == LEVEL_1) ? LEVEL_2 : LEVEL_1;
                        initBackground(&g->map, next);
                        g->player.x  = 2 * TILE_SIZE;
                        g->player.y  = (MAP_ROWS - 3) * TILE_SIZE;
                        g->player.vx = 0; g->player.vy = 0;
                        g->player2.x = 6 * TILE_SIZE;
                        g->player2.y = (MAP_ROWS - 3) * TILE_SIZE;
                        g->player2.vx = 0; g->player2.vy = 0;
                        g->start_ticks = SDL_GetTicks();
                        break;
                    }

                    /* Pause */
                    case SDLK_p:
                        g->paused = !g->paused;
                        break;

                    /* Destroy tile under player (spacebar) – Joueur 1 */
                    case SDLK_SPACE:
                    {
                        int wx = (int)g->player.x + TILE_SIZE/2;
                        int wy = (int)g->player.y + TILE_SIZE + 2;
                        destroyTileAt(&g->map, wx, wy);
                        g->player.score += 10;
                        break;
                    }

                    /* Destroy tile under player 2 – touche N */
                    case SDLK_n:
                    {
                        int wx = (int)g->player2.x + TILE_SIZE/2;
                        int wy = (int)g->player2.y + TILE_SIZE + 2;
                        destroyTileAt(&g->map, wx, wy);
                        g->player2.score += 10;
                        break;
                    }

                    default: break;
                }
                break;

            default: break;
        }
    }
}

/* ------------------------------------------------------------------ */
/* Simple AABB collision: returns 1 if world pixel overlaps solid tile */
/* ------------------------------------------------------------------ */
static int solid_at(const Map *map, int wx, int wy)
{
    int col = wx / TILE_SIZE;
    int row = wy / TILE_SIZE;
    if (row < 0 || row >= MAP_ROWS || col < 0 || col >= MAP_COLS) return 1;
    return (map->tiles[row][col] != PLATFORM_NONE);
}

/* ------------------------------------------------------------------ */
/* move_player – physique + collisions pour UN joueur                  */
/* ------------------------------------------------------------------ */
static void move_player(Player *p, const Map *map, float dt,
                        float vx_input, int jump_input)
{
    float gravity  = 500.0f;
    float jump_vel = -280.0f;
    int pw = TILE_SIZE - 4, ph = TILE_SIZE - 2;

    p->vx = vx_input;

    if (jump_input && p->on_ground) {
        p->vy = jump_vel;
        p->on_ground = 0;
    }

    p->vy += gravity * dt;

    /* --- Move X --- */
    p->x += p->vx * dt;

    if (p->x < 0) { p->x = 0; p->vx = 0; }
    if (p->x + pw > MAP_COLS * TILE_SIZE) {
        p->x = MAP_COLS * TILE_SIZE - pw; p->vx = 0;
    }

    if (p->vx > 0) {
        int right = (int)p->x + pw;
        if (solid_at(map, right, (int)p->y + 4) ||
            solid_at(map, right, (int)p->y + ph - 4)) {
            p->x = (right / TILE_SIZE) * TILE_SIZE - pw; p->vx = 0;
        }
    } else if (p->vx < 0) {
        int left = (int)p->x;
        if (solid_at(map, left, (int)p->y + 4) ||
            solid_at(map, left, (int)p->y + ph - 4)) {
            p->x = (left / TILE_SIZE + 1) * TILE_SIZE; p->vx = 0;
        }
    }

    /* --- Move Y --- */
    p->y += p->vy * dt;
    p->on_ground = 0;

    if (p->vy >= 0) {
        int bot = (int)p->y + ph;
        if (solid_at(map, (int)p->x + 4, bot) ||
            solid_at(map, (int)p->x + pw - 4, bot)) {
            p->y = (bot / TILE_SIZE) * TILE_SIZE - ph;
            p->vy = 0; p->on_ground = 1;
        }
    }
    if (p->vy < 0) {
        int top = (int)p->y;
        if (solid_at(map, (int)p->x + 4, top) ||
            solid_at(map, (int)p->x + pw - 4, top)) {
            p->y = (top / TILE_SIZE + 1) * TILE_SIZE; p->vy = 0;
        }
    }
}

/* ------------------------------------------------------------------ */
void update(GameState *g, float dt)
{
    if (g->paused || g->scores_menu_active) return;

    const Uint8 *ks   = SDL_GetKeyboardState(NULL);
    float speed = 160.0f;

    /* ====== Joueur 1 : ZQSD + flèches ====== */
    float vx1 = 0;
    if (ks[SDL_SCANCODE_LEFT]  || ks[SDL_SCANCODE_Q]) vx1 = -speed;
    if (ks[SDL_SCANCODE_RIGHT] || ks[SDL_SCANCODE_D]) vx1 =  speed;
    int jump1 = ks[SDL_SCANCODE_UP] || ks[SDL_SCANCODE_Z];

    move_player(&g->player, &g->map, dt, vx1, jump1);

    /* Kill plane J1 */
    if (g->player.y > MAP_ROWS * TILE_SIZE) {
        g->player.lives--;
        g->player.x = 2 * TILE_SIZE;
        g->player.y = (MAP_ROWS - 3) * TILE_SIZE;
        g->player.vy = 0;
        if (g->player.lives <= 0) {
            g->scores_menu_active = 1;
            g->waiting_name = 1;
            g->input_len = 0;
            memset(g->input_name, 0, sizeof(g->input_name));
            SDL_StartTextInput();
            g->player.lives = 3;
            g->player.score = 0;
            g->start_ticks  = SDL_GetTicks();
        }
    }

    /* ====== Joueur 2 : IJKL (en mode MULTI seulement) ====== */
    if (g->display_mode == MODE_MULTI) {
        float vx2 = 0;
        if (ks[SDL_SCANCODE_J]) vx2 = -speed;
        if (ks[SDL_SCANCODE_L]) vx2 =  speed;
        int jump2 = ks[SDL_SCANCODE_I];

        move_player(&g->player2, &g->map, dt, vx2, jump2);

        /* Kill plane J2 */
        if (g->player2.y > MAP_ROWS * TILE_SIZE) {
            g->player2.lives--;
            g->player2.x = 6 * TILE_SIZE;
            g->player2.y = (MAP_ROWS - 3) * TILE_SIZE;
            g->player2.vy = 0;
            if (g->player2.lives <= 0) {
                g->player2.lives = 3;
                g->player2.score = 0;
            }
        }

        /* Score time J2 */
        g->player2.score++;   /* incrémenté avec J1 ci-dessous */

        /* Caméra Joueur 2 */
        updateCamera(&g->cam2, &g->player2,
                     MAP_COLS * TILE_SIZE, MAP_ROWS * TILE_SIZE);
    }

    /* Score: bonus temps chaque seconde (J1) */
    static Uint32 last_bonus = 0;
    Uint32 now = SDL_GetTicks();
    if (now - last_bonus > 1000) {
        g->player.score++;
        if (g->display_mode == MODE_MULTI) g->player2.score++;
        last_bonus = now;
    }

    /* ---- Mise à jour des plateformes ---- */
    updatePlatforms(&g->map, dt);

    /* ---- Caméra Joueur 1 (scroll quatre sens) ---- */
    updateCamera(&g->cam, &g->player,
                 MAP_COLS * TILE_SIZE, MAP_ROWS * TILE_SIZE);
}
