// C implementations of image processing functions

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "imgproc.h"

// TODO: define your helper functions here
//! Check if a given pixel lies within an ellipse centered
//! in the middle of the image. The ellipse has radii equal
//! to half the width and half the height of the image.
//!
//! @param img pointer to the Image struct containing width
//!            and height information
//! @param row the row coordinate of the pixel to test
//! @param col the column coordinate of the pixel to test
//!
//! @return true if the pixel lies inside or on the ellipse,
//!         false otherwise
bool is_in_ellipse( struct Image *img, int32_t row, int32_t col ) {
  // Handle edge cases
  if (img == NULL || img->width == 0 || img->height == 0) {
    return false;
  }
  int32_t w = img->width;
  int32_t h = img->height;
  int32_t a = w / 2;
  int32_t b = h / 2;
  int32_t x = col - a;
  int32_t y = row - b;
  return (10000 * x * x) / (a * a) + (10000 * y * y) / (b * b) <= 10000;
}

//! Clamp an integer value to the valid grayscale range [0, 255].
//! Any input below 0 will be clamped to 0, and any input above
//! 255 will be clamped to 255.
//!
//! @param value the integer value to clamp
//!
//! @return the clamped value as an unsigned 32-bit integer
uint32_t clamp_gray(int value) {
  if (value < 0) {
    return 0;
  } else if (value > 255) {
    return 255;
  } else {
    return (uint32_t)value;
  }
}

//! Compute the emboss difference between two pixels by comparing
//! their red, green, and blue channels. The difference is taken
//! as the signed difference from the upper-left (ul) pixel to the
//! current (cur) pixel, with ties broken in the order:
//! red > green > blue.
//!
//! @param cur the current pixel (ARGB format, with red in the
//!            highest 8 bits, green in the next 8, and blue in
//!            the next 8)
//! @param ul  the upper-left neighbor pixel in the same format
//!
//! @return the chosen channel difference (signed integer)
int emboss_diff(uint32_t cur, uint32_t ul) {
  int r  = (int)((cur >> 24) & 0xFF);
  int g  = (int)((cur >> 16) & 0xFF);
  int b  = (int)((cur >>  8) & 0xFF);
  int nr = (int)((ul  >> 24) & 0xFF);
  int ng = (int)((ul  >> 16) & 0xFF);
  int nb = (int)((ul  >>  8) & 0xFF);

  int dr = nr - r, dg = ng - g, db = nb - b;
  int absr = dr >= 0 ? dr : -dr;
  int absg = dg >= 0 ? dg : -dg;
  int absb = db >= 0 ? db : -db;

  int diff = dr, maxabs = absr;
  if (absg > maxabs) { diff = dg; maxabs = absg; }
  if (absb > maxabs) { diff = db; }
  return diff;
}


//! Transform the color component values in each input pixel
//! by applying the bitwise complement operation. I.e., each bit
//! in the color component information should be inverted
//! (1 becomes 0, 0 becomes 1.) The alpha value of each pixel should
//! be left unchanged.
//!
//! @param input_img pointer to the input Image
//! @param output_img pointer to the output Image (in which the
//!                   transformed pixels should be stored)
void imgproc_complement( struct Image *input_img, struct Image *output_img ) {
  // TODO: implement
  int total = input_img->width * input_img->height;

  for (int i = 0; i < total; i++) {
    uint32_t pixel = input_img->data[i];

    uint32_t alpha = pixel & 0x000000FF; 
    uint32_t rgb   = pixel & 0xFFFFFF00;
    uint32_t complemented = (~rgb) & 0xFFFFFF00; // Invert only the RGB bits

    output_img->data[i] = complemented | alpha;
  }
}

