/*
 * PNG-parts
 * parts of a Portable Network Graphics implementation
 * Copyright 2018 Cody Licorish
 *
 * Licensed under the MIT License.
 *
 * test-png.c
 * png base test program
 */

#include "../src/png.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

static int sig_main(int argc, char **argv);
static int crc32_accum_main(int argc, char **argv);

int sig_main(int argc, char **argv){
  int help_tf = 0;
  int format = 0;
  int result;
  FILE *to_write = NULL;
  char const* fname = NULL;
  /* arguments */{
    int argi;
    for (argi = 1; argi < argc; ++argi){
      if (strcmp("-?",argv[argi]) == 0){
        help_tf = 1;
      } else if (strcmp("-c",argv[argi]) == 0){
        format = 1;
      } else {
        fname = argv[argi];
      }
    }
  }
  /*print help */if (help_tf){
    fprintf(stderr,"usage: test_png sig ... [file]\n"
      "  -                  write to stdout\n"
      "  -?                 help message\n"
      "  -c                 C format\n"
      );
    return 2;
  }
  if (fname == NULL
  ||  strcmp("-",fname) == 0)
  {
    to_write = stdout;
  } else {
    to_write = fopen(fname,"wb");
    if (to_write == NULL){
      perror("failed to open file");
      return 1;
    }
  }
  switch (format){
  case 0: /*direct*/
    {
      result = fwrite
        (pngparts_png_signature(),sizeof(unsigned char),8,to_write);
      if (result != 8){
        fprintf(stderr,"failed to write to file\n");
        result = 1;
      } else {
        result = 0;
      }
    }break;
  case 1:
    {
      int i;
      unsigned char sig[8];
      memcpy(sig,pngparts_png_signature(),8*sizeof(unsigned char));
      for (i = 0; i < 8; ++i)
        fprintf(to_write,"%s%#2x",(i>0?", ":""),sig[i]);
      fprintf(to_write,"\n");
      result = 0;
    }break;
  }
  if (to_write != stdout) fclose(to_write);
  return result;
}

int crc32_accum_main(int argc, char **argv){
  FILE* to_read = NULL;
  char const* fname = NULL;
  int argi;
  int help_tf = 0;
  long int count = LONG_MAX;
  long int byte_position = -1;
  int result = 0;
  struct pngparts_png_crc32 chk;
  for (argi = 1; argi < argc; ++argi){
    if (strcmp("-?",argv[argi]) == 0){
      help_tf = 1;
    } else {
      fname = argv[argi];
    }
  }
  /* print help */if (help_tf){
    fprintf(stderr,"usage: test_z crc32_accum ... [file]\n"
      "  -                  read from stdin\n"
      "  -?                 help message\n"
      "  -n (number)        byte position for reading\n"
      "  -c (number)        number of bytes to read\n"
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
    }
    /* read file */{
      int ch;
      chk = pngparts_png_crc32_new();
      while (count > 0){
        count -= 1;
        ch = fgetc(to_read);
        if (ch == EOF){
          break;
        }
        chk = pngparts_png_crc32_accum(chk,ch);
      }
    }
  } while(0);
  /* close file */
  if (to_read != stdin){
    fclose(to_read);
  }
  if (result == 0){
    fprintf(stdout,"%#lX\n",pngparts_png_crc32_tol(chk));
  }
  return result;
}

int main(int argc, char **argv){
  if (argc < 2){
    fprintf(stderr,"available commands: \n"
      "  sig            output the PNG signature\n"
      "  crc32_accum    compute PNG checksum for data\n"
      );
    return 2;
  } else if (strcmp("sig",argv[1]) == 0){
    return sig_main(argc-1,argv+1);
  } else if (strcmp("crc32_accum",argv[1]) == 0){
    return crc32_accum_main(argc-1,argv+1);
  } else {
    fprintf(stdout,"unknown command: %s\n", argv[1]);
    return 2;
  }
}
