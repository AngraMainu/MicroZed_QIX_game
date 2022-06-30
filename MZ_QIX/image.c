#include "image.h"
#include "mapping.h"

#include <stdio.h>
#include <stdlib.h>

#define MAX_COLOR_VALUE 255
#define IMAGE_PPM_FORMAT "P6\n%d %d\n%d\n"

/// Convert one pixel from RGB888 to RGB565.
/// \param px pixel to be converted.
/// \return Pixel in RGB565.
static rgb565_t convert_px(const rgb888_t px)
{
    return (((px.red >> 3) & 0x1f) << 11) | (((px.green >> 2) & 0x3f) << 5) | ((px.blue >> 3) & 0x1f);
}

/// Allocate memory for image structure
/// \param width Width of the image.
/// \param height Height of the image.
/// \returns Pointer to the image structure on success, NULL otherwise.
static img_t *alloc_image(int width, int height, size_t px_size)
{
    img_t *img = malloc(sizeof(img_t));
    byte *pxs = malloc(width * height * px_size);

    if (!img || !pxs) {
        free(img);
        free(pxs);
        return NULL;
    }

    img->width = width;
    img->height = height;
    img->px_size = px_size;
    img->pxs = pxs;

    return img;
}

img_t *load_ppm_image(const char *fp)
{
    FILE *f = fopen(fp, "rb");
    if (!f) {
        return NULL;
    }

    int width, height, color_val;
    if (fscanf(f, IMAGE_PPM_FORMAT, &width, &height, &color_val) != 3
        || width <= 0 || width > SCREEN_WIDTH
        || height <= 0 || height > SCREEN_HEIGHT) {
        fclose(f);
        return NULL;
    }

    const size_t n_bytes = width * height * sizeof(rgb888_t);
    img_t *img = alloc_image(width, height, sizeof(rgb888_t));
 
    if (!img || fread(img->pxs, sizeof(byte), n_bytes, f) != n_bytes) {
        free_image(img);
        fclose(f);
        return NULL;
    }

    fclose(f);

    return img;
}

void save_ppm_image(const img_t *img, const char *fp)
{
    if (img->px_size != sizeof(rgb888_t)) {
        return;
    }

    FILE *f = fopen(fp, "wb");
    if (!f) {
        return;
    }

    fprintf(f, IMAGE_PPM_FORMAT, img->width, img->height, MAX_COLOR_VALUE);
    const size_t n_bytes = img->width * img->height * img->px_size;
    fwrite(img->pxs, sizeof(byte), n_bytes, f);

    fclose(f);
}

void free_image(img_t *img)
{
    if (!img || !img->pxs) {
        return;
    }

    free(img->pxs);
    free(img);
}

img_t *to_rgb565(const img_t *src)
{
    img_t *dst = alloc_image(src->width, src->height, sizeof(rgb565_t));
    if (!dst) {
        return NULL;
    }

    for (int y = 0; y < src->height; ++y) {
        for (int x = 0; x < src->width; ++x) {
            const rgb888_t *src_px = (const rgb888_t *)(src->pxs) + y * src->width + x;
            rgb565_t *dst_px = (rgb565_t *)(dst->pxs) + y * dst->width + x;

            *dst_px = convert_px(*src_px);
        }
    }

    return dst;
}
