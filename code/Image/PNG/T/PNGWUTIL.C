
/* pngwutil.c - utilities to write a png file

   pnglib version 0.6
   For conditions of distribution and use, see copyright notice in png.h
   Copyright (c) 1995 Guy Eric Schalnat, Group 42, Inc.
   May 1, 1995
   */
#define PNG_INTERNAL
#include "png.h"

/* place a 32 bit number into a buffer in png byte order.  We work
   with unsigned numbers for convenience, you may have to cast
   signed numbers (if you use any, most png data is unsigned). */
void
png_save_uint_32(png_byte *buf, png_uint_32 i)
{
   buf[0] = (png_byte)((i >> 24) & 0xff);
   buf[1] = (png_byte)((i >> 16) & 0xff);
   buf[2] = (png_byte)((i >> 8) & 0xff);
   buf[3] = (png_byte)(i & 0xff);
}

/* place a 16 bit number into a buffer in png byte order */
void
png_save_uint_16(png_byte *buf, png_uint_16 i)
{
   buf[0] = (png_byte)((i >> 8) & 0xff);
   buf[1] = (png_byte)(i & 0xff);
}

/* write a 32 bit number */
void
png_write_uint_32(png_struct *png_ptr, png_uint_32 i)
{
   png_byte buf[4];

   buf[0] = (png_byte)((i >> 24) & 0xff);
   buf[1] = (png_byte)((i >> 16) & 0xff);
   buf[2] = (png_byte)((i >> 8) & 0xff);
   buf[3] = (png_byte)(i & 0xff);
   png_write_data(png_ptr, buf, 4);
}

/* write a 16 bit number */
void
png_write_uint_16(png_struct *png_ptr, png_uint_16 i)
{
   png_byte buf[2];

   buf[0] = (png_byte)((i >> 8) & 0xff);
   buf[1] = (png_byte)(i & 0xff);
   png_write_data(png_ptr, buf, 2);
}

/* Write a png chunk all at once.  The type is an array of ASCII characters
   representing the chunk name.  The array must be at least 4 bytes in
   length, and does not need to be null terminated.  To be safe, pass the
   pre-defined chunk names here, and if you need a new one, define it
   where the others are defined.  The length is the length of the data.
   All the data must be present.  If that is not possible, use the
   png_write_chunk_start(), png_write_chunk_data(), and png_write_chunk_end()
   functions instead.  */
void
png_write_chunk(png_struct *png_ptr, png_byte *type,
   png_byte *data, png_uint_32 length)
{
   /* write length */
   png_write_uint_32(png_ptr, length);
   /* write chunk name */
   png_write_data(png_ptr, type, (png_uint_32)4);
   /* reset the crc and run the chunk name over it */
   png_reset_crc(png_ptr);
   png_calculate_crc(png_ptr, type, (png_uint_32)4);
   /* write the data and update the crc */
   if (length)
   {
      png_calculate_crc(png_ptr, data, length);
      png_write_data(png_ptr, data, length);
   }
   /* write the crc */
   png_write_uint_32(png_ptr, ~png_ptr->crc);
}

/* Write the start of a png chunk.  The type is the chunk type.
   The total_length is the sum of the lengths of all the data you will be
   passing in png_write_chunk_data() */
void
png_write_chunk_start(png_struct *png_ptr, png_byte *type,
   png_uint_32 total_length)
{
   /* write the length */
   png_write_uint_32(png_ptr, total_length);
   /* write the chunk name */
   png_write_data(png_ptr, type, (png_uint_32)4);
   /* reset the crc and run it over the chunk name */
   png_reset_crc(png_ptr);
   png_calculate_crc(png_ptr, type, (png_uint_32)4);
}

/* write the data of a png chunk started with png_write_chunk_start().
   Note that multiple calls to this function are allowed, and that the
   sum of the lengths from these calls *must* add up to the total_length
   given to png_write_chunk_start() */
void
png_write_chunk_data(png_struct *png_ptr, png_byte *data, png_uint_32 length)
{
   /* write the data, and run the crc over it */
   if (length)
   {
      png_calculate_crc(png_ptr, data, length);
      png_write_data(png_ptr, data, length);
   }
}

