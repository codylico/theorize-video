/*
 * PNG-parts
 * parts of a Portable Network Graphics implementation
 * Copyright 2018-2019 Cody Licorish
 *
 * Licensed under the MIT License.
 *
 * api.h
 * API main header
 */
#ifndef __PNG_PARTS_API_H__
#define __PNG_PARTS_API_H__

/*
 * export variable
 */
#ifndef PNGPARTS_API
#  if (defined PNGPARTS_API_SHARED)
#    ifdef pngparts_EXPORTS
#      define PNGPARTS_API __declspec(dllexport)
#    else
#      define PNGPARTS_API __declspec(dllimport)
#    endif /*pngparts_EXPORTS*/
#  else
#    define PNGPARTS_API
#  endif /*PNGPARTS_API_SHARED*/
#endif /*PNGPARTS_API*/

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/*
 * API information flags
 */
enum pngparts_api_flag {
  /* whether the library uses explicit exporting */
  PNGPARTS_API_EXPORTS = 1
};

/*
 * Errors
 */
enum pngparts_api_error {
  /* the image is too wide to process */
  PNGPARTS_API_TOO_WIDE = -27,
  /* chunk callback did not set a valid output byte */
  PNGPARTS_API_MISSING_PUT = -26,
  /* state machine caught in a loop */
  PNGPARTS_API_LOOPED_STATE = -25,
  /* chunk size too long */
  PNGPARTS_API_CHUNK_TOO_LONG = -24,
  /* too few IDAT chunk data for pixels */
  PNGPARTS_API_SHORT_IDAT = -23,
  /* weird filter value encountered */
  PNGPARTS_API_WEIRD_FILTER = -22,
  /* critical chunk not handled by any callbacks */
  PNGPARTS_API_UNCAUGHT_CRITICAL = -21,
  /* IHDR damaged or invalid */
  PNGPARTS_API_BAD_HDR = -20,
  /* IHDR missing from start of stream */
  PNGPARTS_API_MISSING_HDR = -19,
  /* CRC mismatch */
  PNGPARTS_API_BAD_CRC = -18,
  /* signature mismatch */
  PNGPARTS_API_BAD_SIGNATURE = -17,
  /* dictionary given was wrong */
  PNGPARTS_API_WRONG_DICT = -16,
  /* bad code length */
  PNGPARTS_API_BAD_CODE_LENGTH = -15,
  /* value not found */
  PNGPARTS_API_NOT_FOUND = -14,
  /* code lengths exceeded the block size */
  PNGPARTS_API_CODE_EXCESS = -13,
  /* block corrupted */
  PNGPARTS_API_BAD_BLOCK = -12,
  /* corrupt length value */
  PNGPARTS_API_CORRUPT_LENGTH = -11,
  /* memory allocation failure */
  PNGPARTS_API_MEMORY = -10,
  /* bad Adler32 checksum */
  PNGPARTS_API_BAD_SUM = -9,
  /* unsupported stream compression algorithm */
  PNGPARTS_API_UNSUPPORTED = -8,
  /* bad bit string or bit length */
  PNGPARTS_API_BAD_BITS = -7,
  /* i/o error */
  PNGPARTS_API_IO_ERROR = -6,
  /* parameter not fit the function */
  PNGPARTS_API_BAD_PARAM = -5,
  /* dictionary requested */
  PNGPARTS_API_NEED_DICT = -4,
  /* bad check value */
  PNGPARTS_API_BAD_CHECK = -3,
  /* state machine broke */
  PNGPARTS_API_BAD_STATE = -2,
  /* premature end of file */
  PNGPARTS_API_EOF = -1,
  /* all is good */
  PNGPARTS_API_OK = 0,
  /* output buffer overflow */
  PNGPARTS_API_OVERFLOW = 1,
  /* the stream is done; quit pushing data */
  PNGPARTS_API_DONE = 2,
  /* the callback is not yet ready */
  PNGPARTS_API_NOT_READY = 3
};
/*
 * Expectation mode for zlib stream processing.
 */
