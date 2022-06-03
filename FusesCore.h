//
//	FusesCore.h
//	===========
//
//	An implementation of the core fuses supported
//	across all the AVR MCUs.
//

#ifndef _FUSES_CORE_H_
#define _FUSES_CORE_H_



//
//	The following table captures the relationship between chips and fuse/locks bits.
//
//	Source file (generated from vendor PDF datasheets) 'AVR Fuses across the range.ods'.
//
//	Note:	To keep the layout of this information manageable a number
//		of the names have been shortened to 7 or less characters.
//		The intent of the names should still be clear.  The methods
//		in the virtual class use the complete names as appropriate.
//
//		Comments below starting with '////' are the core common fuses
//		across all processors in the AVR range.
//
//			Device	48	48PA	88	88PA	168	168PA	328	328PA	640	1280	1281	2560	2561
//	Fuse	Byte	Bit													
//	Byte	Name	Number													
////	0	Low	7	CKDIV8	CKDIV8	CKDIV8	CKDIV8	CKDIV8	CKDIV8	CKDIV8	CKDIV8	CKDIV8	CKDIV8	CKDIV8	CKDIV8	CKDIV8
////			6	CKOUT	CKOUT	CKOUT	CKOUT	CKOUT	CKOUT	CKOUT	CKOUT	CKOUT	CKOUT	CKOUT	CKOUT	CKOUT
////			5	SUT1	SUT1	SUT1	SUT1	SUT1	SUT1	SUT1	SUT1	SUT1	SUT1	SUT1	SUT1	SUT1
////			4	SUT0	SUT0	SUT0	SUT0	SUT0	SUT0	SUT0	SUT0	SUT0	SUT0	SUT0	SUT0	SUT0
////			3	CKSEL3	CKSEL3	CKSEL3	CKSEL3	CKSEL3	CKSEL3	CKSEL3	CKSEL3	CKSEL3	CKSEL3	CKSEL3	CKSEL3	CKSEL3
////			2	CKSEL2	CKSEL2	CKSEL2	CKSEL2	CKSEL2	CKSEL2	CKSEL2	CKSEL2	CKSEL2	CKSEL2	CKSEL2	CKSEL2	CKSEL2
////			1	CKSEL1	CKSEL1	CKSEL1	CKSEL1	CKSEL1	CKSEL1	CKSEL1	CKSEL1	CKSEL1	CKSEL1	CKSEL1	CKSEL1	CKSEL1
////			0	CKSEL0	CKSEL0	CKSEL0	CKSEL0	CKSEL0	CKSEL0	CKSEL0	CKSEL0	CKSEL0	CKSEL0	CKSEL0	CKSEL0	CKSEL0
//	1	Lock	7													
//			6													
//			5			BLB12	BLB12	BLB12	BLB12	BLB12	BLB12	BLB12	BLB12	BLB12	BLB12	BLB12
//			4			BLB11	BLB11	BLB11	BLB11	BLB11	BLB11	BLB11	BLB11	BLB11	BLB11	BLB11
//			3			BLB02	BLB02	BLB02	BLB02	BLB02	BLB02	BLB02	BLB02	BLB02	BLB02	BLB02
//			2			BLB01	BLB01	BLB01	BLB01	BLB01	BLB01	BLB01	BLB01	BLB01	BLB01	BLB01
////			1	LB2	LB2	LB2	LB2	LB2	LB2	LB2	LB2	LB2	LB2	LB2	LB2	LB2
////			0	LB1	LB1	LB1	LB1	LB1	LB1	LB1	LB1	LB1	LB1	LB1	LB1	LB1
//	2	Extndd	7													
//			6													
//			5													
//			4													
//			3													
//			2			BOOTSZ1	BOOTSZ1	BOOTSZ1	BOOTSZ1	BODLVL2	BODLVL2	BODLVL2	BODLVL2	BODLVL2	BODLVL2	BODLVL2
//			1			BOOTSZ0	BOOTSZ0	BOOTSZ0	BOOTSZ0	BODLVL1	BODLVL1	BODLVL1	BODLVL1	BODLVL1	BODLVL1	BODLVL1
//			0	SPMEN	SPMEN	BOOTRST	BOOTRST	BOOTRST	BOOTRST	BODLVL0	BODLVL0	BODLVL0	BODLVL0	BODLVL0	BODLVL0	BODLVL0
//	3	High	7	RSTDSBL	RSTDSBL	RSTDSBL	RSTDSBL	RSTDSBL	RSTDSBL	RSTDSBL	RSTDSBL	OCDEN	OCDEN	OCDEN	OCDEN	OCDEN
//			6	DWEN	DWEN	DWEN	DWEN	DWEN	DWEN	DWEN	DWEN	JTAGEN	JTAGEN	JTAGEN	JTAGEN	JTAGEN
////			5	SPIEN	SPIEN	SPIEN	SPIEN	SPIEN	SPIEN	SPIEN	SPIEN	SPIEN	SPIEN	SPIEN	SPIEN	SPIEN
////			4	WDTON	WDTON	WDTON	WDTON	WDTON	WDTON	WDTON	WDTON	WDTON	WDTON	WDTON	WDTON	WDTON
////			3	EESAVE	EESAVE	EESAVE	EESAVE	EESAVE	EESAVE	EESAVE	EESAVE	EESAVE	EESAVE	EESAVE	EESAVE	EESAVE
//			2	BODLVL2	BODLVL2	BODLVL2	BODLVL2	BODLVL2	BODLVL2	BOOTSZ1	BOOTSZ1	BOOTSZ1	BOOTSZ1	BOOTSZ1	BOOTSZ1	BOOTSZ1
//			1	BODLVL1	BODLVL1	BODLVL1	BODLVL1	BODLVL1	BODLVL1	BOOTSZ0	BOOTSZ0	BOOTSZ0	BOOTSZ0	BOOTSZ0	BOOTSZ0	BOOTSZ0
//			0	BODLVL0	BODLVL0	BODLVL0	BODLVL0	BODLVL0	BODLVL0	BOOTRST	BOOTRST	BOOTRST	BOOTRST	BOOTRST	BOOTRST	BOOTRST
//																
//		Sig	Name													
//		Byte														
//		0	DevSig1	0x1E	0x1E	0x1E	0x1E	0x1E	0x1E	0x1E	0x1E	0x1E	0x1E	0x1E	0x1E	0x1E
//		1	OSC Cal												
//		2	DeSig2	0x92	0x92	0x93	0x93	0x94	0x94	0x95	0x95	0x96	0x97	0x97	0x98	0x98
//		3														
//		4	DevSig3	0x05	0x0A	0x0A	0x0F	0x06	0x0B	0x14	0x0F	0x08	0x03	0x04	0x01	0x02
//	
//

