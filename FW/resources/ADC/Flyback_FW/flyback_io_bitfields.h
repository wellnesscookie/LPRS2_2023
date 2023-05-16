
#ifndef FLYBACK_IO_BITFIELDS_H
#define FLYBACK_IO_BITFIELDS_H

///////////////////////////////////////////////////////////////////////////////

#include "avr_io_bitfields.h"

///////////////////////////////////////////////////////////////////////////////


typedef struct {
	unsigned mosi     : 1;
	unsigned miso_pwm : 1;
	unsigned sck      : 1;
	unsigned adc_in   : 1;
	unsigned n_en_out : 1;
	unsigned n_rst    : 1;
} bf_gpio;

#define ddr (*((volatile bf_gpio*)(&DDRB)))
#define port (*((volatile bf_gpio*)(&PORTB)))
#define pin (*((volatile bf_gpio*)(&PINB)))

///////////////////////////////////////////////////////////////////////////////

#if __AVR_ATtiny13A__

#endif

///////////////////////////////////////////////////////////////////////////////

#endif // FLYBACK_IO_BITFIELDS_H
