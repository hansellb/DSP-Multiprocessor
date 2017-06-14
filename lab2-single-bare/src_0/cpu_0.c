/*
 *
 * Design Constraints
 * The final multiprocessor implementation has to meet the following design constraints in order to pass the lab:
 * - The throughput has to be higher than 420 images / second for 32 x 32 pixel images.
 * or
 * - The throughput has to be higher than 350 images / second for 32 x 32 pixel images and the total code footprint (without the images) should not pass 45 KBytes.
 *
 * Some optimizations
 * 0.3125 * x = (5/16) * x = ((1/4) * x) + ((1/16) * x) = (x >> 2) + (x >> 4)
 * 0.5625 * x = (9/16) * x = ((1/2) * x) + ((1/16) * x) = (x >> 1) + (x >> 4)
 * 0.1250 * x = (1/8)  * x = 							= (x >> 3)
 * 1/3 = 0.333333333333333 ~ 	5/16		= 0.3125
 * 								11/32		= 0.34375
 * 								21/64		= 0.328125
 * 								43/128		= 0.3359375
 * 								85/256		= 0.33203125
 * 								171/512		= 0.333984375
 * 								341/1024	= 0.3330078125
 * 								178956971/536870912
 * 								/1073741824
 */
#include "../inc/common_objs.h"
//#include "../inc/lab2.c"
#include "images.h"

extern void delay (int millisec);





/*
 * Load/Read/Get an image from a specific memory address
 * - Image will be copied from mem_address to img
 */
void get_img(
				uBYTE *img, \
				uDWORD mem_address\
			)
{
#if PERFORMANCE
	uWORD i;
	uQWORD	*img_ptr, *shared_mem_ptr;
	
	img_ptr = (uQWORD *)(img);
	shared_mem_ptr = (uQWORD *) mem_address;

	for(i = 0; i < 384; i++)
	{
		*(img_ptr + i) = *(shared_mem_ptr + i);
	}
	*((uDWORD *)(img_ptr + i)) = *((uDWORD *)(shared_mem_ptr + i));
#else
	uWORD i;
	uBYTE	*shared_mem_ptr;

	shared_mem_ptr = (uBYTE *) mem_address; // shared_mem_ptr points to the address of the image to copy

	img[0] = *(shared_mem_ptr + 0); // width
	img[1] = *(shared_mem_ptr + 1); // height
	img[2] = *(shared_mem_ptr + 2); // max colors
	for(i = 3; i < ((img[0] * img[1] * 3) + 3); i++) // Copy ALL pixels (Offset (i) is 3 because the first three bytes are width , height and max colors)
	{
		img[i] = *(shared_mem_ptr + i); // Copy each byte. Don't care if it is R, G or B
	}
#endif
}





/*
 * Store/Write/Put an image at a specific memory address
 * - Image will be copied from img to mem_address
 */
void put_img(
				uBYTE *img, \
				uDWORD mem_address\
			)
{
#if PERFORMANCE
	uWORD i;
	uQWORD	*img_ptr, *shared_mem_ptr;
	
	img_ptr = (uQWORD *)(img);
	shared_mem_ptr = (uQWORD *) mem_address;

	for(i = 0; i < 384; i++)
	{
		*(shared_mem_ptr + i) = *(img_ptr + i);
	}
	*((uDWORD *)(shared_mem_ptr + i)) = *((uDWORD *)(img_ptr + i));
#else
	uWORD	i;
	uBYTE	*shared_mem_ptr;

	shared_mem_ptr = (uBYTE *) mem_address;

	*(shared_mem_ptr + 0) = img[0];
	*(shared_mem_ptr + 1) = img[1];
	*(shared_mem_ptr + 2) = img[2];

	for(i = 3; i < ((img[0] * img[1] * 3) + 3); i++)
	{
		*(shared_mem_ptr + i) = img[i];
	}
#endif
}





/*
 * Convert the 24-bit ( 3 bytes per pixel ) color image to a grayscale image
 * - img_24bit_rgb is the INPUT image - 32 x 32 x 3
 * - img_width - Input image's WIDTH
 * - img_height - Input image's HEIGHT
 * - img_8bit_grayscale is the OUTPUT image - 32 x 32 x 1
 * - Perform the average of the three colors. Pixel values are:
 * 		RED: (*(img_24bit_rgb + i))
 *		GREEN: (*(img_24bit_rgb + i + 1))
 *		BLUE: (*(img_24bit_rgb + i + 2))
 * - To scan INPUT image and create the OUTPUT image, a for loop is used with i & j variables:
 *		i increments by 3 and is used to scan the INPUT image
 *		j increments by 1 and is used to create the OUTPUT image
 */
