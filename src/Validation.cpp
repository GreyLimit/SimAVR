//
//	Validation.cpp
//	==============
//

#include <cstddef>

#include <stdio.h>
#include <stdlib.h>

#include "Validation.h"

//
//	Create a default Validation Reporter class that will
//	be used if it is not over ridden by re-writing the
//	validation_reports pointer.
//

static class ValidationReporter : public Reporter {
	private:
		bool	_tripped;
	public:
		ValidationReporter( void ) {
			_tripped = false;
		}
		virtual bool raise( Level lvl, Modules from, Exception number ){ ABORT(); return( true ); }		
		virtual bool raise( Level lvl, Modules from, Exception number, word arg ) { ABORT(); return( true ); }
		virtual bool raise( Level lvl, Modules from, Exception number, dword arg1, word arg2 ) { ABORT(); return( true ); }
		virtual bool raise( Level lvl, Modules from, Exception number, int instance, const char *mesg ) { ABORT(); return( true ); }
		virtual bool raise( Level lvl, Modules from, Exception number, int instance, const char *mesg, word arg ) { ABORT(); return( true ); }
		
		virtual bool raise( Level lvl, Modules from, Exception number, const char *file, word line ) {
			ASSERT( lvl == Validation_Level );
			ASSERT( from == Validation_Module );
			switch( number ) {
				case Abort_Simulation: {
					fprintf( stderr, "System Abort: Module \"%s\" line %d\n", file, line );
					abort();
					break;
				}
				case Assertion_Failure: {
					fprintf( stderr, "Assert Failure: Module \"%s\" line %d\n", file, line );
					abort();
					break;
				}
				default: {
					ABORT();
					break;
				}
			}
			return( true );
		}
		virtual bool exception( void ) {
			bool	tripped;

			tripped = _tripped;
			_tripped = false;
			return( tripped );
		}
} default_validation;

//
//	Define the pointer used by this code.
//
Reporter *validation_reports = &( default_validation );

//
//	EOF
//
