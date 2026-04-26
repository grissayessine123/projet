/* display.c – afficher (mode mono / multi), scrolling, temps, guide */
#include "game.h"
#include <SDL2/SDL_image.h>

/* ------------------------------------------------------------------ */
/* Helper: render UTF-8 text                                           */
/* ------------------------------------------------------------------ */
void render_text(SDL_Renderer *r, TTF_Font *font, const char *text,
                 int x, int y, SDL_Color col)
{
    if (!font || !text || text[0] == '\0') return;
    SDL_Surface *surf = TTF_RenderUTF8_Blended(font, text, col);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
    SDL_Rect dst = { x, y, surf->w, surf->h };
    SDL_FreeSurface(surf);
    if (!tex) return;
    SDL_RenderCopy(r, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
}

/* ------------------------------------------------------------------ */
/* afficherTemps – calcul et affichage du temps depuis le lancement   */
/* ------------------------------------------------------------------ */
void afficherTemps(SDL_Renderer *r, TTF_Font *font, Uint32 start_ticks,
                   int x, int y)
{
    Uint32 elapsed = SDL_GetTicks() - start_ticks;
    Uint32 secs    = elapsed / 1000;
    Uint32 mins    = secs / 60;
    secs %= 60;

    char buf[32];
    snprintf(buf, sizeof(buf), "Temps: %02u:%02u", mins, secs);

    SDL_Color yellow = {255, 230, 50, 255};
    render_text(r, font, buf, x, y, yellow);
}

/* ------------------------------------------------------------------ */
/* updateCamera – scroll dans les quatre sens                          */
/* ------------------------------------------------------------------ */
void updateCamera(Camera *cam, const Player *p, int map_w_px, int map_h_px)
{
    /* Centre camera on player */
    int half_w = (VIEWPORT_COLS * TILE_SIZE) / 2;
    int half_h = (VIEWPORT_ROWS * TILE_SIZE) / 2;

    cam->cam_x = (int)p->x - half_w;
    cam->cam_y = (int)p->y - half_h;

    /* Clamp to map boundaries */
    if (cam->cam_x < 0) cam->cam_x = 0;
    if (cam->cam_y < 0) cam->cam_y = 0;
    int max_x = map_w_px - VIEWPORT_COLS * TILE_SIZE;
    int max_y = map_h_px - VIEWPORT_ROWS * TILE_SIZE;
    if (max_x > 0 && cam->cam_x > max_x) cam->cam_x = max_x;
    if (max_y > 0 && cam->cam_y > max_y) cam->cam_y = max_y;
}

/* ------------------------------------------------------------------ */
/* draw_viewport – dessine un viewport pour UN joueur donné           */
/*   p   : le joueur à afficher (position, score, vies)               */
/*   cam : la caméra associée à ce joueur                             */
/*   color : couleur du sprite joueur                                 */
/*   controls_hint : texte de contrôles à afficher en bas             */
/* ------------------------------------------------------------------ */
static void draw_viewport(GameState *g, SDL_Rect vp,
                          const Player *p, const Camera *cam,
                          SDL_Color sprite_col,
                          const char *controls_hint)
{
    /* 1. Background tiles avec la caméra de CE joueur */
    afficherBackground(g->renderer, &g->map, cam, vp);

    /* Remettre le viewport pour le HUD */
    SDL_RenderSetViewport(g->renderer, &vp);

    /* 2. Sprite du joueur : position relative à sa caméra */
    int px = (int)p->x - cam->cam_x;
    int py = (int)p->y - cam->cam_y;
    SDL_Rect prect = { px, py, TILE_SIZE - 4, TILE_SIZE - 2 };
    SDL_SetRenderDrawColor(g->renderer,
                           sprite_col.r, sprite_col.g, sprite_col.b, 255);
    SDL_RenderFillRect(g->renderer, &prect);

    /* Contour noir pour lisibilité */
    SDL_SetRenderDrawColor(g->renderer, 0, 0, 0, 200);
    SDL_RenderDrawRect(g->renderer, &prect);

    /* 3. HUD : score + vies */
    SDL_Color white = {255, 255, 255, 255};
    char hud[64];
    snprintf(hud, sizeof(hud), "Score: %d  Vies: %d", p->score, p->lives);
    render_text(g->renderer, g->font_small, hud, 8, 8, white);

    /* 4. Timer */
    afficherTemps(g->renderer, g->font_small, g->start_ticks,
                  vp.w - 130, 8);

    /* 5. Niveau */
    char lvl[24];
    snprintf(lvl, sizeof(lvl), "Niveau: %d", g->map.level);
    SDL_Color cyan = {100, 230, 255, 255};
    render_text(g->renderer, g->font_small, lvl, 8, 28, cyan);

    /* 6. Rappel contrôles en bas */
    SDL_Color grey = {180, 180, 180, 255};
    render_text(g->renderer, g->font_small, controls_hint,
                8, vp.h - 22, grey);
			    /* ===== JUMANJI LOGO UNDER SCORE (SINGLE BLOCK) ===== */
			do {
			    static SDL_Texture *tex = NULL;
			    static int loaded = 0;

			    if (!loaded) {
				IMG_Init(IMG_INIT_PNG);

				tex = IMG_LoadTexture(
				    g->renderer,
				    "/home/yassine/Téléchargements/sdl_project (2)/sdl_project/assets/jumanji-tvg_logo-removebg-preview.png"
				);

				if (!tex) {
				    printf("IMG_LoadTexture error: %s\n", IMG_GetError());
				}

				loaded = 1;
			    }

			    if (tex) {
				SDL_Rect r;
				r.x = 8;
				r.y = 35;   /* directly under score text */
				r.w = 70;
				r.h = 70;

				SDL_RenderCopy(g->renderer, tex, NULL, &r);
			    }
			} while (0);
			/* ================================================ */

    SDL_RenderSetViewport(g->renderer, NULL);
}

/* ------------------------------------------------------------------ */
/* afficher – MAJ : prend le mode (MODE_MONO / MODE_MULTI) en param   */
/* ------------------------------------------------------------------ */
void afficher(GameState *g, int mode)
{
    SDL_SetRenderDrawColor(g->renderer, 0, 0, 0, 255);
    SDL_RenderClear(g->renderer);

    SDL_Color red  = {255,  80,  80, 255};   /* Joueur 1 */
    SDL_Color blue = { 80, 140, 255, 255};   /* Joueur 2 */

    if (mode == MODE_MONO) {
        /* Plein écran : on affiche uniquement le Joueur 1 */
        SDL_Rect vp = { 0, 0, WINDOW_W, WINDOW_H };
        draw_viewport(g, vp,
                      &g->player, &g->cam,
                      red,
                      "ZQSD/Fleches:deplacer  G:guide  M:mode  S:scores  L:niveau");
    } else {
        /* ---- Split-screen : chaque moitié a sa propre caméra ---- */
        int half = WINDOW_W / 2;
        SDL_Rect left_vp  = { 0,    0, half, WINDOW_H };
        SDL_Rect right_vp = { half, 0, half, WINDOW_H };

        /* Panneau gauche  = Joueur 1 (ZQSD / flèches) */
        draw_viewport(g, left_vp,
                      &g->player, &g->cam,
                      red,
                      "J1: Z=saut  Q/D=depl  Espace=detruire");

        /* Panneau droit   = Joueur 2 (IJKL) */
        draw_viewport(g, right_vp,
                      &g->player2, &g->cam2,
                      blue,
                      "J2: I=saut  J/L=depl  N=detruire");

        /* Ligne séparatrice */
        SDL_RenderSetViewport(g->renderer, NULL);
        SDL_SetRenderDrawColor(g->renderer, 255, 255, 0, 255);
        SDL_RenderDrawLine(g->renderer, half, 0, half, WINDOW_H);

        /* Labels au-dessus de chaque panneau */
        SDL_Color yellow = {255, 220, 0, 255};
        render_text(g->renderer, g->font_small,
                    "-- Joueur 1 --", half/2 - 48, 2, yellow);
        render_text(g->renderer, g->font_small,
                    "-- Joueur 2 --", half + half/2 - 48, 2, yellow);
    }

    /* Guide overlay */
    if (g->show_guide)
        afficherGuide(g->renderer, g->font_small, &g->guide);

    /* Scores overlay */
    if (g->scores_menu_active)
        afficherScoresMenu(g);

    SDL_RenderPresent(g->renderer);
}
