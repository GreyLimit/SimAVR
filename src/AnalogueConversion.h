//
//	AnalogueConversion.h
//	====================
//
//	This is the simulation of the ADC system
//

#ifndef _ANALOGUE_CONVERSION_H_
#define _ANALOGUE_CONVERSION_H_

//
//	What we need..
//
#include "Base.h"
#include "Reporter.h"
#include "DeviceRegister.h"
#include "Pin.h"


//
//	Declare an empty ADC hardware device
//
class AnalogueConversion : public Notification {
	public:
		static const word ADCSRA = 0;
		
	private:
		Reporter	*_report;
		int		_instance;
	public:
		AnalogueConversion( Reporter *report, int instance ) {
			_report = _report;
			_instance = instance;
		}
		virtual byte read_register( word id ) {
			switch( id ) {
				case ADCSRA: {
					return( 0 );
				}
				default: {
					ABORT();
					break;
				}
			}
			return( 0 );
		}
		virtual void write_register( word id, byte value ) {
			switch( id ) {
				case ADCSRA: {
					break;
				}
				default: {
					ABORT();
					break;
				}
			}
		}
		virtual bool examine( word id, Symbols *labels, char *buffer, int max ) {
			switch( id ) {
				case ADCSRA: {
					snprintf( buffer, max, "ADCSRA no info" );
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
