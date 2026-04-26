/* guide.c  – fenetre de guide (indicateurs) et sous-menu scores */
#include "game.h"

/* ================================================================== */
/*  GUIDE WINDOW                                                        */
/* ================================================================== */

void initGuide(GuideWindow *gw)
{
    memset(gw, 0, sizeof(*gw));
    gw->rect    = (SDL_Rect){ 60, 80, 400, 260 };
    gw->visible = 0;

    strncpy(gw->title, "Guide du Jeu", sizeof(gw->title) - 1);

    strncpy(gw->lines[0], "J1: Z/Haut=Saut  Q/Gauche  D/Droite", sizeof(gw->lines[0])-1);
    strncpy(gw->lines[1], "J1: Espace=Detruire tuile dessous",    sizeof(gw->lines[1])-1);
    strncpy(gw->lines[2], "J2: I=Saut  J=Gauche  L=Droite",      sizeof(gw->lines[2])-1);
    strncpy(gw->lines[3], "J2: N=Detruire tuile dessous",         sizeof(gw->lines[3])-1);
    strncpy(gw->lines[4], "M=Mode affichage  L=Niveau  P=Pause",  sizeof(gw->lines[4])-1);
    strncpy(gw->lines[5], "S=Scores  G=Fermer guide",             sizeof(gw->lines[5])-1);
    gw->line_count = 6;
}

void afficherGuide(SDL_Renderer *r, TTF_Font *font, const GuideWindow *gw)
{
    if (!gw->visible) return;

    /* Semi-transparent dark panel */
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 10, 10, 40, 210);
    SDL_RenderFillRect(r, &gw->rect);

    /* Border */
    SDL_SetRenderDrawColor(r, 100, 200, 255, 255);
    SDL_RenderDrawRect(r, &gw->rect);

    /* Title */
    SDL_Color title_col = {255, 220, 50, 255};
    render_text(r, font, gw->title,
                gw->rect.x + 12, gw->rect.y + 10, title_col);

    /* Separator */
    SDL_SetRenderDrawColor(r, 100, 200, 255, 180);
    SDL_RenderDrawLine(r,
        gw->rect.x + 8,  gw->rect.y + 34,
        gw->rect.x + gw->rect.w - 8, gw->rect.y + 34);

    /* Lines */
    SDL_Color text_col = {220, 220, 220, 255};
    for (int i = 0; i < gw->line_count; i++) {
        render_text(r, font, gw->lines[i],
                    gw->rect.x + 12,
                    gw->rect.y + 44 + i * 32,
                    text_col);
    }

    /* Close hint */
    SDL_Color hint = {140, 140, 140, 255};
    render_text(r, font, "[G] Fermer",
                gw->rect.x + 12, gw->rect.y + gw->rect.h - 24, hint);

    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
}

/* ================================================================== */
/*  SCORES SUB-MENU                                                     */
/* ================================================================== */

void initScores(GameState *g)
{
    g->score_count       = 0;
    g->scores_menu_active = 0;
    g->waiting_name      = 0;
    g->input_len         = 0;
    memset(g->input_name, 0, sizeof(g->input_name));
    memset(g->scores, 0, sizeof(g->scores));
    loadScores(g, "scores.dat");
}

void loadScores(GameState *g, const char *filename)
{
    FILE *f = fopen(filename, "r");
    if (!f) return;
    g->score_count = 0;
    while (g->score_count < MAX_SCORES) {
        ScoreEntry *e = &g->scores[g->score_count];
        if (fscanf(f, "%31s %d", e->name, &e->score) != 2) break;
        g->score_count++;
    }
    fclose(f);
}

void saveScores(GameState *g, const char *filename)
{
    FILE *f = fopen(filename, "w");
    if (!f) return;
    for (int i = 0; i < g->score_count; i++)
        fprintf(f, "%s %d\n", g->scores[i].name, g->scores[i].score);
    fclose(f);
}

/* Insert score keeping list sorted descending */
void addScore(GameState *g, const char *name, int score)
{
    if (g->score_count < MAX_SCORES)
        g->score_count++;

    /* Find insertion position */
    int pos = g->score_count - 1;
    while (pos > 0 && g->scores[pos-1].score < score) {
        g->scores[pos] = g->scores[pos-1];
        pos--;
    }
    strncpy(g->scores[pos].name, name, NAME_MAX_LEN - 1);
    g->scores[pos].score = score;

    saveScores(g, "scores.dat");
}

void afficherScoresMenu(GameState *g)
{
    SDL_Renderer *r    = g->renderer;
    TTF_Font     *font = g->font_small;
    TTF_Font     *big  = g->font_large;

    /* Overlay panel */
    SDL_Rect panel = { 150, 100, 500, 380 };
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 10, 10, 30, 230);
    SDL_RenderFillRect(r, &panel);
    SDL_SetRenderDrawColor(r, 255, 200, 0, 255);
    SDL_RenderDrawRect(r, &panel);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

    SDL_Color title_c = {255, 220, 50, 255};
    SDL_Color white   = {255, 255, 255, 255};
    SDL_Color grey    = {160, 160, 160, 255};
    SDL_Color gold    = {255, 200,  0, 255};
    SDL_Color green   = {100, 230, 100, 255};

    render_text(r, big,  "Meilleurs Scores", panel.x + 90, panel.y + 14, title_c);

    /* Column headers */
    SDL_SetRenderDrawColor(r, 255, 200, 0, 180);
    SDL_RenderDrawLine(r, panel.x+10, panel.y+56, panel.x+panel.w-10, panel.y+56);
    render_text(r, font, "#",      panel.x + 14,  panel.y + 60, gold);
    render_text(r, font, "Joueur", panel.x + 50,  panel.y + 60, gold);
    render_text(r, font, "Score",  panel.x + 320, panel.y + 60, gold);

    /* Entries */
    for (int i = 0; i < g->score_count && i < MAX_SCORES; i++) {
        char rank[4], sc[16];
        snprintf(rank, sizeof(rank), "%d.", i+1);
        snprintf(sc,   sizeof(sc),   "%d",  g->scores[i].score);

        SDL_Color row_c = (i == 0) ? gold : white;
        int y = panel.y + 88 + i * 38;
        render_text(r, font, rank,                  panel.x + 14,  y, row_c);
        render_text(r, font, g->scores[i].name,     panel.x + 50,  y, row_c);
        render_text(r, font, sc,                    panel.x + 320, y, row_c);
    }

    /* Name input area (when prompted) */
    if (g->waiting_name) {
        int iy = panel.y + panel.h - 90;
        SDL_SetRenderDrawColor(r, 255, 200, 0, 60);
        SDL_Rect input_bg = { panel.x+10, iy-4, panel.w-20, 32 };
        SDL_RenderFillRect(r, &input_bg);

        render_text(r, font, "Votre nom:", panel.x + 14, iy - 24, green);

        char display[NAME_MAX_LEN + 2];
        snprintf(display, sizeof(display), "%s_", g->input_name);
        render_text(r, font, display, panel.x + 14, iy, white);

        render_text(r, font, "[Entree] Valider  [Echap] Annuler",
                    panel.x + 14, panel.y + panel.h - 30, grey);
    } else {
        render_text(r, font, "[S] Fermer  [Echap] Quitter",
                    panel.x + 14, panel.y + panel.h - 30, grey);
    }
}
