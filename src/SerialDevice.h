//
//	SerialDevice.h
//	=============
//
//	This is the simulation of the Serial UART
//

#ifndef _SERIAL_DEVICE_H_
#define _SERIAL_DEVICE_H_

//
//	What we need..
//
#include "Base.h"
#include "Reporter.h"
#include "Clock.h"
#include "DeviceRegister.h"
#include "SerialIO.h"

//
//	The device registers as follows:
//
//	UDR	Read and the Receive Buffer, written as
//		the Transmit Buffer.  Not the same register.
//
//	UCSRnA	USART Status and Control Register A
//
//		bit	Name	Meaning
//		7	RXC	USART Receive Complete
//		6	TXC	USART Transmit Complete
//		5	UDRE	USART Data Register Empty
//		4	FE	Frame Error
//		3	DOR	Data OverRun
//		2	UPE	USART Parity Error
//		1	U2X	Double the USART Transmission Speed
//		0	MPCM	Multi-processor Communication Mode
//
//	UCSRnB	USART Status and Control Register B
//
//		bit	Name	Meaning
//		7	RXCIE	RX Complete Interrupt Enable
//		6	TXCIE	TX Complete Interrupt Enable
//		5	UDRIE	USART Data Register Empty Interrupt Enable
//		4	RXEN	Receiver Enable
//		3	TXEN	Transmitter Enable
//		2	UCSZ	Character Size (third bit from UCSRnC UCSZ)
//		1	RXB8	Receive Data Bit 8
//		0	TXB8	Transmit Data Bit 8
//
//	UCSRnC	USART Status and Control Register C
//
//		bit	Name	Meaning
//		7-6	UMSEL	USART Mode Select
//
//			00	Asynchronous USART
//			01	Synchronous USART
//			10	Reserved
//			11	Master SPI (MSPIM)
//
//		5-4	UPM	Parity Mode
//
//			00	Disabled
//			01	Reserved
//			10	Even Parity
//			11	Odd Parity
//
//		3	USBS	Stop Bit Select
//
//			0	1 stop bit.
//			1	2 stop bits.
//
//		2-1	UCSZ	Character Size (third bit is UCSRB)
//
//			000	5
//			001	6
//			010	7
//			011	8
//			100	Reserved
//			101	Reserved
//			110	Reserved
//			111	9
//	
//		0	UCPOL	Clock Polarity
//
//				Trans		Recv
//			0	Rising		Falling
//			1	Falling		Rising
//
//	UBRRH/UBRRL Baud Rate Register
//
//		15-12	Reserved
//		11-0	UBRR	0-4095
//
//

//
//	Declare a Generic UART class.
//
//	generic UART declarations go in here.
//
class SerialDevice : public Notification, Tick {
	public:
		//
		//	Register handles
		//
		static const word UDRn = 0;
		static const word UCSRnA = 1;
		static const word UCSRnB = 2;
		static const word UCSRnC = 3;
		static const word UBRRnL = 4;
		static const word UBRRnH = 5;

		//
		//	UCSRnA bit field definitions
		//
		static const byte ucsra_RXC	= BIT( byte, 7 );
		static const byte ucsra_TXC	= BIT( byte, 6 );
		static const byte ucsra_UDRE	= BIT( byte, 5 );
		static const byte ucsra_FE	= BIT( byte, 4 );
		static const byte ucsra_DOR	= BIT( byte, 3 );
		static const byte ucsra_UPE	= BIT( byte, 2 );
		static const byte ucsra_U2X	= BIT( byte, 1 );
		static const byte ucsra_MPCM	= BIT( byte, 0 );
		//
		static const byte ucsra_mask	= ucsra_TXC | ucsra_U2X | ucsra_MPCM;

		//
		//	UCSRnB bit field definitions
		//
		static const byte ucsrb_RXCIE	= BIT( byte, 7 );
		static const byte ucsrb_TXCIE	= BIT( byte, 6 );
		static const byte ucsrb_UDRIE	= BIT( byte, 5 );
		static const byte ucsrb_RXEN	= BIT( byte, 4 );
		static const byte ucsrb_TXEN	= BIT( byte, 3 );
		static const byte ucsrb_UCSZ	= BIT( byte, 2 );
		static const byte ucsrb_RXB8	= BIT( byte, 1 );
		static const byte ucsrb_TXB8	= BIT( byte, 0 );

		//
		//	UCSRnC bit field definitions
		//
		static const byte ucsrc_UMSEL_lsb	= 6;
		static const byte ucsrc_UMSEL_mask	= MASK( byte, 2 );
		static const byte ucsrc_UPM_lsb		= 4;
		static const byte ucsrc_UPM_mask	= MASK( byte, 2 );
		static const byte ucsrc_USBS		= BIT( byte, 3 );
		static const byte ucsrc_UCSZ_lsb	= 1;
		static const byte ucsrc_UCSZ_mask	= MASK( byte, 2 );
		static const byte ucsrc_UCPOL		= BIT( byte, 0 );

