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
		int		_instance;

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
		Map( Reporter *handler, int instance, word size ) {
			ASSERT( size > 0 );
			
			_report = handler;
			_instance = instance;
			_segments = NULL;
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
				_report->report( Error_Level, Map_Module, _instance, Overlap_Error, "New segment at $%04X outside map", (int)adrs );
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
					return( !_report->report( Error_Level, Map_Module, _instance, Overlap_Error, "New segment at $%04X overlaps existing segment", (int)adrs ));
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
			//	And confirm success.
			//
			return( true );
		}

		virtual byte read( word adrs ) {
			component *ptr;
			
			if(( ptr = find( adrs ))) return( ptr->handler->read( adrs - ptr->base ));
			_report->report( Warning_Level, Map_Module, _instance, Address_OOR, "Read address $%04X not in mapped segment", (int)adrs );
			return( 0 );
		}
		
		virtual void write( word adrs, byte value ) {
			component *ptr;
			
			if(( ptr = find( adrs ))) return( ptr->handler->write( adrs - ptr->base, value ));
			_report->report( Warning_Level, Map_Module, _instance, Address_OOR, "Write address $%04X not in mapped segment", (int)adrs );
		}
		
		virtual byte modify( word adrs, byte clear, byte set, byte toggle ) {
			component *ptr;
			
			if(( ptr = find( adrs ))) return( ptr->handler->modify( adrs - ptr->base, clear, set, toggle ));
			_report->report( Warning_Level, Map_Module, _instance, Address_OOR, "Modify address $%04X not in mapped segment", (int)adrs );
			return( 0 );
		}
		
		virtual word capacity( void ) {
			return( _size );
		}
		
		virtual bool examine( word adrs, Symbols *labels, char *buffer, int max ) {
			component *ptr;
			
			if(( ptr = find( adrs ))) return( ptr->handler->examine( adrs - ptr->base, labels, buffer, max ));
			_report->report( Warning_Level, Map_Module, _instance, Address_OOR, "Examine address $%04X not in mapped segment", (int)adrs );
			return( false );
		}
};

#endif

//
//	EOF
//
