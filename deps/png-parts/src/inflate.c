
#include "inflate.h"
#include <stdlib.h>

static int pngparts_inflate_bit
  (int b, struct pngparts_flate *fl, void* put_data, int(*put_cb)(void*,int));
static int pngparts_inflate_history_fetch
  (struct pngparts_flate *fl, void* put_data, int(*put_cb)(void*,int));
static int pngparts_inflate_advance_dynamic(struct pngparts_flate *fl);
static void pngparts_inflate_dynamic_set
  (struct pngparts_flate *fl, struct pngparts_flate_code code);

enum pngparts_inflate_last_block {
  PNGPARTS_INFLATE_STATE = 31,
  PNGPARTS_INFLATE_LAST = PNGPARTS_INFLATE_STATE+1
};

void pngparts_inflate_dynamic_set
  (struct pngparts_flate *fl, struct pngparts_flate_code code)
{
  if (fl->state & 1){
    pngparts_flate_huff_index_set(&fl->distance_table,fl->short_pos,code);
  } else {
    pngparts_flate_huff_index_set(&fl->length_table,fl->short_pos,code);
  }
}
int pngparts_inflate_advance_dynamic(struct pngparts_flate *fl){
  fl->short_pos += 1;
  if (fl->state & 1){
    /* distance */
    if (fl->short_pos == pngparts_flate_huff_get_size(&fl->distance_table)){
      int result = pngparts_flate_huff_generate(&fl->distance_table);
      if (result != PNGPARTS_API_OK) return result;
      pngparts_flate_huff_bit_sort(&fl->distance_table);
      fl->state = 6;
      fl->short_pos = 0;
    }
  } else {
    /* lengths */
    if (fl->short_pos == pngparts_flate_huff_get_size(&fl->length_table)){
      int result = pngparts_flate_huff_generate(&fl->length_table);
      if (result != PNGPARTS_API_OK) return result;
      pngparts_flate_huff_bit_sort(&fl->length_table);
      fl->state += 1;
      fl->short_pos = 0;
    }
  }
  return PNGPARTS_API_OK;
}
int pngparts_inflate_history_fetch
  (struct pngparts_flate *fl, void* put_data, int(*put_cb)(void*,int))
{
  while (fl->repeat_length > 0){
    int value = pngparts_flate_history_get(fl,fl->repeat_distance);
    int result = (*put_cb)(put_data,value);
    if (result != PNGPARTS_API_OK){
      return result;
    }
    pngparts_flate_history_add(fl,value);
    fl->repeat_length -= 1;
  }
  return PNGPARTS_API_OK;
}
void pngparts_inflate_init(struct pngparts_flate *fl){
  fl->bitpos = 0;
  fl->last_input_byte = 0;
  fl->history_bytes = NULL;
  fl->history_size = 0;
  fl->history_pos = 0;
  pngparts_flate_huff_init(&fl->code_table);
  pngparts_flate_huff_init(&fl->length_table);
  pngparts_flate_huff_init(&fl->distance_table);
  return;
}
void pngparts_inflate_free(struct pngparts_flate *fl){
  pngparts_flate_huff_free(&fl->distance_table);
  pngparts_flate_huff_free(&fl->length_table);
  pngparts_flate_huff_free(&fl->code_table);
  free(fl->history_bytes);
  fl->history_bytes = NULL;
  fl->history_size = 0;
  return;
}
void pngparts_inflate_assign_api
  (struct pngparts_api_flate *fcb, struct pngparts_flate *fl)
{
  fcb->cb_data = fl;
  fcb->start_cb = pngparts_inflate_start;
  fcb->dict_cb = pngparts_inflate_dict;
  fcb->one_cb = pngparts_inflate_one;
  fcb->finish_cb = pngparts_inflate_finish;
  return;
}

