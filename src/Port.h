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

		//
		//	Code to attach a pin to a place in a port.
		//
		virtual void attach( Pin *pin, byte bit ) = 0;
		
		//
		//	Return the address of the PIN object at a specified
		//	bit location.
		//
		virtual Pin *raw( byte bit ) = 0;

		//
		//	The device register interface.
		//
		virtual byte read_register( word id ) = 0;
		virtual void write_register( word id, byte value ) = 0;
		//
		//	Mechanism for examining content outside the
		//	framework of the simulation.
		//
		virtual bool examine( word id, Symbols *labels, char *buffer, int max ) = 0;
};

//
//	The actual, configurable, port device.
//
template< word instance > class PortDevice : public Port {
	private:
		//
		//	Where reports go...
		//
		Reporter	*_report;
		//
		//	Define out port details.
		//
		static const word port_pins = 8;
		Pin		*_pin[ port_pins ];
		
	public:
		//
		//	Constructor.
		//
		PortDevice( Reporter *report ) {
			_report = report;
			for( int i = 0; i < port_pins; _pin[ i++ ] = NULL );
		}

		//
		//	Code to attach a pin to a place in a port.
		//
		virtual void attach( Pin *pin, byte bit ) {
			ASSERT( pin != NULL );
			ASSERT( bit < port_pins );
			ASSERT( _pin[ bit ] == NULL );
			_report->raise( Information_Level, Port_Module, Config_Change, instance, "New pin attached", bit );
			_pin[ bit ] = pin;
		}
		
		//
		//	Return the address of the PIN object at a specified
		//	bit location.
		//
		virtual Pin *raw( byte bit ) {
			ASSERT( bit < port_pins );
			return( _pin[ bit ]);
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
							_report->raise( Warning_Level, Port_Module, Device_Missing, instance, "Incomplete pin", i );
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
							_report->raise( Warning_Level, Port_Module, Device_Missing, instance, "Incomplete pin", i );
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
							_report->raise( Warning_Level, Port_Module, Device_Missing, instance, "Incomplete pin", i );
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
							_report->raise( Warning_Level, Port_Module, Device_Missing, instance, "Incomplete pin", i );
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
							_report->raise( Warning_Level, Port_Module, Device_Missing, instance, "Incomplete pin", i );
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
							_report->raise( Warning_Level, Port_Module, Device_Missing, instance, "Incomplete pin", i );
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
					snprintf( buffer, max, "PIN%c=%s", ( 'A' + instance ), bit );
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
					snprintf( buffer, max, "DDR%c=%s", ( 'A' + instance ), bit );
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
					snprintf( buffer, max, "PORT%c=%s", ( 'A' + instance ), bit );
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
