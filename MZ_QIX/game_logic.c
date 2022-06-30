#include "game_logic.h"

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define RED_RGB888 0xFF0000
#define WHITE_RGB888 0xFFFFFF
#define RED 0xF2EA
#define GREEN 0x8D8C
#define BLUE 0x1B12
#define TRAIL_COLOR 0xE79A
#define BORDER_COLOR 0xF567
#define PLAYER_COLOR 0xF75B
#define QIX_COLOR 0xF2EA
#define BACKGROUND_COLOR 0x00E4
#define FILL_COLOR 0xFFFF
#define PSEUDO_COLOR 0xA26A

#define QIX_DEFAULT_SPEED 4
#define PLAYER_DEFAULT_SPEED 2
#define NEXT_ACTION_TRIGGER 100

#define SCORE_X 300
#define SCORE_Y 20  

#define NQIXES 3
#define ENTITY_WIDTH 10
#define ENTITY_HEIGHT 10

#define TRAIL_WIDTH 4
#define BORDER_WIDTH 10
#define BORDER_HEIGHT 10

#define PLAYER_INVUL_ANIM 300

/// Structure for representing entitites: player and qixes.
typedef struct {
    bool invul; ///< True if the entity is invincible, false otherwise.
    int hit_anim_counter; ///< Animation counter.
    int next_action_counter; ///< Next action counter
                             /// (used for changing the direction of qixes).
    int xx; ///< X-coordinate of the upper left corner of the entity.
    int yy; ///< Y-coordinate of the upper left corner of the entity.
    int direction; ///< Current direction of the entity.
    int speed; ///< Speed in pixels of the entity.
    int HP; ///< Current amount of HP of the entity.
    rgb565_t color; ///< Color of the entity.
} entity_t;

static int score = 0;
static rgb565_t prev_color = BORDER_COLOR;

static const rgb565_t qix_color[] = {RED, GREEN, BLUE};
static rgb565_t background[SCREEN_WIDTH][SCREEN_HEIGHT];
static entity_t player;
static entity_t qixes[NQIXES];
static const struct timespec gameloop_delay
    = {.tv_sec = 0, .tv_nsec = 16 * 1000 * 1000};

/// <------------ Start implementation functions declaration ------------>

/// Initializes player settings.
static void init_player();

/// Initializes the given qix settings.
/// \param qix Qix to be initialized.
static void init_qix(entity_t *qix);

/// Adds quarter of trail under the player based on its direction.
/// \param player Player for whom should the quarter of trail be added.
static void add_whole_trail_to_background(const entity_t *player);

/// Adds trail under the player.
/// \param player Player for whom should the quarter of trail be added.
static void add_trail_to_background(const entity_t *player);

/// Buffers the background into screen buffer.
static void draw_background();

/// Buffer all the entitites (player, qixes) into screen buffer.
static void draw_entities();

/// Buffers level into the screen buffer and updates screen.
static void draw_level();

/// Buffers the given entity into screen buffer.
/// \param entity Entity to be buffered.
/// \param color Color of the entity to be buffered.
static void redraw_entity(const entity_t *entity, rgb565_t color);

/// Uses floodfill algorithm to fill the least possible area around
/// the current position of the player based on its direction.
static void floodfill_least_area();

/// Should not be called directly, use pseudo_floodfill() instead.
static int __pseudo_floodfill(int x, int y, rgb565_t old_color);

/// Counts number of pixels that will be filled when the actual
/// floodfill function is used. Used for determining the least shape
/// to be floodfilled.
/// \param x X-coordinate of the pixel around which should
/// pseudo_floodfill() be called.
/// \param y Y-coordinate of the pixel around which should
/// pseudo_floodfill() be called.
/// \param old_color Old color to be filled (counted).
/// \return number of pixels that will be filled when the actual
/// floodfill function is used
static int pseudo_floodfill(int x, int y, rgb565_t old_color);

