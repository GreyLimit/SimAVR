//
//	Base.h
//	======
//
//	Provide a baseline set of definitions upon which
//	other modules rest.
//

#ifndef _BASE_H_
#define _BASE_H_

//
//	Bring in the external definitions.
//
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <stdint.h>
#include <stdlib.h>


//
//	Base Types
//	----------
//
//	The following types form the data types which the
//	remainder of the software uses.
//
//	All types are unsigned.
//
typedef uint8_t		byte;
typedef uint16_t	word;
typedef uint32_t	dword;

//
//	Define come core constant values used across the program.
//
static const char EOS = 0;

//
//	Provide some syntactic sugar to clarify coding.
//
//	This should be wrapped around arguments which are
//	defined but are unused.
//
#define UNUSED(x)	__attribute__((unused)) x

//
//	Return a value of a specified type with the
//	'n'th bit set (bit numbers start at 0)
//
#define BIT(t,n)	((t)1<<(n))
template< class T > static inline T bit( byte b ) {
	return(((T)1) << b );
}

//
//	Return a value of specified type with all
//	the bits below bit 'n' set.  A mask of 'n'
//	bits.
//
#define MASK(t,n)	(((t)1<<(n))-1)
template< class T > static inline T mask( byte b ) {
	return((((T)1) << b ) - 1 );
}

//
//	Return the range of numbers available using
//	only 'n' bits.
//
#define RANGE(t,n)	((t)1<<(n))
template< class T > static inline T range( byte b ) {
	return(((T)1) << b );
}

//
//	Extract a subset of bits as a value
//
//	t	type to use
//	v	input value (of type t)
//	l	least significant bit of value to extract (from 0)
//	m	mask value (assuming value at bit 0)
//
#define EXTRACT(t,v,l,m)	(((v)>>(l))&((t)(m)))
template< class T > static inline T extract( T v, T l, T m ) {
	return(( v >> l ) & m );
}

//
//	Define a huge number of binary constants to simplify the
//	handling and decoding of instructions.
//

//
//	2 Bit binary numbers
//
static const word b00	= 0;
static const word b01	= 1;
static const word b10	= 2;
static const word b11	= 3;

//
//	3 Bit binary numbers
//
static const word b000	= 0;
static const word b001	= 1;
static const word b010	= 2;
static const word b011	= 3;

static const word b100	= 4;
static const word b101	= 5;
static const word b110	= 6;
static const word b111	= 7;

//
//	4 Bit binary numbers
//
static const word b0000	= 0;
static const word b0001	= 1;
static const word b0010	= 2;
static const word b0011	= 3;

static const word b0100	= 4;
static const word b0101	= 5;
static const word b0110	= 6;
static const word b0111	= 7;

static const word b1000	= 8;
static const word b1001	= 9;
static const word b1010	= 10;
static const word b1011	= 11;

static const word b1100	= 12;
static const word b1101	= 13;
static const word b1110	= 14;
static const word b1111	= 15;

//
//	5 Bit binary numbers
//
static const word b00000	= 0;
static const word b00001	= 1;
static const word b00010	= 2;
static const word b00011	= 3;

static const word b00100	= 4;
static const word b00101	= 5;
static const word b00110	= 6;
static const word b00111	= 7;

static const word b01000	= 8;
static const word b01001	= 9;
static const word b01010	= 10;
static const word b01011	= 11;

static const word b01100	= 12;
static const word b01101	= 13;
static const word b01110	= 14;
static const word b01111	= 15;

static const word b10000	= 16;
static const word b10001	= 17;
static const word b10010	= 18;
static const word b10011	= 19;

static const word b10100	= 20;
static const word b10101	= 21;
static const word b10110	= 22;
static const word b10111	= 23;

static const word b11000	= 24;
static const word b11001	= 25;
static const word b11010	= 26;
static const word b11011	= 27;

static const word b11100	= 28;
static const word b11101	= 29;
static const word b11110	= 30;
static const word b11111	= 31;

//
//	6 Bit binary numbers
//
static const word b000000	= 0;
static const word b000001	= 1;
static const word b000010	= 2;
static const word b000011	= 3;
static const word b000100	= 4;
static const word b000101	= 5;
static const word b000110	= 6;
static const word b000111	= 7;

static const word b001000	= 8;
static const word b001001	= 9;
static const word b001010	= 10;
static const word b001011	= 11;
static const word b001100	= 12;
static const word b001101	= 13;
static const word b001110	= 14;
static const word b001111	= 15;

static const word b010000	= 16;
static const word b010001	= 17;
static const word b010010	= 18;
static const word b010011	= 19;
static const word b010100	= 20;
static const word b010101	= 21;
static const word b010110	= 22;
static const word b010111	= 23;

static const word b011000	= 24;
static const word b011001	= 25;
static const word b011010	= 26;
static const word b011011	= 27;
static const word b011100	= 28;
static const word b011101	= 29;
static const word b011110	= 30;
static const word b011111	= 31;

static const word b100000	= 32;
static const word b100001	= 33;
static const word b100010	= 34;
static const word b100011	= 35;
static const word b100100	= 36;
static const word b100101	= 37;
static const word b100110	= 38;
static const word b100111	= 39;

static const word b101000	= 40;
static const word b101001	= 41;
static const word b101010	= 42;
static const word b101011	= 43;
static const word b101100	= 44;
static const word b101101	= 45;
static const word b101110	= 46;
static const word b101111	= 47;

static const word b110000	= 48;
static const word b110001	= 49;
static const word b110010	= 50;
static const word b110011	= 51;
static const word b110100	= 51;
static const word b110101	= 53;
static const word b110110	= 54;
static const word b110111	= 55;

static const word b111000	= 56;
static const word b111001	= 57;
static const word b111010	= 58;
static const word b111011	= 59;
static const word b111100	= 60;
static const word b111101	= 61;
static const word b111110	= 62;
static const word b111111	= 63;

#endif

//
//	EOF
//
