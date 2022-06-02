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
		const int	max_buffer = 128;
		bool		_tripped;
	public:
		ValidationReporter( void ) {
			_tripped = false;
		}
		virtual bool report( Level lvl, Modules from, int instance, Exception number ) {
			ASSERT( lvl == Validation_Level );
			ASSERT( from == Validation_Module );
			ASSERT( instance == 0 );
			ASSERT(( number == Abort_Simulation )||( number == Assertion_Failure ));
			
			char		buffer[ max_buffer ];
			
			fprintf( stderr, "[%s]\n", description( lvl, from, instance, number, buffer, max_buffer ));
			abort();
			return( true );
		}		
		virtual bool report( Level lvl, Modules from, int instance, Exception number, const char *fmt, ... ) {
			ASSERT( lvl == Validation_Level );
			ASSERT( from == Validation_Module );
			ASSERT( instance == 0 );
			ASSERT(( number == Abort_Simulation )||( number == Assertion_Failure ));

			char		buffer[ max_buffer ];
			va_list		args;
			
			va_start( args, fmt );
			fprintf( stderr, "[%s] ", description( lvl, from, instance, number, buffer, max_buffer ));
			vfprintf( stderr, fmt, args );
			fprintf( stderr, "\n" );
			abort();
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
//	Code to support code debugging.
//
#ifdef ENABLE_PAUSE
void pause_here( const char *file, int line ) {
	fprintf( stderr, "Pause in '%s' at line %d\n", file, line );
}
#endif


//
//	EOF
//