/// Floodills the area around the given coordinates of the pixel.
/// \param x X-coordinate of the pixel around which should
/// floodfill() be called.
/// \param y Y-coordinate of the pixel around which should
/// floodfill() be called.
/// \param old_color Old color to be filled.
/// \param new_color New color to be filled with.
static void floodfill(int x, int y, rgb565_t old_color, rgb565_t new_color);

/// Erases entitites from the screen buffer (paints with BACKGROUND_COLOR).
static void erase_entities();

/// Erases the given entity from the screen buffer (paints with BACKGROUND_COLOR).
/// \param entity Entity to be erased.
static void erase_entity(const entity_t *entity);

/// Updates the given entity by its speed and direction. Does not check if qix
/// should go in the opposite direction.
/// \param qix Entity whose coordinates should be updated by its speed.
static void update_no_check(entity_t *qix);

/// Updates the given entity by its speed and direction. Checks if qix
/// should go in the opposite direction.
/// \param qix Entity whose coordinates should be updated by its speed.
static void check_and_update(entity_t *qix);

/// Updates all the qixes.
static void update_qixes();

/// Updates player.
static void update_player();

/// Updates all the entitites (qixes and player).
static void update_entitites();

static bool entity_speed_stop_if_hit_color(entity_t *entity, rgb565_t color,
                                           int default_speed);

/// Buffers the score into the screen buffer.
/// \param score Score to be buffered into the screen buffer.
static void update_and_redraw_score(int score);

/// Changes all pixels of the given color with the other color in the
/// background buffer.
/// \param old_color Old color to be repainted.
/// \param new_color New color to be repainted with.
static void repaint(rgb565_t old_color, rgb565_t new_color);

/// Check if the given entity is inside the given color.
/// \param entity Entity to be checked if is inside the given color.
/// \param color Color to be checked.
/// \return true if the given entity is inside the given color, false otherwise
static bool collision_full_body(const entity_t *entity, rgb565_t color);

/// Check if the given entity has touched any pixel of the given color.
/// \param entity Entity to be checked if touched any pixel of the given color.
/// \param color Color of the pixel.
/// \return true if the given entity has touched any pixel of the given color,
/// false otherwise
static bool collision_with_color(const entity_t *entity, rgb565_t color);

/// Check if the given entity is inside screen (excluding borders).
/// \param entity Entity to be checked if is inside screen.
/// \return true if the given entity is inside screen (excluding borders),
/// false otherwise
static bool inside_screen(const entity_t *entity);

/// Check if the the given player touched the given qix.
/// \param player Player to be checked if touched the given qix.
/// \param qix Qix to be checked for collisions with the given player.
/// \return true if the the given player touched the given qix, false otherwise
static bool collission_check_single(const entity_t *player, const entity_t *qix);

/// Check if the the given player touched any of the given qixes.
/// \param player Player to be checked if touched any of the given qixes.
/// \param qixes Qixes to be checked for collisions with the given player.
/// \return true if the the given player touched any of the given qixes, false
/// otherwise
static bool collission_check(const entity_t *player, const entity_t *qixes);

/// Get the opposing direction.
/// \param input Direction for which should the opposing direction returned.
/// \return the opposing direction on success, NO_INPUT if the given parameter
/// is not a direction
static input_t opposing_direction(enum input_t input);

/// <------------ End mplementation functions declaration ------------>

void init_gamelogic()
{
    init_player();
    for (int i = 0; i < NQIXES; ++i) {
        init_qix(qixes + i);
        qixes[i].color = qix_color[i % NQIXES];
    }

    for (int y = 0; y < SCREEN_HEIGHT; ++y) {
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            if (x >= BORDER_WIDTH && x < SCREEN_WIDTH - BORDER_WIDTH
                && y >= BORDER_HEIGHT && y < SCREEN_HEIGHT - BORDER_HEIGHT) {
                background[x][y] = BACKGROUND_COLOR;
            } else {
                background[x][y] = BORDER_COLOR;
            }
        }
    }

    score = 0;
    prev_color = BORDER_COLOR;
}

