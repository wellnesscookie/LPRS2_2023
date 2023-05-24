#include <SPI.h>
#include <SD.h>


Sd2Card card;
SdVolume volume;
SdFile root;

File myFile;

// chipSelect pin
const int chipSelect = 10;

void setup() {
	pinMode(0, OUTPUT);
	pinMode(1, OUTPUT);
	digitalWrite(0, 0);
	digitalWrite(1, 0);

  Serial.begin(9600);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    while (1);
  }
  
 
}

void loop() {

 String receivedString;
 
 if (Serial.available() > 0) {
    
  receivedString = Serial.readString();

  Serial.println(receivedString);
  
 } else {
  return;
 }
 
 String timestamp = receivedString.substring(0, 8);
 Serial.println("READ: " + timestamp);

String file_name  = timestamp + ".txt";

 myFile = SD.open(file_name, FILE_WRITE);


 if (myFile) {

    myFile.println(receivedString);
    Serial.println("Successfully saved!");
    myFile.close();
  
 }

 myFile = SD.open(file_name, FILE_READ);

 if (myFile) {
   while (myFile.available()) {
   
       Serial.write(myFile.read());
        
   }
  
   myFile.close();

  
 } else {
   Serial.println("Error opening file");
 }
 

 delay(2000);
	
	
}
