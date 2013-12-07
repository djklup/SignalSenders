#include <SPI.h>
//#include <EEPROM.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"


//------Variables--------------------------------------

int pin1_val       =  0; // Value on ID Pin
int pin2_val       =  0; 
uint8_t unitID     =  0; // 0 - Default. 1 - North. 2 - South. 3 - East. 4 - West.    

const int pin1     =  6; // User defined ID pins
const int pin2     =  7; // ID Pin
int pinLED         = 17; // LED North

const int pin_trip =  5; // Pin dedicated to ir trip 
int trip           =  0; // Variable for trip state
uint8_t tripID     =  0; // Used to identify trip and which LED to light.


//------Program Start----------------------------------

RF24 radio(9,10);
void setup(void)
{
   Serial.begin(9600);
   printf_begin();
//------I/O Setup--------------------------------------
 //-----Set up LED Pins--------------------------------
        pinMode(pinLED, OUTPUT);
        
        
 //-----Set up Two pins for ID Process-----------------
 // Pin 1 - ATMEGA328 - 28
	pinMode(pin1, INPUT);
	digitalWrite(pin1,HIGH);
	delay(20); // Wait for Write
  
	pin1_val = digitalRead(pin1);

 // Pin 2 - ATMEGA328 - 27
	pinMode(pin2, INPUT);
	digitalWrite(pin2,HIGH);
	delay(20); // Wait for Write
  
	pin2_val = digitalRead(pin2);
  
 //-----Logic to ID unit from user configuration-------

	if (pin1_val == 0)
		{ unitID = 1; }
	
	//else if (pin1_val == 0 && pin2_val == 1)
		//{ unitID = 4; }
	
	//else if (pin1_val == 1 && pin2_val == 0)
		//{ unitID = 3; }	

	if (pin1_val == 1)
		{ unitID = 2; }	
	
//------Begin radio and define communication pipes-----	
	radio.begin();
	
	const uint64_t talking_pipes[2] = { 0xF0F0F0F0D2LL, 0xF0F0F0F0C3LL };
	//const uint64_t listening_pipes[2] = { 0x3A3A3A3AD2LL, 0x3A3A3A3AC3LL };

	if ( unitID == 1 )
		{
		radio.openReadingPipe(1,talking_pipes[1]);
		radio.openWritingPipe(talking_pipes[0]);
		}
  
	if ( unitID == 2 )
		{
		radio.openReadingPipe(1,talking_pipes[0]);
		radio.openWritingPipe(talking_pipes[1]);
		}

//------Print setup details---------------------------
	printf("%i",unitID);
        //printf(&unitID);
	Serial.println();
	radio.printDetails();

//------Start Listening-------------------------------
	radio.startListening();
}

void loop(void)
{
//------Check for IR Trip-----------------------------
	trip = digitalRead(pin_trip);
	if (trip == 1)
		{
		radio.stopListening();
		printf("TRIP");
		printf("Now sending Trip....\n");
                printf("Unit %i has tripped.\n",unitID);
  //----Send Unit ID to all others to alert
                unsigned long sentID;
                //sentID = unitID;
		bool ok = false;
                ok = radio.write( &unitID, sizeof(unitID) );
		if (ok)
			printf("ok\n\r");
		else
			printf("failed\n\r");
		printf("Listening Resumed\n");
  //----Resume Listen
		radio.startListening();
		// Send to Led Blink
		// tripID = unitID;
		// keep tripID until sent
		// tripID will change when something else is sent.
		}
//------Check for RF Trip----------------------------	
  //----If Radio signals being sensed 
	if ( radio.available() )
		{
		radio.read( &tripID, sizeof(tripID));
		printf("Unit %i has tripped.\n",tripID);
		digitalWrite(pinLED, HIGH);
		}
         else 
                digitalWrite(pinLED, LOW);
           
}
















 
 
 
 
