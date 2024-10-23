#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <stdbool.h>
#include <stdio.h>

#define WIDTH  800
#define HEIGHT 600

#define N 3

#define CELL_WIDTH  (WIDTH / N)
#define CELL_HEIGHT (HEIGHT / N)

#define BACKGROUND_COLOR 28, 28, 28, 255

const SDL_Color grid_color     = {255, 255, 255, 255};
const SDL_Color player_x_color = {255, 0, 0, 255};
const SDL_Color player_o_color = {50, 100, 255, 255};
const SDL_Color tie_color      = {50, 50, 50, 255};

typedef enum
{
    EMPTY    = 0,
    PLAYER_X = 1,
    PLAYER_O = 2,
} Player;

typedef enum
{
    RUNNING,
    PLAYER_X_WON,
    PLAYER_O_WON,
    TIE,
    QUIT
} State;

typedef struct
{
    Player board[N * N];
    Player player;
    State state;
} Game;

void render_grid(SDL_Renderer *ren, const SDL_Color *color)
{
    SDL_SetRenderDrawColor(ren, color->r, color->g, color->b, 255);

    for (int i = 1; i < N; ++i)
    {
        SDL_RenderDrawLine(ren, i * CELL_WIDTH, 0, i * CELL_WIDTH, HEIGHT);
        SDL_RenderDrawLine(ren, 0, i * CELL_HEIGHT, WIDTH, i * CELL_HEIGHT);
    }
}

void render_player(SDL_Renderer *ren, Player player, int row, int column)
{
    const float half_box_side = fmin(CELL_WIDTH, CELL_HEIGHT) * 0.25;
    const float center_x      = CELL_WIDTH * 0.5 + column * CELL_WIDTH;
    const float center_y      = CELL_HEIGHT * 0.5 + row * CELL_HEIGHT;
    SDL_Color color           = grid_color;

    switch (player)
    {
        case PLAYER_X:
            {
                color = player_x_color;

                thickLineRGBA(
                    ren,
                    center_x - half_box_side,
                    center_y - half_box_side,
                    center_x + half_box_side,
                    center_y + half_box_side,
                    10,
                    color.r,
                    color.b,
                    color.b,
                    color.a
                );
                thickLineRGBA(
                    ren,
                    center_x + half_box_side,
                    center_y - half_box_side,
                    center_x - half_box_side,
                    center_y + half_box_side,
                    10,
                    color.r,
                    color.b,
                    color.b,
                    color.a
                );
            }
            break;
        case PLAYER_O:
            {
                color = player_o_color;

                filledCircleRGBA(
                    ren,
                    center_x,
                    center_y,
                    half_box_side,
                    color.r,
                    color.g,
                    color.b,
                    color.a
                );
                filledCircleRGBA(
                    ren,
                    center_x,
                    center_y,
                    half_box_side - 10,
                    28,
                    28,
                    28,
                    color.a
                );
            }
            break;
        default:
            break;
    }
}

void render_board(SDL_Renderer *ren, const Player board[N * N])
{
    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            Player player = board[i * N + j];

            render_player(ren, player, i, j);
        }
    }
}

bool check_player_won(Game *game, Player player)
{
    int row_count    = 0;
    int column_count = 0;

    int diag1_count = 0;
    int diag2_count = 0;

    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            if (game->board[i * N + j] == player)
                row_count++;

            if (game->board[j * N + i] == player)
                column_count++;
        }

        if (row_count >= N || column_count >= N)
            return true;

        row_count    = 0;
        column_count = 0;

        if (game->board[i * N + i] == player)
        {
            diag1_count++;
        }
        if (game->board[i * N + N - i - 1] == player)
        {
            diag2_count++;
        }
    }

    return diag1_count >= N || diag2_count >= N;
}

int count_cells(const Player board[N * N], Player cell)
{
    int count = 0;

    for (int i = 0; i < N * N; ++i)
    {
        if (board[i] == cell)
            count++;
    }

    return count;
}

void game_over_condition(Game *game)
{
    if (check_player_won(game, PLAYER_X))
    {
        game->state = PLAYER_X_WON;
    }
    else if (check_player_won(game, PLAYER_O))
    {
        game->state = PLAYER_O_WON;
    }
    else if (count_cells(game->board, EMPTY) == 0)
    {
        game->state = TIE;
    }
}

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO) == -1)
    {
        fprintf(stderr, "ERROR: Failed to Init video: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_Window *win =
        SDL_CreateWindow("Tic tac toe", 0, 0, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (win == NULL)
    {
        fprintf(stderr, "ERROR: Failed to create window: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (ren == NULL)
    {
        fprintf(
            stderr, "ERROR: Failed to create renderer: %s\n", SDL_GetError()
        );
        exit(1);
    }

    SDL_Event event;
    Game game = {
        .board  = {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
        .player = PLAYER_X,
        .state  = RUNNING
    };

    while (game.state != QUIT)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    game.state = QUIT;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    {
                        if (game.state == RUNNING)
                        {
                            int row    = event.button.y / CELL_HEIGHT;
                            int column = event.button.x / CELL_WIDTH;

                            if (game.board[row * N + column] == EMPTY)
                            {
                                game.board[row * N + column] = game.player;
                                switch (game.player)
                                {
                                    case PLAYER_X:
                                        game.player = PLAYER_O;
                                        break;
                                    case PLAYER_O:
                                        game.player = PLAYER_X;
                                        break;
                                    default:
                                        break;
                                }

                                game_over_condition(&game);
                            }
                        }
                        else
                        {
                            for (int i = 0; i < N * N; ++i)
                            {
                                game.board[i] = EMPTY;
                            }
                            game.player = PLAYER_X;
                            game.state  = RUNNING;
                        }
                    }
                    break;
            }
        }

        SDL_SetRenderDrawColor(ren, BACKGROUND_COLOR);
        SDL_RenderClear(ren);

        const SDL_Color *render_grid_color =
            game.state == RUNNING
                ? &grid_color
                : (game.state == PLAYER_X_WON
                       ? &player_x_color
                       : (game.state == PLAYER_O_WON ? &player_o_color
                          : game.state == TIE        ? &tie_color
                                                     : &tie_color));

        render_grid(ren, render_grid_color);
        render_board(ren, game.board);

        SDL_RenderPresent(ren);
    }

    SDL_DestroyWindow(win);
    SDL_DestroyRenderer(ren);
    SDL_Quit();
}