void start_new_game()
{
    draw_level();

    update_led_rgb1(0);
    update_led_rgb2(0);

    bool running = true;
    while (running) {
        input_t input = input_handler();
        switch (input) {
        case UP:
        case DOWN:
        case LEFT:
        case RIGHT:
            player.direction = input;
            break;
        case BACK:
        case EXIT:
            running = false;
        default:
            break;
        }

        if (!running) {
            break;
        }

        erase_entities();
        update_entitites();

        if (!player.invul) {
            if (collission_check(&player, qixes)) {
                player.invul = true;
                --player.HP;
            }
        } else {
            unsigned int color = player.color == PLAYER_COLOR ? WHITE_RGB888 : RED_RGB888;
            update_led_rgb1(color);
            update_led_rgb2(color);
        }

        draw_entities();
        
        uint32_t LED = player.HP >= 4 ? 0xffffffff 
            : player.HP == 3 ? 0xffffffff<<8
            : player.HP == 2 ? 0xffffffff<<16
            : player.HP == 1 ? 0xffffffff<<24
            : 0;

        update_led_line(LED);

        if (player.HP <= 0) {
            draw_end_game_screen();
            return;
        }

        if (score >= SCREEN_SIZE * 0.8) {
            draw_win_game_screen();
            return;
        }

        update_and_redraw_score(score);

        update_screen();

        clock_nanosleep(CLOCK_MONOTONIC, 0, &gameloop_delay, NULL);
    }
}

static void init_player()
{
    player.invul = false;
    player.hit_anim_counter = 0;
    player.next_action_counter = 0;
    player.xx = 0;
    player.yy = 0;
    player.direction = NO_INPUT;
    player.speed = PLAYER_DEFAULT_SPEED;
    player.HP = 4;
    player.color = PLAYER_COLOR;
}

static void init_qix(entity_t *qix)
{
    qix->invul = false;
    qix->hit_anim_counter = 0;
    qix->next_action_counter = 0;
    qix->xx = SCREEN_WIDTH / 2;
    qix->yy = SCREEN_HEIGHT / 2;
    qix->direction = (rand() % RIGHT) + 1;
    qix->speed = QIX_DEFAULT_SPEED;
    qix->HP = INT32_MAX;
    qix->color = QIX_COLOR;  
}

static void draw_background()
{
    for (int y = 0; y < SCREEN_HEIGHT; ++y) {
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            draw_pixel(x, y, background[x][y]);
        }
    }
}

static void draw_entities()
{
    draw_background();
    redraw_entity(&player, player.color);
    for (int i = 0; i < NQIXES; ++i) {
        redraw_entity(qixes + i, qixes[i].color);
    }
}

static void erase_entities()
{
    erase_entity(&player);
    for (int i = 0; i < NQIXES; ++i) {
        erase_entity(qixes + i);
    }
}

static void update_entitites()
{
    update_qixes();
    update_player();
}

static void draw_level()
{
    draw_background();
    draw_entities();
    update_screen();
}

static bool inside_screen(const entity_t *entity)
{
    return entity->xx >= BORDER_WIDTH + entity->speed
        && entity->xx + ENTITY_WIDTH < SCREEN_WIDTH - BORDER_WIDTH
        && entity->yy >= BORDER_HEIGHT + entity->speed
        && entity->yy + ENTITY_HEIGHT < SCREEN_HEIGHT - BORDER_HEIGHT;
}

static void erase_entity(const entity_t *entity)
{
    for (int i = 0; i < ENTITY_HEIGHT; ++i) {
        for (int j = 0; j < ENTITY_WIDTH; ++j) {
            draw_pixel(entity->xx + j, entity->yy + i, BACKGROUND_COLOR);
        }
    }
}

static void redraw_entity(const entity_t *entity, rgb565_t color)
{
    for (int i = 0; i < ENTITY_HEIGHT; ++i) {
        for (int j = 0; j < ENTITY_WIDTH; ++j) {
            draw_pixel(entity->xx + j, entity->yy + i, color);
        }
    }

    if (player.invul && (player.hit_anim_counter++%10 == 0)) {
        player.color = player.color == PLAYER_COLOR ? RED : PLAYER_COLOR;
        if (player.hit_anim_counter > 120) {
            player.invul = false;
            player.hit_anim_counter = 0;
            player.color = PLAYER_COLOR;
            update_led_rgb1(0);
            update_led_rgb2(0);
        }
    }
}

