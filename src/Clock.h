//
//	Clock.h
//
//	Define a class implementing the basic clock
//	mechanism with an interface class allowing
//	other objects to be 'ticked' as the clock
//	notes the passing of time.
//

#ifndef _CLOCK_H_
#define _CLOCK_H_

//
//	Include the base definitions.
//
#include "Base.h"
#include "Validation.h"
#include "DeviceRegister.h"
#include "Reporter.h"
#include "mul_div.h"

//
//	Define the API Class which the Clock class
//	will interface with.
//
class Tick {
	public:
		//
		//	Called once for every tick which the
		//	clock is simulating.
		//
		//	'handle' gives identifier used when the
		//	tick interface was registered with the clock.
		//
		//	'inst_end' is true if the clock pulse represents
		//	the last tick of an instruction.
		//
		virtual void tick( word handle, bool inst_end ) = 0;
};


//
//	The Clock implementation
//
class Clock : public Notification {
	public:
		//
		//	This is the handle of the device register access
		//	the clock supports.
		//
		static const word CLKPR = 0;
		//
		//	bit/field access.
		//
		static const byte bit_CLKPCE = BIT( byte, 7 );
		static const byte size_CLKPS = 4;
		static const byte lsb_size_CLKPS = 0;
		static const byte mask_CLKPS = MASK( byte, size_CLKPS );
		
	private:
		//
		//	Where we send reports.
		//
		Reporter	*_report;
		
		//
		//	The Clock pre-scale register
		//
		byte		_clkpr;
		
		//
		//	Define the data we need to manage each
		//	object receiving a clock tick.
		//
		struct ticking {
			Tick	*target;
			word	handle,
				interval,
				remaining;
			ticking	*next;
		};

		//
		//	This is the array of ticking targets.
		//
		ticking		*_list;
		
		//
		//	Clock speed in KHz, and the practical
		//	largest millisecond duration we can
		//	handle.
		//
		word		_khz,
				_max;

		//
		//	Keep track of ticks as they go by.
		//
		//	This is a limited mechanism.  If this is supposed
		//	to be simulating a 16MHz clock then this will wrap
		//	back to 0 after just 4.47 simulated seconds (though
		//	in real time who knows how long this might be).
		//
		dword	_count;

	public:
		//
		//	Constructor, pass in the clock speed of the system
		//	in KHz.  This limits the "top speed" of the clock
		//	to ~65MHz, which is faster than most MCUs.
		//
		Clock( Reporter *report, word khz ) {
			//
			//	Reporting to..
			//
			_report = report;
			//
			//	Start pre-scaler empty.
			//
			_clkpr = 0;
			//
			//	Start with an empty list.
			//	
			_list = NULL;
			//
			//	Our 'real world' clock counter
			//	
			_count = 0;
			//
			//	Save clock speed and pertinent limits.
			//
			_khz = khz;
			_max = 0xFFFF / _khz;
		}

		//
		//	Add a new target to the clock.
		//
		virtual bool add( word id, Tick *dev ) {
			return( add( id, dev, _khz ));
		}
		virtual bool add( word id, Tick *dev, word khz ) {
			ticking	*p;
			
			//
			//	Set up new target
			//
			p = new ticking;
			p->target = dev;
			p->handle = id;
			if(( p->interval = _khz / khz ) == 0 ) {
				_report->raise( Warning_Level, Clock_Module, Too_Fast, khz );
				p->interval = 1;
			}
			p->remaining = p->interval;
			p->next = _list;
			_list = p;
			return( true );
		}

		//
		//	Call with the number of ticks which
		//	are supposed to be simulated.
		//
		//	set 'has_end' to true if the call
		//	should emit 'inst_end' as true one
		//	the last tick.
		//
		virtual void tick( word count, bool has_end ) {
			//
			//	Loop through the number of ticks we should be
			//	simulating. 
			//
			while( count-- ) {
				_count++;
				for( ticking *p = _list; p != NULL; p = p->next ) {
					if(( p->remaining -= 1 ) == 0 ) {
						p->remaining = p->interval;
						p->target->tick( p->handle, (( count == 0 ) && has_end ));
					}
				}
			}
		}
		
		//
		//	Convert ms to clock ticks.
		//
		virtual word millis( word duration ) {
			ASSERT( duration <= _max );
			return( duration * _khz );
		}

		//
		//	Convert us to clock ticks.
		//
		virtual word micros( word duration ) {
			return( mul_div<word>( duration, _khz, 1000 ));
		}

		//
		//	Return how many ticks we have handled.
		//
		virtual dword count( void ) {
			return( _count );
		}

		//
		//	Device register notification API
		//
		virtual byte read_register( word id ) {
			ASSERT( id == CLKPR );
			return( _clkpr );
		}
		virtual void write_register( word id, byte value ) {
			ASSERT( id == size_CLKPS );
			if( value == bit_CLKPCE ) {
				_clkpr = bit_CLKPCE;
				return;
			}
			if(( value & ~mask_CLKPS ) != 0 ) {
				_report->raise( Warning_Level, Clock_Module, Parameter_Invalid, value );
				value &= mask_CLKPS;
			}
			if( _clkpr != bit_CLKPCE ) {
				_report->raise( Warning_Level, Clock_Module, Read_Only, _clkpr );
				return;
			}
			_clkpr = value;
		}
};


#endif

//
//	EOF
//
