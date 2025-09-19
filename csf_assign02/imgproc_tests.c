#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include "tctest.h"
#include "imgproc.h"

// An expected color identified by a (non-zero) character code.
// Used in the "struct Picture" data type.
struct ExpectedColor {
  char c;
  uint32_t color;
};

// Type representing a "picture" of an expected image.
// Useful for creating a very simple Image to be accessed
// by test functions.
struct Picture {
  struct ExpectedColor colors[40];
  int width, height;
  const char *data;
};

// Some "basic" colors to use in test struct Pictures.
// Note that the ranges '1'-'5', 'A'-'E', and 'P'-'T'
// are (respectively) colors 'r','g','b','c', and 'm'
// with just the red, green, and blue color component values
#define TEST_COLORS \
    { \
      { ' ', 0xFFFFFFFF }, \
      { '_', 0x000000FF }, \
      { 'r', 0xFF0000FF }, \
      { 'g', 0x00FF00FF }, \
      { 'b', 0x0000FFFF }, \
      { 'c', 0x00FFFFFF }, \
      { 'm', 0xFF00FFFF }, \
      { '1', 0xFF0000FF }, \
      { '2', 0x000000FF }, \
      { '3', 0x000000FF }, \
      { '4', 0x000000FF }, \
      { '5', 0xFF0000FF }, \
      { 'A', 0x000000FF }, \
      { 'B', 0x00FF00FF }, \
      { 'C', 0x000000FF }, \
      { 'D', 0x00FF00FF }, \
      { 'E', 0x000000FF }, \
      { 'P', 0x000000FF }, \
      { 'Q', 0x000000FF }, \
      { 'R', 0x0000FFFF }, \
      { 'S', 0x0000FFFF }, \
      { 'T', 0x0000FFFF }, \
    }

// Data type for the test fixture object.
// This contains data (including Image objects) that
// can be accessed by test functions. This is useful
// because multiple test functions can access the same
// data (so you don't need to create/initialize that
// data multiple times in different test functions.)
typedef struct {
  // smiley-face picture
  struct Picture smiley_pic;

  // original smiley-face Image object
  struct Image *smiley;

  // empty Image object to use for output of
  // transformation on smiley-face image
  struct Image *smiley_out;

  // a square image (same width/height) to use as a test
  // for the transpose transformation
  struct Picture sq_test_pic;

  // original square Image object
  struct Image *sq_test;

  // empty image for output of transpose transformation
  struct Image *sq_test_out;
} TestObjs;

// Functions to create and clean up a test fixture object
TestObjs *setup( void );
void cleanup( TestObjs *objs );

// Helper functions used by the test code
struct Image *picture_to_img( const struct Picture *pic );
uint32_t lookup_color(char c, const struct ExpectedColor *colors);
bool images_equal( struct Image *a, struct Image *b );
void destroy_img( struct Image *img );

// Test functions
void test_complement_basic( TestObjs *objs );
void test_transpose_basic( TestObjs *objs );
void test_ellipse_basic( TestObjs *objs );
void test_emboss_basic( TestObjs *objs );
// TODO: add prototypes for additional test functions

void test_emboss_border_gray_small(TestObjs *objs);
void test_emboss_tie_red_priority_2x2(TestObjs *objs);
void test_emboss_clamp_hi_2x2(TestObjs *objs);
void test_emboss_clamp_lo_2x2(TestObjs *objs);
void test_is_in_ellipse_basic(TestObjs *objs);


int main( int argc, char **argv ) {
  // allow the specific test to execute to be specified as the
  // first command line argument
  if ( argc > 1 )
    tctest_testname_to_execute = argv[1];

  TEST_INIT();

  // Run tests.
  // Make sure you add additional TEST() macro invocations
  // for any additional test functions you add.
  TEST( test_complement_basic );
  TEST( test_transpose_basic );
  TEST( test_ellipse_basic );
  TEST( test_emboss_basic );

  TEST( test_emboss_border_gray_small );
  TEST( test_emboss_tie_red_priority_2x2 );
  TEST( test_emboss_clamp_hi_2x2 );
  TEST( test_emboss_clamp_lo_2x2 );


  TEST_FINI();
}

////////////////////////////////////////////////////////////////////////
// Test fixture setup/cleanup functions
////////////////////////////////////////////////////////////////////////

