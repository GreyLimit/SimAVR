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
		virtual void tick( void ) = 0;
};

//
//	Clock API class
//
class Clock {
	public:
		//
		//	Add a new target to the clock.
		//
		virtual bool add( Tick *dev ) = 0;

		//
		//	Call with the number of ticks which
		//	are supposed to be simulated.
		//
		virtual void tick( word count ) = 0;
		
		//
		//	Call to convert a number of milliseconds
		//	into a corresponding number of clock
		//	ticks.
		//
		//	Due to the types used in this class (at the
		//	moment) this is ultimately limited to a
		//	fairly small duration (depending on the
		//	clock speed being simulated).
		//
		virtual word millis( word duration ) = 0;
		
		//
		//	As above, but for microseconds.
		//
		virtual word micros( word duration ) = 0;

		//
		//	Return how many ticks we have handled.
		//
		virtual dword count( void ) = 0;
};

//
//	The Clock implementation
//
template< word targets> class ClockFrequency : public Clock {
	private:
		//
		//	Define an array of Tick objects we
		//	need to propergate to.
		//
		Tick	*_target[ targets ];
		//
		//	How many defined so far?
		//
		word	_targets;
		
		//
		//	Clock speed in KHz, and the practical
		//	largest millisecond duration we can
		//	handle.
		//
		word	_khs,
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
		ClockFrequency( word khz ) {
			//
			//	
			_targets = 0;
			_count = 0;
			//
			//	Save clock speed and pertinent limits.
			//
			_khs = khz;
			_max = 0xFFFF / _khs;
		}

		//
		//	Add a new target to the clock.
		//
		virtual bool add( Tick *dev ) {
			if( _targets >= targets ) return( false );
			_target[ _targets++ ] = dev;
			return( true );
		}

		//
		//	Call with the number of ticks which
		//	are supposed to be simulated.
		//
		virtual void tick( word count ) {
			while( count-- ) {
				_count++;
				for( word i = 0; i < _targets; _target[ i++ ]->tick());
			}
		}
		
		//
		//	Convert ms to clock ticks.
		//
		virtual word millis( word duration ) {
			ASSERT( duration <= _max );
			return( duration * _khs );
		}

		//
		//	Convert us to clock ticks.
		//
		virtual word micros( word duration ) {
			return( mul_div<word>( duration, _khs, 1000 ));
		}

		//
		//	Return how many ticks we have handled.
		//
		virtual dword count( void ) {
			return( _count );
		}
};


#endif

//
//	EOF
//