void convert_24bitRGB_to_8bitGrayscale(
										uBYTE *img_24bit_rgb, \
										uBYTE img_width, \ 
										uBYTE img_height, \
										uBYTE *img_8bit_grayscale\
									  )

{
	uWORD 	i, j;
	uBYTE	grayscale_pixel;
	
	*( img_8bit_grayscale ) = img_width;
	*( img_8bit_grayscale + 1 ) = img_height;
	*( img_8bit_grayscale + 2 ) = *( img_24bit_rgb + 2 );
	
#if PERFORMANCE
	for( i = 3, j = 3; i <= 3072; i = i + 3, j++ )
	{
    	grayscale_pixel = 	( ((*(img_24bit_rgb + i)) >> 2) + ((*(img_24bit_rgb + i)) >> 4) ) + \
    						( ((*(img_24bit_rgb + i + 1)) >> 1) + ((*(img_24bit_rgb + i + 1)) >> 4) ) + \
    						( ((*(img_24bit_rgb + i + 2)) >> 3) );

        *( img_8bit_grayscale + j ) = grayscale_pixel;
    }
#else
    for( i = 3, j = 3; i <= ( img_width * img_height * 3 ); i = i + 3, j++ )
    {

		grayscale_pixel = ((0.3125f*(*(img_24bit_rgb + i))) + (0.5625f*(*(img_24bit_rgb + i + 1))) + (0.125f*(*(img_24bit_rgb + i + 2))));    // Luminosity method KTH
        *( img_8bit_grayscale + j ) = grayscale_pixel;	// Since R, G and B values are the same, we can use an 8-bit image to store our grayscale-converted original image
    }
#endif
} // void convert_24bitRGB_to_8bitGrayscale( uBYTE *img_24bit_rgb, uBYTE img_width, uBYTE img_height, uBYTE *img_8bit_grayscale )






/*
 * Re-sample / Scale down the image
 * - The scale down algorithm uses the closest three neighbors of a pixel that has not been taken into account before.
 * - Example image matrix with image 32 x 32 x 1 = 1024 bytes or pixels
 * y/x	  0	  1		2	3		 30   31
 * 	0	p000 p001 p002 p003 ... p030 p031
 * 	1	p032 p033 p034 p035 ... p062 p063
 * 	2	p064 p065 p066 p067 ... p094 p095
 * 	3	p096 p097 p098 p099 ... p126 p127
 * 	4	p128 p129 p130 p131 ... p158 p159
 * 	5	p160 p161 p162 p163 ... p190 p191
 * 		...
 *	30	p960 p961 p962 p963 ... p990 p991
 *	31	p992 p993 p994 p995 ... p1022 p1023
 *
 * Following the algorithm we want the following:
 * new y/new x		0				1				15
 *		0		p000 p001		p002 p003	...	p030 p031
 *				p032 p033		p034 p035	...	p062 p063
 *
 *		1		p064 p065		p066 p067	... p094 p095
 *				p096 p097		p098 p099	... p126 p127
 *
 *		2		p128 p129		p130 p131	...	p158 p159
 *				p160 p161		p162 p163	...	p190 p191
 *				...
 *		15		p960 p961		p962 p963	...	p0990 p0991
 *				p992 p993		p994 p995	...	p1022 p1023
 *
 * From the previous two "matrices" we can see that in order to scan the original image & create the scaled down image at the same time, we need to use a for loop with x & y equal to the original image's x & y and increment them by two every time. Assigning the value to the correct position in the new image will require only once "scanning" variable that starts at zero and increments by one (Or we can take the x & y values that are incremented by two every time and divide them by two to get the correct position in the scaled down image). 
 */