#include "Base.h"
#include "Fuses.h"
#include "Reporter.h"

class FusesCore : public Fuses {
	protected:
		//
		//	We need to keep a link to the reporting target
		//	to notify of attempts to modify the locked bits
		//
		Reporter	*_report;
		int		_instance;
		
		//
		//	Declare how many fuses and signature bytes we handle.
		//
		static const byte fuse_bytes	= 4;	// 2 words.
		static const byte sig_bytes	= 6;	// 3 words.

		//
		//	Define logical names for each of the fuse bytes
		//	that tally with the names used in the documentation
		//	(the address the Z register is expected to contain).
		//
		static const byte low_fuse_byte		= 0;
		static const byte lock_bits		= 1;
		static const byte extended_fuse_byte	= 2;
		static const byte high_fuse_byte	= 3;

		//
		//	Common Low Fuse bits (all).
		//
		static const byte bit_CKDIV8		= BIT( byte, 7 );
		static const byte bit_CKOUT		= BIT( byte, 6 );
		//
		static const byte size_CKDIV8		= 1;
		static const byte lsb_CKDIV8		= 7;
		static const byte mask_CKDIV8		= MASK( byte, size_CKDIV8 );
		//
		static const byte size_CKOUT		= 1;
		static const byte lsb_CKOUT		= 6;
		static const byte mask_CKOUT		= MASK( byte, size_CKOUT );
		//
		static const byte size_SUT		= 2;
		static const byte lsb_SUT		= 4;
		static const byte mask_SUT		= MASK( byte, size_SUT );
		//
		static const byte size_CKSEL		= 4;
		static const byte lsb_CKSEL		= 0;
		static const byte mask_CKSEL		= MASK( byte, size_CKSEL );

		//
		//	Common Lock Fuse bits.
		//
		static const byte bit_LB1		= BIT( byte, 0 );
		static const byte bit_LB2		= BIT( byte, 1 );
		//
		static const byte bit_BLB01		= BIT( byte, 2 );
		static const byte bit_BLB02		= BIT( byte, 3 );
		static const byte bit_BLB11		= BIT( byte, 4 );
		static const byte bit_BLB12		= BIT( byte, 5 );
		//
		static const byte size_LB		= 2;
		static const byte lsb_LB		= 0;
		static const byte mask_LB		= MASK( byte, size_LB );
		//
		static const byte size_BLB0		= 2;
		static const byte lsb_BLB0		= 2;
		static const byte mask_BLB0		= MASK( byte, size_BLB0 );
		//
		static const byte size_BLB1		= 2;
		static const byte lsb_BLB1		= 4;
		static const byte mask_BLB1		= MASK( byte, size_BLB1 );
		//

		//
		//	Extended Fuse Bits.
		//
		static const byte bit_SPMEN		= BIT( byte, 0 );
		//
		static const byte size_SPMEN		= 1;
		static const byte lsb_SPMEN		= 0;
		static const byte mask_SPMEN		= MASK( byte, size_SPMEN );