static bool collission_check_single(const entity_t *player, const entity_t *qix)
{
    return( ( (player->xx >= qix->xx && player->xx <= qix->xx+ENTITY_WIDTH)
      || (player->xx+ENTITY_WIDTH >= qix->xx && player->xx+ENTITY_WIDTH <= qix->xx+ENTITY_WIDTH) )
      && ((player->yy >= qix->yy && player->yy <= qix->yy+ENTITY_HEIGHT)
      || (player->yy+ENTITY_HEIGHT >= qix->yy && player->yy+ENTITY_HEIGHT <= qix->yy+ENTITY_HEIGHT)));
}

static bool collission_check(const entity_t *player, const entity_t *qixes)
{
    bool ret = false;

    for (int i = 0; i < NQIXES; i++)
    {
        if (collission_check_single(player, qixes + i)) {
            ret = true;
        }
    }
    
    return ret;
}

static input_t opposing_direction(enum input_t input)
{
    switch (input) {
        case UP: return DOWN;
        case DOWN: return UP;
        case LEFT: return RIGHT;
        case RIGHT: return LEFT;
        default: return NO_INPUT;
    }
}

static void update_no_check(entity_t *qix)
{
    switch (qix->direction) {
    case UP:
        qix->yy = qix->yy - qix->speed < 0 ? 0 : qix->yy - qix->speed;
        break;
    case DOWN:
        qix->yy = qix->yy + qix->speed + ENTITY_HEIGHT >= SCREEN_HEIGHT
            ? SCREEN_HEIGHT - ENTITY_HEIGHT : qix->yy + qix->speed;
        break;
    case LEFT:
        qix->xx = qix->xx - qix->speed < 0 ? 0 : qix->xx - qix->speed;
        break;
    case RIGHT:
        qix->xx = qix->xx + qix->speed + ENTITY_WIDTH >= SCREEN_WIDTH
            ? SCREEN_WIDTH - ENTITY_WIDTH : qix->xx + qix->speed;
        break;
    default:
        break;
    }
}

static void check_and_update(entity_t *qix)
{
    if (inside_screen(qix)) {
        if (qix->next_action_counter++ > NEXT_ACTION_TRIGGER){
            qix->direction = (rand() % RIGHT) + 1;
            qix->next_action_counter = 0;
        }

        if(entity_speed_stop_if_hit_color(qix, FILL_COLOR, QIX_DEFAULT_SPEED)){
            qix->direction = opposing_direction(qix->direction);
        }

        if(collision_full_body(qix, FILL_COLOR)){
            qix->speed = 0;
        }


        if (!qix->invul && !player.invul) {
            if (collision_with_color(qix, TRAIL_COLOR)) {
                qix->direction = opposing_direction(qix->direction);
                player.invul = true;
                qix->invul = true;
                player.HP--;
            }
        }else if (qix->hit_anim_counter++>5) {
            qix->hit_anim_counter = 0;
            qix->invul = false;
        }
    } else {
        qix->direction = opposing_direction(qix->direction);
    }

    update_no_check(qix);
}

static void update_qixes()
{
    for (int i = 0; i < NQIXES; ++i) {
        check_and_update(qixes + i);
    }
}

static void add_trail_to_background(const entity_t *player)
{
    int midx = player->xx + ((ENTITY_WIDTH - TRAIL_WIDTH + 1) / 2);
    int midy = player->yy + ((ENTITY_HEIGHT - TRAIL_WIDTH + 1) / 2);

    int x1, x2, y1, y2;
    x1 = midx;
    x2 = x1 + TRAIL_WIDTH;
    y1 = midy;
    y2 = y1 + TRAIL_WIDTH;

    for (int y = y1; y < y2; ++y) {
        for (int x = x1; x < x2; ++x) {
            if (background[x][y] == BACKGROUND_COLOR) {
                background[x][y] = TRAIL_COLOR;
            }
        }
    }    
}

