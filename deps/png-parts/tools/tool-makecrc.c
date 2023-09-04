/*
 * PNG-parts
 * parts of a Portable Network Graphics implementation
 * Copyright 2018 Cody Licorish
 *
 * Licensed under the MIT License.
 *
 * tool-makecrc.c
 * compute the CRC32 acceleration table
 */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){
  int i;
  for (i = 0; i < 256; ++i){
    int j;
    unsigned long int value = i;
    /* long division */
    for (j = 0; j < 8; ++j){
      if (value & 1){
        value = (value>>1) ^ (0xEDB88320L);
      } else {
        value >>= 1;
      }
    }
    fprintf(stdout,"0x%08lX,\n",value);
  }
  return 0;
}
