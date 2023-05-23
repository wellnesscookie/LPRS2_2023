
///                    

void setup() {
	pinMode(0, OUTPUT);
	pinMode(1, OUTPUT);
	digitalWrite(0, 0);
	digitalWrite(1, 0);
	
	Serial.begin(9600);
}

void loop() {
	// Read one byte from Master.
	if(Serial.available()) {
		byte rxd = Serial.read();
		
		// Blinking LED1 indicate Rx works.
		digitalWrite(1, rxd & 1);

		// Loop it back to Master.
		Serial.write(rxd);
	}
	
	delay(50); // [ms]

}
