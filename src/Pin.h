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
//	Declare a call back class for detecting pin changes
//
class PinUpdate {
	public:
		virtual void pin_change( word pin, bool value ) = 0;
};

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
		//	Our link to an upstream interested party.
		//
		PinUpdate	*_update;

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
			_update = NULL;
		}
		//
		//	Attach an interested party..
		//
		void attach( PinUpdate *party ) {
			ASSERT( _update == NULL );
			ASSERT( party != NULL );
			_update = party;
		}
		//
		//	We will implement the PIN API in the same way as the hardware..
		//
		bool get_DDR( void ) {
			return( _output );
		}
		void set_DDR( bool output ) {
			if( output != _output ) _report->report( Information_Level, Pin_Module, _number, Config_Change, "Change direction to %s", ( output? "OUT": "IN" ));
			_output = output;
		}
			
		bool get_PORT( void ) {
			return( _value );
		}
		void set_PORT( bool value ) {
			if( _output ) {
				if( value != _value ) {
					_report->report( Information_Level, Pin_Module, _number, Config_Change, "Change value to %d", ( value? 1: 0 ));
					_value = value;
					if( _update ) _update->pin_change( _number, _value );
				}
			}
			else {
				if( value != _pullup ) _report->report( Information_Level, Pin_Module, _number, Config_Change, "Change pullup %s", ( value? "ON": "OFF" ));
				_pullup = value;
			}
		}

		bool get_PIN( void ) {
			return( _value );
		}
		void set_PIN( bool value ) {
			if( value ) {
				_value != _value;
				_report->report( Information_Level, Pin_Module, _number, Config_Change, "Toggle value to %d", ( _value? 1: 0 ));
				if( _update ) _update->pin_change( _number, _value );
			}
		}
		//
		//	Finally get and set the "input" reading onto the pin.
		//
		bool get_value( void ) {
			return( _value );
		}
		void set_value( bool value ) {
			if( !_output ) {
				if( _value != value ) {
					_value = value;
					if( _update ) _update->pin_change( _number, _value );
				}
			}
		}
};



#endif

//
//	EOF
//