/* finish a chunk started with png_write_chunk_start() */
void
png_write_chunk_end(png_struct *png_ptr)
{
   /* write the crc */
   png_write_uint_32(png_ptr, ~png_ptr->crc);
}

/* simple function to write the signature */
void
png_write_sig(png_struct *png_ptr)
{
   /* write the 8 byte signature */
   png_write_data(png_ptr, png_sig, (png_uint_32)8);
}

/* Write the IHDR chunk, and update the png_struct with the necessary
   information.  Note that the rest of this code depends upon this
   information being correct.  */
void
png_write_IHDR(png_struct *png_ptr, png_uint_32 width, png_uint_32 height,
   int bit_depth, int color_type, int compression_type, int filter_type,
   int interlace_type)
{
   png_byte buf[13]; /* buffer to store the IHDR info */

   /* pack the header information into the buffer */
   png_save_uint_32(buf, width);
   png_save_uint_32(buf + 4, height);
   buf[8] = bit_depth;
   buf[9] = color_type;
   buf[10] = compression_type;
   buf[11] = filter_type;
   buf[12] = interlace_type;
   /* save off the relevent information */
   png_ptr->bit_depth = bit_depth;
   png_ptr->color_type = color_type;
   png_ptr->interlaced = interlace_type;
   png_ptr->width = width;
   png_ptr->height = height;

   switch (color_type)
   {
      case 0:
      case 3:
         png_ptr->channels = 1;
         break;
      case 2:
         png_ptr->channels = 3;
         break;
      case 4:
         png_ptr->channels = 2;
         break;
      case 6:
         png_ptr->channels = 4;
         break;
   }
   png_ptr->pixel_depth = bit_depth * png_ptr->channels;
   png_ptr->rowbytes = ((width * (png_uint_32)png_ptr->pixel_depth + 7) >> 3);
   /* set the usr info, so any transformations can modify it */
   png_ptr->usr_width = png_ptr->width;
   png_ptr->usr_bit_depth = png_ptr->bit_depth;
    png_ptr->usr_channels = png_ptr->channels;

   /* write the chunk */
   png_write_chunk(png_ptr, png_IHDR, buf, (png_uint_32)13);
}

/* write the palette.  We are careful not to trust png_color to be in the
   correct order for PNG, so people can redefine it to any convient
   structure. */
void
png_write_PLTE(png_struct *png_ptr, png_color *palette, int number)
{
   int i;
   png_color *pal_ptr;
   png_byte buf[3];

   png_write_chunk_start(png_ptr, png_PLTE, number * 3);
   for (i = 0, pal_ptr = palette;
      i < number;
      i++, pal_ptr++)
   {
      buf[0] = pal_ptr->red;
      buf[1] = pal_ptr->green;
      buf[2] = pal_ptr->blue;
      png_write_chunk_data(png_ptr, buf, (png_uint_32)3);
   }
   png_write_chunk_end(png_ptr);
}

/* write an IDAT chunk */
void
png_write_IDAT(png_struct *png_ptr, png_byte *data, png_uint_32 length)
{
   png_write_chunk(png_ptr, png_IDAT, data, length);
}

/* write an IEND chunk */
void
png_write_IEND(png_struct *png_ptr)
{
   png_write_chunk(png_ptr, png_IEND, NULL, (png_uint_32)0);
}

/* write a gAMA chunk */
void
png_write_gAMA(png_struct *png_ptr, float gamma)
{
   png_uint_32 igamma;
   png_byte buf[4];

   /* gamma is saved in 1/100,000ths */
   igamma = (png_uint_32)(gamma * 100000.0 + 0.5);
   png_save_uint_32(buf, igamma);
   png_write_chunk(png_ptr, png_gAMA, buf, (png_uint_32)4);
}

/* write the sBIT chunk */
void
png_write_sBIT(png_struct *png_ptr, png_color_8 *sbit, int color_type)
{
   png_byte buf[4];
   int size;

   /* make sure we don't depend upon the order of png_color_8 */
   if (color_type & PNG_COLOR_MASK_COLOR)
   {
      buf[0] = sbit->red;
      buf[1] = sbit->green;
      buf[2] = sbit->blue;
      size = 3;
   }
   else
   {
      buf[0] = sbit->gray;
      size = 1;
   }

   if (color_type & PNG_COLOR_MASK_ALPHA)
   {
      buf[size++] = sbit->alpha;
   }

   png_write_chunk(png_ptr, png_sBIT, buf, (png_uint_32)size);
}

