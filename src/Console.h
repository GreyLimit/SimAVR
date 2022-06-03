//
//	Console.h
//	=========
//
//	Provide a reporting (and reply) mechanism through the
//	console device connected to the simulation.
//

#ifndef _CONSOLE_H_
#define _CONSOLE_H_

//
//	The C standard stdio system.
//
#include <stdio.h>
#include <stdlib.h>

//
//	Basics required.
//
#include "Base.h"

//
//	Bring in the Reporting API
//
#include "Reporter.h"

//
//	The Console system
//
class Console : public Reporter {
	private:
		//
		//	We will use indirect variables to the IO stream
		//	to enable extensions later on.
		//
		FILE		*_output,
				*_input;

		//
		//	The exception trip flag.
		//
		bool		_tripped;

		//
		//	Define a generic buffer size.
		//
		static const int max_buffer = 100;

		//
		//	Keep list of exceptions that are always ignored.
		//
		struct ignore_item {
			//
			//	What this record applies to.
			//
			Level		lvl;
			Modules		from;
			int		instance;
			Exception	number;
			//
			//	Permanent?  If not how many left to ignore.
			//
			bool		permanent;
			word		count;
			//
			ignore_item	*next;
		};
		ignore_item	*_ignore_list;
		
		//
		//	Find the address of an ignore record, if there is one.
		//
		ignore_item *find_ignore( Level lvl, Modules from, int instance, Exception number ) {
			for( ignore_item *p = _ignore_list; p != NULL; p = p->next ) {
				if(( lvl == p->lvl )&&( from == p->from )&&( instance == p->instance )&&( number == p->number )) return( p );
			}
			return( NULL );
		}

		//
		//	Routines to add and test ignore tests.
		//
		bool ignore_exception( Level lvl, Modules from, int instance, Exception number ) {
			ignore_item *p = find_ignore( lvl, from, instance, number );
			
			if( p == NULL ) return( false );
			if( p->permanent ) return( true );
			if( p->count > 0 ) {
				p->count -= 1;
				return( true );
			}
			return( false );
		}
		void set_ignore( Level lvl, Modules from, int instance, Exception number, word count ) {
			ignore_item *p = find_ignore( lvl, from, instance, number );
			
			if( p == NULL ) {
				p = new ignore_item;
				p->lvl = lvl;
				p->from = from;
				p->instance = instance;
				p->number = number;
				p->next = _ignore_list;
				_ignore_list = p;
			}
			p->permanent = ( count == 0 );
			p->count = count;
		}
		
		//
		//	Ask the console device how to continue as a result
		//	of this error.
		//
		//	C	Return false for continue this once
		//
		//	In	Return false for the next n times.  If n is
		//		missing or 0 then always return false.
		//
		//	B	Return false for continue, but terminates a
		//		run or trace command at this point.
		//
		//	F	Return true telling the simulation to cascade
		//		an error back through the system.
		//
		//	A	Do not return for Abort (end program with core dump)
		//
		bool choose_action( Level lvl, Modules from, int instance, Exception number ) {

			char reply[ max_buffer ], *dec;

			fprintf( _output, "(C)ontinue, (I)gnore, (B)reak, (F)ail or (A)bort? " );
			fflush( _output );
			while( fgets( reply, max_buffer, _input )) {
				dec = reply;
				switch( *dec++ ) {
					case 'c':
					case 'C': {
						return( false );
					}
					case 'i':
					case 'I': {
						set_ignore( lvl, from, instance, number, (word)atoi( dec ));
						return( false );
					}
					case 'b':
					case 'B': {
						_tripped = true;
						return( false );
					}
					case 'f':
					case 'F': {
						_tripped = true;
						return( true );
					}
					case 'a':
					case 'A': {
						abort();
						break;
					}
					default: {
						fprintf( _output, "C, I, B, F or A? " );
						fflush( _output );
						break;
					}
				}
			}
			//
			//	We will assume that we simply continue at this point.
			//
			return( false );
		}
			
	public:
		Console( void ) {
			_output = stdout;
			_input = stdin;
			_tripped = false;
			_ignore_list = NULL;
		}
		
		virtual bool report( Level lvl, Modules from, int instance, Exception number ) {
			char buffer[ max_buffer ];

			if( lvl == Information_Level ) {
				fprintf( _output, "[%s]\n", description( lvl, from, instance, number, buffer, max_buffer ));
				return( false );
			}
			if( ignore_exception( lvl, from, instance, number )) return( false );
			fprintf( _output, "[%s]\n", description( lvl, from, instance, number, buffer, max_buffer ));
			return( choose_action( lvl, from, instance, number ));
		}
		
		virtual bool report( Level lvl, Modules from, int instance, Exception number, const char *fmt, ... ) {
			char		buffer[ max_buffer ];
			va_list		args;
			
			
			if( lvl == Information_Level ) {
				va_start( args, fmt );
				fprintf( _output, "[%s] ", description( lvl, from, instance, number, buffer, max_buffer ));
				vfprintf( _output, fmt, args );
				fprintf( _output, "\n" );
				return( false );
			}
			if( ignore_exception( lvl, from, instance, number )) return( false );
			va_start( args, fmt );
			fprintf( _output, "[%s] ", description( lvl, from, instance, number, buffer, max_buffer ));
			vfprintf( _output, fmt, args );
			fprintf( _output, "\n" );
			return( choose_action( lvl, from, instance, number ));
		}
		

		//
		//	Check if an exception has been raised (using above
		//	routines).  Return true if any have since the last
		//	call to this routine.  Clear flag if set.
		//
		virtual bool exception( void ) {
			bool tripped = _tripped;
			_tripped = false;
			return( tripped );
		}
};

#endif

//
//	EOF
