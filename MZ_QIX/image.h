/// \file image.h
/// Header file for loading PPM images, saving them converting to rgb565.

#ifndef IMAGE_H_INCLUDED
#define IMAGE_H_INCLUDED

#define _POSIX_C_SOURCE 200112L

#include <stddef.h>
#include <stdint.h>

typedef uint8_t byte;

/// Structure for representing RGB pixel.
typedef struct {
    byte red; ///< Red color of the pixel.
    byte green; ///< Green color of the pixel.
    byte blue; ///< Blue color of the pixel.
} rgb888_t;

typedef uint16_t rgb565_t;

/// Structure for representing images.
typedef struct {
    int width; ///< Width of the image.
    int height; ///< Height of the image.
    size_t px_size; ///< Bytes per pixel.
    byte *pxs; ///< Pointer to the pixels array.
} img_t;

/// Load an image from the given filepath in PPM format.
/// \param filepath Filepath to the image to be loaded.
/// \return pointer to the image on success, NULL otherwise.
/// Image has to be freed manually.
img_t *load_ppm_image(const char *filepath);

/// Free memory allocated for image.
/// \param img pointer to the image.
void free_image(img_t *image);

/// Convert image from RGB888 to RGB565.
/// \param img image to be converted.
/// \return pointe to the converted picture.
img_t *to_rgb565(const img_t *source);

/// Save image in PPM format (RGB888).
/// \param img Image to be saved.
/// \param filepath Path to the saved image.
void save_ppm_image(const img_t *img, const char *filepath);

#endif // IMAGE_H_INCLUDED
