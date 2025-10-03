/*
 * x86-64 assembly language implementations of functions
 */

	.section .text

/* Offsets of struct Image fields */
#define IMAGE_WIDTH_OFFSET   0
#define IMAGE_HEIGHT_OFFSET  4
#define IMAGE_DATA_OFFSET    8

/*
 * TODO: define your helper functions here.
 * Don't forget to use the .globl directive to make
 * them visible to the test code.
 */

/*
 * Check if a given pixel lies within an ellipse centered
 * in the middle of the image. The ellipse has radii equal
 * to half the width and half the height of the image.
 *
 * @param img pointer to the Image struct containing width
 * and height information (passed in %rdi)
 * @param row the row coordinate of the pixel to test (passed in %esi)
 * @param col the column coordinate of the pixel to test (passed in %edx)
 *
 * @return true (1 in %rax) if the pixel lies inside or on the ellipse,
 *         false (0 in %rax) otherwise.
 */
    .globl is_in_ellipse
    /*
     * Register use:
     *   %rdi  - pointer to input Image structure
     *   %edx  - x-coordinate (column index, `j`)
     *   %esi  - y-coordinate (row index, `i`)
     *   %eax  - temporary storage for width and other calculations
     *   %r12d - height of the image
     *   %r13d - a = width / 2 (ellipse horizontal radius)
     *   %r14d - b = height / 2 (ellipse vertical radius)
     *   %r15d - x = col - a (horizontal shift for ellipse center)
     *   %ebx  - y = row - b (vertical shift for ellipse center)
     *   %rcx  - divisor for terms (e.g., a² or b²)
     *   %rax  - intermediate results for computations
     *   %r15  - term1 = (10000 * x²) / a²
     */
is_in_ellipse:
    /* Prologue */
    pushq %rbp
    movq  %rsp, %rbp
    pushq %rbx
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    test  %rdi, %rdi              /* Check if img == NULL */
    je    .Lis_in_ellipse_false   /* If so, return false */

    movl  (%rdi), %eax            /* eax = width (img->width) */
    test  %eax, %eax              /* Check if width == 0 */
    je    .Lis_in_ellipse_false

    movl  4(%rdi), %r12d          /* r12d = height (img->height) */
    test  %r12d, %r12d            /* Check if height == 0 */
    je    .Lis_in_ellipse_false

    movl  %eax, %r13d             /* r13d = width */
    sarl  $1, %r13d               /* r13d = a (width / 2) */
    movl  %r12d, %r14d            /* r14d = height */
    sarl  $1, %r14d               /* r14d = b (height / 2) */

    testl %r13d, %r13d            /* Check if a == 0 */
    je    .Lis_in_ellipse_false
    testl %r14d, %r14d            /* Check if b == 0 */
    je    .Lis_in_ellipse_false

    movl  %edx, %r15d             /* r15d = col (j) */
    subl  %r13d, %r15d            /* r15d = x = col - a */
    movl  %esi, %ebx              /* ebx = row (i) */
    subl  %r14d, %ebx             /* ebx = y = row - b */

    /* term1 = (10000 * x*x) / (a*a) */
    movslq %r15d, %rax            /* Sign-extend x */
    imulq  %rax, %rax             /* rax = x² */
    imulq  $10000, %rax           /* rax = 10000 * x² */
    movslq %r13d, %rcx            /* Sign-extend a */
    imulq  %rcx, %rcx             /* rcx = a² */
    cqto                           /* Sign-extend rax for division */
    idivq  %rcx                   /* rax = (10000 * x²) / a² */
    movq   %rax, %r15             /* Save term1 in r15 */

    /* term2 = (10000 * y*y) / (b*b) */
    movslq %ebx, %rax             /* Sign-extend y */
    imulq  %rax, %rax             /* rax = y² */
    imulq  $10000, %rax           /* rax = 10000 * y² */
    movslq %r14d, %rcx            /* Sign-extend b */
    imulq  %rcx, %rcx             /* rcx = b² */
    cqto                           /* Sign-extend rax for division */
    idivq  %rcx                   /* rax = (10000 * y²) / b² */

    addq   %r15, %rax             /* term1 + term2 */
    cmpq   $10000, %rax           /* Compare sum with 10000 */
    jg     .Lis_in_ellipse_false  /* If sum > 10000, return false */

    movq   $1, %rax               /* Return true */
    jmp    .Lis_in_ellipse_done

