/*
 * PNG-parts
 * parts of a Portable Network Graphics implementation
 * Copyright 2018 Cody Licorish
 *
 * Licensed under the MIT License.
 *
 * test-z.c
 * zlib base test program
 */

#include "../src/z.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

static int fcheck_main(int argc, char **argv);
static int encode_header_main(int argc, char **argv);
static int decode_header_main(int argc, char **argv);
static int adler32_accum_main(int argc, char **argv);

int fcheck_main(int argc, char **argv){
  int argi;
  struct pngparts_z_header hdr;
  int check_compute;
  int help_tf = 0;
  hdr = pngparts_z_header_new();
  if (argc == 1){
    help_tf = 1;
  } else for (argi = 1; argi < argc; ++argi){
    if (strcmp("-cinfo",argv[argi]) == 0){
      if (++argi < argc){
        hdr.cinfo = atoi(argv[argi]);
      }
    } else if (strcmp("-cm",argv[argi]) == 0){
      if (++argi < argc){
        hdr.cm = atoi(argv[argi]);
      }
    } else if (strcmp("-fcheck",argv[argi]) == 0){
      if (++argi < argc){
        hdr.fcheck = atoi(argv[argi]);
      }
    } else if (strcmp("-fdict",argv[argi]) == 0){
      if (++argi < argc){
        hdr.fdict = atoi(argv[argi]);
      }
    } else if (strcmp("-flevel",argv[argi]) == 0){
      if (++argi < argc){
        hdr.flevel = atoi(argv[argi]);
      }
    } else if (strcmp("-",argv[argi]) == 0){
      /* ignore */
    } else if (strcmp("-?",argv[argi]) == 0){
      help_tf = 1;
    } else {
      fprintf(stderr,"unrecognized option: %s\n",argv[argi]);
      help_tf = 1;
    }
  }
  /*print help */if (help_tf){
    fprintf(stderr,"usage: test_z header_check ...\n"
      "  -                  (ignored)\n"
      "  -?                 help message\n"
      "  -cm (number)       set compression method\n"
      "  -cinfo (number)    set compression information\n"
      "  -fcheck (number)   set check value\n"
      "  -fdict (number)    set dictionary flag\n"
      "  -flevel (number)   set compression level\n"
      );
    return 2;
  }
  /* compute the real check value */
  check_compute = pngparts_z_header_check(hdr);
  if (check_compute%31 == hdr.fcheck%31){
    fprintf(stdout,"check ok\n");
    return 0;
  } else {
    fprintf(stdout,"check bad, expected fcheck %i\n",check_compute);
    return 1;
  }
}

int encode_header_main(int argc, char **argv){
  char const* fname = NULL;
  int help_tf = 0;
  unsigned char buf[2];
  int argi;
  long byte_position = -1;
  struct pngparts_z_header hdr;
  hdr = pngparts_z_header_new();
  for (argi = 1; argi < argc; ++argi){
    if (strcmp("-?",argv[argi]) == 0){
      help_tf = 1;
    } else if (strcmp("-n",argv[argi]) == 0){
      if (++argi < argc){
        byte_position = strtol(argv[argi],NULL,0);
      }
    } else if (strcmp("-cinfo",argv[argi]) == 0){
      if (++argi < argc){
        hdr.cinfo = atoi(argv[argi]);
      }
    } else if (strcmp("-cm",argv[argi]) == 0){
      if (++argi < argc){
        hdr.cm = atoi(argv[argi]);
      }
    } else if (strcmp("-fcheck",argv[argi]) == 0){
      if (++argi < argc){
        if (strcmp(argv[argi],".") == 0)
          hdr.fcheck = -1;
        else
          hdr.fcheck = atoi(argv[argi]);
      }
    } else if (strcmp("-fdict",argv[argi]) == 0){
      if (++argi < argc){
        hdr.fdict = atoi(argv[argi]);
      }
    } else if (strcmp("-flevel",argv[argi]) == 0){
      if (++argi < argc){
        hdr.flevel = atoi(argv[argi]);
      }
    } else {
      fname = argv[argi];
    }
  }
  /*print help */if (help_tf){
    fprintf(stderr,"usage: test_z header_put ... [file]\n"
      "  -                  write to stdout\n"
      "  -?                 help message\n"
      "  -n (number)        byte position for writing\n"
      "  -cm (number)       set compression method\n"
      "  -cinfo (number)    set compression information\n"
      "  -fcheck (number)   set check value\n"
      "  -fcheck .          set correct check value\n"
      "  -fdict (number)    set dictionary flag\n"
      "  -flevel (number)   set compression level\n"
      );
    return 2;
  }
  /* check the header */
  if (hdr.fcheck == -1){
    hdr.fcheck = pngparts_z_header_check(hdr);
  }
  /* encode the header */
  pngparts_z_header_put(buf,hdr);
  /* write the file */
  if (fname == NULL){
    fprintf(stderr,"No filename given.\n");
    return 2;
  } else if (strcmp("-",fname) == 0){
    size_t n;
    n = fwrite(buf,sizeof(unsigned char),2,stdout);
    if (n < 2){
      fprintf(stderr,"Failed to write two bytes to stdout.\n");
      return 1;
    }
  } else {
    FILE *to_write;
    to_write = fopen(fname,byte_position>=0?"wb+":"wb");
    if (to_write == NULL){
      int errval = errno;
      fprintf(stderr,"Failed to open '%s'.\n\t%s\n",
        fname, strerror(errval));
      return 1;
    } else {
      size_t n;
      int result = 0;
      do {
        if (byte_position > 0){
          int seek_response = fseek(to_write,byte_position,SEEK_SET);
          if (seek_response != 0){
            int errval = errno;
            fprintf(stderr,"Failed to seek '%s' to %li.\n\t%s\n",
              fname, byte_position, strerror(errval));
            result = 1;break;
          }
        }
        n = fwrite(buf,sizeof(unsigned char),2,to_write);
        if (n < 2){
          fprintf(stderr,"Failed to write two bytes to '%s'.\n",
            fname);
          result = 1;break;
        }
      } while (0);
      fclose(to_write);
      if (result) return result;
    }
  }
  return 0;
}

