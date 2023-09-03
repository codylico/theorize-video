/*
 * PNG-parts
 * parts of a Portable Network Graphics implementation
 * Copyright 2018-2019 Cody Licorish
 *
 * Licensed under the MIT License.
 *
 * flate.h
 * flate main header
 */
#ifndef __PNG_PARTS_FLATE_H__
#define __PNG_PARTS_FLATE_H__

#include "api.h"

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/*
 * Compression block types.
 */
enum pngparts_flate_block_type {
  /* put bytes as is */
  PNGPARTS_FLATE_PLAIN = 0,
  /* use text-optimized Huffman code table */
  PNGPARTS_FLATE_FIXED = 1,
  /* use dynamic Huffman code table */
  PNGPARTS_FLATE_DYNAMIC = 2
};

/*
 * Compression block levels.
 */
enum pngparts_flate_block_level {
  /* put bytes as is */
  PNGPARTS_FLATE_OFF = 0,
  /* low compression */
  PNGPARTS_FLATE_LOW = 1,
  /* medium compression */
  PNGPARTS_FLATE_MEDIUM = 2,
  /* high compression */
  PNGPARTS_FLATE_HIGH = 3
};

/*
 * Length conversion structure.
 */
struct pngparts_flate_extra {
  /* stream value */
  int literal;
  /* first length in bytes */
  int length_value;
  /* extra bits for encoding length */
  int extra_bits;
};
/*
 * Huffman code item
 */
struct pngparts_flate_code {
  /* bit length of the item */
  short length;
  /* bits for the item */
  unsigned int bits;
  /* corresponding length-literal value */
  int value;
};
/*
 * Huffman code table
 */
struct pngparts_flate_huff {
  /* array of items */
  struct pngparts_flate_code *its;
  /* capacity of items */
  int cap;
  /* number of items */
  int count;
};

/*
 * Past hash table
 */
struct pngparts_flate_hash {
  /* next indices */
  unsigned short *next;
  /* first indices */
  unsigned short *first;
  /* size of next indices */
  unsigned short next_size;
  /* current table write position */
  unsigned short pos;
  /* last few bytes */
  unsigned char bytes[2];
  /* minimal bytes processed so far */
  unsigned char byte_size;
  /* maximum index of first array */
  unsigned char first_max;
};

/*
 * base for flater
 */
struct pngparts_flate {
  /* bit position for last input byte */
  signed char bitpos;
  /* last input byte */
  unsigned char last_input_byte;
  /* state */
  signed char state;
  /* size of history of bits */
  unsigned char bitlength;
  /* history of previous bits */
  unsigned int bitline;
  /* amount of inscription committed so far */
  unsigned short inscription_commit;
  /* position in current inscription */
  unsigned short inscription_pos;
  /* size of inscription in short integers */
  unsigned short inscription_size;
  /*
   * run-time parameter:
   * number of bytes after which to throw away a second match
   */
  unsigned short match_truncate;
  /* text to write */
  unsigned short* inscription_text;
  /* past bytes read */
  unsigned char* history_bytes;
  /* size of history in bytes */
  unsigned int history_size;
  /* position in history */
  unsigned int history_pos;
  /* forward and reverse lengths */
  unsigned char shortbuf[4];
  /* short buffer position */
  unsigned short short_pos;
  /* inscription alphabet */
  unsigned short* alphabet;
  /* direct block length */
  unsigned int block_length;
  /* repeat length */
  unsigned int repeat_length;
  /* repeat distance */
  unsigned int repeat_distance;
  /* code for code table */
  struct pngparts_flate_huff code_table;
  /* length code table */
  struct pngparts_flate_huff length_table;
  /* distance code table */
  struct pngparts_flate_huff distance_table;
  /* compression type */
  unsigned char block_type;
  /* compression level */
  unsigned char block_level;
  /* next output byte */
  unsigned char next_output_byte;
  /* next inscription alternative */
  unsigned short alt_inscription[5];
  /* hash table for compression pairs */
  struct pngparts_flate_hash pointer_hash;
};


/*
 * Construct a code struct by literal or length value.
 * - value literal or length
 * @return the code
 */
