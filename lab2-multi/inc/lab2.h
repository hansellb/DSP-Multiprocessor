// #include "common_objs.h" must be included before this file

/*
 * Load/Read/Get an image from a specific memory address
 */
void get_img( uBYTE *img, uWORD img_size, uDWORD mem_address );

/*
 * Store/Write/Put an image at a specific memory address
 */
void put_img( uBYTE *img, uWORD img_size, uDWORD mem_address );

/*
 * Convert the 24-bit ( 3 bytes per pixel ) color image to a grayscale image 8-bit ( 1 byte per pixel )
 */
void convert_24bitRGB_to_8bitGrayscale( uBYTE *img_24bit_rgb, uWORD img_size, uBYTE *img_8bit_grayscale );

/*
 * Re-sample / Scale down the image by a factor of 2 ( 50% size )
 */
void scale_grayscale_img( uBYTE *img_8bit_grayscale, uBYTE *img_scaled );

/*
 * Edge detection - Sobel operators
 */
void sobel( uBYTE *img_scaled, uBYTE *img_sobel );

/*
 * Show the image in the terminal using ASCIIs
 */
void print_ASCII( uBYTE *img_8bit_grayscale, uBYTE isSobel );
