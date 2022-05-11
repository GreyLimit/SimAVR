//
//	Pin.h
//	=====
//
//	Define a basic unit of Input or Output
//

#ifndef _PIN_H_
#define _PIN_H_

//
//	Dependencies
//
#include "Base.h"
#include "Reporter.h"

//
//	Pins come in a number of flavours
//
enum PinMode {
	PinDisabled,
	PinInput,
	PinOutput,
	PinAnalogue
};

//
//	The underlying Pin class
//
class Pin {
	private:
		//
		//	Reporting route.
		//
		Reporter	*_report;

		//
		//	Our pin number, for human reference.
		//
		word		_number;

		//
		//	Our current mode.
		//
		PinMode		_mode;

		//
		//	Our current value.
		//
		word		_value;

	public:
		Pin( Reporter *report, word number ) {
			_report = report;
			_number = number;
			_mode = PinDisabled;
			_value = 0;
		}
		
		//
		//	Basic things that can be done to a pin:
		//
		void set_mode( PinMode mode ) {
			_report->raise( Information_Level, Pin_Module, Config_Change, _number, "New pin mode", (word)mode );
			_mode =  mode;
			_value = 0;
		}
			
		PinMode get_mode( void ) {
			return( _mode );
		}

		bool get_input( void ) {
			if( _mode != PinInput ) {
				_report->raise( Error_Level, Pin_Module, Read_Invalid, _number, "Read from non-input pin" );
				return( false );
			}
			return( _value? true: false );
		}
			
		void set_output( bool to )  {
			if( _mode != PinOutput ) {
				_report->raise( Error_Level, Pin_Module, Read_Invalid, _number, "Write to non-output pin" );
			}
			else {
				_value = ( to? 1: 0 );
			}
		}
		
		word get_analogue( void ) {
			if( _mode != PinAnalogue ) {
				_report->raise( Error_Level, Pin_Module, Read_Invalid, _number, "Read from non-analogue pin" );
				return( 0 );
			}
			return( _value );
		}
};



#endif

//
//	EOF
//