void scale_grayscale_img(
							uBYTE *img_8bit_grayscale, \
							uBYTE img_width, \
							uBYTE img_height, \
							uBYTE new_width, \
							uBYTE new_height, \
							uBYTE *img_scaled\
						)
{
#if PERFORMANCE
	uBYTE	i, j;
	uDWORD	k, l;

	k = 3;
	l = 0;
	for (i = 0; i < 16; i++)
    {
    	for (j = 0; j < 16; j++)
        {
    		img_scaled[ k ] = img_8bit_grayscale[ ( l << 1 ) + 3 ];
    		k++;
    		l++;
        }

    	l += 16;
    }
	img_scaled[0] = 16;
	img_scaled[1] = 16;
    img_scaled[2] = 255;
#else
	uBYTE	i, j, x;
	uDWORD	k;

	k = 3;
	for (i = 0; i < img_height; i+=2)
    {
    	for (j = 0; j < img_width; j+=2)
        {
    		img_scaled[ k ] = ( \
								img_8bit_grayscale[ (i * img_width) + j + 3 ] + \
								img_8bit_grayscale[ (i * img_width) + (j + 1) + 3 ] + \
								img_8bit_grayscale[ ((i + 1) * img_width) + j + 3 ] + \
								img_8bit_grayscale[ ((i + 1) * img_width) + (j + 1) + 3 ] \
							  ) / 4;
    		k++;
        }

    }
	img_scaled[0] = new_width;
	img_scaled[1] = new_height;
    img_scaled[2] = 255;
#endif
} // void scale_grayscale_img( uBYTE *img_8bit_grayscale, uBYTE img_width, uBYTE img_height, uBYTE *img_scaled )





/*
 * Square root - https://www.codeproject.com/Articles/69941/Best-Square-Root-Method-Algorithm-Function-Precisi
 * - Based on the Fast inverse square root implementation used in Quake 3 game
 */
#define SQRT_MAGIC_F 0x5f3759df 
float  sqrt2(const float x)
{
	const float xhalf = 0.5f*x;
 
	union // get bits for floating value
	{
		float x;
		int i;
	}u;
	u.x = x;
	u.i = SQRT_MAGIC_F - (u.i >> 1);  // gives initial guess y0
	return x*u.x*(1.5f - xhalf*u.x*u.x);// Newton step, repeating increases accuracy 
}





/*
 * Edge detection - Sobel operators
 *
 *        Horizontal            Vertical            Image "window"/section
 *          -1 0 1               1  2  1                         p1 p2 p3
 *        [ -2 0 2 ]           [ 0  0  0 ]      pixel_matrix = [ p4 p5 p6 ]
 *          -1 0 1              -1 -2 -1                         p7 p8 p9
 *
 *  Gx = p3 + 2*p6 + p9 - ( p1 + 2*p4 + p7 ) = p3 + ( p6 + p6 ) + p9 - ( p1 + ( p4 + p4 ) + p7 );
 *  Gy = p1 + 2*p2 + p3 - ( p7 + 2*p8 + p9 ) = p1 + ( p2 + p2 ) + p3 - ( p7 + ( p8 + p8 ) + p9 );
 */
