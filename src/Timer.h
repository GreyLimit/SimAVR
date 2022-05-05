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

	public:

		//
		//	(Clock) Tick API
		//	================
		//
		//	Called once for every tick which the
		//	clock is simulating.
		//
		virtual void tick( void ) = 0;

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
template< word instance, word maximum, byte compa, byte compb, byte ovrf, byte capt > class TimerDevice : public Timer {
	private:
		//
		//	Note down where we send reports and interrupts.
		//
		Reporter	*_report;
		Interrupts	*_interrupt;
		
		//
		//	The timer internal registers
		//
		
	public:
		//
		//	Build a new timer device!
		//
		TimerDevice( Reporter *channel, Interrupts *handler ) {
			_report = channel;
			_interrupt = handler;
			
		}

		//
		//	(Clock) Tick API
		//	================
		//
		//	Called once for every tick which the
		//	clock is simulating.
		//
		virtual void tick( void ) {
		}

		//
		//	These API routines provide the  interface
		//	back from the IO register to the device
		//	itself.
		//
		virtual byte read_register( word id ) {
			switch( id ) {
				case OCRnBH: {
					break;
				}
				case OCRnBL: {
					break;
				}	
				case OCRnAH: {
					break;
				}
				case OCRnAL: {
					break;
				}
				case ICRnH: {
					break;
				}
				case ICRnL: {
					break;
				}
				case TCNTnH: {
					break;
				}
				case TCNTnL: {
					break;
				}
				case TCCRnC: {
					break;
				}
				case TCCRnB: {
					break;
				}
				case TCCRnA: {
					break;
				}
				case TIFRn: {
					break;
				}
				case TIMSKn: {
					break;
				}
				default: {
					break;
				}
			}
			return( 0 );
		}
		virtual void write_register( word id, byte value )  {
			switch( id ) {
				case OCRnBH: {
					break;
				}
				case OCRnBL: {
					break;
				}	
				case OCRnAH: {
					break;
				}
				case OCRnAL: {
					break;
				}
				case ICRnH: {
					break;
				}
				case ICRnL: {
					break;
				}
				case TCNTnH: {
					break;
				}
				case TCNTnL: {
					break;
				}
				case TCCRnC: {
					break;
				}
				case TCCRnB: {
					break;
				}
				case TCCRnA: {
					break;
				}
				case TIFRn: {
					break;
				}
				case TIMSKn: {
					break;
				}
				default: {
					break;
				}
			}
		}	
};



#endif

//
//	EOF
//