PNGPARTS_API
struct pngparts_flate_code pngparts_flate_code_by_literal(int value);
/*
 * Compare two codes by bit strings.
 * - a the first code struct
 * - b the second code struct
 * @return 0 if equal, -1 if a < b, +1 if a > b
 */
PNGPARTS_API
int pngparts_flate_code_bitcmp(void const* a, void const* b);

/*
 * Initialize a Huffman code table.
 * - hf Huffman code table
 */
PNGPARTS_API
void pngparts_flate_huff_init(struct pngparts_flate_huff* hf);
/*
 * Free out a Huffman code table.
 * - hf Huffman code table
 */
PNGPARTS_API
void pngparts_flate_huff_free(struct pngparts_flate_huff* hf);
/*
 * Resize the given code table.
 * - hf table to resize
 * - siz number of items
 * @return OK on success, MEMORY on failure
 */
PNGPARTS_API
int pngparts_flate_huff_resize(struct pngparts_flate_huff* hf, int siz);
/*
 * Get table size in codes.
 * - hf Huffman code table
 * @return a length
 */
PNGPARTS_API
int pngparts_flate_huff_get_size(struct pngparts_flate_huff const* hf);
/*
 * Access a code in the table.
 * - hf table to modify
 * - i array index
 * @return the code at that index
 */
PNGPARTS_API
struct pngparts_flate_code pngparts_flate_huff_index_get
  (struct pngparts_flate_huff const* hf, int i);
/*
 * Access a code in the table.
 * - hf table to modify
 * - i array index
 * - c new code
 */
PNGPARTS_API
void pngparts_flate_huff_index_set
  (struct pngparts_flate_huff* hf, int i, struct pngparts_flate_code c);
/*
 * Generate from bit lengths the bit strings.
 * - hf table to modify
 * @return OK on success, CODE_EXCESS if code count exceeds constraints,
 *   BAD_PARAM if the lengths too long
 */
PNGPARTS_API
int pngparts_flate_huff_generate(struct pngparts_flate_huff* hf);
/*
 * Generate from histograms the bit lengths.
 * - hf table to modify
 * - hist relative histogram, size equal to size of table
 */
PNGPARTS_API
void pngparts_flate_huff_make_lengths
  (struct pngparts_flate_huff* hf, int const* hist);
/*
 * Generate from histograms the bit lengths.
 * - hf table to modify
 * - hist relative histogram, size equal to size of table
 * - max_len maximum length for any code
 */
PNGPARTS_API
void pngparts_flate_huff_limit_lengths
  (struct pngparts_flate_huff* hf, int const* hist, int max_len);
/*
 * Sort by code bits.
 * - hf table to sort
 */
PNGPARTS_API
void pngparts_flate_huff_bit_sort(struct pngparts_flate_huff* hf);
/*
 * Sort by values.
 * - hf table to sort
 */
PNGPARTS_API
void pngparts_flate_huff_value_sort(struct pngparts_flate_huff* hf);
/*
 * Search by code bits, binary search.
 * - hf table to sort
 * - length length of bit string
 * - bits bit string; last bit is lsb,
 * @return the value corresponding to the bit string, NOT_FOUND if
 *   the value was not found, or BAD_BITS if the length is improper
 */
PNGPARTS_API
int pngparts_flate_huff_bit_bsearch
  (struct pngparts_flate_huff const* hf, int length, int bits);
/*
 * Search by code bits, linear search.
 * - hf table to sort
 * - length length of bit string
 * - bits bit string; last bit is lsb,
 * @return the value corresponding to the bit string, NOT_FOUND if
 *   the value was not found, or BAD_BITS if the length is improper
 */
PNGPARTS_API
int pngparts_flate_huff_bit_lsearch
  (struct pngparts_flate_huff const* hf, int length, int bits);

/*
 * Load the fixed Huffman literal table into a structure.
 * - hf table to modify, should have been already resized to
 *   at least 288 entries
 */