/* write the cHRM chunk */
void
png_write_cHRM(png_struct *png_ptr, float white_x, float white_y,
   float red_x, float red_y, float green_x, float green_y,
   float blue_x, float blue_y)
{
   png_uint_32 itemp;
   png_byte buf[32];

   /* each value is saved int 1/100,000ths */
   itemp = (png_uint_32)(white_x * 100000.0 + 0.5);
   png_save_uint_32(buf, itemp);
   itemp = (png_uint_32)(white_y * 100000.0 + 0.5);
   png_save_uint_32(buf, itemp);
   itemp = (png_uint_32)(red_x * 100000.0 + 0.5);
   png_save_uint_32(buf, itemp);
   itemp = (png_uint_32)(red_y * 100000.0 + 0.5);
   png_save_uint_32(buf, itemp);
   itemp = (png_uint_32)(green_x * 100000.0 + 0.5);
   png_save_uint_32(buf, itemp);
   itemp = (png_uint_32)(green_y * 100000.0 + 0.5);
   png_save_uint_32(buf, itemp);
   itemp = (png_uint_32)(blue_x * 100000.0 + 0.5);
   png_save_uint_32(buf, itemp);
   itemp = (png_uint_32)(blue_y * 100000.0 + 0.5);
   png_save_uint_32(buf, itemp);
   png_write_chunk(png_ptr, png_cHRM, buf, (png_uint_32)32);
}

/* write the tRNS chunk */
void
png_write_tRNS(png_struct *png_ptr, png_byte *trans, png_color_16 *tran,
   int num_trans, int color_type)
{
   png_byte buf[6];

   if (color_type == PNG_COLOR_TYPE_PALETTE)
   {
      /* write the chunk out as it is */
      png_write_chunk(png_ptr, png_tRNS, trans, (png_uint_32)num_trans);
   }
   else if (color_type == PNG_COLOR_TYPE_GRAY)
   {
      /* one 16 bit value */
      png_save_uint_16(buf, tran->gray);
      png_write_chunk(png_ptr, png_tRNS, buf, (png_uint_32)2);
   }
   else if (color_type == PNG_COLOR_TYPE_RGB)
   {
      /* three 16 bit values */
      png_save_uint_16(buf, tran->red);
      png_save_uint_16(buf + 2, tran->green);
      png_save_uint_16(buf + 4, tran->blue);
      png_write_chunk(png_ptr, png_tRNS, buf, (png_uint_32)6);
   }
}

/* write the background chunk */
void
png_write_bKGD(png_struct *png_ptr, png_color_16 *back, int color_type)
{
   png_byte buf[6];

   if (color_type == PNG_COLOR_TYPE_PALETTE)
   {
      buf[0] = back->index;
      png_write_chunk(png_ptr, png_bKGD, buf, (png_uint_32)1);
   }
   else if (color_type & PNG_COLOR_MASK_COLOR)
   {
      png_save_uint_16(buf, back->red);
      png_save_uint_16(buf + 2, back->green);
      png_save_uint_16(buf + 4, back->blue);
      png_write_chunk(png_ptr, png_bKGD, buf, (png_uint_32)6);
   }
   else
   {
      png_save_uint_16(buf, back->gray);
      png_write_chunk(png_ptr, png_bKGD, buf, (png_uint_32)2);
   }
}

/* write the histogram */
void
png_write_hIST(png_struct *png_ptr, png_uint_16 *hist, int number)
{
   int i;
   png_byte buf[3];

   png_write_chunk_start(png_ptr, png_hIST, (png_uint_32)(number * 2));
   for (i = 0; i < number; i++)
   {
      png_save_uint_16(buf, hist[i]);
      png_write_chunk_data(png_ptr, buf, (png_uint_32)2);
   }
   png_write_chunk_end(png_ptr);
}