static void add_whole_trail_to_background(const entity_t *player)
{
    int midx = player->xx + ((ENTITY_WIDTH - TRAIL_WIDTH + 1) / 2);
    int midy = player->yy + ((ENTITY_HEIGHT - TRAIL_WIDTH + 1) / 2);

    int x1, x2, y1, y2;
    x1 = midx;
    x2 = x1 + TRAIL_WIDTH;
    y1 = midy;
    y2 = y1 + TRAIL_WIDTH;

    switch (player->direction) {
    case UP:
        y1 = player->yy;
        y2 = y1 + ENTITY_HEIGHT / 2;
        break;
    case DOWN:
        y2 = player->yy + ENTITY_HEIGHT;
        break;
    case LEFT:
        x1 = player->xx;
        x2 = x1 + ENTITY_WIDTH / 2;
        break;
    case RIGHT:
        x2 = player->xx + ENTITY_WIDTH;
        break;
    }

    for (int y = y1; y < y2; ++y) {
        for (int x = x1; x < x2; ++x) {
            if (background[x][y] == BACKGROUND_COLOR) {
                background[x][y] = TRAIL_COLOR;
            }
        }
    }
}

static bool collision_full_body(const entity_t *entity, rgb565_t color)
{
    return background[entity->xx][entity->yy] == color
        && background[entity->xx+ENTITY_WIDTH][entity->yy] == color
        && background[entity->xx][entity->yy+ENTITY_HEIGHT] == color
        && background[entity->xx+ENTITY_WIDTH][entity->yy+ENTITY_HEIGHT] == color;
}

static bool collision_with_color(const entity_t *e, rgb565_t color)
{
    return background[e->xx][e->yy] == color
        || background[e->xx+ENTITY_WIDTH][e->yy] == color
        || background[e->xx][e->yy+ENTITY_HEIGHT] == color
        || background[e->xx+ENTITY_WIDTH][e->yy+ENTITY_HEIGHT] == color;
}

static void update_prev_color()
{
    int midx = player.xx + ENTITY_WIDTH / 2;
    int midy = player.yy + ENTITY_HEIGHT / 2;

    switch (player.direction) {
    case UP:
        prev_color = background[midx][midy - 1];
        break;
    case LEFT:
        prev_color = background[midx - 1][midy];
        break;
    case DOWN:
    case RIGHT:
        prev_color = background[midx][midy];
        break;
    }
}

static bool entity_speed_stop_if_hit_color(entity_t *entity, rgb565_t color, int default_speed){

    if ((background[entity->xx][entity->yy] == color 
    && background[entity->xx+ENTITY_WIDTH][entity->yy] == color 
    && entity->direction == UP) 
     || 
    (background[entity->xx][entity->yy+ENTITY_HEIGHT] == color 
    && background[entity->xx+ENTITY_WIDTH][entity->yy+ENTITY_HEIGHT] == color 
    && entity->direction == DOWN)
    ||
     (background[entity->xx][entity->yy] == color 
    && background[entity->xx][entity->yy+ENTITY_HEIGHT] == color 
    && entity->direction == LEFT )
    || 
      (background[entity->xx+ENTITY_WIDTH][entity->yy] == color 
    && background[entity->xx+ENTITY_WIDTH][entity->yy+ENTITY_HEIGHT] == color 
    && entity->direction == RIGHT))
    {
        return true;
    } else {
        return false;
    }
}

