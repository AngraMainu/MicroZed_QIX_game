#include "mapping.h"
#include "mzapo_regs.h"
#include "mzapo_phys.h"
#include "mzapo_parlcd.h"
#include "font_types.h"
#include "game_logic.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#define PX_SEPARATOR 5
#define PX_BORDER 20
#define PX_NEWLINE 10

static byte *parlcd_mem_base = NULL;
static byte *mem_base = NULL;
static rgb565_t current_screen[SCREEN_SIZE];
static font_descriptor_t *fdes = &font_winFreeSystem14x16;
uint32_t knobs_val = 0;
uint32_t prev_knobs_val = 0;

static bool booted = false;

void memory_map_boot()
{
    parlcd_mem_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
    parlcd_hx8357_init(parlcd_mem_base);
    mem_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);

    booted = true;
}

void draw_pixel(int x, int y, rgb565_t color)
{
    if (!booted || x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) {
        return;
    }

    current_screen[y * SCREEN_WIDTH + x] = color;
}

void draw_pixel_big(int x, int y, int scale, rgb565_t color)
{
    if (!booted) {
        return;
    }

    for (int i = 0; i < scale; ++i) {
        for (int j = 0; j < scale; ++j) {
            draw_pixel(x + i, y + j, color);
        }
    }
}

void draw_img(const img_t *img)
{
    if (!booted || !img || !img->pxs) {
        return;
    }

    const rgb565_t *pxs = img->pxs;

    for (int i = 0; i < SCREEN_SIZE; ++i) {
        current_screen[i] = pxs[i];
    }
}

void draw_img_on_coord(int coord_x, int coord_y, const img_t *img)
{
    if (!booted || !img || !img->pxs) {
        return;
    }

    const rgb565_t *pxs = img->pxs;

    for (int y = 0; y < img->height; ++y) {
        for (int x = 0; x < img->width; ++x) {
            draw_pixel(coord_x + x, coord_y + y, pxs[y * img->width + x]);
        }
    }
}

void update_screen()
{
    if (!booted) {
        return;
    }

    parlcd_write_cmd(parlcd_mem_base, 0x2c);
    for (int i = 0; i < SCREEN_SIZE; ++i) {
        parlcd_write_data(parlcd_mem_base, current_screen[i]);
    }
}

int char_width(char ch)
{
    if (!booted) {
        return 0;
    }

    int width;
    if (!fdes->width) {
        width = fdes->maxwidth;
    } else {
        width = fdes->width[ch-fdes->firstchar];
    }

    return width;
}

void draw_char(int x, int y, char ch, int scale, rgb565_t color)
{
    if (!booted) {
        return;
    }

    int w = char_width(ch);
    const font_bits_t *ptr;
    if ((ch >= fdes->firstchar) && (ch - fdes->firstchar < fdes->size)) {
        if (fdes->offset) {
            ptr = &fdes->bits[fdes->offset[ch - fdes->firstchar]];
        } else {
            int bw = (fdes->maxwidth + 15) / 16;
            ptr = &fdes->bits[(ch - fdes->firstchar) * bw * fdes->height];
        }

        for (int i = 0; i < fdes->height; ++i) {
            font_bits_t val = *ptr;
            for (int j = 0; j < w; ++j) {
                if ((val & 0x8000) != 0) {
                    draw_pixel_big(x + scale * j, y + scale * i, scale, color);
                }
                val <<= 1;
            }

            ++ptr;
        }
    }
}

void draw_rect(int x, int y, int w, int h, rgb565_t color)
{
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            draw_pixel(x+i, y+j, color);
        } 
    }
}

void print_string_on_screen(int x, int y, const char *string_to_print,
                            int scale, rgb565_t color)
{
    if (!booted) {
        return;
    }

    int len = strlen(string_to_print);
    for (int i = 0; i < len; i++) {
        draw_char(x, y, string_to_print[i], scale, color);
        x += char_width(string_to_print[i]) * scale + PX_SEPARATOR;

        if (x > SCREEN_WIDTH - PX_BORDER) {
            x = PX_BORDER;
            y += fdes->height + PX_NEWLINE;
        }
    }
}

void fill_screen(rgb565_t color)
{
    if (!booted) {
        return;
    }

    for (int i = 0; i < SCREEN_SIZE; ++i) {
        current_screen[i] = color;
    }
}

bool input_detect()
{
    if (!booted) {
        return false;
    }

    return knobs_val != prev_knobs_val;
}

input_t input_handler()
{
    if (!booted) {
        return NO_INPUT;
    }

    prev_knobs_val = knobs_val;
    knobs_val = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o);

    if (!input_detect()) {
        return NO_INPUT;
    }

    byte prev_blue_knobs = prev_knobs_val & 0xff;
    byte blue_knobs = knobs_val & 0xff;

    if (prev_blue_knobs >= 0xfc && blue_knobs <= 0x04){
        return RIGHT;
    } else if (prev_blue_knobs <= 0x04 && blue_knobs >= 0xfc ){
        return LEFT;
    } else if (blue_knobs > prev_blue_knobs){
        return RIGHT;
    } else if (blue_knobs < prev_blue_knobs){
        return LEFT;
    }

    byte prev_green_knobs = (prev_knobs_val >> 8) & 0xff;
    byte green_knobs = (knobs_val >> 8) & 0xff;

    if (prev_green_knobs >= 0xfc && green_knobs <= 0x04){
        return UP;
    } else if (prev_green_knobs <= 0x04 && green_knobs >= 0xfc){
        return DOWN;
    } else if (green_knobs > prev_green_knobs){
        return UP;
    } else if (green_knobs < prev_green_knobs){
        return DOWN;
    }

    byte prev_back = (prev_knobs_val >> 24) & 0x1;
    byte back = (knobs_val >> 24) & 0x1;
    
    if (prev_back < back) {
        return BACK;
    }

    byte prev_confirm = (prev_knobs_val >> 24) & 0x2;
    byte confirm = (knobs_val >> 24) & 0x2;

    if (prev_confirm < confirm){
        return CONFIRM;
    }

    byte prev_exit = (prev_knobs_val >> 24) & 0x4;
    byte exit = (knobs_val >> 24) & 0x4;

    if (prev_exit < exit) {
        return EXIT;
    }

    return NO_INPUT;
}

void update_led_line(uint32_t LED) 
{
    if (!booted) {
        return;
    }

    *(volatile uint32_t*)(mem_base + SPILED_REG_LED_LINE_o) = LED;   
}

void update_led_rgb1(uint32_t RGB)
{
    if (!booted) {
        return;
    }

    *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = RGB;   
}

void update_led_rgb2(uint32_t RGB)
{
    if (!booted) {
        return;
    }

    *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = RGB;   
}
