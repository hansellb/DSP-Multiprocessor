#include "lab2.h"

/*
 * Load/Read/Get an image from a specific memory address
 * - Image will be copied from mem_address to img
 */
void get_img(\
				uBYTE *img,\
				uWORD img_size,\
				uDWORD mem_address\
			)
{
	uWORD i;
	uBYTE	*img_byte_ptr, *shared_mem_byte_ptr;
	uQWORD	*img_ptr, *shared_mem_ptr;

	img_byte_ptr = (uBYTE *)(img);
	shared_mem_byte_ptr = (uBYTE *) mem_address;
	img_ptr = (uQWORD *)(img);
	shared_mem_ptr = (uQWORD *) mem_address;

	for(i = 0; i < img_size >> 3; i++)
	{
		*(img_ptr + i) = *(shared_mem_ptr + i);
	}
	*((uDWORD *)(img_ptr + i)) = *((uDWORD *)(shared_mem_ptr + i));
}





/*
 * Store/Write/Put an image at a specific memory address
 * - Image will be copied from img to mem_address
 */
void put_img(\
				uBYTE *img,\
				uWORD img_size,\
				uDWORD mem_address\
			)
{
	uWORD	i;
	uBYTE	*img_byte_ptr, *shared_mem_byte_ptr;
	uQWORD	*img_ptr, *shared_mem_ptr;

	img_byte_ptr = (uBYTE *)(img);
	shared_mem_byte_ptr = (uBYTE *) mem_address;
	img_ptr = (uQWORD *)(img);
	shared_mem_ptr = (uQWORD *) mem_address;

	//alt_printf("shared_mem_ptr:%x img_ptr:%x *img_byte_ptr:%x *(img_byte_ptr+1):%x img_size:%x \n", shared_mem_ptr, img_ptr, *img_byte_ptr, *(img_byte_ptr+1), img_size );
	for(i = 0; i < img_size >> 3; i++)
	{
		//alt_printf("*(shared_mem_ptr + %x):%x %x *(img_ptr + %x):%x %x\n", i, *(shared_mem_ptr + i), i, *(img_ptr + i));
		*(shared_mem_ptr + i) = *(img_ptr + i);
	}
	//alt_printf("*((uDWORD *)(shared_mem_ptr + %x)):%x *((uDWORD *)(img_ptr + %x)):%x\n", i, *((uDWORD *)(shared_mem_ptr + i)), i, *((uDWORD *)(img_ptr + i)) );
	*((uDWORD *)(shared_mem_ptr + i)) = *((uDWORD *)(img_ptr + i));
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
void convert_24bitRGB_to_8bitGrayscale(\
										uBYTE *img_24bit_rgb,\
										uWORD img_size,\
										uBYTE *img_8bit_grayscale\
									  )
{
	uWORD 	i, j;
	uBYTE	grayscale_pixel;

#if PERFORMANCE
    for( i = 3, j = 3; i <= 3072; i = i + 3, j++ )
#else
	img_8bit_grayscale[0] = img_24bit_rgb[0];
	img_8bit_grayscale[1] = img_24bit_rgb[1];
	img_8bit_grayscale[2] = img_24bit_rgb[2];
	for( i = 3, j = 3; i <= img_size; i = i + 3, j++ )
#endif
    {
    	grayscale_pixel = 	( ((*(img_24bit_rgb + i)) >> 2) + ((*(img_24bit_rgb + i)) >> 4) ) + \
    						( ((*(img_24bit_rgb + i + 1)) >> 1) + ((*(img_24bit_rgb + i + 1)) >> 4) ) + \
    						( ((*(img_24bit_rgb + i + 2)) >> 3) );

        *( img_8bit_grayscale + j ) = grayscale_pixel;
    }
}





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
void scale_grayscale_img(\
							uBYTE *img_8bit_grayscale,\
							uBYTE *img_scaled\
						)
{
	uBYTE	i, j;
	uDWORD	k, l;

#if PERFORMANCE
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
    //img_scaled[2] = 255;
#else
	k = 3;
	l = 0;
	for (i = 0; i < *(img_8bit_grayscale + 1)>>1; i++)
    {
    	for (j = 0; j < *(img_8bit_grayscale)>>1; j++)
        {
    		img_scaled[ k ] = img_8bit_grayscale[ ( l << 1 ) + 3 ];
    		k++;
    		l++;
        }

    	l += *(img_8bit_grayscale)>>1;
    }
	img_scaled[0] = *(img_8bit_grayscale)>>1;
	img_scaled[1] = *(img_8bit_grayscale + 1)>>1;
	//img_scaled[2] = *(img_8bit_grayscale + 2);
#endif
} // void scale_grayscale_img( uBYTE *img_8bit_grayscale, uBYTE *img_scaled )





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
void sobel(uBYTE *img_scaled, uBYTE *img_sobel)
{
	uBYTE	i, j;
	uBYTE	p1, p2, p3, p4, p5, p6, p7, p8, p9;
	uWORD	k;
	DWORD	Gx, Gy;
	uDWORD	abs_temp;

#if PERFORMANCE
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
	//img_sobel[2] = 255;
#else
	//alt_printf( "img_scaled:%x img_sobel:%x \n", img_scaled, img_sobel );
	k=3;
    for( i = 1; i < (*(img_scaled + 1) - 1) ; i++)
    {
        for( j = 1; j < (*(img_scaled) - 1) ; j++)
        {
            p1 = img_scaled[ ( ( i - 1 ) * (*(img_scaled)) ) + ( j - 1 ) + 3 ];
            p2 = img_scaled[ ( ( i - 1 ) * (*(img_scaled)) ) + ( j ) + 3 ];
            p3 = img_scaled[ ( ( i - 1 ) * (*(img_scaled)) ) + ( j + 1 ) + 3 ];
            p4 = img_scaled[ ( ( i ) * (*(img_scaled)) ) + ( j - 1 ) + 3];
            p5 = img_scaled[ ( i * (*(img_scaled)) ) + j + 3 ];
            p6 = img_scaled[ ( ( i ) * (*(img_scaled)) ) + ( j + 1 ) + 3];
            p7 = img_scaled[ ( ( i + 1 ) * (*(img_scaled)) ) + ( j - 1 ) + 3];
            p8 = img_scaled[ ( ( i + 1 ) * (*(img_scaled)) ) + ( j ) + 3];
            p9 = img_scaled[ ( ( i + 1 ) * (*(img_scaled)) ) + ( j + 1 ) + 3];

            Gx = ( p3 + ( p6 + p6 ) + p9 - ( p1 + ( p4 + p4 ) + p7 ) );
            Gy = ( p1 + ( p2 + p2 ) + p3 - ( p7 + ( p8 + p8 ) + p9 ) );

            abs_temp = Gx >> 31;
            Gx ^= abs_temp;
            Gx += abs_temp & 1;

            abs_temp = Gy >> 31;
            Gy ^= abs_temp;
            Gy += abs_temp & 1;

            //img_sobel[ ( i * (*(img_scaled)) ) + j + 3 ] = ( ( Gx + Gy ) >> 3 );
			*(img_sobel + ( ( i * (*(img_scaled)) ) + j + 3 ) ) = ( ( Gx + Gy ) >> 3 );
        }
    }
    *(img_sobel) = *(img_scaled); //img_sobel[0] = *(img_scaled);
	*(img_sobel + 1) = *(img_scaled + 1); //img_sobel[1] = *(img_scaled + 1);
	//img_sobel[2] = 255;
#endif
} // void sobel(uBYTE *img_scaled, uBYTE *img_sobel)




#ifdef CPU_0
/*
 * Show the image in the terminal using ASCIIs
 * 1.- Show the actual hex values ( padded to two hex digits )
 * 2.- Show the image using the ASCII symbols in ascii_symbols
 */
void print_ASCII( uBYTE *img_8bit_grayscale, uBYTE isSobel )
{
	uBYTE i, j, img_width, img_height;
	uWORD k;
	//uBYTE grayscale_levels;

	img_width = img_8bit_grayscale[0];
	img_height = img_8bit_grayscale[1];
#if LOW_LEVEL_DEBUG
// Print the "real" values
    k = 3;
    for( i = 0; i < img_height; i++)
    {
		for( j = 0; j < img_width; j++)
		{
			alt_printf( "%x", *( img_8bit_grayscale + k ) );
			k++;
		}
		alt_printf("\n");
	}
#endif
	if( isSobel )
	{
		for( i = 1; i < ( img_height - 1 ); i++)
		{
			for( j = 1; j < ( img_width - 1 ); j++)
			{
				alt_printf( "%c", ascii_symbols[ ( ( img_8bit_grayscale[ ( i * img_width ) + j + 3 ] ) >> 3 ) ] ); // To be able to use a right shift instead of a division by 8, we use 32 symbols to divide the grayscale, therefore, 255 / 8  we need 31 which is equal to symbols - 1, therefore symbols = 32
			}
			alt_printf("\n");
		}
		alt_printf("\n");
		return;
	}
// Print the ASCII values'
    k = 3;
    for( i = 0; i < img_height; i++)
    {
        for( j = 0; j < img_width; j++)
        {
            //alt_printf( "%c", ascii_symbols[ ( *( img_8bit_grayscale + k ) >> 3 ) ] ); // To be able to use a right shift instead of a division by 8, we use 32 symbols to divide the grayscale, therefore, 255 / 8  we need 31 which is equal to symbols - 1, therefore symbols = 32
			alt_printf( "%c", ascii_symbols[ ( *( img_8bit_grayscale + k ) >> 4 ) ] ); // To be able to use a right shift instead of a division by 16, we use 16 symbols to divide the grayscale, therefore, 255 / 16  we need 15 which is equal to symbols - 1, therefore symbols = 16
            k++;
        }
        alt_printf("\n");
    }
    alt_printf("\n");
} // void print_ascii( uBYTE *img_8bit_grayscale, uBYTE img_width, uBYTE img_height, uBYTE max_grayscale_level )
#endif