//! Transform the input image by swapping the row and column
//! of each source pixel when copying it to the output image.
//! E.g., a pixel at row i and column j of the input image
//! should be copied to row j and column i of the output image.
//! Note that this transformation can only be applied to square
//! images (where the width and height are identical.)
//!
//! @param input_img pointer to the input Image
//! @param output_img pointer to the output Image (in which the
//!                   transformed pixels should be stored)
//!
//! @return 1 if the transformation succeeded, or 0 if the
//!         transformation can't be applied because the image
//!         width and height are not the same
int imgproc_transpose( struct Image *input_img, struct Image *output_img ) {
  // TODO: implement
  if (input_img->width != input_img->height) {
    return 0;
  }
  int n = input_img->width; 
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      output_img->data[j * n + i] = input_img->data[i * n + j];
    }
  }
  return 1;
}

//! Transform the input image by copying only those pixels that are
//! within an ellipse centered within the bounds of the image.
//! Pixels not in the ellipse should be left unmodified, which will
//! make them opaque black.
//!
//! Let w represent the width of the image and h represent the
//! height of the image. Let a=floor(w/2) and b=floor(h/2).
//! Consider the pixel at row b and column a is being at the
//! center of the image. When considering whether a specific pixel
//! is in the ellipse, x is the horizontal distance to the center
//! of the image and y is the vertical distance to the center of
//! the image. The pixel at the coordinates described by x and y
//! is in the ellipse if the following inequality is true:
//!
//!   floor( (10000*x*x) / (a*a) ) + floor( (10000*y*y) / (b*b) ) <= 10000
//!
//! @param input_img pointer to the input Image
//! @param output_img pointer to the output Image (in which the
//!                   transformed pixels should be stored)
void imgproc_ellipse( struct Image *input_img, struct Image *output_img ) {
  // TODO: implement
  for (int32_t i = 0; i < input_img->height; i++) {
    for (int32_t j = 0; j < input_img->width; j++) {
      //if in ellipse, copy pixel from input to output
      if (is_in_ellipse(input_img, i, j)) {
        output_img->data[i * input_img->width + j] = input_img->data[i * input_img->width + j];
      }
    }
  }
}

//! Transform the input image using an "emboss" effect. The pixels
//! of the source image are transformed as follows.
//!
//! The top row and left column of pixels are transformed so that their
//! red, green, and blue color component values are all set to 128,
//! and their alpha values are not modified.
//!
//! For all other pixels, we consider the pixel's color component
//! values r, g, and b, and also the pixel's upper-left neighbor's
//! color component values nr, ng, and nb. In comparing the color
//! component values of the pixel and its upper-left neighbor,
//! we consider the differences (nr-r), (ng-g), and (nb-b).
//! Whichever of these differences has the largest absolute value
//! we refer to as diff. (Note that in the case that more than one
//! difference has the same absolute value, the red difference has
//! priority over green and blue, and the green difference has priority
//! over blue.)
//!
//! From the value diff, compute the value gray as 128 + diff.
//! However, gray should be clamped so that it is in the range
//! 0..255. I.e., if it's negative, it should become 0, and if
//! it is greater than 255, it should become 255.
//!
//! For all pixels not in the top or left row, the pixel's red, green,
//! and blue color component values should be set to gray, and the
//! alpha value should be left unmodified.
//!
//! @param input_img pointer to the input Image
//! @param output_img pointer to the output Image (in which the
//!                   transformed pixels should be stored)
void imgproc_emboss( struct Image *input_img, struct Image *output_img ) {
  int w = input_img->width;
  int h = input_img->height;
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      uint32_t p = input_img->data[j * w + i];
      uint32_t a = p & 0xFF; 
      //Top row or left column turn grey
      if (j == 0 || i == 0) {
        output_img->data[j * w + i] =
          (128u << 24) | (128u << 16) | (128u << 8) | a;
        continue;
      }
      // Upper-left neighbor and chosen diff
      uint32_t pn = input_img->data[(j - 1) * w + (i - 1)];
      int diff = emboss_diff(p, pn);
      uint32_t gray = clamp_gray(128 + diff); // gray clamp when extreme
      output_img->data[j * w + i] =
        (gray << 24) | (gray << 16) | (gray << 8) | a;
    }
  }
}
