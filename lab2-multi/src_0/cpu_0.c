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
#ifndef CPU_0
#define CPU_0	1
#endif

#include "../inc/common_objs.h"
#include "../inc/lab2.c"
#include "images.h"

extern void delay (int millisec);





/*
 * Main
 */
int main()
{
// Variable's declarations
	uWORD			i, print;//j, k;	//general loop variable
	uDWORD			fifo_element;
	uWORD			executions, images_received;	//general purpose variables
	uBYTE			*images[11];

	// Expected maximum image arrays
#if PERFORMANCE
	uBYTE			img_sobel[ ( 16 * 16 * 1 ) + 3 ] __attribute__ (( aligned(64) ));	//
#else
	uBYTE			img_sobel[ ( 40 * 40 * 1 ) + 3 ];	//
#endif
	alt_mutex_dev	*mutex_0;	// Handle to the mutex hardware

	alt_printf( "***** %s is initializing... *****\n", ALT_CPU_NAME );

// Variable's initializations
	mutex_0 = altera_avalon_mutex_open(MUTEX_0_NAME);	// Get the handle to the hardware mutex

	// FIFOs' initialization
	altera_avalon_fifo_init( FIFO_0_IN_CSR_BASE, 0, FIFO_ALMOST_EMPTY_LEVEL, FIFO_ALMOST_FULL_LEVEL );
	altera_avalon_fifo_init( FIFO_0_OUT_CSR_BASE, 0, FIFO_ALMOST_EMPTY_LEVEL, FIFO_ALMOST_FULL_LEVEL );
	altera_avalon_fifo_init( FIFO_1_IN_CSR_BASE, 0, 0, 16 );
	altera_avalon_fifo_init( FIFO_1_OUT_CSR_BASE, 0, 0, 16 );

	// The first three are used for performance measurements
#if PERFORMANCE
	images[ 0] = img1_32_32; // Smiley face
	images[ 1] = img2_32_32; // Spiral
	images[ 2] = img_3; // img_3 Check Mark
	images[ 3] = img_4; // img_4 Not Allowed sign
	images[ 4] = img_5; // img_5 Star
	images[ 5] = img_6; // img_6 Flower
	images[ 6] = img_7; // img_7 Dots
	images[ 7] = img_8; // img_8 Revert
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
	//images[10] = img3_32_32;
#endif

#if PERFORMANCE
	executions =420;
#else
	executions = 420;
#endif
	print = 0;
	i = 0;
	images_received = 0;

	alt_printf("Waiting for all CPUs to be ready...\n");
	// Empty FIFOs from previously stored elements
	while ( altera_avalon_fifo_read_status( FIFO_0_OUT_CSR_BASE, ALTERA_AVALON_FIFO_STATUS_E_MSK )!= 0x02 ) // Read FIFO while it is not empty
	{
		fifo_element = altera_avalon_fifo_read_fifo( FIFO_0_OUT_BASE, FIFO_0_OUT_CSR_BASE );
	}
	while ( altera_avalon_fifo_read_status( FIFO_1_OUT_CSR_BASE, ALTERA_AVALON_FIFO_STATUS_E_MSK )!= 0x02 ) // Read FIFO while it is not empty
	{
		fifo_element = altera_avalon_fifo_read_fifo( FIFO_1_OUT_BASE, FIFO_1_OUT_CSR_BASE );
	}
	// Wait until ALL four CPUs have written to the FIFO. WARNING: This depends on the "almostempty" initialization value which is set to 3. With this value, when 4 elements have been written to the FIFO, the "almost empty" flag will be cleared
	while ( altera_avalon_fifo_read_status( FIFO_0_OUT_CSR_BASE, ALTERA_AVALON_FIFO_STATUS_AE_MSK ) == 0x08 ){}
	// Read all messages to double check they are from cpu_1 through cpu_4
	for( i = 0; i < 4; i++ ) // Read ONLY four messages. With the while we can read ALL messages until FIFO buffer is empty
	{
		fifo_element = altera_avalon_fifo_read_fifo( FIFO_0_OUT_BASE, FIFO_0_OUT_CSR_BASE );
		switch( fifo_element )
		{
			case 0x1:	alt_printf( "cpu_1 is ready!!!\n" ); break;
			case 0x2:	alt_printf( "cpu_2 is ready!!!\n" ); break;
			case 0x3:	alt_printf( "cpu_3 is ready!!!\n" ); break;
			case 0x4:	alt_printf( "cpu_4 is ready!!!\n" ); break;
			default:	alt_printf( "Something weird happened!!! Message received was: %x\n", fifo_element ); break;

		}
	}
	alt_printf("ALL CPUs are ready!!!\n");

#if PERFORMANCE
	// Performance counter initialization
	// Reset Performance Counter
	PERF_RESET(PERFORMANCE_COUNTER_0_BASE);

	// Start Measuring
	PERF_START_MEASURING (PERFORMANCE_COUNTER_0_BASE);

	// Section 1
	PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, SECTION_1);
	while( altera_avalon_mutex_trylock( mutex_0, ALT_CPU_CPU_ID_VALUE ) ){}
		put_img(images[ executions % NUMBER_OF_IMAGES ], IMG_SIZE_32x32x3, IMG_1_ADDR);
		--executions;
		altera_avalon_fifo_write_fifo( FIFO_1_IN_BASE, FIFO_1_IN_CSR_BASE, IMG_1_ADDR );
		put_img(images[ executions % NUMBER_OF_IMAGES ], IMG_SIZE_32x32x3, IMG_2_ADDR);
		--executions;
		altera_avalon_fifo_write_fifo( FIFO_1_IN_BASE, FIFO_1_IN_CSR_BASE, IMG_2_ADDR );
	altera_avalon_mutex_unlock( mutex_0 );
	PERF_END(PERFORMANCE_COUNTER_0_BASE, SECTION_1);
	PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, SECTION_5);