static void update_player()
{
    update_prev_color();
    add_trail_to_background(&player);
    update_no_check(&player);

    if (!player.invul) {
        if (collission_check(&player, qixes)) {
            player.invul = true;
            --player.HP;
        }

    } else {
        unsigned int color = player.color == PLAYER_COLOR ? WHITE_RGB888 : RED_RGB888;
        update_led_rgb1(color);
        update_led_rgb2(color);
    }

    if (entity_speed_stop_if_hit_color(&player, TRAIL_COLOR, PLAYER_DEFAULT_SPEED)){
        player.speed = 0;
        if (!player.invul) {
            player.HP--;
            player.invul = true;
        }
    } else {
        player.speed = PLAYER_DEFAULT_SPEED;
    }

    if ((collision_with_color(&player, BORDER_COLOR) || collision_with_color(&player, FILL_COLOR))
            && prev_color == BACKGROUND_COLOR) {
        add_whole_trail_to_background(&player);
        floodfill_least_area();   
    }
}

static void repaint(rgb565_t old_color, rgb565_t new_color)
{
    for (int y = 0; y < SCREEN_HEIGHT; ++y) {
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            if (background[x][y] == old_color) {
                background[x][y] = new_color;
            }
        }
    }
}

static void floodfill_least_area()
{
    int x1, y1, x2, y2;
    x1 = y1 = x2 = y2 = 0;
    switch (player.direction) {
    case UP:
        x1 = player.xx;
        x2 = player.xx + ENTITY_WIDTH;
        y1 = y2 = player.yy + ENTITY_HEIGHT;
        break;
    case DOWN:
        x1 = player.xx;
        x2 = player.xx + ENTITY_WIDTH;
        y1 = y2 = player.yy;
        break;
    case LEFT:
        x1 = x2 = player.xx + ENTITY_WIDTH;
        y1 = player.yy;
        y2 = player.yy + ENTITY_HEIGHT;
        break;
    case RIGHT:
        x1 = x2 = player.xx;
        y1 = player.yy;
        y2 = player.yy + ENTITY_HEIGHT;
        break;
    default:
        break;
    }

    int rect1_pxs = pseudo_floodfill(x1, y1, BACKGROUND_COLOR);
    int rect2_pxs = pseudo_floodfill(x2, y2, BACKGROUND_COLOR);

    if (rect1_pxs < rect2_pxs) {
        score += rect1_pxs;
        floodfill(x1, y1, BACKGROUND_COLOR, FILL_COLOR);
    } else {
        score += rect2_pxs;
        floodfill(x2, y2, BACKGROUND_COLOR, FILL_COLOR);
    }

    if (rect1_pxs > 0 && rect2_pxs > 0) {
        repaint(TRAIL_COLOR, FILL_COLOR);
    }
}

static int __pseudo_floodfill(int x, int y, rgb565_t old_color)
{
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT
        || background[x][y] != old_color) {
        return 0;
    }

    background[x][y] = PSEUDO_COLOR;

    int n_filled_pxs = 1
        + __pseudo_floodfill(x - 1, y, old_color)
        + __pseudo_floodfill(x + 1, y, old_color)
        + __pseudo_floodfill(x, y - 1, old_color)
        + __pseudo_floodfill(x, y + 1, old_color);

    return n_filled_pxs;
}

static int pseudo_floodfill(int x, int y, rgb565_t old_color)
{
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT
        || background[x][y] != old_color) {
        return 0;
    }

    int n_filled_pxs = __pseudo_floodfill(x, y, old_color);
    floodfill(x, y, PSEUDO_COLOR, BACKGROUND_COLOR);

    return n_filled_pxs;
}

static void floodfill(int x, int y, rgb565_t old_color, rgb565_t new_color)
{
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT
        || background[x][y] != old_color) {
            return;
    }

    background[x][y] = new_color;

    floodfill(x - 1, y, old_color, new_color);
    floodfill(x + 1, y, old_color, new_color);
    floodfill(x , y - 1, old_color, new_color);
    floodfill(x , y + 1, old_color, new_color);
}

static void update_and_redraw_score(int score)
{
    int score_cpy = score;
    char buffer[20];
    sprintf(buffer, "%d", score_cpy);
    print_string_on_screen(SCORE_X, SCORE_Y, "SCORE:", 1, RED);
    print_string_on_screen(SCORE_X + 90, SCORE_Y, buffer, 1, RED);
}