enum pngparts_api_z_mode {
  /* normal reading */
  PNGPARTS_API_Z_NORMAL = 0,
  /* treat it like it's the end */
  PNGPARTS_API_Z_FINISH = 1
};


/*
 * Generic free callback.
 * - ptr data to release
 */
typedef void (*pngparts_api_free_cb)(void* ptr);

/*
 * Start callback.
 * - cb_data flate callback data
 * - fdict dictionary
 * - flevel compression level
 * - cm compression method
 * - cinfo compression information
 * @return OK if the callback supports the stream,
 *   or UNSUPPORTED otherwise
 */
typedef int (*pngparts_api_flate_start_cb)
  ( void* cb_data,
    short int fdict, short int flevel, short int cm, short int cinfo);
/*
 * Dictionary byte callback.
 * - cb_data flate callback data
 * - ch byte, or -1 for repeat bytes
 * @return OK, or UNSUPPORTED if preset dictionaries are not supported
 */
typedef int (*pngparts_api_flate_dict_cb)(void* cb_data, int ch);
/*
 * Put an output byte.
 * - zout callback data
 * - ch byte to put (0-255)
 * @return OK on success, OVERFLOW if no more room for bytes
 */
typedef int (*pngparts_api_flate_put_cb)(void* zout, int ch);
/*
 * Byte callback.
 * - cb_data flate callback data
 * - ch byte, or -1 for repeat bytes
 * - put_data data to pass to put callback
 * - put_cb callback for putting output bytes
 * @return OK, or OVERFLOW if the output buffer is too full,
 *   or DONE at the end of the bit stream; return other negative on error
 */
typedef int (*pngparts_api_flate_one_cb)
  ( void* cb_data, int ch, void* put_data, pngparts_api_flate_put_cb put_cb);
/*
 * Finish callback.
 * - cb_data flate callback data
 * - put_data data to pass to put callback
 * - put_cb callback for putting output bytes
 * @return zero, or EOF if the callback expected more data
 */
typedef int (*pngparts_api_flate_finish_cb)
  (void* cb_data, void* put_data, pngparts_api_flate_put_cb put_cb);
/*
 * Interface for DEFLATE algorithms
 */
struct pngparts_api_flate {
  /* callback data */
  void* cb_data;
  /* start callback */
  pngparts_api_flate_start_cb start_cb;
  /* dictionary callback */
  pngparts_api_flate_dict_cb dict_cb;
  /* bit callback */
  pngparts_api_flate_one_cb one_cb;
  /* finish callback */
  pngparts_api_flate_finish_cb finish_cb;
};
/*
 * Create an empty DEFLATE callback interface.
 * @return an empty interface structure
 */
struct pngparts_api_flate pngparts_api_flate_empty(void);



/*
 * Setup an input buffer for next use.
 * - zs zlib stream struct (struct pngparts_z)
 * - inbuf input buffer
 * - insize amount of data to input
 */
typedef void (*pngparts_api_z_set_input_cb)
  (void *zs, void* inbuf, int insize);

/*
 * Setup an output buffer for next use.
 * - zs zlib stream struct (struct pngparts_z)
 * - outbuf output buffer
 * - outsize amount of space available for output
 */
typedef void (*pngparts_api_z_set_output_cb)
  (void *zs, void* outbuf, int outsize);

/*
 * Check if the reader has used up all the latest input.
 * - zs zlib stream struct (struct pngparts_z)
 * @return nonzero if the input is used up
 */
typedef int (*pngparts_api_z_input_done_cb)(void const* zs);

/*
 * Check how much output bytes wait for you.
 * - zs zlib stream struct (struct pngparts_z)
 * @return byte count for the output bytes
 */
typedef int (*pngparts_api_z_output_left_cb)(void const* zs);

/*
 * Process a part of a stream.
 * - zs zlib stream struct (struct pngparts_z)
 * - mode stream expectation mode (enum pngparts_api_z_mode)
 * @return OK on success, DONE at end of stream, EOF
 *   on unexpected end of stream
 */
