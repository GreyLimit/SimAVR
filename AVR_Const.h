//
//	AVR_Const.h
//	===========
//
//	Define constants relating to the AVR chips
//

#ifndef _AVR_CONST_H_
#define _AVR_CONST_H_

//
//	Bring in the base definitions.
//
#include "Base.h"

//
//	Enumerate the various AVR MCUs we shall aim
//	to simulate.
//
typedef enum {
	//
	//	Define the different AVR architectures
	//			
	AVR_Inst	= 0,	// "AVR"	Original instruction set from 1995.
	
	AVRe_Inst	= 1,	// "megaAVR"	Multiply (xMULxx), Move Word (MOVW),
				//		and enhanced Load Program Memory (LPM)
				//		added to the AVR instruction set.
				
	AVRet_Inst	= 2,	// "tinyAVR"	Multiply not included, but else equal
				//		to AVRe for megaAVR ("et" is my label
				//		to apply to this option).
				
	AVRxm_Inst	= 3,	// "XMEGA"	The Read Modify Write (RMW) and
				//		DES encryption instructions are
				//		unique to this version.
				
	AVRxt_Inst	= 4,	// "AVR"	Base AVR from 2016 and onwards.
	
	AVRrc_Inst	= 5	// "tinyAVR"	The Reduced Core AVR CPU.
} AVR_InstSet;

//
//	Define the number of types we will support (as above).
//
const byte AVR_InstructionTypes = 6;

//
//	Define the various AVR Micro Processors
//
typedef enum {
	AVR_ATmega48A,
	AVR_ATmega48PA,
	AVR_ATmega88A,
	AVR_ATmega88PA,
	AVR_ATmega168A,
	AVR_ATmega168PA,
	AVR_ATmega328,
	AVR_ATmega328P,
	AVR_ATmega640,
	AVR_ATmega1280,
	AVR_ATmega1281,
	AVR_ATmega2560,
	AVR_ATmega2561
} AVR_Processor;

#endif

//
//	EOF
//