TestObjs *setup( void ) {
  TestObjs *objs = (TestObjs *) malloc( sizeof(TestObjs) );

  struct Picture smiley_pic = {
    TEST_COLORS,
    16, // width
    10, // height
    "    mrrrggbc    "
    "   c        b   "
    "  r   r  b   c  "
    " b            b "
    " b            r "
    " g   b    c   r "
    "  c   ggrb   b  "
    "   m        c   "
    "    gggrrbmc    "
    "                "
  };
  objs->smiley_pic = smiley_pic;
  objs->smiley = picture_to_img( &smiley_pic );

  objs->smiley_out = (struct Image *) malloc( sizeof( struct Image ) );
  img_init( objs->smiley_out, objs->smiley->width, objs->smiley->height );

  struct Picture sq_test_pic = {
    TEST_COLORS,
    12, // width
    12, // height
    "rrrrrr      "
    " ggggg      "
    "  bbbb      "
    "   mmm      "
    "    cc      "
    "     r      "
    "            "
    "            "
    "            "
    "            "
    "            "
    "            "
  };
  objs->sq_test_pic = sq_test_pic;
  objs->sq_test = picture_to_img( &sq_test_pic );
  objs->sq_test_out = (struct Image *) malloc( sizeof( struct Image ) );
  img_init( objs->sq_test_out, objs->sq_test->width, objs->sq_test->height );

  return objs;
}

void cleanup( TestObjs *objs ) {
  destroy_img( objs->smiley );
  destroy_img( objs->smiley_out );
  destroy_img( objs->sq_test );
  destroy_img( objs->sq_test_out );

  free( objs );
}

////////////////////////////////////////////////////////////////////////
// Test code helper functions
////////////////////////////////////////////////////////////////////////

struct Image *picture_to_img( const struct Picture *pic ) {
  struct Image *img;

  img = (struct Image *) malloc( sizeof(struct Image) );
  img_init( img, pic->width, pic->height );

  for ( int i = 0; i < pic->height; ++i ) {
    for ( int j = 0; j < pic->width; ++j ) {
      int index = i * img->width + j;
      uint32_t color = lookup_color( pic->data[index], pic->colors );
      img->data[index] = color;
    }
  }

  return img;
}

uint32_t lookup_color(char c, const struct ExpectedColor *colors) {
  for (int i = 0; ; i++) {
    assert(colors[i].c != 0);
    if (colors[i].c == c) {
      return colors[i].color;
    }
  }
}

// Returns true IFF both Image objects are identical
bool images_equal( struct Image *a, struct Image *b ) {
  if ( a->width != b->width || a->height != b->height )
    return false;

  for ( int i = 0; i < a->height; ++i )
    for ( int j = 0; j < a->width; ++j ) {
      int index = i*a->width + j;
      if ( a->data[index] != b->data[index] )
        return false;
    }

  return true;
}

void destroy_img( struct Image *img ) {
  if ( img != NULL )
    img_cleanup( img );
  free( img );
}

////////////////////////////////////////////////////////////////////////
// Test functions
////////////////////////////////////////////////////////////////////////

void test_complement_basic( TestObjs *objs ) {
  {
    imgproc_complement( objs->smiley, objs->smiley_out );

    int height = objs->sq_test->height;
    int width = objs->sq_test->width;

    for ( int i = 0; i < height; ++i ) {
      for ( int j = 0; j < width; ++j ) {
        int index = i*width + j;
        uint32_t pixel = objs->smiley_out->data[ index ];
        uint32_t expected_color = ~( objs->smiley->data[ index ] ) & 0xFFFFFF00;
        uint32_t expected_alpha = objs->smiley->data[ index ] & 0xFF;
        ASSERT( pixel == (expected_color | expected_alpha ) );
      }
    }
  }

  {
    imgproc_complement( objs->sq_test, objs->sq_test_out );

    int height = objs->sq_test->height;
    int width = objs->sq_test->width;

    for ( int i = 0; i < height; ++i ) {
      for ( int j = 0; j < width; ++j ) {
        int index = i*width + j;
        uint32_t pixel = objs->sq_test_out->data[ index ];
        uint32_t expected_color = ~( objs->sq_test->data[ index ] ) & 0xFFFFFF00;
        uint32_t expected_alpha = objs->sq_test->data[ index ] & 0xFF;
        ASSERT( pixel == (expected_color | expected_alpha ) );
      }
    }
  }
}

void test_transpose_basic( TestObjs *objs ) {
  struct Picture sq_test_transpose_expected_pic = {
    {
      { ' ', 0x000000ff },
      { 'a', 0xff0000ff },
      { 'b', 0xffffffff },
      { 'c', 0x00ff00ff },
      { 'd', 0x0000ffff },
      { 'e', 0xff00ffff },
      { 'f', 0x00ffffff },
    },
    12, // width
    12, // height
    "abbbbbbbbbbb"
    "acbbbbbbbbbb"
    "acdbbbbbbbbb"
    "acdebbbbbbbb"
    "acdefbbbbbbb"
    "acdefabbbbbb"
    "bbbbbbbbbbbb"
    "bbbbbbbbbbbb"
    "bbbbbbbbbbbb"
    "bbbbbbbbbbbb"
    "bbbbbbbbbbbb"
    "bbbbbbbbbbbb"
  };

  struct Image *sq_test_transpose_expected =
    picture_to_img( &sq_test_transpose_expected_pic );

  imgproc_transpose( objs->sq_test, objs->sq_test_out );

  ASSERT( images_equal( objs->sq_test_out, sq_test_transpose_expected ) );

  destroy_img( sq_test_transpose_expected );
}

