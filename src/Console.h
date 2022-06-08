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
		//	Define a set of responses which can be selected
		//	from when a report is specifically identified.
		//
		enum Response {
			Do_Ask,		// Force system to ask the operator.
			Do_Hide,	// Continue operation without displaying report.
			Do_Display	// Continue operation after displaying the report.
		};

		//
		//	Keep list of exceptions that are specifically managed.
		//
		struct identify_item {
			//
			//	What this record applies to.
			//
			Level		lvl;
			Modules		from;
			int		instance;
			Exception	number;
			//
			//	What is our action when this crops up again?
			//
			Response	action;
			bool		permanent;
			word		count;
			//
			identify_item	*next;
		};
		identify_item	*_identify_list;
		
		//
		//	Find the address of an identify record, if there is one.
		//
		identify_item *find_identify( Level lvl, Modules from, int instance, Exception number ) {
			for( identify_item *p = _identify_list; p != NULL; p = p->next ) {
				if(( lvl == p->lvl )&&( from == p->from )&&( instance == p->instance )&&( number == p->number )) return( p );
			}
			return( NULL );
		}

		//
		//	Routines to add and test identify tests.
		//
		Response identify_exception( Level lvl, Modules from, int instance, Exception number ) {
			identify_item *p = find_identify( lvl, from, instance, number );
			
			if( p == NULL ) return( Do_Ask );
			if( p->permanent ) return( p->action );
			if( p->count > 0 ) {
				p->count -= 1;
				return( p->action );
			}
			return( Do_Ask );
		}
		void set_identify( Level lvl, Modules from, int instance, Exception number, bool forever, word count, Response action ) {
			identify_item *p = find_identify( lvl, from, instance, number );
			
			if( p == NULL ) {
				p = new identify_item;
				p->lvl = lvl;
				p->from = from;
				p->instance = instance;
				p->number = number;
				p->next = _identify_list;
				_identify_list = p;
			}
			p->action = action;
			p->permanent = forever;
			p->count = count;
		}
		
		//
		//	Ask the console device how to continue as a result
		//	of this error.  Return false if no remedial action
		//	is to be taken, true if the simulator should be
		//	reacting to the report.
		//
		//	C	Return false and continue this once
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
						set_identify( lvl, from, instance, number, ( *dec == ASTERIX ), (word)atoi( dec ), Do_Display );
						return( false );
					}
					case 'i':
					case 'I': {
						set_identify( lvl, from, instance, number, ( *dec == ASTERIX ), (word)atoi( dec ), Do_Hide );
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
			_identify_list = NULL;
		}
		
		virtual bool report( Level lvl, Modules from, int instance, Exception number ) {
			char		buffer[ max_buffer ];
			Response	rep;

			if(( rep = identify_exception( lvl, from, instance, number )) == Do_Hide ) return( false );
			fprintf( _output, "[%s]\n", description( lvl, from, instance, number, buffer, max_buffer ));
			if( rep == Do_Display ) return( false );
			return( choose_action( lvl, from, instance, number ));
		}
		
		virtual bool report( Level lvl, Modules from, int instance, Exception number, const char *fmt, ... ) {
			char		buffer[ max_buffer ];
			va_list		args;
			Response	rep;

			if(( rep = identify_exception( lvl, from, instance, number )) == Do_Hide ) return( false );
			va_start( args, fmt );
			fprintf( _output, "[%s] ", description( lvl, from, instance, number, buffer, max_buffer ));
			vfprintf( _output, fmt, args );
			fprintf( _output, "\n" );
			if( rep == Do_Display ) return( false );
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
