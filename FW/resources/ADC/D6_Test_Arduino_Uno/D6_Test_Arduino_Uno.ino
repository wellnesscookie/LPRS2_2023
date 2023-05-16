// - Board: Arduino Uno
// - Shield: proto-board

#include "avr_io_bitfields.h"

// URL: https://github.com/PaulStoffregen/TimerOne
// To install it do Ctrl+Shift+I to open Library Manager,
// search "TimerOne" and click "Install" button.
#include <TimerOne.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif

/*
	- Loaded with 2k8
		- When continuosly driving (pulse on every T):
			-O BC640 and 2SA512 will burn
			-O With BC640 and BD140 off time is few us.
			-O With TO220 MOSFET: off is 1V, on is between 2V and 2.7V.
		- When occasional driving (pulse on every 24T):
			-O With TO220 MOSFET: off is 0V, on is between 1V and 9V.
			-O With BC640: off is 0V, on is between 1V and 9V.
	- Loaded with 220+470=690 Ohm
		- When occasional driving (pulse on every 24T):
			-O With BC640: off is 0V, on is between 0.5->1V and 7->6V.
				-O After divider: on is between 0.2V and 2V
					-C Lower bound will be saturted.
*/


// TDxy pulse should be:
// phase = 2 and 3
// t_w = 10.5us
// T = 21us
// Sampling should start after pulse?
#define T_PHASE_US 5

#define TEST 0

// -C Do not need it. Will do it with decoders.
#define TRISTATE_TD_EMULATE 0


// Pin positions.
#define EXT_MUX_A 2
#define EXT_MUX_B 3
#define TDxy1 4

#define EXT_MUX_X 0 // [h0-h3]
#define EXT_MUX_Y 1 // [h4-h7]
#define MUX_H8 2
#define MUX_H9 3

#define SWITCH_A 5

#define ND 7

const u8 enc_to_coil_to_pin[4][6] = {
	{TDxy1, ND, ND, ND, ND, ND},
	{ND, ND, ND, ND, ND, ND},
	{ND, ND, ND, ND, ND, ND},
	{ND, ND, ND, ND, ND, ND},
};

//TODO Remove.
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))


static void print_reg(const char* name, u8 r) {
	Serial.print(name);
	Serial.print(" = 0b");
	for(int i = 7; i >= 0; i--){
		if((r >> i) & 1){
			Serial.print('1');
		}else{
			Serial.print('0');
		}
		if(i == 4){
			Serial.print('_');
		}
	}
	Serial.println();
}

static void set_mcu_mux(u8 mux) {
	ADMUX = bf_to_u8<bf_admux>({
		.mux = mux,
		._res = 0,
		.adlar = 0, //TODO Use just 8b. // Left align ADC value to 8 bits from ADCH register.
		.refs = 0b01 // Vcc
	});
}

static void steer_mux(u8 h) {
	// h is [0, 9].

	bf_8b hh = *(bf_8b*)&h;

	u8 ext_mux_ab;
	u8 mcu_mux;
	if(hh.b3){
		// h8, h9
		if(hh.b0){
			set_mcu_mux(MUX_H9);
		}else{
			set_mcu_mux(MUX_H8);
		}
	}else{
		if(hh.b2){
			// [h4-h7]
			set_mcu_mux(EXT_MUX_Y);
		}else{
			// [h0-h3]
			set_mcu_mux(EXT_MUX_X);
		}

		digitalWrite(EXT_MUX_B, hh.b1);
		digitalWrite(EXT_MUX_A, hh.b0);
	}	
}
static void steer_mux_to_next() {
	static u8 h = 9;
	if(h == 9){
		h = 0;
	}else{
		h++;
	}
	steer_mux(h);
}


static void set_pulse(u8 enc, u8 coil) {
#if TEST
	static u8 x = 0;
	x++;
	static u8 t = 0;
	if((x & 0x3) == 0){
		t = !t;
		digitalWrite(TDxy1, t);
	}

#else
#if TRISTATE_TD_EMULATE
	pinMode(enc_to_coil_to_pin[enc][coil], OUTPUT); // 1
	digitalWrite(enc_to_coil_to_pin[enc][coil], 1);
#else
	digitalWrite(enc_to_coil_to_pin[enc][coil], 1);
#endif
#endif
}

static void clear_pulse(u8 enc, u8 coil) {
#if TEST
#else
#if TRISTATE_TD_EMULATE
	pinMode(enc_to_coil_to_pin[enc][coil], INPUT); // tri-state 0
#else
	digitalWrite(enc_to_coil_to_pin[enc][coil], 0);
#endif
#endif
}

#define N_SAMPLES 128

//TODO Use just 8b.
volatile u16 samples[N_SAMPLES];
volatile u16 sample_idx = 0;


