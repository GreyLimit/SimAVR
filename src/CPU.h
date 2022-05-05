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
};

#endif

//
//	EOF
//
