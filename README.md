# Jeu de Plateformes SDL2

Projet complet implémentant toutes les fonctionnalités demandées.

## Structure du projet

```
sdl_project/
├── Makefile
├── README.md
├── include/
│   └── game.h          ← toutes les structures et prototypes
└── src/
    ├── main.c          ← init SDL2, boucle de jeu
    ├── background.c    ← initBackground / afficherBackground (Level1/Level2)
    ├── platforms.c     ← initPlatforms / updatePlatforms / afficherPlatforms
    ├── display.c       ← afficher(mode) + updateCamera + afficherTemps
    ├── guide.c         ← initGuide / afficherGuide + initScores / afficherScoresMenu
    └── events.c        ← handle_events / update (physique, collisions)
```

## Fonctionnalités implémentées

| Spécification | Fichier | Fonction(s) |
|---|---|---|
| Background fixe/mobile/destructible Level1/Level2 | `background.c` | `initBackground()`, `afficherBackground()` |
| Scrolling 4 directions | `display.c` | `updateCamera()` |
| Partage d'écran (mono/multi) | `display.c` | `afficher(g, MODE_MONO/MODE_MULTI)` |
| Gestion du temps | `display.c` | `afficherTemps()` |
| Fenêtre de guide (indicateurs) | `guide.c` | `initGuide()`, `afficherGuide()` |
| Sous-menu meilleurs scores | `guide.c` | `initScores()`, `afficherScoresMenu()`, `addScore()` |

## Dépendances

```bash
sudo apt install libsdl2-dev libsdl2-ttf-dev
```

## Compilation & lancement

```bash
make          # compiler
make run      # compiler + lancer
make clean    # nettoyer
```

## Contrôles en jeu

| Touche | Action |
|--------|--------|
| Z / ↑ | Sauter |
| Q / ← | Aller à gauche |
| D / → | Aller à droite |
| S / ↓ | Descendre |
| ESPACE | Détruire la plateforme sous le joueur |
| G | Afficher/masquer le guide |
| M | Basculer mode affichage (Mono ↔ Multi) |
| L | Changer de niveau (Level 1 ↔ Level 2) |
| S | Ouvrir/fermer le sous-menu scores |
| P | Pause |
| Échap | Quitter |

## Types de plateformes

- **Vert** – Plateforme fixe (`PLATFORM_FIXED`)
- **Bleu** – Plateforme mobile (oscille horizontalement) (`PLATFORM_MOBILE`)
- **Orange** – Plateforme destructible (ESPACE pour détruire) (`PLATFORM_DESTRUCTIBLE`)

## Mode affichage

- **MODE_MONO** : un seul viewport plein écran
- **MODE_MULTI** : écran partagé en deux (split-screen côte à côte)

La fonction `afficher(GameState *g, int mode)` prend le mode en paramètre
comme demandé dans les spécifications.

## Scores

- Les scores sont triés et sauvegardés dans `scores.dat`
- À la fin d'une partie (plus de vies), le joueur saisit son nom
- Persistance entre les sessions via fichier texte

## Format afficherTemps

```c
void afficherTemps(SDL_Renderer *r, TTF_Font *font, Uint32 start_ticks, int x, int y);
// Affiche: "Temps: MM:SS"
```
