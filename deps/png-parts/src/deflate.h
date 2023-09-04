/*
 * PNG-parts
 * parts of a Portable Network Graphics implementation
 * Copyright 2018-2019 Cody Licorish
 *
 * Licensed under the MIT License.
 *
 * deflate.h
 * header for deflate writer and compressor
 */
#ifndef __PNG_PARTS_DEFLATE_H__
#define __PNG_PARTS_DEFLATE_H__

#include "api.h"
#include "flate.h"

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/*
 * Initialize a deflater.
 * - fl the flate struct to prepare
 */
PNGPARTS_API
void pngparts_deflate_init(struct pngparts_flate *fl);

/*
 * Free up a deflater.
 * - fl the flate struct to free
 */
PNGPARTS_API
void pngparts_deflate_free(struct pngparts_flate *fl);

/*
 * Compose a flate callback for a deflate module.
 * - dst the callback information structure
 * - src the flate struct
 */
PNGPARTS_API
void pngparts_deflate_assign_api
  (struct pngparts_api_flate *fcb, struct pngparts_flate *fl);

/*
 * Start callback.
 * - fl the flate struct to use
 * - hdr header information
 * @return zero if the callback supports the stream,
 *   or other negative value otherwise
 */
PNGPARTS_API
int pngparts_deflate_start
  ( void *fl,
    short int fdict, short int flevel, short int cm, short int cinfo);

/*
 * Dictionary byte callback.
 * - fl the flate struct to use
 * - ch byte, or -1 for repeat bytes
 * @return zero, or UNSUPPORTED if this deflater does not
 *   support preset dictionaries
 */
PNGPARTS_API
int pngparts_deflate_dict(void *fl, int ch);

/*
 * Byte callback.
 * - fl the flate struct to use
 * - ch byte, or -1 for repeat bytes
 * - put_cb callback for putting output bytes
 * - put_data data to pass to put callback
 * @return zero, or OVERFLOW if the output buffer is too full,
 *   or DONE at the end of the bit stream
 */
PNGPARTS_API
int pngparts_deflate_one
  (void *fl, int ch, void* put_data, int(*put_cb)(void*,int));

/*
 * Finish callback.
 * - fl the flate struct to use
 * - put_cb unused
 * - put_data unused
 * @return zero, or OVERFLOW if the output buffer is too full
 */
PNGPARTS_API
int pngparts_deflate_finish
  (void* fl, void* put_data, int(*put_cb)(void*,int));

#ifdef __cplusplus
};
#endif /*__cplusplus*/

#endif /*__PNG_PARTS_DEFLATE_H__*/
