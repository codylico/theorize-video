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

#include "../src/flate.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

static int length_encode_main(int argc, char **argv);
static int length_decode_main(int argc, char **argv);
static int distance_encode_main(int argc, char **argv);
static int distance_decode_main(int argc, char **argv);

int length_encode_main(int argc, char **argv){
  int argi;
  struct pngparts_flate_extra extra;
  char const* length_text = NULL;
  int help_tf = 0;
  for (argi = 1; argi < argc; ++argi){
    if (strcmp("-",argv[argi]) == 0){
      /* ignore */
    } else if (strcmp("-?",argv[argi]) == 0){
      help_tf = 1;
    } else {
      length_text = argv[argi];
    }
  }
  /*print help */if (help_tf || length_text == NULL){
    fprintf(stderr,"usage: test_flate length_encode ... (length)\n"
      "  -                  (ignored)\n"
      "  -?                 help message\n"
      );
    return 2;
  }
  /* compute the tokens */{
    unsigned int const length = (unsigned int)strtoul(length_text,NULL,0);
    extra = pngparts_flate_length_encode(length);
    if (extra.literal < 0){
      fprintf(stderr,"(not found)\n");
      return 1;
    } else {
      if (extra.extra_bits > 0){
        fprintf(stdout,"%i %i\n",extra.literal,
            (length - extra.length_value)&((1<<extra.extra_bits)-1));
      } else {
        fprintf(stdout,"%i\n",extra.literal);
      }
      fprintf(stderr,"extra_bits: %u\nlength_value: %u\nliteral: %u\n",
        extra.extra_bits, extra.length_value, extra.literal);
      return 0;
    }
  }
}

int length_decode_main(int argc, char **argv){
  int argi;
  struct pngparts_flate_extra extra;
  char const* length_text = NULL;
  char const* length_extra_text = NULL;
  int help_tf = 0;
  for (argi = 1; argi < argc; ++argi){
    if (strcmp("-",argv[argi]) == 0){
      /* ignore */
    } else if (strcmp("-?",argv[argi]) == 0){
      help_tf = 1;
    } else if (length_text == NULL){
      length_text = argv[argi];
    } else {
      length_extra_text = argv[argi];
    }
  }
  /*print help */if (help_tf || length_text == NULL){
    fprintf(stderr,"usage: test_flate length_decode ... (token) [token]\n"
      "  -                  (ignored)\n"
      "  -?                 help message\n"
      );
    return 2;
  }
  /* compute the length */{
    unsigned int const length = (unsigned int)strtoul(length_text,NULL,0);
    extra = pngparts_flate_length_decode(length);
    if (extra.length_value < 0){
      /* check if this is something else */{
        if (length < 256){
          fprintf(stderr,"(not found; literal: %u)\n", length);
        } else if (length == 256){
          fprintf(stderr,"(not found; stop code)\n");
        } else {
          fprintf(stderr,"(not found)\n");
        }
      }
      return 1;
    } else if (extra.extra_bits > 0 && length_extra_text == NULL) {
      unsigned int const fake_length = extra.length_value;
      fprintf(stderr,"(length at least %u; need extra bits)\n",
        fake_length);
      fprintf(stderr,"extra_bits: %u\nlength_value: %u\nliteral: %u\n",
        extra.extra_bits, extra.length_value, extra.literal);
      return 3;
    } else {
      unsigned int const length_extra =
        (unsigned int)strtoul(length_extra_text,NULL,0);
      unsigned int const real_length =
        extra.length_value + (length_extra&((1<<extra.extra_bits)-1));
      fprintf(stdout,"%u\n",real_length);
      fprintf(stderr,"extra_bits: %u\nlength_value: %u\nliteral: %u\n",
        extra.extra_bits, extra.length_value, extra.literal);
      return 0;
    }
  }
}


