//
//	SerialTerminal.h
//	================
//
//	Provide a mechanism to enable data to be captured from and sent
//	to a Serial UART (SerialComms) device.
//

#ifndef _SERIAL_TERMINAL_H_
#define _SERIAL_TERMINAL_H_

#include <ctype.h>

#include "Base.h"
#include "SerialIO.h"

template< int rows, int cols > class SerialTerminal : public SerialIO {
	private:
		//
		//	Define constants for the virtual terminal
		//
		static const int last_col = cols-1;
		static const int last_row = rows-1;

		//
		//	define some buffer sizes.
		//
		static const int out_buf = 80;
		static const int in_buf = 80;

		//
		//	The virtual terminal display
		//
		char	_display[ rows ][ cols ];
		int	_row, _col;
		
		//
		//	Where the input is buffered.
		//
		char	_out_buf[ out_buf ];	// data leaving terminal
		int	_pending;		// and amount left to send.

		//
		//	Where control codes are gathered.
		//
		char	_in_buf[ in_buf ];	// control data arriving.
		int	_waiting;		// and amount waiting to be processed.

		//
		//	Flag indicating if we are in escape mode.
		//
		bool	_escaped;

		//
		//	Flag indicating cursor is just off screen at the bottom
		//
		bool	_bottom;

		//
		//	Scroll routines.
		//
		void scroll_up( void ) {
			for( int r = 0; r < last_row; r++ ) {
				for( int c = 0; c < cols; c++ ) {
					_display[ r ][ c ] = _display[ r+1 ][ c ];
				}
			}
			for( int c = 0; c < cols; c++ ) {
				_display[ last_row ][ c ] = SPACE;
			}
		}

		//
		//	Cursor routines
		//
		void cursor_down( void ) {
			if( _bottom ) {
				scroll_up();
			}
			else {
				if( _row < last_row ) {
					_row++;
				}
				else {
					if( _col == 0 ) {
						_bottom = true;
					}
					else {
						scroll_up();
					}
				}
			}
		}
		void cursor_back( void ) {
			_col = 0;
		}
		void cursor_left( void ) {
			if( _bottom ) {
				_col = last_col;
				_bottom = false;
			}
			else {
				if( _col > 0 ) {
					_col--;
				}
				else {
					if( _row > 0 ) {
						_col = last_col;
						_row--;
					}
				}
			}
		}
		void cursor_right( void ) {
			if( _bottom ) {
				_col = 1;
				_bottom = false;
				scroll_up();
			}
			else {
				if( _col < last_col ) {
					_col++;
				}
				else {
					if( _row < last_row ) {
						_col = 0;
						_row++;
					}
					else {
						_bottom = true;
					}
				}
			}
		}

		//
		//	Handle the escape codes sent.
		//
		void process_escape( char *data, int len ) {
			//
			//	Not implemented yet.
			//
		}
		
	public:
		//
		//	Constructor
		//
		SerialTerminal( void ) {
			for( int r = 0; r < rows; r++ ) {
				for( int c = 0; c < cols; c++ ) {
					_display[ r ][ c ] = SPACE;
				}
			}
			_row = 0;
			_col = 0;
			_pending = 0;
			_waiting = 0;
			_escaped = false;
			_bottom = false;
		}

		//
		//	SerialIO API
		//	------------
		//
		//	The API to send and receive data.
		//
		virtual void write( byte c ) {
			switch( c ) {
				case DEL: {
					break;
				}
				case BS: {
					cursor_left();
					break;
				}
				case TAB: {
					if( _bottom ) {
						scroll_up();
						_col = 8;
					}
					else {
						cursor_right();
						while( !_bottom && (( _col % 8 ) != 0 )) cursor_right();
					}
					break;
				}
				case NL: {
					cursor_down();
					break;
				}
				case CR: {
					cursor_back();
					break;
				}
				case ESCAPE: {
					_waiting = 1;
					_in_buf[ 0 ] = ESCAPE;
					_escaped = true;
					break;
				}
				default: {
					if( _escaped ) {
						if( _waiting < in_buf ) _in_buf[ _waiting++ ] = c;
						if( isalpha( c )) {
							process_escape( _in_buf, _waiting );
							_waiting = 0;
							_escaped = false;
						}
					}
					else {
						if(( c >= SPACE )||( c < DEL )) {
							_display[ _row ][ _col++ ] = c;
							if( _col >= cols ) {
								_col = 0;
								if( _row < last_row ) {
									_row++;
								}
								else {
									scroll_up();
								}
							}
						}
					}
				}
			}
		}
		virtual bool read( byte *c ) {
			if( _pending ) {
				_pending--;
				*c = _out_buf[ 0 ];
				for( int i = 0; i < _pending; i++ ) {
					_out_buf[ i ] = _out_buf[ i+1 ];
				}
				return( true );
			}
			return( false );
		}
		virtual void display( FILE *to ) {
			//
			//	Output a 'drawing' of the terminal and
			//	the state of the IO buffers.
			//
			fprintf( to, "+" );
			for( int c = 0; c < cols; c++ ) fprintf( to, (( c ==_col )? "v": "-" ));
			fprintf( to, "+\n" );
			for( int r = 0; r < rows; r++ ) {
				fprintf( to, (( r == _row )? ">": "|" ));
				for( int c = 0; c < cols; c++ ) fprintf( to, "%c", _display[ r ][ c ]);
				fprintf( to, (( r == _row )? "<\n": "|\n" ));
			}
			fprintf( to, "+" );
			for( int c = 0; c < cols; c++ ) fprintf( to, (( c ==_col )? "^": "-" ));
			fprintf( to, "+\n" );
			if( _pending ) {
				fprintf( to, "sending:" );
				for( int i = 0; i < _pending; i++ ) {
					char c = _out_buf[ i ];
					
					if(( c <= SPACE )||( c >= DEL )) {
						fprintf( to, " $%02X", (int)c );
					}
					else {
						fprintf( to, " %c", c );
					}
				}
				fprintf( to, "\n" );
			}
			if( _waiting ) {
				fprintf( to, "escape:" );
				for( int i = 0; i < _waiting; i++ ) {
					char c = _in_buf[ i ];
					
					if(( c <= SPACE )||( c >= DEL )) {
						fprintf( to, " $%02X", (int)c );
					}
					else {
						fprintf( to, " %c", c );
					}
				}
				fprintf( to, "\n" );
			}
		}
		virtual void supply( char c ) {
			if( _pending < out_buf ) _out_buf[ _pending++ ] = c;
		}

};

#endif

//
//	EOF
//
