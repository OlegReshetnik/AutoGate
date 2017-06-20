/*
Automatic Gate rontroller
Attiny13a, 9600000
*/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#define CONTROL_PULSE	100		// 1 milliseconds
#define VOR_OPEN_TIME	20*5	// gate open time, 20 sec
#define VOR_CLOSE_TIME	3*5		// gate close time, 3 sec


#define BUTTON_DELAY	5		// 10 milliseconds
#define SHORT_CNT		( 10 / BUTTON_DELAY ) // short time (ms)
#define LONG_CNT		( 500 / BUTTON_DELAY ) // long time (ms)
#define OFF_CNT			( 5000 / BUTTON_DELAY ) // off time (ms)
#define G1				1		// short press code
#define G2				2		// long press code


#define BUTTON_IN_PORT		PORTB
#define BUTTON_IN_PIN		PINB
#define BUTTON_IN_DDR		DDRB
#define BUTTON_IN			1

#define BUTTON_OUT_PORT		PORTB
#define BUTTON_OUT_PIN		PINB
#define BUTTON_OUT_DDR		DDRB
#define BUTTON_OUT			0

#define CONTROL_OPEN_PORT	PORTB
#define CONTROL_OPEN_DDR	DDRB
#define OPEN_SG				3

#define CONTROL_CLOSE_PORT	PORTB
#define CONTROL_CLOSE_DDR	DDRB
#define CLOSE_SG			4

#define OUT_PRESS ( ( BUTTON_OUT_PIN & ( 1 << BUTTON_OUT ) ) == 0 )
#define IN_PRESS  ( ( BUTTON_IN_PIN & ( 1 << BUTTON_IN ) ) == 0 )

const uint8_t pass[] PROGMEM = { G1, G1, G1, G2, G2, G2, G1, G1, G1, 0 }; // SOS, ( Tsss! Password :-)

void do_open( void )
{
	CONTROL_OPEN_PORT |= ( 1 << OPEN_SG ); // door open pulse
	_delay_ms( CONTROL_PULSE );
	CONTROL_OPEN_PORT &= ~( 1 << OPEN_SG );

	uint8_t del = VOR_OPEN_TIME;
	while( del -- ) _delay_ms( 200 ); // wait open

	CONTROL_CLOSE_PORT |= ( 1 << CLOSE_SG ); // door close pulse
	_delay_ms( CONTROL_PULSE );
	CONTROL_CLOSE_PORT &= ~( 1 << CLOSE_SG );

	del = VOR_CLOSE_TIME;
	while( del -- ) _delay_ms( 200 ); // wait to close

}

void io_init( void )
{
	CONTROL_OPEN_DDR |= 1 << OPEN_SG; // set control pins to output
	CONTROL_CLOSE_DDR |= 1 << CLOSE_SG;

	BUTTON_IN_DDR &= ~( 1 << BUTTON_IN ); // set buttons pins to input
	BUTTON_OUT_DDR &= ~( 1 << BUTTON_OUT );

	BUTTON_IN_PORT |= 1 << BUTTON_IN; // pull up input pins
	BUTTON_OUT_PORT |= 1 << BUTTON_OUT;
}

int main( void )
{
	uint8_t cnt = 0, i = 0;
	uint16_t off_cnt = 0;

	io_init();

	for(;;)
	{
		_delay_ms( BUTTON_DELAY );
		
		if( IN_PRESS ) // press in button, don't check password, just open&close
		{
			 do_open();
			 i = 0; cnt = 0; off_cnt = 0;
		}
		
		if( OUT_PRESS ) // press out button, check password
		{
			 cnt++;
			 off_cnt = 0;
		}
		else
		{
			if( ++off_cnt > OFF_CNT ) // idle
			{
				i = 0; cnt = 0; off_cnt = 0; // reset counters
			}
			else
			{
				if( cnt > LONG_CNT )
				{
					if( pgm_read_byte( &pass[ i ] ) == G2 ) i++;
					else i = 0;
				}
				else if( cnt > SHORT_CNT )
				{
					if( pgm_read_byte( &pass[ i ] ) == G1 ) i++;
					else i = 0;
				}
				if( ( i > 0 ) && ( pgm_read_byte( &pass[ i ] ) == 0 ) )
				{
					i = 0;
					do_open();
				}
				cnt = 0;
			}
		}
	}
}
