//
//	Program.h
//
//	Define a template class that implements an amount of
//	program flash memory.
//

#ifndef _PROGRAM_H_
#define _PROGRAM_H_

//
//	Base types
//
#include "Base.h"

//
//	Derived from Flash and uses Exceptions.
//
#include "Flash.h"
#include "Reporter.h"
#include "Validation.h"

//
//	The flash memory class.
//
template< word _page_size, word _page_count, word _boot_pages, word _op_duration > class Program : public Flash {
	private:
		//
		//	Define the total number of words the flash memory
		//	will contain, and the boundary between the application
		//	and boot sections.
		//
		static const dword total = (dword)_page_size * (dword)_page_count;
		static const dword application = total - ((dword)_page_size * (dword)_boot_pages );
		
		//
		//	Arbitrary buffer length for textual file input.
		//
		static const word buffer_size = 256;

		//
		//	Arbitrary decoded buffer length once an input
		//	record has been converted from text to data.
		//
		static const word decoded_size = 64;
		
		//
		//	Define where we keep the flash data.
		//
		word		_storage[ total ],
				_buffer[ _page_size ];
		
		//
		//	Flags to control the Write Locking facility that
		//	the flash demonstrates.  This is to say that once
		//	a page erase or write has been started that section
		//	of flash cannot be read from until the area has been
		//	enabled again.
		//
		bool		_locked,
				_application;
				
		//
		//	Define an enumeration tracking any pending actions.
		//
		enum {
			None_Pending,
			Erase_Pending,
			Write_Pending
		}
				_pending;
		word		_target;

		//
		//	Where we send Exceptions
		//
		Reporter	*_report;
		int		_instance;

		//
		//	Helper routines for conversion from text
		//	to binary data.
		//
		bool is_hexidecimal( char letter ) {
			return((( letter >= '0' )&&( letter <= '9' ))||(( letter >= 'A' )&&( letter <= 'F' )));
		}
		byte hexidecimal( char letter ) {
			if(( letter >= '0' )&&( letter <= '9' )) return( letter - '0' );
			return( 10 + letter - 'A' );
		}
		bool from_hex( char *source, byte *dest ) {
			if( is_hexidecimal( source[ 0 ]) && is_hexidecimal( source[ 1 ])) {
				*dest = ( hexidecimal( source[ 0 ]) << 4 ) | hexidecimal( source[ 1 ]);
				return( true );
			}
			return( false );
		}
		word load_word( byte *from ) {
			return(((word)( from[ 0 ]) << 8 ) | (word)( from[ 1 ]));
		}
		
	public:
		//
		//	Constructor
		//
		Program( Reporter *handler, int instance ) {
			_report = handler;
			_instance = instance;
			for( dword a = 0; a < total; _storage[ a++ ] = 0 );
			for( word w = 0; w < _page_size; _buffer[ w++ ] = ~((word)0 ));
			_locked = false;
			_application = true;
			_pending = None_Pending;
		}

		//
		//	Load flash from hex file.
		//
		//	This supports the Intel HEX file format (see
		//	"https://en.wikipedia.org/wiki/Intel_HEX").
		//
		//	Note/	There is a contradiction between the basic
		//		assumption on how program memory is arranged
		//		taken by the HEX file, and that of reality
		//		taken by the AVR system.
		//
		//		Hex files are byte addressed, and need to be
		//		read and applied using that nomenclature.
		//
		//		The actual flash memory is word addressed, as
		//		is the underlying executable code.
		//
		virtual bool load_hex( const char *filename ) {
			FILE	*source;
			char	buffer[ buffer_size ];
			word	line;
			dword	ext_adrs;

			//
			//	If we are passed NULL we assume that nothing
			//	is to be loaded, so we just return success.
			//
			if( filename == NULL ) return( true );
			//
			//	Start by getting the file..
			//
			if(( source = fopen( filename, "r" )) == NULL ) {
				_report->report( Error_Level, Program_Module, _instance, File_Open_Failed, "Cannot open file '%s'", filename );
				return( false );
			}

			//
			//	We initialise the extended address (for accessing
			//	an extended address space) to zero, therefore we
			//	default to using just the first 64 KBytes of flash
			//	(32 KWords).
			//
			ext_adrs = 0x00000000;

			//
			//	Read the Hex file line by line...
			//
			line = 0;
			while( fgets( buffer, buffer_size, source )) {
				char *look;

				//
				//	Note new line number.
				//
				line++;
				
				//
				//	The buffer should contain something of the following
				//	format (where '/n' indicates the number of hexidecimal
				//	digits expected).
				//
				//		[skip]:[count/2][address/4][record/2][data/count*2][checksum/2]
				//
				//	where
				//
				//		skip		Data to be ignored leading upto initial
				//				colon symbol marking start of valid data.
				//
				//		:		The formal start of the record.
				//
				//		count		The number of data bytes to be expected
				//				in the record (two hex digits, range 00
				//				to FF, 0 to 255 decimal).
				//
				//		address		The 16 bit address relating to this data
				//				(four hex digits, range 0000 to FFFF, 0
				//				65535 decimal).
				//
				//		record		The record type assigning meaning to the
				//				data provided (two hex digits, range 00
				//				to FF, 0 to 255 decimal).
				//
				//			00	Data
				//			01	End of File
				//			02	Extended Segment Address
				//			03	Start Segment Address
				//			04	Extended Linear Address
				//			05	Start Linear Address
				//
				//		data		count bytes of data (as two digit hexidecimal
				//				numbers).
				//
				//		checksum	A calculated two digit value allowing integrity
				//				of the record to be confirmed.
				//
				//				The checksum is the twos complement of the
				//				least significant byte of the sum of all the
				//				numerical values following the colon and
				//				preceding the checksum itself.
				//

				//
				//	Symbolic record start symbol.
				//
				static const char start_record = ':';

				//
				//	Note the number positions of each of the fields
				//	in the record.
				//
				static const word record_count =  0;
				static const word record_address = 1;
				static const word record_type = 3;
				static const word record_data = 4;
					
				//
				//	Start by trying to find the colon.
				//
				for( look = buffer; *look != EOS; look++ ) if( *look == start_record ) break;
				
				if( *look == start_record ) {
					byte	decoded[ decoded_size ];
					word	available, sum, adrs;
					byte	count;
			
					//
					//	Found the start of a valid record, so build up the
					//	binary representation.
					//
					look++;			// Skip the colon.

					//
					//	Transcribe the hex data into the decoded array.
					//
					available = 0;
					while( from_hex( look, &( decoded[ available ]))) {
						//
						//	Two hex digits correctly parsed in, move on
						//	and try again.
						//
						look += 2;
						if( ++available >= decoded_size ) {
							//
							//	Just too big, raise error and bail out.
							//
							_report->report( Error_Level, Program_Module, _instance, Line_Too_Long, "Line %d too long (maximum %d bytes)", line, available );
							fclose( source );
							return( false );
						}
					}

					//
					//	Did we get something?  A valid record has to
					//	have at least 5 binary values (10 hex digits).
					//
					//	eg/	:00,01FF
					//
					if( available <= record_data ) {
						_report->report( Error_Level, Program_Module, _instance, Format_Error, "Line %d truncated", line );
						fclose( source );
						return( false );
					}

					//
					//	Cheat a little here, we roll available back
					//	off the checksum so we can ignore it later
					//	and access it directly using available as the
					//	index.
					//
					available--;
					
					//
					//	Perform basic checks on the line data:
					//
					//	Checksum:
					//
					sum = 0;
					for( word i = 0; i < available; sum += decoded[ i++ ]);
					sum = ( ~sum + 1 ) & 0xff;
					if( sum != decoded[ available ]) {
						_report->report( Error_Level, Program_Module, _instance, Checksum_Error, "Line %d, checksum error", line );
						fclose( source );
						return( false );
					}

					//
					//	Record length w.r.t. count:
					//
					if(( count = decoded[ record_count ]) != ( available - record_data )) {
						_report->report( Error_Level, Program_Module, _instance, Format_Error, "Line %d, record size inconsistent", line );
						fclose( source );
						return( false );
					}

					//
					//	Now process the content as verification
					//	has been passed.
					//
					adrs = load_word( &( decoded[ record_address ]));
					switch( decoded[ record_type ]) {
						case 0x00: {
							//
							//	00	Data
							//
							for( word c = 0; c < count; c++ ) {
								word	*target;
								dword	wide_adrs;
								bool	msb;
								
								//
								//	Place the bytes into memory
								//	from adrs, incrementing adrs
								//	for each data byte.
								//
								//	Establish which word we are modifying.
								//
								wide_adrs = ext_adrs + adrs;
								
								//
								//	Using upper or lower byte? Also remember to
								//	shift right to get the word address we need.
								//
								msb = (( wide_adrs & 1 ) != 0 );
								wide_adrs >>= 1;

								//
								//	In range?
								//
								if( wide_adrs >= total ) {
									_report->report( Error_Level, Program_Module, _instance, Program_Too_Big, "Line %d, program too large at address $%06X", line, (int)wide_adrs );
									fclose( source );
									return( false );
								}
								target = &( _storage[ wide_adrs ]);

								//
								//	Now apply the value to either the MSByte or
								//	LSByte of the word being addressed.
								//
								if( msb ) {
									*target = ( *target & 0x00ff ) | (((word)decoded[ record_data + c ]) << 8 );
								}
								else {
									*target = ( *target & 0xff00 ) | ((word)decoded[ record_data + c ]);
								}

								//
								//	Move to next address
								//
								if( ++adrs == 0x0000 ) {
									_report->report( Error_Level, Program_Module, _instance, Address_Wraps, "Line %d, program address wraps to 0", line );
									fclose( source );
									return( false );
								}
							}
							break;
						}
						case 0x01: {
							//
							//	01	End of File
							//
							//	Nothing to do here, we have the last
							//	record decoded, so close the file and
							//	return success.
							//
							fclose( source );
							return( true );
						}
						case 0x02: {
							//
							//	02	Extended Segment Address
							//
							if( count != 2 ) {
								_report->report( Error_Level, Program_Module, _instance, Format_Error, "Line %d, segment address format error", line );
								fclose( source );
								return( false );
							}
							ext_adrs = (dword)load_word( &( decoded[ record_data ])) << 4;
							break;
						}
						case 0x03: {
							//
							//	03	Start Segment Address
							//
							_report->report( Error_Level, Program_Module, _instance, Not_Implemented, "Line %d, start segment address not implemented", line );
							fclose( source );
							return( false );
						}
						case 0x04: {
							//
							//	04	Extended Linear Address
							//
							if( count != 2 ) {
								_report->report( Error_Level, Program_Module, _instance, Format_Error, "Line %d, extended address format error", line );
								fclose( source );
								return( false );
							}
							ext_adrs = (dword)load_word( &( decoded[ record_data ])) << 16;
							break;
						}
						case 0x05: {
							//
							//	05	Start Linear Address
							//	
							//
							_report->report( Error_Level, Program_Module, _instance, Not_Implemented, "Line %d, start linear address not implemented", line );
							fclose( source );
							return( false );
						}
						default: {
							//
							//	Unrecognised Record Number.
							//
							_report->report( Error_Level, Program_Module, _instance, Record_Error, "Line %d, unrecognised record number", line );
							fclose( source );
							return( false );
						}
					}
				}
			}

			//
			//	If we get here then there was no end of file
			//	record, which is an error in its own right.
			//
			_report->report( Error_Level, Program_Module, _instance, Program_Truncated, "Line %d, end of program missing", line );
			fclose( source );
			return( false );
		}

		//
		//	The Flash API
		//	=============
		//
		//	Reading flash can be done word-at-a-time style using
		//	linear addressing (on an address per word basis).
		//
		//
		virtual word read( dword adrs ) {
			if( adrs >= total ) {
				_report->report( Error_Level, Program_Module, _instance, Address_OOR, "Address $%06X outside program space", (int)adrs );
				return( 0 );
			}
			if( _locked && (( adrs < application ) == _application )) _report->report( Error_Level, Program_Module, _instance, Write_Only, "Address $%06X set WRITE ONLY", (int)adrs );
			return( _storage[ adrs ]);
		}

		//
		//	Flash Buffer Page
		//	-----------------
		//
		//	Writing to flash requires a temporary page buffer
		//	be cleared, then filled, then written to permanent
		//	flash storage.
		//
		//	This is all done using a "Page" as the basic unit
		//	written to permanent flash storage.
		//
		//	Clear the temp buffer, all bits will be set.
		//
		virtual void clear( void ) {
			for( word w = 0; w < _page_size; _buffer[ w++ ] = ~((word)0 ));
		}
		//
		//	Place a value into the temp buffer (using AND op), return microseconds
		//	required.
		//
		virtual word place( word adrs, word value ) {
			_pending = None_Pending;
			if( adrs >= _page_size ) {
				_report->report( Error_Level, Program_Module, _instance, Address_OOR, "Write to buffer address $%04X invalid", (int)adrs );
				return( 0 );
			}
			_buffer[ adrs ] &= value;
			return( 0 );
		}


		//
		//	Flash Modification
		//	------------------
		//
		//	Erasing or Writing a page will "lock" the section of
		//	flash being written to.  The two sections pertinent to
		//	this comment are those pages at the start of flash,
		//	the RWW section, and those above this, the NRWW
		//	section.
		//
		//	Simplistically, and not entirely accurately, you can read
		//	from one while modifying to the other.
		//
		//	Erase a page of flash memory, return the
		//	microseconds this will require to complete.
		//
		virtual word erase( word page ) {
			if( _locked && (( page < ( _page_count - _boot_pages )) != _application )) {
				//
				//	We are write locked, and where we are locked is not the
				//	same place we are writing now..
				//
				_report->report( Error_Level, Program_Module, _instance, Write_Invalid, "Cannot erase READ ONLY page $%04X", (int)page );
			}
			_pending = Erase_Pending;
			_target = page;
			_locked = true;
			_application = ( page < ( _page_count - _boot_pages ));
			return( _op_duration );
		}
		//
		//	Write the temp buffer into the Flash memory, return the
		//	microseconds this will require to complete.
		//
		virtual word write( word page ) {
			if( _locked && (( page < ( _page_count - _boot_pages )) != _application )) {
				//
				//	We are write locked, and where we are locked is not the
				//	same place we are writing now..
				//
				_report->report( Error_Level, Program_Module, _instance, Write_Invalid, "Cannot update READ ONLY page $%04X", (int)page );
			}
			_pending = Write_Pending;
			_target = page;
			_locked = true;
			_application = ( page < ( _page_count - _boot_pages ));
			return( _op_duration );
		}
			
		//
		//	Commit a previously initiated page erase or page
		//	write action.
		//
		//	The combination of calling erase()/write(), then
		//	waiting the specified period of time, then calling
		//	commit() allows a reasonably authentic simulation
		//	of how flash memory operates.  This is to say that
		//	inadvertently initiating another similar action before
		//	commit() is called will drop the earlier action.
		//
		virtual void commit( void ) {
			ASSERT( _locked );
			switch( _pending ) {
				case Erase_Pending: {
					dword a = (dword)_target * (dword)_page_size;
					
					//
					//	This operation explicitly sets all
					//	bits in the page to 1.
					//
					for( word w = 0; w < _page_size; _storage[ a++ ] = ~((word)0 ));
					clear();
					break;
				}
				case Write_Pending: {
					dword a = (dword)_target * (dword)_page_size;
					
					//
					//	This operation ANDs the buffer with the storage.
					//
					for( word w = 0; w < _page_size; _storage[ a++ ] &= _buffer[ w ]);
					break;
				}
				default: {
					ABORT();
					break;
				}
				_pending = None_Pending;
			}
		}
		
		//
		//	Read-While-Write control
		//	------------------------
		//
		//	Writing to any section of the flash memory will
		//	automatically lock this section from reading.  To
		//	re-enable this read access this routine needs to
		//	be called.
		//
		virtual void enable( void ) {
			ASSERT( _pending == None_Pending );
			_locked = false;
		}

		//
		//	Flash Characteristics
		//	---------------------
		//
		//	Return the total number of pages of flash memory,
		//	the number of pages (from zero) which form the
		//	Read While Write section and finally the size of
		//	a single page in words.
		//
		virtual word total_pages( void ) {
			return( _page_count );
		}
		virtual word application_pages( void ) {
			return( _page_count - _boot_pages );
		}
		virtual word page_size( void ) {
			return( _page_size );
		}
		
		//
		//	Supporting routines
		//	-------------------
		//
		//	Routines to simplify the code around using this API.
		//
		//	page_number	Return, right shifted and masked, the
		//			flash page number corresponding to the
		//			flash (word) address provided.
		//
		//	word_number	Return, masked off, the word number within
		//			a page that the supplied address points
		//			to.
		//
		virtual word page_number( dword adrs ) {
			return( adrs / _page_size );
		}
		virtual word word_number( dword adrs ) {
			return( adrs % _page_size );
		}

		//
		//	Program space examiner API
		//
		virtual bool examine_words( dword adrs, Symbols *labels, char *buffer, int max ) {
			if( adrs >= total ) return( false );
			snprintf( buffer, max, "$%04X", (int)_storage[ adrs ]);
			return( true );
		}
		virtual bool examine_bytes( dword adrs, Symbols *labels, char *buffer, int max ) {

			if( adrs >= ( total << 1 )) return( false );
			if( adrs & 1 ) {
				snprintf( buffer, max, "$%02X", (int)high_byte( _storage[ adrs >> 1 ]));
			}
			else {
				snprintf( buffer, max, "$%02X", (int)low_byte( _storage[ adrs >> 1 ]));
			}
			return( true );
		}
};

#endif

//
//	EOF
//







	