/* write a tEXt chunk */
void
png_write_tEXt(png_struct *png_ptr, char *key, char *text,
   png_uint_32 text_len)
{
   int key_len;

   key_len = strlen(key);
   /* make sure we count the 0 after the key */
   png_write_chunk_start(png_ptr, png_tEXt,
      (png_uint_32)(key_len + text_len + 1));
   /* key has an 0 at the end.  How nice */
   png_write_chunk_data(png_ptr, (png_byte *)key, (png_uint_32)(key_len + 1));
   png_write_chunk_data(png_ptr, (png_byte *)text, (png_uint_32)text_len);
   png_write_chunk_end(png_ptr);
}

/* write a compressed chunk */
void
png_write_zTXt(png_struct *png_ptr, char *key, char *text,
   png_uint_32 text_len, int compression)
{
   int key_len;
   char buf[1];
   int i, ret;
   char **output_ptr = NULL; /* array of pointers to output */
   int num_output_ptr = 0; /* number of output pointers used */
   int max_output_ptr = 0; /* size of output_ptr */

   key_len = strlen(key);

   /* we can't write the chunk until we find out how much data we have,
      which means we need to run the compresser first, and save the
      output.  This shouldn't be a problem, as the vast majority of
      comments should be reasonable, but we will set up an array of
      malloced pointers to be sure. */

   /* set up the compression buffers */
   png_ptr->zstream->avail_in = (uInt)text_len;
   png_ptr->zstream->next_in = (Byte *)text;
   png_ptr->zstream->avail_out = (uInt)png_ptr->zbuf_size;
   png_ptr->zstream->next_out = (Byte *)png_ptr->zbuf;

   /* this is the same compression loop as in png_write_row() */
   do
   {
      /* compress the data */
      ret = deflate(png_ptr->zstream, Z_NO_FLUSH);
      if (ret != Z_OK)
      {
         /* error */
         if (png_ptr->zstream->msg)
            png_error(png_ptr, png_ptr->zstream->msg);
         else
            png_error(png_ptr, "zlib error");
      }
      /* check to see if we need more room */
      if (!png_ptr->zstream->avail_out && png_ptr->zstream->avail_in)
      {
         /* make sure the output array has room */
         if (num_output_ptr >= max_output_ptr)
         {
            max_output_ptr = num_output_ptr + 4;
            if (output_ptr)
               output_ptr = png_realloc(png_ptr, output_ptr,
                  max_output_ptr * sizeof (char *));
            else
               output_ptr = png_malloc(png_ptr,
                  max_output_ptr * sizeof (char *));
         }

         /* save the data */
         output_ptr[num_output_ptr] = png_large_malloc(png_ptr,
            png_ptr->zbuf_size);
         memcpy(output_ptr[num_output_ptr], png_ptr->zbuf,
            (png_size_t)png_ptr->zbuf_size);
         num_output_ptr++;

         /* and reset the buffer */
         png_ptr->zstream->avail_out = (uInt)png_ptr->zbuf_size;
         png_ptr->zstream->next_out = png_ptr->zbuf;
      }
   /* continue until we don't have anymore to compress */
   } while (png_ptr->zstream->avail_in);

   /* finish the compression */
   do
   {
      /* tell zlib we are finished */
      ret = deflate(png_ptr->zstream, Z_FINISH);
      if (ret != Z_OK && ret != Z_STREAM_END)
      {
         /* we got an error */
         if (png_ptr->zstream->msg)
            png_error(png_ptr, png_ptr->zstream->msg);
         else
            png_error(png_ptr, "zlib error");
      }

      /* check to see if we need more room */
      if (!png_ptr->zstream->avail_out && ret == Z_OK)
      {
         /* check to make sure our output array has room */
         if (num_output_ptr >= max_output_ptr)
         {
            max_output_ptr = num_output_ptr + 4;
            if (output_ptr)
               output_ptr = png_realloc(png_ptr, output_ptr,
                  max_output_ptr * sizeof (char *));
            else
               output_ptr = png_malloc(png_ptr,
                  max_output_ptr * sizeof (char *));
         }

         /* save off the data */
         output_ptr[num_output_ptr] = png_large_malloc(png_ptr,
            png_ptr->zbuf_size);
         memcpy(output_ptr[num_output_ptr], png_ptr->zbuf,
            (png_size_t)png_ptr->zbuf_size);
         num_output_ptr++;

         /* and reset the buffer pointers */
         png_ptr->zstream->avail_out = (uInt)png_ptr->zbuf_size;
         png_ptr->zstream->next_out = png_ptr->zbuf;
      }
   } while (ret != Z_STREAM_END);

   /* text length is number of buffers plus last buffer */
   text_len = png_ptr->zbuf_size * num_output_ptr;
   if (png_ptr->zstream->avail_out < png_ptr->zbuf_size)
      text_len += (png_uint_32)(png_ptr->zbuf_size -
         png_ptr->zstream->avail_out);

   /* write start of chunk */
   png_write_chunk_start(png_ptr, png_zTXt,
      (png_uint_32)(key_len + text_len + 2));
   /* write key */
   png_write_chunk_data(png_ptr, (png_byte *)key, (png_uint_32)(key_len + 1));
   buf[0] = compression;
   /* write compression */
   png_write_chunk_data(png_ptr, (png_byte *)buf, (png_uint_32)1);

   /* write saved output buffers, if any */
   for (i = 0; i < num_output_ptr; i++)
   {
      png_write_chunk_data(png_ptr, (png_byte *)output_ptr[i], png_ptr->zbuf_size);
      png_large_free(png_ptr, output_ptr[i]);
   }
   if (max_output_ptr)
      png_free(png_ptr, output_ptr);
   /* write anything left in zbuf */
   if (png_ptr->zstream->avail_out < png_ptr->zbuf_size)
      png_write_chunk_data(png_ptr, png_ptr->zbuf,
         png_ptr->zbuf_size - png_ptr->zstream->avail_out);
   /* close the chunk */
   png_write_chunk_end(png_ptr);

   /* reset zlib for another zTXt or the image data */
   deflateReset(png_ptr->zstream);
}

