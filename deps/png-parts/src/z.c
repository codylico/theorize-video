/*
 * PNG-parts
 * parts of a Portable Network Graphics implementation
 * Copyright 2018 Cody Licorish
 *
 * Licensed under the MIT License.
 *
 * z.c
 * zlib main header
 */
#include "z.h"
#include <string.h>

int pngparts_z_header_check(struct pngparts_z_header hdr){
  int holding = 0;
  holding |= (hdr.fdict&1)<<5;
  holding |= (hdr.flevel&3)<<6;
  holding |= (hdr.cm&15)<<8;
  holding |= (hdr.cinfo&15)<<12;
  return 31-(holding%31);
}
struct pngparts_z_header pngparts_z_header_new(void){
  struct pngparts_z_header out;
  out.fdict = 0;
  out.flevel = 3;
  out.cm = 8;
  out.cinfo = 7;
  out.fcheck = 26;
  return out;
}
void pngparts_z_header_put(void* buf, struct pngparts_z_header  hdr){
  unsigned char* ptr = (unsigned char*)buf;
  ptr[0] = (hdr.cm&15)<<0
         | (hdr.cinfo&15)<<4;
  ptr[1] = (hdr.fcheck&31)<<0
         | (hdr.fdict&1)<<5
         | (hdr.flevel&3)<<6;
  return;
}
struct pngparts_z_header pngparts_z_header_get(void const* buf){
  unsigned char const* ptr = (void const*)buf;
  struct pngparts_z_header hdr;
  hdr.cm = (ptr[0]&15);
  hdr.cinfo = ((ptr[0]>>4)&15);
  hdr.fcheck = (ptr[1]&31);
  hdr.fdict = ((ptr[1]>>5)&1);
  hdr.flevel = ((ptr[1]>>6)&3);
  return hdr;
}

struct pngparts_z_adler32 pngparts_z_adler32_new(void){
  struct pngparts_z_adler32 out;
  out.s1 = 1;
  out.s2 = 0;
  return out;
}
unsigned long int pngparts_z_adler32_tol(struct pngparts_z_adler32 chk){
  return chk.s1 | ((chk.s2)<<16);
}
struct pngparts_z_adler32 pngparts_z_adler32_accum
  (struct pngparts_z_adler32 chk, int ch)
{
  unsigned long int xs1;
  struct pngparts_z_adler32 out;
  xs1 = (chk.s1+ch)%65521;
  out.s1 = xs1;
  out.s2 = (chk.s2+xs1)%65521;
  return out;
}

void pngparts_z_setup_input
  (void* zs, void* inbuf, int insize)
{
  struct pngparts_z *reader = (struct pngparts_z*)zs;
  reader->inbuf = (unsigned char*)inbuf;
  reader->inpos = 0;
  reader->insize = insize;
  return;
}
int pngparts_z_input_done(void const* zs){
  struct pngparts_z const*reader = (struct pngparts_z const*)zs;
  return reader->inpos >= reader->insize;
}
void pngparts_z_setup_output
  (void *zs, void* outbuf, int outsize)
{
  struct pngparts_z *reader = (struct pngparts_z*)zs;
  reader->outbuf = (unsigned char*)outbuf;
  reader->outpos = 0;
  reader->outsize = outsize;
  return;
}
int pngparts_z_output_left(void const* zs){
  struct pngparts_z const*reader = (struct pngparts_z const*)zs;
  return reader->outpos;
}
void pngparts_z_set_cb
  ( struct pngparts_z *reader, struct pngparts_api_flate const* cb)
{
  memcpy(&reader->cb,cb,sizeof(*cb));
  return;
}
