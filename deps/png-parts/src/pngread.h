/*
* PNG-parts
* parts of a Portable Network Graphics implementation
* Copyright 2018 Cody Licorish
*
* Licensed under the MIT License.
*
* pngread.h
* png reader header
*/
#ifndef __PNG_PARTS_PNGREAD_H__
#define __PNG_PARTS_PNGREAD_H__

#include "api.h"
#include "png.h"

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/*
 * Initialize a reader for PNG.
 * - p the reader to initialize
 */
PNGPARTS_API
void pngparts_pngread_init(struct pngparts_png* p);
/*
 * Free out a reader for PNG.
 * - p the reader to free; any registered chunks receive the
 *     DESTROY message at this point
 */
PNGPARTS_API
void pngparts_pngread_free(struct pngparts_png* p);
/*
 * Process the active buffer.
 * - p the reader to use
 */
PNGPARTS_API
int pngparts_pngread_parse(struct pngparts_png* p);

/*
 * Assign an API for reading IDAT chunks.
 * - cb chunk callback
 * - z zlib stream reader
 */
PNGPARTS_API
int pngparts_pngread_assign_idat_api
  (struct pngparts_png_chunk_cb* cb, struct pngparts_api_z const* z);

/*
 * Assign an API for reading PLTE chunks.
 * - cb chunk callback
 */
PNGPARTS_API
int pngparts_pngread_assign_plte_api(struct pngparts_png_chunk_cb* cb);
#ifdef __cplusplus
};
#endif /*__cplusplus*/

#endif /*__PNG_PARTS_PNGREAD_H__*/