/* write the pHYs chunk */
void
png_write_pHYs(png_struct *png_ptr, png_uint_32 x_pixels_per_unit,
   png_uint_32 y_pixels_per_unit,
   int unit_type)
{
   png_byte buf[9];

   png_save_uint_32(buf, x_pixels_per_unit);
   png_save_uint_32(buf + 4, y_pixels_per_unit);
   buf[8] = unit_type;

   png_write_chunk(png_ptr, png_pHYs, buf, (png_uint_32)9);
}

/* write the oFFs chunk */
void
png_write_oFFs(png_struct *png_ptr, png_uint_32 x_offset,
   png_uint_32 y_offset,
   int unit_type)
{
   png_byte buf[9];

   png_save_uint_32(buf, x_offset);
   png_save_uint_32(buf + 4, y_offset);
   buf[8] = unit_type;

   png_write_chunk(png_ptr, png_oFFs, buf, (png_uint_32)9);
}

/* two time chunks are given.  This chunk assumes you have a gmtime()
   function.  If you don't have that, use the other tIME function */
void
png_write_tIME(png_struct *png_ptr, png_time *mod_time)
{
   png_byte buf[7];

   png_save_uint_16(buf, mod_time->year);
   buf[2] = mod_time->month;
   buf[3] = mod_time->day;
   buf[4] = mod_time->hour;
   buf[5] = mod_time->minute;
   buf[6] = mod_time->second;

   png_write_chunk(png_ptr, png_tIME, buf, (png_uint_32)7);
}

/* initializes the row writing capability of pnglib */
void
png_write_start_row(png_struct *png_ptr)
{
   /* set up row buffer */
   png_ptr->row_buf = (png_byte *)png_large_malloc(png_ptr,
      (((png_uint_32)png_ptr->usr_channels *
      (png_uint_32)png_ptr->usr_bit_depth *
      png_ptr->width) >> 3) + 1);
   /* set up filtering buffers, if filtering */
   if (png_ptr->bit_depth >= 8 && png_ptr->color_type != 3)
   {
      png_ptr->prev_row = (png_byte *)png_large_malloc(png_ptr,
         png_ptr->rowbytes + 1);
      memset(png_ptr->prev_row, 0, (png_size_t)png_ptr->rowbytes + 1);
      png_ptr->save_row = (png_byte *)png_large_malloc(png_ptr,
         png_ptr->rowbytes + 1);
      memset(png_ptr->save_row, 0, (png_size_t)png_ptr->rowbytes + 1);
   }

   /* if interlaced, we need to set up width and height of pass */
   if (png_ptr->interlaced)
   {
      if (!(png_ptr->transformations & PNG_INTERLACE))
      {
         png_ptr->num_rows = (png_ptr->height + png_pass_yinc[0] - 1 -
            png_pass_ystart[0]) / png_pass_yinc[0];
         png_ptr->usr_width = (png_ptr->width +
            png_pass_inc[0] - 1 -
            png_pass_start[0]) /
            png_pass_inc[0];
      }
      else
      {
         png_ptr->num_rows = png_ptr->height;
         png_ptr->usr_width = png_ptr->width;
      }
   }
   else
   {
      png_ptr->num_rows = png_ptr->height;
      png_ptr->usr_width = png_ptr->width;
   }
}