void test_ellipse_basic( TestObjs *objs ) {
  struct Picture smiley_ellipse_expected_pic = {
    {
      { ' ', 0x000000ff },
      { 'a', 0x00ff00ff },
      { 'b', 0xffffffff },
      { 'c', 0x0000ffff },
      { 'd', 0xff0000ff },
      { 'e', 0x00ffffff },
      { 'f', 0xff00ffff },
    },
    16, // width
    10, // height
    "        a       "
    "    bbbbbbbbc   "
    "  dbbbdbbcbbbeb "
    " cbbbbbbbbbbbbcb"
    " cbbbbbbbbbbbbdb"
    "babbbcbbbbebbbdb"
    " bebbbaadcbbbcbb"
    " bbfbbbbbbbbebbb"
    "  bbaaaddcfebbb "
    "    bbbbbbbbb   "
  };

  struct Image *smiley_ellipse_expected =
    picture_to_img( &smiley_ellipse_expected_pic );

  imgproc_ellipse( objs->smiley, objs->smiley_out );

  ASSERT( images_equal( objs->smiley_out, smiley_ellipse_expected ) );

  destroy_img( smiley_ellipse_expected );
}

void test_emboss_basic( TestObjs *objs ) {
  struct Picture smiley_emboss_expected_pic = {
    {
      { ' ', 0x000000ff },
      { 'a', 0x808080ff },
      { 'b', 0xffffffff },
    },
    16, // width
    10, // height
    "aaaaaaaaaaaaaaaa"
    "aaaba       baaa"
    "aaba abaabaaa aa"
    "aba aaa aa aaaba"
    "ab aaaaaaaaaaab "
    "ab aabaaaabaaab "
    "aa aaa bbba aba "
    "aaa aaa    aba a"
    "aaaabbbbbbbba aa"
    "aaaaa        aaa"
  };

  struct Image *smiley_emboss_expected =
    picture_to_img( &smiley_emboss_expected_pic );

  imgproc_emboss( objs->smiley, objs->smiley_out );

  ASSERT( images_equal( objs->smiley_out, smiley_emboss_expected ) );

  destroy_img( smiley_emboss_expected );
}

void test_emboss_border_gray_small(TestObjs *objs) {
  (void)objs; // unused

  // Input: all identical pixels -> inner diff == 0
  // Expected: entire image gray 128, alpha preserved
  struct Picture in_pic = {
    { { 'p', 0x0A141EAA } },  // R=10,G=20,B=30,A=0xAA
    3, 3,
    "ppp"
    "ppp"
    "ppp"
  };
  struct Picture exp_pic = {
    { { 'a', 0x808080AA } },  // gray 128, A=0xAA
    3, 3,
    "aaa"
    "aaa"
    "aaa"
  };

  struct Image *in  = picture_to_img(&in_pic);
  struct Image *out = (struct Image *)malloc(sizeof(struct Image));
  img_init(out, in->width, in->height);
  struct Image *exp = picture_to_img(&exp_pic);

  imgproc_emboss(in, out);
  ASSERT( images_equal(out, exp) );

  destroy_img(in);
  destroy_img(out);
  destroy_img(exp);
}

//Test helper Functions
//test is_in_ellipse function
void test_is_in_ellipse_basic(TestObjs *objs) {
  (void)objs; // unused - we'll create our own test images
  
  // Test 1: NULL image and edge cases
  ASSERT( !is_in_ellipse(NULL, 0, 0) );
  
  struct Image zero_width_img = {0, 5, NULL};
  ASSERT( !is_in_ellipse(&zero_width_img, 2, 2) );
  
  // Test 2: Create a test image for ellipse calculations
  struct Image test_img;
  img_init(&test_img, 10, 6);  // width=10, height=6, center at (3, 5)
  
  // Center point should always be inside
  ASSERT( is_in_ellipse(&test_img, 3, 5) );
  
  // Points clearly inside the ellipse
  ASSERT( is_in_ellipse(&test_img, 3, 4) );  // one unit left of center
  ASSERT( is_in_ellipse(&test_img, 2, 5) );  // one unit above center
  
  // Points on ellipse boundary
  ASSERT( is_in_ellipse(&test_img, 3, 0) );   // left edge
  ASSERT( is_in_ellipse(&test_img, 0, 5) );   // top edge
  
  // Points clearly outside the ellipse
  ASSERT( !is_in_ellipse(&test_img, 0, 0) );   // top-left corner
  ASSERT( !is_in_ellipse(&test_img, 5, 9) );   // bottom-right corner
  
  // Test 3: Square image (circular ellipse)
  struct Image square_img;
  img_init(&square_img, 8, 8);  // Creates circle with radius 4, center at (4, 4)
  
  ASSERT( is_in_ellipse(&square_img, 4, 4) );  // center
  ASSERT( is_in_ellipse(&square_img, 4, 2) );  // inside (distance = 2 < 4)
  ASSERT( !is_in_ellipse(&square_img, 0, 0) ); // corner (distance > 4)
  
  // Test 4: Very small image
  struct Image tiny_img;
  img_init(&tiny_img, 2, 2);
  
  ASSERT( is_in_ellipse(&tiny_img, 0, 0) );   // all points should be inside
  ASSERT( is_in_ellipse(&tiny_img, 1, 1) );   // for such a small ellipse
  
  // Cleanup
  img_cleanup(&test_img);
  img_cleanup(&square_img);
  img_cleanup(&tiny_img);
}

