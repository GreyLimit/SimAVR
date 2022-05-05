//
//	Programmer.h
//	============
//
//	The implementation of the Self Programming system.
//
//

#ifndef _PROGRAMMER_H_
#define _PROGRAMMER_H_

//
//	The programmer code rest on the following systems:
//
#include "Base.h"
#include "AVR_CPU.h"
#include "Clock.h"
#include "Memory.h"
#include "Flash.h"
#include "Validation.h"
#include "Interrupts.h"
#include "Fuses.h"
#include "DeviceRegister.h"

//
//	The "Programming Module" needs the following:
//
//	Clock Module:
//
//		To provide regular CPU time to simulate writing flash
//		memory, and to control the limited time window between
//		the IO port being updated and the SPM calls being made.
//
//	Memory API:
//
//		Needed to provide the IO port itself which is accessed
//		through the memory access interface
//
//	Flash API:
//
//		How else do you write to it?
//
//	Validation Module
//
//		So we know if things are not right.
//
//	Interrupts Module;
//
//		We will need to raise interrupts (if requested).
//

//
//	Flash Memory Sections
//	---------------------
//
//	The flash memory is (on most MCUs) divided into two sections:
//
//		a)	RWW Section (Read While Writing)
//		b)	NRWW Section (Non-Read While Writing)
//
//	Sections are always a whole number of pages.
//
//	The difference between these sections:
//
//		a)	While writing to the RWW Section, data can still
//			be accessed from the NRWW Section (program code).
//
//			The RWW Section is the "main application" space, and
//			represents the bulk of the flash pages from "word 0,
//			page 0" (word address 0).  
//
//		b)	While writing to the NRWW Section no other flash
//			IO can be undertaken (CPU waits for operation to
//			complete).
//
//			The NRWW Section is reserved for the "boot loader",
//			and starts at the first page after the end of the
//			"application code" through to the last page in flash
//			memory.
//
//	The division point between RWW and NRWW Section is set by a
//	combination of the MCU type and the Fuses.
//			


//
//	The Self Programming Class
//
class Programmer : public Tick, Notification {
	public:
		//
		//	This is the handle that the Device Register will use
		//	when communicating updates to the control register.
		//
		static const word SPMCSR = 0;
		
	public:
		//
		//	Define the SPM Commands this module understands.
		//
		//	Return the number of *additional* clock cycles the
		//	call required (opcode read will have already been
		//	accounted for).
		//
		virtual word call_spm( dword from, bool increment ) = 0;

		//
		//	Define the LPM command this module understands.
		//
		//	return 0 if this is not the right time for this
		//	code to execute, or the number of clock cycles this
		//	code required to do its thing.
		//
		virtual word call_lpm( dword from, bool increment ) = 0;
			
		//
		//	Clock Tick API
		//	==============
		//
		//	Called once for every tick which the
		//	clock is simulating.
		//
		//	This gets called frequently and is used to
		//	simulate activities happening in parallel
		//	while other tasks are executing.
		//
		virtual void tick( void ) = 0;

		//
		//	Notification API
		//	================
		//
		//	Device register updates appear through here.
		//
		virtual byte read_register( word id ) = 0;
		virtual void write_register( word id, byte value ) = 0;
};


