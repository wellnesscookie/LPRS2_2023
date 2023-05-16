
#include <Arduino.h>

#define DEBUG(x) \
	do{ \
		Serial.print(#x" = "); Serial.println(x); \
	}while(0)


void setup() {
	pinMode(LED_BUILTIN, OUTPUT);
	
	Serial.begin(115200);
}

void loop() {
	static bool b;
	b = !b;
	digitalWrite(LED_BUILTIN, b);
	delay(500); // [ms]
}
