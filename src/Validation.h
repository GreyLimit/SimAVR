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

#endif

//
//	EOF
//

