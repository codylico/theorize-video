/*
 * PNG-parts
 * parts of a Portable Network Graphics implementation
 * Copyright 2018 Cody Licorish
 *
 * Licensed under the MIT License.
 *
 * test-zread.c
 * zlib reader test program
 */

#include "../src/zread.h"
#include "../src/inflate.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

int main(int argc, char**argv){
  FILE *to_read = NULL, *to_write = NULL;
  char const* in_fname = NULL, *out_fname = NULL,
    *dict_fname = NULL;
  struct pngparts_z reader;
  struct pngparts_flate inflater;
  int help_tf = 0;
  int result = 0;
  {
    int argi;
    for (argi = 1; argi < argc; ++argi){
      if (strcmp(argv[argi],"-?") == 0){
        help_tf = 1;
      } else if (strcmp(argv[argi],"-d") == 0){
        if (++argi < argc){
          dict_fname = argv[argi];
        }
      } else if (in_fname == NULL){
        in_fname = argv[argi];
      } else if (out_fname == NULL){
        out_fname = argv[argi];
      }
    }
    if (help_tf){
      fprintf(stderr,"usage: test_zread ... (infile) (outfile)\n"
        "  -                  stdin/stdout\n"
        "  -?                 help message\n"
        "  -d (file)          dictionary\n"
        );
      return 2;
    }
  }
  /* open */
  if (in_fname == NULL){
    fprintf(stderr,"No input file name given.\n");
    return 2;
  } else if (strcmp(in_fname,"-") == 0){
    to_read = stdin;
  } else {
    to_read = fopen(in_fname,"rb");
    if (to_read == NULL){
      int errval = errno;
      fprintf(stderr,"Failed to open '%s'.\n\t%s\n",
        in_fname, strerror(errval));
      return 1;
    }
  }
  if (out_fname == NULL){
    fprintf(stderr,"No output file name given.\n");
    return 2;
  } else if (strcmp(out_fname,"-") == 0){
    to_write = stdout;
  } else {
    to_write = fopen(out_fname,"wb");
    if (to_write == NULL){
      int errval = errno;
      fprintf(stderr,"Failed to open '%s'.\n\t%s\n",
        out_fname, strerror(errval));
      if (to_read != stdin) fclose(to_read);
      return 1;
    }
  }
  /* parse the zlib stream */
  pngparts_zread_init(&reader);
  pngparts_inflate_init(&inflater);
  {
    struct pngparts_api_flate fcb;
    pngparts_inflate_assign_api(&fcb, &inflater);
    pngparts_z_set_cb( &reader, &fcb);
  }
  do {
    unsigned char inbuf[256];
    unsigned char outbuf[128];
    size_t readlen;
    while ((readlen = fread(inbuf,sizeof(unsigned char),256,to_read)) > 0){
      pngparts_z_setup_input(&reader,inbuf,readlen);
      pngparts_z_setup_output(&reader,outbuf,sizeof(outbuf));
      while (!pngparts_z_input_done(&reader)){
        size_t writelen;
        result = pngparts_zread_parse(&reader,PNGPARTS_API_Z_NORMAL);
        if (result == PNGPARTS_API_NEED_DICT
        &&  dict_fname != NULL)
        {
          FILE* to_adjust = fopen(dict_fname,"rb");
          long int dict_length;
          unsigned char *dict_data = NULL;
          if (to_adjust == NULL){
            int errval = errno;
            fprintf(stderr,"Failed to open dictionary '%s'.\n\t%s\n",
              dict_fname, strerror(errval));
            result = PNGPARTS_API_IO_ERROR;
            break;
          }
          do {
            /* get dictionary length */
            result = fseek(to_adjust,0,SEEK_END);
            if (result != 0){
              int errval = errno;
              fprintf(stderr,"Failed to seek in dictionary.\n\t%s\n",
                strerror(errval));
              result = PNGPARTS_API_IO_ERROR;
              break;
            }
            dict_length = ftell(to_adjust);
            if (dict_length < 0){
              int errval = errno;
              fprintf(stderr,"Dictionary size unavailable.\n\t%s\n",
                strerror(errval));
              result = PNGPARTS_API_IO_ERROR;
              break;
            } else if (dict_length >= INT_MAX){
              fprintf(stderr,"Dictionary size too big.\n\t%li > INT_MAX\n",
                dict_length);
              result = PNGPARTS_API_IO_ERROR;
              break;
            }
            fseek(to_adjust,0,SEEK_SET);
            dict_data = malloc(dict_length);
            if (dict_data == NULL){
              int errval = errno;
              fprintf(stderr,"Dictionary memory could not be allocated.\n"
                "\t%s\n",strerror(errval));
              result = PNGPARTS_API_IO_ERROR;
              break;
            }
            /* read the dictionary */
            if (fread(dict_data,sizeof(unsigned char),dict_length,to_adjust)
                != dict_length)
            {
              fprintf(stderr,"Dictionary read failed.\n");
              result = PNGPARTS_API_IO_ERROR;
              break;
            }
            /* put the dictionary */
            result = pngparts_zread_set_dictionary
              (&reader, dict_data, dict_length);
          } while (0);
          free(dict_data);
          fclose(to_adjust);
          if (result != PNGPARTS_API_OK) break;
          else continue;
        } else if (result < 0) break;
        writelen = pngparts_z_output_left(&reader);
        if (writelen > 0){
          size_t writeresult =
            fwrite(outbuf,sizeof(unsigned char),writelen,to_write);
          if (writeresult != writelen){
            result = PNGPARTS_API_IO_ERROR;
            break;
          }
          pngparts_z_setup_output(&reader,outbuf,sizeof(outbuf));
        }
      }
      if (result < 0) break;
    }
    if (result < 0) break;
  } while (0);
  if (result >= 0){
    result = pngparts_zread_parse(&reader,PNGPARTS_API_Z_FINISH);
  }
  pngparts_inflate_free(&inflater);
  pngparts_zread_free(&reader);
  /* close */
  if (to_write != stdout) fclose(to_write);
  if (to_read != stdin) fclose(to_read);
  fflush(NULL);
  if (result){
    fprintf(stderr,"\nResult code %i: %s\n",
      result,pngparts_api_strerror(result));
  }
  return result;
}
