// Header for image processing API functions (imgproc_complement, etc.)
// as well as any helper functions they rely on.

#ifndef IMGPROC_H
#define IMGPROC_H

#include "image.h" // for struct Image and related functions

//! Transform the color component values in each input pixel
//! by applying the bitwise complement operation. I.e., each bit
//! in the color component information should be inverted
//! (1 becomes 0, 0 becomes 1.) The alpha value of each pixel should
//! be left unchanged.
//!
//! @param input_img pointer to the input Image
//! @param output_img pointer to the output Image (in which the
//!                   transformed pixels should be stored)
void imgproc_complement( struct Image *input_img, struct Image *output_img );

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
int imgproc_transpose( struct Image *input_img, struct Image *output_img );

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
void imgproc_ellipse( struct Image *input_img, struct Image *output_img );

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
void imgproc_emboss( struct Image *input_img, struct Image *output_img );

// TODO: add prototypes for your helper functions
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
bool is_in_ellipse( struct Image *img, int32_t row, int32_t col );

//! Clamp an integer value to the valid grayscale range [0, 255].
//! Any input below 0 will be clamped to 0, and any input above
//! 255 will be clamped to 255.
//!
//! @param value the integer value to clamp
//!
//! @return the clamped value as an unsigned 32-bit integer
uint32_t clamp_gray(int value);

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
int emboss_diff(uint32_t cur, uint32_t ul);
#endif // IMGPROC_H
