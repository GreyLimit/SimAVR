//
//	SRAM.h
//
//	Static Random Access Memory.
//

#ifndef _SRAM_H_
#define _SRAM_H_

//
//	SRAM is derived from Memory and uses Exceptions.
//
#include "Memory.h"
#include "Reporter.h"

//
//	You can only read or write to a memory location
//
template< word size > class SRAM : public Memory {
	private:
		//
		//	Where we send errors..
		//
		Reporter	*_report;
		
		//
		//	The Memory area and a single byte used to
		//	handle out of range 'at' requests.
		//
		byte		_ram[ size ],
				_x;
		
	public:
		//
		//	Constructor, do not allocate 64 KBytes RAM,
		//	this constructor will fail!
		//
		SRAM( Reporter *handler ) {
			_report = handler;
			for( word i = 0; i < size; _ram[ i++ ]);
		}
		//
		//	Simple read or write actions
		//
		virtual byte read( word adrs ) {
			if( adrs >= size ) {
				_report->raise( Error_Level, SRAM_Module, Address_OOR, adrs );
				return( 0 );
			}
			return( _ram[ adrs ]);
		}
		virtual void write( word adrs, byte value ) {
			if( adrs >= size ) {
				_report->raise( Error_Level, SRAM_Module, Address_OOR, adrs );
				return;
			}
			_ram[ adrs ] = value;
		}
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
		virtual byte modify( word adrs, byte clear, byte set, byte toggle ) {
			byte	v;
			
			if( adrs >= size ) {
				_report->raise( Error_Level, SRAM_Module, Address_OOR, adrs );
				return( 0 );
			}
			v = _ram[ adrs ];
			_ram[ adrs ] = (( v & ~clear ) | set ) ^ toggle;
			return( v );
		}
		//
		//	Query memory features.
		//
		virtual word capacity( void ) {
			return( size );
		}

		//
		//	Mechanism for examining content outside the
		//	framework of the simulation.
		//
		virtual bool examine( word adrs, Symbols *labels, char *buffer, int max ) {
			if( adrs >= size ) return( false );
			snprintf( buffer, max, "SRAM[%04X]=%02X", (int)adrs, (int)_ram[ adrs ]);
			return( true );
		}

};	

#endif

//
//	EOF
//