		//
		//	UBRR specifics
		//
		static const word ubrr_mask		= MASK( word, 12 );

		//
		//	Clock handle
		//
		static const word System_Clock = 0;

		//
		//	The Device Registers API
		//
		virtual byte read_register( word id ) = 0;
		virtual void write_register( word id, byte value ) = 0;
		virtual bool examine( word id, Symbols *labels, char *buffer, int max ) = 0;

		//
		//	The Clock ticking API
		//
		virtual void tick( word handle, bool inst_end ) = 0;

};


//
//	Declare a specific UART hardware device
//
template< byte rx, byte dre, byte tx > class SerialDriver : public SerialDevice {
	private:
		Interrupts	*_interrupt;
		Reporter	*_report;
		int		_instance;
		SerialIO	*_target;

		//
		//	Internal state variables
		//
		byte		_recv_buffer,
				_trans_buffer,
				_ucsra,
				_ucsrb,
				_ucsrc;
		word		_ubrr;

		//
		//	defined data/character size.
		//
		byte		_char_bits,
				_stop_bits;

		//
		//	INPUT variables
		//
		dword		_input_clock_count;

		//
		//	OUTPUT variables
		//
		dword		_output_clock_count;

		//
		//	This is the completely factored clock
		//	counting target.  This is not restricted
		//	to the UART 12 bit counter range and includes
		//	any clock doubling required.
		//
		//	As a result a 16 word cannot hold the maximum
		//	possible range required (4095 * 2 * 10).
		//
		dword		_clock_target;

		//
		//	simplified routines to various parameters
		//
		void reset_clock_target( void ) {
			//
			//	Set the real ticks per bit sent/received.
			//
			if( _ucsra & ucsra_U2X ) {
				_clock_target = _ubrr;
			}
			else {
				_clock_target = _ubrr << 1;
			}
			//
			//	Now multiply by the actual number of bits
			//	to get clock counts per character of data.
			//
			_clock_target *= _char_bits + _stop_bits;
		}
		void reset_stop_bits( void ) {
			byte	s;
			
			//
			//	Just set it directly
			//
			s = ( _ucsrc & SerialDevice::ucsrc_USBS )? 2: 1;
			if( _stop_bits != s ) {
				_stop_bits = s;
				_report->report( Information_Level, Serial_Module, _instance, Config_Change, "USBS%d = %d stop bits", _instance, (int)s );
			}
		}
		void reset_char_bits( void ) {
			byte s;

			s = extract<byte>( _ucsrc, SerialDevice::ucsrc_UCSZ_lsb, SerialDevice::ucsrc_UCSZ_mask );
			if( _ucsrb & SerialDevice::ucsrb_UCSZ ) {
				if( s != 3 ) {
					_report->report( Warning_Level, Serial_Module, _instance, Parameter_Invalid, "UCSZ%d = %d reserved", _instance, (int)( s+4 ));
					return;
				}
				s = 9;
			}
			else {
				s += 5;
			}
			if( _char_bits != s ) {
				_char_bits = s;
				_report->report( Information_Level, Serial_Module, _instance, Config_Change, "UCSZ%d = %d char bits", _instance, (int)s );
			}
		}
		
	public:
		SerialDriver( Reporter *report, int instance, Interrupts *interrupt, SerialIO *target ) {
			_report = report;
			_instance = instance;
			_interrupt = interrupt;
			_target = target;

			_recv_buffer = 0;
			_trans_buffer = 0;
			_ucsra = SerialDevice::ucsra_UDRE;	// transmit buffer is empty
			_ucsrb = 0;
			_ucsrc = 0;
			_ubrr = 0;

			_char_bits = 5;
			_stop_bits = 1;

			_input_clock_count = 0;
			_output_clock_count = 0;
			
			_clock_target = 0;
			_stop_bits = 0;
			_char_bits = 0;

			reset_clock_target();
			reset_stop_bits();
			reset_char_bits();
		}
		//
		//	The Device Registers API
		//
		virtual byte read_register( word id ) {
			switch( id ) {
				case SerialDevice::UDRn: {
					return( _recv_buffer );
				}
				case SerialDevice::UCSRnA: {
					return( _ucsra );
				}
				case SerialDevice::UCSRnB: {
					return( _ucsrb );
				}
				case SerialDevice::UCSRnC: {
					return( _ucsrc );
				}
				case SerialDevice::UBRRnL: {
					return( low_byte( _ubrr ));
				}
				case SerialDevice::UBRRnH: {
					return( high_byte( _ubrr ));
				}
				default: {
					ABORT();
					break;
				}
			}
			return( 0 );
		}
		virtual void write_register( word id, byte value ) {
			switch( id ) {
				case SerialDevice::UDRn: {
					if( _ucsra & SerialDevice::ucsra_UDRE ) {
						_trans_buffer = value;
						_ucsra &= ~SerialDevice::ucsra_UDRE;
					}
					else {
						_report->report( Warning_Level, Serial_Module, _instance, Write_Invalid, "UDR%d(TXB) busy (data %d dropped)", _instance, (int)value );
					}
					break;
				}
				case SerialDevice::UCSRnA: {
					if( value & ~SerialDevice::ucsra_mask ) _report->report( Warning_Level, Serial_Module, _instance, Parameter_Invalid, "UCSR%dA = $%02X", _instance, (int)value );
					value &= SerialDevice::ucsra_mask;
					if( value & SerialDevice::ucsra_TXC ) {
						if( _ucsra & SerialDevice::ucsra_TXC ) {
							_report->report( Information_Level, Serial_Module, _instance, Config_Change, "UCSR%dA TXC cleared", _instance );
							_ucsra &= ~SerialDevice::ucsra_TXC;
						}
					}
					if(( value & SerialDevice::ucsra_U2X ) != ( _ucsra & SerialDevice::ucsra_U2X )) {
						_ucsra = ( _ucsra & ~SerialDevice::ucsra_U2X )|( value & SerialDevice::ucsra_U2X );
						_report->report( Information_Level, Serial_Module, _instance, Config_Change, "U2X%d = %s", _instance, (( _ucsra & SerialDevice::ucsra_U2X )?"On" : "Off" ));
					}
					if(( value & SerialDevice::ucsra_MPCM ) != ( _ucsra & SerialDevice::ucsra_MPCM )) {
						_ucsra = ( _ucsra & ~SerialDevice::ucsra_MPCM )|( value & SerialDevice::ucsra_MPCM );
						_report->report( Information_Level, Serial_Module, _instance, Config_Change, "MPCM%d = %s", _instance, (( _ucsra & SerialDevice::ucsra_MPCM )?"On" : "Off" ));
					}
					break;
				}
				case SerialDevice::UCSRnB: {
					reset_char_bits();
					break;
				}
				case SerialDevice::UCSRnC: {
					reset_char_bits();
					reset_stop_bits();
					break;
				}
				case SerialDevice::UBRRnL: {
					word	v;

					v = combine( high_byte( _ubrr ), value );
					if( _ubrr != v ) {
						_report->report( Information_Level, Serial_Module, _instance, Config_Change, "UBRR%d(L) = %d (from %d)", _instance, (int)v, (int)_ubrr );
						_ubrr = v;
						//
						//	The clock target is only updated on the LOW byte
						//	being modified.
						//
						reset_clock_target();
					}
					break;
				}
				case SerialDevice::UBRRnH: {
					word	v;

					v = combine( value, low_byte( _ubrr ));
					if( v & ~SerialDevice::ubrr_mask ) _report->report( Warning_Level, Serial_Module, _instance, Parameter_Invalid, "UBRR%dH = $%02X", _instance, (int)value );
					v &= SerialDevice::ubrr_mask;
					if( v != _ubrr ) {
						_report->report( Information_Level, Serial_Module, _instance, Config_Change, "UBRR%d(H) = %d (from %d)", _instance, (int)v, (int)_ubrr );
						_ubrr = v;
					}
					break;
				}
				default: {
					ABORT();
					break;
				}
			}
		}
		virtual bool examine( word id, Symbols *labels, char *buffer, int max ) {
			switch( id ) {
				case SerialDevice::UDRn: {
					break;
				}
				case SerialDevice::UCSRnA: {
					break;
				}
				case SerialDevice::UCSRnB: {
					break;
				}
				case SerialDevice::UCSRnC: {
					break;
				}
				case SerialDevice::UBRRnL:
				case SerialDevice::UBRRnH: {
					break;
				}
				default: {
					ABORT();
					break;
				}
			}
			return( false );
		}
		//
		//	The Clock ticking API
		//
		virtual void tick( word handle, bool inst_end ) {
			switch( handle ) {
				case System_Clock: {
					if( _ucsrb & SerialDevice::ucsrb_RXEN ) {
						if( _input_clock_count ) {
							_input_clock_count--;
						}
					}
					if( _ucsrb & SerialDevice::ucsrb_TXEN ) {
						if( _output_clock_count ) {
							_output_clock_count--;
							_target->write( _trans_buffer );
							_ucsra |= SerialDevice::ucsra_UDRE;
							if( _ucsrb & SerialDevice::ucsrb_UDRIE ) _interrupt->raise( dre, &_ucsra, SerialDevice::ucsra_UDRE );
						}
					}
					break;
				}
				default: {
					ABORT();
					break;
				}
			}
		}
};

#endif

//
//	EOF
//
