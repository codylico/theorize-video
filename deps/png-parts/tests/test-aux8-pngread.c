/*
 * PNG-parts
 * parts of a Portable Network Graphics implementation
 * Copyright 2018-2019 Cody Licorish
 *
 * Licensed under the MIT License.
 *
 * test-pngread.c
 * png reader test program
 */

#include "../src/auxi.h"
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
static void test_image_recv_pixel
  ( void* img, long int x, long int y, unsigned int red,
    unsigned int green, unsigned int blue, unsigned int alpha);

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
void test_image_recv_pixel
  ( void* img_ptr, long int x, long int y, unsigned int red,
    unsigned int green, unsigned int blue, unsigned int alpha)
{
  struct test_image *img = (struct test_image*)img_ptr;
  unsigned char *const pixel = (&img->bytes[(y*img->width + x)*4]);
  /*fprintf(stderr, "pixel for %li %li..\n", x, y);*/
  pixel[0] = red;
  pixel[1] = green;
  pixel[2] = blue;
  pixel[3] = alpha;
  return;
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
  FILE *to_write = NULL;
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
      return 1;
    }
  }
  img.outfile = to_write;
  /* set image callback */{
    struct pngparts_api_image img_api;
    img_api.cb_data = &img;
    img_api.start_cb = &test_image_header;
    img_api.put_cb = &test_image_recv_pixel;
    /* parse the PNG stream */
    result = pngparts_aux_read_png_8(&img_api, in_fname);
  }
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
