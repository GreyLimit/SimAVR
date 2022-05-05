//
//	FusesPhaseOne.h
//	===============
//
//	Common fuses across all the (as I characterise them) the
//	phase 1 devices (48, 88 and 168 MCUs).
//

#ifndef _FUSES_PHASE_ONE_H_
#define _FUSES_PHASE_ONE_H_

#include "FusesCore.h"
#include "Validation.h"

class FusesPhaseOne : public FusesCore {
	private:
		//
		//	Obtain the raw BOD value
		//
		byte raw_bodlevel( void ) { return( extract<byte>( _fuse[ high_fuse_byte ], lsb_BODLEVEL, mask_BODLEVEL )); }

	public:
		FusesPhaseOne( Reporter *report ) : FusesCore( report ) {
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
		virtual bool RSTDISBL( void ) {
			return(( _fuse[ high_fuse_byte ] & bit_RSTDISBL ) == 0 );	// External reset disabled?
		}
		virtual bool DWEN( void ) {
			return(( _fuse[ high_fuse_byte ] & bit_DWEN ) == 0 );	// debugWIRE enabled?
		}
};
		

#endif

//
//	EOF
//
