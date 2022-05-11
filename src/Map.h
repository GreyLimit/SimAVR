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
		virtual void shifted( word offset ) = 0;
		
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
		//	This is default base address of the map.
		//
		word		_base;

		//
		//	This is the pre-defined size of this map
		//	segment.
		//
		word		_size;

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
		MapSegments( Reporter *handler, word size ) {
			ASSERT( size > 0 );
			
			_report = handler;
			_segments = NULL;
			_base = 0;
			_size = size;
		}

		//
		//	Insert section into memory map.
		//
		virtual bool segment( Memory *block, word adrs ) {
			component	*p, **a;
			
			word		z = block->capacity();
			word		t = adrs + z;

			//
			//	Verify segment inside map space
			//
			if( t > _size ) {
				_report->raise( Error_Level, Map_Module, Overlap_Error, instance, "New segment extends pas end of map", adrs );
				return( false );
			}
			//
			//	Search segments to see where this new on fits.
			//
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
			//	Create and insert a new record inside a.
			//
			p = new component;
			p->base = adrs;
			p->size = z;
			p->handler = block;
			p->next = *a;
			*a = p;
			//
			//	Finally tell it what our base address is
			//
			block->shifted( adrs );
			//
			//	And confirm success.
			//
			return( true );
		}
		virtual void shifted( word offset ) {
			_report->raise( Information_Level, Map_Module, Config_Change, instance, "Base address shifted", offset );
			_base += offset;
			for( component	*p = _segments; p != NULL; p = p->next ) {
				p->handler->shifted( offset );
			}
		}
		
		//
		//	Class Memory
		//	------------
		//
		virtual byte read( word adrs ) {
			component *ptr;
			
			if(( ptr = find( adrs ))) return( ptr->handler->read( adrs - ptr->base ));
			_report->raise( Warning_Level, Map_Module, Address_OOR, instance, "Read address not in mapped segment", adrs );
			return( 0 );
		}
		virtual void write( word adrs, byte value ) {
			component *ptr;
			
			if(( ptr = find( adrs ))) return( ptr->handler->write( adrs - ptr->base, value ));
			_report->raise( Warning_Level, Map_Module, Address_OOR, instance, "Write address not in mapped segment", adrs );
		}
		virtual byte modify( word adrs, byte clear, byte set, byte toggle ) {
			component *ptr;
			
			if(( ptr = find( adrs ))) return( ptr->handler->modify( adrs - ptr->base, clear, set, toggle ));
			_report->raise( Warning_Level, Map_Module, Address_OOR, instance, "Modify address not in mapped segment", adrs );
			return( 0 );
		}
		virtual word capacity( void ) {
			
			return( _size );
		}
};

#endif

//
//	EOF
//
