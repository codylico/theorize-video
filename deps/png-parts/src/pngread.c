/*
 * PNG-parts
 * parts of a Portable Network Graphics implementation
 * Copyright 2018-2019 Cody Licorish
 *
 * Licensed under the MIT License.
 *
 * pngread.c
 * PNG reader source
 */

#include "pngread.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>


static unsigned long int pngparts_pngread_get32(unsigned char const*);
/*
 * Execute `calloc`, checking for integer overflow.
 * - sz the desired allocation size
 * @return a pointer to the allocation on success, NULL otherwise
 */
static void* pngparts_pngread_calloc(unsigned long int sz);

unsigned long int pngparts_pngread_get32(unsigned char const* b) {
  return (((unsigned long int)(b[0] & 255)) << 24)
    | (((unsigned long int)(b[1] & 255)) << 16)
    | (((unsigned long int)(b[2] & 255)) << 8)
    | (((unsigned long int)(b[3] & 255)) << 0);
}

void* pngparts_pngread_calloc(unsigned long int sz){
  /*
   * size_t is not guaranteed larger than long in C,
   *   so check against a runtime value
   */
  size_t const static max_sz = (size_t)(~0ul);
  if (sz > max_sz)
    return NULL;
  else return calloc((size_t)sz, sizeof(unsigned char));
}

