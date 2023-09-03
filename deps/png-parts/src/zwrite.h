/*
 * PNG-parts
 * parts of a Portable Network Graphics implementation
 * Copyright 2019 Cody Licorish
 *
 * Licensed under the MIT License.
 *
 * zwrite.h
 * zlib writer header
 */
#ifndef __PNG_PARTS_ZWRITE_H__
#define __PNG_PARTS_ZWRITE_H__

#include "api.h"
#include "z.h"

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/


/*
 * Initialize a stream writer.
 * - zs the writer to initialize
 */
PNGPARTS_API
void pngparts_zwrite_init(struct pngparts_z *zs);

/*
 * End a stream writer.
 * - zs the writer to end
 */
PNGPARTS_API
void pngparts_zwrite_free(struct pngparts_z *zs);

/*
 * Assign the callbacks for a zlib stream writer.
 * - dst destination API interface
 * - src stream writer
 */
PNGPARTS_API
void pngparts_zwrite_assign_api
  (struct pngparts_api_z *dst, struct pngparts_z *src);

/*
 * Parse a part of a stream.
 * - zs zlib stream structure
 * - mode reader expectation mode
 * @return OK on success, DONE at end of stream, EOF
 *   on unexpected end of stream
 */
PNGPARTS_API
int pngparts_zwrite_generate(void* zs, int mode);

/*
 * Try to set the dictionary for use.
 * - zs zlib stream structure
 * - ptr bytes of the dictionary
 * - len dictionary length in bytes
 * @return OK if the dictionary matches the stream's
 *   dictionary checksum, or BAD_STATE if called after
 *   the dictionary checksum is available for writing
 */
PNGPARTS_API
int pngparts_zwrite_set_dictionary
  (void* zs, unsigned char const* ptr, int len);

#ifdef __cplusplus
};
#endif /*__cplusplus*/

#endif /*__PNG_PARTS_ZWRITE_H__*/
