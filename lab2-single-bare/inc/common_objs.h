/*
 * This file contains objects that may be used by all CPUs
 */
//#include <stdio.h>
#include "system.h"
//#include "altera_avalon_pio_regs.h"
#include "io.h"
#include "time.h"

/*
 * Parallel IO registers
 */
#include <altera_avalon_pio_regs.h>

/*
 *	Use alt_xxx functions and definitions for portability and code size.
 *	alt_stdio.h has a smaller code footprint but less functionality than stdio.h
 */
#include "sys/alt_stdio.h"
#include "alt_types.h"

/*
 * Mutex HAL and LWHAL (LightWeight)
 */
#include "altera_avalon_mutex.h"
//#include "altera_avalon_mutex_lwhal.h"

/*
 * Timing headers - System Clock & Timestamp
 */
#include "sys/alt_alarm.h"
//#include "sys/alt_timestamp.h"

/*
 * FIFO headers
 */
#include "altera_avalon_fifo_regs.h"
#include "altera_avalon_fifo_util.h"
#include "altera_avalon_fifo.h"

/*
 * Performance counter headers
 */
#include <altera_avalon_performance_counter.h>

#define requests_notepad_area_1	(philosopher *) SHARED_ONCHIP_BASE + 1024

#define TRUE 1

#define DEBUG 0
#define LOW_LEVEL_DEBUG 0
#define PERFORMANCE 0

#define USE_ISR_FOR_BUTTONS		0

#define SHOW_PRINTF_LOW_LEVEL	0
#define SHOW_PRINTF				1

#define DATA_NOT_PROCESSED	0x69
#define DATA_PROCESSED		0x96

#define CPU_1_OFFSET		0x000
#define CPU_2_OFFSET		0x400
#define CPU_3_OFFSET		0x800
#define CPU_4_OFFSET		0xc00

#define CPU_1_SHARED_ONCHIP_ADDR	( SHARED_ONCHIP_BASE + CPU_1_OFFSET )
#define CPU_2_SHARED_ONCHIP_ADDR	( SHARED_ONCHIP_BASE + CPU_2_OFFSET )
#define CPU_3_SHARED_ONCHIP_ADDR	( SHARED_ONCHIP_BASE + CPU_3_OFFSET )
#define CPU_4_SHARED_ONCHIP_ADDR	( SHARED_ONCHIP_BASE + CPU_4_OFFSET )

#define DATA_BYTES			0x400				// Amount of BYTES in a data section/block
#define DATA_WORDS			(DATA_BYTES / 2)	// Amount of WORDS in a data section/block
#define DATA_DWORDS			(DATA_BYTES / 4)	// Amount of INTS in a data section/block
#define DATA_QWORDS			(DATA_BYTES / 8)	// Amount of LONGS in a data section/block

#define CPU_1_MESSAGE		0x11111111
#define CPU_2_MESSAGE		0x22222222
#define CPU_3_MESSAGE		0x33333333
#define CPU_4_MESSAGE		0x44444444

#define SECTION_1			1
#define SECTION_2			2
#define SECTION_3			3
#define SECTION_4			4
#define SECTION_5			5