/* Internal use only.   Called when finished processing a row of data */
void
png_write_finish_row(png_struct *png_ptr)
{
   int ret;

   /* next row */
   png_ptr->row_number++;
   /* see if we are done */
   if (png_ptr->row_number < png_ptr->num_rows)
      return;

   /* if interlaced, go to next pass */
   if (png_ptr->interlaced)
   {
      png_ptr->row_number = 0;
      if (png_ptr->transformations & PNG_INTERLACE)
      {
         png_ptr->pass++;
      }
      else
      {
         /* loop until we find a non-zero width or height pass */
         do
         {
            png_ptr->pass++;
            if (png_ptr->pass >= 7)
               break;
            png_ptr->usr_width = (png_ptr->width +
               png_pass_inc[png_ptr->pass] - 1 -
               png_pass_start[png_ptr->pass]) /
               png_pass_inc[png_ptr->pass];
            png_ptr->num_rows = (png_ptr->height +
               png_pass_yinc[png_ptr->pass] - 1 -
               png_pass_ystart[png_ptr->pass]) /
               png_pass_yinc[png_ptr->pass];
         } while (png_ptr->usr_width == 0 || png_ptr->num_rows == 0);

      }

      /* reset filter row */
      if (png_ptr->prev_row)
         memset(png_ptr->prev_row, 0, (png_size_t)png_ptr->rowbytes + 1);
      /* if we have more data to get, go get it */
      if (png_ptr->pass < 7)
         return;
   }

   /* if we get here, we've just written the last row, so we need
      to flush the compressor */
   do
   {
      /* tell the compressor we are done */
      ret = deflate(png_ptr->zstream, Z_FINISH);
      /* check for an error */
      if (ret != Z_OK && ret != Z_STREAM_END)
      {
         if (png_ptr->zstream->msg)
            png_error(png_ptr, png_ptr->zstream->msg);
         else
            png_error(png_ptr, "zlib error");
      }
      /* check to see if we need more room */
      if (!png_ptr->zstream->avail_out && ret == Z_OK)
      {
         png_write_IDAT(png_ptr, png_ptr->zbuf, png_ptr->zbuf_size);
         png_ptr->zstream->next_out = png_ptr->zbuf;
         png_ptr->zstream->avail_out = (uInt)png_ptr->zbuf_size;
      }
   } while (ret != Z_STREAM_END);

   /* write any extra space */
   if (png_ptr->zstream->avail_out < png_ptr->zbuf_size)
   {
      png_write_IDAT(png_ptr, png_ptr->zbuf, png_ptr->zbuf_size -
         png_ptr->zstream->avail_out);
   }

    deflateReset(png_ptr->zstream);
}

/* pick out the correct pixels for the interlace pass.

   The basic idea here is to go through the row with a source
   pointer and a destination pointer (sp and dp), and copy the
   correct pixels for the pass.  As the row gets compacted,
   sp will always be >= dp, so we should never overwrite anything.
   See the default: case for the easiest code to understand.
   */
