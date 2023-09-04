/*
 * PNG-parts
 * parts of a Portable Network Graphics implementation
 * Copyright 2018-2020 Cody Licorish
 *
 * Licensed under the MIT License.
 *
 * aux.h
 * header for auxiliary functions
 */
#ifndef __PNG_PARTS_AUX_H__
#define __PNG_PARTS_AUX_H__

#include "api.h"

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

struct pngparts_png_chunk_cb;
struct pngparts_png_header;
struct pngparts_pngwrite_sieve;

enum pngparts_aux_format {
  /* one sample of luminance */
  PNGPARTS_AUX_L = 0,
  /* first a sample of red, then green, then blue */
  PNGPARTS_AUX_RGB = 2,
  /* first a luminance sample, then alpha */
  PNGPARTS_AUX_LA = 4,
  /* first a red sample, then green, then blue, then alpha */
  PNGPARTS_AUX_RGBA = 6
};

/*
 * Send a DESTROY message to a PNG chunk callback without a png structure.
 * - cb the callback to receive the message
 * @return the result from the callback, or OK if no callback
 */
PNGPARTS_API
int pngparts_aux_destroy_png_chunk(struct pngparts_png_chunk_cb const* cb);

/*
 * Read a PNG file with 16-bit color values.
 * - img image interface
 * - fname file name to read
 * @return OK on success, negative value otherwise
 */
PNGPARTS_API
int pngparts_aux_read_png_16
  (struct pngparts_api_image* img, char const* fname);

/*
 * Write a PNG file with 16-bit color values.
 * - img image interface
 * - fname file name to read
 * @return OK on success, negative value otherwise
 */
PNGPARTS_API
int pngparts_aux_write_png_16
  (struct pngparts_api_image* img, char const* fname);

/*
 * Read a PNG file with 8-bit color values.
 * - img image interface
 * - fname file name to read
 * @return OK on success, negative value otherwise
 */
PNGPARTS_API
int pngparts_aux_read_png_8
  (struct pngparts_api_image* img, char const* fname);

/*
 * Write a PNG file with 8-bit color values.
 * - img image interface
 * - fname file name to read
 * @return OK on success, negative value otherwise
 */
PNGPARTS_API
int pngparts_aux_write_png_8
  (struct pngparts_api_image* img, char const* fname);

/*
 * Free some memory.
 * - p a pointer to some memory to free
 */
PNGPARTS_API
void pngparts_aux_free(void* p);

/*
 * Write a block of contiguous scanlines to a PNG file.
 * - width width of each scanline, in pixels
 * - height number of scanlines
 * - stride gap in bytes between pixels
 * - vstride gap in bytes between each scanline
 * - format one of the PNGPARTS_AUX_... formats
 * - bits either 8 (char) or 16 (short)
 * - data pointer to unsigned char or unsigned short
 * - fname name of PNG image file to read
 * @return OK on success, negative otherwise
 */
PNGPARTS_API
int pngparts_aux_write_block
  ( unsigned int width, unsigned int height,
    unsigned int stride, unsigned int vstride,
    unsigned int format, unsigned int bits, void const* data,
    char const* fname);

/*
 * Read a block of contiguous scanlines to a PNG file.
 * - width width of each scanline, in pixels
 * - height number of scanlines
 * - stride gap in bytes between each scanline
 * - format one of the PNGPARTS_AUX_... formats
 * - bits either 8 (char) or 16 (short)
 * - data pointer to unsigned char or unsigned short
 * - fname name of PNG image file to read
 * @return OK on success, negative otherwise
 */
PNGPARTS_API
int pngparts_aux_read_block
  ( unsigned int width, unsigned int height,
    unsigned int stride, unsigned int vstride,
    unsigned int format, unsigned int bits, void* data,
    char const* fname);

/*
 * Read a block of contiguous scanlines to a PNG file, with .
 * - header header to read
 * - fname name of PNG image file to read
 * @return OK on success, negative otherwise
 */
PNGPARTS_API
int pngparts_aux_read_header
  (struct pngparts_png_header* header, char const* fname);

/*
 * Configure a sieve to use RFC 2083 section 9.6 recommendations.
 * - sv the sieve to configure
 * - header image header from which to configure
 */
PNGPARTS_API
void pngparts_aux_set_sieve2083
  (struct pngparts_pngwrite_sieve* sv, struct pngparts_png_header* header);

#ifdef __cplusplus
};
#endif /*__cplusplus*/

#endif /*__PNG_PARTS_AUX_H__*/
