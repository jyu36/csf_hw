// C main function for image processing program

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "imgproc.h"

struct Transformation {
  const char *name;
  int (*apply)( struct Image *input_img, struct Image *output_img, int argc, char **argv );
};

int apply_complement( struct Image *input_img, struct Image *output_img, int argc, char **argv );
int apply_transpose( struct Image *input_img, struct Image *output_img, int argc, char **argv );
int apply_ellipse( struct Image *input_img, struct Image *output_img, int argc, char **argv );
int apply_emboss( struct Image *input_img, struct Image *output_img, int argc, char **argv );

static const struct Transformation s_transformations[] = {
  { "complement", apply_complement },
  { "transpose", apply_transpose },
  { "ellipse", apply_ellipse },
  { "emboss", apply_emboss },
  { NULL, NULL },
};

void usage( const char *progname ) {
  fprintf( stderr, "Error: invalid command-line arguments\n" );
  fprintf( stderr, "Usage: %s <transform> <input img> <output img> [args...]\n", progname );
  exit( 1 );
}

// Make a new empty image.
// If transformation is "rgb", then the new image will
// have width and height twice that of the input image,
// otherwise the output image will be the same dimensions as
// the input image.
struct Image *create_output_img( struct Image *input_img, const char *transformation ) {
  struct Image *out_img;
  int32_t out_w = input_img->width, out_h = input_img->height;

  if ( strcmp( transformation, "rgb" ) == 0 ) {
    out_w *= 2;
    out_h *= 2;
  }

  // Allocate Image object
  out_img = (struct Image *) malloc( sizeof( struct Image ) );
  if ( out_img == NULL )
    return NULL;

  // Set data to NULL for now
  out_img->data = NULL;

  // Attempt to initialize the Image object by calling img_init
  if ( img_init( out_img, out_w, out_h ) != IMG_SUCCESS ) {
    free( out_img );
    return NULL;
  }

  // Success!
  return out_img;
}

// Free memory allocated to given Image object
void cleanup_image( struct Image *img ) {
  if ( img != NULL ) {
    img_cleanup( img );
    free( img );
  }
}

int main( int argc, char **argv ) {
  if ( argc < 4 )
    usage( argv[0] );

  const char *transformation = argv[1];
  const char *input_filename = argv[2];
  const char *output_filename = argv[3];

  // Allocate and read the input image
  struct Image *input_img = (struct Image *) malloc( sizeof( struct Image ) );
  if ( input_img == NULL ) {
    fprintf( stderr, "Error: couldn't allocate input image\n" );
    exit( 1 );
  }
  if ( img_read( input_filename, input_img ) != IMG_SUCCESS ) {
    fprintf( stderr, "Error: couldn't read input image\n" );
    free( input_img );
    return 1;
  }

  // Create output Image object
  struct Image *output_img = create_output_img( input_img, transformation );
  if ( output_img == NULL ) {
    fprintf( stderr, "Error: couldn't create output image object\n" );
    cleanup_image( input_img );
    return 1;
  }

  // find transformation
  const struct Transformation *xform = NULL;
  for ( int i = 0; s_transformations[i].name != NULL; ++i )
    if ( strcmp( s_transformations[i].name, transformation ) == 0 ) {
      xform = &s_transformations[i];
      break;
    }

  int success;

  if ( xform != NULL ) {
    // apply the transformation!
    success = xform->apply( input_img, output_img, argc, argv ) != 0;
  } else {
    fprintf( stderr, "Error: unknown transformation '%s'\n", transformation );
    success = 0;
  }

  if ( success ) {
    // Write output image
    if ( img_write( output_filename, output_img ) != IMG_SUCCESS ) {
      fprintf( stderr, "Error: couldn't write output image\n" );
      success = false;
    }
  }

  cleanup_image( input_img );
  cleanup_image( output_img );

  return success ? 0 : 1;
}

int apply_complement( struct Image *input_img, struct Image *output_img, int argc, char **argv ) {
  (void) argc;
  (void) argv;
  imgproc_complement( input_img, output_img );
  return 1;
}

int apply_transpose( struct Image *input_img, struct Image *output_img, int argc, char **argv ) {
  (void) argc;
  (void) argv;
  int success = imgproc_transpose( input_img,  output_img );
  if ( !success )
    fprintf( stderr, "Error: transpose transformation failed\n" );
  return success;
}

int apply_ellipse( struct Image *input_img, struct Image *output_img, int argc, char **argv ) {
  (void) argc;
  (void) argv;
  imgproc_ellipse( input_img,  output_img );
  return 1;
}

int apply_emboss( struct Image *input_img, struct Image *output_img, int argc, char **argv ) {
  (void) argc;
  (void) argv;
  imgproc_emboss( input_img,  output_img );
  return 1;
}
