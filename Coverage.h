//
//	Coverage.h
//	==========
//
//	A system for tracking where, and top some extent, how
//	code is executing.
//

#ifndef _COVERAGE_H_
#define _COVERAGE_H_

#include "Base.h"

//
//	The following definition captures the different
//	sorts of coverage that will be noted.
//
typedef enum  {
	//
	//	Code based access types.
	//
	Execute_Access	= 0,
	Jump_Access	= 1,
	Call_Access	= 2,
	Data_Access	= 3,
	//
	//	Data based access types.
	//
	Read_Access	= 4,
	Write_Access	= 5,
	Stack_Access	= 6
} AccessType;

//
//
class Coverage {
	public:
	private:
		//
		//	Access modes; the number of above types of access.
		//
		static const int access_modes = 7;
		
		//
		//	Define some key elements of the package that will
		//	facilitate its implementation.
		//

		//
		//	A tiered access mechanism will be used so that we
		//	find the appropriate page reasonably quickly.
		//
		//	This will be a 3 tier system where the address is
		//	broken into 'top' and 'middle' tiers before the
		//	page level being the 'bottom' tier.
		//

		//
		//	Bottom Tier
		//	===========
		//
		//	The page record itself.
		//
		//	Define number of bits used to access addresses
		//	inside a page and consequential size:
		//
		static const word page_adrs_bits = 8;
		static const word page_size = RANGE( word, page_adrs_bits );
		static const word page_mask = MASK( word, page_adrs_bits );

		//
		//	Middle Tier
		//	===========
		//
		//	Define the number of bits to be used in the
		//	second, consolidation, tier.
		//
		static const word cons_adrs_bits = 8;
		static const word cons_size = RANGE( word, cons_adrs_bits );
		static const word cons_mask = MASK( word, cons_adrs_bits );

		//
		//	Top Tier
		//	========
		//
		//	This is the data structure at the top of the system.
		//	this can be chained togther in the event that ranges
		//	exceed a single address block.
		//
		static const word block_adrs_bits = ( 24 - cons_adrs_bits ) - page_adrs_bits;
		static const word block_size = RANGE( word, block_adrs_bits );
		static const word block_mask = MASK( word, block_adrs_bits );

		//
		//	Declare the data structures needed to support the above
		//	view of the world.
		//
		struct access_record {
			dword	count[ access_modes ];
		};
		struct page_record {
			access_record	address[ page_size ];
		};
		struct cons_record {
			page_record	*page[ cons_size ];
		};
		struct block_record {
			word		number;
			cons_record	*cons[ block_size ];
			block_record	*next;
		};

		//
		//	All of the above hangs of this pointer.
		//
		block_record		*_data;

		//
		//	We will report through here.
		//
		Reporter		*_report;
		int			_instance;

		//
		//	Define a routine to convert a raw address
		//	into a set of tier specific values.
		//
		//	Returns any bits (suitably shifted) that
		//	fall outside the basic three tiers.
		//
		word separate( dword adrs, word *b_adrs, word *c_adrs, word *p_adrs ) {
			*p_adrs = adrs & page_mask;
			adrs >>= page_adrs_bits;
			*c_adrs = adrs & cons_mask;
			adrs >>= cons_adrs_bits;
			*b_adrs = adrs & block_mask;
			return( adrs >> block_adrs_bits );
		}

		//
		//	recombine a set of block, cons, pages and addresses
		//	to get the orignal value..
		//
		dword combine( word b_num, word c_num, word p_num, word adrs ) {
			return(		((	((	((dword)b_num << block_adrs_bits )
						| (dword)c_num ) << cons_adrs_bits )
					| (dword)p_num ) << page_adrs_bits )
				| (dword)adrs );
		}

	public:
		//
		//	Start empty..
		//
		Coverage( Reporter *report, int instance ) {
			_data = NULL;
			_report = report;
			_instance = instance;
		}

