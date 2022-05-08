//
//	BreakPoint.h
//	============
//
//
//	A simple class for managing storage and checking of breakpoints.
//

#ifndef _BREAK_POINT_H_
#define _BREAK_POINT_H_

#include "Base.h"

class BreakPoint {
	private:
		//
		//	Define the structure we will use to keep breakpoints
		//
		struct breakpoint {
			int		index;
			dword		address;
			breakpoint	*next;
		};
		//
		//	This is the active list of breakpoints
		//
		breakpoint		*_active;
		//
		//	This is the list of reusable records.
		//
		breakpoint		*_inactive;
		//
		//	This is the next available breakpoint
		//	number.
		//
		int			_next;

	public:
		BreakPoint( void ) {
			_active = NULL;
			_inactive = NULL;
			_next = 1;
		}
		//
		//	Define the basic breakpoint API.
		//
		//
		//	Check an address, return its break point number
		//	(1..n) if this is a valid break point or 0 if not.
		//
		int check( dword adrs ) {
			for( breakpoint *p = _active; p != NULL; p = p->next ) if( adrs == p->address ) return( p->index );
			return( 0 );
		}
		//
		//	Add a new breakpoint and return its number
		//	or 0 on failure.
		//
		int add( dword adrs ) {
			breakpoint *p;
			
			for( p = _active; p != NULL; p = p->next ) if( adrs == p->address ) return( p->index );
			p = new breakpoint;
			p->index = _next++;
			p->address = adrs;
			p->next = _active;
			_active = p;
			return( p->index );
		}
		//
		//	Delete a numbered breakpoint.
		//
		bool remove( int number ) {
			breakpoint *p, **a;

			for( a = &_active; ( p = *a ) != NULL; a = &( p->next )) {
				if( number == p->index ) {
					*a = p->next;
					p->next = _inactive;
					_inactive = p;
					return( true );
				}
			}
			return( false );
		}
		//
		//	Return the address associated with
		//	a numbered break point.  Returns
		//	~0 on error (rather than 0, which is
		//	a valid address).
		//
		dword address( int number ) {
			for( breakpoint *p = _active; p != NULL; p = p->next ) if( number == p->index ) return( p->address );
			return( ~0 );
		}
		//
		//	Finally, generate a list of all current break points
		//
		int list( int *array, int max ) {
			int count = 0;

			for( breakpoint *p = _active; ( p != NULL )&&( count < max ); p = p->next ) array[ count++ ] = p->index;
			return( count );
		}
};


#endif

//
//	EOF
//
