//
//	SerialIO.h
//	==========
//
//	Base Serial IO API behind which anything could be placed.
//

#ifndef _SERIAL_IO_H_
#define _SERIAL_IO_H_

#include <stdio.h>

class SerialIO {
	public:
		//
		//	The API to send and receive data through
		//	the serial device
		//
		virtual void write( byte c ) = 0;
		virtual bool read( byte *c ) = 0;
		//
		//	The API to enable the output to be displayed
		//	and input to be provided.
		//
		virtual void display( FILE *to ) = 0;
		virtual void supply( char c ) = 0;
};

#endif

//
//	EOF
//