		//
		//	Floating fuse functions always seem to remain
		//	in the same bit locations, even if the fuse
		//	byte they are stored in is different (Extended
		//	v High bytes).
		//
		static const byte bit_BOOTRST		= BIT( byte, 0 );
		//
		static const byte size_BOOTRST		= 1;
		static const byte lsb_BOOTRST		= 0;
		static const byte mask_BOOTRST		= MASK( byte, size_BOOTRST );
		//
		static const byte size_BOOTSZ		= 2;
		static const byte lsb_BOOTSZ		= 1;
		static const byte mask_BOOTSZ		= MASK( byte, size_BOOTSZ );
		//
		static const byte size_BODLEVEL		= 3;
		static const byte lsb_BODLEVEL		= 0;
		static const byte mask_BODLEVEL		= MASK( byte, size_BODLEVEL );

		//
		//	High Fuse Bits.
		//
		static const byte bit_EESAVE		= BIT( byte, 3 );
		static const byte bit_WDTON		= BIT( byte, 4 );
		static const byte bit_SPIEN		= BIT( byte, 5 );
		static const byte bit_DWEN		= BIT( byte, 6 );
		static const byte bit_JTAGEN		= BIT( byte, 6 );
		static const byte bit_RSTDISBL		= BIT( byte, 7 );
		static const byte bit_OCDEN		= BIT( byte, 7 );
		//
		static const byte size_EESAVE		= 1;
		static const byte lsb_EESAVE		= 3;
		static const byte mask_EESAVE		= MASK( byte, size_EESAVE );
		//
		static const byte size_WDTON		= 1;
		static const byte lsb_WDTON		= 4;
		static const byte mask_WDTON		= MASK( byte, size_WDTON );
		//
		static const byte size_SPIEN		= 1;
		static const byte lsb_SPIEN		= 5;
		static const byte mask_SPIEN		= MASK( byte, size_SPIEN );
		//
		static const byte size_DWEN		= 1;
		static const byte lsb_DWEN		= 6;
		static const byte mask_DWEN		= MASK( byte, size_DWEN );
		//
		static const byte size_JTAGEN		= 1;
		static const byte lsb_JTAGEN		= 6;
		static const byte mask_JTAGEN		= MASK( byte, size_JTAGEN );
		//
		static const byte size_RSTDISBL		= 1;
		static const byte lsb_RSTDISBL		= 7;
		static const byte mask_RSTDISBL		= MASK( byte, size_RSTDISBL );
		//
		static const byte size_OCDEN		= 1;
		static const byte lsb_OCDEN		= 7;
		static const byte mask_OCDEN		= MASK( byte, size_OCDEN );

		//
		//	Logical names for the various signature bytes.
		//
		static const byte device_sig_1		= 0;
		static const byte osc_calibration_byte	= 1;
		static const byte device_sig_2		= 2;
		static const byte unspecified_byte_3	= 3;
		static const byte device_sig_3		= 4;
		static const byte unspecified_byte_5	= 5;
		
		//
		//	Storage for locks and signature bytes.
		//
		byte	_fuse[ fuse_bytes ],
			_sig[ sig_bytes ];

	public:
		FusesCore( Reporter *report, int instance ) {
			for( int i = 0; i < fuse_bytes; i++ ) _fuse[ i ] = 0xFF;
			for( int i = 0; i < sig_bytes; i++ ) _sig[ i ] = 0xFF;
			_report = report;
			_instance = instance;
		}
		//
		//	Read a fuse byte..
		//
		virtual byte read( word adrs ) {
			if( adrs >= fuse_bytes ) {
				_report->report( Error_Level, Fuse_Module, _instance, Address_OOR, "Invalid Fuse index %d", (int)adrs );
				return( 0xFF );
			}
			return( _fuse[ adrs ]);
		}
		virtual void write( word adrs, byte value ) {
			byte	rb;
			
			if( adrs >= fuse_bytes ) {
				_report->report( Error_Level, Fuse_Module, _instance, Address_OOR, "Invalid Fuse index %d", (int)adrs );
				return;
			}
			if(( adrs == lock_bits )&& Locks_Locked()) {
				_report->report( Error_Level, Fuse_Module, _instance, Read_Only );
				return;
			}
			//
			//	Need to confirm that once a fuse has been
			//	programmed, it cannot be reset without a
			//	full chip reset and re-program.
			//
			if(( rb = ( ~_fuse[ adrs ] & value )) != 0 ) {
				if( _report->report( Error_Level, Fuse_Module, _instance, Restore_Invalid, "Reset programmed bits %d in Fuse %d?", (int)rb, (int)adrs )) return;
			}
			_fuse[ adrs ] &= value;
		}
		
