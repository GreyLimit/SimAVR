//
//	Fuses.h
//	=======
//
//	An abstraction of fuses which the
//	system maintains and uses to configure
//	elements of the hardware.
//

#ifndef _FUSES_H_
#define _FUSES_H_

//
//	This is a complex array of common and not common features across
//	AVR chips.  The following virtual class provides a chip independent
//	mechanism to query the features controlled by the locks/fuses and
//	read/write the underlying controlling bits.
//

class Fuses {
	public:
		//
		//	Read/write a fuse byte..
		//
		virtual byte read( word adrs ) = 0;
		virtual void write( word adrs, byte value ) = 0;
		//
		//	Read a signature byte.
		//
		virtual byte read_sig( word adrs ) = 0;
		//
		//	Access status of a features
		//	===========================
		//
		virtual bool CKDIV8( void ) = 0;		// Clock Divided by 8?
		virtual bool CKOUT( void ) = 0;			// Clock output to port B
		virtual byte SUT( void ) = 0;			// Start-up time required
		virtual byte CKSEL( void ) = 0;			// Clock division factor
		//
		virtual bool Locks_Locked( void ) = 0;		// Have the fuses and locks been locked?
		virtual bool Flash_Locked( void ) = 0;		// Has the flash (and EEPROM) memory been locked?
		virtual bool SPM_App_Locked( void ) = 0;	// Is SPM be locked out of the Application area?
		virtual bool LPM_App_Locked( void ) = 0;	// Is LPM (running in Boot area) locked out of the Application area?
		virtual bool SPM_Boot_Locked( void ) = 0;	// Is SPM be locked out of the Boot area?
		virtual bool LPM_Boot_Locked( void ) = 0;	// Is LPM (running in Application area) locked out of the Boot area?
		//
		virtual bool BODLEVEL_Enabled( void ) = 0;	// Brown-out detection enabled?
		virtual float BODLEVEL_Min( void ) = 0;		// Minimum V_BOT
		virtual float BODLEVEL_Typical( void ) = 0;	// Typical V_BOT
		virtual float BODLEVEL_Max( void ) = 0;		// Maximum V_BOT
		//
		virtual word BOOTSZ( void ) = 0;		// Return number of words forming boot area
		virtual bool BOOTRST( void ) = 0;		// Use Boot IRQ Vector?
		//
		virtual bool RSTDISBL( void ) = 0;		// External reset disabled?
		virtual bool DWEN( void ) = 0;			// debugWIRE enabled?
		virtual bool OCDEN( void ) = 0;			// On-Chip Debugger Enabled?
		virtual bool JTAGEN( void ) = 0;		// Joint Test Action Group support enabled?
		virtual bool SPIEN( void ) = 0;			// SPI program and data downloading enabled?
		virtual bool WDTON( void ) = 0;			// Watchdog timer enabled?
		virtual bool EESAVE( void ) = 0;		// EEPROM memory is preserved through the chip erase?
};
		

#endif

//
//	EOF
//
