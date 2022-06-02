//
//	Validation.h
//	============
//
//	Provide code assurance basics
//

#ifndef _VALIDATION_H_
#define _VALIDATION_H_

//
//	All unsuccessful validation results are reported.
//
#include "Reporter.h"

//
//	Where we will send any bad news..
//
extern Reporter *validation_reports;

//
//	Validation primitives.
//
#define ABORT()		(void)validation_reports->report(Validation_Level,Validation_Module,0,Abort_Simulation,"Abort: file '%s', line %d",__FILE__,__LINE__)
#define ASSERT(v)	do{if(!(v))(void)validation_reports->report(Validation_Level,Validation_Module,0,Assertion_Failure,"Assert: file '%s', line %d",__FILE__,__LINE__);}while(0)
#define VALIDATION(p)	do{validation_reports=(p);}while(0)

//
//	Debugging primitive
//
//	Place the PAUSE( expr ) line in code
//	and set a breakpoint on pause_here()
//	routine.  The let the program run, and the
//	break will be tripped when the expression
//	evaluates true.
//

#ifdef ENABLE_PAUSE
//
//	If enabled...
//
extern void 		pause_here( const char *file, int line );
#define PAUSE(v)	do{if((v))pause_here(__FILE__,__LINE__);}while(false)

#else
//
//	If NOT enabled...
//
#define PAUSE(v)

#endif

#endif

//
//	EOF
//

