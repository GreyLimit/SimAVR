//
//	Main program entry point.
//

#include <stdio.h>

//
//	Include Simulator Header files so that they are
//	all parsed and compiled to facilitate debugging
//	of the included code while the package is not
//	yet complete.
//
#include "AVR_Const.h"
#include "AVR_CPU.h"
#include "Base.h"
#include "Clock.h"
#include "CPU.h"
#include "Flash.h"
#include "Fuses.h"
#include "FusesCore.h"
#include "Fuses_48.h"
#include "Fuses_88_168.h"
#include "Fuses_328.h"
#include "FusesPhaseOne.h"
#include "FusesPhaseTwo.h"
#include "Interrupts.h"
#include "Map.h"
#include "Memory.h"
#include "Program.h"
#include "Programmer.h"
#include "Reporter.h"
#include "SRAM.h"
#include "Symbols.h"
#include "Validation.h"
#include "Console.h"
#include "Timer.h"
#include "DeviceRegister.h"





//
//	wrapper to convert data space address to IO port number.
//
#define EXT_IO(n)	((n)-0x20)

static CPU *atmega328p( Reporter *channel, const char *load ) {
	
	AVR_CPU		*board		= new AVR_CPU();

	Fuses		*fuses		= new Fuses_328( channel, AVR_ATmega328P );

					//
					//	Declare an interrupt manager for IRQs 1 through to 26.
					//
	Interrupts	*handler	= new InterruptDevice< 26 >( channel );

	Flash		*firmware	= new Program< 64, 16384, 32, 4000 >( channel );
						firmware->load_hex( load );

	Clock		*crystal	= new ClockFrequency< 32 >( 16000 );

					//
					//	Create specific memory map for the IO ports.
					//	This will include both "normal" IO ports (with
					//	numbers from 0 to 63) and the external IO
					//	ports that can only be accessed via the data
					//	address space.  At the "port" level these are
					//	numbered from 64 to 223.  The EXT_IO() macro
					//	needs to be used to convert the DS address to
					//	one of the extended IO port numbers.
					//
	Memory		*ports		= new Map< 64 >( channel );
						//
						//	"Special" CPU Registers
						//
						//	EIND, RAMD, RAMX, RAMY, RAMZ, SPL, SPH, SREG
						//
						ports->segment( new SRAM< 8 >( channel ), 0x0038 );

					//
					//	Timer 0, the first 8 bit timer.
					//
					//	15	0x00E TIMER0_COMPA Timer/Counter0 Compare Match A
					//	16	0x00F TIMER0_COMPB Timer/Counter0 Compare Match B
					//	17	0x010 TIMER0_OVF Timer/Counter0 Overflow
					//
	Timer		*timer0		= new TimerDevice< 0, 0xFF, 15, 16, 17, 0 >( channel, handler );
						ports->segment( new DeviceRegister( (Notification *)timer0, Timer::TIMSKn ), EXT_IO( 0x6E ));
						ports->segment( new DeviceRegister( (Notification *)timer0, Timer::OCRnB ), 0x28 );
						ports->segment( new DeviceRegister( (Notification *)timer0, Timer::OCRnA ), 0x27 );
						ports->segment( new DeviceRegister( (Notification *)timer0, Timer::TCNTn ), 0x26 );
						ports->segment( new DeviceRegister( (Notification *)timer0, Timer::TCCRnB ), 0x25 );
						ports->segment( new DeviceRegister( (Notification *)timer0, Timer::TCCRnA ), 0x24 );
						ports->segment( new DeviceRegister( (Notification *)timer0, Timer::TIFRn ), 0x15 );
						crystal->add( timer0 );

					//
					//	Timer 1, the 16 bit timer.
					//
					//	11	0x00A TIMER1_CAPT Timer/Counter1 Capture Event
					//	12	0x00B TIMER1_COMPA Timer/Counter1 Compare Match A
					//	13	0x00C TIMER1_COMPB Timer/Counter1 Compare Match B
					//	14	0x00D TIMER1_OVF Timer/Counter1 Overflow
					//
	Timer		*timer1		= new TimerDevice< 1, 0xFFFF, 12, 13, 14, 11 >( channel, handler );
						ports->segment( new DeviceRegister( (Notification *)timer1, Timer::OCRnBH ), EXT_IO( 0x8B ));
						ports->segment( new DeviceRegister( (Notification *)timer1, Timer::OCRnBL ), EXT_IO( 0x8A ));
						ports->segment( new DeviceRegister( (Notification *)timer1, Timer::OCRnAH ), EXT_IO( 0x89 ));
						ports->segment( new DeviceRegister( (Notification *)timer1, Timer::OCRnAL ), EXT_IO( 0x88 ));
						ports->segment( new DeviceRegister( (Notification *)timer1, Timer::ICRnH ), EXT_IO( 0x87 ));
						ports->segment( new DeviceRegister( (Notification *)timer1, Timer::ICRnL ), EXT_IO( 0x86 ));
						ports->segment( new DeviceRegister( (Notification *)timer1, Timer::TCNTnH ), EXT_IO( 0x85 ));
						ports->segment( new DeviceRegister( (Notification *)timer1, Timer::TCNTnL ), EXT_IO( 0x84 ));
						ports->segment( new DeviceRegister( (Notification *)timer1, Timer::TCCRnC ), EXT_IO( 0x82 ));
						ports->segment( new DeviceRegister( (Notification *)timer1, Timer::TCCRnB ), EXT_IO( 0x81 ));
						ports->segment( new DeviceRegister( (Notification *)timer1, Timer::TCCRnA ), EXT_IO( 0x80 ));
						ports->segment( new DeviceRegister( (Notification *)timer1, Timer::TIMSKn ), EXT_IO( 0x6F ));
						ports->segment( new DeviceRegister( (Notification *)timer1, Timer::TIFRn ), 0x16 );
						crystal->add( timer1 );
	
					//
					//	Timer 2, the second 8 bit timer.
					//
					//	8	0x007 TIMER2_COMPA Timer/Counter2 Compare Match A
					//	9	0x008 TIMER2_COMPB Timer/Counter2 Compare Match B
					//	10	0x009 TIMER2_OVF Timer/Counter2 Overflow
					//
	Timer		*timer2		= new TimerDevice< 0, 0xFF, 8, 9, 10, 0 >( channel, handler );
						ports->segment( new DeviceRegister( (Notification *)timer2, Timer::TIMSKn ), EXT_IO( 0x6E ));
						ports->segment( new DeviceRegister( (Notification *)timer2, Timer::OCRnB ), 0x28 );
						ports->segment( new DeviceRegister( (Notification *)timer2, Timer::OCRnA ), 0x27 );
						ports->segment( new DeviceRegister( (Notification *)timer2, Timer::TCNTn ), 0x26 );
						ports->segment( new DeviceRegister( (Notification *)timer2, Timer::TCCRnB ), 0x25 );
						ports->segment( new DeviceRegister( (Notification *)timer2, Timer::TCCRnA ), 0x24 );
						ports->segment( new DeviceRegister( (Notification *)timer2, Timer::TIFRn ), 0x15 );
						crystal->add( timer2 );
					//
					//	The Flash (Re)Programming Device.
					//
					//	26	0x019 SPM_Ready Store Program Memory Ready
					//
	Programmer	*programmer	= new ProgrammerDevice< 0, 26 >( channel, firmware, board, handler, crystal, fuses );
						ports->segment( new DeviceRegister( (Notification *)programmer, Programmer::SPMCSR ), 0x37 );
						crystal->add( programmer );

	Memory		*data		= new Map< 8 >( channel );
						data->segment( ports, 0x0020 );

	Memory		*regs		= new SRAM< 32 >( channel );
						data->segment( regs, 0x0000 );
					
	Memory		*sram		= new SRAM< 2048 >( channel );
						data->segment( sram, 0x0200 );
								
	board->construct(	AVRxt_Inst,	// Modern basic instructions
				14,		// Program Address size (bits)
				firmware,	
				programmer,	
				data,
				regs,
				ports,
				handler,
				crystal,
				channel );

	return( board );
}