void pngparts_pngread_init(struct pngparts_png* p) {
  p->state = 0;
  p->check = pngparts_png_crc32_new();
  p->shortpos = 0;
  p->last_result = 0;
  p->flags_tf = 0;
  p->chunk_cbs = NULL;
  p->active_chunk_cb = NULL;
  p->palette_count = 0;
  p->palette = NULL;
  return;
}
void pngparts_pngread_free(struct pngparts_png* p) {
  pngparts_png_drop_chunk_cbs(p);
  free(p->palette);
  p->palette = NULL;
  return;
}
int pngparts_pngread_parse(struct pngparts_png* p) {
  int result = p->last_result;
  int state = p->state;
  int shortpos = p->shortpos;
  while (result == PNGPARTS_API_OK
    &&  (p->pos < p->size))
  {
      /* states:
       * 0  - start
       * 1  - length, header
       * 2  - IEND contents
       * 3  - IEND CRC
       * 4  - done
       * 5  - unknown chunk contents
       * 6  - unknown chunk CRC
       * 7  - IHDR handling
       */
    int ch;
    if (p->flags_tf & PNGPARTS_PNG_REPEAT_CHAR) {
      /* put dummy character */
      ch = -1;
    } else if (p->pos < p->size)
      /* put actual character */ ch = p->buf[p->pos] & 255;
    else
      /* put dummy character */ ch = -1;
    switch (state) {
    case 0: /* signature */
      {
        if (ch >= 0 && shortpos < 8) {
          p->shortbuf[shortpos] = (unsigned char)ch;
          shortpos += 1;
        }
        if (shortpos >= 8) {
          int cmp_result = memcmp
            ( p->shortbuf, pngparts_png_signature(),
              8 * sizeof(unsigned char));
          if (cmp_result != 0){
            result = PNGPARTS_API_BAD_SIGNATURE;
            break;
          } else {
            state = 1;
            shortpos = 0;
          }
        }
      }break;
    case 1: /* length, header */
      {
        if (ch >= 0 && shortpos < 8) {
          p->shortbuf[shortpos] = (unsigned char)ch;
          shortpos += 1;
        }
        if (shortpos >= 8) {
          unsigned char chunk_name[4];
          /* chunk size */
          p->chunk_size = pngparts_pngread_get32(p->shortbuf);
          if (p->chunk_size > 0x7fffFFFF) {
            result = PNGPARTS_API_CHUNK_TOO_LONG;
            break;
          }
          /* chunk name */
          memcpy(chunk_name, p->shortbuf + 4, 4 * sizeof(unsigned char));
          /* prepare the crc */
          p->check = pngparts_png_crc32_new();
          {
            int i;
            for (i = 0; i < 4; ++i) {
              p->check = pngparts_png_crc32_accum(p->check, chunk_name[i]);
            }
          }
          if ((p->flags_tf & PNGPARTS_PNG_IHDR_DONE) == 0) {
            if (memcmp(chunk_name, "\x49\x48\x44\x52", 4) == 0) {
              if (p->chunk_size == 13) {
                state = 7;
                shortpos = 0;
                p->flags_tf |= PNGPARTS_PNG_IHDR_DONE;
              } else result = PNGPARTS_API_BAD_HDR;
            } else {
              result = PNGPARTS_API_MISSING_HDR;
            }
          } else if (memcmp(chunk_name, "\x49\x45\x4E\x44", 4) == 0) {
            /*end of PNG stream */
            if (p->chunk_size > 0) state = 2;
            else state = 3;
          } else {
            /* try to find the matching chunk callback */
            struct pngparts_png_chunk_cb const*
              link = pngparts_png_find_chunk_cb(p, chunk_name);
            if (link != NULL) {
              struct pngparts_png_message message;
              message.byte = 0;
              memcpy(message.name, chunk_name, 4 * sizeof(unsigned char));
              message.ptr = NULL;
              message.type = PNGPARTS_PNG_M_START;
              result = pngparts_png_send_chunk_msg(p, link, &message);
              if (result == PNGPARTS_API_OK) {
                p->active_chunk_cb = link;
                if (p->chunk_size > 0) state = 8;
                else state = 9;
                shortpos = 0;
              } else /* result is set, */break;
            } else if ((chunk_name[0] & 0x20) == 0) {
              result = PNGPARTS_API_UNCAUGHT_CRITICAL;
            } else {
              /* dummy stream */
              if (p->chunk_size > 0) state = 5;
              else state = 6;
            }
          }
          shortpos = 0;
        }
      }break;
    case 2: /* IEND chunk data */
      {
        if (p->chunk_size > 0 && ch >= 0) {
          p->check = pngparts_png_crc32_accum(p->check, ch);
          p->chunk_size -= 1;
        }
        if (p->chunk_size == 0) {
          state = 3;
        }
      }break;
    case 3: /* IEND CRC */
      {
        if (shortpos < 4 && ch >= 0) {
          p->shortbuf[shortpos] = (unsigned char)ch;
          shortpos += 1;
        }
        if (shortpos >= 4) {
          /* check the sum */
          unsigned long int stream_chk
            = pngparts_pngread_get32(p->shortbuf);
          if (pngparts_png_crc32_tol(p->check) != stream_chk) {
            result = PNGPARTS_API_BAD_SUM;
          } else {
            /* notify done */
            shortpos = 0;
            result = PNGPARTS_API_DONE;
            if (ch >= 0) p->pos += 1;/*not else */
            state = 4;
          }
          /* notify end of file */ {
            struct pngparts_png_message message;
            int broadcast_feedback;
            message.byte = 0;
            message.ptr = NULL;
            message.type = PNGPARTS_PNG_M_ALL_DONE;
            broadcast_feedback =
              pngparts_png_broadcast_chunk_msg(p, &message);
            if (broadcast_feedback < 0)
              result = broadcast_feedback;
          }
        }
      }break;
    case 4:
      {
        result = PNGPARTS_API_DONE;
      }break;
    case 5: /* unknown chunk data */
      {
        if (p->chunk_size > 0 && ch >= 0) {
          p->check = pngparts_png_crc32_accum(p->check, ch);
          p->chunk_size -= 1;
        }
        if (p->chunk_size == 0) {
          state = 6;
        }
      }break;
    case 6: /* other CRC */
      {
        if (shortpos < 4 && ch >= 0) {
          p->shortbuf[shortpos] = (unsigned char)ch;
          shortpos += 1;
        }
        if (shortpos >= 4) {
          /* check the sum */
          unsigned long int stream_chk
            = pngparts_pngread_get32(p->shortbuf);
          if (pngparts_png_crc32_tol(p->check) != stream_chk) {
            result = PNGPARTS_API_BAD_CRC;
          } else {
            /* notify done */
            shortpos = 0;
            result = PNGPARTS_API_OK;
            state = 1;
          }
        }
      }break;
    case 7: /* IHDR handling */
      {
        if (shortpos < 13 && ch >= 0) {
          p->check = pngparts_png_crc32_accum(p->check, ch);
          p->shortbuf[shortpos] = (unsigned char)ch;
          shortpos += 1;
        }
        if (shortpos >= 13) {
          /* extract the header */
          p->header.width = pngparts_pngread_get32(p->shortbuf + 0);
          p->header.height = pngparts_pngread_get32(p->shortbuf + 4);
          p->header.bit_depth = (int)*(p->shortbuf + 8);
          p->header.color_type = (int)*(p->shortbuf + 9);
          p->header.compression = (int)*(p->shortbuf + 10);
          p->header.filter = (int)*(p->shortbuf + 11);
          p->header.interlace = (int)*(p->shortbuf + 12);
          /* confirm the header */
          if (!pngparts_png_header_is_valid(p->header)) {
            result = PNGPARTS_API_BAD_HDR;
            break;
          }
          /* report the header */
          if (p->img_cb.start_cb != NULL) {
            result = (*p->img_cb.start_cb)(p->img_cb.cb_data,
              p->header.width, p->header.height,
              p->header.bit_depth, p->header.color_type,
              p->header.compression, p->header.filter,
              p->header.interlace);
            if (result != PNGPARTS_API_OK) break;
          }
          /* check the CRC32 */
          state = 6;
          shortpos = 0;
        }
      }break;
    case 8: /* callback chunk data */
      {
        if (p->chunk_size > 0 && ch >= 0) {
          unsigned long int const chunk_size = p->chunk_size;
          struct pngparts_png_crc32 const check = p->check;
          /* notify */ {
            struct pngparts_png_message message;
            message.byte = ch;
            memcpy(message.name, p->active_chunk_cb->name,
              4 * sizeof(unsigned char));
            message.ptr = NULL;
            message.type = PNGPARTS_PNG_M_GET;
            result = pngparts_png_send_chunk_msg
              (p, p->active_chunk_cb, &message);
          }
          p->check = pngparts_png_crc32_accum(check, ch);
          p->chunk_size = chunk_size-1;
        }
        if (p->chunk_size == 0) {
          state = 9;
          shortpos = 0;
        }
      }break;
    case 9: /* callback CRC */
      {
        if (shortpos < 4 && ch >= 0) {
          p->shortbuf[shortpos] = (unsigned char)ch;
          shortpos += 1;
        }
        if (shortpos >= 4) {
          /* check the sum */
          unsigned long int stream_chk
            = pngparts_pngread_get32(p->shortbuf);
          if (pngparts_png_crc32_tol(p->check) != stream_chk) {
            /* notify */ {
              struct pngparts_png_message message;
              message.byte = PNGPARTS_API_BAD_CRC;
              memcpy(message.name, p->active_chunk_cb->name,
                4 * sizeof(unsigned char));
              message.ptr = NULL;
              message.type = PNGPARTS_PNG_M_FINISH;
              result = pngparts_png_send_chunk_msg
                (p, p->active_chunk_cb, &message);
              if (result >= 0)
                result = PNGPARTS_API_BAD_CRC;
            }
          } else {
            /* notify */ {
              struct pngparts_png_message message;
              message.byte = PNGPARTS_API_OK;
              memcpy(message.name, p->active_chunk_cb->name,
                4 * sizeof(unsigned char));
              message.ptr = NULL;
              message.type = PNGPARTS_PNG_M_FINISH;
              result = pngparts_png_send_chunk_msg
                (p, p->active_chunk_cb, &message);
            }
            shortpos = 0;
            state = 1;
          }
        }
      }break;
    default:
      {
        result = PNGPARTS_API_BAD_STATE;
      }break;
    }
    p->state = state;
    p->shortpos = shortpos;
    if (result != PNGPARTS_API_OK) {
      break;
    } else if (p->flags_tf & PNGPARTS_PNG_REPEAT_CHAR) {
      /* reset the flag */
      p->flags_tf &= ~PNGPARTS_PNG_REPEAT_CHAR;
      p->pos += 1;
    } else if (ch >= 0) {
      p->pos += 1;
    }
  }
  p->last_result = result;
  p->state = (short)state;
  p->shortpos = (short)shortpos;
  if (result) {
    p->flags_tf |= PNGPARTS_PNG_REPEAT_CHAR;
  }
  return result;
}

