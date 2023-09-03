/*
 * PNG-parts
 * parts of a Portable Network Graphics implementation
 * Copyright 2018 Cody Licorish
 *
 * Licensed under the MIT License.
 *
 * test-hash.c
 * Hash table test program
 */

#include "../src/flate.h"
#include "../src/api.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>

int main(int argc, char **argv){
  FILE* to_read = NULL;
  char const* fname = NULL;
  char const* test_triple = NULL;
  int argi;
  int help_tf = 0;
  long int count = LONG_MAX;
  long int byte_position = -1;
  long int last_byte_position;
  int result = 0;
  unsigned int start_distance = 0;
  struct pngparts_flate_hash hasher;
  unsigned char ring_buffer[1024];
  unsigned int const ring_size = sizeof(ring_buffer);
  for (argi = 1; argi < argc; ++argi){
    if (strcmp("-?",argv[argi]) == 0){
      help_tf = 1;
    } else if (strcmp("-n",argv[argi]) == 0){
      if (++argi < argc){
        byte_position = strtol(argv[argi], NULL, 0);
      }
    } else if (strcmp("-c",argv[argi]) == 0){
      if (++argi < argc){
        count = strtol(argv[argi], NULL, 0);
      }
    } else if (strcmp("-s",argv[argi]) == 0){
      if (++argi < argc){
        start_distance = (unsigned int)strtol(argv[argi], NULL, 0);
      }
    } else if (test_triple == NULL){
      test_triple = argv[argi];
    } else {
      fname = argv[argi];
    }
  }
  if (test_triple == NULL){
    fprintf(stderr,"Missing triple for which to search!\n");
    help_tf = 1;
  }
  /* print help */if (help_tf){
    fprintf(stderr,"usage: test_hash ... (triple) [file]\n"
      "  -                  read from stdin\n"
      "  -?                 help message\n"
      "  -n (number)        byte position for reading\n"
      "  -c (number)        number of bytes to read\n"
      "  -s (number)        distance value of previous match at\n"
      "                       which to start\n"
      );
    return 2;
  }
  /* get file */
  if (fname == NULL){
    fprintf(stderr,"No filename given.\n");
    return 2;
  } else if (strcmp(fname,"-") == 0){
    to_read = stdin;
  } else {
    to_read = fopen(fname,"rb");
    if (to_read == NULL){
      int errval = errno;
      fprintf(stderr,"Failed to open '%s'.\n\t%s\n",
        fname, strerror(errval));
      return 2;
    }
  }
  pngparts_flate_hash_init(&hasher);
  /* prepare the hash table */{
    int hash_prepare_result =
      pngparts_flate_hash_prepare(&hasher, ring_size);
    if (hash_prepare_result < 0){
      fprintf(stderr,"Could not prepare hash:\n\t%s\n",
          pngparts_api_strerror(hash_prepare_result));
      return hash_prepare_result;
    }
  }
  do {
    /* seek file */if (byte_position >= 0
    &&  to_read != stdin)
    {
      int seek_response = fseek(to_read,byte_position,SEEK_SET);
      if (seek_response != 0){
        int errval = errno;
        fprintf(stderr,"Failed to seek '%s' to %li.\n\t%s\n",
          fname, byte_position, strerror(errval));
        result = 2;break;
      }
      last_byte_position = byte_position;
    } else last_byte_position = 0;
    /* read file */{
      int ch;
      unsigned int ring_pos = 0;
      while (count > 0){
        count -= 1;
        ch = fgetc(to_read);
        if (ch == EOF){
          break;
        }
        ring_buffer[ring_pos] = ch;
        pngparts_flate_hash_add(&hasher,ch);
        ring_pos = (ring_pos+1)%ring_size;
        last_byte_position += 1;
      }
    }
  } while(0);
  /* close file */
  if (to_read != stdin){
    fclose(to_read);
  }
  /* look for the triple */if (result == 0){
    unsigned char search_text[3];
    /* copy the search text */{
      int i;
      for (i = 0; i < 3 && test_triple[i] != 0; ++i){
        search_text[i] = (unsigned char)test_triple[i];
      }
      for (; i < 3; ++i){
        search_text[i] = 0;
      }
    }
    /* search */{
      unsigned int const distance = pngparts_flate_hash_check
        (&hasher, ring_buffer, search_text, start_distance);
      fprintf(stdout,"%u\n",distance);
      if (distance > 0){
        fprintf(stderr,"(Last match found at location %#lX.)\n",
            last_byte_position-distance);
      } else {
        fprintf(stderr,"(No match found.)\n");
      }
    }
  }
  pngparts_flate_hash_free(&hasher);
  return result;
}
