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
//	an extension to the base set of 64 io addresses giving a total
//	of 224 possible port locations.
//
#define EXT_IO(n)	((n)-0x20)

static CPU *atmega328p( Reporter *channel, Coverage *tracker, const char *load, Fuses *fuses, Clock *crystal ) {
	
					//
					//	Set up all the pins on the package.  We create
					//	all of them though some are not IO pins.  Note
					//	that the package will number them 1 to 28, the
					//	array will index them 0 to 28.  0 will be ignored.
					//
	Pin		*pin[ 29 ];
					for( int i = 0; i < 29; i++ ) pin[ i ] = new Pin( channel, i );
					
					//
					//	Declare an interrupt manager for IRQs 1 through to 26.
					//
	Interrupts	*irq_router	= new InterruptDevice< 26 >( channel, 0 );

	Flash		*firmware	= new Program< 64, 256, 32, 4000 >( channel, 0 );
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
	Memory		*ports		= new Map( channel, 1, 224 );

	Port		*portb		= new Port( channel, 1 );
						//
						//	Add in bits of IO
						//
						//portb->attach( new Pin( channel, 0 ), 0 );
						portb->attach( pin[ 15 ], 1 );
						portb->attach( pin[ 16 ], 2 );
						portb->attach( pin[ 17 ], 3 );
						portb->attach( pin[ 18 ], 4 );
						portb->attach( pin[ 19 ], 5 );
						portb->attach( pin[ 9 ], 6 );
						portb->attach( pin[ 10 ], 7 );
						ports->segment( new DeviceRegister( portb, Port::PORTn ), 0x05 );
						ports->segment( new DeviceRegister( portb, Port::DDRn ), 0x04 );
						ports->segment( new DeviceRegister( portb, Port::PINn ), 0x03 );

					//
					//	Build the ADC system
					//
	AnalogueConversion *adc		= new AnalogueConversion( channel, 0 );
						ports->segment( new DeviceRegister( adc, AnalogueConversion::ADCSRA ), EXT_IO( 0x7A ));

					//
					//	The USART
					//
	SerialComms *serial		= new SerialComms( channel, 0 );
						ports->segment( new DeviceRegister( serial, SerialComms::UDRn ), EXT_IO( 0xC6 ));
						ports->segment( new DeviceRegister( serial, SerialComms::UBRRnH ), EXT_IO( 0xC5 ));
						ports->segment( new DeviceRegister( serial, SerialComms::UBRRnL ), EXT_IO( 0xC4 ));
						ports->segment( new DeviceRegister( serial, SerialComms::UCSRnC ), EXT_IO( 0xC2 ));
						ports->segment( new DeviceRegister( serial, SerialComms::UCSRnB ), EXT_IO( 0xC1 ));
						ports->segment( new DeviceRegister( serial, SerialComms::UCSRnA ), EXT_IO( 0xC0 ));

					//
					//	Declare the processor core.
					//
	AVR_CPU		*processor	= new AVR_CPU( channel, 0, tracker );
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
						//
						//	The processor sees a WDT clock at 128 KHz, but does
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
	Timer		*timer0		= new TimerDevice< 0, true, 15, 16, 17, 0 >( channel, irq_router );
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
	Timer		*timer1		= new TimerDevice< 1, false, 12, 13, 14, 11 >( channel, irq_router );
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
	Timer		*timer2		= new TimerDevice< 2, true, 8, 9, 10, 0 >( channel, irq_router );
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
					//	IRQ
					//
					//	26	0x019 SPM_Ready Store Program Memory Ready
					//
	Programmer	*programmer	= new ProgrammerDevice< 26 >( channel, 0, firmware, processor, irq_router, crystal, fuses );
						ports->segment( new DeviceRegister( (Notification *)programmer, Programmer::SPMCSR ), 0x37 );
						crystal->add( Programmer::System_Clock, programmer );

					//
					//	Finally create and include the program data space itself.
					//
	Memory		*sram		= new SRAM< 2048 >( channel, 1 );

					//
					//	Create the "Data Space" address map that the
					//	register, port and SRAM will be mapped into.
					//
					//	We anticipate an address space of 256 bytes (registers
					//	and ports) plus the 2048 bytes of static RAM.
					//
	Memory		*data		= new Map( channel, 0, 0x0100 + 2048 );
						//
						//	Map the registers and other areas into the data address space.
						//
						for( int i = 0; i < AVR_CPU::GPRegisters; i++ ) data->segment( new DeviceRegister( (Notification *)processor, i ), i );
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
				ports,
				pin, 29,	// remember 0 then 1 to 28.
				irq_router,
				crystal );

	return( processor );
}

#define LIST	32
#define BUFFER	128

