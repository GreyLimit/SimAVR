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
		FILE	*_output,
			*_input;

		//
		//	Define a generic buffer size.
		//
		static const int max_buffer = 100;
		
		//
		//	Ask the console devise how to continue as a result
		//	of this error.
		//
		//		Return false for continue
		//		Return true for terminate (current action)
		//		Do not return for Abort (end program with core dump)
		//
		bool continue_terminate_abort( Level lvl ) {

			char reply[ max_buffer ];

			switch( lvl ) {
				case Debug_Level:	return( false );
				case Information_Level:	return( false );
				case Validation_Level: {
					abort();
					return( true );
				}
				default: {
					fprintf( _output, "(C)ontinue, (T)erminate or (A)bort? " );
					fflush( _output );
					while( fgets( reply, max_buffer, _input )) {
						switch( reply[ 0 ]) {
							case 'c':
							case 'C':	return( false );
							case 't':
							case 'T':	return( true );
							case 'a':
							case 'A': 	abort();
							default: {
								fprintf( _output, "C, T or A? " );
								fflush( _output );
								break;
							}
						}
					}
					break;
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
		}
		
		virtual bool raise( Level lvl, Modules from, Exception number ) {
			char buffer[ max_buffer ];

			fprintf( _output, "[%s]\n", description( lvl, from, number, buffer, max_buffer ));
			return( continue_terminate_abort( lvl ));
		}
		
		virtual bool raise( Level lvl, Modules from, Exception number, word arg ) {
			char buffer[ max_buffer ];

			fprintf( _output, "[%s] %d\n", description( lvl, from, number, buffer, max_buffer ), (int)arg );
			return( continue_terminate_abort( lvl ));
		}
		
		virtual bool raise( Level lvl, Modules from, Exception number, dword arg1, word arg2 ) {
			char buffer[ max_buffer ];

			fprintf( _output, "[%s] %ld, %d\n", description( lvl, from, number, buffer, max_buffer ), (long int)arg1, (int)arg2 );
			return( continue_terminate_abort( lvl ));
		}
		
		virtual bool raise( Level lvl, Modules from, Exception number, const char *mesg ) {
			char buffer[ max_buffer ];

			fprintf( _output, "[%s] %s\n", description( lvl, from, number, buffer, max_buffer ), mesg );
			return( continue_terminate_abort( lvl ));
		}
		
		virtual bool raise( Level lvl, Modules from, Exception number, const char *file, word line ) {
			char buffer[ max_buffer ];

			fprintf( _output, "[%s] '%s' line %d\n", description( lvl, from, number, buffer, max_buffer ), file, line );
			return( continue_terminate_abort( lvl ));
		}
};

#endif

//
//	EOF
