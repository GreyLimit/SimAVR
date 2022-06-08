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
	protected:
		//
		//	Note down where we send reports and interrupts.
		//
		Reporter	*_report;
		Interrupts	*_interrupt;

		//
		//	Define the individual bits which this timers test or set.
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
		//	Define the various modes that the external pin can
		//	be manipulated by timer events.
		//
		enum PinOp {
			//
			//	COMnx1	COMnx0	Description
			//
			//	0	0	No pin operation, pin disconnected
			//	0	1	Toggle output pin on Compare Match
			//	1	0	Clear (0) output pin on Compare Match
			//	1	1	Set (1) output pin on Compare Match
			//
			PinOp_None	= b00,
			PinOp_Toggle	= b01,
			PinOp_Clear	= b10,
			PinOp_Set	= b11
		};

		//
		//	Provide the structure used to manage the comparison pin ops
		//
		struct ComOp {
			PinOp		op;
			const char	*desc;
		};
		static const int pin_op_modes = 4;
		//
		static ComOp _pin_op_mode[ pin_op_modes ];
		
		//
		//	Convert the configuration bits into a pin mode.
		//
		//	Mode is value 0, 1, 2 or 3.
		//
		ComOp *select_pin_mode( byte mode ) {
			ASSERT( mode < pin_op_modes );
			return( &( _pin_op_mode[ mode ]));
		}

		//
		//	Specify possible timings for actions..
		//
		enum ActionAt {
			At_Max,			// At counter maximum value.
			At_Top,			// At value specified in OCR.
			At_Bottom,		// At counter minimum value.
			At_Imm,			// Action immediately.
			At_Never		// Never perform action.
		};

		//
		//	Enumerate the possible match on registers
		//
		enum MatchOn {
			On_Fixed,
			On_OCRA,
			On_ICR,
			On_Never
		};

		//
		//	Declare a structure used to capture the functionality of
		//	of a specified 'waveform'.
		//
		struct WaveForm {
			bool		eight;		// Is this an 8 bit mode.
			byte		mode;		// The mode number this specifies.
			word		maximum;	// The maximum value being applied.
			MatchOn		loop_on;	// Where is the applied maximum found.
			ActionAt	set_ocr,	// When the OCR value is updated.
					set_tov;	// Where the TOV flag is set.
			bool		up_down;	// Do we count up and down?
			//
			//	'report()' formatting output with embedded '%d'
			//	expecting an integer instance number to be provided.
			//
			const char	*desc;
		};

		//
		//	The table of possible waveforms
		//
		static const int waveform_modes = 24;
		static WaveForm _waveform[ waveform_modes ];

		//
		//	Return the address of the waveform description for this mode.
		//
		WaveForm *select_waveform( bool eight, byte mode ) {
			WaveForm *p = _waveform;
			for( int r = 0; r < waveform_modes; r++ ) {
				if(( p->eight == eight )&&( p->mode == mode )) return( p );
				p++;
			}
			//
			//	This is a programming error, all possibilities ought
			//	to be in the _waveform table.  Just bail out, debugging
			//	definitely required.
			//
			ABORT();
			return( _waveform );
		}
		
		//
		//	Declare a data structure used to capture the
		//	various operating modes of of the clock counter
		//	system.
		//
		struct ClockMode {
			bool		running,
					external,
					rising_edge;
			word		prescaler;
			//
			//	'report()' formatting text with %d for instance number.
			//
			const char	*desc;
		};
		
		//
		//	Define the data table that encode all the clock modes.
		//
		static const int clock_modes = 8;
		static ClockMode _clock_mode[ clock_modes ];
				
		//
		//	Return address of a ClockMode record providing clock
		//	operating details.
		//
		ClockMode *select_clock( byte mode ) {
			ASSERT( mode < clock_modes );
			return( &( _clock_mode[ mode ]));
		}

		//
		//	byte/word conversions.
		//
		inline byte high_byte( word value ) { return( value >> 8 ); }
		inline byte low_byte( word value ) { return( value ); }
		inline word high_low( byte hi, byte lo ) { return(( (word)hi << 8 )||((word)lo )); }


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
		//	Common constructor.
		//
		Timer( Reporter *channel, Interrupts *handler ) {
			_report = channel;
			_interrupt = handler;
		}

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
		//
		//	Mechanism for examining content outside the
		//	framework of the simulation.
		//
		virtual bool examine( word id, Symbols *labels, char *buffer, int max ) = 0;
};


