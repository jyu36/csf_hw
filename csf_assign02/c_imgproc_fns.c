// C implementations of image processing functions

#include <stdlib.h>
#include <assert.h>
#include "imgproc.h"

// TODO: define your helper functions here
// Clamp function to ensure value stays in between min and max, inclusive
double clamp (double value, double min, double max) {
  if (value < min) {
      return min;
  } else if (value > max) {
      return max;
  } else {
      return value;
  }
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
  int w = input_img->width;
  int h = input_img->height;
  int a = w / 2;
  int b = h / 2;

  for (int i = 0; i < w; i++) {
    for (int j = 0; j < h; j++) {
      int x = i - a;
      int y = j - b;
      //if in ellipse, copy pixel from input to output
      if ((10000 * x * x) / (a * a) + (10000 * y * y) / (b * b) <= 10000) {
        output_img->data[j * w + i] = input_img->data[j * w + i];
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
  // TODO: implement
}
