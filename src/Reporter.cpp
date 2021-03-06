//
//	Reporter.cpp
//	============
//

#include "Reporter.h"
#include "Validation.h"


//
//	We are going to code this safely, and not assume
//	the enumerated types are ordered meaningfully.
//

typedef struct {
	Modules		module;
	const char	*name;
} ModuleName;

static ModuleName modules[] = {
	{ Validation_Module,	"Validation"		},
	{ Clock_Module,		"Clock"			},
	{ CPU_Module,		"CPU"			},
	{ Flash_Module,		"Flash"			},
	{ Interrupt_Module,	"Interrupt"		},
	{ Program_Module,	"Program"		},
	{ Programmer_Module,	"Programmer"		},
	{ Map_Module,		"Map"			},
	{ SRAM_Module,		"SRAM"			},
	{ Fuse_Module,		"Fuse"			},
	{ Symbols_Module,	"Symbols"		},
	{ Timer_Module,		"Timer"			},
	{ Pin_Module,		"Pin"			},
	{ Port_Module,		"Port"			},
	{ Coverage_Module,	"Coverage"		},
	{ Application_Module,	"Application"		},
	{ Factory_Module,	"Factory"		},
	{ Serial_Module,	"Serial"		}
};

char *Reporter::module_name( Modules module, char *buffer, int len ) {
	
	ASSERT( buffer != NULL );
	ASSERT( len > 1 );
	
	for( int i = 0; i < sizeof( modules ); i++ ) {
		if( modules[ i ].module == module ) {
			buffer[ --len ] = EOS;
			return( strncpy( buffer, modules[ i ].name, len ));
		}
	}
	snprintf( buffer, len, "Module#%d", (int)module );
	return( buffer );
}





typedef struct {
	Level		lvl;
	const char	*name;
} LevelName;

static LevelName levels[] = {
	{ Debug_Level,		"Debug"		},
	{ Information_Level,	"Information"	},
	{ Warning_Level,	"Warning"	},
	{ Error_Level,		"Error"		},
	{ Terminate_Level,	"Terminate"	},
	{ Validation_Level,	"Validation"	}
};

char *Reporter::level_name( Level lvl, char *buffer, int len ) {
	
	ASSERT( buffer != NULL );
	ASSERT( len > 1 );
	
	for( int i = 0; i < sizeof( levels ); i++ ) {
		if( levels[ i ].lvl == lvl ) {
			buffer[ --len ] = EOS;
			return( strncpy( buffer, levels[ i ].name, len ));
		}
	}
	snprintf( buffer, len, "Level#%d", (int)lvl );
	return( buffer );
}




typedef struct {
	Exception	cause;
	const char	*name;
} ExceptionName;

static ExceptionName exceptions[] = {
	{ Abort_Simulation,		"Abort"			},
	{ Assertion_Failure,		"Assert Failed"		},
	{ Not_Implemented,		"Not Implemented"	},
	
	{ File_Open_Failed,		"Open Failed"		},
	{ Program_Too_Big,		"Program too big"	},
	{ Line_Too_Long,		"Line too long"		},
	{ Format_Error,			"Formattng error"	},
	{ Checksum_Error,		"Checksum error"	},
	{ Record_Error,			"Record error"		},
	{ Program_Truncated,		"Program truncated"	},
	{ Address_Wraps,		"Address wraps"		},
	
	{ Not_Supported,		"Not supported"		},
	{ Invalid_Identifier,		"Identifier invalid"	},
	{ Invalid_Number,		"Number invalid"	},
	{ Overlap_Error,		"Objects Overlap"	},
	{ Too_Fast,			"Sub clock too quick"	},
	
	{ Config_Change,		"Config Change"		},
		
	{ Address_OOR,			"Address OOR"		},
	{ Data_OOR,			"Data OOR"		},
	{ Register_OOR,			"Register OOR"		},
	{ Source_ORR,			"Source OOR"		},
	{ Destination_OOR,		"destination OOR"	},
	{ Interrupt_OOR,		"Interrupt OOR"		},
	{ Device_Missing,		"Hardware incomplete"	},
	{ Read_Only,			"Read only"		},
	{ Read_Invalid,			"Read invalid"		},
	{ Write_Only,			"Write only"		},
	{ Write_Invalid,		"Write invalid"		},
	{ Restore_Invalid,		"Restore invalid"	},
	{ Parameter_Invalid,		"Parameter invalid"	},
	{ Feature_Disabled,		"Feature disabled"	},
	{ Feature_Reserved,		"Feature reserved"	},
	{ Illegal_Instruction,		"Illegal inst"		},
	{ Unsupported_Instruction,	"Unsupported inst"	},
	{ Reserved_Instruction,		"Reserved inst"		},
	
	{ Hardware_Break,		"CPU BREAK"		},
	{ Hardware_Sleep,		"CPU SLEEP"		},
	{ Watchdog_Reset,		"CPU WDT Reset"		},
	{ Watchdog_tick,		"CPU WDT tick"		},
	{ Skip_Instruction,		"CPU Skip inst"		},
	{ Accept_Interrupt,		"CPU Accept IRQ"	},
	
	{ Unexplained_Error,		"Unexplained error"	}
};

char *Reporter::exception_name( Exception cause, char *buffer, int len ) {
	
	ASSERT( buffer != NULL );
	ASSERT( len > 1 );
	
	for( int i = 0; i < sizeof( exceptions ); i++ ) {
		if( exceptions[ i ].cause == cause ) {
			buffer[ --len ] = EOS;
			return( strncpy( buffer, exceptions[ i ].name, len ));
		}
	}
	snprintf( buffer, len, "Exception#%d", (int)cause );
	return( buffer );
}


//
//	All three above, combined.
//
char *Reporter::description( Level lvl, Modules module, int instance, Exception cause, char *buffer, int len ) {

	static const int max_lvl = 15;
	static const int max_module = 15;
	static const int max_exception = 20;

	char	lvl_desc[ max_lvl ],
		module_desc[ max_module ],
		exception_desc[ max_exception ];

	snprintf( buffer, len, "%s/%s.%d/%s",	level_name( lvl, lvl_desc, max_lvl ),
						module_name( module, module_desc, max_module ),
						instance,
						exception_name( cause, exception_desc, max_exception ));
	return( buffer );
}



//
//	EOF
//
