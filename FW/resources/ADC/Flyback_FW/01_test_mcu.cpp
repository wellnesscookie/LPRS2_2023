
#include "avr_io_bitfields.h"
#include <util/delay.h>

int main(void) {
	ddrb.b0 = 1; // MOSI
	ddrb.b1 = 1; // MISO_PWM
	ddrb.b4 = 0; // #EN
	uint8_t s = 0;
	while(1){ //infinite loop
		s = !s;
		portb.b0 = s;
		portb.b1 = s;
		//portb.b4 = s;
		_delay_ms(1000/2/100); // For 100Hz square.
	}
}
