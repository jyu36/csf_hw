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

	.globl is_in_ellipse
is_in_ellipse:
	/* TODO: implement */
	ret

	.globl clamp_gray
clamp_gray:
	/* TODO: implement */
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
	cmpl %r15d, %r14d
	jge .Ldone 

	movl   (%r12,%r14,4), %ebx    # load pixel from input (32-bit)
	movl   %ebx, %edx
	andl   $0x000000FF, %edx      # alpha
	andl   $0xFFFFFF00, %ebx      # rgb
	notl   %ebx
	andl   $0xFFFFFF00, %ebx      # complemented RGB
	orl    %edx, %ebx             # combine alpha + RGB

	movl   %ebx, (%r13,%r14,4)    # store pixel to output (32-bit)

	incl   %r14d                  # i++
	jmp    .Lloop

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
imgproc_ellipse:
	/* TODO: implement */
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