void test_emboss_tie_red_priority_2x2(TestObjs *objs) {
  (void)objs; // unused

  // (1,1) vs (0,0): dr=+50, dg=-50, db=0 => tie on |50| -> prefer RED
  // gray = 128 + 50 = 178 (0xB2)
  struct Picture in_pic = {
    {
      { 'N', 0x963264AA },  // neighbor at (0,0): (150,50,100,A=AA)
      { 'X', 0x000000AA },  // border filler
      { 'Y', 0x000000AA },  // border filler
      { 'C', 0x646464AA }   // current at (1,1): (100,100,100,A=AA)
    },
    2, 2,
    "NX"
    "YC"
  };
  struct Picture exp_pic = {
    {
      { 'a', 0x808080AA },  // border gray 128
      { 'b', 0xB2B2B2AA }   // 178 gray with A=AA
    },
    2, 2,
    "aa"
    "ab"
  };

  struct Image *in  = picture_to_img(&in_pic);
  struct Image *out = (struct Image *)malloc(sizeof(struct Image));
  img_init(out, in->width, in->height);
  struct Image *exp = picture_to_img(&exp_pic);

  imgproc_emboss(in, out);
  ASSERT( images_equal(out, exp) );

  destroy_img(in);
  destroy_img(out);
  destroy_img(exp);
}

void test_emboss_clamp_hi_2x2(TestObjs *objs) {
  (void)objs; // unused

  // dr = +240 -> 128+240 = 368 -> clamp to 255 (0xFF)
  struct Picture in_pic = {
    {
      { 'N', 0xFA0000AA },  // neighbor: (250,0,0,A=AA)
      { 'X', 0x000000AA },  // border
      { 'Y', 0x000000AA },  // border
      { 'C', 0x0A0000AA }   // current: (10,0,0,A=AA)
    },
    2, 2,
    "NX"
    "YC"
  };
  struct Picture exp_pic = {
    {
      { 'a', 0x808080AA },  // border gray 128
      { 'b', 0xFFFFFFAA }   // gray 255 with A=AA
    },
    2, 2,
    "aa"
    "ab"
  };

  struct Image *in  = picture_to_img(&in_pic);
  struct Image *out = (struct Image *)malloc(sizeof(struct Image));
  img_init(out, in->width, in->height);
  struct Image *exp = picture_to_img(&exp_pic);

  imgproc_emboss(in, out);
  ASSERT( images_equal(out, exp) );

  destroy_img(in);
  destroy_img(out);
  destroy_img(exp);
}

void test_emboss_clamp_lo_2x2(TestObjs *objs) {
  (void)objs; // unused

  // dr = -240 -> 128-240 = -112 -> clamp to 0
  struct Picture in_pic = {
    {
      { 'N', 0x0A0000AA },  // neighbor: (10,0,0,A=AA)
      { 'X', 0x000000AA },  // border
      { 'Y', 0x000000AA },  // border
      { 'C', 0xFA0000AA }   // current: (250,0,0,A=AA)
    },
    2, 2,
    "NX"
    "YC"
  };
  struct Picture exp_pic = {
    {
      { 'a', 0x808080AA },  // border gray 128
      { 'k', 0x000000AA }   // gray 0 with A=AA
    },
    2, 2,
    "aa"
    "ak"
  };

  struct Image *in  = picture_to_img(&in_pic);
  struct Image *out = (struct Image *)malloc(sizeof(struct Image));
  img_init(out, in->width, in->height);
  struct Image *exp = picture_to_img(&exp_pic);

  imgproc_emboss(in, out);
  ASSERT( images_equal(out, exp) );

  destroy_img(in);
  destroy_img(out);
  destroy_img(exp);
}