void sobel(uBYTE *img_scaled, uBYTE width, uBYTE height, uBYTE *img_sobel)
//void sobel(uBYTE *img_scaled, uBYTE *img_sobel)
{
#if PERFORMANCE
	uBYTE	i, j;
	uBYTE	p1, p2, p3, p4, p5, p6, p7, p8, p9;
	uWORD	k;
	DWORD	Gx, Gy;
	uDWORD	abs_temp;

	k=3;
    for( i = 1; i < 15 ; i++)
    {
        for( j = 1; j < 15 ; j++)
        {
            p1 = img_scaled[ ( ( i - 1 ) << 4 ) + ( j - 1 ) + 3 ];
            p2 = img_scaled[ ( ( i - 1 ) << 4 ) + ( j ) + 3 ];
            p3 = img_scaled[ ( ( i - 1 ) << 4 ) + ( j + 1 ) + 3 ];
            p4 = img_scaled[ ( ( i ) << 4 ) + ( j - 1 ) + 3];
            p5 = img_scaled[ ( i << 4 ) + j + 3 ];
            p6 = img_scaled[ ( ( i ) << 4 ) + ( j + 1 ) + 3];
            p7 = img_scaled[ ( ( i + 1 ) << 4 ) + ( j - 1 ) + 3];
            p8 = img_scaled[ ( ( i + 1 ) << 4 ) + ( j ) + 3];
            p9 = img_scaled[ ( ( i + 1 ) << 4 ) + ( j + 1 ) + 3];

            Gx = ( p3 + ( p6 + p6 ) + p9 - ( p1 + ( p4 + p4 ) + p7 ) );
            Gy = ( p1 + ( p2 + p2 ) + p3 - ( p7 + ( p8 + p8 ) + p9 ) );

            abs_temp = Gx >> 31;
            Gx ^= abs_temp;
            Gx += abs_temp & 1;

            abs_temp = Gy >> 31;
            Gy ^= abs_temp;
            Gy += abs_temp & 1;

            img_sobel[ ( i << 4 ) + j + 3 ] = ( ( Gx + Gy ) >> 3 );
        }
    }
    img_sobel[0] = 16;
	img_sobel[1] = 16;
	img_sobel[2] = 255;
#else
	uBYTE	i, j;
	//uBYTE	width, height;
	uBYTE	p1, p2, p3, p4, p5, p6, p7, p8, p9;
	uWORD	k;
	DWORD	Gx, Gy, Gsobel;
	uDWORD	abs_temp;

	k=3;
    for( i = 1; i < ( height - 1 ) ; i++)
    {
        for( j = 1; j < ( width - 1 ) ; j++)
        {
        	// Use width because we are scanning all pixels in a row before changing to the next row
            p1 = img_scaled[ ( ( i - 1 ) * width ) + ( j - 1 ) + 3 ];
            p2 = img_scaled[ ( ( i - 1 ) * width ) + ( j ) + 3 ];
            p3 = img_scaled[ ( ( i - 1 ) * width ) + ( j + 1 ) + 3 ];
            p4 = img_scaled[ ( ( i ) * width ) + ( j - 1 ) + 3];
            p5 = img_scaled[ ( i * width ) + j + 3 ];
            p6 = img_scaled[ ( ( i ) * width ) + ( j + 1 ) + 3];
            p7 = img_scaled[ ( ( i + 1 ) * width ) + ( j - 1 ) + 3];
            p8 = img_scaled[ ( ( i + 1 ) * width ) + ( j ) + 3];
            p9 = img_scaled[ ( ( i + 1 ) * width ) + ( j + 1 ) + 3];

            Gx = ( p3 + ( 2 * p6 ) + p9 - ( p1 + ( 2 * p4 ) + p7 ) ) / 9; // Divide by 9 to normalize values
            Gy = ( p1 + ( 2 * p2 ) + p3 - ( p7 + ( 2 * p8 ) + p9 ) ) / 9; // Divide by 9 to normalize values

			Gsobel = sqrt2((Gx * Gx) + (Gy * Gy));
			
            img_sobel[ ( i * width ) + j + 3 ] = (uBYTE)Gsobel;
        } // for( j = 1; j < ( width - 1 ) ; j++)
    } // for( i = 1; i < ( height - 1 ) ; i++)
    img_sobel[0] = width;
	img_sobel[1] = height;
	img_sobel[2] = 255;
#endif
} // void sobel(uBYTE *img_scaled, uBYTE *img_sobel)





/*
 * Show the image in the terminal using ASCIIs
 * 1.- Show the actual hex values ( padded to two hex digits )
 * 2.- Show the image using the ASCII symbols in ascii_symbols
 */
void print_ASCII( uBYTE *img_8bit_grayscale)
{
	uBYTE i, j, img_width, img_height;
	uWORD k;
	//uBYTE grayscale_levels;

	uBYTE ascii_symbols [] = {  ' ', '.', '\'', '`', '^', '"', ',', ':', ';', \
								'-', '-', '-', '~', '~', \
	                            '~', '=', '=', '=', '+', 'o', \
	                            'x', 'x', 'O', 'c', 'o', '0', \
	                            '#', '#', '&', '8', '%', '@' \
							 }; // 32 grayscale symbols. Blank space represents complete black!

	img_width = img_8bit_grayscale[0];
	img_height = img_8bit_grayscale[1];
// Print the ASCII values'
    k = 3;
    for( i = 0; i < img_height; i++)
    {
        for( j = 0; j < img_width; j++)
        {
            alt_printf( "%c", ascii_symbols[ ( *( img_8bit_grayscale + k ) / 8 ) ] );
            k++;
        }
        alt_printf("\n");
    }
    alt_putstr("\n");
} // void print_ascii( uBYTE *img_8bit_grayscale, uBYTE img_width, uBYTE img_height, uBYTE max_grayscale_level )