/*BEGIN IDAT*/
struct pngparts_pngread_idat {
  int level;
  int pixel_size;
  unsigned long int line_width;
  unsigned long int line_height;
  long int x;
  long int y;
  struct pngparts_api_z z;
  /* amount of next pixel space remaining */
  int next_left;
  /* current and same-line previous pixel data */
  unsigned char nextbuf[16];
  /* previous scan line */
  unsigned char *outbuf;
  unsigned long int outsize;
  unsigned long int outpos;
  int filter_mode;
  unsigned long int byte_count;
};
static int pngparts_pngread_start_line
  (struct pngparts_png*, struct pngparts_pngread_idat*);
static int pngparts_pngread_idat_msg
  (struct pngparts_png*, void* cb_data, struct pngparts_png_message* msg);
static void pngparts_pngread_idat_shift
  (struct pngparts_pngread_idat*, int shift);
static void pngparts_pngread_idat_add
  (struct pngparts_pngread_idat*, int shift);
static void pngparts_pngread_idat_submit
  (struct pngparts_png*, struct pngparts_pngread_idat* idat);

void pngparts_pngread_idat_submit
  (struct pngparts_png* p, struct pngparts_pngread_idat* idat)
{
  int const color_type = p->header.color_type;
  static const int multiplier[9] =
    { 0, 0xffff, 0x5555, 0x2492, 0x1111, 0, 0, 0, 0x0101 };
  struct pngparts_api_image img;
  pngparts_png_get_image_cb(p, &img);
  switch (idat->pixel_size) {
  case 1: /* either L/1 or index/1 */
  case 2: /* either L/2 or index/2 */
  case 4: /* either L/4 or index/4 */
  case 8: /* either L/8 or index/8 */
    {
      int i, count = 8/idat->pixel_size;
      int mask = (1 << idat->pixel_size) - 1;
      long int nx, ny;
      for (i = count-1; i >= 0; --i) {
        pngparts_png_adam7_reverse_xy(idat->level, &nx, &ny, idat->x, idat->y);
        if (nx < p->header.width) {
          unsigned int const bit_string =
            (idat->nextbuf[8] >> (i*idat->pixel_size))&mask;
          switch (color_type) {
          case 0: /* L/i */
            {
              unsigned int const lumin
                = bit_string*multiplier[idat->pixel_size];
              (*img.put_cb)(img.cb_data, nx, ny, lumin, lumin, lumin, 65535);
            }break;
          case 3: /* index/i */
            {
              struct pngparts_png_plte_item color;
              if (bit_string < pngparts_png_get_plte_size(p)) {
                color = pngparts_png_get_plte_item(p, bit_string);
                (*img.put_cb)(img.cb_data, nx, ny,
                  (color.red << 8) | color.red,
                  (color.green << 8) | color.green,
                  (color.blue << 8) | color.blue,
                  (color.alpha << 8) | color.alpha);
              } else {
                /* skip this pixel */
              }
            }break;
          }
          idat->x += 1;
        }
      }
      pngparts_pngread_idat_add(idat, 1);
      pngparts_pngread_idat_shift(idat, 1);
    }break;
  case 16: /* either L/16, LA/8 */
    {
      long int nx, ny;
      pngparts_png_adam7_reverse_xy(idat->level, &nx, &ny, idat->x, idat->y);
      if (nx < p->header.width) {
        switch (color_type) {
        case 0: /* L/16 */
          {
            unsigned int lumin;
            lumin = (idat->nextbuf[8] << 8) | (idat->nextbuf[9]);
            (*img.put_cb)(img.cb_data, nx, ny, lumin, lumin, lumin, 65535);
          }break;
        case 4: /* LA/8 */
          {
            unsigned int lumin, alpha;
            lumin = (idat->nextbuf[8] << 8) | (idat->nextbuf[8]);
            alpha = (idat->nextbuf[9] << 8) | (idat->nextbuf[9]);
            (*img.put_cb)(img.cb_data, nx, ny, lumin, lumin, lumin, alpha);
          }break;
        }
      }
      pngparts_pngread_idat_add(idat, 2);
      pngparts_pngread_idat_shift(idat, 2);
      idat->x += 1;
    }break;
  case 24: /* either RGB/8 */
    {
      long int nx, ny;
      pngparts_png_adam7_reverse_xy(idat->level, &nx, &ny, idat->x, idat->y);
      if (nx < p->header.width) {
        unsigned int red, green, blue;
        red = (idat->nextbuf[8] << 8) | (idat->nextbuf[8]);
        green = (idat->nextbuf[9] << 8) | (idat->nextbuf[9]);
        blue = (idat->nextbuf[10] << 8) | (idat->nextbuf[10]);
        (*img.put_cb)(img.cb_data, nx, ny, red, green, blue, 65535);
      }
      pngparts_pngread_idat_add(idat, 3);
      pngparts_pngread_idat_shift(idat, 3);
      idat->x += 1;
    }break;
  case 32: /* either LA/16 or RGBA/8 */
    {
      long int nx, ny;
      pngparts_png_adam7_reverse_xy(idat->level, &nx, &ny, idat->x, idat->y);
      if (nx < p->header.width) {
        switch (color_type) {
        case 4: /* LA/16 */
          {
            unsigned int lumin, alpha;
            lumin = (idat->nextbuf[8] << 8) | (idat->nextbuf[9]);
            alpha = (idat->nextbuf[10] << 8) | (idat->nextbuf[11]);
            (*img.put_cb)(img.cb_data, nx, ny, lumin, lumin, lumin, alpha);
          }break;
        case 6: /* RGBA/8 */
          {
            unsigned int red, green, blue, alpha;
            red = (idat->nextbuf[8] << 8) | (idat->nextbuf[8]);
            green = (idat->nextbuf[9] << 8) | (idat->nextbuf[9]);
            blue = (idat->nextbuf[10] << 8) | (idat->nextbuf[10]);
            alpha = (idat->nextbuf[11] << 8) | (idat->nextbuf[11]);
            (*img.put_cb)(img.cb_data, nx, ny, red, green, blue, alpha);
          }break;
        }
      }
      pngparts_pngread_idat_add(idat, 4);
      pngparts_pngread_idat_shift(idat, 4);
      idat->x += 1;
    }break;
  case 48: /* only RGB/16 */
    {
      long int nx, ny;
      pngparts_png_adam7_reverse_xy(idat->level, &nx, &ny, idat->x, idat->y);
      if (nx < p->header.width) {
        unsigned int red, green, blue;
        red   = (idat->nextbuf[ 8] << 8) | (idat->nextbuf[ 9]);
        green = (idat->nextbuf[10] << 8) | (idat->nextbuf[11]);
        blue  = (idat->nextbuf[12] << 8) | (idat->nextbuf[13]);
        (*img.put_cb)(img.cb_data, nx, ny, red, green, blue, 65535);
      }
      pngparts_pngread_idat_add(idat, 6);
      pngparts_pngread_idat_shift(idat, 6);
      idat->x += 1;
    }break;
  case 64: /* only RGBA/16 */
    {
      long int nx, ny;
      pngparts_png_adam7_reverse_xy(idat->level, &nx, &ny, idat->x, idat->y);
      if (nx < p->header.width) {
        unsigned int red, green, blue, alpha;
        red  = (idat->nextbuf[ 8] << 8) | (idat->nextbuf[ 9]);
        green= (idat->nextbuf[10] << 8) | (idat->nextbuf[11]);
        blue = (idat->nextbuf[12] << 8) | (idat->nextbuf[13]);
        alpha= (idat->nextbuf[14] << 8) | (idat->nextbuf[15]);
        (*img.put_cb)(img.cb_data, nx, ny, red, green, blue, alpha);
      }
      pngparts_pngread_idat_add(idat, 8);
      pngparts_pngread_idat_shift(idat, 8);
      idat->x += 1;
    }break;
  }
}
void pngparts_pngread_idat_shift(struct pngparts_pngread_idat* d, int shift) {
  d->next_left += shift;
  memmove(d->nextbuf, d->nextbuf + shift, 16 - shift);
}
void pngparts_pngread_idat_add(struct pngparts_pngread_idat* d, int shift) {
  if (d->outpos >= shift) {
    memcpy(d->outbuf + d->outpos - shift, d->nextbuf + 8 - shift, shift);
  }
  d->outpos += shift;
}
int pngparts_pngread_start_line
  (struct pngparts_png* p, struct pngparts_pngread_idat* idat)
{
  /*compute line length*/
  unsigned long int buffer_length;
  {
    unsigned long int line_length = idat->pixel_size;
    struct pngparts_png_size const line_size =
      pngparts_png_adam7_pass_size(
        p->header.width, p->header.height, idat->level
      );
    idat->line_width = line_size.width;
    idat->line_height = line_size.height;
    /*fprintf(stderr, "pass %i: width %lu height %lu\n",
      idat->level, idat->line_width, idat->line_height);*/
    if (idat->line_width == 0 || idat->line_height == 0) {
      return PNGPARTS_API_OVERFLOW;
    }
    if (idat->pixel_size == 0
    ||  idat->line_width >= ULONG_MAX/idat->pixel_size)
    {
      return PNGPARTS_API_TOO_WIDE;
    }
    line_length = idat->line_width*idat->pixel_size;
    buffer_length = (line_length + 7) >> 3;
  }
  if (idat->outsize != buffer_length) {
    /* resize the buffer */
    unsigned char* new_buffer;
    free(idat->outbuf);
    idat->outbuf = NULL;
    new_buffer = (unsigned char*)pngparts_pngread_calloc(buffer_length);
    if (new_buffer == NULL) {
      return PNGPARTS_API_MEMORY;
    }
    idat->outbuf = new_buffer;
    idat->outsize = buffer_length;
  } else {
    memset(idat->outbuf, 0, idat->outsize * sizeof(unsigned char));
  }
  memset(idat->nextbuf, 0, 8*sizeof(unsigned char));
  idat->outpos = 0;
  idat->filter_mode = -1;
  return PNGPARTS_API_OK;
}
int pngparts_pngread_idat_msg
  (struct pngparts_png* p, void* cb_data, struct pngparts_png_message* msg)
{
  int result;
  struct pngparts_pngread_idat *idat =
    (struct pngparts_pngread_idat *)cb_data;
  switch (msg->type) {
  case PNGPARTS_PNG_M_READY:
    {
      result = PNGPARTS_API_OK;
    }break;
  case PNGPARTS_PNG_M_START:
    {
      if (idat->level == -1) {
        idat->byte_count = 0;
        /* get level */
        if (p->header.interlace == 1) {
          idat->level = 1;
          result = PNGPARTS_API_OK;
        } else if (p->header.interlace == 0) {
          idat->level = 0;
          result = PNGPARTS_API_OK;
        } else {
          result = PNGPARTS_API_UNSUPPORTED;
          break;
        }
        /* compute the sample size in bytes */ {
          switch (p->header.color_type) {
          case 0: /* gray */
          case 3: /* palette */
            idat->pixel_size = p->header.bit_depth;
            break;
          case 4: /* gray alpha */
            idat->pixel_size = p->header.bit_depth * 2;
            break;
          case 2: /* red green blue*/
            idat->pixel_size = p->header.bit_depth * 3;
            break;
          case 6: /* red green blue alpha*/
            idat->pixel_size = p->header.bit_depth * 4;
            break;
          }
        }
        /* prepare the line */{
          int line_out = pngparts_pngread_start_line(p, idat);
          if (line_out == PNGPARTS_API_OVERFLOW) {
            /* give up */
            idat->filter_mode = 5;
            break;
          } else if (line_out == PNGPARTS_API_MEMORY){
            result = PNGPARTS_API_MEMORY;
            break;
          }
        }
        idat->filter_mode = -1;
        idat->next_left = 8;
        idat->x = 0;
        idat->y = 0;
      } else result = PNGPARTS_API_OK;
    }break;
  case PNGPARTS_PNG_M_GET:
    {
      int z_result;
      int gold = 0;/* continuation flag */
      unsigned char inbuf[1];
      int const pixel_byte_size = ((idat->pixel_size + 7) / 8);
      int const pixel_left = 8 - pixel_byte_size;
      inbuf[0] = (unsigned char)(msg->byte & 255);
      /*fprintf(stderr, "x+%2x\n", inbuf[0]);*/
      (*idat->z.set_input_cb)(idat->z.cb_data, inbuf, 1);
      do {
        gold = 0;
        if (idat->next_left > 0) {
          (*idat->z.set_output_cb)(idat->z.cb_data,
            idat->nextbuf + 16 - idat->next_left, idat->next_left);
          z_result = (*idat->z.churn_cb)
            (idat->z.cb_data, PNGPARTS_API_Z_NORMAL);
          if (z_result < 0){
            /* the zlib stream is corrupted; give up and */break;
          }
        } else z_result = PNGPARTS_API_OK;
        /* report bytes */ {
          unsigned int byte_count = (*idat->z.output_left_cb)(idat->z.cb_data);
          idat->byte_count += byte_count;
          /* print bytes *//*{
            fprintf(stderr, "\tbytes: [");
            unsigned int bi;
            unsigned char*const report_buf =
              idat->nextbuf + 16 - idat->next_left;
            for (bi = 0; bi < byte_count; ++bi) {
              fprintf(stderr, "%s%2x", bi ? ", " : "", report_buf[bi]);
            }
            fprintf(stderr, "]\n");
          }*/
          idat->next_left -= byte_count;
        }
        switch (idat->filter_mode) {
        case -1: /* filter search */
          {
            if (idat->next_left <= 7) {
              /* filter code available */
              int const filter_code = idat->nextbuf[8];
              pngparts_pngread_idat_shift(idat, 1);
              memset(idat->nextbuf, 0, 8 * sizeof(unsigned char));
              if (filter_code > 4) {
                z_result = PNGPARTS_API_WEIRD_FILTER;
                break;
              }
              /*fprintf(stderr, "line %3li: filter %i\n",
                idat->y, filter_code);*/
              idat->filter_mode = filter_code;
              gold = 1;
            }
          }break;
        case 0:
          {
            if (idat->next_left <= pixel_left) {
              /* null filter */
              pngparts_pngread_idat_submit(p,idat);
              gold = 1;
            }
          }break;
        case 1:
          {
            if (idat->next_left <= pixel_left) {
              /* subtraction filter */
              int i;
              for (i = 0; i < pixel_byte_size; ++i) {
                idat->nextbuf[8 + i] = (idat->nextbuf[8 + i]
                  + idat->nextbuf[pixel_left + i]) & 255;
              }
              pngparts_pngread_idat_submit(p, idat);
              gold = 1;
            }
          }break;
        case 2:
          {
            if (idat->next_left <= pixel_left) {
              /* up filter */
              int i;
              for (i = 0; i < pixel_byte_size; ++i) {
                idat->nextbuf[8 + i] = (idat->nextbuf[8 + i]
                  + idat->outbuf[idat->outpos + i]) & 255;
              }
              pngparts_pngread_idat_submit(p, idat);
              gold = 1;
            }
          }break;
        case 3:
          {
            if (idat->next_left <= pixel_left) {
              /* average filter */
              int i;
              for (i = 0; i < pixel_byte_size; ++i) {
                unsigned int average = idat->outbuf[idat->outpos + i];
                average += idat->nextbuf[pixel_left + i];
                idat->nextbuf[8 + i] =
                  (idat->nextbuf[8+i]+(average >> 1))&255;
              }
              pngparts_pngread_idat_submit(p, idat);
              gold = 1;
            }
          }break;
        case 4:
          {
            if (idat->next_left <= pixel_left) {
              /* Paeth filter */
              int i;
              for (i = 0; i < pixel_byte_size; ++i) {
                int const corner_byte = (idat->outpos >= pixel_byte_size)
                  ? idat->outbuf[idat->outpos - pixel_byte_size + i]
                  : 0;
                idat->nextbuf[8 + i] = (pngparts_png_paeth_predict
                  ( idat->nextbuf[pixel_left + i],
                    idat->outbuf[idat->outpos + i],
                    corner_byte) + idat->nextbuf[8 + i]) & 255;
              }
              pngparts_pngread_idat_submit(p, idat);
              gold = 1;
            }
          }break;
        case 5:
          {
            /* done */
          }break;
        }
        if (z_result == PNGPARTS_API_WEIRD_FILTER){
          /* the scanline stream is broken, so */break;
        }
        if (idat->x >= idat->line_width) {
          pngparts_pngread_idat_add(idat, pixel_byte_size);
          idat->x = 0;
          idat->y += 1;
          idat->outpos = 0;
          memset(idat->nextbuf, 0, 8*sizeof(unsigned char));
          idat->filter_mode = -1;
        }
        if (idat->y >= idat->line_height) {
          /* continue to next phase */
          if (idat->level == 0 || idat->level == 7) {
            /* cease translation */
            idat->filter_mode = 5;
          } else {
            int level_result;
            idat->x = 0;
            idat->y = 0;
            idat->filter_mode = -1;
            do {
              idat->level += 1;
              level_result =
                pngparts_pngread_start_line(p, idat);
              if (level_result != PNGPARTS_API_OVERFLOW) break;
            } while (idat->level <= 7);
            if (level_result == PNGPARTS_API_OVERFLOW) {
              idat->filter_mode = 5;
            }
          }
        }
      } while (z_result == PNGPARTS_API_OVERFLOW || gold);
      result = z_result;
      /*fprintf(stderr, "byte count: %lu\n", idat->byte_count);*/
      if (result == PNGPARTS_API_DONE) {
        /* last chance check */
        if (idat->filter_mode == 5)
          result = PNGPARTS_API_OK;
        else
          result = PNGPARTS_API_SHORT_IDAT;
      }
    }break;
  case PNGPARTS_PNG_M_FINISH:
    {
      result = PNGPARTS_API_OK;
    }break;
  case PNGPARTS_PNG_M_ALL_DONE:
    {
      if (idat->filter_mode == 5)
        result = PNGPARTS_API_OK;
      else
        result = PNGPARTS_API_SHORT_IDAT;
    }break;
  case PNGPARTS_PNG_M_DESTROY:
    {
      free(idat->outbuf);
      free(idat);
      result = PNGPARTS_API_OK;
    }break;
  default:
    result = PNGPARTS_API_BAD_STATE;
    break;
  }
  return result;
}
int pngparts_pngread_assign_idat_api
  (struct pngparts_png_chunk_cb* cb, struct pngparts_api_z const* z)
{
  unsigned char static const name[4] = { 0x49,0x44,0x41,0x54 };
  struct pngparts_pngread_idat* ptr = (struct pngparts_pngread_idat*)malloc
    (sizeof(struct pngparts_pngread_idat));
  if (ptr == NULL) {
    return PNGPARTS_API_MEMORY;
  } else {
    memcpy(cb->name, name, 4 * sizeof(unsigned char));
    memcpy(&ptr->z, z, sizeof(*z));
    ptr->level = -1;
    ptr->outbuf = NULL;
    ptr->outsize = 0;
    ptr->outpos = 0;
    ptr->filter_mode = -2;
    cb->cb_data = ptr;
    cb->message_cb = pngparts_pngread_idat_msg;
    return PNGPARTS_API_OK;
  }
}
/*END   IDAT*/

