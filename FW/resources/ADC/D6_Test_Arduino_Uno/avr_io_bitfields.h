
#ifndef AVR_IO_BITFIELDS_H
#define AVR_IO_BITFIELDS_H

///////////////////////////////////////////////////////////////////////////////

#include <avr/io.h>

///////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus

namespace _priv_bf_to_u8 {
	template<typename T>
	union bf_to_u8_un {
		T bf;
		uint8_t a;
	};
};
template<typename T>
uint8_t bf_to_u8(const T&& bf) {
	return _priv_bf_to_u8::bf_to_u8_un<T>{ .bf = bf }.a;
}

#endif

///////////////////////////////////////////////////////////////////////////////


typedef struct {
	unsigned b0 : 1;
	unsigned b1 : 1;
	unsigned b2 : 1;
	unsigned b3 : 1;
	unsigned b4 : 1;
	unsigned b5 : 1;
	unsigned b6 : 1;
	unsigned b7 : 1;
} bf_8b;

// Data Direction
#define ddra (*((volatile bf_8b*)(&DDRA)))
#define ddrb (*((volatile bf_8b*)(&DDRB)))
#define ddrc (*((volatile bf_8b*)(&DDRC)))
#define ddrd (*((volatile bf_8b*)(&DDRD)))

// Output Port
#define porta (*((volatile bf_8b*)(&PORTA)))
#define portb (*((volatile bf_8b*)(&PORTB)))
#define portc (*((volatile bf_8b*)(&PORTC)))
#define portd (*((volatile bf_8b*)(&PORTD)))

// Input Port
#define pina (*((volatile bf_8b*)(&PINA)))
#define pinb (*((volatile bf_8b*)(&PINB)))
#define pinc (*((volatile bf_8b*)(&PINC)))
#define pind (*((volatile bf_8b*)(&PIND)))

///////////////////////////////////////////////////////////////////////////////

//TODO Some macros for auto gen of structs,
// and also defining comment and default value.

typedef struct {
	unsigned mux : 4;
	unsigned _res : 1;
	unsigned adlar : 1;
	unsigned refs : 2;
} bf_admux;
#define admux (*((volatile bf_admux*)(&ADMUX)))

typedef struct {
	unsigned adps : 3;
	unsigned adie : 1;
	unsigned adif : 1;
	unsigned adate : 1;
	unsigned adsc : 1;
	unsigned aden :1;
} bf_adcsra;
#define adcsra (*((volatile bf_adcsra*)(&ADCSRA)))

typedef struct {
	unsigned adts : 3;
	unsigned _res0 : 3;
	unsigned acme : 1;
	unsigned _res1 : 1;
} bf_adcsrb;
#define adcsrb (*((volatile bf_adcsrb*)(&ADCSRB)))

typedef struct {
	unsigned adcxd : 5;
	unsigned _res : 2;
} bf_didr0;
#define didr0 (*((volatile bf_didr0*)(&DIDR0)))

///////////////////////////////////////////////////////////////////////////////

#endif // AVR_IO_BITFIELDS_H