/* The total amount of shared memory (8k = 8192 bytes 0x102000 to 0x103fff), allows to hold:
 * - 2	32x32 24bit RGB images (each image is 3k = 3072 = 32*32*3 = img_width * img_height * bytes_per_pixel)
 * - 8	32x32 8bit grayscale images (each image is 1k = 1024 = 32*32*1), but we need 3 more bytes to store the width, height and maximum color levels, therefore only 7 can fit in shared memory
 * - 32	16x16 8bit grayscale images (each image is 256 = 16*16*1), but we need 3 more bytes to store the width, height and maximum color levels, therefore only 31 can fit in shared memory
 *
 * In shared memory we can store:
 * 2  32*32*3 images, leaving (8192 - ( 2*(3072 + 3)) = 2048 - 6	bytes to be used
 * 		In the memory space left we can store:
 * 		2  32*32*1 images, leaving ((2048 - 6) - ( 2*(1024 + 3))) =    0 - 6 -  6	bytes needed
 * 		1  32*32*1 images, leaving ((2048 - 6) - ( 1*(1024 + 3))) = 1024 - 6 -  3	bytes needed
 * 			In the memory space left we can store:
 * 			4 16*16*1 images, leaving ((1024 - 6 - 3) - (4*(256 + 3)) =   0 - 6 - 3 - 12	bytes needed
 * 			3 16*16*1 images, leaving ((1024 - 6 - 3) - (3*(256 + 3)) = 256 - 6 - 3 - 9		bytes needed
 * 		8  16*16*1 images, leaving ((2048 - 6) - ( 8*( 256 + 3))) =    0 - 6 - 24	bytes needed
 * 		7  16*16*1 images, leaving ((2048 - 6) - ( 7*( 256 + 3))) =  256 - 6 - 21	bytes needed
 *
 * 8  32*32*1 images, leaving (8192 - ( 8*(1024 + 3))) =    0 - 24	bytes needed
 * 7  32*32*1 images, leaving (8192 - ( 7*(1024 + 3))) = 1024 - 21	bytes needed
 * 32 16*16*1 images, leaving (8192 - (32*( 256 + 3))) =    0 - 96	bytes needed
 * 31 16*16*1 images, leaving (8192 - (31*( 256 + 3))) =  256 - 93	bytes needed
 */
#define IMG_SIZE_32x32x3		0xC00	// Image size used during performance measurements ( 32 * 32 * 3 ) = 3072 = 0xC00. We need to add three more bytes corresponding to the dimensions (width and height) and maximum color levels
#define IMG_SIZE_32x32x1		0x400	// Image size used during performance measurements ( 32 * 32 * 1 ) = 1024 = 0x400. We need to add three more bytes corresponding to the dimensions (width and height) and maximum color levels
#define IMG_SIZE_16x16x1		0x100	// Image size used during performance measurements ( 16 * 16 * 1 ) =  256 = 0x100. We need to add three more bytes corresponding to the dimensions (width and height) and maximum color levels
#define IMG_QWORDS				(IMG_SIZE_32x32x3 >> 3)	// IMG_SIZE_PERF / 8
#define IMG_QDORDS				(IMG_SIZE_32x32x3 >> 2)	// IMG_SIZE_PERF / 4
#define GRAYSCALE_LEVELS		8
#define IMG_1_ADDR				SHARED_ONCHIP_BASE
#define IMG_2_ADDR				(IMG_1_ADDR + IMG_SIZE_32x32x3 + 0x8)	// The +0x8 aligns data in memory to 64 bits and leaves 5 bytes free between images
#define IMG_1_GRAYSCALE_ADDR	(IMG_2_ADDR + IMG_SIZE_32x32x3 + 0x8)
#define IMG_1_SCALED_ADDR		(IMG_1_GRAYSCALE_ADDR + IMG_SIZE_32x32x1 + 0x8)		// This address is the previous address + the size of the object in the previous address + some alignment bytes
#define IMG_2_SCALED_ADDR		(IMG_1_SCALED_ADDR + IMG_SIZE_16x16x1 + 0x8)
#define IMG_1_SOBEL_ADDR		(IMG_2_SCALED_ADDR + IMG_SIZE_16x16x1 + 0x8)

typedef alt_8	BYTE;
typedef alt_16	WORD;
typedef alt_32	DWORD;
typedef alt_64	QWORD;

typedef alt_u8	uBYTE;
typedef alt_u16	uWORD;
typedef alt_u32	uDWORD;
typedef alt_u64	uQWORD;

//Defines properties of a fork/chopstick (mutex)
typedef struct forks
{
	alt_mutex_dev	*handle;	// Handle to the mutex produced by _open
	alt_u8			is_held;	// Allows to know if the CPU has a hold of the mutex

}forks;

//This construct is often used to avoid writing: struct some_struct {...}; and then typedef struct some_struct another_struct;
typedef struct philosopher
{
	//alt_u8		number;
	//alt_u8*		notepad;		// Base address in "shared_onchip" where the philosopher can make "requests"
	alt_u8		has_request;	// Flag variable to let "Dinner host" if the philosopher has a pending request.
	alt_u8		request_type;	//
	alt_u8		angry_level;	//
	alt_u8		food[50];
	alt_u8		philosophical_thought[50];
}philosopher;
