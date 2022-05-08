//
//	Fuses_48.h
//	==========
//
//	Fuses Specific to the '48' chips.
//

#ifndef _FUSES_48_H_
#define _FUSES_48_H_


#include "FusesPhaseOne.h"
#include "Validation.h"

class Fuses_48 : public FusesPhaseOne {
	private:
		//
		//	Define this as a private feature as the simulation
		//	will always use the four defined program protection
		//	API routines.
		//
		bool SPMEN( void ) {
			return(( _fuse[ extended_fuse_byte ] & bit_SPMEN ) == 0 ); // Self programming enable?
		}
	public:
		Fuses_48( Reporter *report, AVR_Processor mcu ) : FusesPhaseOne( report ) {
			//
			//	Set up the signature bytes.
			//
			switch( mcu ) {
				case AVR_ATmega48A: {
					_sig[ device_sig_1 ] = 0x1E;
					_sig[ device_sig_2 ] = 0x92;
					_sig[ device_sig_3 ] = 0x05;
					break;
				}
				case AVR_ATmega48PA: {
					_sig[ device_sig_1 ] = 0x1E;
					_sig[ device_sig_2 ] = 0x92;
					_sig[ device_sig_3 ] = 0x0A;
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

			if( strcmp( name, "SPMEN" ) == 0 ) {
				*number = extended_fuse_byte;
				*lsb = lsb_SPMEN;
				*mask = mask_SPMEN;
				return( true );
			}
			//
			//	Tail recursion it we fail to match.
			//
			return( FusesPhaseOne::decode( name, number, lsb, mask ));		
		}
		
		//
		//	Program memory protection.
		//
		virtual bool SPM_App_Locked( void ) { return( SPMEN()); }	// Is SPM be locked out of the Application area?
		virtual bool LPM_App_Locked( void ) { return( SPMEN()); }	// Is LPM (running in Boot area) locked out of the Application area?
		virtual bool SPM_Boot_Locked( void ) { return( SPMEN()); }	// Is SPM be locked out of the Boot area?
		virtual bool LPM_Boot_Locked( void ) { return( SPMEN()); }	// Is LPM (running in Application area) locked out of the Boot area?
		//
		virtual word BOOTSZ( void ) { return( 0 ); }			// Return number of words forming boot area
		virtual bool BOOTRST( void ) { return( false ); }		// Use Boot IRQ Vector?
		//
		virtual bool OCDEN( void ) { return( false ); }			// On-Chip Debugger Enabled?
		virtual bool JTAGEN( void ) { return( false ); }		// Joint Test Action Group support enabled?
};
		

#endif

//
//	EOF
//
