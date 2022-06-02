//
//	Interrupts.h
//
//	Define a class used to capture, deliver and clear records of
//	interrupts raised.  This system operates interrupt numbers
//	from 1 to N, giving N interrupts.
//

#ifndef _INTERRUPTS_H_
#define _INTERRUPTS_H_

//
//	Base types required.
//
#include "Base.h"
#include "Reporter.h"

//
//	The documentation provides the following details on how the
//	interrupts are allocated:
//
//	Vector No. Program Address Source Interrupt Definition
//
//	1	0x000 RESET External Pin, Power-on Reset, Brown-out Reset and Watchdog System Reset
//	2	0x001 INT0 External Interrupt Request 0
//	3	0x002 INT1 External Interrupt Request 1
//	4	0x003 PCINT0 Pin Change Interrupt Request 0
//	5	0x004 PCINT1 Pin Change Interrupt Request 1
//	6	0x005 PCINT2 Pin Change Interrupt Request 2
//	7	0x006 WDT Watchdog Time-out Interrupt
//	8	0x007 TIMER2_COMPA Timer/Counter2 Compare Match A
//	9	0x008 TIMER2_COMPB Timer/Counter2 Compare Match B
//	10	0x009 TIMER2_OVF Timer/Counter2 Overflow
//	11	0x00A TIMER1_CAPT Timer/Counter1 Capture Event
//	12	0x00B TIMER1_COMPA Timer/Counter1 Compare Match A
//	13	0x00C TIMER1_COMPB Timer/Counter1 Compare Match B
//	14	0x00D TIMER1_OVF Timer/Counter1 Overflow
//	15	0x00E TIMER0_COMPA Timer/Counter0 Compare Match A
//	16	0x00F TIMER0_COMPB Timer/Counter0 Compare Match B
//	17	0x010 TIMER0_OVF Timer/Counter0 Overflow
//	18	0x011 SPI_STC SPI Serial Transfer Complete
//	19	0x012 USART_RX USART Rx Complete
//	20	0x013 USART_UDRE USART, Data Register Empty
//	21	0x014 USART_TX USART, Tx Complete
//	22	0x015 ADC ADC Conversion Complete
//	23	0x016 EE_READY EEPROM Ready
//	24	0x017 ANALOG_COMP Analog Comparator
//	25	0x018 TWI 2-wire Serial Interface
//	26	0x019 SPM_Ready Store Program Memory Ready
//
//	This module has no requirement to know which (or how many) irqs
//	do what, but gathering this info here seems like a good idea.
//

//
//	The Interrupts API class.
//
class Interrupts {
	public:
		//
		//	Reset the IRQ system
		//
		virtual void reset( void ) = 0;

		//
		//	Raise an interrupt, and raise an interrupt
		//	with an auto clear action.
		//
		virtual void raise( byte number ) = 0;
		virtual void raise( byte number, byte *locn, byte flag ) = 0;		

		//
		//	Clear an interrupt...
		//
		virtual void clear( byte number ) = 0;

		//
		//	Find an active and raised interrupt, returns true if one found and
		//	completes the error number.
		//
		virtual bool find( byte *found ) = 0;

		//
		//	Mask an interrupt (make inactive).
		//
		virtual void mask( byte number ) = 0;
		
		//
		//	Unmask an interrupt (make active).
		//
		virtual void unmask( byte number ) = 0;
};

//
//	Template Interrupt class.
//
template< byte last_irq > class InterruptDevice : public Interrupts {
	private:
		//
		//	Set up an array maximum value to simplify
		//	code in the module.
		//
		static const byte total_irqs = last_irq + 1;
		
		//
		//	Reporter..
		//
		Reporter	*_reporter;
		int		_instance;

		//
		//	An individual interrupt action.
		//
		struct status {
			bool	pending,
				active,
				clear_flag;
			byte	*locn,
				flag;
		};
		
		//
		//	Array of interrupts pending.
		//
		status	_irq[ total_irqs ];
		byte	_raised;

	public:
		//
		//	Start empty.
		//
		InterruptDevice( Reporter *handler, int instance ) {
			_reporter = handler;
			_instance = instance;
			reset();
		}

		//
		//	Reset all interrupts
		//
		virtual void reset( void ) {
			for( byte i = 0; i < total_irqs; i++ ) {
				_irq[ i ].pending = false;
				_irq[ i ].active = true;
				_irq[ i ].clear_flag = false;
				_irq[ i ].locn = NULL;
				_irq[ i ].flag = 0;
			}
			_raised = 0;
		}

		//
		//	Raise an interrupt...
		//
		virtual void raise( byte number ) {
			if(( number > 0 )&&( number < total_irqs )) {
				status *irq = &( _irq[ number ]);
				
				if( !irq->pending ) {
					irq->pending = true;
					irq->clear_flag = false;
					if( irq->active ) _raised++;
				}
			}
			else {
				_reporter->report( Error_Level, Interrupt_Module, _instance, Interrupt_OOR, "IRQ number %d out of range", (int)number );
			}
		}
		virtual void raise( byte number, byte *locn, byte flag ) {
			if(( number > 0 )&&( number < total_irqs )) {
				status *irq = &( _irq[ number ]);
				
				if( !irq->pending ) {
					irq->pending = true;
					irq->clear_flag = true;
					irq->locn = locn;
					irq->flag = flag;
					if( irq->active ) _raised++;
				}
			}
			else {
				_reporter->report( Error_Level, Interrupt_Module, _instance, Interrupt_OOR, "IRQ number %d out of range", (int)number );
			}
		}		

		//
		//	Clear an interrupt...
		//
		virtual void clear( byte number ) {
			if(( number > 0 )&&( number < total_irqs )) {
				status *irq = &( _irq[ number ]);
				
				if( irq->pending ) {
					irq->pending = false;
					irq->clear_flag = false;
					if( irq->active ) _raised--;
				}
			}
			else {
				_reporter->report( Error_Level, Interrupt_Module, _instance, Interrupt_OOR, "IRQ number %d out of range", (int)number );
			}
		}

		//
		//	Find an active and raised interrupt, returns true if one found and
		//	completes the error number.
		//
		virtual bool find( byte *found ) {
			if( _raised ) {
				for( byte i = 1; i < total_irqs; i++ ) {
					status *irq = &( _irq[ i ]);
				
					if( irq->active && irq->pending ) {
						*found = i;
						if( irq->clear_flag ) *( irq->locn ) &= ~irq->flag;
						return( true );
					}
				}
			}
			return( false );
		}

		//
		//	Mask an interrupt (make inactive).
		//
		virtual void mask( byte number ) {
			if(( number > 0 )&&( number < total_irqs )) {
				status *irq = &( _irq[ number ]);
				
				if( irq->active ) {
					irq->active = false;
					if( irq->pending ) _raised--;
				}
			}
			else {
				_reporter->report( Error_Level, Interrupt_Module, _instance, Interrupt_OOR, "IRQ number %d out of range", (int)number );
			}
		}
		
		//
		//	Unmask an interrupt (make active).
		//
		virtual void unmask( byte number ) {
			if(( number > 0 )&&( number < total_irqs )) {
				status *irq = &( _irq[ number ]);
				
				if( !irq->active ) {
					irq->active = true;
					if( irq->pending ) _raised++;
				}
			}
			else {
				_reporter->report( Error_Level, Interrupt_Module, _instance, Interrupt_OOR, "IRQ number %d out of range", (int)number );
			}
		}
};

#endif

//
//	EOF
//
