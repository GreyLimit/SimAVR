//
//	FusesPhaseTwo.h
//	===============
//
//	Common fuses across all the (as I characterise them) the
//	phase 2 devices (328, 640, 128X, 256X).
//

#ifndef _FUSES_PHASE_TWO_H_
#define _FUSES_PHASE_TWO_H_

#include "FusesCore.h"
#include "Validation.h"

class FusesPhaseTwo : public FusesCore {
	private:
		//
		//	Encode the various boot sections sizes (in words).
		//
		//	I know this can be a static table, but (for the moment)
		//	I encode it here to enable the whole code to be placed
		//	in the header file.
		//
		word _bootsz[ RANGE( byte, size_BOOTSZ )];

		//
		//	Obtain the raw BOD value
		//
		inline byte raw_bodlevel( void ) { return( extract<byte>( _fuse[ extended_fuse_byte ], lsb_BODLEVEL, mask_BODLEVEL )); }

	public:
		FusesPhaseTwo( Reporter *report, AVR_Processor mcu ) : FusesCore( report ) {
			//
			//	Set up boot sizes.
			//
			ASSERT( RANGE( byte, size_BOOTSZ ) == 4 );
			switch( mcu ) {
				case AVR_ATmega328:
				case AVR_ATmega328P: {
					_bootsz[ b11 ] = 256;
					_bootsz[ b10 ] = 512;
					_bootsz[ b01 ] = 1024;
					_bootsz[ b00 ] = 2048;
					break;
				}
				case AVR_ATmega640:
				case AVR_ATmega1280:
				case AVR_ATmega1281:
				case AVR_ATmega2560:
				case AVR_ATmega2561: {
					_bootsz[ b11 ] = 512;
					_bootsz[ b10 ] = 1024;
					_bootsz[ b01 ] = 2048;
					_bootsz[ b00 ] = 4096;
					break;
				}
				default: {
					ABORT();
					break;
				}
			}
		}
		//
		//	Brown Out Detection
		//
		virtual bool BODLEVEL_Enabled( void ) {		// Brown-out detection enabled?
			return( raw_bodlevel() != mask_BODLEVEL );
		}
		virtual float BODLEVEL_Min( void ) {		// Minimum V_BOT
			switch( raw_bodlevel()) {
				case b111: {
					_report->raise( Warning_Level, Fuse_Module, Feature_Disabled );
					return( 0.0 );
				}
				case b110: return( 1.7 );
				case b101: return( 2.5 );
				case b100: return( 4.1 );
				default:   break;
			}
			_report->raise( Warning_Level, Fuse_Module, Feature_Reserved );
			return( 5.0 );
		}
		virtual float BODLEVEL_Typical( void ) {	// Typical V_BOT
			switch( raw_bodlevel()) {
				case b111: {
					_report->raise( Warning_Level, Fuse_Module, Feature_Disabled );
					return( 0.0 );
				}
				case b110: return( 1.8 );
				case b101: return( 2.7 );
				case b100: return( 4.3 );
				default:   break;
			}
			_report->raise( Warning_Level, Fuse_Module, Feature_Reserved );
			return( 5.0 );
		}
		virtual float BODLEVEL_Max( void ) {	// Maximum V_BOT
			switch( raw_bodlevel()) {
				case b111: {
					_report->raise( Warning_Level, Fuse_Module, Feature_Disabled );
					return( 0.0 );
				}
				case b110: return( 2.0 );
				case b101: return( 2.9 );
				case b100: return( 4.5 );
				default:   break;
			}
			_report->raise( Warning_Level, Fuse_Module, Feature_Reserved );
			return( 5.0 );
		}
		
		//
		//	Boot Area features.
		//
		virtual word BOOTSZ( void ) {	// Return number of words forming boot area
			return( _bootsz[ extract<byte>( _fuse[ high_fuse_byte ], lsb_BOOTSZ, mask_BOOTSZ )]);
		}
		virtual bool BOOTRST( void ) {
			return(( _fuse[ high_fuse_byte ] & bit_BOOTRST ) == 0 );	// Use Boot IRQ Vector?
		}
		
		//
		//	Program memory protection.
		//
		virtual bool SPM_App_Locked( void ) {
			return(( _fuse[ lock_bits ] & bit_BLB01 ) == 0 );	// Is SPM be locked out of the Application area?
		}
		virtual bool LPM_App_Locked( void ) {
			return(( _fuse[ lock_bits ] & bit_BLB02 ) == 0 );	// Is LPM (running in Boot area) locked out of the Application area?
		}
		virtual bool SPM_Boot_Locked( void ) {
			return(( _fuse[ lock_bits ] & bit_BLB11 ) == 0 );	// Is SPM be locked out of the Boot area?
		}
		virtual bool LPM_Boot_Locked( void ) {
			return(( _fuse[ lock_bits ] & bit_BLB12 ) == 0 );	// Is LPM (running in Application area) locked out of the Boot area?
		}
};
		

#endif

//
//	EOF
//