.Lis_in_ellipse_false:
    xorq   %rax, %rax             /* Return false (rax = 0) */

.Lis_in_ellipse_done:
    /* Epilogue */
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbx
    popq %rbp
    ret

/*
 * Clamp an integer value to the valid grayscale range [0, 255].
 * Any input below 0 will be clamped to 0, and any input above
 * 255 will be clamped to 255.
 *
 * @param value the integer value to clamp (passed in %esi)
 *
 * @return the clamped value in the range [0, 255]
 */
    .globl clamp_gray
clamp_gray:
    /*
     * Register use:
     *   %edi - parameter: value to clamp (32-bit input argument)
     *   %eax - working register for clamping and return value
     */

    movl %edi, %eax          # Copy input argument into %eax

    cmpl $0, %eax            # Compare with 0
    jl .Lclamp_zero          # Jump if value < 0

    cmpl $255, %eax          # Compare with 255
    jg .Lclamp_255           # Jump if value > 255

    ret                      # Return value already in %eax

.Lclamp_zero:
    movl $0, %eax
    ret

.Lclamp_255:
    movl $255, %eax
    ret


	.globl emboss_diff
emboss_diff:
	/* TODO: implement */
	ret

/*
 * Definitions of image transformation functions
 */

/*
 *  Transform the color component values in each input pixel
 *  by applying the bitwise complement operation. I.e., each bit
 *  in the color component information should be inverted
 *  (1 becomes 0, 0 becomes 1.) The alpha value of each pixel should
 *  be left unchanged.
 * 
 *  Parameters:
 *  %rdi - pointer to the input Image
 *  %rsi - pointer to the output Image (in which the
 *         transformed pixels should be stored)
 */
	.globl imgproc_complement
	/*
	* Register use:
	*   %r12 - pointer to input Image data array
	*   %r13 - pointer to output Image data array  
	*   %r14d - loop index (pixel counter)
	*   %r15d - total number of pixels (width * height)
	*   %ebx - current pixel value being processed
	*   %eax - temporary for width * height calculation
	*   %edx - extracted alpha component (lower 8 bits)
	*/
imgproc_complement:
	# Prologue: save callee-saved registers
    pushq %rbx
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

	# Compute total = input_img->width * input_img->height
	movl   0(%rdi), %eax          # eax = width   (32-bit)
    imull  4(%rdi), %eax          # eax *= height (32-bit)
    movl   %eax, %r15d            # total -> r15d

	# Load points to data
	movq   8(%rdi), %r12          # input_img->data (64-bit)
    movq   8(%rsi), %r13          # output_img->data (64-bit)

	# Initialize loop index = 0
	movl $0, %r14d 

.Lloop:
	cmpl %r15d, %r14d			  # if i >= limit, jump to done
	jge .Ldone 

	movl   (%r12,%r14,4), %ebx    # load pixel from input (32-bit)
	movl   %ebx, %edx
	andl   $0x000000FF, %edx      # alpha
	andl   $0xFFFFFF00, %ebx      # rgb
	notl   %ebx					  # complement rgb
	andl   $0xFFFFFF00, %ebx	  # Ensures only the RGB bits are inverted, alpha is still 0 here
	orl    %edx, %ebx             # combine alpha + RGB
	movl   %ebx, (%r13,%r14,4)    # store pixel to output (32-bit)

	incl   %r14d                  # i++
	jmp    .Lloop				  # loop again

.Ldone:
	# Epilogue: restore callee-saved registers
	popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbx
	ret

