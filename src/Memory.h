//
//	Memory.h
//
//	Define the generic memory interface API.  This
//	is the basis for all memory IO hardware elements.
//

#ifndef _MEMORY_H_
#define _MEMORY_H_

//
//	Base line is required.
//
#include "Base.h"
#include "Validation.h"
#include "Symbols.h"

//
//	You can only read or write to a memory location
//
class Memory {
	public:
		//
		//	Simple read or write actions
		//
		virtual byte read( word adrs ) = 0;
		virtual void write( word adrs, byte value ) = 0;
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
		virtual byte modify( word adrs, byte clear, byte set, byte toggle ) = 0;
		//
		//	Query memory features.
		//
		virtual word capacity( void ) = 0;

		//
		//	Insert new segment into mapped address space.
		//
		//	Default routine simply fails providing the safe default
		//	action.
		//
		virtual bool segment( Memory *handler, word adrs ) { return( false ); }

		//
		//	Mechanism for examining content outside the
		//	framework of the simulation.
		//
		virtual bool examine( word adrs, Symbols *labels, char *buffer, int max ) = 0;
};

#endif

//
//	EOF
//