int decode_header_main(int argc, char **argv){
  char const* fname = NULL;
  int help_tf = 0;
  unsigned char buf[2];
  int argi;
  long byte_position = 0;
  int check_ok = 0;
  int check_compute;
  struct pngparts_z_header hdr;
  for (argi = 1; argi < argc; ++argi){
    if (strcmp("-?",argv[argi]) == 0){
      help_tf = 1;
    } else if (strcmp("-n",argv[argi]) == 0){
      if (++argi < argc){
        byte_position = strtol(argv[argi],NULL,0);
      }
    } else {
      fname = argv[argi];
    }
  }
  /*print help */if (help_tf){
    fprintf(stderr,"usage: test_z header_get ... [file]\n"
      "  -                  read from stdin\n"
      "  -?                 help message\n"
      "  -n (number)        byte position for reading\n"
      );
    return 2;
  }
  /* read the file */
  if (fname == NULL){
    fprintf(stderr,"No filename given.\n");
    return 2;
  } else if (strcmp("-",fname) == 0){
    size_t n;
    n = fread(buf,sizeof(unsigned char),2,stdin);
    if (n < 2){
      fprintf(stderr,"Failed to read two bytes from stdin.\n");
      return 2;
    }
  } else {
    FILE *to_read;
    to_read = fopen(fname,"rb");
    if (to_read == NULL){
      int errval = errno;
      fprintf(stderr,"Failed to open '%s'.\n\t%s\n",
        fname, strerror(errval));
      return 2;
    } else {
      size_t n;
      int result = 0;
      do {
        if (byte_position > 0){
          int seek_response = fseek(to_read,byte_position,SEEK_SET);
          if (seek_response != 0){
            int errval = errno;
            fprintf(stderr,"Failed to seek '%s' to %li.\n\t%s\n",
              fname, byte_position, strerror(errval));
            result = 2;break;
          }
        }
        n = fread(buf,sizeof(unsigned char),2,to_read);
        if (n < 2){
          fprintf(stderr,"Failed to read two bytes from '%s'.\n",
            fname);
          result = 2;break;
        }
      } while (0);
      fclose(to_read);
      if (result) return result;
    }
  }
  /* decode the header */
  hdr = pngparts_z_header_get(buf);
  /* check the header */
  check_compute = pngparts_z_header_check(hdr);
  check_ok = (check_compute%31 == hdr.fcheck%31);
  if (check_ok){
    fprintf(stderr,"check ok\n");
  } else {
    fprintf(stderr,"check bad, expected fcheck %i\n",check_compute);
  }
  /*print the header */
  fprintf(stdout,"-fcheck %i\n-fdict %i\n-flevel %i\n-cm %i\n-cinfo %i\n",
      hdr.fcheck,hdr.fdict,hdr.flevel,hdr.cm,hdr.cinfo);
  return check_ok?0:1;
}
int adler32_accum_main(int argc, char **argv){
  FILE* to_read = NULL;
  char const* fname = NULL;
  int argi;
  int help_tf = 0;
  long int count = LONG_MAX;
  long int byte_position = -1;
  int result = 0;
  struct pngparts_z_adler32 chk;
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
    } else {
      fname = argv[argi];
    }
  }
  /* print help */if (help_tf){
    fprintf(stderr,"usage: test_z adler32_accum ... [file]\n"
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
      chk = pngparts_z_adler32_new();
      while (count > 0){
        count -= 1;
        ch = fgetc(to_read);
        if (ch == EOF){
          break;
        }
        chk = pngparts_z_adler32_accum(chk,ch);
      }
    }
  } while(0);
  /* close file */
  if (to_read != stdin){
    fclose(to_read);
  }
  if (result == 0){
    fprintf(stdout,"%#lX\n",pngparts_z_adler32_tol(chk));
  }
  return result;
}

int main(int argc, char **argv){
  if (argc < 2){
    fprintf(stderr,"available commands: \n"
      "  header_check   check a zlib header\n"
      "  header_put     write a zlib header\n"
      "  header_get     read and check a zlib header\n"
      "  adler32_accum  compute Adler32 checksum for data\n"
      );
    return 2;
  } else if (strcmp("header_check",argv[1]) == 0){
    return fcheck_main(argc-1,argv+1);
  } else if (strcmp("header_put",argv[1]) == 0){
    return encode_header_main(argc-1,argv+1);
  } else if (strcmp("header_get",argv[1]) == 0){
    return decode_header_main(argc-1,argv+1);
  } else if (strcmp("adler32_accum",argv[1]) == 0){
    return adler32_accum_main(argc-1,argv+1);
  } else {
    fprintf(stdout,"unknown command: %s\n", argv[1]);
    return 2;
  }
}
