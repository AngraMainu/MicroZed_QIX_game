/// \file init_window.h
/// Header file for mapping starting menus + donkey kong animation.

#ifndef INIT_WINDOW_H_INCLUDED
#define INIT_WINDOW_H_INCLUDED

/// Initializes images for starting menu.
void init_starting_menu();

/// Frees images buffered for starting menu.
void cleanup_starting_menu();

/// Draws menu onto the screen and updates screen. 0 for default,
/// 1 for first selection, 2 for second selection, 3 for third selection.
/// If index is out of range draws nothing.
/// For correct work needs screen and other peripherals to be booted.
void draw_starting_menu(int idx);

/// Draws exit screen onto the screen and updates screen.
/// For correct work needs screen and other peripherals to be booted.
void draw_exit_screen();

/// Draws end game screen onto the screen and updates screen.
/// For correct work needs screen and other peripherals to be booted.
void draw_end_game_screen();

/// Draws win game screen onto the screen and updates screen.
/// For correct work needs screen and other peripherals to be booted.
void draw_win_game_screen();

/// Draws credits screen onto the screen and updates screen.
/// For correct work needs screen and other peripherals to be booted.
void draw_credits_screen();

/// Draws top scores screen onto the screen and updates screen.
/// For correct work needs screen and other peripherals to be booted.
void draw_top_scores_screen();

#endif // INIT_WINDOW_H_INCLUDED