/*BEGIN PLTE*/
static int pngparts_pngread_plte_msg
  (struct pngparts_png* p, void* cb_data, struct pngparts_png_message* msg);

struct pngparts_pngread_plte {
  int pos;
  int sample;
  int done;
  struct pngparts_png_plte_item color;
};
int pngparts_pngread_plte_msg
  (struct pngparts_png* p, void* cb_data, struct pngparts_png_message* msg)
{
  int result;
  struct pngparts_pngread_plte *plte =
    (struct pngparts_pngread_plte *)cb_data;
  switch (msg->type) {
  case PNGPARTS_PNG_M_READY:
    {
      result = plte->done ? PNGPARTS_API_DONE : PNGPARTS_API_OK;
    }break;
  case PNGPARTS_PNG_M_START:
    {
      /* compute palette size */
      long int chunk_size = pngparts_png_chunk_remaining(p);
      int palette_size;
      if (chunk_size > 768) {
        palette_size = 256;
      } else {
        palette_size = chunk_size / 3;
      }
      /* prepare palette */ {
        result = pngparts_png_set_plte_size(p, palette_size);
      }
      plte->pos = 0;
      plte->sample = 0;
      plte->color.alpha = 255;
    }break;
  case PNGPARTS_PNG_M_GET:
    {
      switch (plte->sample) {
      case 0:
        plte->color.red = (unsigned char)(msg->byte);
        plte->sample += 1;
        break;
      case 1:
        plte->color.green = (unsigned char)(msg->byte);
        plte->sample += 1;
        break;
      case 2:
        plte->color.blue = (unsigned char)(msg->byte);
        /* set the color to the palette */ {
          pngparts_png_set_plte_item(p, plte->pos, plte->color);
        }
        plte->sample = 0;
        plte->pos += 1;
        if (plte->pos >= pngparts_png_get_plte_size(p)) {
          plte->sample = 4;
        }
        break;
      case 4:
        /* unused */
        break;
      }
      result = PNGPARTS_API_OK;
    }break;
  case PNGPARTS_PNG_M_FINISH:
    {
      result = PNGPARTS_API_OK;
    }break;
  case PNGPARTS_PNG_M_ALL_DONE:
    {
      result = PNGPARTS_API_OK;
    }break;
  case PNGPARTS_PNG_M_DESTROY:
    {
      free(plte);
      result = PNGPARTS_API_OK;
    }break;
  default:
    result = PNGPARTS_API_BAD_STATE;
    break;
  }
  return result;
}
int pngparts_pngread_assign_plte_api(struct pngparts_png_chunk_cb* cb){
  unsigned char static const name[4] = { 0x50,0x4C,0x54,0x45 };
  struct pngparts_pngread_plte* ptr = (struct pngparts_pngread_plte*)malloc
    (sizeof(struct pngparts_pngread_plte));
  if (ptr == NULL) {
    return PNGPARTS_API_MEMORY;
  } else {
    memcpy(cb->name, name, 4 * sizeof(unsigned char));
    ptr->pos = -1;
    ptr->sample = 0;
    ptr->done = 0;
    cb->cb_data = ptr;
    cb->message_cb = pngparts_pngread_plte_msg;
    return PNGPARTS_API_OK;
  }
}
/*END   PLTE*/
