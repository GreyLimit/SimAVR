//
//	Fuses_88_168.h
//	==============
//
//	Fuses Specific to the '88' and '168' chips.
//

#ifndef _FUSES_88_168_H_
#define _FUSES_88_168_H_


#include "FusesPhaseOne.h"
#include "Validation.h"

class Fuses_88_168 : public FusesPhaseOne {
	private:
		//
		//	Encode the various boot sections sizes (in words).
		//
		//	I know this can be a static table, but (for the moment)
		//	I encode it here to 
		//
		word _bootsz[ RANGE( byte, size_BOOTSZ )];
		
	public:
		Fuses_88_168( Reporter *report, int instance, AVR_Processor mcu ) : FusesPhaseOne( report, instance ) {
			//
			//	Set up the signature bytes.
			//
			switch( mcu ) {
				case AVR_ATmega88A: {
					_sig[ device_sig_1 ] = 0x1E;
					_sig[ device_sig_2 ] = 0x93;
					_sig[ device_sig_3 ] = 0x0A;
					break;
				}
				case AVR_ATmega88PA: {
					_sig[ device_sig_1 ] = 0x1E;
					_sig[ device_sig_2 ] = 0x93;
					_sig[ device_sig_3 ] = 0x0F;
					break;
				}
				case AVR_ATmega168A: {
					_sig[ device_sig_1 ] = 0x1E;
					_sig[ device_sig_2 ] = 0x94;
					_sig[ device_sig_3 ] = 0x06;
					break;
				}
				case AVR_ATmega168PA: {
					_sig[ device_sig_1 ] = 0x1E;
					_sig[ device_sig_2 ] = 0x94;
					_sig[ device_sig_3 ] = 0x0B;
					break;
				}
				default: {
					ABORT();
					break;
				}
			}
			//
			//	Set up boot sizes.
			//
			ASSERT( RANGE( byte, size_BOOTSZ ) == 4 );
			_bootsz[ b11 ] = 128;
			_bootsz[ b10 ] = 256;
			_bootsz[ b01 ] = 512;
			_bootsz[ b00 ] = 1024;
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

			
			if( strcmp( name, "OCDEN" ) == 0 ) {
				*number = high_fuse_byte;
				*lsb = lsb_OCDEN;
				*mask = mask_OCDEN;
				return( true );
			}
			if( strcmp( name, "JTAGEN" ) == 0 ) {
				*number = high_fuse_byte;
				*lsb = lsb_JTAGEN;
				*mask = mask_JTAGEN;
				return( true );
			}
			//
			//	Tail recursion it we fail to match.
			//
			return( FusesPhaseOne::decode( name, number, lsb, mask ));		
		}

		//
		virtual bool RSTDISBL( void ) {
			return( false );					// External reset disabled?
		}
		virtual bool DWEN( void ) {
			return( false );					// debugWIRE enabled?
		}
		
		//
		virtual bool OCDEN( void ) {
			return(( _fuse[ high_fuse_byte ] & bit_OCDEN ) == 0 );	// On-Chip Debugger Enabled?
		}
		virtual bool JTAGEN( void ) {
			return(( _fuse[ high_fuse_byte ] & bit_JTAGEN ) == 0 );	// Joint Test Action Group support enabled?
		}

};
		

#endif

//
//	EOF
//
