/*
 * PNG-parts
 * parts of a Portable Network Graphics implementation
 * Copyright 2018-2019 Cody Licorish
 *
 * Licensed under the MIT License.
 *
 * test-aux-pngread.c
 * auxiliary png reader test program
 */

#include "../src/auxi.h"
#include "../src/png.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

struct test_image {
  int width;
  int height;
  unsigned char* bytes;
  FILE* outfile;
  FILE* alphafile;
};
static int test_image_header
  ( void* img, long int width, long int height, short bit_depth,
    short color_type, short compression, short filter, short interlace);

int test_image_header
  ( void* img_ptr, long int width, long int height, short bit_depth,
    short color_type, short compression, short filter, short interlace)
{
  struct test_image *img = (struct test_image*)img_ptr;
  void* bytes;
  fprintf(stderr, "{\"image info\":{\n"
    "  \"width\": %li,\n"
    "  \"height\": %li,\n"
    "  \"bit depth\": %i,\n"
    "  \"color type\": %i,\n"
    "  \"compression\": %i,\n"
    "  \"filter\": %i,\n"
    "  \"interlace\": %i\n}}\n",
    width, height, bit_depth, color_type, compression, filter, interlace
  );
  if (width > 10000 || height > 10000) return PNGPARTS_API_UNSUPPORTED;
  bytes = malloc(width*height * 4);
  if (bytes == NULL) return PNGPARTS_API_UNSUPPORTED;
  img->width = (int)width;
  img->height = (int)height;
  img->bytes = (unsigned char*)bytes;
  memset(bytes, 55, width*height * 4);
  return PNGPARTS_API_OK;
}
void test_image_put_ppm(struct test_image* img) {
  int x, y;
  fprintf(img->outfile, "P3\n%i %i\n255\n",
    img->width, img->height);
  for (y = 0; y < img->height; ++y) {
    for (x = 0; x < img->width; ++x) {
      unsigned char *const pixel = (&img->bytes[(y*img->width + x) * 4]);
      fprintf(img->outfile, "%i %i %i\n",
        pixel[0], pixel[1], pixel[2]);
    }
  }
}

void test_image_put_alphapgm(struct test_image* img) {
  int x, y;
  fprintf(img->alphafile, "P2\n%i %i\n255\n", img->width, img->height);
  for (y = 0; y < img->height; ++y) {
    for (x = 0; x < img->width; ++x) {
      unsigned char *const pixel = (&img->bytes[(y*img->width + x) * 4]);
      fprintf(img->alphafile, "%i\n", pixel[3]);
    }
  }
  return;
}


int main(int argc, char**argv) {
  FILE *to_read = NULL, *to_write = NULL;
  char const* in_fname = NULL, *out_fname = NULL;
  char const* alpha_fname = NULL;
  int help_tf = 0;
  int result = 0;
  struct test_image img = { 0,0,NULL,NULL,NULL };
  {
    int argi;
    for (argi = 1; argi < argc; ++argi) {
      if (strcmp(argv[argi], "-?") == 0) {
        help_tf = 1;
      } else if (strcmp("-a",argv[argi]) == 0){
        if (argi+1 < argc){
          argi += 1;
          alpha_fname = argv[argi];
        }
      } else if (in_fname == NULL) {
        in_fname = argv[argi];
      } else if (out_fname == NULL) {
        out_fname = argv[argi];
      }
    }
    if (help_tf) {
      fprintf(stderr,
        "usage: test_pngread [...options...] (infile) (outfile)\n"
        "  -                  stdin/stdout\n"
        "  -?                 help message\n"
        "options:\n"
        "  -a (file)          alpha channel output file\n"
      );
      return 2;
    }
  }
  /* open */
  if (out_fname == NULL) {
    fprintf(stderr, "No output file name given.\n");
    return 2;
  } else if (strcmp(out_fname, "-") == 0) {
    to_write = stdout;
  } else {
    to_write = fopen(out_fname, "wb");
    if (to_write == NULL) {
      int errval = errno;
      fprintf(stderr, "Failed to open '%s'.\n\t%s\n",
        out_fname, strerror(errval));
      if (to_read != stdin) fclose(to_read);
      return 1;
    }
  }

  /* read the header */{
    struct pngparts_png_header img_header;
    result = pngparts_aux_read_header(&img_header, in_fname);
    if (result == PNGPARTS_API_OK){
      result = test_image_header(
          &img, img_header.width, img_header.height,
          img_header.bit_depth, img_header.color_type, img_header.compression,
          img_header.filter, img_header.interlace
        );
    }
  }
  if (result == PNGPARTS_API_OK) {
    result = pngparts_aux_read_block(
        img.width, img.height, 0, 0, PNGPARTS_AUX_RGBA, 8, img.bytes, in_fname
      );
  }
  img.outfile = to_write;
  /* output to PPM */ {
    test_image_put_ppm(&img);
  }

  /* output the alpha channel */if (alpha_fname != NULL){
    FILE *alphafile;
    if (to_write != stdout){
      fclose(to_write);
      to_write = stdout;
    }
    alphafile = fopen(alpha_fname, "wt");
    if (alphafile == NULL){
      int errval = errno;
      fprintf(stderr, "Failed to open '%s' for alpha channel.\n\t%s\n",
        alpha_fname, strerror(errval));
      /* close */
      free(img.bytes);
      if (to_write != stdout) fclose(to_write);
      if (to_read != stdin) fclose(to_read);
      return 1;
    } else {
      img.alphafile = alphafile;
      test_image_put_alphapgm(&img);
      fclose(alphafile);
    }
  }

  /* close */
  free(img.bytes);
  if (to_write != stdout) fclose(to_write);
  fflush(NULL);
  if (result) {
    fprintf(stderr, "\nResult code %i: %s\n",
      result, pngparts_api_strerror(result));
  }
  return result;
}
