/* main.c – initialisation SDL2 et boucle de jeu principale */
#include "game.h"




#include <SDL2/SDL_image.h>

#include <stdio.h>   
#include <unistd.h>  
#include <fcntl.h>  
#include <errno.h>   
#include <termios.h>  
#include <string.h>  
#include <sys/ioctl.h>
#include <stdint.h> 
#include "serie.h"
/* ------------------------------------------------------------------ */
int game_init(GameState *g)
{   
    memset(g, 0, sizeof(*g));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 0;
    }
    if (TTF_Init() != 0) {
        fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
        return 0;
    }

    g->window = SDL_CreateWindow(
        "Jeu de Plateformes SDL2",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H,
        SDL_WINDOW_SHOWN);
    if (!g->window) {
        fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        return 0;
    }

    g->renderer = SDL_CreateRenderer(
        g->window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g->renderer) {
        fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        return 0;
    }

    /* Load fonts – try common system paths */
    const char *font_paths[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSansBold.ttf",
        "/usr/share/fonts/TTF/DejaVuSans-Bold.ttf",
        NULL
    };

    g->font_large = NULL;
    g->font_small = NULL;
    for (int i = 0; font_paths[i]; i++) {
        if (!g->font_large)
            g->font_large = TTF_OpenFont(font_paths[i], 22);
        if (!g->font_small)
            g->font_small = TTF_OpenFont(font_paths[i], 15);
        if (g->font_large && g->font_small) break;
    }

    if (!g->font_large || !g->font_small) {
        fprintf(stderr, "Could not load any TTF font. Continuing without text.\n");
        /* Not fatal – game will run without text */
    }

    /* Player 1 */
    g->player.x     = 2 * TILE_SIZE;
    g->player.y     = (MAP_ROWS - 3) * TILE_SIZE;
    g->player.score = 0;
    g->player.lives = 3;

    /* Player 2 – démarre un peu plus loin */
    g->player2.x     = 6 * TILE_SIZE;
    g->player2.y     = (MAP_ROWS - 3) * TILE_SIZE;
    g->player2.score = 0;
    g->player2.lives = 3;

    /* Map / background */
    initBackground(&g->map, LEVEL_1);

    /* Camera */
    g->cam.cam_x  = 0;  g->cam.cam_y  = 0;
    g->cam2.cam_x = 0;  g->cam2.cam_y = 0;

    /* Display mode */
    g->display_mode = MODE_MONO;

    /* Guide */
    initGuide(&g->guide);

    /* Scores */
    initScores(g);

    /* Timer */
    g->start_ticks = SDL_GetTicks();

    g->running = 1;
    g->paused  = 0;
    
    
    return 1;
    
    
}

/* ------------------------------------------------------------------ */
void game_cleanup(GameState *g)
{
    if (g->font_large) TTF_CloseFont(g->font_large);
    if (g->font_small) TTF_CloseFont(g->font_small);
    if (g->renderer)   SDL_DestroyRenderer(g->renderer);
    if (g->window)     SDL_DestroyWindow(g->window);
    TTF_Quit();
    SDL_Quit();
}

/* ------------------------------------------------------------------ */
int main(int argc, char *argv[])
{
    (void)argc; (void)argv;
char buffer[2];                   // un buffer
    int i;

    // ouverture du port à 115200 bauds
    int fd = serialport_init("/dev/ttyACM0", 9600);
    if (fd==-1) return -1;

    GameState g;
    if (!game_init(&g)) {
        game_cleanup(&g);
        return EXIT_FAILURE;
    }

    Uint32 prev = SDL_GetTicks();
    const float MAX_DT = 1.0f / 20.0f;   /* cap at 50 ms */

    while (g.running) {
     serialport_read_until(fd, buffer, '\r', 1, 50);

        // suppression de la fin de ligne
        for (i=0 ; buffer[i]!='\r' && i<2 ; i++);
        buffer[1] = 0;
        if(strstr(buffer,"r"))
        {
                g.player.x    +=10;
        }
         if(strstr(buffer,"l"))
        {
                g.player.x    -=10;
        }
      
        // écriture du résultat
        printf("%s", buffer);
        Uint32 now = SDL_GetTicks();
        float dt   = (now - prev) / 1000.0f;
        if (dt > MAX_DT) dt = MAX_DT;
        prev = now;

        handle_events(&g);
        update(&g, dt);
        afficher(&g, g.display_mode);
       
        /* Draw Jumanji image under score */


        /* Frame cap ~60 FPS */
        Uint32 frame_ms = SDL_GetTicks() - now;
        if (frame_ms < 16) SDL_Delay(16 - frame_ms);
    }
     // fermeture du port
    serialport_flush(fd);
    serialport_close(fd);
    game_cleanup(&g);
    return EXIT_SUCCESS;
}