static void take_prev_sample_and_start_new_sampling() {
	if(sample_idx < N_SAMPLES){
		steer_mux_to_next(); //TODO Final.
		delayMicroseconds(1); //FIXME Could we live without it?
		
		// Read prevous sample:
		//TODO Use just 8b.
		samples[sample_idx++] = ADCL | ((u16)ADCH << 8);
	
		// Start new sample.
		adcsra.adsc = 1;
	}
}

static void phase_irq() {
	static u8 phase = 3;
	static u8 coil = 5;
	static u8 enc = 3;
	
	if(phase == 3){
		phase = 0;
		if(coil == 5){
			coil = 0;
			if(enc == 3){
				enc = 0;
			}else{
				enc++;
			}
		}else{
			coil++;
		}
	}else{
		phase++;
	}
	
	// We are on start of phase.
	switch(phase){
#if 0
// According to IF4 board RE.
		case 0:
			clear_pulse(0, 0);
			take_prev_sample_and_start_new_sampling();
			break;
		case 1:
			break;
		case 2:
			set_pulse(0, 0);
			break;
		case 3:
			break;
#else
		case 0: // Rise time.
			set_pulse(enc, coil);
			break;
		case 1: // Sample in middle of pulse.
			//take_prev_sample_and_start_new_sampling();
			digitalWrite(SWITCH_A, 1);
			break;
		case 2: // Fall time.
			digitalWrite(SWITCH_A, 0);
			clear_pulse(enc, coil);
			break;
		case 3: // Dead time.
			break;
#endif
	}

}


void setup() {

	Serial.begin(115200);
	Serial.println("setup() running...");
	delay(1); // [ms]

	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, 0);
	pinMode(EXT_MUX_A, OUTPUT);
	pinMode(EXT_MUX_B, OUTPUT);
#if TRISTATE_TD_EMULATE
	//pinMode(TDxy1, OUTPUT);
	digitalWrite(TDxy1, 1);
	pinMode(TDxy1, INPUT); // tri-state 0
#else
	pinMode(TDxy1, OUTPUT);
	digitalWrite(TDxy1, 0);
#endif

	pinMode(SWITCH_A, OUTPUT);


	
	DIDR0 = bf_to_u8<bf_didr0>({
		.adcxd = 0b11111, // Disable digital buffer on analog inputs.
		._res = 0
	});
	
	set_mcu_mux(0);
	//steer_mux_to_next(); // TODO Final.
		
	ADCSRB = bf_to_u8<bf_adcsrb>({
		.adts = 0, // Free running mode.
		._res0 = 0,
		.acme = 0, // Comparator turned off.
		._res1 = 0,
	});
	
	ADCSRA = bf_to_u8<bf_adcsra>({
		.adps = 0b111, // 128 prescaler for slower.
		//.adps = 0b101, // 32 prescaler for 38.5 KHz.
		//.adps = 0b100, // 16 prescaler for 76.9 KHz.
		//.adps = 0b011, // 8 prescaler for 153.8 KHz.
		.adie = 0, // Single-conversion mode.
		.adif = 0, // No IRQ when ADC sampling is done.
		.adate = 0, // No trigger.
		.adsc = 1, // Warm up ADC, because first sampling is 25 CLKs.
		.aden = 1 // Enable ADC.
	});

#if 0
	print_reg("ADCSRA", ADCSRA);
	print_reg("ADMUX", ADMUX);
	print_reg("ADCSRB", ADCSRB);
	print_reg("ADCSRA", ADCSRA);
	print_reg("ADCL", ADCL);
	print_reg("ADCH", ADCH);
	Serial.println();
#endif

	Timer1.initialize(T_PHASE_US); // [us]
	Timer1.attachInterrupt(phase_irq);

	
	//Serial.println("setup() finished.");
}


void print_new_row(int i) {
	char s[5];
	sprintf(s, "\n% 4d:", i);
	Serial.print(s);
}
void print_sample(int i) {
	
	char s[5];
	sprintf(s, " %04x", samples[i]);
	Serial.print(s);
}


#define N_COLS 16
void loop() {
	
	if(sample_idx == N_SAMPLES){
		sample_idx++;
		digitalWrite(TDxy1, 0);
		digitalWrite(LED_BUILTIN, 1);

		int i = 0;
		Serial.print("sample_idx (dec): samples (hex)");

#if 0
		for(int r = 0; r < N_SAMPLES/N_COLS; r++){
			print_new_row(i);
			
			for(int c = 0; c < N_COLS; c++){
				print_sample(i);
				i++;
			}
		}
#else
	print_new_row(i);
	print_sample(i);
	i++;
	for(int r = 0; r < N_SAMPLES/10; r++){
		print_new_row(i);
		
		for(int c = 0; c < 10; c++){
			print_sample(i);
			i++;
		}
	}
#endif
	}
}