		//
		//	Touch an address in a specified way..
		//
		void touch( dword adrs, AccessType how ) {
			word	b_num, b_adrs, c_adrs, p_adrs;

			b_num = separate( adrs, &b_adrs, &c_adrs, &p_adrs );
			
			block_record	**abp, *bp;

			//
			//	Find the right block record.
			//
			abp = &_data;
			while(( bp = *abp )) {
				if( b_num <= bp->number ) break;
				abp = &( bp->next );
			}
			if(( bp == NULL )||( b_num < bp->number )) {
				bp = new block_record;
				bp->number = b_num;
				for( word i = 0; i < block_size; bp->cons[ i++ ] = NULL );
				bp->next = *abp;
				*abp = bp;
			}

			cons_record	*cp;
			
			//
			//	Verify require cons level exists
			//
			if(!( cp = bp->cons[ b_adrs ])) {
				cp = new cons_record;
				for( word i = 0; i < cons_size; cp->page[ i++ ] = NULL );
				bp->cons[ b_adrs ] = cp;
			}

			page_record	*pp;

			//
			//	Verify required page level exists
			//
			if(!( pp = cp->page[ c_adrs ])) {
				pp = new page_record;
				for( word i = 0; i < page_size; i++ ) 
					for( word j = 0; j < access_modes; pp->address[ i ].count[ j++ ] = 0 );
				cp->page[ c_adrs ] = pp;
			}

			//
			//	Finally, pp is the page we are looking for.
			//
			pp->address[ p_adrs ].count[ how ] += 1;
		}

		//
		//	Clear the cached coverage data
		//
		void clear( void ) {
			block_record *bp;
			for( bp = _data; bp != NULL; bp = bp->next ) {
				for( int b = 0; b < block_size; b++ ) {
					cons_record *cp;
					
					if(( cp = bp->cons[ b ]) != NULL ) {
						for( int c = 0; c < cons_size; c++ ) {
							page_record *pp;

							if(( pp = cp->page[ c ]) != NULL ) {
								for( int a = 0; a < page_size; a++ ) {
									access_record *ap = &( pp->address[ a ]);

									for( int m = 0; m < access_modes; m++ ) {
										ap->count[ m ] = 0;
									}
								}
							}
						}
					}
				}

			}
		}

		//
		//	Dump coverage stats (so far)
		//
		void dump( FILE *to, int *select, int selected ) {
			ASSERT( to != NULL );
			ASSERT( select != NULL );
			ASSERT( selected > 0 );
			
			block_record *br;
			printf( "Target" );
			for( int i = 0; i < selected; i++ ) {
				switch( select[ i ]) {
					case Execute_Access: {
						printf( "\tExec" );
						break;
					}
					case Jump_Access: {
						printf( "\tJump" );
						break;
					}
					case Call_Access:{
						printf( "\tCall" );
						break;
					}
					case Data_Access:{
						printf( "\tData" );
						break;
					}
					case Read_Access:{
						printf( "\tRead" );
						break;
					}
					case Write_Access:{
						printf( "\tWrite" );
						break;
					}
					case Stack_Access:{
						printf( "\tStack" );
						break;
					}
					default: {
						ABORT();
						break;
					}
				}
			}
			printf( "\n" );
			for( br = _data; br; br = br->next ) {
				for( word bi = 0; bi < block_size; bi++ ) {
					cons_record *cr;
					if(( cr = br->cons[ bi ])) {
						for( word ci = 0; ci < cons_size; ci++ ) {
							page_record *pr;
							if(( pr = cr->page[ ci ])) {
								for( word pi = 0; pi < page_size; pi++ ) {
									access_record *ar = &( pr->address[ pi ]);
									for( int i = 0; i < selected; i++ ) {
										if( ar->count[ select[ i ]]) {
											fprintf( to, "%06X", (int)combine( br->number, bi, ci, pi ));
											for( int j = 0; j < selected; printf( "\t%d", (int)ar->count[ select[ j++ ]]));
											printf( "\n" );
											break;
										}
									}
								}
							}
						}
					}
				}
			}
		}
};

#endif

//
//	EOF
//