int pngparts_inflate_start
  ( void* data,
    short int fdict, short int flevel, short int cm, short int cinfo)
{
  struct pngparts_flate *fl = (struct pngparts_flate *)data;
  if (cm != 8) return PNGPARTS_API_UNSUPPORTED;
  if (cinfo > 7) return PNGPARTS_API_UNSUPPORTED;
  {
    unsigned int nsize = 1u<<(cinfo+8);
    unsigned char* ptr = (unsigned char*)malloc(nsize);
    if (ptr == NULL) return PNGPARTS_API_MEMORY;
    free(fl->history_bytes);
    fl->history_bytes = ptr;
    fl->history_size = (unsigned int)nsize;
    fl->history_pos = 0;
    fl->bitpos = 0;
    fl->last_input_byte = -1;
    fl->bitline = 0;
    fl->bitlength = 0;
    fl->state = 0;
  }
  return PNGPARTS_API_OK;
}
int pngparts_inflate_dict(void* data, int ch){
  struct pngparts_flate *fl = (struct pngparts_flate *)data;
  pngparts_flate_history_add(fl,ch);
  return PNGPARTS_API_OK;
}
int pngparts_inflate_bit
  (int b, struct pngparts_flate *fl, void* put_data, int(*put_cb)(void*,int))
{
  int result = PNGPARTS_API_OK;
  int bit = (b&255)?1:0;
  int repeat_tf = b&256;
  int state = fl->state&PNGPARTS_INFLATE_STATE;
  int last_block = fl->state&PNGPARTS_INFLATE_LAST;
  switch (state){
  case 0: /*header*/
    {
      if (fl->bitlength < 3){
        fl->bitline |= (bit<<fl->bitlength);
        fl->bitlength += 1;
      }
      if (fl->bitlength >= 3){
        /* final block? */
        if ((fl->bitline&1) == 1){
          last_block = PNGPARTS_INFLATE_LAST;
        }
        switch (fl->bitline&6){
        case 0: /* direct */
          {
            if (fl->bitpos == 7){
              state = 2;
              fl->short_pos = 0;
            } else state = 1;/* wait until end of block */
          }break;
        case 2: /* text Huffman */
          {
            /* set up the text Huffman */
            result = pngparts_flate_huff_resize(&fl->length_table,288);
            if (result != PNGPARTS_API_OK) break;
            result = pngparts_flate_huff_resize(&fl->distance_table,32);
            if (result != PNGPARTS_API_OK) break;
            pngparts_flate_fixed_lengths(&fl->length_table);
            pngparts_flate_fixed_distances(&fl->distance_table);
            pngparts_flate_huff_bit_sort(&fl->length_table);
            /* do length,distance reading */
            state = 6;
            fl->bitline = 0;
            fl->bitlength = 0;
          }break;
        case 4: /* custom codes */
          {
            /* do read code tree first */
            state = 11;
            fl->bitline = 0;
            fl->bitlength = 0;
          }break;
        default:
          state = 5;
          result = PNGPARTS_API_BAD_BLOCK;
          break;
        }
      }
    }break;
  case 1: /* direct patience */
    {
      /* do nothing */
      if (fl->bitpos == 7){
        state = 2;
        fl->short_pos = 0;
      }
    }break;
  case 2: /* direct length */
    break;
  case 3: /* direct characters */
    break;
  case 4:
    result = PNGPARTS_API_DONE;
    break;
  case 5: /* NOTE reserved: bad block */
    result = PNGPARTS_API_BAD_BLOCK;
    break;
  case 6: /* coded length */
    {
      int value;
      if (!repeat_tf){
        fl->bitline = (fl->bitline<<1)|bit;
        fl->bitlength += 1;
      }
      value = pngparts_flate_huff_bit_bsearch
        (&fl->length_table, fl->bitlength, fl->bitline);
      if (value >= 0){
        if (value == 256){/* stop code */
          state = 0;
          fl->bitlength = 0;
          fl->bitline = 0;
        } else if (value < 256){/* literal */
          result = (*put_cb)(put_data,value);
          if (result != PNGPARTS_API_OK){
            break;
          } else {
            pngparts_flate_history_add(fl,value);
          }
          fl->bitlength = 0;
          fl->bitline = 0;
        } else {/* do history stuff */
          struct pngparts_flate_extra extra;
          extra = pngparts_flate_length_decode(value);
          fl->repeat_length = extra.length_value;
          fl->bitline = 0;
          fl->short_pos = extra.extra_bits;
          fl->bitlength = 0;
          if (extra.extra_bits > 0){
            state = 7;
          } else state = 8;
        }
      } else if (value == PNGPARTS_API_NOT_FOUND){
        result = PNGPARTS_API_OK;
      } else {
        result = value;
      }
    }break;
  case 7: /* coded length extra bits */
    {
      if (!repeat_tf){
        if (fl->bitlength < fl->short_pos){
          fl->bitline |= (bit<<fl->bitlength);
          fl->bitlength += 1;
        }
      }
      if (fl->short_pos == fl->bitlength){
        fl->repeat_length += fl->bitline;
        fl->bitline = 0;
        fl->bitlength = 0;
        state = 8;
      }
    }break;
  case 8: /* coded distance */
    {
      int value;
      if (!repeat_tf){
        fl->bitline = (fl->bitline<<1)|bit;
        fl->bitlength += 1;
      }
      value = pngparts_flate_huff_bit_bsearch
        (&fl->distance_table, fl->bitlength, fl->bitline);
      if (value >= 0){
        struct pngparts_flate_extra extra;
        extra = pngparts_flate_distance_decode(value);
        fl->repeat_distance = extra.length_value;
        fl->bitline = 0;
        fl->short_pos = extra.extra_bits;
        fl->bitlength = 0;
        if (extra.extra_bits > 0){
          state = 9;
        } else state = 10;
        if (state == 10){
          result = pngparts_inflate_history_fetch(fl,put_data,put_cb);
          if (result == PNGPARTS_API_OK){
            state = 6;
          }
        }
      } else if (value == PNGPARTS_API_NOT_FOUND){
        result = PNGPARTS_API_OK;
      } else {
        result = value;
      }
    }break;
  case 9: /* distance extra */
    {
      if (!repeat_tf){
        if (fl->bitlength < fl->short_pos){
          fl->bitline |= (bit<<fl->bitlength);
          fl->bitlength += 1;
        }
      }
      if (fl->short_pos == fl->bitlength){
        fl->repeat_distance += fl->bitline;
        fl->bitline = 0;
        fl->bitlength = 0;
        state = 10;
        result = pngparts_inflate_history_fetch(fl,put_data,put_cb);
        if (result == PNGPARTS_API_OK){
          state = 6;
        }
      }
    }break;
  case 10: /* code history fetch */
    {
      result = pngparts_inflate_history_fetch(fl,put_data,put_cb);
      if (result == PNGPARTS_API_OK){
        state = 6;
        fl->bitlength = 0;
        fl->bitline = 0;
      }
    }break;
  case 11: /* microheader for dynamic block */
    {
      if (fl->bitlength < 14){
        fl->bitline |= (bit<<fl->bitlength);
        fl->bitlength += 1;
      }
      if (fl->bitlength >= 14){
        result = pngparts_flate_huff_resize
          (&fl->code_table,4+((fl->bitline>>10)&15));
        pngparts_flate_dynamic_codes(&fl->code_table);
        if (result != PNGPARTS_API_OK) break;
        result = pngparts_flate_huff_resize
          (&fl->distance_table,1+((fl->bitline>>5)&31));
        if (result != PNGPARTS_API_OK) break;
        result = pngparts_flate_huff_resize
          (&fl->length_table,257+((fl->bitline)&31));
        if (result != PNGPARTS_API_OK) break;
        state = 12;
        fl->short_pos = 0;
        fl->bitlength = 0;
        fl->bitline = 0;
      }
    }break;
  case 12: /* 3 bit integers */
    {
      if (fl->bitlength < 3){
        fl->bitline |= (bit<<fl->bitlength);
        fl->bitlength += 1;
      }
      if (fl->bitlength >= 3){
        struct pngparts_flate_code code =
          pngparts_flate_huff_index_get(&fl->code_table, fl->short_pos);
        code.length = fl->bitline;
        pngparts_flate_huff_index_set(&fl->code_table, fl->short_pos, code);
        fl->short_pos += 1;
        if (fl->short_pos == pngparts_flate_huff_get_size(&fl->code_table)){
          pngparts_flate_huff_value_sort(&fl->code_table);
          result = pngparts_flate_huff_generate(&fl->code_table);
          if (result != PNGPARTS_API_OK) break;
          pngparts_flate_huff_bit_sort(&fl->code_table);
          state = 14;
          fl->short_pos = 0;
          fl->shortbuf[0] = 0;
        }
        fl->bitline = 0;
        fl->bitlength = 0;
      }
    }break;
  case 14: /* length extraction */
  case 15: /* distance extraction */
    {
      int value;
      int reset_short_pos = fl->short_pos;
      if (!repeat_tf){
        fl->bitline = (fl->bitline<<1)|bit;
        fl->bitlength += 1;
      }
      value = pngparts_flate_huff_bit_bsearch
        (&fl->code_table, fl->bitlength, fl->bitline);
      if (value >= 0){
        if (value < 16){/* literal */
          struct pngparts_flate_code code;
          code.value = fl->short_pos;
          code.length = value;
          pngparts_inflate_dynamic_set(fl,code);
          result = pngparts_inflate_advance_dynamic(fl);
          if (result != PNGPARTS_API_OK){
            fl->short_pos = reset_short_pos;
            break;
          } else {
            state = fl->state;
          }
          fl->shortbuf[0] = (unsigned char)value;
          fl->bitlength = 0;
          fl->bitline = 0;
        } else if (value == 16){/* previous copy */
          state += 2;
          fl->bitlength = 0;
          fl->bitline = 0;
        } else if (value == 17){/* do zero stuff */
          state += 4;
          fl->bitlength = 0;
          fl->bitline = 0;
        } else if (value == 18){/* do zero stuff */
          state += 6;
          fl->bitlength = 0;
          fl->bitline = 0;
        }
      } else if (value == PNGPARTS_API_NOT_FOUND){
        result = PNGPARTS_API_OK;
      } else {
        result = value;
      }
    }break;
  case 16: /* lengths: repeat value */
  case 17: /* distances: repeat value */
    {
      int reset_short_pos = fl->short_pos;
      if (!repeat_tf){
        if (fl->bitlength < 2){
          fl->bitline |= (bit<<fl->bitlength);
          fl->bitlength += 1;
        }
      }
      if (fl->bitlength >= 2){
        int i;
        fl->bitline += 3;
        for (i = 0; i < fl->bitline; ++i){
          struct pngparts_flate_code code;
          code.value = fl->short_pos;
          code.length = fl->shortbuf[0];
          pngparts_inflate_dynamic_set(fl,code);
          result = pngparts_inflate_advance_dynamic(fl);
          if (result != PNGPARTS_API_OK){
            fl->short_pos = reset_short_pos;
            break;
          } else {
            state = fl->state;
          }
        }
        if (result != PNGPARTS_API_OK) break;
        /*fl->shortbuf[0] = (unsigned char)value;*/
        fl->bitline = 0;
        fl->bitlength = 0;
        if (state != 6) state -= 2;
      }
    }break;
  case 18: /* lengths: zero value 3 bit */
  case 19: /* distances: zero value 3 bit */
    {
      int reset_short_pos = fl->short_pos;
      if (!repeat_tf){
        if (fl->bitlength < 3){
          fl->bitline |= (bit<<fl->bitlength);
          fl->bitlength += 1;
        }
      }
      if (fl->bitlength >= 3){
        int i;
        fl->bitline += 3;
        for (i = 0; i < fl->bitline; ++i){
          struct pngparts_flate_code code;
          code.value = fl->short_pos;
          code.length = 0;
          pngparts_inflate_dynamic_set(fl,code);
          result = pngparts_inflate_advance_dynamic(fl);
          if (result != PNGPARTS_API_OK){
            fl->short_pos = reset_short_pos;
            break;
          } else {
            state = fl->state;
          }
        }
        if (result != PNGPARTS_API_OK) break;
        fl->shortbuf[0] = 0u;
        fl->bitline = 0;
        fl->bitlength = 0;
        if (state != 6) state -= 4;
      }
    }break;
  case 20: /* lengths: zero value 7 bit */
  case 21: /* distances: zero value 7 bit */
    {
      int reset_short_pos = fl->short_pos;
      if (!repeat_tf){
        if (fl->bitlength < 7){
          fl->bitline |= (bit<<fl->bitlength);
          fl->bitlength += 1;
        }
      }
      if (fl->bitlength >= 7){
        int i;
        fl->bitline += 11;
        for (i = 0; i < fl->bitline; ++i){
          struct pngparts_flate_code code;
          code.value = fl->short_pos;
          code.length = 0;
          pngparts_inflate_dynamic_set(fl,code);
          result = pngparts_inflate_advance_dynamic(fl);
          if (result != PNGPARTS_API_OK){
            fl->short_pos = reset_short_pos;
            break;
          } else {
            state = fl->state;
          }
        }
        if (result != PNGPARTS_API_OK) break;
        fl->shortbuf[0] = 0u;
        fl->bitline = 0;
        fl->bitlength = 0;
        if (state != 6) state -= 6;
      }
    }break;
  default:
    result = PNGPARTS_API_BAD_STATE;
    break;
  }
  if (state == 0 && last_block != 0){
    result = PNGPARTS_API_DONE;
    state = 4;
  }
  fl->state = state|last_block;
  return result;
}
int pngparts_inflate_one
  (void* data, int ch, void* put_data, int(*put_cb)(void*,int))
{
  struct pngparts_flate *fl = (struct pngparts_flate *)data;
  int res = PNGPARTS_API_OK;
  int state = fl->state&PNGPARTS_INFLATE_STATE;
  if (state >= 2 && state <= 3){
    int last_block = fl->state&PNGPARTS_INFLATE_LAST;
    if (ch < 0){
      ch = fl->last_input_byte;
    }
    /* direct bytes mode */
    switch (state){
    case 2: /* forward and reverse lengths */
      {
        if (fl->short_pos < 2){
          fl->shortbuf[fl->short_pos++] = ((unsigned char)(ch&255));
        } else if (fl->short_pos < 4){
          fl->shortbuf[fl->short_pos++] = ((unsigned char)((~ch)&255));
        }
        if (fl->short_pos >= 4){
          if (fl->shortbuf[0] != fl->shortbuf[2]
          ||  fl->shortbuf[1] != fl->shortbuf[3])
          {
            res = PNGPARTS_API_CORRUPT_LENGTH;
            break;
          } else {
            fl->block_length =
              fl->shortbuf[0]|((unsigned int)fl->shortbuf[1]<<8);
            if (fl->block_length > 0)
              state = 3;
            else if (last_block) {
              state = 4;
              res = PNGPARTS_API_DONE;
            } else {
              state = 0;
              fl->bitline = 0;
              fl->bitlength = 0;
            }
          }
        }
      }break;
    case 3:
      {
        if (fl->block_length > 0){
          res = (*put_cb)(put_data,ch);
          if (res != PNGPARTS_API_OK){
            break;
          } else {
            fl->block_length -= 1;
            pngparts_flate_history_add(fl,ch);
          }
        }
        if (fl->block_length == 0){
          if (last_block) {
            state = 4;
            res = PNGPARTS_API_DONE;
          } else {
            state = 0;
            fl->bitline = 0;
            fl->bitlength = 0;
          }
        }
      }break;
    }
    fl->state = state|last_block;
    if (res != PNGPARTS_API_OK) fl->last_input_byte = ch;
    return res;
  } else if (ch >= 0)
    fl->bitpos = 0;
  else {
    ch = fl->last_input_byte;
    res = pngparts_inflate_bit((ch&(1<<fl->bitpos))|256,fl,put_data,put_cb);
    if (res != 0) return res;
    else fl->bitpos += 1;
  }
  for (; fl->bitpos < 8; ++fl->bitpos){
    res = pngparts_inflate_bit(ch&(1<<fl->bitpos),fl,put_data,put_cb);
    if (res != 0) break;
  }
  if (fl->bitpos < 8){
    fl->last_input_byte = ch;
  }
  return res;
}
int pngparts_inflate_finish
  (void* data, void* put_data, int(*put_cb)(void*,int))
{
  struct pngparts_flate *fl = (struct pngparts_flate *)data;
  (void)put_data;
  (void)put_cb;
  if ((fl->state & PNGPARTS_INFLATE_STATE) != 4)
    return PNGPARTS_API_EOF;
  else
    return PNGPARTS_API_DONE;
}
