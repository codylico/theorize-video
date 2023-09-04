/*
 * PNG-parts
 * parts of a Portable Network Graphics implementation
 * Copyright 2019 Cody Licorish
 *
 * Licensed under the MIT License.
 *
 * zwrite.c
 * zlib writer source
 */
#include "zwrite.h"
#include <stdlib.h>
#include <string.h>

static int pngparts_zwrite_put_cb(void* prs, int ch);
static void pngparts_zwrite_put32(unsigned char* , unsigned long int );

void pngparts_zwrite_put32(unsigned char* b, unsigned long int v){
  b[3] = (v>>0)&255;
  b[2] = (v>>8)&255;
  b[1] = (v>>16)&255;
  b[0] = (v>>24)&255;
  return;
}

void pngparts_zwrite_init(struct pngparts_z *zs){
  zs->state = 0;
  zs->header = pngparts_z_header_new();
  zs->shortpos = 0;
  memset(zs->shortbuf,0,sizeof(zs->shortbuf));
  zs->check = pngparts_z_adler32_new();
  zs->inbuf = NULL;
  zs->inpos = 0;
  zs->insize = 0;
  zs->outbuf = NULL;
  zs->outpos = 0;
  zs->outsize = 0;
  zs->flags_tf = 0;
  zs->cb = pngparts_api_flate_empty();
  zs->last_result = 0;
  return;
}

void pngparts_zwrite_free(struct pngparts_z *zs){
  return;
}

void pngparts_zwrite_assign_api
  (struct pngparts_api_z * dst, struct pngparts_z * src)
{
  dst->cb_data = src;
  dst->churn_cb = pngparts_zwrite_generate;
  dst->input_done_cb = pngparts_z_input_done;
  dst->output_left_cb = pngparts_z_output_left;
  dst->set_dict_cb = pngparts_zwrite_set_dictionary;
  dst->set_input_cb = pngparts_z_setup_input;
  dst->set_output_cb = pngparts_z_setup_output;
}

int pngparts_zwrite_generate(void *zs_v, int mode){
  struct pngparts_z *zs = (struct pngparts_z *)zs_v;
  int result = zs->last_result;
  int state = zs->state;
  int sticky_finish = ((mode&PNGPARTS_API_Z_FINISH) != 0);
  int trouble_counter = 0;
  if (result == PNGPARTS_API_OVERFLOW){
    if (zs->outpos < zs->outsize)
      result = PNGPARTS_API_OK;
  }
  while (result == PNGPARTS_API_OK
  &&     (sticky_finish || zs->inpos < zs->insize)){
    /* states:
     * 0  - start
     * 1  - dictionary checksum
     * 2  - data
     * 3  - adler32 checksum
     * 4  - done
     */
    int ch;
    int skip_back = 0;
    if (zs->flags_tf&2) {
      /* put dummy character */
      ch = -1;
    } else if (zs->inpos < zs->insize)
      /* put actual character */ ch = zs->inbuf[zs->inpos]&255;
    else
      /* put dummy character */ ch = -1;
    switch (state){
    case 0:
      if (zs->shortpos == 0){
        if ((zs->flags_tf&1) == 0){
          /* reconcile the dictionary flag */{
            zs->header.fdict = 0;
          }
          /* check and confirm the header */{
            int flate_result;
            zs->header.fcheck = pngparts_z_header_check(zs->header);
            flate_result = (*zs->cb.start_cb)(
                  zs->cb.cb_data, zs->header.fdict,
                  zs->header.flevel, zs->header.cm, zs->header.cinfo
                );
            if (flate_result != PNGPARTS_API_OK){
              result = flate_result;
              break;
            }
          }
        } else {
          /* start has already been called */
        }
        /* generate the bytes for the header */{
          pngparts_z_header_put(zs->shortbuf, zs->header);
        }
      }
      if (zs->shortpos < 2){
        skip_back = 1;
        result = pngparts_zwrite_put_cb(zs, zs->shortbuf[zs->shortpos]);
        if (result != PNGPARTS_API_OK)
          break;
        zs->shortpos += 1;
      }
      if (zs->shortpos >= 2){
        if (zs->flags_tf&1){
          state = 1;
          zs->shortpos = 2;
        } else {
          state = 2;
          zs->shortpos = 0;
        }
      }
      break;
    case 1:
      /* a prior function has already written the dictionary code */
      if (zs->shortpos < 6){
        skip_back = 1;
        result = pngparts_zwrite_put_cb(zs, zs->shortbuf[zs->shortpos]);
        if (result != PNGPARTS_API_OK)
          break;
        zs->shortpos += 1;
      }
      if (zs->shortpos >= 6){
        state = 2;
        zs->shortpos = 0;
      }
      break;
    case 2:
      /* churn the input byte */if (zs->inpos < zs->insize){
        result = (*zs->cb.one_cb)(
            zs->cb.cb_data, ch, zs, &pngparts_zwrite_put_cb
          );
        if (result != PNGPARTS_API_OK)
          break;
        /* check the byte only after the byte succeeds */if (ch >= 0){
          zs->check = pngparts_z_adler32_accum(zs->check, ch);
        } else if (ch == -1){
          /* ch is a repeat code; get the actual character */
          int const actual_ch = zs->inbuf[zs->inpos]&255;
          zs->check = pngparts_z_adler32_accum(zs->check, actual_ch);
        }
      } else if (sticky_finish) {
        /* notify the deflater of the finale */
        result = (*zs->cb.finish_cb)(
            zs->cb.cb_data, zs, &pngparts_zwrite_put_cb
          );
        if (result == PNGPARTS_API_DONE)
          result = PNGPARTS_API_OK;
        else if (result != PNGPARTS_API_OK)
          break;
        /* move to the finale */
        state = 3;
      }
      break;
    case 3:
      if (zs->shortpos == 0){
        unsigned long int const text_check =
          pngparts_z_adler32_tol(zs->check);
        /* generate the bytes for the checksum */{
          pngparts_zwrite_put32(zs->shortbuf, text_check);
        }
      }
      if (zs->shortpos < 4){
        skip_back = 1;
        result = pngparts_zwrite_put_cb(zs, zs->shortbuf[zs->shortpos]);
        if (result != PNGPARTS_API_OK)
          break;
        zs->shortpos += 1;
      }
      if (zs->shortpos >= 4){
        state = 4;
        zs->shortpos = 0;
      }
      break;
    case 4:
      {
        result = PNGPARTS_API_DONE;
      }break;
    default:
      result = PNGPARTS_API_BAD_STATE;
      break;
    }
    if (result != PNGPARTS_API_OK){
      break;
    } else if (skip_back) {
      trouble_counter += 1;
      if (trouble_counter > 16){
        result = PNGPARTS_API_LOOPED_STATE;
        break;
      }
    } else if (zs->flags_tf & 2){
      /* reset the flag */
      zs->flags_tf &= ~2;
      zs->inpos += 1;
    } else if (ch >= 0){
      zs->inpos += 1;
    }
  }
  zs->last_result = result;
  zs->state = (short)state;
  if (result){
    zs->flags_tf |= 2;
  }
  return result;
}