void
png_do_write_interlace(png_row_info *row_info, png_byte *row, int pass)
{
   /* we don't have to do anything on the last pass (6) */
   if (row && row_info && pass < 6)
   {
      /* each pixel depth is handled seperately */
      switch (row_info->pixel_depth)
      {
         case 1:
         {
            png_byte *sp;
            png_byte *dp;
            int shift;
            int d;
            int value;
            png_uint_32 i;

            dp = row;
            d = 0;
            shift = 7;
            for (i = png_pass_start[pass];
               i < row_info->width;
               i += png_pass_inc[pass])
            {
               sp = row + (png_size_t)(i >> 3);
               value = (int)(*sp >> (7 - (i & 7))) & 0x1;
               d |= (value << shift);

               if (shift == 0)
               {
                  shift = 7;
                  *dp++ = d;
                  d = 0;
               }
               else
                  shift--;

            }
            if (shift != 7)
               *dp = d;
            break;
         }
         case 2:
         {
            png_byte *sp;
            png_byte *dp;
            int shift;
            int d;
            int value;
            png_uint_32 i;

            dp = row;
            shift = 6;
            d = 0;
            for (i = png_pass_start[pass];
               i < row_info->width;
               i += png_pass_inc[pass])
            {
               sp = row + (i >> 2);
               value = (*sp >> ((3 - (i & 3)) << 1)) & 0x3;
               d |= (value << shift);

               if (shift == 0)
               {
                  shift = 6;
                  *dp++ = d;
                  d = 0;
               }
               else
                  shift -= 2;
            }
            if (shift != 6)
                   *dp = d;
            break;
         }
         case 4:
         {
            png_byte *sp;
            png_byte *dp;
            int shift;
            int d;
            int value;
            png_uint_32 i;

            dp = row;
            shift = 4;
            d = 0;
            for (i = png_pass_start[pass];
               i < row_info->width;
               i += png_pass_inc[pass])
            {
               sp = row + (i >> 1);
               value = (*sp >> ((1 - (i & 1)) << 2)) & 0xf;
               d |= (value << shift);

               if (shift == 0)
               {
                  shift = 4;
                  *dp++ = d;
                  d = 0;
               }
               else
                  shift -= 4;
            }
            if (shift != 4)
               *dp = d;
            break;
         }
         default:
         {
            png_byte *sp;
            png_byte *dp;
            png_uint_32 i;
            int pixel_bytes;

            /* start at the beginning */
            dp = row;
            /* find out how many bytes each pixel takes up */
            pixel_bytes = (row_info->pixel_depth >> 3);
            /* loop through the row, only looking at the pixels that
               matter */
            for (i = png_pass_start[pass];
               i < row_info->width;
               i += png_pass_inc[pass])
            {
               /* find out where the original pixel is */
               sp = row + i * pixel_bytes;
               /* move the pixel */
               if (dp != sp)
                  memcpy(dp, sp, pixel_bytes);
               /* next pixel */
               dp += pixel_bytes;
            }
            break;
         }
      }
      /* set new row width */
      row_info->width = (row_info->width +
         png_pass_inc[pass] - 1 -
         png_pass_start[pass]) /
         png_pass_inc[pass];
      row_info->rowbytes = ((row_info->width *
         row_info->pixel_depth + 7) >> 3);

   }
}

/* this filters the row.  Both row and prev_row have space at the
   first byte for the filter byte. */
