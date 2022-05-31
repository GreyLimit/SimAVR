//
//	Fuses_328.h
//	===========
//
//	Fuses Specific to the '328' chips.
//

#ifndef _FUSES_328_H_
#define _FUSES_328_H_


#include "FusesPhaseTwo.h"
#include "Validation.h"

class Fuses_328 : public FusesPhaseTwo {
	public:
		Fuses_328( Reporter *report, int instance, AVR_Processor mcu ) : FusesPhaseTwo( report, instance, mcu ) {
			//
			//	Set up the signature bytes.
			//
			switch( mcu ) {
				case AVR_ATmega328: {
					_sig[ device_sig_1 ] = 0x1E;
					_sig[ device_sig_2 ] = 0x95;
					_sig[ device_sig_3 ] = 0x14;
					break;
				}
				case AVR_ATmega328P: {
					_sig[ device_sig_1 ] = 0x1E;
					_sig[ device_sig_2 ] = 0x95;
					_sig[ device_sig_3 ] = 0x0F;
					break;
				}
				default: {
					ABORT();
					break;
				}
			}
		}
		
		//
		//	Decode fuse name
		//	================
		//
		virtual bool decode( const char *name, byte *number, byte *lsb, byte *mask ) {
			ASSERT( name != NULL );
			ASSERT( number != NULL );
			ASSERT( lsb != NULL );
			ASSERT( mask != NULL );

			if( strcmp( name, "RSTDISBL" ) == 0 ) {
				*number = high_fuse_byte;
				*lsb = lsb_RSTDISBL;
				*mask = mask_RSTDISBL;
				return( true );
			}
			if( strcmp( name, "DWEN" ) == 0 ) {
				*number = high_fuse_byte;
				*lsb = lsb_DWEN;
				*mask = mask_DWEN;
				return( true );
			}
			//
			//	Tail recursion it we fail to match.
			//
			return( FusesPhaseTwo::decode( name, number, lsb, mask ));		
		}

		//
		virtual bool RSTDISBL( void ) {
			return(( _fuse[ high_fuse_byte ] & bit_RSTDISBL ) == 0 );	// External reset disabled?
		}
		virtual bool DWEN( void ) {
			return(( _fuse[ high_fuse_byte ] & bit_DWEN ) == 0 );		// debugWIRE enabled?
		}
		
		//
		virtual bool OCDEN( void ) {
			return( false );						// On-Chip Debugger Enabled?
		}
		virtual bool JTAGEN( void ) {
			return( false );						// Joint Test Action Group support enabled?
		}

};
		

#endif

//
//	EOF
//
