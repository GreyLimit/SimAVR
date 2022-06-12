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
			dword		starts,
					ends;
			breakpoint	*next;
		};
		//
		//	This is the active list of breakpoints
		//
		breakpoint		*_active,
					*_transient;
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
			_transient = NULL;
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
			breakpoint **a, *p;

			//
			//	Transient break points.
			//
			a = &_transient;
			while(( p = *a )) {
				if( adrs == p->starts ) {
					*a = p->next;
					p->next = _inactive;
					_inactive = p;
					return( p->index );
				}
				a = &( p->next );
			}
			//
			//	Long term break points.
			//
			for( p = _active; p != NULL; p = p->next ) {
				if(( adrs >= p->starts )&&( adrs < p->ends )) return( p->index );
			}
			return( 0 );
		}
		//
		//	Add a new transient break point on a single address.
		//
		int add( dword adrs ) {
			breakpoint *p;

			if(( p = _inactive )) {
				_inactive = _inactive->next;
			}
			else {
				p = new breakpoint;
			}
			p->index = _next++;
			p->starts = adrs;
			p->ends = adrs+1;
			p->next = _transient;
			_transient = p;
			return( p->index );
		}
		//
		//	Add a new breakpoint and return its number or 0 on failure.
		//
		//	With the addition of 'ranges' of break addresses, this mechanism
		//	has to become more complex.  A new range can over lap one (or more)
		//	existing ranges.  The solution is (actually) simple:  Any ranges
		//	that are overlapped extend the new range (if necessary) and are then
		//	deleted.  At the end of the routine the new (modified) range is
		//	created and returned.
		//
		int add( dword starts, dword ends ) {
			breakpoint	**a,
					*p;
			bool		d;

			ASSERT( starts < ends );
			a = &_active;
			while(( p = *a ) != NULL ) {
				d = false;
				if(( starts >= p->starts )&&( starts < p->ends )) {
					starts = p->starts;
					d = true;
				}
				if(( ends > p->starts )&&( ends <= p->ends )) {
					ends = p->ends;
					d = true;
				}
				if(( starts < p->starts )&&( ends > p->ends )) {
					d = true;
				}
				if( d ) {
					*a = p->next;
					p->next = _inactive;
					_inactive = p;
				}
				else {
					a = &( p->next );
				}
			}
			if(( p = _inactive )) {
				_inactive = _inactive->next;
			}
			else {
				p = new breakpoint;
			}
			p->index = _next++;
			p->starts = starts;
			p->ends = ends;
			p->next = _active;
			_active = p;
			return( p->index );
		}
		//
		//	Delete a numbered breakpoint.
		//
		bool remove( int number ) {
			breakpoint *p, **a;

			for( a = &_transient; ( p = *a ) != NULL; a = &( p->next )) {
				if( number == p->index ) {
					*a = p->next;
					p->next = _inactive;
					_inactive = p;
					return( true );
				}
			}
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
		bool address( int number, dword *starts, dword *ends ) {
			for( breakpoint *p = _transient; p != NULL; p = p->next ) {
				if( number == p->index ) {
					*starts = p->starts;
					*ends = p->ends;
					return( true );
				}
			}
			for( breakpoint *p = _active; p != NULL; p = p->next ) {
				if( number == p->index ) {
					*starts = p->starts;
					*ends = p->ends;
					return( true );
				}
			}
			return( false );
		}
		//
		//	Finally, generate a list of all current break points
		//
		int list( int *array, int max ) {
			int count = 0;

			for( breakpoint *p = _transient; ( p != NULL )&&( count < max ); p = p->next ) array[ count++ ] = p->index;
			for( breakpoint *p = _active; ( p != NULL )&&( count < max ); p = p->next ) array[ count++ ] = p->index;
			return( count );
		}
};


#endif

//
//	EOF
//