int distance_encode_main(int argc, char **argv){
  int argi;
  struct pngparts_flate_extra extra;
  char const* distance_text = NULL;
  int help_tf = 0;
  for (argi = 1; argi < argc; ++argi){
    if (strcmp("-",argv[argi]) == 0){
      /* ignore */
    } else if (strcmp("-?",argv[argi]) == 0){
      help_tf = 1;
    } else {
      distance_text = argv[argi];
    }
  }
  /*print help */if (help_tf || distance_text == NULL){
    fprintf(stderr,"usage: test_flate distance_encode ... (distance)\n"
      "  -                  (ignored)\n"
      "  -?                 help message\n"
      );
    return 2;
  }
  /* compute the tokens */{
    unsigned int const distance = (unsigned int)strtoul(distance_text,NULL,0);
    extra = pngparts_flate_distance_encode(distance);
    if (extra.literal < 0){
      fprintf(stderr,"(not found)\n");
      return 1;
    } else {
      if (extra.extra_bits > 0){
        fprintf(stdout,"%i %i\n",extra.literal,
            (distance - extra.length_value)&((1<<extra.extra_bits)-1));
      } else {
        fprintf(stdout,"%i\n",extra.literal);
      }
      fprintf(stderr,"extra_bits: %u\ndistance_value: %u\nliteral: %u\n",
        extra.extra_bits, extra.length_value, extra.literal);
      return 0;
    }
  }
}

int distance_decode_main(int argc, char **argv){
  int argi;
  struct pngparts_flate_extra extra;
  char const* distance_text = NULL;
  char const* distance_extra_text = NULL;
  int help_tf = 0;
  for (argi = 1; argi < argc; ++argi){
    if (strcmp("-",argv[argi]) == 0){
      /* ignore */
    } else if (strcmp("-?",argv[argi]) == 0){
      help_tf = 1;
    } else if (distance_text == NULL){
      distance_text = argv[argi];
    } else {
      distance_extra_text = argv[argi];
    }
  }
  /*print help */if (help_tf || distance_text == NULL){
    fprintf(stderr,"usage: test_flate distance_decode ... (token) [token]\n"
      "  -                  (ignored)\n"
      "  -?                 help message\n"
      );
    return 2;
  }
  /* compute the distance */{
    unsigned int const distance = (unsigned int)strtoul(distance_text,NULL,0);
    extra = pngparts_flate_distance_decode(distance);
    if (extra.length_value < 0){
      /* check if this is something else */{
        if (distance < 256){
          fprintf(stderr,"(not found; literal: %u)\n", distance);
        } else if (distance == 256){
          fprintf(stderr,"(not found; stop code)\n");
        } else {
          fprintf(stderr,"(not found)\n");
        }
      }
      return 1;
    } else if (extra.extra_bits > 0 && distance_extra_text == NULL) {
      unsigned int const fake_distance = extra.length_value;
      fprintf(stderr,"(distance at least %u; need extra bits)\n",
        fake_distance);
      fprintf(stderr,"extra_bits: %u\ndistance_value: %u\nliteral: %u\n",
        extra.extra_bits, extra.length_value, extra.literal);
      return 3;
    } else {
      unsigned int const distance_extra =
        (unsigned int)strtoul(distance_extra_text,NULL,0);
      unsigned int const real_distance =
        extra.length_value + (distance_extra&((1<<extra.extra_bits)-1));
      fprintf(stdout,"%u\n",real_distance);
      fprintf(stderr,"extra_bits: %u\ndistance_value: %u\nliteral: %u\n",
        extra.extra_bits, extra.length_value, extra.literal);
      return 0;
    }
  }
}


int main(int argc, char **argv){
  if (argc < 2){
    fprintf(stderr,"available commands: \n"
      "  length_encode    encode a length value\n"
      "  length_decode    decode a length value\n"
      "  distance_encode  encode a distance value\n"
      "  distance_decode  decode a distance value\n"
      );
    return 2;
  } else if (strcmp("length_encode",argv[1]) == 0){
    return length_encode_main(argc-1,argv+1);
  } else if (strcmp("length_decode",argv[1]) == 0){
    return length_decode_main(argc-1,argv+1);
  } else if (strcmp("distance_encode",argv[1]) == 0){
    return distance_encode_main(argc-1,argv+1);
  } else if (strcmp("distance_decode",argv[1]) == 0){
    return distance_decode_main(argc-1,argv+1);
  } else {
    fprintf(stdout,"unknown command: %s\n", argv[1]);
    return 2;
  }
}
