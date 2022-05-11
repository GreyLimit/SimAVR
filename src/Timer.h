//
//	Timer.h
//	=======
//
//	The definition and implementation of a timer.
//

#ifndef _TIMER_H_
#define _TIMER_H_

//
//	Include basics and dependencies.
//

#include "Base.h"
#include "Clock.h"
#include "Memory.h"
#include "Interrupts.h"
#include "Validation.h"
#include "DeviceRegister.h"

//
//	This is the generic timer class that can be handled in
//	in a non-specific manner.
//
class Timer : public Tick, Notification {
	public:
		//
		//	Declare all the possible timer device registers
		//	this module recognises
		//
		//	OCRnBH		Output Compare Register B High Byte
		//	OCRnBL		Output Compare Register B Low Byte
		//	OCRnAH		Output Compare Register A High Byte
		//	OCRnAL		Output Compare Register A Low Byte
		//	ICRnH		Input Capture Register High Byte
		//	ICRnL		Input Capture Register Low Byte
		//	TCNTnH		Counter Register High Byte
		//	TCNTnL		Counter Register Low Byte
		//
		//			7	6	5	4	3	2	1	0
		//	TCCRnC		FOC1A	FOC1B	–	–	–	–	–	–
		//	TCCRnB		ICNC1	ICES1	–	WGM13	WGM12	CS12	CS11	CS10
		//	TCCRnA		COM1A1	COM1A0	COM1B1	COM1B0	–	–	WGM11	WGM10
		//
		//	TIFRn		-	-	ICFn	-	-	OCFnB	OCFnA	TOVn
		//	TIMSKn		–	-	ICIEn	–	–	OCIEnB	OCIEnA	TOIEn
		//
		//	The specific identifiers associated with the above
		//	labels do not have any factual relationship to any
		//	IO port location that it may be presented in.
		//
		static const word OCRnBH	= 1;
		static const word OCRnBL	= 2;	
		static const word OCRnB		= 2;	// Alias for OCRnBL
		static const word OCRnAH	= 3;
		static const word OCRnAL	= 4;
		static const word OCRnA		= 4;	// Alias for OCRnAL
		static const word ICRnH		= 5;
		static const word ICRnL		= 6;
		static const word TCNTnH	= 7;
		static const word TCNTnL	= 8;
		static const word TCNTn		= 8;	// Alias for TCNTnL
		static const word TCCRnC	= 9;
		static const word TCCRnB	= 10;
		static const word TCCRnA	= 11;
		static const word TIFRn		= 12;
		static const word TIMSKn	= 13;

		//
		//	This is our handle for the system clock.
		//
		static const word System_Clock = 0;

		//
		//	(Clock) Tick API
		//	================
		//
		//	Called once for every tick which the
		//	clock is simulating.
		//
		virtual void tick( word id, bool inst_end ) = 0;

		//
		//	Notification API
		//	================
		//
		//	This is where the links from DeviceRegister
		//	object are received.
		//
		virtual byte read_register( word id ) = 0;
		virtual void write_register( word id, byte value ) = 0;
};


