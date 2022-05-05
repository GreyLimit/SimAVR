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
#define ABORT()		(void)validation_reports->raise(Validation_Level,Validation_Module,Abort_Simulation,__FILE__,__LINE__)
#define ASSERT(v)	do{if(!(v))(void)validation_reports->raise(Validation_Level,Validation_Module,Assertion_Failure,__FILE__,__LINE__);}while(0)
#define VALIDATION(p)	do{validation_reports=(p);}while(0)

#endif

//
//	EOF
//

