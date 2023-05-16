
#include <Arduino.h>

#define RELAY_PIN A1

#include <TimerOne.h>
#include <EEPROM.h>

#include "avr_io_bitfields.h"


#define DEBUG(x) \
	do{ \
		Serial.print(#x" = "); Serial.println(x); \
	}while(0)

/////
// LCD stuff.

#include <LiquidCrystal.h>

#define LCD_BL 10
LiquidCrystal lcd(
	8, // RS
	9, // EN
	4, // D4
	5, // D5
	6, // D6
	7  // D7
);

enum pb_t {
	PB_RIGHT,
	PB_UP,
	PB_DOWN,
	PB_LEFT,
	PB_SELECT,
	PB_NONE
};

pb_t read_LCD_Shield_PBs() {
	int x;
	x = analogRead(0);
	if(x < 60){
		return PB_RIGHT;
	}else if(x < 200){
		return PB_UP;
	}else if(x < 400){
		return PB_DOWN;
	}else if(x < 600){
		return PB_LEFT;
	}else if(x < 800){
		return PB_SELECT;
	}else{
		return PB_NONE;
	}
}

enum mode_t {
	EDITING,
	COUNTING
};
mode_t mode = EDITING;

void test_LCD_Shield_PBs() {
	switch(read_LCD_Shield_PBs()){
	case PB_RIGHT:
		Serial.println("PB_RIGHT");
		break;
	case PB_UP:
		Serial.println("PB_UP");
		break;
	case PB_DOWN:
		Serial.println("PB_DOWN");
		break;
	case PB_LEFT:
		Serial.println("PB_LEFT");
		break;
	case PB_SELECT:
		Serial.println("PB_SELECT");
		break;
	default:
		Serial.println("PB_NONE");
		break;
	}
}

/////

//TODO
class Time {
public:
	u8 sec1;
	u8 sec10;
	u8 min1;
	u8 min10;
	u8 hour1;
	u8 hour10;
	u8 days;
};

u16 start_sec;
u16 sec;
u16 cursor_pos;

void print_editing() {
	lcd.clear();
	static char buffer[20];
	sprintf(buffer, "%04d sec", start_sec);
	lcd.setCursor(0, 0);
	lcd.print(buffer);

	lcd.setCursor(cursor_pos, 1);
	lcd.print("^");
}

void print_counting() {
	lcd.clear();
	static char buffer[20];
	sprintf(buffer, "%04d sec", sec);
	lcd.setCursor(0, 0);
	lcd.print(buffer);

	lcd.setCursor(0, 1);
	lcd.print("on");
}

void on_1_sec() {
	if(mode == COUNTING){
		sec--;
		if(sec == 0){
			mode = EDITING;
			Timer1.stop();
			digitalWrite(RELAY_PIN, 0);
			
			print_editing();
		}else{
			print_counting();
		}
	}
}


void setup() {
	digitalWrite(RELAY_PIN, 0);
	pinMode(RELAY_PIN, OUTPUT);
	
	Serial.begin(115200);

	lcd.begin(16, 2);

	Timer1.initialize();
	Timer1.attachInterrupt(
		on_1_sec,
		1*1000L*1000L
	);
	Timer1.stop();
	//TODO 2B
	start_sec = EEPROM.read(0);
	cursor_pos = 0;

	print_editing();
}

const int cursor_pos__to__base10[] = {
	1000,
	100,
	10,
	1
};

void loop() {
	static pb_t prev_pb = PB_NONE;
	pb_t curr_pb = read_LCD_Shield_PBs();
	if(prev_pb == PB_NONE){
		switch(curr_pb){
			case PB_RIGHT:
				if(mode == EDITING){
					cursor_pos++;
					print_editing();
				}
				break;
			case PB_LEFT:
				if(mode == EDITING){
					cursor_pos--;
					print_editing();
				}
				break;
			case PB_UP:
				if(mode == EDITING){
					start_sec += cursor_pos__to__base10[cursor_pos];
					print_editing();
				}
				break;
			case PB_DOWN:
				if(mode == EDITING)
				{
					start_sec -= cursor_pos__to__base10[cursor_pos];
					print_editing();
				}
				break;
			case PB_SELECT:
				if(mode == EDITING){
					mode = COUNTING;
					sec = start_sec;
					digitalWrite(RELAY_PIN, 1);
					Timer1.restart();
					
					print_counting();
					
					int eeprom_sec = EEPROM.read(0);
					if(start_sec != eeprom_sec){ 
						// Save.
						EEPROM.write(0, start_sec);
					}
				}else{
					mode = EDITING;
					digitalWrite(RELAY_PIN, 0);
					
					print_editing();
				}
				break;
			default:
				break;
		}
	}
	prev_pb = curr_pb;

	const char* mode_s = mode == EDITING ? "EDITING" : "COUNTING";
	DEBUG(mode_s);
	DEBUG(sec);
	DEBUG(start_sec);


#if 0
	// Test.
	pb_t pb = read_LCD_Shield_PBs();
	bool on = pb == PB_SELECT;
	digitalWrite(RELAY_PIN, on);
	test_LCD_Shield_PBs();

	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("on  = ");
	lcd.print(on);
#endif

	delay(10);
}
