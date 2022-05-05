//
//	DeviceRegister.h
//
//	This is, effectively, a redirection mechanism allowing
//	the object implementing a device to receive and control
//	the content of a IO location (though as a memory derived
//	class it would work anywhere).
//

#ifndef _DEVICE_REGISTER_H_
#define _DEVICE_REGISTER_H_

//
//	Device Register is derived from Memory.
//
#include "Memory.h"
#include "Validation.h"

//
//	Define a notification class through which updates to the
//	register are forwarded to a supervising device object.
//
class Notification {
	public:
		//
		//	These API routines provide the  interface
		//	back from the IO register to the device
		//	itself.
		//
		virtual byte read_register( word id ) = 0;
		virtual void write_register( word id, byte value ) = 0;
};

//
//	You can only read or write to a bit register, parameters
//	details how the bits work.
//
//		id	Internal identification for the bit register.
//
class DeviceRegister : public Memory {
	private:
		//
		//	Where we go to for supervisory actions.
		//
		Notification	*_control;

		//
		//	Keep copy of our internal identification.
		//
		word		_id;
				
	public:
		//
		//	Constructor, do not allocate 64 KBytes RAM,
		//	this constructor will fail!
		//
		DeviceRegister( Notification *supervisor, word id ) {
			_control = supervisor;
			_id = id;
		}
		//
		//	Simple read or write actions
		//
		virtual byte read( word adrs ) {
			ASSERT( adrs == 0 );
			return( _control->read_register( _id ));
		}
		virtual void write( word adrs, byte value ) {
			ASSERT( adrs == 0 );
			_control->write_register( _id, value );
		}
		//
		//	Modify action enables a read then adjust
		//	a memory location.
		//
		//	The value returned is the data at the address
		//	before the modification (v).  The data at the
		//	address after the modification is given by
		//	the following:
		//
		//		new value = (( v & ~clear ) | set ) ^ toggle;
		//
		virtual byte modify( word adrs, byte clear, byte set, byte toggle ) {
			byte	v;
			
			ASSERT( adrs == 0 );
			v = _control->read_register( _id );
			_control->write_register( _id, ((( v & ~clear ) | set ) ^ toggle ));
			return( v );
		}
		//
		//	Query memory features.
		//
		virtual word capacity( void ) {
			return( 1 );
		}
};	

#endif

//
//	EOF
//
