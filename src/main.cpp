//
//	Main program entry point.
//

#include <stdio.h>
#include <signal.h>


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
#include "BreakPoint.h"
#include "Pin.h"
#include "AnalogueConversion.h"
#include "Port.h"
#include "SerialComms.h"
#include "Coverage.h"


//
//	Routine to catch Ctrl-C
//
static volatile bool keep_running = true;
static void Ctrl_C( int dummy ) {
	keep_running = false;
}


//
//	wrapper to convert data space address to IO port number.  This
//	is used on the extended ports as we have implemented them as
//	a single extended set of io addresses (224 possible locations)
//
#define EXT_IO(n)	((n)-0x20)

static CPU *atmega328p( Reporter *channel, Coverage *tracker, const char *load, Fuses *fuses ) {
	
					//
					//	Declare an interrupt manager for IRQs 1 through to 26.
					//
	Interrupts	*handler	= new InterruptDevice< 26 >( channel );

	Flash		*firmware	= new Program< 0, 64, 256, 32, 4000 >( channel );
						//
						//	Load firmware from supplied hex file.
						//
						firmware->load_hex( load );

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
	Memory		*ports		= new MapSegments< 1 >( channel, 224 );

	Port		*portb		= new PortDevice< 1 >( channel );
						//
						//	Add in bits of IO
						//
						portb->attach( new Pin( channel, 0 ), 0 );
						portb->attach( new Pin( channel, 15 ), 1 );
						portb->attach( new Pin( channel, 16 ), 2 );
						portb->attach( new Pin( channel, 17 ), 3 );
						portb->attach( new Pin( channel, 18 ), 4 );
						portb->attach( new Pin( channel, 19 ), 5 );
						portb->attach( new Pin( channel, 9 ), 6 );
						portb->attach( new Pin( channel, 10 ), 7 );
						ports->segment( new DeviceRegister( portb, Port::PORTn ), 0x05 );
						ports->segment( new DeviceRegister( portb, Port::DDRn ), 0x04 );
						ports->segment( new DeviceRegister( portb, Port::PINn ), 0x03 );

					//
					//	Build the ADC system
					//
	AnalogueConversion *adc		= new AnalogueConversion( channel );
						ports->segment( new DeviceRegister( adc, AnalogueConversion::ADCSRA ), EXT_IO( 0x7A ));

					//
					//	The USART
					//
	SerialComms *serial		= new SerialComms( channel );
						ports->segment( new DeviceRegister( serial, SerialComms::UDRn ), EXT_IO( 0xC6 ));
						ports->segment( new DeviceRegister( serial, SerialComms::UBRRnH ), EXT_IO( 0xC5 ));
						ports->segment( new DeviceRegister( serial, SerialComms::UBRRnL ), EXT_IO( 0xC4 ));
						ports->segment( new DeviceRegister( serial, SerialComms::UCSRnC ), EXT_IO( 0xC2 ));
						ports->segment( new DeviceRegister( serial, SerialComms::UCSRnB ), EXT_IO( 0xC1 ));
						ports->segment( new DeviceRegister( serial, SerialComms::UCSRnA ), EXT_IO( 0xC0 ));

					//
					//	Declare the processor core.
					//
	AVR_CPU		*processor	= new AVR_CPU();
						//
						//	"Special" CPU Registers that are located
						//	in the port address space.
						//
						ports->segment( new DeviceRegister( (Notification *)processor, AVR_CPU::WDTCSR ), EXT_IO( 0x60 ));
						ports->segment( new DeviceRegister( (Notification *)processor, AVR_CPU::SREG ), 0x3F );
						ports->segment( new DeviceRegister( (Notification *)processor, AVR_CPU::SPH ), 0x3E );
						ports->segment( new DeviceRegister( (Notification *)processor, AVR_CPU::SPL ), 0x3D );
						ports->segment( new DeviceRegister( (Notification *)processor, AVR_CPU::EIND ), 0x3C );
						ports->segment( new DeviceRegister( (Notification *)processor, AVR_CPU::RAMZ ), 0x3B );
						ports->segment( new DeviceRegister( (Notification *)processor, AVR_CPU::RAMY ), 0x3A );
						ports->segment( new DeviceRegister( (Notification *)processor, AVR_CPU::RAMX ), 0x39 );
						ports->segment( new DeviceRegister( (Notification *)processor, AVR_CPU::RAMD ), 0x38 );
						ports->segment( new DeviceRegister( (Notification *)processor, AVR_CPU::MCUCR ), 0x35 );
						ports->segment( new DeviceRegister( (Notification *)processor, AVR_CPU::MCUSR ), 0x34 );

	Clock		*crystal	= new Clock( channel, 16000 );
						//
						//	The processor see a WDT clock at 128 KHz, but does
						//	not need to see any other clock input.
						//
						ports->segment( new DeviceRegister( (Notification *)crystal, Clock::CLKPR ), EXT_IO( 0x61 ));
						crystal->add( AVR_CPU::WDT_Clock, (Tick *)processor, 128 );

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
						crystal->add( Timer::System_Clock, timer0 );

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
						crystal->add( Timer::System_Clock, timer1 );
	
					//
					//	Timer 2, the second 8 bit timer.
					//
					//	8	0x007 TIMER2_COMPA Timer/Counter2 Compare Match A
					//	9	0x008 TIMER2_COMPB Timer/Counter2 Compare Match B
					//	10	0x009 TIMER2_OVF Timer/Counter2 Overflow
					//
	Timer		*timer2		= new TimerDevice< 0, 0xFF, 8, 9, 10, 0 >( channel, handler );
						ports->segment( new DeviceRegister( (Notification *)timer2, Timer::OCRnB ), EXT_IO( 0xB4 ));
						ports->segment( new DeviceRegister( (Notification *)timer2, Timer::OCRnA ), EXT_IO( 0xB3 ));
						ports->segment( new DeviceRegister( (Notification *)timer2, Timer::TCNTn ), EXT_IO( 0xB2 ));
						ports->segment( new DeviceRegister( (Notification *)timer2, Timer::TCCRnB ), EXT_IO( 0xB1 ));
						ports->segment( new DeviceRegister( (Notification *)timer2, Timer::TCCRnA ), EXT_IO( 0xB0 ));
						ports->segment( new DeviceRegister( (Notification *)timer2, Timer::TIMSKn ), EXT_IO( 0x70 ));
						ports->segment( new DeviceRegister( (Notification *)timer2, Timer::TIFRn ), 0x17 );
						crystal->add( Timer::System_Clock, timer2 );
					//
					//	The Flash (Re)Programming Device.
					//
					//	26	0x019 SPM_Ready Store Program Memory Ready
					//
	Programmer	*programmer	= new ProgrammerDevice< 0, 26 >( channel, firmware, processor, handler, crystal, fuses );
						ports->segment( new DeviceRegister( (Notification *)programmer, Programmer::SPMCSR ), 0x37 );
						crystal->add( Programmer::System_Clock, programmer );

					//
					//	Define the CPU registers as a small piece of memory.
					//
	Memory		*regs		= new SRAM< 32 >( channel );

					//
					//	Finally create and include the program data space itself.
					//
	Memory		*sram		= new SRAM< 2048 >( channel );

					//
					//	Create the "Data Space" address map that the
					//	register, port and SRAM will be mapped into.
					//
	Memory		*data		= new MapSegments< 0 >( channel, 0x0100 + 2048 );
						//
						//	The other areas into the data address space.
						//
						data->segment( regs,  0x0000 );
						data->segment( ports, 0x0020 );
						data->segment( sram,  0x0100 );
	//
	//	Bring all the pieces together in the CPU object.
	//							
	processor->construct(	AVRxt_Inst,	// Modern basic instructions
				14,		// Program Address size (bits)
				firmware,	
				programmer,
				fuses,
				data,
				regs,
				ports,
				handler,
				crystal,
				tracker,
				channel );

	return( processor );
}