int pngparts_zwrite_put_cb(void* data, int ch){
  struct pngparts_z* zs = (struct pngparts_z*)data;
  if (zs->outpos < zs->outsize){
    unsigned char chc = (unsigned char)(ch&255);
    zs->outbuf[zs->outpos++] = chc;
    return PNGPARTS_API_OK;
  } else {
    return PNGPARTS_API_OVERFLOW;
  }
}

int pngparts_zwrite_set_dictionary
  (void* zs_v, unsigned char const* ptr, int len)
{
  struct pngparts_z *zs = (struct pngparts_z *)zs_v;
  if (zs->state == 0 && zs->shortpos == 0) {
    int i;
    struct pngparts_z_adler32 check;
    unsigned long int dict_chk;
    /* compute the dictionary checksum */
    check = pngparts_z_adler32_new();
    for (i = 0; i < len; ++i){
      check = pngparts_z_adler32_accum(check, ptr[i]&255);
    }
    dict_chk = pngparts_z_adler32_tol(check);
    /* write the checksum */{
      pngparts_zwrite_put32(zs->shortbuf+2, dict_chk);
    }
    /* update the header */{
      zs->header.fdict = 1;
      zs->header.fcheck = pngparts_z_header_check(zs->header);
    }
    /* start the flate callback */if ((zs->flags_tf&1) == 0){
      int flate_result;
      flate_result = (*zs->cb.start_cb)(
            zs->cb.cb_data, zs->header.fdict,
            zs->header.flevel, zs->header.cm, zs->header.cinfo
          );
      if (flate_result != PNGPARTS_API_OK){
        return flate_result;
      }
    }
    /* set the dictionary start flag */{
      zs->flags_tf |= 1;
    }
    /* apply the dictionary */ {
      int result = PNGPARTS_API_OK;
      if (zs->cb.dict_cb != NULL){
        for (i = 0; i < len && result == PNGPARTS_API_OK; ++i){
          result = (*zs->cb.dict_cb)(zs->cb.cb_data, ptr[i]&255);
        }
      }
      return result;
    }
  } else return PNGPARTS_API_BAD_STATE;
}
