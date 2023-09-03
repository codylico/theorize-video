/*
 * PNG-parts
 * parts of a Portable Network Graphics implementation
 * Copyright 2018-2019 Cody Licorish
 *
 * Licensed under the MIT License.
 *
 * test-aux-pngwrite.c
 * auxiliary png writer test program
 */

#include "../src/auxi.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

struct test_image {
  int width;
  int height;
  unsigned char* bytes;
  FILE* fptr;
  char interlace_tf;
  char color_type;
  char bit_depth;
  FILE* alpha_fptr;
};
static int test_image_resize
  ( void* img_ptr, long int width, long int height);
static int test_image_get_text_word
  (struct test_image* img, char* buf, int count);
static int test_file_get_text_word(FILE* img, char* buf, int count);
static int test_image_get_ppm(struct test_image* img);
static int test_image_get_alphapgm(struct test_image* img);


int test_image_resize
  (void* img_ptr, long int width, long int height)
{
  struct test_image *img = (struct test_image*)img_ptr;
  void* bytes;
  if (width > 10000 || height > 10000) return PNGPARTS_API_UNSUPPORTED;
  bytes = malloc(width*height * 4);
  if (bytes == NULL) return PNGPARTS_API_UNSUPPORTED;
  img->width = (int)width;
  img->height = (int)height;
  img->bytes = (unsigned char*)bytes;
  memset(bytes, 55, width*height * 4);
  return PNGPARTS_API_OK;
}


int test_image_get_text_word(struct test_image* img, char* buf, int count){
  return test_file_get_text_word(img->fptr, buf, count);
}
int test_file_get_text_word(FILE* fptr, char* buf, int count){
  int ch;
  int i = 0;
  int const countm1 = count-1;
  /* scan past spaces */
  ch = fgetc(fptr);
  while (ch != EOF && isspace(ch)){
    ch = fgetc(fptr);
  }
  /* read the word */
  if (ch == EOF || countm1 <= 0){
    return 0;
  } else while (ch != EOF && (!isspace(ch))){
    buf[i] = (char)ch;
    i += 1;
    if (i >= countm1)
      break;
    ch = fgetc(fptr);
  }
  /* NUL-terminate it */
  if (i >= countm1)
    buf[countm1] = 0;
  else
    buf[i] = 0;
  return i;
}

int test_image_get_ppm(struct test_image* img) {
  int x, y;
  char word_buf[32];
  int pixel_size;
  /* determine pixel size */
  switch (img->color_type&6u) {
    case PNGPARTS_AUX_L: pixel_size = 1; break;
    case PNGPARTS_AUX_RGB: pixel_size = 3; break;
    case PNGPARTS_AUX_LA: pixel_size = 2; break;
    case PNGPARTS_AUX_RGBA: pixel_size = 4; break;
  }
  if (test_image_get_text_word(img, word_buf, sizeof(word_buf)) <= 0){
    return PNGPARTS_API_IO_ERROR;
  } else if (strcmp("P3", word_buf) == 0){
    int width, height, max_value;
    /* read width */
    if (test_image_get_text_word(img, word_buf, sizeof(word_buf)) <= 0){
      return PNGPARTS_API_IO_ERROR;
    }
    width = atoi(word_buf);
    /* read height */
    if (test_image_get_text_word(img, word_buf, sizeof(word_buf)) <= 0){
      return PNGPARTS_API_IO_ERROR;
    }
    height = atoi(word_buf);
    /* read value maximum */
    if (test_image_get_text_word(img, word_buf, sizeof(word_buf)) <= 0){
      return PNGPARTS_API_IO_ERROR;
    }
    max_value = atoi(word_buf);
    /* allocate the space */{
      int const resize_result = test_image_resize(img, width, height);
      if (resize_result != PNGPARTS_API_OK){
        return resize_result;
      }
    }
    for (y = 0; y < height; ++y) {
      for (x = 0; x < width; ++x) {
        unsigned char *const pixel =
          (&img->bytes[(y*img->width + x) * pixel_size]);
        int red, green, blue, alpha = max_value;
        /* read */
        if (test_image_get_text_word(img, word_buf, sizeof(word_buf)) <= 0){
          return PNGPARTS_API_IO_ERROR;
        }
        red = atoi(word_buf);
        if (test_image_get_text_word(img, word_buf, sizeof(word_buf)) <= 0){
          return PNGPARTS_API_IO_ERROR;
        }
        green = atoi(word_buf);
        if (test_image_get_text_word(img, word_buf, sizeof(word_buf)) <= 0){
          return PNGPARTS_API_IO_ERROR;
        }
        blue = atoi(word_buf);
        /* put */switch (pixel_size) {
        case 2:
          pixel[1] = alpha*255/max_value;
        case 1:
          pixel[0] = red*255/max_value;
          break;
        case 4:
          pixel[3] = alpha*255/max_value;
          /*[[fallthrough]]*/;
        case 3:
          pixel[0] = red*255/max_value;
          pixel[1] = green*255/max_value;
          pixel[2] = blue*255/max_value;
          break;
        }
      }
    }
  } else {
    return PNGPARTS_API_UNSUPPORTED;
  }
  return PNGPARTS_API_OK;
}

