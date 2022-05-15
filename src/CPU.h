//
//	CPU.h
//
//	The generic CPU API class.
//

#ifndef _CPU_H_
#define _CPU_H_

//
//	We use Symbols.
//
#include "Symbols.h"

//
//	These are our valid addressing domains
//
typedef enum {
	Register_Address,
	Port_Address,
	Memory_Address
} AddressDomain;

//
//	Base Types
//
class CPU {
	public:
		//
		//	Reset CPU to initial condition ready for first
		//	instruction execution.
		//
		virtual void reset( void ) = 0;
		
		//
		//	Execute a single instruction, move PC forward, accepts
		//	interrupts etc as necessary.
		//
		virtual void step( void ) = 0;

		//
		//	Disassemble the instruction at address
		//
		//	Returns number of opcodes used by the disassembly.
		//
		virtual word disassemble( dword address, Symbols *labels, char *buffer, int max ) = 0;

		//
		//	Return the address of the next instruction to execute.
		//
		virtual dword next_instruction( void ) = 0;

		//
		//	Place textual representation of an object (adrs in domain) into the buffer
		//	supplied.
		//
		//	Return true if there was something there, false otherwise.
		//
		virtual bool examine( AddressDomain domain, word adrs, Symbols *labels, char *buffer, int max ) = 0;
};

#endif

//
//	EOF
//