#else
	while( altera_avalon_mutex_trylock( mutex_0, ALT_CPU_CPU_ID_VALUE ) ){}
		put_img( images[ executions % NUMBER_OF_IMAGES ], ( (*((uBYTE *)(images[ executions % NUMBER_OF_IMAGES ]))) * (*((uBYTE *)(images[ executions % NUMBER_OF_IMAGES ] + 1))) * 3 ), IMG_1_ADDR);
		--executions;
	altera_avalon_mutex_unlock( mutex_0 );
	// Send FIFO message
	altera_avalon_fifo_write_fifo( FIFO_1_IN_BASE, FIFO_1_IN_CSR_BASE, IMG_1_ADDR );
	//PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, SECTION_2);
#endif

	

/*
 * Infinite loop
 */
	while( TRUE )
	{
		// Wait for "image read" messages from the other CPUs indicating that at least one of the images has been read. The message is the address of the read image
		fifo_element = 0;
		while ( fifo_element == 0 )
		{
			fifo_element = altera_avalon_fifo_read_fifo( FIFO_0_OUT_BASE, FIFO_0_OUT_CSR_BASE );
		}

		// Check if message received was "image processed successfully":
		if( fifo_element == IMG_DSP_DONE_MSG )
		{
			// Read the address of the processed image
			fifo_element = altera_avalon_fifo_read_fifo( FIFO_0_OUT_BASE, FIFO_0_OUT_CSR_BASE );
			while( altera_avalon_mutex_trylock( mutex_0, ALT_CPU_CPU_ID_VALUE ) ){}
#if PERFORMANCE
				get_img(img_sobel, IMG_SIZE_16x16x1, fifo_element);
#else
				get_img(img_sobel, ( (*((uBYTE *)(fifo_element))) * (*((uBYTE *)(fifo_element + 1))) * 3 ), fifo_element);
#endif	
			altera_avalon_mutex_unlock( mutex_0 );

			++images_received;
			//alt_printf("%x\n",images_received);
			//print_ASCII(img_sobel, 0);
#if PERFORMANCE
#else
				alt_printf("%x\n",images_received);
				print_ASCII(img_sobel, 0);
#endif
		}else
		{
			if(executions > 0)
			{
				// After receiving the message, store another image in shared memory
				while( altera_avalon_mutex_trylock( mutex_0, ALT_CPU_CPU_ID_VALUE ) ){}
#if PERFORMANCE
					put_img(images[ executions % NUMBER_OF_IMAGES ], IMG_SIZE_32x32x3, fifo_element);
#else
					put_img( images[ executions % NUMBER_OF_IMAGES ], ( (*((uBYTE *)(images[ executions % NUMBER_OF_IMAGES ]))) * (*((uBYTE *)(images[ executions % NUMBER_OF_IMAGES ] + 1))) * 3 ), fifo_element);
#endif
					--executions;
				altera_avalon_mutex_unlock( mutex_0 );

				altera_avalon_fifo_write_fifo( FIFO_1_IN_BASE, FIFO_1_IN_CSR_BASE, fifo_element );
			}else
			{

			} // if(executions > 0)

		} // if( fifo_element == IMG_DSP_DONE_MSG )

#if PERFORMANCE
		if (images_received == 420 && executions == 0)
		{
		PERF_END(PERFORMANCE_COUNTER_0_BASE, SECTION_5);

		// End Measuring
		PERF_STOP_MEASURING(PERFORMANCE_COUNTER_0_BASE);

		// Print measurement report
		perf_print_formatted_report( (void *) PERFORMANCE_COUNTER_0_BASE,
			ALT_CPU_FREQ,                 // defined in "system.h"
			5,                            // How many sections to print
			"Put 1st 2 imgs", " ", " ", " ", "Total Execution" );    // Display-name of section(s).
		}
#endif
	} //End of while (TRUE)	//Infinite loop

	return 0;
}