#define BUFFER 128

int main( int argc, char* argv[]) {
	char	*hex;
	
	Reporter *channel	= new Console;
	Symbols *labels		= new Symbols( channel );

	hex = NULL;
	for( int a = 1; a < argc; a++ ) {
		char	*p;

		if(( p = strrchr( argv[ a ], '.' ))) {
			p++;
			if( strcmp( p, "hex" ) == 0 ) {
				if( hex ) {
					fprintf( stderr, "Only one HEX file can be specified.\n" );
					return( 1 );
				}
				hex = argv[ a ];
			}
			else if( strcmp( p, "sym" ) == 0 ) {
				if( !labels->load_symbols( argv[ a ])) {
					fprintf( stderr, "Error loading symbol file '%s'.\n", argv[ a ]);
					return( 1 );
				}
			}
			else {
				fprintf( stderr, "Unrecognised file argument '%s'.\n", argv[ a ]);
				return( 1 );
			}
		}
	}
	
	CPU	*simulate	= atmega328p( channel, hex );

	while( true ) {
		char	adrs[ BUFFER ],
			inst[ BUFFER ];
		
		dword	pc	= simulate->next_instruction();
		word	len	= simulate->disassemble( pc, labels, inst, BUFFER );
		
		printf( "%s: %s\n", labels->expand( program_address, pc, adrs, BUFFER ), inst );

		if( fgets( inst, BUFFER, stdin ) == NULL ) break;
		
		simulate->step();
	}
	
	return( 0 );
}

//
//	EOF
//
