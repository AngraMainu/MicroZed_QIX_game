/// \file mapping.h
/// This header file serves the purpose of creating convenient interface with
/// peripherals like LCD screen, LEDs, etc...

#ifndef MAPPING_H_INCLUDED
#define MAPPING_H_INCLUDED

#define _POSIX_C_SOURCE 200112L

#include "game_logic.h"
#include "image.h"
#include "init_window.h"

#include <stdbool.h>

#define SCREEN_WIDTH 480 ///< Width of the screen.
#define SCREEN_HEIGHT 320 ///< Height of the screen.
#define SCREEN_SIZE 153600 ///< Number of pixels on the screen.

/// Enum for representing different types of input.
typedef enum input_t {
    NO_INPUT, ///< If there is no new input.
    UP, ///< If the green knob is turned right.
    DOWN, ///< If the green knob is turned left.
    LEFT, ///< If the blue knob is turned left.
    RIGHT, ///< If the blue knob is turned right.
    CONFIRM, ///< If the green knob is pressed.
    BACK, ///< If the blue knob is pressed.
    EXIT ///< If the red knob is pressed.
} input_t;

/// Maps physical addresses of knobs, screen, leds needed for working properly.
/// If is not booted, functions do nothing.
void memory_map_boot();

/// Draws pixel into screen buffer.
/// \param x X-coordinate on the screen
/// \param y Y-coordinate on the screen.
/// \param px Pixel in rgb565 to be drawn.
void draw_pixel(int x, int y, rgb565_t px);

/// Draws pixel scaled by integer into screen buffer.
/// \param x X-coordinate on the screen
/// \param y Y-coordinate on the screen.
/// \param scale Integer to scale the given pixel.
/// \param px Pixel in rgb565 to be drawn.
void draw_pixel_big(int x, int y, int scale, rgb565_t px);

/// Draws the given image consecutively into screen buffer.
/// To be drawn correctly, image has to be 480x320 pixels.
/// \param img Picture to be drawn.
void draw_img(const img_t *img);

/// Draws the given image into the screen buffer at the given coordinates.
/// \param coord_x X-coordinate.
/// \param coord_y Y-coordinate.
/// \param img Image to be buffered into the screen buffer.
void draw_img_on_coord(int coord_x, int coord_y, const img_t *img);

/// Fills screen buffer with the given color.
/// \param color Color to be filled with.
void fill_screen(rgb565_t color);

/// Maps screen buffer onto actual LCD display.
/// Buffer stays the same.
void update_screen();

/// Get width of the given character from booted font.
/// \param ch Character whose width should be gotten.
/// \return width of the given character from booted font
int char_width(char ch);

/// Draws the given char with the given color into the screen buffer.
/// \param x X-coordinate on the screen
/// \param y Y-coordinate on the screen.
/// \param scale Integer to scale the given character.
/// \param color Color of the character.
void draw_char(int x, int y, char ch, int scale, rgb565_t color);

/// Draws the given string into the screen buffer.
/// \param x X-coordinate on the screen
/// \param y Y-coordinate on the screen.
/// \param string_to_print String to be printed.
/// \param scale Integer to scale the given string.
/// \param color Color of the string.
void print_string_on_screen(int x, int y, const char *string_to_print,
                            int scale, rgb565_t color);

/// Check if there is new input from knobs.
/// \return true if input is detected, false otherwise
bool input_detect();

/// Draws a rectangle with the given width, height and color at the give
/// coordinates.
/// \param x X-coordinate of the left upper corner of the rectangle.
/// \param y Y-coordinate of the left upper corner of the rectangle.
/// \param w Width of the rectangle.
/// \param h Height of the rectangle.
/// \param color Color of the rectangle. 
void draw_rect(int x, int y, int w, int h, rgb565_t color);

/// Get the input from knobs.
/// \return NO_INPUT if there is no new input, LEFT if the blue knobs is
/// turned left, RIGHT if the the blue knobs is turned right, UP if the green
/// knobs is turned right, DOWN if the green knobs is turned left, BACK if
/// the blue knobs is pressed, CONFIRM if the green knobs is pressed,
/// EXIT if the red knobs is pressed.
enum input_t input_handler();

/// Directly maps the given integer onto LEDs.
/// \param LED New value to be mapped.
void update_led_line(uint32_t LED);

/// Directly maps the given integer left-most RGB LED.
/// \param RGB New value (rgb888) to be mapped.
void update_led_rgb1(uint32_t RGB);

/// Directly maps the given integer right-most RGB LED.
/// \param RGB New value (rgb888) to be mapped.
void update_led_rgb2(uint32_t RGB);

#endif // MAPPING_H_INCLUDED
