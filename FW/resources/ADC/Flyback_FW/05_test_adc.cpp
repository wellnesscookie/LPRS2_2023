
#include "avr_io_bitfields.h"
#include <util/delay.h>

// [mV]
#define V_INT 1100

#define mV_2_sample(v) ((v)*1024L/V_INT)
#if mV_2_sample(550) != 512 
#error "Wrong macro"
#endif

int main(void) {
	// Sampling on ADC3 aka PB3.
	
	set_bf(adc.g.didr, {
		.adc1d = 0,
		.adc3d = 1 // Disable digital buffer on analog input.
	});
	set_bf(adc.g.admux, {
		.mux = 0b11, // ADC3
		.adlar = 0,
		.refs = 1 // Using internal 1.1V reference.
	});
	set_bf(adc.g.adcsrb, {
		.adts = 0 // Free Running mode
	});
	set_bf(adc.g.adcsra, {
		.adps = 0b111, // 128 prescaler.
		.adie  = 0, // No IRQ.
		.adif  = 0, // Clear IRQ.
		.adate = 0, // Auto trigger disabled.
		.adsc = 1, // Warm up ADC, because first sampling is 25 CLKs.
		.aden = 1 // Enable ADC.
	});

	ddrb.b1 = 1; // MISO_PWM
	ddrb.b4 = 1; // #EN_OUT
	
	portb.b4 = 1; // Disable Flyback.

	ddrb.b0 = 1; // MOSI
	
	uint8_t s = 0;
	while(true){
		s = !s;
		portb.b0 = s;
		_delay_ms(1); // For 500Hz square.

			
		// Start new sample.
		adc.f.adsc = 1;
		
		// Waiting conversion to be done.
		while(adc.f.adsc){}
		
		// TODO This does not work.
		//u16 sample = ADCL | ((u16)ADCH << 8);
		//u16 sample = ((u16)adc.r.adcl) | (((u16)adc.r.adch) << 8);
		// This work.
		//u16 sample = ADCW;
		u16 sample = adc.r.adcw;
		
		portb.b1 = sample > mV_2_sample(550); // MOSI_PWM
		
	}

}
