//
//	Flash.h
//
//	Define the API for access flash memory
//

#ifndef _FLASH_H_
#define _FLASH_H_

//
//	Base types.
//
#include "Base.h"

//	How the flash memory is presented.
//	==================================
//
//	As far as I have managed to determine, so far (and this could all
//	be wrong), the following statements capture how the flash memory
//	operates.
//
//	Flash memory ..
//
//		.. is divided into pages which are sub-divided into words
//		of 16-bits per word.
//
//		.. page size is page_size() words long.
//
//		.. has total_pages() number of pages available.
//
//		.. is therefore word addressed from word 0 to word number
//		( total_pages() * page_size()) - 1.
//
//	The flash API allows reading of words from any word address as
//	required.
//
//	However, to write data to the flash a page buffer is used into which
//	new page content is placed before a specified page is over written.
//
//	Notes on writing data to flash:
//
//		.. writing a page of flash takes time, the write operating
//		returns the number of microseconds it will take.
//
//		.. The act of writing a page performs essentially an AND
//		operation between the page buffer and the old data in the
//		page.  To write a clean new page erase the page first.
//
//		.. This API uses words to return timings and so limits time
//		to just 65ms maximum duration.
//
//	Also, and (possibly) unique to the AVR flash memory is the division
//	of the flash into RWW and NRWW sections.  See the following URL for
//	Microships' own definition of what this means.
//
//		https://microchipsupport.force.com/s/article/RWW-and-NRWW-in-FLASH-Memory
//

//
//	Flash Memory API
//
class Flash {
	public:
		//
		//	Flash Content
		//	-------------
		//
		//	Reading flash can be done word-at-a-time style using
		//	linear addressing (on an address per word basis).
		//
		//
		virtual word read( dword adrs ) = 0;
	
		//
		//	Flash Buffer Page
		//	-----------------
		//
		//	Writing to flash requires a temporary page buffer
		//	be cleared, then filled, then written to permanent
		//	flash storage.
		//
		//	This is all done using a "Page" as the basic unit
		//	written to permanent flash storage.
		//
		//	Clear the temp buffer, all bits will be set.
		//
		virtual void clear( void ) = 0;
		//
		//	Place a value into the temp buffer, return microseconds
		//	required.
		//
		virtual word place( word adrs, word value ) = 0;

		//
		//	Flash Modification
		//	------------------
		//
		//	Erasing or Writing a page will "lock" the section of
		//	flash being written to.  The two sections pertinent to
		//	this comment are those pages at the start of flash,
		//	the RWW section, and those above this, the NRWW
		//	section.
		//
		//	Simplistically, and not entirely accurately, you can read
		//	from one while modifying to the other.
		//
		//	Erase a page of flash memory, return the
		//	microseconds this will require to complete.
		//
		virtual word erase( word page ) = 0;
		//
		//	Write the temp buffer into the Flash memory, return the
		//	microseconds this will require to complete.
		//
		virtual word write( word page ) = 0;
		//
		//	Commit a previously initiated page erase or page
		//	write action.
		//
		//	The combination of calling erase()/write(), then
		//	waiting the specified period of time, then calling
		//	commit() allows a reasonably authentic simulation
		//	of how flash memory operates.  This is to say that
		//	inadvertently initiating another similar action before
		//	commit() is called will drop the earlier action.
		//
		virtual void commit( void ) = 0;
		
		//
		//	Read-While-Write control
		//	------------------------
		//
		//	Writing to any section of the flash memory will
		//	automatically lock this section from reading.  To
		//	re-enable this read access this routine needs to
		//	be called.
		//
		virtual void enable( void ) = 0;

		//
		//	Flash Characteristics
		//	---------------------
		//
		//	Return the total number of pages of flash memory,
		//	the number of pages (from zero) which form the
		//	application section and finally the size of
		//	a single page in words.
		//
		virtual word total_pages( void ) = 0;
		virtual word application_pages( void ) = 0;
		virtual word page_size( void ) = 0;
		
		//
		//	Supporting routines
		//	-------------------
		//
		//	Routines to simplify the code around using this API.
		//
		//	page_number	Return, right shifted and masked, the
		//			flash page number corresponding to the
		//			flash (word) address provided.
		//
		//	word_number	Return, masked off, the word number within
		//			a page that the supplied address points
		//			to.
		//
		virtual word page_number( dword adrs ) = 0;
		virtual word word_number( dword adrs ) = 0;

		//
		//	Interface to load a hex file into flash.
		//
		virtual bool load_hex( const char *filename ) = 0;
};

#endif

//
//	EOF
//