//
//	This is the generalised Timer Device for both 8 and 16 bit devices (so far).
//
//	instance	The hardware device number.
//	bits		The size of the counter in bits.
//	compa		The interrupt numbers the associated events raise.
//	compb
//	ovrf
//	xxxx
//
template< int instance, bool eight_bit, byte compa, byte compb, byte ovrf, byte capt > class TimerDevice : public Timer {
	private:
		//
		//	The timer internal registers
		//
		word		_tcnt,
				_ocra,	_pending_ocra,
				_ocrb,	_pending_ocrb,
				_icr;
				
		word		*_loop_on;		// points to where the upper loop value
							// is stored.
				
		byte		_tifr,
				_timsk,
				_tccra,
				_tccrb,
				_tccrc,
				_temp;

		word		_counter;		// The pre-scaling counter.
		bool		_skip_match,
				_countdown;
		
		//
		//	Define elements of the configuration as extracted from
		//	the timer registers.
		//
		ComOp		*_pin_op_a,
				*_pin_op_b;
		WaveForm	*_waveform;
		ClockMode	*_clock;
		 
		//
		//	Is it time to apply an action?
		//
		bool do_action( ActionAt when, word target ) {
			switch( when ) {
				case At_Max:	return( _tcnt == _waveform->maximum );
				case At_Top:	return( _tcnt == target );
				case At_Bottom:	return( _tcnt == 0 );
				case At_Imm:	return( true );
				case At_Never:	return( false );
				default: break;
			}
			ABORT();
			return( false );
		}

		//
		//	Return the address of where the loop target is stored.
		//
		word *locate_loop_on( void ) {
			ASSERT( _waveform != NULL );
			
			switch( _waveform->loop_on ) {
				case On_OCRA: return( &_ocra );
				case On_ICR: return( &_icr );
				default: break;
			}
			return( &( _waveform->maximum ));
		}


	public:
		//
		//	Build a new timer device!
		//
		TimerDevice( Reporter *channel, Interrupts *handler ) : Timer( channel, handler ) {
			_tcnt = 0;
			_ocra = 0; _pending_ocra = 0;
			_ocrb = 0; _pending_ocrb = 0;
			_icr = 0;
			_temp = 0;
			_tifr = 0;
			_timsk = 0;
			_tccra = 0;
			_tccrb = 0;
			_tccrc = 0;

			_pin_op_a = select_pin_mode( 0 );
			_pin_op_b = select_pin_mode( 0 );
			_waveform = select_waveform( eight_bit, 0 );
			_loop_on = locate_loop_on();
			_clock = select_clock( 0 );

			_counter = 0;
			_skip_match = false;
			_countdown = false;
		}

		//
		//	(Clock) Tick API
		//	================
		//
		//	Called once for every tick which the
		//	clock is simulating.
		//
		virtual void tick( word id, UNUSED( bool end_inst )) {
			ASSERT( id == System_Clock );
			if( _clock->running ) {
				if( _clock->external ) {
					//
					//	Not implemented yet - produce no output as its
					//	pointless.
					//
				}
				else {
					if(( _counter += 1 ) >= _clock->prescaler ) {
						_counter = 0;
						//
						//	Increment/decrement the pre-scaled counter.
						//
						if( _waveform->up_down ) {
							//
							//	Doing a saw-tooth counter..
							//
							if( _countdown ) {
								if( _tcnt > 0 ) {
									_tcnt -= 1;
								}
								else {
									_countdown = false;
									_tcnt += 1;
								}
							}
							else {
								if( _tcnt < *_loop_on ) {
									_tcnt += 1;
								}
								else {
									_countdown = true;
									_tcnt -= 1;
								}
							}
						}
						else {
							//
							//	Doing a triangle counter
							//
							if( _tcnt < *_loop_on ) {
								_tcnt += 1;
							}
							else {
								_countdown = false;
								_tcnt = 0;
							}
						}
						//
						//	Now we check for various conditions and
						//	implement the appropriate action in the
						//	event of a suitable match.
						//
						if( do_action( _waveform->set_ocr, _ocra )) {
							if( _ocra != _pending_ocra ) {
								_report->report( Information_Level, Timer_Module, instance, Config_Change, "OCR%dA = %d (from %d )", instance, (int)_pending_ocra, (int)_ocra );
								_ocra = _pending_ocra;
							}
							if( _ocrb != _pending_ocrb ) {
								_report->report( Information_Level, Timer_Module, instance, Config_Change, "OCR%dA = %d (from %d )", instance, (int)_pending_ocra, (int)_ocra );
								_ocrb = _pending_ocrb;
							}
						}
						if( do_action( _waveform->set_tov, _ocra )) {
							_tifr |= bit_TOVn;
							if( _timsk & bit_TOIEn ) _interrupt->raise( ovrf, &_tifr, bit_TOVn );
						}
						if(( _timsk & bit_OCIEnA )&&( _tcnt == _ocra )) {
							_tifr |= bit_OCFnA;
							_interrupt->raise( compa, &_tifr, bit_OCFnA );
						}
						if(( _timsk & bit_OCIEnB )&&( _tcnt == _ocrb )) {
							_tifr |= bit_OCFnB;
							_interrupt->raise( compb, &_tifr, bit_OCFnB );
						}
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
					ASSERT( !eight_bit );
					
					return( _temp );
				}	
				case OCRnBL: {
					_temp = high_byte( _ocrb );
					return( low_byte( _ocrb ));
				}	
				case OCRnAH: {
					ASSERT( !eight_bit );
					
					return( _temp );
				}
				case OCRnAL: {
					_temp = high_byte( _ocra );
					return( low_byte( _ocra ));
				}
				case TCNTnH: {
					ASSERT( !eight_bit );
					
					return( _temp );
				}
				case TCNTnL: {
					_temp = high_byte( _tcnt );
					return( low_byte( _tcnt ));
				}
				case ICRnH: {
					ASSERT( !eight_bit );
					
					return( _temp );
				}
				case ICRnL: {
					_temp = high_byte( _icr );
					return( low_byte( _icr ));
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
					ASSERT( !eight_bit );
					
					_temp = value;
					_report->report( Information_Level, Timer_Module, instance, Config_Change, "Timer %d Temp MSB = %d", instance, (int)value );
					break;
				}
				case OCRnBL: {
					word whole = high_low( _temp, value );
					
					if( whole != _pending_ocrb ) {
						_pending_ocrb = whole;
						if( _waveform->set_ocr == At_Imm ) {
							_ocrb = _pending_ocrb;
							_report->report( Information_Level, Timer_Module, instance, Config_Change, "OCR%dB = %d", instance, (int)_pending_ocrb );
						}
						else {
							_report->report( Information_Level, Timer_Module, instance, Config_Change, "Pending OCR%dB = %d", instance, (int)_pending_ocrb );
						}
					}
					break;
				}	
				case OCRnAH: {
					ASSERT( !eight_bit );
					
					_temp = value;
					_report->report( Information_Level, Timer_Module, instance, Config_Change, "Timer %d Temp MSB = %d", instance, (int)value );
					break;
				}
				case OCRnAL: {
					word whole = high_low( _temp, value );
					
					if( whole != _pending_ocra ) {
						_pending_ocra = whole;
						if( _waveform->set_ocr == At_Imm ) {
							_ocra = _pending_ocra;
							_report->report( Information_Level, Timer_Module, instance, Config_Change, "OCR%dA = %d", instance, (int)_pending_ocra );
						}
						else {
							_report->report( Information_Level, Timer_Module, instance, Config_Change, "Pending OCR%dA = %d", instance, (int)_pending_ocra );
						}
					}
					break;
				}
				case TCNTnH: {
					ASSERT( !eight_bit );
					
					_temp = value;
					_report->report( Information_Level, Timer_Module, instance, Config_Change, "Timer %d Temp MSB = %d", instance, (int)value );
					break;
				}
				case TCNTnL: {
					_tcnt = high_low( _temp, value );
					_skip_match = true;
					_report->report( Information_Level, Timer_Module, instance, Config_Change, "TCNT%d = %d", instance, (int)_tcnt );
					break;
				}
				case ICRnH: {
					ASSERT( !eight_bit );
					
					_temp = value;
					_report->report( Information_Level, Timer_Module, instance, Config_Change, "Timer %d Temp MSB = %d", instance, (int)value );
					break;
				}
				case ICRnL: {
					_icr = high_low( _temp, value );
					_report->report( Information_Level, Timer_Module, instance, Config_Change, "ICR%d = %d", instance, (int)_icr );
					break;
				}
				case TCCRnC: {
					//
					//		7	6	5	4	3	2	1	0
					//	TCCRnC*	FOCnA	FOCnB	–	–	–	–	–	–
					//		r/w	r/w	0	0	0	0	0	0
					//
					//	* This whole register only on 16 bit timer/counter.
					//
					if(( value & 0x3F ) != 0 ) _report->report( Warning_Level, Timer_Module, instance, Parameter_Invalid, "Setting invalid bits in TCCR%dC", instance );
					//
					_report->report( Information_Level, Timer_Module, instance, Config_Change, "TCCR%dC = $%02X", instance, (int)value );
					//if( value & 0x80 ) force_compare_a();
					//if( value & 0x40 ) force_compare_b();
					_tccrc = 0;
					break;
				}
				case TCCRnB: {
					//
					//		7	6	5	4*	3	2	1	0
					//	TCCRnB	ICNCn	ICESn	–	WGMn3	WGMn2	CSn2	CSn1	CSn0
					//		r/w	r/w	0	r/w	r/w	r/w	r/w	r/w
					//
					//	* Only on 16 bit timer/counter.
					//
					byte tccrb_zeros = eight_bit? 0x30: 0x20;
					byte tccrb_mask = ~tccrb_zeros;
					
					_report->report( Information_Level, Timer_Module, instance, Config_Change, "TCCR%dB = $%02X", instance, (int)value );
					if(( value & tccrb_zeros ) != 0 ) _report->report( Warning_Level, Timer_Module, instance, Parameter_Invalid, "Setting invalid bits in TCCR%dB", instance );
					//
					_tccrb = value & tccrb_mask;
					//
					//	(ICNC) Input Capture Noise Canceller.
					//
					//	Ignored.
					//
					//	(ICES) Input Capture Edge Select.
					//
					//	Ignored, for the moment.
					//
					//	(WGM) The output wave generation is configured across
					//	both TCCRnA and TCCRnB.  We can gather all 4 bits to do
					//	this for bth 8 and 16 bit timers as bit 4 will always be
					//	0 on 8 bit timers.
					//
					_waveform = select_waveform( eight_bit, (( _tccrb >> 1 ) & 0x0C )|( _tccra & 0x03 ));
					_loop_on = locate_loop_on();
					_report->report( Information_Level, Timer_Module, instance, Config_Change, _waveform->desc, instance );
					//
					//	(CS) Clock select
					//
					_clock = select_clock( _tccrb & 0x07 );
					_report->report( Information_Level, Timer_Module, instance, Config_Change, _clock->desc, instance );
					break;
				}
				case TCCRnA: {
					//
					//		7	6	5	4	3	2	1	0
					//	TCCRnA	COMnA1	COMnA0	COMnB1	COMnB0	–	-	WGMn1	WGMn0
					//		r/w	r/w	r/w	r/w	0	0	r/w	r/w
					//
					_report->report( Information_Level, Timer_Module, instance, Config_Change, "TCCR%dA = $%02X", instance, (int)value );
					if(( value & 0x0C ) != 0 ) _report->report( Warning_Level, Timer_Module, instance, Parameter_Invalid, "Setting bits 2 or 3 in TCCR%dA", instance );
					//
					_tccra = value & 0xF3;
					//
					//	(COM) Update output compare modes
					//
					_pin_op_a = select_pin_mode(( _tccra >>  6 ) & 0x03 );
					_report->report( Information_Level, Timer_Module, instance, Config_Change, _pin_op_a->desc, instance, 'A' );
					_pin_op_b = select_pin_mode(( _tccra >>  4 ) & 0x03 );
					_report->report( Information_Level, Timer_Module, instance, Config_Change, _pin_op_b->desc, instance, 'B' );
					//
					//	(WGM) The output wave generation is configured across
					//	both TCCRnA and TCCRnB.  We can gather all 4 bits to do
					//	this for bth 8 and 16 bit timers as bit 4 will always be
					//	0 on 8 bit timers.
					//
					_waveform = select_waveform( eight_bit, (( _tccrb >> 1 ) & 0x0C )|( _tccra & 0x03 ));
					_loop_on = locate_loop_on();
					_report->report( Information_Level, Timer_Module, instance, Config_Change, _waveform->desc, instance );
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
		//
		//	Mechanism for examining content outside the
		//	framework of the simulation.
		//
		virtual bool examine( word id, Symbols *labels, char *buffer, int max ) {
			switch( id ) {
				case OCRnBH:
				case OCRnBL: {
					snprintf( buffer, max, "OCR%dB = %d", instance, (int)_ocrb );
					return( true );
				}	
				case OCRnAH:
				case OCRnAL: {
					snprintf( buffer, max, "OCR%dA = %d", instance, (int)_ocra );
					return( true );
				}
				case TCNTnH:
				case TCNTnL: {
					snprintf( buffer, max, "TCNT%d = %d", instance, (int)_tcnt );
					return( true );
				}
				case ICRnH:
				case ICRnL: {
					snprintf( buffer, max, "ICR%d = %d", instance, (int)_icr );
					return( true );
				}
				case TCCRnC: {
					snprintf( buffer, max, "TCCR%dC = $%02X", instance, (int)_tccrc );
					return( true );
				}
				case TCCRnB: {
					snprintf( buffer, max, "TCCR%dB = $%02X", instance, (int)_tccrb );
					return( true );
				}
				case TCCRnA: {
					snprintf( buffer, max, "TCCR%dA = $%02X", instance, (int)_tccra );
					return( true );
				}
				case TIFRn: {
					snprintf( buffer, max, "TIFR%d = $%02X", instance, (int)_tifr );
					return( true );
				}
				case TIMSKn: {
					snprintf( buffer, max, "TIMSK%d = $%02X", instance, (int)_timsk );
					return( true );
				}
				default: {
					ABORT();
					break;
				}
			}
			return( false );
		}
};


#endif

//
//	EOF
//
