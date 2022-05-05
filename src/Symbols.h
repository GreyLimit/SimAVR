//
//	Symbols.h
//
//	This is where the associations between constant
//	values and symbolic names are stored.
//

#ifndef _SYMBOLS_H_
#define _SYMBOLS_H_

//
//	System libraries
//
#include <stdio.h>
#include <cstring>


//
//	Bring in the base settings.
//
#include "Base.h"
#include "Reporter.h"

//
//	Define various symbol names spaces
//
typedef enum  {
	program_address,
	memory_address,
	bit_constant,
	byte_constant,
	word_constant,
	program_word,
	byte_register,
	word_register,
	port_number,
	unspecified_type
} symbol_type;


//
//	How we store the associations
//
class Symbols {
	private:
		//
		//	Errors go here.
		//
		Reporter	*_report;
		
		//
		//	Arbitrary input buffer size.
		//
		static const int max_buffer = 32;
		
		//
		//	Define a prefixes for non-decimal values.
		//
		static const char hexidecimal	= '$';
		static const char binary	= '%';

		//
		//	storage node.
		//
		struct label {
			char		*name;
			symbol_type	type;
			dword		value;
			label		*next_by_name,
					*next_by_value,
					**prev_by_value;
		};

		//
		//	The head of the list, sorted into value order
		//	not name order.
		//
		label	*_by_name,		// sorted in name order.
			*_by_value;		// sorted in value order.

		//
		//	Find a label record by either name or value.
		//
		label *find_label( char *name ) {
			label	*look;

			for( look = _by_name; look; look = look->next_by_name ) {
				int test = strcmp( name, look->name );
				if( test == 0 )	return( look );
				if( test < 0 ) return( NULL );
			}
			return( NULL );
		}
		label *find_nearest( symbol_type type, dword value ) {
			label	*look,
				*last;

			last = NULL;
			for( look = _by_value; look; look = look->next_by_value ) {
				if( type == look->type ) {
					if( value == look->value ) return( look );
					last = look;
				}		
				else if( value < look->value ) break;
			}
			return( last );
		}

		//
		//	Convert text to a symbol type value.
		//
		symbol_type type_name( const char *domain ) {
			if( strcmp( domain, "program_address" ) == 0 ) return( program_address );
			if( strcmp( domain, "memory_address" ) == 0 ) return( memory_address );
			if( strcmp( domain, "bit_constant" ) == 0 ) return( bit_constant );
			if( strcmp( domain, "byte_constant" ) == 0 ) return( byte_constant );
			if( strcmp( domain, "word_constant" ) == 0 ) return( word_constant );
			if( strcmp( domain, "program_word" ) == 0 ) return( program_word );
			if( strcmp( domain, "byte_register" ) == 0 ) return( byte_register );
			if( strcmp( domain, "word_register" ) == 0 ) return( word_register );
			if( strcmp( domain, "port_number" ) == 0 ) return( port_number );
			(void)_report->raise( Error_Level, Symbols_Module, Record_Error );
			return( unspecified_type );
		}

	public:
		//
		//	Build an empty object.
		//
		Symbols( Reporter *errors ) {
			_by_value = NULL;
			_by_name = NULL;
			_report = errors;
		}

		//
		//	Add a new label
		//
		bool new_label( char *name, symbol_type type, dword value ) {
			label	**adrs, *ptr, *rec;

			adrs = &_by_name;
			while(( ptr = *adrs ) != NULL ) {
				if( strcmp( name, ptr->name ) < 0 ) break;
				adrs = &( ptr->next_by_name );
			}
			//
			//	We add the new record where ever address
			//	points, remembering to tag on the ptr record.
			//
			rec = new label;
			rec->name = strdup( name );
			rec->type = type;
			rec->value = value;
			rec->next_by_name = ptr;
			*adrs = rec;
			//
			//	Now add into the value list.
			//
			adrs = &_by_value;
			while(( ptr = *adrs ) != NULL ) {
				if( value < ptr->value ) break;
				adrs = &( ptr->next_by_value );
			}
			*( rec->prev_by_value = adrs ) = rec;
			if(( rec->next_by_value = ptr ) != NULL ) {
				ptr->prev_by_value = &( rec->next_by_value );
			}
			return( true );
		}

		//
		//	delete a label
		//
		bool delete_label( char *name ) {
			label	**adrs, *ptr;

			adrs = &_by_name;
			for( ptr = *adrs; ptr; adrs = &( ptr->next_by_name )) {
				int test = strcmp( name, ptr->name );
				if( test == 0 )	{
					free( ptr->name );
					*adrs = ptr->next_by_name;
					if(( *( ptr->prev_by_value ) = ptr->next_by_value )) ptr->next_by_value->prev_by_value = ptr->prev_by_value;
					delete ptr;
					return( true );
				}
				if( test < 0 ) break;
			}
			return( false );
		}

		//
		//	Expand value to symbol[+offset] or hex constant.
		//
		char *expand( symbol_type type, dword value, char *buffer, int max ) {
			label	*look;

			if(( look = find_nearest( type, value ))) {
				if(( look->value < value )&&(( type == program_address )||( type == memory_address ))) {
					snprintf( buffer, max, "%s+%d", look->name, ( value - look->value  ));
				}
				else {
					snprintf( buffer, max, "%s", look->name );
				}
			}
			else {
				switch( type ) {
					case bit_constant: {
						if( value ) {
							int	len;
							dword	reversed;

							len = 0;
							reversed = 0;
							while( value ) {
								len++;
								reversed = ( reversed << 1 ) | ( value & 1 );
								value >>= 1;
							}
							if(( len + 1 ) >= max ) {
								snprintf( buffer, max, "%c%X", hexidecimal, value );
							}
							else {
								char *a = buffer;
								
								*a++ = binary;
								while( len-- ) *a++ = ( reversed & 1 )? '1': '0';
								*a = EOS;
							}
						}
						else {
							snprintf( buffer, max, "%c0", binary );
						}
						break;
					}
					case byte_constant: {
						snprintf( buffer, max, "%c%02X", hexidecimal, value & 0xFF );
						break;
					}
					case word_constant: {
						snprintf( buffer, max, "%c%04X", hexidecimal, value & 0xFFFF );
						break;
					}
					case program_word: {
						snprintf( buffer, max, "%c%04X", hexidecimal, value & 0xFFFF );
						break;
					}
					case byte_register: {
						snprintf( buffer, max, "r%d", value );
						break;
					}
					case word_register: {
						snprintf( buffer, max, "r%d:r%d", value+1, value );
						break;
					}
					case port_number: {
						snprintf( buffer, max, "%c%02X", hexidecimal, value );
						break;
					}
					default: {
						snprintf( buffer, max, "%c%06X", hexidecimal, value );
						break;
					}
				}
			}
			return( buffer );
		}

		//
		//	Read in symbols from a file.
		//
		//	Note/
		//
		//		The code in this routine is essentially unsafe and broken
		//		as this offers an opportunity for a buffer over flow if the
		//		input data is too large.
		//
		bool load_symbols( const char *file ) {
			FILE	*source;

			char	domain[ max_buffer ],
				name[ max_buffer ];
			int	value;

			if( file == NULL ) return( true );
			if(( source = fopen( file, "r" )) == NULL ) return( false );
			while( fscanf( source, "%s%s%d\n", domain, name, &value ) == 3 ) {
				if( new_label( name, type_name( domain ), (dword)value )) {
					printf( "%s / %s = %d\n", domain, name, value );
				}
				else {
					return( false );
				}
			}
			fclose( source );
			return( true );
		}
};


#endif

//
//	EOF
//
