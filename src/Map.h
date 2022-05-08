//
//	Map.h
//
//	Define the class which manages the memory mapping facility
//	of a system.
//
//	This is handed a number of memory based objects and fits them
//	into a structured address map through which they can be accessed.
//

#ifndef _MAP_H_
#define _MAP_H_

//
//	Bring in the Memory definition.
//
#include "Memory.h"
#include "Reporter.h"

//
//	Define the template map class.
//
class Map : public Memory {
	public:
		//
		//	The Map API
		//	===========
		//
		virtual bool segment( Memory *block, word adrs ) = 0;
		
		//
		//	Memory API
		//	==========
		//
		virtual byte read( word adrs ) = 0;
		virtual void write( word adrs, byte value ) = 0;
		virtual byte modify( word adrs, byte clear, byte set, byte toggle ) = 0;
		virtual word capacity( void ) = 0;
};


//
//	Define the template map class.
//
template< int instance >class MapSegments : public Memory {
	private:
		//
		//	Internal Map structure.
		//
		struct component {
			word		base,
					size;
			Memory		*handler;
			component	*next;
		};

		//
		//	Map elements.
		//
		component	*_segments;

		//
		//	Error handling.
		//
		Reporter	*_report;

		//
		//	Find the target segment...
		//
		component *find( word adrs ) {
			component *ptr;

			for( ptr = _segments; ptr != NULL; ptr = ptr->next ) {
				if(( adrs >= ptr->base )&&(( adrs - ptr->base ) < ptr->size )) return( ptr );
			}
			return( NULL );
		}

	public:
		//
		//	Constructor.
		//
		MapSegments( Reporter *handler ) {
			_report = handler;
			_segments = NULL;
		}

		//
		//	Insert section into memory map.
		//
		virtual bool segment( Memory *block, word adrs ) {
			component	*p, **a;
			
			word		z = block->capacity();
			word		t = adrs + z;

			for( a = &_segments; ( p = *a ) != NULL; a = &( p->next )) {
				if(( adrs < p->base )&&( t <= p->base )) {
					//
					//	The whole of this block comes before
					//	the block p, so we can add here.
					//
					break;
				}
				if( adrs < ( p->base + p->size )) {
					//
					//	The start of this block is inside the
					//	block at p, so we definitely cannot add
					//	this block.
					//
					return( !_report->raise( Error_Level, Map_Module, Overlap_Error, instance, "New segments overlaps existing segment", adrs ));
				}
			}
			//
			//	Create and insert a new record at a.
			//
			p = new component;
			p->base = adrs;
			p->size = z;
			p->handler = block;
			p->next = *a;
			*a = p;
			return( true );
		}
		
		//
		//	Class Memory
		//	------------
		//
		virtual byte read( word adrs ) {
			component *ptr;
			
			if(( ptr = find( adrs ))) return( ptr->handler->read( adrs - ptr->base ));
			_report->raise( Warning_Level, Map_Module, Address_OOR, instance, "Address not in mapped segment", adrs );
			return( 0 );
		}
		virtual void write( word adrs, byte value ) {
			component *ptr;
			
			if(( ptr = find( adrs ))) return( ptr->handler->write( adrs - ptr->base, value ));
			_report->raise( Warning_Level, Map_Module, Address_OOR, instance, "Address not in mapped segment", adrs );
		}
		virtual byte modify( word adrs, byte clear, byte set, byte toggle ) {
			component *ptr;
			
			if(( ptr = find( adrs ))) return( ptr->handler->modify( adrs - ptr->base, clear, set, toggle ));
			_report->raise( Warning_Level, Map_Module, Address_OOR, instance, "Address not in mapped segment", adrs );
			return( 0 );
		}
		virtual word capacity( void ) {
			component *l;
			
			if( _segments == NULL ) return( 0 );
			for( l = _segments; l->next != NULL; l = l->next );
			return(( l->base - _segments->base ) + l->size );
		}
};

#endif

//
//	EOF
//
