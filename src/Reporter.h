//
//	Reporter.h
//
//	Define the interface to the exception
//	mechanism.
//

#ifndef _REPORTER_H_
#define _REPORTER_H_

//
//	Base Types
//
#include "Base.h"

//
//	Define all of the modules which can raise an exception.
//
typedef enum {
	Validation_Module,
	Clock_Module,
	CPU_Module,
	Flash_Module,
	Interrupt_Module,
	Program_Module,
	Programmer_Module,
	Map_Module,
	SRAM_Module,
	Fuse_Module,
	Symbols_Module,
	Timer_Module,
	Application_Module
} Modules;

//
//	Define the various levels which reports which can be submitted under.
//
typedef enum {
	Debug_Level		= 0,
	Information_Level,
	Warning_Level,
	Error_Level,
	Terminate_Level,
	Validation_Level
} Level;


//
//	Define all of the Exceptions which we can except.
//
typedef enum {
	//
	//	Exceptions specific to the simulation software
	//
	Abort_Simulation,		// Unexpected Code Execution.
	Assertion_Failure,		// Code assumption invalid.
	Not_Implemented,		// Feature not implemented (yet).

	//
	//	Program loading problems.
	//
	File_Open_Failed,
	Program_Too_Big,
	Line_Too_Long,
	Format_Error,
	Checksum_Error,
	Record_Error,
	Program_Truncated,
	Address_Wraps,

	//
	//	Exceptions capturing errors in the simulation
	//
	Not_Supported,			// Requested API function not supported.
	Ticker_Full,			// Failed to register with the clock.

	//
	//	Exceptions specific to the execution of the simulation
	//
	Address_OOR,			// OOR -> Out Of Range
	Data_OOR,
	Register_OOR,
	Source_ORR,
	Destination_OOR,
	Interrupt_OOR,
	Read_Only,
	Write_Only,
	Write_Invalid,			// Write operation not valid (writing to locked section).
	Restore_Invalid,		// Typically 'undoing' a programmed lock/fuse (ie 0 -> 1 )
	Parameter_Invalid,		// Hardware being asked to process an invalid parameter.
	Feature_Disabled,		// Querying/Testing hardware/feature that is not enabled.
	Feature_Reserved,		// Querying/Testing hardware/feature that is set to a reserved state.

	//
	//	Exceptions raised by the simulation
	//
	Illegal_Instruction,		// Opcode does not map to valid AVR MCU instruction
	Unsupported_Instruction,	// Instruction not available on this model AVR MCU
	Reserved_Instruction,		// Instruction set aside for future use
	Hardware_Break,			// AVR executes debugging break point
	Hardware_Sleep,			// AVR MCU enters sleep mode

	//
	//	Catch all exception
	//
	Unexplained_Error	
} Exception;

//
//	The Reporter API
//
class Reporter {
	protected:
		//
		//	Provide mechanisms to convert enumerated values
		//	to printable equivalent text.
		//
		//	Routine returns buffer address containing EOS
		//	terminated string on no more than len bytes
		//	including the EOS.
		//
		char *module_name( Modules module, char *buffer, int len );
		char *level_name( Level lvl, char *buffer, int len );
		char *exception_name( Exception cause, char *buffer, int len );
		//
		//	All three above, combined.
		//
		char *description( Level lvl, Modules module, Exception cause, char *buffer, int len );

	public:
		//
		//	Reports of all types funnelled through these
		//	calls.  Return true if the exception raised
		//	should terminate the action which caused the
		//	issue.
		//
		virtual bool raise( Level lvl, Modules from, Exception number ) = 0;
		virtual bool raise( Level lvl, Modules from, Exception number, word arg ) = 0;
		virtual bool raise( Level lvl, Modules from, Exception number, dword arg1, word arg2 ) = 0;
		virtual bool raise( Level lvl, Modules from, Exception number, const char *mesg ) = 0;
		virtual bool raise( Level lvl, Modules from, Exception number, const char *file, word line ) = 0;
};

#endif

//
//	EOF
//