void
png_write_filter_row(png_row_info *row_info, png_byte *row,
   png_byte *prev_row)
{
   int minf, bpp;
   png_uint_32 i, v;
   png_uint_32 s, mins;
   png_byte *rp, *pp, *cp, *lp;

   /* find out how many bytes offset each pixel is */
   bpp = (row_info->pixel_depth + 7) / 8;
   if (bpp < 1)
      bpp = 1;

   /* the prediction method we use is to find which method provides
      the smallest value when summing the abs of the distances from
      zero using anything >= 128 as negitive numbers. */
   for (i = 0, s = 0, rp = row + 1; i < row_info->rowbytes; i++, rp++)
   {
      v = *rp;
      if (v < 128)
         s += v;
      else
         s += 256 - (png_int_32)v;
   }

   mins = s;
   minf = 0;

   /* check sub filter */
   for (i = 0, s = 0, rp = row + 1, lp = row + 1 - bpp;
      i < row_info->rowbytes; i++, rp++, lp++)
   {
      if (i >= bpp)
         v = (png_byte)(((int)*rp - (int)*lp) & 0xff);
      else
         v = *rp;

      if (v < 128)
         s += v;
      else
         s += 256 - v;
   }

   if (s < mins)
   {
      mins = s;
      minf = 1;
   }

   /* check up filter */
   for (i = 0, s = 0, rp = row + 1, pp = prev_row + 1;
      i < row_info->rowbytes; i++, rp++, pp++)
   {
      v = (png_byte)(((int)*rp - (int)*pp) & 0xff);

      if (v < 128)
         s += v;
      else
         s += 256 - v;
   }

   if (s < mins)
   {
      mins = s;
      minf = 2;
   }

   /* check avg filter */
   for (i = 0, s = 0, rp = row + 1, pp = prev_row + 1, lp = row + 1 - bpp;
      i < row_info->rowbytes; i++, rp++, pp++, lp++)
   {
      if (i >= bpp)
         v = (png_byte)(((int)*rp - (((int)*pp + (int)*lp) / 2)) & 0xff);
      else
         v = (png_byte)(((int)*rp - ((int)*pp / 2)) & 0xff);

      if (v < 128)
         s += v;
      else
         s += 256 - v;
   }

   if (s < mins)
   {
      mins = s;
      minf = 3;
   }

   /* check paeth filter */
   for (i = 0, s = 0, rp = row + 1, pp = prev_row + 1, lp = row + 1 - bpp,
         cp = prev_row + 1 - bpp;
      i < row_info->rowbytes; i++, rp++, pp++, lp++, cp++)
   {
      int a, b, c, pa, pb, pc, p;

      b = *pp;
      if (i >= bpp)
      {
         c = *cp;
         a = *lp;
      }
      else
      {
         a = c = 0;
      }
      p = a + b - c;
      pa = abs(p - a);
      pb = abs(p - b);
      pc = abs(p - c);

      if (pa <= pb && pa <= pc)
         p = a;
      else if (pb <= pc)
         p = b;
      else
         p = c;

      v = (png_byte)(((int)*rp - p) & 0xff);

      if (v < 128)
         s += v;
      else
         s += 256 - v;
   }

   if (s < mins)
   {
      mins = s;
      minf = 4;
   }

   /* set filter byte */
   row[0] = minf;

   /* do filter */
   switch (minf)
   {
      /* sub filter */
      case 1:
         for (i = bpp, rp = row + (png_size_t)row_info->rowbytes,
            lp = row + (png_size_t)row_info->rowbytes - bpp;
            i < row_info->rowbytes; i++, rp--, lp--)
         {
            *rp = (png_byte)(((int)*rp - (int)*lp) & 0xff);
         }
         break;
      /* up filter */
      case 2:
         for (i = 0, rp = row + (png_size_t)row_info->rowbytes,
            pp = prev_row + (png_size_t)row_info->rowbytes;
            i < row_info->rowbytes; i++, rp--, pp--)
         {
            *rp = (png_byte)(((int)*rp - (int)*pp) & 0xff);
         }
         break;
      /* avg filter */
      case 3:
         for (i = row_info->rowbytes,
            rp = row + (png_size_t)row_info->rowbytes,
            pp = prev_row + (png_size_t)row_info->rowbytes,
            lp = row + (png_size_t)row_info->rowbytes - bpp;
            i > bpp; i--, rp--, lp--, pp--)
         {
            *rp = (png_byte)(((int)*rp - (((int)*lp + (int)*pp) /
               2)) & 0xff);
         }
         for (; i > 0; i--, rp--, pp--)
         {
            *rp = (png_byte)(((int)*rp - ((int)*pp / 2)) & 0xff);
         }
         break;
      /* paeth filter */
      case 4:
         for (i = row_info->rowbytes,
            rp = row + (png_size_t)row_info->rowbytes,
            pp = prev_row + (png_size_t)row_info->rowbytes,
            lp = row + (png_size_t)row_info->rowbytes - bpp,
            cp = prev_row + (png_size_t)row_info->rowbytes - bpp;
            i > 0; i--, rp--, lp--, pp--, cp--)
         {
            int a, b, c, pa, pb, pc, p;

            b = *pp;
            if (i > bpp)
            {
               c = *cp;
               a = *lp;
            }
            else
            {
               a = c = 0;
            }
            p = a + b - c;
            pa = abs(p - a);
            pb = abs(p - b);
            pc = abs(p - c);

            if (pa <= pb && pa <= pc)
               p = a;
            else if (pb <= pc)
               p = b;
            else
               p = c;

            *rp = (png_byte)(((int)*rp - p) & 0xff);
         }
         break;
   }
}