typedef int (*pngparts_api_z_churn_cb)(void* zs, int mode);

/*
 * Try to set the dictionary for use.
 * - zs zlib stream struct
 * - ptr bytes of the dictionary
 * - len dictionary length in bytes
 * @return OK if the dictionary matches the stream's
 *   dictionary checksum, WRONG_DICT if the dictionary match fails,
 *   or BAD_STATE if called before the dictionary checksum is
 *   available
 */
typedef int (*pngparts_api_z_set_dict_cb)
  (void* zs, unsigned char const* ptr, int len);

/*
 * Interface for zlib stream algorithms
 */
struct pngparts_api_z {
  /* callback data */
  void* cb_data;
  /* start callback */
  pngparts_api_z_set_input_cb set_input_cb;
  /* dictionary callback */
  pngparts_api_z_set_output_cb set_output_cb;
  /* bit callback */
  pngparts_api_z_input_done_cb input_done_cb;
  /* finish callback */
  pngparts_api_z_output_left_cb output_left_cb;
  /* set dictionary callback */
  pngparts_api_z_set_dict_cb set_dict_cb;
  /* processing callback */
  pngparts_api_z_churn_cb churn_cb;
};

/*
 * Create an empty zlib stream callback interface.
 * @return an empty interface structure
 */
struct pngparts_api_z pngparts_api_z_empty(void);




/*
 * Callback for starting image processing.
 * - img callback data
 * - width image width
 * - height image height
 * - bit_depth sample depth
 * - color_type PNG color bit field
 * - compression method of compression (should be zero)
 * - filter filter method (should be zero)
 * - interlace interlace method (should be zero or one)
 * @return OK if the header good for the image, UNSUPPORTED
 *   otherwise
 */
typedef int (*pngparts_api_image_start_cb)
  ( void* img, long int width, long int height, short bit_depth,
    short color_type, short compression, short filter, short interlace);
/*
 * Put a color to the image.
 * - img image
 * - x x-coordinate
 * - y y-coordinate
 * - red red sample
 * - green green sample
 * - blue blue sample
 * - alpha alpha sample
 */
typedef void (*pngparts_api_image_put_cb)
  ( void* img, long int x, long int y,
    unsigned int red, unsigned int green, unsigned int blue,
    unsigned int alpha);

/*
 * Callback for describing the image to process.
 * - img callback data
 * - width output for image width
 * - height output for image height
 * - bit_depth output for sample depth
 * - color_type output for PNG color bit field
 * - compression output for method of compression (should be zero)
 * - filter output for filter method (should be zero)
 * - interlace output for interlace method (should be zero or one)
 */
typedef void (*pngparts_api_image_describe_cb)
  ( void* img, long int *width, long int *height, short *bit_depth,
    short *color_type, short *compression, short *filter, short *interlace);

/*
 * Get a color from the image.
 * - img image
 * - x x-coordinate
 * - y y-coordinate
 * - red output for red sample
 * - green output for green sample
 * - blue output for blue sample
 * - alpha output for alpha sample
 */
typedef void (*pngparts_api_image_get_cb)
  ( void* img, long int x, long int y,
    unsigned int *red, unsigned int *green, unsigned int *blue,
    unsigned int *alpha);

struct pngparts_api_image {
  /* callback data */
  void* cb_data;
  /* image start callback (read only)*/
  pngparts_api_image_start_cb start_cb;
  /* image color posting callback (read only)*/
  pngparts_api_image_put_cb put_cb;
  /* image describe callback (write only)*/
  pngparts_api_image_describe_cb describe_cb;
  /* image color fetch callback (write only)*/
  pngparts_api_image_get_cb get_cb;
};

/*
 * API information as an integer
 */
PNGPARTS_API
int pngparts_api_info(void);
/*
 * Error message.
 * - result error value
 * @return corresponding error message
 */
PNGPARTS_API
char const* pngparts_api_strerror(int result);

#ifdef __cplusplus
};
#endif /*__cplusplus*/

#endif /*__PNG_PARTS_API_H__*/