//
//	The Self Programming Class
//
template< word instance, byte irq_number > class ProgrammerDevice : public Programmer {
	public:
		//
		//	This is the handle that the DeviceRegister will use
		//	when communicating updates to the control register.
		//
		static const word SPMCSR = 0;
		
		//
		//	The logical address of the locking bits.
		//
		static const word locking_bits = 1;
				
	private:
		//
		//	Save pointers to all the things we need to access
		//	to facilitate this device function.
		//
		Interrupts	*_irq;
		Clock		*_clock;
		Flash		*_flash;
		Fuses		*_config;
		AVR_CPU		*_mcu;
		Reporter	*_report;

		//
		//	Save the details of the Flash memory we are
		//	interfacing with.
		//
		dword		_flash_total_size,
				_flash_boot_start;

		//
		//	This is the SPMCSR register.
		//
		//	Bit	7	6	5	4	3	2	1	0
		//		SPMIE	RWWSB	SIGRD	RWWSRE	BLBSET	PGWRT	PGERS	SPMEN
		//		R/W	RO	R/W	R/W	R/W	R/W	R/W	R/W
		//
		byte		_spmcsr;
		//
		//	Define aliases for the individual bits.
		//
		static const byte bit_SPMIE = BIT( byte, 7 );
		static const byte bit_RWWSB = BIT( byte, 6 );
		static const byte bit_SIGRD = BIT( byte, 5 );
		static const byte bit_RWWSRE = BIT( byte, 4 );
		static const byte bit_BLBSET = BIT( byte, 3 );
		static const byte bit_PGWRT = BIT( byte, 2 );
		static const byte bit_PGERS = BIT( byte, 1 );
		static const byte bit_SPMEN = BIT( byte, 0 );
		//
		//	Define an alias for the operation control bits.
		//
		static const byte control_mask = bit_SIGRD | bit_RWWSRE | bit_BLBSET | bit_PGWRT | bit_PGERS | bit_SPMEN;
		//
		//	Flag is true if interrupts are enabled.
		//
		bool		_int_enable;
		//
		//	Enumerate the various actions that are possible
		//	through setting specific values into the SPMCSR
		//	register.
		//
		enum {
			PM_EMPTY,	// No action pending.
			PM_SIGRD,	// Bit 5 - SIGRD: Read signature byte
			PM_RWWSRE,	// Bit 4 – RWWSRE: Read-While-Write Section Read Enable
			PM_BLBSET,	// Bit 3 – BLBSET: Boot Lock Bit Set
			PM_PGWRT,	// Bit 2 – PGWRT: Page Write
			PM_PGERS,	// Bit 1 – PGERS: Page Erase
			PM_SPMEN	// Bit 0 – SPMEN: Self Programming Enable
					//		: Write word to temp page
		}
				_pm_mode;

		//
		//	Define the SPM clock window counter.
		//
		word		_action_counter,
				_parallel_counter;
				
		//
		//	Routine to update the content of
		//	the SPMCSR register and corresponding
		//	flags.
		//
		void update_spmcsr( byte value ) {
			_spmcsr		= ( _spmcsr & bit_RWWSB ) | ( value & ~bit_RWWSB );
			_int_enable	= (( value & bit_SPMIE ) != 0 );
			switch( value & control_mask ) {
				case bit_SIGRD | bit_SPMEN: {
					//
					//	SIGRD: Read signature byte
					//
					_pm_mode = PM_SIGRD;
					_action_counter = 3;
					break;
				}
				case bit_RWWSRE | bit_SPMEN: {
					//
					//	RWWSRE: Read-While-Write Section Read Enable.
					//
					_pm_mode = PM_RWWSRE;
					_action_counter = 4;
					break;
				}
				case bit_BLBSET | bit_SPMEN: {
					//
					//	BLBSET: Boot Lock Bit Set
					//
					_pm_mode = PM_BLBSET;
					_action_counter = 4;
					break;
				}
				case bit_PGWRT | bit_SPMEN: {
					//
					//	PGWRT: Page Write
					//
					_pm_mode = PM_PGWRT;
					_action_counter = 4;
					break;
				}
				case bit_PGERS | bit_SPMEN: {
					//
					//	PGERS: Page Erase
					//
					_pm_mode = PM_PGERS;
					_action_counter = 4;
					break;
				}
				case bit_SPMEN: {
					//
					//	SPMEN: Write to flash buffer page
					//
					_pm_mode = PM_SPMEN;
					_action_counter = 4;
					break;
				}
				default: {
					//
					//	Anything else is invalid and ignored, though
					//	we will report this as an error.
					//
					_report->raise( Error_Level, Programmer_Module, Parameter_Invalid, value );
					_pm_mode = PM_EMPTY;
					_action_counter = 0;
					break;
				}
			}
		}
		
		//
		//	Return the page number appropriate for the command.
		//
		word target_page( bool inc ) {
			if( inc ) return( _flash->page_number( _mcu->inc_rampz_rz()));
			return( _flash->page_number( _mcu->get_rampz_rz()));
		}
					
		//
		//	Return the word number appropriate for the command.
		//
		word target_word( bool inc ) {
			if( inc ) return( _flash->word_number( _mcu->inc_rampz_rz()));
			return( _flash->word_number( _mcu->get_rampz_rz()));
		}
					
	public:
		//
		//	Constructor.
		//	------------
		//
		ProgrammerDevice( Reporter *report, Flash *flash, AVR_CPU *mcu, Interrupts *irq_handler, Clock *clock, Fuses *fuses ) {
			//
			//	Empty the control register
			//
			_spmcsr = 0;
			_int_enable = false;
			_pm_mode = PM_EMPTY;
			//
			//	Save the link to the external systems.
			//
			_report = report;
			_flash = flash;
			_mcu = mcu;
			_irq = irq_handler;
			_clock = clock;
			_config = fuses;
			//
			//	Clear count down timers.
			//
			_action_counter = 0;
			_parallel_counter = 0;
			//
			//	Import our Lock and Fuse based configuration.
			//
			_flash_total_size = (dword)flash->total_pages() * (dword)flash->page_size();
			_flash_boot_start = _flash_total_size - (dword)fuses->BOOTSZ();
		}

		//
		//	Define the SPM Commands this module understands.
		//
		//	Return the number of *additional* clock cycles the
		//	call required (opcode read will have already been
		//	accounted for).
		//
		virtual word call_spm( dword from, bool increment ) {
			word	clocks;
			
			//
			//	Has this happened in the clock window
			//	during which the command is valid?
			//
			if( _action_counter == 0 ) return( 0 );
			//
			//	Clear to ensure only activated once.
			//
			_action_counter = 0;
			//
			//	The SPM instruction choices:
			//
			switch( _pm_mode ) {
				case PM_RWWSRE: {
					//
					//	Read-While-Write Section Read Enable
					//
					//	Setting this bit to one enables the RWW
					//	(Read While Write) feature of the flash
					//	memory that supports it.
					//
					_parallel_counter = 0;
					_spmcsr &= ~( bit_RWWSB | bit_RWWSRE | bit_SPMEN );
					_flash->enable();
					return( 0 );
				}
				case PM_BLBSET: {
					//
					//	The SPM instruction sets Boot Lock bits and
					//	Memory Lock bits, according to the data in R0.
					//
					_spmcsr &= ~( bit_BLBSET | bit_SPMEN );
					_config->write( locking_bits, _mcu->read_reg( 0 ));
					break;
				}
				case PM_PGWRT: {
					//
					//	The SPM instruction executes Page Write, from
					//	the data stored in the temporary buffer. The
					//	page address is taken from the high part of the
					//	Z-pointer.
					//
					_spmcsr |= bit_RWWSB;
					_spmcsr &= ~bit_PGWRT;
					_parallel_counter = _clock->micros( _flash->write( target_page( increment )));
					break;
				}
				case PM_PGERS: {
					//
					//	Executes Page Erase. The page address is taken
					//	from the high part of the Z-pointer.
					//
					_spmcsr |= bit_RWWSB;
					_parallel_counter = _clock->micros( _flash->erase( target_page( increment )));
					break;
				}
				case PM_SPMEN: {
					//
					//	Only SPMEN is set, the SPM instruction will
					//	store the value in R1:R0 in the temporary page
					//	buffer addressed by the Z-pointer. The LSB of
					//	the Z-pointer is ignored.
					//
					_spmcsr &= ~bit_RWWSB;
					_parallel_counter = 0;
					_flash->enable();
					_flash->place( target_word( increment ), _mcu->get_word_reg( 0 ));
					break;
				}
				default: {
					//
					//	This was an invalid call to SPM
					//
					_report->raise( Warning_Level, Programmer_Module, Parameter_Invalid, _spmcsr );
					break;
				}
			}
			return( clocks );
		}
		//
		//	Define the LPM command this module understands.
		//
		//	return 0 if this is not the right time for this
		//	code to execute, or the number of clock cycles this
		//	code required to do its thing.
		//
		virtual word call_lpm( dword from, bool increment ) {
			//
			//	This is the test to see if this code
			//	is going to hijack the LPM instruction.
			//
			if( !_action_counter ) return( 0 );
			//
			//	Clear to ensure only activated once.
			//
			_action_counter = 0;
			//
			//	The LPM instruction choices:
			//
			switch( _pm_mode ) {
				default: {
					//
					//	This was an invalid call to LPM
					//
					_report->raise( Warning_Level, Programmer_Module, Parameter_Invalid );
					break;
				}
			}
			return( 1 );
			
		}
			
		//
		//	Clock Tick API
		//	==============
		//
		//	Called once for every tick which the
		//	clock is simulating.
		//
		//	This gets called frequently and is used to
		//	simulate activities happening in parallel
		//	while other tasks are executing.
		//
		virtual void tick( void ) {
			//
			//	Reduce count down timers, take actions as appropriate.
			//
			if( _action_counter ) if(( _action_counter -= 1 ) == 0 ) _spmcsr &= ~control_mask;
			if( _parallel_counter ) {
				if(( _parallel_counter -= 1 ) == 0 ) {
					_flash->commit();
					_spmcsr &= ~bit_SPMEN;
					if( _int_enable ) _irq->raise( irq_number );
				}
			}
		}

		//
		//	Notification API
		//	================
		//
		//	Device register updates appear through here.
		//
		virtual byte read_register( word id ) {
			return( _spmcsr );
		}
		virtual void write_register( word id, byte value ) {
			update_spmcsr( value );
		}
};


#endif

//
//	EOF
//
