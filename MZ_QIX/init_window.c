#include "init_window.h"
#include "mapping.h"
#include "image.h"

#include <stdlib.h>
#include <time.h>

#define NMENUS 4
#define NDKDANCES 13
#define FONT_SCALE 2
#define TEXT_COLOR 0x0

static const char *menusfp[] = {"qix_default.ppm", "qix_1.ppm", "qix_2.ppm", "qix_3.ppm"};
static const char *dkdancefp[] = {"dk_1.ppm", "dk_2.ppm", "dk_3.ppm", "dk_4.ppm",
                                  "dk_5.ppm", "dk_6.ppm","dk_7.ppm", "dk_8.ppm",
                                  "dk_9.ppm","dk_10.ppm", "dk_11.ppm","dk_12.ppm","dk_13.ppm"};

static img_t *menus[NMENUS] = {NULL};
static img_t *dkdance[NDKDANCES] = {NULL};

static const int menu_anim_counter = 10;
static int cur_dk_dance = 0;

void init_starting_menu()
{
    for (int i = 0; i < NMENUS; ++i) {
        if (!menus[i]) {
            img_t *src = load_ppm_image(menusfp[i]);
            menus[i] = to_rgb565(src);
            free_image(src);
        }
    }

    for (int i = 0; i < NDKDANCES; ++i) {
        if (!dkdance[i]) {
            img_t *src = load_ppm_image(dkdancefp[i]);
            dkdance[i] = to_rgb565(src);
            free_image(src);
        }
    }
}

void draw_starting_menu(int idx)
{
    if (idx < 0 || idx >= NMENUS) {
        return;
    }

    draw_img(menus[idx]);
    print_string_on_screen(150, 120, "Start Game", FONT_SCALE, TEXT_COLOR);
    print_string_on_screen(150, 180, "Top Scores", FONT_SCALE, TEXT_COLOR);
    print_string_on_screen(150, 240, "Credits", FONT_SCALE, TEXT_COLOR);
    update_screen();
}

void cleanup_starting_menu()
{
    for (int i = 0; i < NMENUS; ++i) {
        free_image(menus[i]);
    }

    for (int i = 0; i < NDKDANCES; ++i) {
        free_image(dkdance[i]);
    }
}

void draw_end_game_screen()
{
    struct timespec loop_delay = {.tv_sec = 1, .tv_nsec = 1 * 1000 * 1000};

    for (int y = 0; y < SCREEN_HEIGHT; ++y) {
        for (int x = 0; x < SCREEN_WIDTH; x++){
            draw_pixel(x, y, 0);
        }
    }

    rgb565_t color565 = 0xF2EA;
    uint32_t color888 = 0xFF0000;

    for (int i = 0; i < 10; i++)
    {
        color565 = color565 == 0xF2EA? 0xF75B : 0xF2EA;
        color888 = color888 == 0xFF0000? 0xFFFFFF : 0xFF0000;
        update_screen();
        print_string_on_screen(100, 130, "YOU LOST!!!", 3, color565);
        update_led_line(rand());
        update_led_rgb1(color888);
        update_led_rgb2(color888);
        clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
    }
}

void draw_win_game_screen()
{
    struct timespec loop_delay = {.tv_sec = 1, .tv_nsec = 1 * 1000 * 1000};

    for (int y = 0; y < SCREEN_HEIGHT; ++y) {
        for (int x = 0; x < SCREEN_WIDTH; x++){
            draw_pixel(x, y, 0);
        }
    }

    rgb565_t color565 = 0x8D8C;
    uint32_t color888 = 0x00FF00;
    for (int i = 0; i < 10; i++)
    {
        color565 = color565 == 0x8D8C? 0xF75B : 0x8D8C;
        color888 = color888 == 0x00FF00? 0xFFFFFF : 0x00FF00;
        update_screen();
        print_string_on_screen(100, 130, "YOU WON!!!", 3, color565);
        update_led_line(rand());
        update_led_rgb1(color888);
        update_led_rgb2(color888);
        clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
    }
    
}

void draw_exit_screen()
{
    struct timespec loop_delay = {.tv_sec = 3, .tv_nsec = 0};
    fill_screen(0);
    print_string_on_screen(100, 125, "Exiting...", 3, 0xffff);
    update_screen();

    clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
    fill_screen(0);
    update_screen();
}

void draw_top_scores_screen()
{
    struct timespec loop_delay = {.tv_sec = 0, .tv_nsec = 1 * 1000 * 1000};

    fill_screen(0);
    print_string_on_screen(50, 100, "Not yet implemented", 2, 0xffff);
    print_string_on_screen(50, 150, "Press any key to exit", 2, 0xffff);
    update_screen();

    while (true) {
        input_t input = input_handler();
        if (input == BACK || input == EXIT || input == CONFIRM) {
            break;
        }

        clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
    }
}

void draw_credits_screen()
{
    struct timespec loop_delay = {.tv_sec = 0, .tv_nsec = 100 * 1000 * 1000};

    int anim_counter_copy = menu_anim_counter;

    fill_screen(0);

    int r = 0;
    rgb565_t color565 = 0x8D8C;
    uint32_t color888 = 0x00FF00;

    while (true) {
        print_string_on_screen(10, 50, "Students: Vadim Mychko, Trenin Egor", 1, color565);
        print_string_on_screen(10, 80, "Teachers: Ing. Pavel Pisa, Ph.D", 1, color565);
        print_string_on_screen(10, 110, "             RNDr. Petr Stepan, Ph.D", 1, color565);
        print_string_on_screen(10, 140, "CVUT, Otevrena Informatika, 1. rok, APO35", 1, color565);

        update_led_line(r);
        update_led_rgb1(color888);
        update_led_rgb2(color888);

        if (anim_counter_copy-- < 0) {
            color565 = color565 == 0x8D8C? 0xF75B : 0x8D8C;
            color888 = color888 == 0x00FF00? 0xFFFFFF : 0x00FF00;
            anim_counter_copy = menu_anim_counter;
            r = rand();
        }

        draw_rect(205, 250, 100, 70, 0);
        draw_img_on_coord(205, 250, dkdance[cur_dk_dance++]);
        cur_dk_dance = cur_dk_dance > 12 ? 0 : cur_dk_dance;

        update_screen();

        if (input_handler() == BACK) {
            return;
        }

        clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
    }
}
