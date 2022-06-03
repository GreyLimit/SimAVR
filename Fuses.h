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

#include "Base.h"
#include "Symbols.h"

//
//	This is a complex array of common and not common features across
//	AVR chips.  The following virtual class provides a chip independent
//	mechanism to query the features controlled by the locks/fuses and
//	read/write the underlying controlling bits.
//

class Fuses {
	private:
		//
		//	File reading buffer size and argument sizes.
		//
		static const int buffer_size = 80;
		static const int arg_size = 40;
		
	public:
		//
		//	Read/write a fuse byte..
		//
		virtual byte read( word adrs ) = 0;
		virtual void write( word adrs, byte value ) = 0;
		
		//
		//	Burn a fuse byte.  Only to be used
		//	by the file reading routine.
		//
		virtual bool burn( word adrs, byte value ) = 0;
		
		//
		//	Read a signature byte.
		//
		virtual byte read_sig( word adrs ) = 0;

		//
		//	Decode fuse name
		//	================
		//
		//	This, and derived routines,  will between them
		//	convert a textual fuse name into a set of values
		//	representing the fuse byte number, least significant
		//	bit number and bit mask for the fuse.  Returns
		//	true if the name is identified, false other wise.
		//
		//	This the "tail end charlie" of the tail recsursive
		//	system, and always returns false, name unrecognised.
		//
		virtual bool decode( const char *name, byte *number, byte *lsb, byte *mask ) {
			ASSERT( name != NULL );
			ASSERT( number != NULL );
			ASSERT( lsb != NULL );
			ASSERT( mask != NULL );			
			return( false );
		}

		//
		//	Load fuses from a file.
		//	=======================
		//
		bool load_fuses( const char *file, Symbols *labels ) {
			FILE	*in;
			char	buffer[ buffer_size ],
				name[ arg_size ],
				expr[ arg_size ];
			int	line, problems;

			if( file == NULL ) return( true );
			if(( in = fopen( file, "r" )) == NULL ) return( false );
			problems = 0;
			line = 0;
			while( fgets( buffer, buffer_size, in )) {
				ASSERT( strlen( buffer ) > 1 );
				buffer[ strlen( buffer ) - 1 ] = EOS;
				line++;
				if( buffer[ 0 ] != '#' ) {
					if( sscanf( buffer, "%s%s", name, expr ) == 2 ) {
						byte	number,
							lsb,
							mask;
						
						if( decode( name, &number, &lsb, &mask )) {
							dword	value;
							byte	v;

							ASSERT( lsb < 8 );
							if( labels->evaluate( bit_constant, expr, &value )) {
								if( value > mask ) {
									fprintf( stderr, "Fuse value '%d' too big, line %d.", (int)value, line );
									problems++;
								}
								else {
									v = read( number );
									v = ( v & ~( mask << lsb ))|( (byte)value << lsb );
									if( !burn( number, v )) {
										fprintf( stderr, "Burn fuse '%s' failed, line %d.", name, line );
										problems++;
									}
								}
							}
							else {
								fprintf( stderr, "Fuse value '%s' unrecognised, line %d.", expr, line );
								problems++;
							}
						}
						else {
							fprintf( stderr, "Fuse name '%s' unrecognised, line %d.\n", name, line );
							problems++;
						}	
					}
					else {
						fprintf( stderr, "Fuse file line %d unrecognised format.\n", line );
						problems++;
					}
				}
			}
			return( problems == 0 );
		}
		
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
