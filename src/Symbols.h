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
#include <ctype.h>

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
	data_address,
	bit_constant,
	byte_constant,
	word_constant,
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
		int		_instance;
		
		//
		//	Arbitrary input buffer size.
		//
		static const int max_buffer = 80;
		
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
			bool		tag;
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
		label *find_label( symbol_type type, char *name ) {
			label	*look;

			for( look = _by_name; look; look = look->next_by_name ) {
				if( type == look->type ) {
					int test = strcmp( name, look->name );
					if( test == 0 )	return( look );
					if( test < 0 ) return( NULL );
				}
			}
			return( NULL );
		}
		label *find_nearest( symbol_type type, dword value ) {
			label	*look,
				*last;

			last = NULL;
			for( look = _by_value; look; look = look->next_by_value ) {
				if( value < look->value ) break;
				if( type == look->type ) {
					if( value == look->value ) return( look );
					last = look;
				}		
			}
			return( last );
		}


		//
		//	Two routines for checking characters are valid
		//	symbolic name components.
		//
		bool first_letter( char l ) {
			return( isalpha( l ) || ( l == '_' ));
		}
		//
		bool next_letter( char l ) {
			return( isalnum( l ) || ( l == '_' ));
		}
		//
		//	Return the number of characters which
		//	which form a valid identifier.
		//
		int ident_len( char *p ) {
			int i = 0;
			if( first_letter( *p++ )) {
				i++;
				while( next_letter( *p++ )) i++;
			}
			return( i );
		}
		//
		//	Check a character for possible numeric value
		//
		bool numeric( char c, int *r ) {
			if(( c >= '0' )&&( c <= '9' )) {
				*r = c - '0';
				return( true );
			}
			if(( c >= 'a' )&&( c <= 'f' )) {
				*r = 10 + c - 'a';
				return( true );
			}
			if(( c >= 'A' )&&( c <= 'F' )) {
				*r = 10 + c - 'A';
				return( true );
			}
			return( false );
		}

	public:
		//
		//	Build an empty object.
		//
		Symbols( Reporter *errors, int instance ) {
			_by_value = NULL;
			_by_name = NULL;
			_report = errors;
			_instance = instance;
		}

		//
		//	Convert text to a symbol type value or the other way.
		//
		symbol_type type_name( char *domain ) {
			if( strcmp( domain, "program_address" ) == 0 ) return( program_address );
			if( strcmp( domain, "PA" ) == 0 ) return( program_address );
			if( strcmp( domain, "memory_address" ) == 0 ) return( memory_address );
			if( strcmp( domain, "MA" ) == 0 ) return( memory_address );
			if( strcmp( domain, "data_address" ) == 0 ) return( data_address );
			if( strcmp( domain, "DA" ) == 0 ) return( data_address );
			if( strcmp( domain, "bit_constant" ) == 0 ) return( bit_constant );
			if( strcmp( domain, "byte_constant" ) == 0 ) return( byte_constant );
			if( strcmp( domain, "word_constant" ) == 0 ) return( word_constant );
			if( strcmp( domain, "byte_register" ) == 0 ) return( byte_register );
			if( strcmp( domain, "word_register" ) == 0 ) return( word_register );
			if( strcmp( domain, "port_number" ) == 0 ) return( port_number );
			(void)_report->report( Error_Level, Symbols_Module, _instance, Record_Error, "Symbol domain '%s' not recognised", domain );
			return( unspecified_type );
		}
		const char *name_type( symbol_type domain ) {
			switch( domain ) {
				case program_address: return( "program_address" );
				case memory_address: return( "memory_address" );
				case data_address: return( "data_address" );
				case bit_constant: return( "bit_constant" );
				case byte_constant: return( "byte_constant" );
				case word_constant: return( "word_constant" );
				case byte_register: return( "byte_register" );
				case word_register: return( "word_register" );
				case port_number: return( "port_number" );
				default: {
					(void)_report->report( Terminate_Level, Symbols_Module, _instance, Record_Error, "Invalid symbol domain reference %d", domain );
					break;
				}
			}
			return( "unrecognised_type" );
		}
		
		//
		//	Add a new label
		//
		bool new_label( char *name, symbol_type type, dword value ) {
			label	**adrs, *ptr, *rec;

			//
			//	check label itself is ok
			//
			if( ident_len( name ) != strlen( name )) {
				_report->report( Error_Level, Symbols_Module, _instance, Invalid_Identifier, "Invalid identifier '%s'", name );
				return( false );
			}
			//
			//	Insert into the name order list
			//
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
			rec->tag = false;
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
		bool delete_label( char *name, symbol_type type ) {
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
		//	Expand a constant value using specific domain rules.
		//
		char *constant( symbol_type type, dword value, char *buffer, int max ) {
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
							while( len-- ) {
								*a++ = ( reversed & 1 )? '1': '0';
								reversed >>= 1;
							}
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
				case memory_address: {
					snprintf( buffer, max, "%c%04X", hexidecimal, value );
					break;
				}
				default: {
					snprintf( buffer, max, "%c%06X", hexidecimal, value );
					break;
				}
			}
			return( buffer );
		}

		//
		//	Expand value to symbol[+offset] or hex constant.
		//
		char *expand( symbol_type type, dword value, char *buffer, int max ) {
			label	*look;

			//
			//	Any label we can reference off?
			//
			if(( look = find_nearest( type, value ))) {
				//
				//	Exact match is easy case.
				//
				if( look->value == value ) {
					snprintf( buffer, max, "%s", look->name );
					return( buffer );
				}
				if(( type == program_address )||( type == memory_address )||( type == data_address )) {
					ASSERT( look->value < value );
					snprintf( buffer, max, "%s+%d", look->name, ( value - look->value  ));
					return( buffer );
				}
				//
				//	Fall through to numerical value.
				//
			}
			//
			//	Output a pure numerical answer.
			//
			return( constant( type, value, buffer, max ));
		}

		//
		//	Offer a mechanism for converting an EOS terminated
		//	string into an actual value.  This needs to be provided
		//	with the domain the calculation belongs in.
		//
		bool evaluate( symbol_type type, char *string, dword *value ) {
			//
			//	For the moment, this is limited to the absolutely
			//	the most basic parsing;  the text follows one of
			//	the following layout/formats:
			//
			//	number	[0-9][0-9]*			Decimal
			//		%[01][01]*			Binary
			//		$[0-9A-Fa-f][0-9A-Fa-f]*	Hexidecimal
			//
			//	ident	[_A-Za-z][_A-Za-z0-9]*
			//
			//	expr	{ident}[+-]{number}		Expression
			//
			//	Full expression evaluation could be placed here, but
			//	seems over the top at this point.
			//
			dword	base,
				sum;
			char	*ptr,
				*fix, with;
			bool	add;
			int	i, b;

			base = 0;
			sum = 0;
			ptr = string;
			fix = NULL;
			add = true;
			if(( i = ident_len( ptr )) > 0 ) {
				label	*l;

				ptr += i;
				with = *( fix = ptr );
				*ptr++ = EOS;

				if(( l = find_label( type, string )) == NULL ) {
					_report->report( Error_Level, Symbols_Module, _instance, Invalid_Identifier, "Identifier '%s' not found in domain %s", string, name_type( type ));
					return( false );
				}
				base = l->value;
				switch( with ) {
					case EOS: {
						*value = base;
						return( true );
					}
					case '+': {
						add = true;
						break;
					}
					case '-': {
						add = false;
						break;
					}
					default: {
						_report->report( Error_Level, Symbols_Module, _instance, Format_Error );
						return( false );
					}
				}
			}
			switch( *ptr ) {
				case hexidecimal: {
					b = 16;
					ptr++;
					break;
				}
				case binary: {
					b = 2;
					ptr++;
					break;
				}
				default: {
					b = 10;
					break;
				}
			}
			while( numeric( *ptr, &i )) {
				if( i >= b ) {
					_report->report( Error_Level, Symbols_Module, _instance, Invalid_Number );
					return( false );
				}
				sum = sum * b + i;
				ptr++;
			}
			if( *ptr != EOS ) {
				_report->report( Error_Level, Symbols_Module, _instance, Format_Error );
				return( false );
			}
			if( fix ) *fix = with;
			if( add ) {
				base += sum;
			}
			else {
				base -= sum;
			}
			*value = base;
			return( true );
		}

		//
		//	Read in symbols from a file.
		//
		//	Note/
		//
		//		The code in this routine is essentially unsafe and broken
		//		as this offers an opportunity for a buffer overflow if the
		//		input data is too large.
		//
		//		I need to fix this.
		//
		bool load_symbols( char *file ) {
			FILE	*source;

			char		buffer[ max_buffer ],
					domain[ max_buffer ],
					name[ max_buffer ],
					expr[ max_buffer ];
			symbol_type	type;
			int		line, n, problems;
			dword		value;

			if( file == NULL ) return( true );
			if(( source = fopen( file, "r" )) == NULL ) return( false );
			problems = 0;
			line = 0;
			while( fgets( buffer, max_buffer, source )) {
				line++;
				if(( *buffer != '#' )&&(( n = sscanf( buffer, "%s%s%s", domain, name, expr )) == 3 )) {
					type = type_name( domain );
					if( evaluate( type, expr, &value )) {
						if( !new_label( name, type, value )) {
							if( _report->report( Error_Level, Symbols_Module, _instance, Record_Error, "Import error, file '%s', line %d", file, line )) {
								fclose( source );
								return ( false );
							}
							problems++;
						}
					}
					else {
						if( _report->report( Error_Level, Symbols_Module, _instance, Invalid_Number, "Import error, file '%s', line %d", file, line )) {
							fclose( source );
							return( false );
						}
						problems++;
					}
				}
			}
			fclose( source );
			return( problems == 0 );
		}

		//
		//	Save all the symbols to a file.
		//
		bool save_symbols( char *file ) {
			FILE	*to;
			label	*look;

			if( file == NULL ) return( false );
			if(( to = fopen( file, "w" )) == NULL ) {
				_report->report( Error_Level, Symbols_Module, _instance, File_Open_Failed );
				return( false );
			}
			for( look = _by_name; look != NULL; look = look->next_by_name ) {
				fprintf( to, "%s %s %ld\n", name_type( look->type ), look->name, (long int)( look->value ));
			}
			fclose( to );
			return( true );
		}

		//
		//	create textual representation of the 'i'th symbol
		//	(starting at 0) return true if there is such a symbol.
		//
		bool show_symbol( int i, bool name_order, char *pattern, char *buffer, int max ) {
			char	expr[ max_buffer ];
			label	*look;
			
			if( i == 0 ) {
				bool	all;
				int	first;

				if( pattern != NULL ) {
					all = (( first = strlen( pattern )) == 0 );
				}
				else {
					all = true;
					first = 0;
				}
				for( look = _by_name; look; look = look->next_by_name ) {
					if(!( look->tag = all )) {
						look->tag = ( strncmp( pattern, look->name, first ) == 0 );
					}
				}
			}
			if( name_order ) {
				for( look = _by_name; look; look = look->next_by_name ) {
					if( look->tag ) if( i-- == 0 ) break;
				}
				if( look == NULL ) return( false );
				snprintf( buffer, max,  "%s/%s=%s", name_type( look->type ), look->name, constant( look->type, look->value, expr, max_buffer ));
				return( true );
			}
			for( look = _by_value; look; look = look->next_by_value ) {
				if( look->tag ) if( i-- == 0 ) break;
			}
			if( look == NULL ) return( false );
			snprintf( buffer, max,  "%s/%s=%s", name_type( look->type ), look->name, constant( look->type, look->value, expr, max_buffer ));
			return( true );
		}
			
};


#endif

//
//	EOF
//
