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
template< word elements > class Map : public Memory {
	private:
		//
		//	Internal Map structure.
		//
		struct component {
			word	base,
				size;
			Memory	*handler;
		};

		//
		//	Map elements.
		//
		component	_segment[ elements ];
		word		_defined;
		byte		_bad_memory;

		//
		//	Error handling.
		//
		Reporter	*_report;

		//
		//	Find the target segment...
		//
		component *find( word adrs ) {
			component *ptr;

			ptr = _segment;
			for( word i = 0; i < _defined; i++, ptr++ ) if(( adrs >= ptr->base )&&(( adrs - ptr->base ) < ptr->size )) return( ptr );
			return( NULL );
		}

	public:
		//
		//	Constructor.
		//
		Map( Reporter *handler ) {
			_report = handler;
			_defined = 0;
			_bad_memory = 0;
		}

		//
		//	Insert section into memory map.
		//
		virtual bool segment( Memory *handler, word adrs ) {
			if( _defined >= elements ) return( false );
			_segment[ _defined ].base = adrs;
			_segment[ _defined ].size = handler->capacity();
			_segment[ _defined ].handler = handler;
			_defined++;
			return( true );
		}
		
		//
		//	Class Memory
		//	------------
		//
		virtual byte read( word adrs ) {
			component *ptr;
			
			if(( ptr = find( adrs ))) return( ptr->handler->read( adrs - ptr->base ));
			_report->raise( Warning_Level, Map_Module, Address_OOR, adrs );
			return( 0 );
		}
		virtual void write( word adrs, byte value ) {
			component *ptr;
			
			if(( ptr = find( adrs ))) return( ptr->handler->write( adrs - ptr->base, value ));
			_report->raise( Warning_Level, Map_Module, Address_OOR, adrs );
		}
		virtual byte modify( word adrs, byte clear, byte set, byte toggle ) {
			component *ptr;
			
			if(( ptr = find( adrs ))) return( ptr->handler->modify( adrs - ptr->base, clear, set, toggle ));
			_report->raise( Warning_Level, Map_Module, Address_OOR, adrs );
			return( 0 );
		}
		virtual word capacity( void ) {
			component *best;
			
			switch( _defined ) {
				case 0: {
					return( 0 );
				}
				case 1: {
					best = _segment;
					break;
				}
				default: {
					component *ptr;

					best = &( _segment[ 0 ]);
					ptr = &( _segment[ 1 ]);
					for( word i = 1; i < _defined; i++, ptr++ ) {
						if( ptr->base > best->base ) best = ptr;
					}
					break;
				}
			}
			return( best->base + best->size );
		}
};

#endif

//
//	EOF
//
