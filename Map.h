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
			word		starts,
					ends;
			Memory		*handler;
			int		weight;
			component	*before,
					*after;
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
		//	This is the pre-defined maximum size of the
		//	whole mapped segment.
		//
		word		_size;

		//
		//	Find the target segment.
		//
		component *find( word adrs ) {
			component *ptr = _segments;

			while( ptr ) {
				if( adrs < ptr->starts ) {
					ptr = ptr->before;
				}
				else {
					if( adrs >= ptr->ends ) {
						ptr = ptr->after;
					}
					else {
						return( ptr );
					}
				}
			}
			return( NULL );
		}

		//
		//	Serialise a tree into increasing order.
		//
		component *serialise( component *ptr, component *tail ) {
			component *p;

			if( ptr == NULL ) return( tail );
			p = ptr->before;
			ptr->before = NULL;
			if(( ptr->after = serialise( ptr->after, tail )) != NULL ) {
				ptr->weight = ptr->after->weight + 1;
			}
			else {
				ptr->weight = 1;
			}
			return( serialise( p, ptr ));
		}
		//
		//	Rebuild a tree from a serialised list of nodes.
		//
		component *build( component *p, int offset ) {
			int		h;
			component	*b, *r, **t;

			//
			//	Simple case first.
			//
			if( p == NULL ) return( NULL );

			//
			//	Where is the half way point?
			//
			h = ( p->weight - offset ) / 2;

			//
			//	Roll off h record to the before pointer b.
			//
			t = &b;
			while( h-- ) {

				ASSERT( p != NULL );
				ASSERT( p->before == NULL );
				
				*t = p;
				t = &( p->after );
				p = p->after;
			}
			*t = NULL;

			//
			//	p is now the new root node.
			//
			ASSERT( p != NULL );
			r = p;
			p = p->after;

			//
			//	Rebuild root node.
			//
			r->before = build( b, r->weight );
			r->after = build( p, offset );
			r->weight = 1 + (( r->before != NULL )? r->before->weight: 0 ) + (( r->after != NULL )? r->after->weight: 0 );

			//
			//	Done!
			//
			return( r );
		}
		
		//
		//	Balance a tree.
		//
		component *balance( component *ptr ) {
			return( build( serialise( ptr, NULL ), 0 ));
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
			//	Just make sure our assumption about the
			//	start being before the end is true..
			//
			ASSERT( adrs < t );
			
			//
			//	Verify segment inside map space
			//
			if( t > _size ) return( !_report->report( Error_Level, Map_Module, _instance, Overlap_Error, "New segment at $%04X (%d bytes) outside map", (int)adrs, (int)z ));

			//
			//	Search segments to see where this new one fits.
			//
			a = &_segments;
			while(( p = *a )) {
				if( t <= p->starts ) {
					a = &( p->before );
				}
				else {
					if( adrs >= p->ends ) {
						a = &( p->after );
					}
					else {
						return( !_report->report( Error_Level, Map_Module, _instance, Overlap_Error, "New segment at $%04X overlaps existing segment", (int)adrs ));
					}
				}
			}
			//
			//	Create and insert a new record at a.
			//
			p = new component;
			p->starts = adrs;
			p->ends = t;
			p->handler = block;
			p->weight = 1;
			p->before = NULL;
			p->after = NULL;
			*a = p;
			//
			//	re-balance tree (re evaluates weights)
			//
			_segments = balance( _segments ); 
			//
			//	And confirm success.
			//
			return( true );
		}

		virtual byte read( word adrs ) {
			component *ptr;
			
			if(( ptr = find( adrs ))) return( ptr->handler->read( adrs - ptr->starts ));
			_report->report( Warning_Level, Map_Module, _instance, Address_OOR, "Read address $%04X not in mapped segment", (int)adrs );
			return( 0 );
		}
		
		virtual void write( word adrs, byte value ) {
			component *ptr;
			
			if(( ptr = find( adrs ))) return( ptr->handler->write( adrs - ptr->starts, value ));
			_report->report( Warning_Level, Map_Module, _instance, Address_OOR, "Write address $%04X not in mapped segment", (int)adrs );
		}
		
		virtual byte modify( word adrs, byte clear, byte set, byte toggle ) {
			component *ptr;
			
			if(( ptr = find( adrs ))) return( ptr->handler->modify( adrs - ptr->starts, clear, set, toggle ));
			_report->report( Warning_Level, Map_Module, _instance, Address_OOR, "Modify address $%04X not in mapped segment", (int)adrs );
			return( 0 );
		}
		
		virtual word capacity( void ) {
			return( _size );
		}
		
		virtual bool examine( word adrs, Symbols *labels, char *buffer, int max ) {
			component *ptr;
			
			if(( ptr = find( adrs ))) return( ptr->handler->examine( adrs - ptr->starts, labels, buffer, max ));
			_report->report( Warning_Level, Map_Module, _instance, Address_OOR, "Examine address $%04X not in mapped segment", (int)adrs );
			return( false );
		}
};

#endif

//
//	EOF
//
