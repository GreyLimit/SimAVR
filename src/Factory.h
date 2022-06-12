//
//	Factory.h
//	=========
//
//	Define the class that "makes" other classes
//

#ifndef _FACTORY_H_
#define _FACTORY_H_

#include "Base.h"
#include "SerialIO.h"

class Factory {
	public:
		virtual SerialIO *serial_io( int instance ) = 0;
};
		


#endif

//
//	EOF
//