int test_image_get_alphapgm(struct test_image* img) {
  int x, y;
  char word_buf[32];
  int pixel_size;
  int alpha_index;
  /* determine pixel size */
  switch (img->color_type) {
    case PNGPARTS_AUX_L:
    case PNGPARTS_AUX_RGB:
      fprintf(stderr, "Ignoring alpha channel image for color type %i.\n",
          img->color_type);
      /* alpha unused, so */return PNGPARTS_API_OK;
    case PNGPARTS_AUX_LA: pixel_size = 2;alpha_index = 1; break;
    case PNGPARTS_AUX_RGBA: pixel_size = 4;alpha_index = 3; break;
  }
  if (test_file_get_text_word
      (img->alpha_fptr, word_buf, sizeof(word_buf)) <= 0)
  {
    return PNGPARTS_API_IO_ERROR;
  } else if (strcmp("P2", word_buf) == 0){
    int width, height, max_value;
    /* read width */
    if (test_file_get_text_word
        (img->alpha_fptr, word_buf, sizeof(word_buf)) <= 0)
    {
      return PNGPARTS_API_IO_ERROR;
    }
    width = atoi(word_buf);
    /* read height */
    if (test_file_get_text_word
        (img->alpha_fptr, word_buf, sizeof(word_buf)) <= 0)
    {
      return PNGPARTS_API_IO_ERROR;
    }
    height = atoi(word_buf);
    /* read value maximum */
    if (test_file_get_text_word
        (img->alpha_fptr, word_buf, sizeof(word_buf)) <= 0)
    {
      return PNGPARTS_API_IO_ERROR;
    }
    max_value = atoi(word_buf);
    /* check the space */{
      if (width != img->width || height != img->height){
        fprintf(stderr, "Alpha channel image is wrong size.\n");
        return PNGPARTS_API_UNSUPPORTED;
      }
    }
    for (y = 0; y < height; ++y) {
      for (x = 0; x < width; ++x) {
        unsigned char *const pixel =
          (&img->bytes[(y*img->width + x) * pixel_size]);
        int alpha;
        /* read */
        if (test_file_get_text_word
            (img->alpha_fptr, word_buf, sizeof(word_buf)) <= 0)
        {
          return PNGPARTS_API_IO_ERROR;
        }
        alpha = atoi(word_buf);
        /* put */
        pixel[alpha_index] = alpha*255/max_value;
      }
    }
  } else {
    return PNGPARTS_API_UNSUPPORTED;
  }
  return PNGPARTS_API_OK;
}


int main(int argc, char**argv) {
  FILE *to_read = NULL, *to_write = NULL;
  char const* in_fname = NULL, *out_fname = NULL;
  char const* alpha_fname = NULL;
  int help_tf = 0;
  int result = 0;
  struct test_image img = { 0,0,NULL,NULL,0,2,8,NULL };
  {
    int argi;
    for (argi = 1; argi < argc; ++argi) {
      if (strcmp(argv[argi], "-?") == 0) {
        help_tf = 1;
      } else if (strcmp(argv[argi], "-b") == 0) {
        if (argi + 1 < argc){
          argi += 1;
          img.bit_depth = atoi(argv[argi]);
        }
      } else if (strcmp(argv[argi], "-c") == 0) {
        if (argi + 1 < argc){
          argi += 1;
          img.color_type = ((unsigned int)atoi(argv[argi])&6u);
        }
      } else if (strcmp(argv[argi], "-i") == 0) {
        img.interlace_tf = 1;
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
        "usage: test_pngwrite [...options...] (infile) (outfile)\n"
        "  -                  stdin/stdout\n"
        "  -?                 help message\n"
        "options:\n"
        "  -i                 enable interlacing\n"
        "  -c (type)          set color type\n"
        "  -b (depth)         set sample bit depth\n"
        "  -a (file)          read alpha channel file\n"
      );
      return 2;
    }
  }
  /* open */
  if (in_fname == NULL) {
    fprintf(stderr, "No input file name given.\n");
    return 2;
  } else if (strcmp(in_fname, "-") == 0) {
    to_read = stdin;
  } else {
    to_read = fopen(in_fname, "rb");
    if (to_read == NULL) {
      int errval = errno;
      fprintf(stderr, "Failed to open '%s'.\n\t%s\n",
        in_fname, strerror(errval));
      return 1;
    }
  }
  img.fptr = to_read;
  /* input from PPM */ {
    int const in_result = test_image_get_ppm(&img);
    if (in_result != PNGPARTS_API_OK){
      /* close */
      free(img.bytes);
      if (to_read != stdin) fclose(to_read);
      fflush(NULL);
      if (in_result) {
        fprintf(stderr, "\nResult code from PPM %i: %s\n",
          in_result, pngparts_api_strerror(in_result));
      }
      return in_result;
    }
  }

  /* input from alpha PGM */ if (alpha_fname != NULL){
    FILE* alpha_file = fopen(alpha_fname, "rt");
    int alpha_result;
    if (alpha_file != NULL){
      img.alpha_fptr = alpha_file;
      alpha_result = test_image_get_alphapgm(&img);
      fclose(alpha_file);
    } else {
      alpha_result = PNGPARTS_API_IO_ERROR;
    }
    if (alpha_result != PNGPARTS_API_OK){
      /* close */
      free(img.bytes);
      if (to_write != stdout) fclose(to_write);
      if (to_read != stdin) fclose(to_read);
      fflush(NULL);
      if (alpha_result) {
        fprintf(stderr, "\nResult code from alpha PGM %i: %s\n",
          alpha_result, pngparts_api_strerror(alpha_result));
      }
      return alpha_result;
    }
  }

  result = pngparts_aux_write_block
    (img.width, img.height, 0, 0, img.color_type, 8, img.bytes, out_fname);
  /* close */
  free(img.bytes);
  if (to_read != stdin) fclose(to_read);
  fflush(NULL);
  if (result) {
    fprintf(stderr, "\nResult code %i: %s\n",
      result, pngparts_api_strerror(result));
  }
  return result;
}