/*
 *  Transform the input image by swapping the row and column
 *  of each source pixel when copying it to the output image.
 *  E.g., a pixel at row i and column j of the input image
 *  should be copied to row j and column i of the output image.
 *  Note that this transformation can only be applied to square
 *  images (where the width and height are identical.)
 * 
 *  Parameters:
 *  %rdi - pointer to the input Image
 *  %rsi - pointer to the output Image (in which the
 *                    transformed pixels should be stored)
 * 
 *  @return 1 if the transformation succeeded, or 0 if the
 *          transformation can't be applied because the image
 *          width and height are not the same
 *
 *   Register use:
 *   %rbx  - n = width (=height)
 *   %r12  - input->data pointer
 *   %r13  - output->data pointer
 *   %r14d - loop variable i (row index)
 *   %r15d - loop variable j (col index)
 *   %edx  - temporary pixel value
 */
	.globl imgproc_transpose
imgproc_transpose:
	/* TODO: implement */

    # Prologue: save callee-saved registers
    pushq %rbx
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    # Load width and height from input image
    movl 0(%rdi), %eax        # eax = input width
    movl 4(%rdi), %ebx        # ebx = input height
    cmpl %ebx, %eax           # compare width vs height
    jne .Ltrans_fail          # if not equal then fail

    movl %eax, %ebx           # store n = width in ebx

    # Load pointers to pixel arrays
    movq 8(%rdi), %r12        # r12 = input data
    movq 8(%rsi), %r13        # r13 = output data

    # Initialize outer loop: i = 0
    xorl %r14d, %r14d

.Ltrans_outer_loop:
    cmpl %ebx, %r14d          # if i >= n, exit outer loop
    jge .Ltrans_success

    # Initialize inner loop: j = 0
    xorl %r15d, %r15d

.Ltrans_inner_loop:
    cmpl %ebx, %r15d          # if j >= n, exit inner loop
    jge .Ltrans_next_i

    # Compute in_index = i*n + j
    movl %r14d, %eax          # eax = i
    imull %ebx, %eax          # eax = i * n
    addl %r15d, %eax          # eax = i*n + j
    movl (%r12,%rax,4), %edx  # edx = input->data[in_index]

    # Compute out_index = j*n + i
    movl %r15d, %ecx          # ecx = j
    imull %ebx, %ecx          # ecx = j * n
    addl %r14d, %ecx          # ecx += i
    movl %edx, (%r13,%rcx,4)  # output->data[out_index] = pixel

    incl %r15d                # j++
    jmp .Ltrans_inner_loop    # repeat inner loop

.Ltrans_next_i:
    incl %r14d                # i++
    jmp .Ltrans_outer_loop    # repeat outer loop

.Ltrans_success:
    movl $1, %eax             # return 1 (success)
    jmp .Ltrans_done

.Ltrans_fail:
    movl $0, %eax             # return 0 (fail)

.Ltrans_done:
    # Epilogue: restore callee-saved registers
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbx
    ret


/*
 *  Transform the input image by copying only those pixels that are
 *  within an ellipse centered within the bounds of the image.
 *  Pixels not in the ellipse should be left unmodified, which will
 *  make them opaque black.
 * 
 *  Let w represent the width of the image and h represent the
 *  height of the image. Let a=floor(w/2) and b=floor(h/2).
 *  Consider the pixel at row b and column a is being at the
 *  center of the image. When considering whether a specific pixel
 *  is in the ellipse, x is the horizontal distance to the center
 *  of the image and y is the vertical distance to the center of
 *  the image. The pixel at the coordinates described by x and y
 *  is in the ellipse if the following inequality is true:
 * 
 *    floor( (10000*x*x) / (a*a) ) + floor( (10000*y*y) / (b*b) ) <= 10000
 * 
 *  Parameters:
 *  %rdi - pointer to the input Image
 *  %rsi - pointer to the output Image (in which the
 *                    transformed pixels should be stored)
 */
    .globl imgproc_ellipse
    /*
     * Register use:
     *   %r8d  - input image width
     *   %r9d  - input image height
     *   %r12  - pointer to input image data
     *   %r13  - pointer to output image data
     *   %r14d - row index (`i`)
     *   %r15d - column index (`j`)
     *   %eax  - calculated pixel index (i * width + j)
     *   %edx  - pixel value from input image
     */
