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
//	The Pin Class
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
		//	Pin characteristics
		//
		bool		_output,
				_pullup,
				_value;

	public:
		Pin( Reporter *report, word number ) {
			_report = report;
			_number = number;
			_output = false;
			_pullup = false;
			_value = false;
		}
		//
		//	We will implement the PIN API in the same way as the hardware..
		//
		bool get_DDR( void ) {
			return( _output );
		}
		void set_DDR( bool output ) {
			if( output != _output ) _report->raise( Error_Level, Pin_Module, Config_Change, _number, "Change direction", ( output? 1: 0 ));
			_output = output;
		}
			
		bool get_PORT( void ) {
			return( _value );
		}
		void set_PORT( bool value ) {
			if( _output ) {
				if( value != _value ) _report->raise( Error_Level, Pin_Module, Config_Change, _number, "Change value", ( value? 1: 0 ));
				_value = value;
			}
			else {
				if( value != _pullup ) _report->raise( Error_Level, Pin_Module, Config_Change, _number, "Change pullup", ( value? 1: 0 ));
				_pullup = value;
			}
		}

		bool get_PIN( void ) {
			return( _value );
		}
		void set_PIN( bool value ) {
			if( value ) {
				_value != _value;
				_report->raise( Error_Level, Pin_Module, Config_Change, _number, "Toggle value", ( _value? 1: 0 ));
			}
		}
};



#endif

//
//	EOF
//
