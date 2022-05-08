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
			Level		lvl;
			Modules		from;
			Exception	number;
			ignore_item	*next;
		};
		ignore_item	*_ignore_list;

		//
		//	Routines to add and test ignore tests.
		//
		bool ignore_exception( Level lvl, Modules from, Exception number ) {
			for( ignore_item *p = _ignore_list; p != NULL; p = p->next ) {
				if(( lvl == p->lvl )&&( from == p->from )&&( number == p->number )) return( true );
			}
			return( false );
		}
		void ignore_always( Level lvl, Modules from, Exception number ) {
			ignore_item *p;
			
			ASSERT( !ignore_exception( lvl, from, number ));
			p = new ignore_item;
			p->lvl = lvl;
			p->from = from;
			p->number = number;
			p->next = _ignore_list;
			_ignore_list = p;
		}
		
		//
		//	Ask the console devise how to continue as a result
		//	of this error.
		//
		//		Return false for continue
		//		Return true for terminate (current action)
		//		Do not return for Abort (end program with core dump)
		//
		bool continue_fail_abort( Level lvl, Modules from, Exception number ) {

			char reply[ max_buffer ];

			if( ignore_exception( lvl, from, number )) return( false );
			fprintf( _output, "(C)ontinue, (I)gnore, (B)reak, (F)ail or (A)bort? " );
			fflush( _output );
			while( fgets( reply, max_buffer, _input )) {
				switch( reply[ 0 ]) {
					case 'c':
					case 'C': {
						return( false );
					}
					case 'i':
					case 'I': {
						ignore_always( lvl, from, number );
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
		
		virtual bool raise( Level lvl, Modules from, Exception number ) {
			char buffer[ max_buffer ];

			fprintf( _output, "[%s]\n", description( lvl, from, number, buffer, max_buffer ));
			return( continue_fail_abort( lvl, from, number ));
		}
		
		virtual bool raise( Level lvl, Modules from, Exception number, word arg ) {
			char buffer[ max_buffer ];

			fprintf( _output, "[%s] %d\n", description( lvl, from, number, buffer, max_buffer ), (int)arg );
			return( continue_fail_abort( lvl, from, number ));
		}
		
		virtual bool raise( Level lvl, Modules from, Exception number, dword arg1, word arg2 ) {
			char buffer[ max_buffer ];

			fprintf( _output, "[%s] %ld, %d\n", description( lvl, from, number, buffer, max_buffer ), (long int)arg1, (int)arg2 );
			return( continue_fail_abort( lvl, from, number ));
		}
		
		virtual bool raise( Level lvl, Modules from, Exception number, int instance, const char *mesg ) {
			char buffer[ max_buffer ];

			fprintf( _output, "[%s] %d:  %s\n", description( lvl, from, number, buffer, max_buffer ), instance, mesg );
			return( continue_fail_abort( lvl, from, number ));
		}
		
		virtual bool raise( Level lvl, Modules from, Exception number, int instance, const char *mesg, word arg ) {
			char buffer[ max_buffer ];

			fprintf( _output, "[%s] %d:  %s %d\n", description( lvl, from, number, buffer, max_buffer ), instance, mesg, arg );
			return( continue_fail_abort( lvl, from, number ));
		}
		
		virtual bool raise( Level lvl, Modules from, Exception number, const char *file, word line ) {
			char buffer[ max_buffer ];

			fprintf( _output, "[%s] '%s' line %d\n", description( lvl, from, number, buffer, max_buffer ), file, line );
			return( continue_fail_abort( lvl, from, number ));
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