PNGPARTS_API
void pngparts_flate_fixed_lengths(struct pngparts_flate_huff* hf);
/*
 * Load the fixed Huffman distance table into a structure.
 * - hf table to modify, should have been already resized to
 *   at least 32 entries
 */
PNGPARTS_API
void pngparts_flate_fixed_distances(struct pngparts_flate_huff* hf);
/*
 * Load code length values.
 * - hf table to modify, should have at least four entries
 */
PNGPARTS_API
void pngparts_flate_dynamic_codes(struct pngparts_flate_huff* hf);

/*
 * Add to history.
 * - fl flate structure to update
 * - ch byte to add (0 - 255)
 */
PNGPARTS_API
void pngparts_flate_history_add(struct pngparts_flate *fl, int ch);
/*
 * Get the history byte.
 * - fl flate with history book to read
 * - dist distance to go back (1 is most recent)
 * @return the value at that point in history
 */
PNGPARTS_API
int pngparts_flate_history_get(struct pngparts_flate *fl, int dist);

/*
 * Literal to length conversion function.
 * - literal stream length code
 * @return a structure holding the minimum repeat length for
 *   the given length code, along with the number of extra bits
 *   needed for encoding the number, or a structure with negative
 *   repeat length on error
 */
PNGPARTS_API
struct pngparts_flate_extra pngparts_flate_length_decode(int literal);
/*
 * Distance code to distance conversion function.
 * - dcode stream distance code
 * @return a structure holding the minimum repeat distance for
 *   the given distance code, along with the number of extra bits
 *   needed for encoding the number, or a structure with negative
 *   repeat distance on error
 */
PNGPARTS_API
struct pngparts_flate_extra pngparts_flate_distance_decode(int dcode);

/*
 * Literal from length conversion function.
 * - length stream length in bytes
 * @return a structure holding the minimum repeat length for
 *   the given length code, along with the number of extra bits
 *   needed for encoding the number, or a structure with negative
 *   repeat length on error
 */
PNGPARTS_API
struct pngparts_flate_extra pngparts_flate_length_encode(int length);

/*
 * Distance code from distance conversion function.
 * - dcode stream distance in bytes
 * @return a structure holding the minimum repeat distance for
 *   the given distance code, along with the number of extra bits
 *   needed for encoding the number, or a structure with negative
 *   repeat distance on error
 */
PNGPARTS_API
struct pngparts_flate_extra pngparts_flate_distance_encode
  (unsigned int distance);

/*
 * Initialize an empty hash table.
 * - hash the table to initialize
 */
PNGPARTS_API
void pngparts_flate_hash_init(struct pngparts_flate_hash *hash);

/*
 * Empty the hash table.
 * - hash the table to empty
 */
PNGPARTS_API
void pngparts_flate_hash_free(struct pngparts_flate_hash *hash);

/*
 * Empty the hash table.
 * - hash the table to configure
 * - size new history size
 * @return zero on success, MEMORY otherwise
 */
PNGPARTS_API
int pngparts_flate_hash_prepare
  (struct pngparts_flate_hash *hash, unsigned int size);

/*
 * Add to hash table.
 * - hash table structure to update
 * - ch byte to add (0 - 255)
 */
PNGPARTS_API
void pngparts_flate_hash_add(struct pngparts_flate_hash *hash, int ch);

/*
 * Add to hash triple collation, but skip insertion into the hash table.
 * - hash table structure to update
 * - ch byte to add (0 - 255)
 */
PNGPARTS_API
void pngparts_flate_hash_skip(struct pngparts_flate_hash *hash, int ch);

/*
 * Check the hash table.
 * - hash table structure to query
 * - history_bytes bytes of history from a corresponding flate structure
 * - chs 3-byte sequence for which to search
 * - start point in history from which to start searching, or zero
 *     to start from most recent history
 * @return a distance in history if found, or zero if not found
 */
PNGPARTS_API
unsigned int pngparts_flate_hash_check
  ( struct pngparts_flate_hash *hash, unsigned char const* history_bytes,
    unsigned char const* chs, unsigned int start);

#ifdef __cplusplus
};
#endif /*__cplusplus*/

#endif /*__PNG_PARTS_FLATE_H__*/
