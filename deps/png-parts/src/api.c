/*
 * PNG-parts
 * parts of a Portable Network Graphics implementation
 * Copyright 2018-2019 Cody Licorish
 *
 * Licensed under the MIT License.
 *
 * api.h
 * API main header
 */
#include "api.h"
#include <stddef.h>

int pngparts_api_info(void){
  int out = 0;
#ifdef PNGPARTS_API_SHARED
  out |= PNGPARTS_API_EXPORTS;
#endif /*PNGPARTS_API_SHARED*/
  return out;
}

char const* pngparts_api_strerror(int result){
  switch (result){
  case PNGPARTS_API_TOO_WIDE: return "image width too large to process";
  case PNGPARTS_API_MISSING_PUT: return "missing output byte from callback";
  case PNGPARTS_API_LOOPED_STATE: return "state machine caught in a loop";
  case PNGPARTS_API_CHUNK_TOO_LONG: return "chunk length too large";
  case PNGPARTS_API_SHORT_IDAT: return "too few IDAT chunk data for pixels";
  case PNGPARTS_API_WEIRD_FILTER: return "weird filter value encountered";
  case PNGPARTS_API_UNCAUGHT_CRITICAL: return "unhandled critical chunk";
  case PNGPARTS_API_BAD_HDR: return "corrupted IHDR chunk";
  case PNGPARTS_API_MISSING_HDR: return "missing IHDR from start of stream";
  case PNGPARTS_API_BAD_CRC: return "bad CRC32 checksum";
  case PNGPARTS_API_BAD_SIGNATURE: return "bad file signature";
  case PNGPARTS_API_WRONG_DICT: return "dictionary given was wrong";
  case PNGPARTS_API_BAD_CODE_LENGTH: return "bad code length";
  case PNGPARTS_API_NOT_FOUND: return "value not found";
  case PNGPARTS_API_CODE_EXCESS: return "values overflowed the bit string";
  case PNGPARTS_API_BAD_BLOCK: return "bad block type";
  case PNGPARTS_API_CORRUPT_LENGTH: return "direct length corrupted";
  case PNGPARTS_API_MEMORY: return "memory loss";
  case PNGPARTS_API_BAD_SUM: return "bad Adler32 checksum";
  case PNGPARTS_API_UNSUPPORTED: return "unsupported technique";
  case PNGPARTS_API_BAD_BITS: return "bad bit string";
  case PNGPARTS_API_OVERFLOW: return "output overflow";
  case PNGPARTS_API_IO_ERROR: return "input/output error";
  case PNGPARTS_API_BAD_PARAM: return "bad parameter";
  case PNGPARTS_API_NEED_DICT: return "dictionary requested";
  case PNGPARTS_API_BAD_CHECK: return "bad header check";
  case PNGPARTS_API_BAD_STATE: return "bad state machine";
  case PNGPARTS_API_EOF: return "stream too quick to finish";
  case PNGPARTS_API_OK: return "all is good";
  case PNGPARTS_API_DONE: return "the stream is done";
  case PNGPARTS_API_NOT_READY: return "the callback is not ready";
  default: return "?";
  }
}

struct pngparts_api_flate pngparts_api_flate_empty(void){
  struct pngparts_api_flate out = {NULL,NULL,NULL,NULL,NULL};
  return out;
}

struct pngparts_api_z pngparts_api_z_empty(void){
  struct pngparts_api_z out = {NULL,NULL,NULL,NULL,NULL,NULL,NULL};
  return out;
}