/*
 * Main
 */
int main()
{
// Variable's declarations
	uWORD			i; //, j, k;	//general loop variable
	uBYTE			executions, print;	//general purpose variables
	uBYTE			*images[11];

	// Expected maximum image arrays
#if PERFORMANCE
	uBYTE			img_24bit_rgb[ (32 * 32 * 3) + 3 ];
	uBYTE			img_8bit_grayscale[ ( 32 * 32 * 1 ) + 3 ];
	uBYTE			img_scaled[ ( 16 * 16 * 1 ) + 3 ];
	uBYTE			img_sobel[ ( 16 * 16 * 1 ) + 3 ];
#else
	uBYTE			img_24bit_rgb[ (40 * 40 * 3) + 3 ];
	uBYTE			img_8bit_grayscale[ ( 40 * 40 * 1 ) + 3 ];
	uBYTE			img_scaled[ ( 20 * 20 * 1 ) + 3 ];
	uBYTE			img_sobel[ ( 40 * 40 * 1 ) + 3 ];
#endif

// Variable's initializations
	alt_printf("Initializing...\n");
	for(i = 0; i < sizeof(img_sobel); i++)
	{
		img_sobel[i] = 0;
	}
	// The first three are used for performance measurements
#if PERFORMANCE
	images[ 0] = img1_32_32;
	images[ 1] = img2_32_32;
	images[ 2] = img_3;
	images[ 3] = img_4;
	images[ 4] = img_5;
	images[ 5] = img_6;
	images[ 6] = img_7;
	images[ 7] = img_8;
	images[ 8] = img_9;
	images[ 9] = img_10;
#else
	images[ 0] = img1_24_24;
	images[ 1] = img1_32_22;
	images[ 2] = img1_32_32;
	images[ 3] = img1_40_28;
	images[ 4] = img1_40_40;
	images[ 5] = img2_24_24;
	images[ 6] = img2_32_22;
	images[ 7] = img2_32_32;
	images[ 8] = img2_40_28;
	images[ 9] = img2_40_40;
	images[10] = img3_32_32;
	
	//images[ 0] = img1_32_32;
	//images[ 1] = img2_32_32;
	//images[ 2] = img_3;
	//images[ 3] = img_4;
	//images[ 4] = img_5;
	//images[ 5] = img_6;
	//images[ 6] = img_7;
	//images[ 7] = img_8;
	//images[ 8] = img_9;
	//images[ 9] = img_10;
#endif

#if PERFORMANCE
	executions = 42; //42 for 420 images or 35 for 350 images;
	print = 1;
	// Performance counter initialization
	// Reset Performance Counter
	PERF_RESET(PERFORMANCE_COUNTER_0_BASE);  

	// Start Measuring
	PERF_START_MEASURING (PERFORMANCE_COUNTER_0_BASE);

	// Section 1
	PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, SECTION_1);
#else
	executions = 1;
	print = 0;
#endif