		//
		//	Burn a fuse byte.  Only to be used
		//	by the file reading routine.
		//
		virtual bool burn( word adrs, byte value ) {
			if( adrs >= fuse_bytes ) return( false );
			_fuse[ adrs ] = value;
			return( true );
		}
		
		//
		//	Read a signature byte.
		//
		virtual byte read_sig( word adrs ) {
			if( adrs >= sig_bytes ) {
				_report->report( Error_Level, Fuse_Module, _instance, Address_OOR, "Invalid Fuse index %d", (int)adrs );
				return( 0xFF );
			}
			return( _sig[ adrs ]);
		}
		
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
		virtual bool decode( const char *name, byte *number, byte *lsb, byte *mask ) {
			ASSERT( name != NULL );
			ASSERT( number != NULL );
			ASSERT( lsb != NULL );
			ASSERT( mask != NULL );

			
			if( strcmp( name, "CKDIV8" ) == 0 ) {
				*number = low_fuse_byte;
				*lsb = lsb_CKDIV8;
				*mask = mask_CKDIV8;
				return( true );
			}
			if( strcmp( name, "CKOUT" ) == 0 ) {
				*number = low_fuse_byte;
				*lsb = lsb_CKOUT;
				*mask = mask_CKOUT;
				return( true );
			}
			if( strcmp( name, "SUT" ) == 0 ) {
				*number = low_fuse_byte;
				*lsb = lsb_SUT;
				*mask = mask_SUT;
				return( true );
			}
			if( strcmp( name, "CKSEL" ) == 0 ) {
				*number = low_fuse_byte;
				*lsb = lsb_CKSEL;
				*mask = mask_CKSEL;
				return( true );
			}
			if( strcmp( name, "LB" ) == 0 ) {
				*number = lock_bits;
				*lsb = lsb_LB;
				*mask = mask_LB;
				return( true );
			}
			if( strcmp( name, "SPIEN" ) == 0 ) {
				*number = high_fuse_byte;
				*lsb = lsb_SPIEN;
				*mask = mask_SPIEN;
				return( true );
			}
			if( strcmp( name, "WDTON" ) == 0 ) {
				*number = high_fuse_byte;
				*lsb = lsb_WDTON;
				*mask = mask_WDTON;
				return( true );
			}
			if( strcmp( name, "EESAVE" ) == 0 ) {
				*number = lock_bits;
				*lsb = lsb_EESAVE;
				*mask = mask_EESAVE;
				return( true );
			}
			//
			//	Tail recursion it we fail to match.
			//
			return( Fuses::decode( name, number, lsb, mask ));		
		}

		//
		//	Access status of a features
		//	===========================
		//
		//	The fuse setting encoded below are common across
		//	all AVR chips and can be consolidated here.
		//
		//	Chip function settings.
		//	-----------------------
		//
		virtual bool CKDIV8( void ) {
			return(( _fuse[ low_fuse_byte ] & bit_CKDIV8 ) == 0 );		// Clock Divided by 8?
		}
		virtual bool CKOUT( void ) {
			return(( _fuse[ low_fuse_byte ] & bit_CKOUT ) == 0 );		// Clock output to port B
		}
		virtual byte SUT( void ) {
			return(( _fuse[ low_fuse_byte ] >> lsb_SUT ) & mask_SUT );	// Start-up time required
		}
		virtual byte CKSEL( void ) {
			return(( _fuse[ low_fuse_byte ] >> lsb_CKSEL ) & mask_CKSEL );	// Clock division factor
		}
		//
		//	Memory Lock Protection
		//	----------------------
		//
		//	LB2	LB1	Lock Bits	Flash/EEPROM
		//			Locked?		Locked?
		//
		//	1	1	No		No
		//	1	0	No		Yes
		//	0	0	Yes		Yes
		//
		virtual bool Locks_Locked( void ) {
			return(( _fuse[ lock_bits ] & bit_LB2 ) == 0 );	// Have the fuses and locks been locked?
		}
		virtual bool Flash_Locked( void ) {
			return(( _fuse[ lock_bits ] & bit_LB1 ) == 0 );	// Has the flash (and EEPROM) memory been locked?
		}

		//
		//	Serial Programming Interface Enabled.
		//
		virtual bool SPIEN( void ) {
			return(( _fuse[ high_fuse_byte ] & bit_SPIEN ) == 0 );	// SPI program and data downloading enabled?
		}
		virtual bool WDTON( void ) {
			return(( _fuse[ high_fuse_byte ] & bit_WDTON ) == 0 );	// Watchdog timer enabled?
		}
		virtual bool EESAVE( void ) {
			return(( _fuse[ high_fuse_byte ] & bit_EESAVE ) == 0 );	// EEPROM memory is preserved through the chip erase?
		}
};
		

#endif

//
//	EOF
//
