//
//	Port.h
//	======
//
//	Define a concentration of pins into a port.
//

#ifndef _PORT_H_
#define _PORT_H_

#include "Base.h"
#include "Reporter.h"
#include "Pin.h"
#include "DeviceRegister.h"
#include "Validation.h"

//
//	The generic port API
//
class Port : public Notification {
	public:
		//
		//	Registers come in three flavours:
		//
		static const word PINn = 0;
		static const word DDRn = 1;
		static const word PORTn = 2;

	private:
		//
		//	Where reports go...
		//
		Reporter	*_report;
		int		_instance;
		//
		//	Define out port details.
		//
		static const word port_pins = 8;
		Pin		*_pin[ port_pins ];

		//
		//	Convert instance number to letter.
		//
		inline char name( int id ) {
			return( 'A' + id );
		}
		
	public:
		//
		//	Constructor.
		//
		Port( Reporter *report, int instance ) {
			_report = report;
			_instance = instance;
			for( int i = 0; i < port_pins; _pin[ i++ ] = NULL );
		}

		//
		//	Code to attach a pin to a place in a port.
		//
		void attach( Pin *pin, byte bit ) {
			ASSERT( pin != NULL );
			ASSERT( bit < port_pins );
			ASSERT( _pin[ bit ] == NULL );
			_report->report( Information_Level, Port_Module, _instance, Config_Change, "Port %c, new pin at bit %d", name( _instance ), (int)bit );
			_pin[ bit ] = pin;
		}
		
		//
		//	The device register API.
		//	========================
		//
		virtual byte read_register( word id ) {
			switch( id ) {
				case PINn: {
					byte	res, bit;

					res = 0;
					bit = 1;
					for( int i = 0; i < port_pins; i++ ) {
						if( _pin[ i ] != NULL ) {
							if( _pin[ i ]->get_PIN()) res |= bit;
						}
						else {
							_report->report( Warning_Level, Port_Module, _instance, Device_Missing, "PIN%c, reading missing bit %d", name( _instance ), i );
						}
						bit <<= 1;
					}
					return( res );
				}
				case DDRn: {
					byte	res, bit;

					res = 0;
					bit = 1;
					for( int i = 0; i < port_pins; i++ ) {
						if( _pin[ i ] != NULL ) {
							if( _pin[ i ]->get_DDR()) {
								res |= bit;
							}
						}
						else {
							_report->report( Warning_Level, Port_Module, _instance, Device_Missing, "DDR%c, reading missing bit %d", name( _instance ), i );
						}
						bit <<= 1;
					}
					return( res );
				}
				case PORTn: {
					byte	res, bit;

					res = 0;
					bit = 1;
					for( int i = 0; i < port_pins; i++ ) {
						if( _pin[ i ] != NULL ) {
							if( _pin[ i ]->get_PIN()) {
								res |= bit;
							}
						}
						else {
							_report->report( Warning_Level, Port_Module, _instance, Device_Missing, "PORT%c, reading missing bit %d", name( _instance ), i );
						}
						bit <<= 1;
					}
					return( res );
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
				case PINn: {
					byte	bit;

					bit = 1;
					for( int i = 0; i < port_pins; i++ ) {
						if( _pin[ i ] != NULL ) {
							_pin[ i ]->set_PIN( value & bit );
						}
						else {
							_report->report( Warning_Level, Port_Module, _instance, Device_Missing, "PIN%c, write missing bit %d", name( _instance ), i );
						}
						bit <<= 1;
					}
					break;
				}
				case DDRn: {
					byte	bit;

					bit = 1;
					for( int i = 0; i < port_pins; i++ ) {
						if( _pin[ i ] != NULL ) {
							_pin[ i ]->set_DDR( value & bit );
						}
						else {
							_report->report( Warning_Level, Port_Module, _instance, Device_Missing, "DDR%c, write missing bit %d", name( _instance ), i );
						}
						bit <<= 1;
					}
					break;
				}
				case PORTn: {
					byte	bit;

					bit = 1;
					for( int i = 0; i < port_pins; i++ ) {
						if( _pin[ i ] != NULL ) {
							_pin[ i ]->set_PORT( value & bit );
						}
						else {
							_report->report( Warning_Level, Port_Module, _instance, Device_Missing, "PORT%c, write missing bit %d", name( _instance ), i );
						}
						bit <<= 1;
					}
					break;
				}
				default: {
					ABORT();
					break;
				}
			}
		}
		
		//
		//	Mechanism for examining content outside the
		//	framework of the simulation.
		//
		virtual bool examine( word id, Symbols *labels, char *buffer, int max ) {
			char	bit[ port_pins +1 ];

			switch( id ) {
				case PINn: {
					for( int i = 0; i < port_pins; i++ ) {
						int j = ( port_pins - 1 ) - i;
						
						if( _pin[ i ] != NULL ) {
							bit[ j ] = _pin[ i ]->get_PIN()? '1': '0';
						}
						else {
							bit[ j ] = '_';
						}
					}
					bit[ port_pins ] = EOS;
					snprintf( buffer, max, "PIN%c=%%%s", name( _instance ), bit );
					return( true );
				}
				case DDRn: {
					for( int i = 0; i < port_pins; i++ ) {
						int j = ( port_pins - 1 ) - i;
						
						if( _pin[ i ] != NULL ) {
							bit[ j ] = _pin[ i ]->get_DDR()? '1': '0';
						}
						else {
							bit[ j ] = '_';
						}
					}
					bit[ port_pins ] = EOS;
					snprintf( buffer, max, "DDR%c=%%%s", name( _instance ), bit );
					return( true );
				}
				case PORTn: {
					for( int i = 0; i < port_pins; i++ ) {
						int j = ( port_pins - 1 ) - i;
						
						if( _pin[ i ] != NULL ) {
							bit[ j ] = _pin[ i ]->get_PORT()? '1': '0';
						}
						else {
							bit[ j ] = '_';
						}
					}
					bit[ port_pins ] = EOS;
					snprintf( buffer, max, "PORT%c=%%%s", name( _instance ), bit );
					return( true );
				}
				default: {
					ABORT();
					break;
				}
			}
			return( false );
		}
};

#endif

//
//	EOF
//