/*
 * Infinite loop
 */
	while( TRUE )
	{
		while( executions )
		{
#if PERFORMANCE
			for( i = 0; i < 10; i++)
#else
			for( i = 0; i < 11; i++)
#endif
			{
#if PERFORMANCE
#else
				// Performance counter initialization
				// Reset Performance Counter
				PERF_RESET(PERFORMANCE_COUNTER_0_BASE);  

				// Start Measuring
				PERF_START_MEASURING (PERFORMANCE_COUNTER_0_BASE);

				// Section 1
				PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, SECTION_1);
#endif
				//// Section 2
				//PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, SECTION_2);
				convert_24bitRGB_to_8bitGrayscale( images[i], *images[i], *(images[i]+1), img_8bit_grayscale );
				//convert_24bitRGB_to_8bitGrayscale( img_24bit_rgb, img_24bit_rgb[0], img_24bit_rgb[1], img_8bit_grayscale );
				//convert_24bitRGB_to_8bitGrayscale( \
													(uBYTE *)IMG_1_ADDR, \
													*((uBYTE *)IMG_1_ADDR), \
													*((uBYTE *)(IMG_1_ADDR + 1)), \
													(uBYTE *)IMG_1_GRAYSCALE_ADDR \
												 );
				//PERF_END(PERFORMANCE_COUNTER_0_BASE, SECTION_2);
				
				//// Section 3
				//PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, SECTION_3);
				scale_grayscale_img( \
										img_8bit_grayscale, \
										img_8bit_grayscale[0], \
										img_8bit_grayscale[1], \
										( img_8bit_grayscale[0] >> 1 ), \
										( img_8bit_grayscale[1] >> 1 ), \
										img_scaled \
								   );
				//scale_grayscale_img( \
										(uBYTE *)(IMG_1_GRAYSCALE_ADDR), \
										*((uBYTE *)(IMG_1_GRAYSCALE_ADDR)), \
										*((uBYTE *)(IMG_1_GRAYSCALE_ADDR + 1)), \
										(*((uBYTE *)(IMG_1_GRAYSCALE_ADDR)) >> 1), \
										(*((uBYTE *)(IMG_1_GRAYSCALE_ADDR + 1)) >> 1), \
										(uBYTE *)(IMG_1_SCALED_ADDR) \
								   );
				//PERF_END(PERFORMANCE_COUNTER_0_BASE, SECTION_3);
				
				//// Section 4
				//PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, SECTION_4);
				sobel( img_scaled, *( img_scaled ), *( img_scaled + 1 ), img_sobel );
				//sobel( \
						(uBYTE *)(IMG_1_SCALED_ADDR), \
						*((uBYTE *)(IMG_1_SCALED_ADDR)), \
						*((uBYTE *)(IMG_1_SCALED_ADDR + 1)), \
						(uBYTE *)(IMG_1_SOBEL_ADDR) \
					 );
				//sobel( img_8bit_grayscale, *( img_8bit_grayscale ), *( img_8bit_grayscale + 1 ), img_sobel );
				//PERF_END(PERFORMANCE_COUNTER_0_BASE, SECTION_4);
#if PERFORMANCE
#else
				PERF_END(PERFORMANCE_COUNTER_0_BASE, SECTION_1);
			
				// End Measuring
				PERF_STOP_MEASURING(PERFORMANCE_COUNTER_0_BASE);

				// Print measurement report
				perf_print_formatted_report( (void *) PERFORMANCE_COUNTER_0_BASE,
				ALT_CPU_FREQ,                 // defined in "system.h"
				4,                            // How many sections to print
				"Execution time", "RGB->Grayscale", "Resize", "Filter" );    // Display-name of section(s).
				
				print_ASCII(img_8bit_grayscale);
				print_ASCII(img_scaled);
				print_ASCII(img_sobel);
				//print_ASCII((uBYTE *)(IMG_1_SOBEL_ADDR), 1);
				//delay(2000);
#endif				
			} // for( i = 0; i < 11; i++)
			--executions;
		} // while( executions )

		if (print)
		{
			PERF_END(PERFORMANCE_COUNTER_0_BASE, SECTION_1);
			print = 0;

			////// Section 5
			////PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, SECTION_5);
			//print_ASCII((uBYTE *)(IMG_1_GRAYSCALE_ADDR));
			//print_ASCII((uBYTE *)(IMG_1_SCALED_ADDR));
			//print_ASCII((uBYTE *)(IMG_1_SOBEL_ADDR));
			//print_ASCII((uBYTE *)(IMG_1_SOBEL_ADDR));

			//print_ASCII(img_8bit_grayscale);
			//print_ASCII(img_scaled);
			//print_ASCII(img_sobel);
			//print_ASCII(img_sobel);
			//PERF_END(PERFORMANCE_COUNTER_0_BASE, SECTION_5);

			// End Measuring
			PERF_STOP_MEASURING(PERFORMANCE_COUNTER_0_BASE);

			// Print measurement report
			perf_print_formatted_report( (void *) PERFORMANCE_COUNTER_0_BASE,
				ALT_CPU_FREQ,                 // defined in "system.h"
				//4,                            // How many sections to print
				1,                            // How many sections to print
				//"Execution time", "RGB->Grayscale", "Resize", "Filter" );    // Display-name of section(s).
				"Execution time" );    // Display-name of section(s).
			
			print_ASCII(img_8bit_grayscale);
			print_ASCII(img_scaled);
			print_ASCII(img_sobel);
		}
	} //End of while (TRUE)	//Infinite loop
	
	return 0;
}
