//
//	Port.h
//	======
//
//	Define a concentration of pins into a port.
//

#ifndef _PORT_H_
#define _PORT_H_

#include "Base.h"
#include "Reporter.h"
#include "Pin.h"
#include "DeviceRegister.h"

//
//	The generic port API
//
class Port : public Notification {
	public:
		//
		//	Registers come in three flavours:
		//
		static const word PINn = 0;
		static const word DDRn = 1;
		static const word PORTn = 2;

		//
		//	The device register interface.
		//
		virtual byte read_register( word id ) = 0;
		virtual void write_register( word id, byte value ) = 0;
};

//
//	The actual, configurable, port device.
//
template< word instance > class PortDevice : public Port {
	public:
		//
		//	The device register interface.
		//
		virtual byte read_register( word id ) {
			return( 0 );
		}
		virtual void write_register( word id, byte value ) {
		}
};

#endif

//
//	EOF
//
