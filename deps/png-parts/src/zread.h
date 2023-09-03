/*
 * PNG-parts
 * parts of a Portable Network Graphics implementation
 * Copyright 2018-2019 Cody Licorish
 *
 * Licensed under the MIT License.
 *
 * zread.h
 * zlib reader header
 */
#ifndef __PNG_PARTS_ZREAD_H__
#define __PNG_PARTS_ZREAD_H__

#include "api.h"
#include "z.h"
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/



/*
 * Initialize a stream reader.
 * - prs the reader to initialize
 */
PNGPARTS_API
void pngparts_zread_init(struct pngparts_z *prs);
/*
 * End a stream reader.
 * - prs the reader to end
 */
PNGPARTS_API
void pngparts_zread_free(struct pngparts_z *prs);
/*
 * Assign the callbacks for a zlib stream reader.
 * - dst destination API interface
 * - src stream reader
 */
PNGPARTS_API
void pngparts_zread_assign_api
  (struct pngparts_api_z *dst, struct pngparts_z *src);
/*
 * Parse a part of a stream.
 * - zs zlib stream structure
 * - mode reader expectation mode
 * @return OK on success, DONE at end of stream, EOF
 *   on unexpected end of stream
 */
PNGPARTS_API
int pngparts_zread_parse(void* zs, int mode);
/*
 * Try to set the dictionary for use.
 * - zs zlib stream structure
 * - ptr bytes of the dictionary
 * - len dictionary length in bytes
 * @return OK if the dictionary matches the stream's
 *   dictionary checksum, WRONG_DICT if the dictionary match fails,
 *   or BAD_STATE if called before the dictionary checksum is
 *   available
 */
PNGPARTS_API
int pngparts_zread_set_dictionary
  (void* zs, unsigned char const* ptr, int len);

#ifdef __cplusplus
};
#endif /*__cplusplus*/

#endif /*__PNG_PARTS_ZREAD_H__*/
