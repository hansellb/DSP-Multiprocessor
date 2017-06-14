/*
 *
 */
#include "../inc/common_objs.h"
#include "../inc/lab2.c"



/*
 * Main
 */
int main()
{
// Variable's declarations
	uDWORD			fifo_element;

	// Expected maximum image arrays
#if PERFORMANCE
	uBYTE			img1[ (32 * 32 * 3) + 3 ] __attribute__ (( aligned(8) ));
	uBYTE			img2[ ( 16 * 16 * 1 ) + 3 ] __attribute__ (( aligned(8) ));
#else
	uBYTE			img1[ (40 * 40 * 3) + 3 ] __attribute__ (( aligned(8) ));
#endif

#if PERFORMANCE
	alt_mutex_dev	*mutex_0, *mutex_1;	// Handle to the mutex hardware
#endif

	//alt_printf( "***** %s is initializing... *****\n", ALT_CPU_NAME );

// Variable's initializations
#if PERFORMANCE
	mutex_0 = altera_avalon_mutex_open(MUTEX_0_NAME);	// Get the handle to the hardware mutex
	mutex_1 = altera_avalon_mutex_open(MUTEX_1_NAME);	// Get the handle to the hardware mutex
#endif

	// FIFOs' initialization
	altera_avalon_fifo_init( FIFO_0_IN_CSR_BASE, 0, FIFO_ALMOST_EMPTY_LEVEL, FIFO_ALMOST_FULL_LEVEL );
	altera_avalon_fifo_init( FIFO_0_OUT_CSR_BASE, 0, FIFO_ALMOST_EMPTY_LEVEL, FIFO_ALMOST_FULL_LEVEL );
	altera_avalon_fifo_init( FIFO_1_IN_CSR_BASE, 0, 0, 16 );
	altera_avalon_fifo_init( FIFO_1_OUT_CSR_BASE, 0, 0, 16 );

	// Send message to cpup_0 indicating cpu_1 is ready
	altera_avalon_fifo_write_fifo( FIFO_0_IN_BASE, FIFO_0_IN_CSR_BASE, 0x2 );



/*
 * Infinite loop
 */
	while( TRUE )
	{
				// Check for "new image available" messages from cpu_0. If there are any messages, read one
				fifo_element = 0;
				while ( fifo_element == 0 )
				{
					fifo_element = altera_avalon_fifo_read_fifo( FIFO_1_OUT_BASE, FIFO_1_OUT_CSR_BASE );
				}

#if PERFORMANCE
				while( altera_avalon_mutex_trylock( mutex_0, ALT_CPU_CPU_ID_VALUE ) ){}
					get_img(img1, IMG_SIZE_32x32x3, fifo_element);
				altera_avalon_mutex_unlock( mutex_0 );
#else
					get_img(img1, ( (*((uBYTE *)(fifo_element))) * (*((uBYTE *)(fifo_element + 1))) * 3 ), fifo_element);
#endif
				altera_avalon_fifo_write_fifo( FIFO_0_IN_BASE, FIFO_0_IN_CSR_BASE, fifo_element );

#if PERFORMANCE
				convert_24bitRGB_to_8bitGrayscale( img1, IMG_SIZE_32x32x3, img1 );
#else
				convert_24bitRGB_to_8bitGrayscale( img1, ((*((uBYTE *)(img1))) * (*((uBYTE *)(img1 + 1))) * 3), img1 );
#endif

				scale_grayscale_img( img1, img1 );

#if PERFORMANCE
				sobel( img1, img2 );
#else
				sobel( img1, img1+IMG_SIZE_20x20x1+8 );
#endif

#if PERFORMANCE
				while( altera_avalon_mutex_trylock( mutex_0, ALT_CPU_CPU_ID_VALUE ) ){}
					put_img(img2, IMG_SIZE_16x16x1, IMG_2_16x16x1_ADDR);
				altera_avalon_mutex_unlock( mutex_0 );
#else
					put_img(img1+IMG_SIZE_20x20x1+8, ((*((uBYTE *)(img1+IMG_SIZE_20x20x1+8))) * (*((uBYTE *)(img1+IMG_SIZE_20x20x1+8 + 1)))), IMG_2_20x20x1_ADDR);
#endif

#if PERFORMANCE
				while( altera_avalon_mutex_trylock( mutex_1, ALT_CPU_CPU_ID_VALUE ) ){}
					altera_avalon_fifo_write_fifo( FIFO_0_IN_BASE, FIFO_0_IN_CSR_BASE, IMG_DSP_DONE_MSG );
					altera_avalon_fifo_write_fifo( FIFO_0_IN_BASE, FIFO_0_IN_CSR_BASE, IMG_2_16x16x1_ADDR );
				altera_avalon_mutex_unlock( mutex_1 );
#else
					altera_avalon_fifo_write_fifo( FIFO_0_IN_BASE, FIFO_0_IN_CSR_BASE, IMG_DSP_DONE_MSG );
					altera_avalon_fifo_write_fifo( FIFO_0_IN_BASE, FIFO_0_IN_CSR_BASE, IMG_2_20x20x1_ADDR );
#endif
	} //End of while (TRUE)	//Infinite loop

	return 0;
}
