//
//	SerialComms.h
//	====================
//
//	This is the simulation of the USART
//

#ifndef _SERIAL_COMMS_H_
#define _SERIAL_COMMS_H_

//
//	What we need..
//
#include "Base.h"
#include "Reporter.h"
#include "DeviceRegister.h"


//
//	Declare an empty USART hardware device
//
class SerialComms : public Notification {
	public:
		static const word UDRn = 0;
		static const word UCSRnA = 1;
		static const word UCSRnB = 2;
		static const word UCSRnC = 3;
		static const word UBRRnL = 4;
		static const word UBRRnH = 5;
		
	private:
		Reporter	*_report;
		int		_instance;
	public:
		SerialComms( Reporter *report, int instance ) {
			_report = _report;
			_instance = instance;
		}
		virtual byte read_register( word id ) {
			switch( id ) {
				case UDRn: {
					return( 0 );
				}
				case UCSRnA: {
					return( 0 );
				}
				case UCSRnB: {
					return( 0 );
				}
				case UCSRnC: {
					return( 0 );
				}
				case UBRRnL: {
					return( 0 );
				}
				case UBRRnH: {
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
				case UDRn: {
					break;
				}
				case UCSRnA: {
					break;
				}
				case UCSRnB: {
					break;
				}
				case UCSRnC: {
					break;
				}
				case UBRRnL: {
					break;
				}
				case UBRRnH: {
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
				case UDRn: {
					break;
				}
				case UCSRnA: {
					break;
				}
				case UCSRnB: {
					break;
				}
				case UCSRnC: {
					break;
				}
				case UBRRnL: {
					break;
				}
				case UBRRnH: {
					break;
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
