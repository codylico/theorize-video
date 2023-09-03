/*
 * PNG-parts
 * parts of a Portable Network Graphics implementation
 * Copyright 2018 Cody Licorish
 *
 * Licensed under the MIT License.
 *
 * z.h
 * zlib main header
 */
#ifndef __PNG_PARTS_Z_H__
#define __PNG_PARTS_Z_H__

#include "api.h"

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/


struct pngparts_z;

/*
 * Header structure
 */
struct pngparts_z_header {
  /* check value */
  short int fcheck;
  /* flag: whether the stream has a dictionary */
  short int fdict;
  /* compression level */
  short int flevel;
  /* compression method (8 = deflate) */
  short int cm;
  /* compression information */
  short int cinfo;
};

/*
 * Accumulator for Adlr32 checksums
 */
struct pngparts_z_adler32 {
  /* Flat accumulator */
  unsigned long int s1;
  /* Compound accumulator */
  unsigned long int s2;
};


/*
 * Base structure for zlib processors.
 */
struct pngparts_z {
  /* callback information */
  struct pngparts_api_flate cb;
  /* reader state */
  short state;
  /* the stream header */
  struct pngparts_z_header header;
  /* short position */
  unsigned short shortpos;
  /* buffer for short byte chunks */
  unsigned char shortbuf[15];
  /*
   * 1: if a dictionary has been set
   * 2: skip reading a byte
   */
  unsigned char flags_tf;
  /* dictionary checksum */
  unsigned long int dict_check;
  /* the current checksum */
  struct pngparts_z_adler32 check;
  /* the input buffer for deflated data */
  unsigned char* inbuf;
  /* the output buffer for inflated data */
  unsigned char* outbuf;
  /* the size of the input buffer in bytes */
  int insize;
  /* the size of the output buffer in bytes */
  int outsize;
  /* the position of bytes read from the buffer */
  int inpos;
  /* the position of bytes written to the buffer */
  int outpos;
  /* next error found while reading */
  int last_result;
};

/*
 * Compute a header check value.
 * - hdr the header to check
 * @return the new check value. If this value is not equal to the current
 *   check value, the header may be damaged.
 */
PNGPARTS_API
int pngparts_z_header_check(struct pngparts_z_header hdr);
/*
 * Make a default header.
 * @return the new header
 */
PNGPARTS_API
struct pngparts_z_header pngparts_z_header_new(void);

/*
 * Put the header into a byte stream.
 * - buf buffer of bytes to which to write (2 bytes)
 * - hdr header to write
 */
PNGPARTS_API
void pngparts_z_header_put(void* buf, struct pngparts_z_header  hdr);
/*
 * Get the header from a byte stream.
 * - buf the buffer to read
 * @return the header
 */
PNGPARTS_API
struct pngparts_z_header pngparts_z_header_get(void const* buf);

/*
 * Create a new Adler32 checksum.
 * @return the new checksum
 */
PNGPARTS_API
struct pngparts_z_adler32 pngparts_z_adler32_new(void);
/*
 * Convert an accumulator to a single value.
 * - chk the accumulator
 * @return the single 32-bit checksum
 */
PNGPARTS_API
unsigned long int pngparts_z_adler32_tol(struct pngparts_z_adler32 chk);
/*
 * Accumulate a single byte.
 * - chk the current checksum accumulator state
 * - ch the character to accumulate
 * @return the new accumulator state
 */
PNGPARTS_API
struct pngparts_z_adler32 pngparts_z_adler32_accum
  (struct pngparts_z_adler32 chk, int ch);

/*
 * Setup an input buffer for next use.
 * - zs zlib stream struct (struct pngparts_z)
 * - inbuf input buffer
 * - insize amount of data to input
 */
PNGPARTS_API
void pngparts_z_setup_input
  (void *zs, void* inbuf, int insize);
/*
 * Setup an output buffer for next use.
 * - zs zlib stream struct (struct pngparts_z)
 * - outbuf output buffer
 * - outsize amount of space available for output
 */
PNGPARTS_API
void pngparts_z_setup_output
  (void *zs, void* outbuf, int outsize);
/*
 * Check if the reader has used up all the latest input.
 * - zs zlib stream struct (struct pngparts_z)
 * @return nonzero if the input is used up
 */
PNGPARTS_API
int pngparts_z_input_done(void const* zs);
/*
 * Check how much output bytes wait for you.
 * - zs zlib stream struct (struct pngparts_z)
 * @return byte count for the output bytes
 */
PNGPARTS_API
int pngparts_z_output_left(void const* zs);
/*
 * Set the compression callbacks.
 * - base the reader
 * - cb callback information structure
 */
PNGPARTS_API
void pngparts_z_set_cb
  ( struct pngparts_z *base, struct pngparts_api_flate const* cb);

#ifdef __cplusplus
};
#endif /*__cplusplus*/

#endif /*__PNG_PARTS_Z_H__*/