imgproc_ellipse:
    /* Prologue */
    pushq %rbx
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    /* Load width and height into stable registers */
    movl 0(%rdi), %r8d        /* r8d = input->width */
    movl 4(%rdi), %r9d        /* r9d = input->height */

    /* Load pointers to pixel arrays */
    movq 8(%rdi), %r12        /* r12 = input->data */
    movq 8(%rsi), %r13        /* r13 = output->data */

    /* Initialize outer loop: i = 0 */
    xorl %r14d, %r14d         /* r14d = i = 0 */

.Lellipse_outer_loop:
    cmpl %r9d, %r14d          /* Compare i with height */
    jge .Lellipse_done        /* If i >= height, exit outer loop */

    /* Initialize inner loop: j = 0 */
    xorl %r15d, %r15d         /* r15d = j = 0 */

.Lellipse_inner_loop:
    cmpl %r8d, %r15d          /* Compare j with width */
    jge .Lellipse_next_i      /* If j >= width, exit inner loop */

    /* Call is_in_ellipse(i, j) */
    movl %r14d, %esi          /* esi = i (row index) */
    movl %r15d, %edx          /* edx = j (column index) */
    call is_in_ellipse
    testq %rax, %rax          /* Test return value of is_in_ellipse */
    je .Lellipse_skip_pixel   /* If not in ellipse, skip pixel */

    /* Compute index = i * width + j */
    movl %r14d, %eax          /* eax = i */
    imull %r8d, %eax          /* eax = i * width */
    addl %r15d, %eax          /* eax = i * width + j */

    /* Copy pixel from input to output */
    movl (%r12,%rax,4), %edx  /* edx = input->data[index] */
    movl %edx, (%r13,%rax,4)  /* output->data[index] = input->data[index] */

.Lellipse_skip_pixel:
    incl %r15d                /* j++ */
    jmp .Lellipse_inner_loop

.Lellipse_next_i:
    incl %r14d                /* i++ */
    jmp .Lellipse_outer_loop

.Lellipse_done:
    /* Epilogue */
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbx
    ret

/*
 *  Transform the input image using an "emboss" effect. The pixels
 *  of the source image are transformed as follows.
 * 
 *  The top row and left column of pixels are transformed so that their
 *  red, green, and blue color component values are all set to 128,
 *  and their alpha values are not modified.
 * 
 *  For all other pixels, we consider the pixel's color component
 *  values r, g, and b, and also the pixel's upper-left neighbor's
 *  color component values nr, ng, and nb. In comparing the color
 *  component values of the pixel and its upper-left neighbor,
 *  we consider the differences (nr-r), (ng-g), and (nb-b).
 *  Whichever of these differences has the largest absolute value
 *  we refer to as diff. (Note that in the case that more than one
 *  difference has the same absolute value, the red difference has
 *  priority over green and blue, and the green difference has priority
 *  over blue.)
 * 
 *  From the value diff, compute the value gray as 128 + diff.
 *  However, gray should be clamped so that it is in the range
 *  0..255. I.e., if it's negative, it should become 0, and if
 *  it is greater than 255, it should become 255.
 * 
 *  For all pixels not in the top or left row, the pixel's red, green,
 *  and blue color component values should be set to gray, and the
 *  alpha value should be left unmodified.
 * 
 *  Parameters:
 *  %rdi - pointer to the input Image
 *  %rsi - pointer to the output Image (in which the
 *                    transformed pixels should be stored)
 */
	.globl imgproc_emboss
imgproc_emboss:
	/* TODO: implement */
	ret

/*
vim:ft=gas:
*/
