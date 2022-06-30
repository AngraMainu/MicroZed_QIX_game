/// \file main.c

#define _POSIX_C_SOURCE 200112L

#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <setjmp.h>

#include "mapping.h"
#include "game_logic.h"
#include "init_window.h"

/// Structure for representing selected menu.
typedef enum selection_t {
    NO_SELECTION, ///< If no menu is selected.
    START_GAME, ///< If "Start Game" is selected.
    TOP_SCORES, ///< If "Top Scores" is selected.
    CREDITS ///< If "Credits" is selected.
} selection_t;

static int LED_counter = 3;
static jmp_buf buf;
static input_t last_input = NO_INPUT;
static selection_t cur_menu = 0;
static const struct timespec loop_delay
    = {.tv_sec = 0, .tv_nsec = 250 * 1000 * 1000};

/// Shows menu until one of the buttons is selected or pressed exit.
void show_menu_until_selected();

/// Shows next screen of the selected button.
void show_next_selection();

/// Main entry point. Shows initial menu.
int main()
{
    memory_map_boot();
    init_starting_menu();

    setjmp(buf);

    show_menu_until_selected();

    bool should_exit = false;

    while (!should_exit) {
        switch (last_input) {
        case CONFIRM:
            show_next_selection();
            break;
        case EXIT:
            draw_exit_screen();
            should_exit = true;
            break;
        default:
            show_menu_until_selected();
            break;
        }
    }

    cleanup_starting_menu();

    return 0;
}

void show_menu_until_selected()
{
    cur_menu = 0;
    draw_starting_menu(cur_menu);

    while (true) {
        last_input = input_handler();
        if (last_input == UP || last_input == DOWN) {
            selection_t prev_menu = cur_menu;
            cur_menu = last_input == UP ? (cur_menu > 1 ? cur_menu - 1 : cur_menu)
                 : last_input == DOWN ? (cur_menu < 3 ? cur_menu + 1 : cur_menu)
                 : cur_menu;

            if (cur_menu != prev_menu) {
                draw_starting_menu(cur_menu);
            }
        }

        if (last_input == CONFIRM || last_input == EXIT) {
            break;
        }

        if (LED_counter-- < 0){
            update_led_rgb1(rand());
            update_led_rgb2(rand());
            update_led_line(rand());
            LED_counter = 3;
        }

        clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
    }
}

void show_next_selection()
{
    switch (cur_menu) {
    case START_GAME:
        init_gamelogic();
        start_new_game();
        break;
    case TOP_SCORES:
        draw_top_scores_screen();
        break;
    case CREDITS:
        draw_credits_screen();
        break;
    default:
        break;
    }

    longjmp(buf, 0);
}