#define LIST	32
#define BUFFER	128

int main( int argc, char* argv[]) {
	char	*hex;
	
	Reporter *channel	= new Console;
	Symbols	*labels		= new Symbols( channel );
	Fuses	*fuses		= new Fuses_328( channel, AVR_ATmega328P );

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
			else if( strcmp( p, "fuse" ) == 0 ) {
				if( !fuses->load_fuses( argv[ a ], labels )) {
					fprintf( stderr, "Error loading fuse file '%s'.\n", argv[ a ]);
					return( 1 );
				}
			}
			else {
				fprintf( stderr, "Unrecognised file argument '%s'.\n", argv[ a ]);
				return( 1 );
			}
		}
	}
	
	BreakPoint	*breaks		= new BreakPoint();
	Coverage	*tracker	= new Coverage( channel );
	CPU		*simulate	= atmega328p( channel, tracker, hex, fuses );

	//
	//	Prepare to catch Ctrl-C
	//
	signal( SIGINT, Ctrl_C );
	
	while( true ) {
		char	adrs[ BUFFER ],
			inst[ BUFFER ],
			*dec;
		
		dword	pc	= simulate->next_instruction();
		word	len	= simulate->disassemble( pc, labels, inst, BUFFER );

		printf( "%s: %s\n", labels->expand( program_address, pc, adrs, BUFFER ), inst );
		printf( "> " );
		fflush( stdout );
		
		if( fgets( inst, BUFFER, stdin ) == NULL ) break;
		keep_running = true;

		inst[ strlen( inst )-1 ] = EOS;
		dec = inst;
		
		switch( *dec++ ) {
			case EOS: {
				//
				//	Single step on just pressing enter.
				//
				simulate->step();
				break;
			}
			case 'r': {
				//
				//	Run, or run a number of instructions.
				//
				bool	counter;
				int	left, count, n;

				counter = (( left = ( count = atoi( dec ))) > 0 );
				while( keep_running ) {
					simulate->step();
					if(( n = breaks->check( simulate->next_instruction())) != 0 ) {
						printf( "Break point %d.\n", n );
						break;
					}
					if( counter ) {
						if( --count == 0 ) break;
						if( channel->exception()) {
							printf( "Exception after %d instructions.\n", count - left );
							break;
						}
					}
					else {
						if( channel->exception()) {
							printf( "Exception stops execution.\n" );
							break;
						}
					}
				}
				break;
			}
			case 't': {
				//
				//	Trace instructions.  Like rujn but displays instructions
				//	as they are about to be executed.
				//
				bool	counter;
				int	left, count, n;

				counter = (( left = ( count = atoi( dec ))) > 0 );
				while( keep_running ) {
					simulate->step();
					if(( n = breaks->check( simulate->next_instruction())) != 0 ) {
						printf( "Break point %d.\n", n );
						break;
					}
					if( counter ) {
						if( --count == 0 ) break;
						if( channel->exception()) {
							printf( "Exception after %d instructions.\n", count - left );
							break;
						}
					}
					else {
						if( channel->exception()) {
							printf( "Exception stops execution.\n" );
							break;
						}
					}
					pc = simulate->next_instruction();
					len = simulate->disassemble( pc, labels, inst, BUFFER );
					printf( "%s: %s\n", labels->expand( program_address, pc, adrs, BUFFER ), inst );
				}
				break;
			}
			case 'd': {
				//
				//	Disassemble a number of instructions.
				//
				dword	a;
				int	c;
				char	*at;

				if(( at = strchr( dec, '@' )) != NULL ) {
					*at++ = EOS;
					if( !labels->evaluate( program_address, at, &a )) {
						printf( "Start address not recognised.\n" );
						break;
					}
				}
				else {
					a = pc;
				}
				if(( c = atoi( dec )) == 0 ) c = 1;
				while( c-- ) {
					word len = simulate->disassemble( a, labels, inst, BUFFER );
					printf( "%s: %s\n", labels->expand( program_address, a, adrs, BUFFER ), inst );
					a += len;
				}
				break;
			}
			case 'w': {
				//
				//	Write out the symbol table to a file.
				//
				if( !labels->save_symbols( dec )) {
					printf( "Failed to write to file '%s'.\n", inst+1 );
				}
				else {
					printf( "done.\n" );
				}
				break;
			}
			case 'b': {
				//
				//	Add a break point
				//
				word	n;
				dword	a;

				if( labels->evaluate( program_address, dec, &a )) {
					if(( n = breaks->add( a )) == 0 ) {
						printf( "Unable to add new breakpoint.\n" );
					}
					else {
						printf( "Breakpoint %d set.\n", n );
					}
				}
				break;
			}
			case 'x': {
				//
				//	Remove a break point
				//
				word n = atoi( dec );

				if( !breaks->remove( n )) {
					printf( "Invalid breakpoint %d.\n", n );
				}
				break;
			}
			case '?': {
				switch( *dec++ ) {
					case 'v': {
						word	r;

						//
						//	Display Symbols by value.
						//
						r = 0;
						while( labels->show_symbol( r++, false, inst, BUFFER )) printf( "%s\n", inst );
						break;
					}
					case 's': {
						word	r;

						//
						//	Display Symbols by name.
						//
						r = 0;
						while( labels->show_symbol( r++, true, inst, BUFFER )) printf( "%s\n", inst );
						break;
					}
					case 'r': {
						word	r;

						//
						//	Display registers of the CPU.
						//
						if( *dec == EOS ) {
							r = 0;
							while( simulate->examine( Register_Address, r++, labels, inst, BUFFER )) printf( "%15s%c", inst, (( r & 3 )? '\t': '\n' ));
							printf( "\n" );
						}
						else {
							dword	d;
							
							if( labels->evaluate( byte_register, dec, &d )) {
								r = d;
								if( simulate->examine( Register_Address, r, labels, inst, BUFFER )) {
									printf( "%s\n", inst );
								}
								else {
									printf( "Register %d unrecognised.\n", r );
								}
							}
						}
						break;
					}
					case 'p': {
						word	p;

						//
						//	Display Port registers.
						//
						if( *dec == EOS ) {
							p = 0;
							while( simulate->examine( Port_Address, p++, labels, inst, BUFFER )) printf( "%15s%c", inst, (( p & 3 )? '\t': '\n' ));
							printf( "\n" );
						}
						else {
							dword	d;
							
							if( labels->evaluate( port_number, dec, &d )) {
								p = d;
								if( simulate->examine( Port_Address, p, labels, inst, BUFFER )) {
									printf( "%s\n", inst );
								}
								else {
									printf( "Port %d unrecognised.\n", p );
								}
							}
						}
						break;
					}
					case 'b': {
						//
						//	Breakpoints
						//
						int	id[ LIST ],
							count;
							
						if(( count = breaks->list( id, LIST ))) {
							printf( "Break points:\n" );
							for( int i = 0; i < count; i++ ) printf( "\t%d @ %s\n", id[ i ], labels->expand( program_address, breaks->address( id[ i ]), inst, BUFFER ));
						}
						else {
							printf( "No breaks set.\n" );
						}
						break;
					}
					case 'c': {
						//
						//	Coverage data.
						//
						tracker->dump( stdout );
						break;
					}
					default: {
						printf( "Help display not written yet.\n" );
						break;
					}
				}
				break;
			}
			default: {
				printf( "Eh '%c'?\n", *inst );
				break;
			}
		}
	}
	
	return( 0 );
}

//
//	EOF
//
