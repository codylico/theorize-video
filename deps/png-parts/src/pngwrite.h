/*
 * PNG-parts
 * parts of a Portable Network Graphics implementation
 * Copyright 2018-2019 Cody Licorish
 *
 * Licensed under the MIT License.
 *
 * pngwrite.h
 * png writer header
 */
#ifndef __PNG_PARTS_PNGWRITE_H__
#define __PNG_PARTS_PNGWRITE_H__

#include "api.h"
#include "png.h"

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/*
 * Filter selection callback.
 * - user_data callback's data
 * - img_data image callback data
 * - img_get_cb pixel value query function
 * - width Adam7 scanline width
 *     (only equal to the image width on non-interlaced images)
 * - y Adam7 scanline index to process for filtering, or the physical
 *     scanline coordinate if the image is not interlaced.
 *     (Use pngparts_png_adam7_reverse_xy to get the physical coordinate.)
 * - level Adam7 interlace level, or zero (0) for non-interlaced images
 * @return a filter identifier
 */
typedef int (*pngparts_pngwrite_filter_cb)
  ( void* user_data, void* img_data, pngparts_api_image_get_cb img_get_cb,
    long int width, long int y, int level);

struct pngparts_pngwrite_sieve {
  /* callback data */
  void* cb_data;
  /* destructor */
  pngparts_api_free_cb free_cb;
  /* filter selection callback */
  pngparts_pngwrite_filter_cb filter_cb;
};

/*
 * Initialize a writer for PNG.
 * - p the writer to initialize
 */
PNGPARTS_API
void pngparts_pngwrite_init(struct pngparts_png* w);

/*
 * Free out a writer for PNG.
 * - w the writer to free; any registered chunks receive the
 *     DESTROY message at this point
 */
PNGPARTS_API
void pngparts_pngwrite_free(struct pngparts_png* w);

/*
 * Process the active buffer.
 * - w the writer to use
 */
PNGPARTS_API
int pngparts_pngwrite_generate(struct pngparts_png* w);

/*
 * Assign an API for writing IDAT chunks.
 * - cb chunk callback
 * - z zlib stream writer
 * - chunk_size size of chunks to generate (or 0 for default value)
 * @return OK on success
 */
PNGPARTS_API
int pngparts_pngwrite_assign_idat_api
  ( struct pngparts_png_chunk_cb* cb, struct pngparts_api_z const* z,
    int chunk_size);

/*
 * Add a sieve to the IDAT chunk writer. If a destructor is provided,
 * the chunk callback will destroy the sieve's callback data when the
 * chunk callback receives the PNGPARTS_PNG_M_DESTROY message.
 * - cb the IDAT chunk writer
 * - sieve the sieve to take and use
 * @return OK on success; if this function fails, the caller retains
 *   ownership of the sieve structure
 */
PNGPARTS_API
int pngparts_pngwrite_set_idat_sieve
  ( struct pngparts_png_chunk_cb* cb,
    struct pngparts_pngwrite_sieve const* sieve);

/*
 * Assign an API for writing a PLTE chunk.
 * - cb chunk callback
 * @return OK on success
 */
PNGPARTS_API
int pngparts_pngwrite_assign_plte_api( struct pngparts_png_chunk_cb* cb);

#ifdef __cplusplus
};
#endif /*__cplusplus*/

#endif /*__PNG_PARTS_PNGWRITE_H__*/