int main( int argc, char* argv[]) {
	char	*hex;
	
	Reporter *channel	= new Console;
	Symbols	*labels		= new Symbols( channel, 0 );
	Fuses	*fuses		= new Fuses_328( channel, 0, AVR_ATmega328P );
	Clock	*crystal	= new Clock( channel, 0, 16000 );
	
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
	Coverage	*tracker	= new Coverage( channel, 0 );
	CPU		*simulate	= atmega328p( channel, tracker, hex, fuses, crystal );

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

		printf( "%8lu %s: %s\n", (unsigned long int)crystal->count(), labels->expand( program_address, pc, adrs, BUFFER ), inst );
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
							printf( "Exception after %d instructions.\n",  left - count  );
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
					printf( "%8lu %s: %s\n", (unsigned long int)crystal->count(), labels->expand( program_address, pc, adrs, BUFFER ), inst );
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
				dword	a1, a2;
				char	*c;

				if(( c = strchr( dec, COMMA )) != NULL ) {
					*c++ = EOS;
					if( labels->evaluate( program_address, dec, &a1 ) && labels->evaluate( program_address, c, &a2 )) {
						if( a2 < a1 ) {
							printf( "Invalid end of breakpoint range.\n" );
						}
						else {
							if(( n = breaks->add( a1, a2+1 )) == 0 ) {
								printf( "Unable to add new breakpoint.\n" );
							}
							else {
								printf( "Breakpoint %d set.\n", n );
							}
						}
					}
					else {
						printf( "Invalid breakpoint address\n" );
					}
				}
				else {
					if( labels->evaluate( program_address, dec, &a1 )) {
						if(( n = breaks->add( a1, a1+1 )) == 0 ) {
							printf( "Unable to add new breakpoint.\n" );
						}
						else {
							printf( "Breakpoint %d set.\n", n );
						}
					}
					else {
						printf( "Invalid breakpoint address\n" );
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
							for( int i = 0; i < count; i++ ) {
								dword	s, e;

								if( breaks->address( id[ i ], &s, &e )) {
									e -= 1;
									if( e == s ) {
										printf( "\t%d @ %s\n", id[ i ], labels->expand( program_address, s, inst, BUFFER ));
									}
									else {
										printf( "\t%d @ %s,%s\n", id[ i ], labels->expand( program_address, s, inst, BUFFER ), labels->expand( program_address, e, adrs, BUFFER ));
									}
								}
							}
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
						int sf[ LIST ], fc;
						switch( *dec++ ) {
							case 'm': {
								fc = 0;
								sf[ fc++ ] =  Read_Access;
								sf[ fc++ ] =  Write_Access;
								sf[ fc++ ] =  Stack_Access;
								break;
							}
							case 'p': {
								fc = 0;
								sf[ fc++ ] =  Execute_Access;
								sf[ fc++ ] =  Jump_Access;
								sf[ fc++ ] =  Call_Access;
								sf[ fc++ ] =  Data_Access;
								break;
							}
							case 'a':
							default: {
								fc = 0;
								sf[ fc++ ] =  Execute_Access;
								sf[ fc++ ] =  Jump_Access;
								sf[ fc++ ] =  Call_Access;
								sf[ fc++ ] =  Data_Access;
								sf[ fc++ ] =  Read_Access;
								sf[ fc++ ] =  Write_Access;
								sf[ fc++ ] =  Stack_Access;
								break;
							}
						}
						tracker->dump( stdout, sf, fc );
						break;
					}
					default: {
						printf( "Help:\n" );
						printf( "<CR>\tSingle step\n" );
						printf( "r\tRun\n" );
						printf( "rN\tRun N instructions\n" );
						printf( "t\tTrace\n" );
						printf( "tN\tTrace N instructions\n" );
						printf( "dN\tDisassemble N instructions\n" );
						printf( "dN@A\tas above but from address A\n" );
						printf( "wF\tSave symbols to file F\n" );
						printf( "bA\tSet breakpoint at address A\n" );
						printf( "xN\tDelete breakpoint number N\n" );
						printf( "?\tThis help\n" );
						printf( "?v\tDisplay symbols by value\n" );
						printf( "?s\tDisplay symbols by name\n" );
						printf( "?r\tDisplay all CPU registers\n" );
						printf( "?rN\tDisplay CPU register N\n" );
						printf( "?p\tDisplay all ports\n" );
						printf( "?pN\tDisplay port number N\n" );
						printf( "?b\tDisplay breakpoints\n" );
						printf( "?ca\tDisplay all coverage data\n" );
						printf( "?cp\tDisplay program coverage data\n" );
						printf( "?cm\tDisplay memory coverage data\n" );
						printf( "\n\tN and A have the form '({symbol}[+-])?{number}'\n" );
						printf( "\twhere number is '$' hex, '%%' bin or decimal.\n" );
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