//
//	This is the Timer Class which is defined to be a specific size
//	of timing device.
//
//	instance	This is the abstract number of the timer, 0, 1,
//			2 etc.  Used for nothing other than associating
//			events being logged with the right device.
//
//	maximum		Defines the highest value that the counter can
//			reach before wrapping.
//
//	compa		The interrupt number the associated events raise.
//	compb
//	ovrf
//	capt
//
template< int instance, word maximum, byte compa, byte compb, byte ovrf, byte capt > class TimerDevice : public Timer {
	private:
		//
		//	Define the individual bits which this device tests or sets.
		//
		static const byte bit_ICFn	= BIT( byte, 5 );
		static const byte bit_OCFnB	= BIT( byte, 2 );
		static const byte bit_OCFnA	= BIT( byte, 1 );
		static const byte bit_TOVn	= BIT( byte, 0 );
		//
		static const byte bit_ICIEn	= BIT( byte, 5 );
		static const byte bit_OCIEnB	= BIT( byte, 2 );
		static const byte bit_OCIEnA	= BIT( byte, 1 );
		static const byte bit_TOIEn	= BIT( byte, 0 );
		
		//
		//	Note down where we send reports and interrupts.
		//
		Reporter	*_report;
		Interrupts	*_interrupt;
		
		//
		//	The timer internal registers
		//
		word		_tcnt,
				_ocra,
				_ocrb,
				_icr;
		byte		_temp_high_byte,
				_tifr,
				_timsk,
				_tccra,
				_tccrb,
				_tccrc;

		//
		//	Define a hybrid set of registers that provide
		//	a more decomposed and accessible version of the
		//	timer configuration.
		//
		bool		_running,
				_external,
				_rising;
		word		_prescaler,
				_counter;

		//
		//	byte/word conversions.
		//
		inline byte high_byte( word value ) { return( value >> 8 ); }
		inline byte low_byte( word value ) { return( value ); }
		inline word high_low( byte hi, byte lo ) { return(( (word)hi << 8 )||((word)lo )); }

		//
		//	The force compare routines.
		//
		void force_compare_a( void ) {
		}
		void force_compare_b( void ) {
		}

		//
		//	Setting up output compare modes.
		//
		//	Mode is value 0, 1, 2 or 3.
		//
		void compare_output_mode_a( byte mode ) {
			//
			//	COMnA1	COMnA0	Description
			//
			//	0	0	No pin operation, pin disconnected.
			//	0	1	Toggle output pin on Compare Match.
			//	1	0	Clear (0) output pin on Compare Match.
			//	1	1	Set (1) output pin on Compare Match.
			//
			switch( mode ) {
				case b00: {
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "COMnA=00, No pin op" );
					break;
				}
				case b01: {
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "COMnA=01, Toggle pin" );
					break;
				}
				case b10: {
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "COMnA=10, Clear pin" );
					break;
				}
				case b11: {
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "COMnA=11, Set pin" );
					break;
				}
				default: {
					ABORT();
					break;
				}
			}
		}
		void compare_output_mode_b( byte mode ) {
			//
			//	COMnB1	COMnB0	Description
			//
			//	0	0	No pin operation, pin disconnected.
			//	0	1	Toggle output pin on Compare Match.
			//	1	0	Clear (0) output pin on Compare Match.
			//	1	1	Set (1) output pin on Compare Match.
			//
			switch( mode ) {
				case b00: {
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "COMnB=00, No pin op" );
					break;
				}
				case b01: {
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "COMnB=01, Toggle pin" );
					break;
				}
				case b10: {
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "COMnB=10, Clear pin" );
					break;
				}
				case b11: {
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "COMnB=11, Set pin" );
					break;
				}
				default: {
					ABORT();
					break;
				}
			}
		}

		//
		//	Set up the wave form generator mode
		//
		//	Mode is 16 values 0 to 15.
		//
		void waveform_generator( byte mode ) {
			//
			//	WGMn3	WGMn2	WGMn1	 WGMn0	Timer/Counter Mode		TOP	Update of	TOVn Flag
			//		(CTCn)	(PWMn1)	(PWMn0)	of Operation				OCRnx at	Set on
			//
			//	0	0	0	0	Normal				0xFFFF	Immediate	MAX
			//	0	0	0	1	PWM, Phase Correct, 8-bit	0x00FF	TOP		BOTTOM
			//	0	0	1	0	PWM, Phase Correct, 9-bit 	0x01FF	TOP		BOTTOM
			//	0	0	1	1	PWM, Phase Correct, 10-bit 	0x03FF	TOP		BOTTOM
			//	0	1	0	0	CTC				OCR1A	Immediate	MAX
			//	0	1	0	1	Fast PWM, 8-bit			0x00FF	BOTTOM		TOP
			//	0	1	1	0	Fast PWM, 9-bit			0x01FF	BOTTOM		TOP
			//	0	1	1	1	Fast PWM, 10-bit		0x03FF	BOTTOM		TOP
			//	1	0	0	0	PWM, Phase and Freq Correct	ICR1	BOTTOM		BOTTOM
			//	1	0	0	1	PWM, Phase and Freq Correct	OCR1A	BOTTOM		BOTTOM
			//	1	0	1	0	PWM, Phase Correct		ICR1	TOP		BOTTOM
			//	1	0	1	1	PWM, Phase Correct		OCR1A	TOP		BOTTOM
			//	1	1	0	0	CTC				ICR1	Immediate	MAX
			//	1	1	0	1	(Reserved)			–	–		–
			//	1	1	1	0	Fast PWM			ICR1	BOTTOM		TOP
			//	1	1	1	1	Fast PWM			OCR1A	BOTTOM		TOP	
			//
			//	Notes/
			//
			//	CTC	Clear Timer on Compare match
			//
			switch( mode ) {
				case b0000: {
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "WGMn=0000, Normal" );
					break;
				}
				case b0001: {
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "WGMn=0001, PWM, Phase Correct, 8-bit" );
					break;
				}
				case b0010: {
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "WGMn=0010, PWM, Phase Correct, 9-bit" );
					break;
				}
				case b0011: {
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "WGMn=0011, PWM, Phase Correct, 10-bit" );
					break;
				}
				case b0100: {
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "WGMn=0100, CTC (OCRnA)" );
					break;
				}
				case b0101: {
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "WGMn=0101, Fast PWM, 8-bit" );
					break;
				}
				case b0110: {
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "WGMn=0110, Fast PWM, 9-bit" );
					break;
				}
				case b0111: {
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "WGMn=0111, Fast PWM, 10-bit" );
					break;
				}
				case b1000: {
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "WGMn=1000, PWM, Phase and Freq Correct (ICRn)" );
					break;
				}
				case b1001: {
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "WGMn=1001, PWM, Phase and Freq Correct (OCRnA)" );
					break;
				}
				case b1010: {
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "WGMn=1010, PWM, Phase Correct (ICRn)" );
					break;
				}
				case b1011: {
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "WGMn=1011, PWM, Phase Correct (OCRnA)" );
					break;
				}
				case b1100: {
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "WGMn=1100, CTC (IRCn)" );
					break;
				}
				case b1101: {
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "WGMn=1101, Set pin" );
					break;
				}
				case b1110: {
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "WGMn=1110, Set pin" );
					break;
				}
				case b1111: {
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "WGMn=1111, Set pin" );
					break;
				}
				default: {
					ABORT();
					break;
				}
			}
		}

		//
		//	Select the clock source and any pre-scaler.
		//
		//	Mode is 8 value 0 to 7.
		//
		void select_clock( byte mode ) {
			//
			//	CSn2	CSn1	CSn0	Description
			//	0	0	0	No clock source (Timer/Counter stopped).
			//	0	0	1	clk I/O /1 (No prescaling)
			//	0	1	0	clk I/O /8 (From prescaler)
			//	0	1	1	clk I/O /64 (From prescaler)
			//	1	0	0 	clk I/O /256 (From prescaler)
			//	1	0	1	clk I/O /1024 (From prescaler)
			//	1	1	0	External clock source on T1 pin. Clock on falling edge.
			//	1	1	1	External clock source on T1 pin. Clock on rising edge.
			//
			switch( mode ) {
				case b000: {
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "CS=000, No Clock" );
					_running = false;
					_external = false;
					_rising = false;
					_prescaler = 1;
					_counter = 0;
					break;
				}
				case b001: {
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "CS=001, Clock/1" );
					_running = true;
					_external = false;
					_rising = false;
					_prescaler = 1;
					_counter = 0;
					break;
				}
				case b010: {
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "CS=010, Clock/8" );
					_running = true;
					_external = false;
					_rising = false;
					_prescaler = 8;
					_counter = 0;
					break;
				}
				case b011: {
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "CS=011, Clock/64" );
					_running = true;
					_external = false;
					_rising = false;
					_prescaler = 64;
					_counter = 0;
					break;
				}
				case b100: {
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "CS=100, Clock/256" );
					_running = true;
					_external = false;
					_rising = false;
					_prescaler = 256;
					_counter = 0;
					break;
				}
				case b101: {
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "CS=101, Clock/1024" );
					_running = true;
					_external = false;
					_rising = false;
					_prescaler = 1024;
					_counter = 0;
					break;
				}
				case b110: {
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "CS=110, External/falling" );
					_report->raise( Warning_Level, Timer_Module, Not_Implemented, instance, "External Clock not implemented" );
					_running = true;
					_external = true;
					_rising = false;
					_prescaler = 1;
					_counter = 0;
					break;
				}
				case b111: {
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "CS=111, External/rising" );
					_report->raise( Warning_Level, Timer_Module, Not_Implemented, instance, "External Clock not implemented" );
					_running = true;
					_external = true;
					_rising = true;
					_prescaler = 1;
					_counter = 0;
					break;
				}
				default: {
					ABORT();
					break;
				}
			}
		}
		 
	public:
		//
		//	Build a new timer device!
		//
		TimerDevice( Reporter *channel, Interrupts *handler ) {
			_report = channel;
			_interrupt = handler;
			
			_tcnt = 0;
			_ocra = 0;
			_ocrb = 0;
			_icr = 0;
			_temp_high_byte = 0;
			_tifr = 0;
			_timsk = 0;
			_tccra = 0;
			_tccrb = 0;
			_tccrc = 0;
			
			_running = false;
			_external = false;
			_rising = false;
			_prescaler = 1;
			_counter = 0;
		}

		//
		//	(Clock) Tick API
		//	================
		//
		//	Called once for every tick which the
		//	clock is simulating.
		//
		virtual void tick( UNUSED( word id ), UNUSED( bool end_inst )) {
			if( _running ) {
				if( _external ) {
					//
					//	Do nothing here.  Do not produce any warning
					//	or information as this will create far too many
					//	events.
					//
				}
				else {
					if(( _counter += 1 ) == _prescaler ) {
						if( _tcnt >= maximum ) {
							//
							//	OVERFLOW
							//
							_tcnt = 0;
						}
						_tcnt += 1;
						//if( _tcnt == _  HERE
					}
				}
			}
		}

		//
		//	These API routines provide the  interface
		//	back from the IO register to the device
		//	itself.
		//
		virtual byte read_register( word id ) {
			switch( id ) {
				case OCRnBH: {
					return( _temp_high_byte );
				}
				case OCRnBL: {
					_temp_high_byte = high_byte( _ocrb );
					return( low_byte( _ocrb ));
				}	
				case OCRnAH: {
					return( _temp_high_byte );
				}
				case OCRnAL: {
					_temp_high_byte = high_byte( _ocra );
					return( low_byte( _ocra ));
				}
				case ICRnH: {
					return( _temp_high_byte );
				}
				case ICRnL: {
					_temp_high_byte = high_byte( _icr );
					return( low_byte( _icr ));
				}
				case TCNTnH: {
					return( _temp_high_byte );
				}
				case TCNTnL: {
					_temp_high_byte = high_byte( _tcnt );
					return( low_byte( _tcnt ));
				}
				case TCCRnC: {
					return( _tccrc );
				}
				case TCCRnB: {
					return( _tccrb );
				}
				case TCCRnA: {
					return( _tccra );
				}
				case TIFRn: {
					return( _tifr );
				}
				case TIMSKn: {
					return( _timsk );
				}
				default: {
					ABORT();
					break;
				}
			}
			return( 0 );
		}
		virtual void write_register( word id, byte value )  {
			switch( id ) {
				case OCRnBH: {
					_temp_high_byte = value;
					break;
				}
				case OCRnBL: {
					_ocrb = high_low( _temp_high_byte, value );
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "OCRnB =", _ocrb );
					break;
				}	
				case OCRnAH: {
					_temp_high_byte = value;
					break;
				}
				case OCRnAL: {
					_ocra = high_low( _temp_high_byte, value );
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "OCRnA =", _ocra );
					break;
				}
				case ICRnH: {
					_temp_high_byte = value;
					break;
				}
				case ICRnL: {
					_icr = high_low( _temp_high_byte, value );
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "ICRn =", _icr );
					break;
				}
				case TCNTnH: {
					_temp_high_byte = value;
					break;
				}
				case TCNTnL: {
					break;
					_tcnt = high_low( _temp_high_byte, value );
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "TCNTn =", _tcnt );
				}
				case TCCRnC: {
					//
					//		7	6	5	4	3	2	1	0
					//	TCCRnC	FOCnA	FOCnB	–	–	–	–	–	–
					//		r/w	r/w	0	0	0	0	0	0
					//
					if(( value & 0x3F ) != 0 ) _report->raise( Warning_Level, Timer_Module, Parameter_Invalid );
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "TCCRnC =", value );
					if( value & 0x80 ) force_compare_a();
					if( value & 0x40 ) force_compare_b();
					_tccrc = 0;
					break;
				}
				case TCCRnB: {
					//
					//		7	6	5	4	3	2	1	0
					//	TCCRnB	ICNCn	ICESn	–	WGMn3	WGMn2	CSn2	CSn1	CSn0
					//		r/w	r/w	0	r/w	r/w	r/w	r/w	r/w
					//
					if(( value & 0x20 ) != 0 ) _report->raise( Warning_Level, Timer_Module, Parameter_Invalid );
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "TCCRnB =", value );
					_tccrb = value & 0xDF;
					//
					//	(ICNC) Input Capture Noise Canceler.
					//
					// Ignored.
					//
					//	(ICES) Input Capture Edge Select.
					//
					// Ignored.
					//
					//	(WGM) The output wave generation is configured across
					//	both TCCRnA and TCCRnB....
					//
					waveform_generator((( _tccrb >> 1) & 0xC0 )|( _tccra & 0x03 ));
					//
					//	(CS) Clock select
					//
					select_clock( _tccrb & 0x07 );
					break;
				}
				case TCCRnA: {
					//
					//		7	6	5	4	3	2	1	0
					//	TCCRnA	COMnA1	COMnA0	COMnB1	COMnB0	–	-	WGMn1	WGMn0
					//		r/w	r/w	r/w	r/w	0	0	r/w	r/w
					//
					if(( value & 0x0C ) != 0 ) _report->raise( Warning_Level, Timer_Module, Parameter_Invalid );
					_report->raise( Information_Level, Timer_Module, Config_Change, instance, "TCCRnA =", value );
					_tccra = value & 0xF3;
					//
					//	(COM) Update output compare modes
					//
					compare_output_mode_a(( _tccra >>  6 ) & 0x03 );
					compare_output_mode_b(( _tccra >>  4 ) & 0x03 );
					//
					//	(WGM) The output wave generation is configured across
					//	both TCCRnA and TCCRnB....
					//
					waveform_generator((( _tccrb >> 1) & 0xC0 )|( _tccra & 0x03 ));
					break;
				}
				case TIFRn: {
					//
					//	Timer Interrupt Flags
					//
					//		7	6	5	4	3	2	1	0
					//	TIFRn	-	-	ICFn	-	–	OCFnB	OCFnA	TOVn
					//		0	0	r/w	0	0	r/w	r/w	r/w
					//
					break;
				}
				case TIMSKn: {
					//
					//	Timer Interrupt Mask
					//
					//		7	6	5	4	3	2	1	0
					//	TIMSKn	-	-	ICIEn	-	–	OCIEnB	OCIEnA	TOIEn
					//		0	0	r/w	0	0	r/w	r/w	r/w
					//
					break;
				}
				default: {
					ABORT();
					break;
				}
			}
		}	
};



#endif

//
//	EOF
//
