#include <SPI.h>
//#include <EEPROM.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"


//------Variables--------------------------------------

int pin1_val       =  0; // Value on ID Pin
int pin2_val       =  0; 
uint8_t unitID     =  0; // 0 - Default. 1 - North. 2 - South. 3 - East. 4 - West.    

const int pin1     =  18; // User defined ID pins
const int pin2     =  19; // ID Pin

int pinNORTH       = 17;
int pinEAST        = 16;
int pinSOUTH       = 15;
int pinWEST        = 14;

//Set up the timers
unsigned long timer1 =0;
unsigned long timer2 =0;
unsigned long timer3 =0;
unsigned long timer4 =0;
unsigned long interval = 5000; //Time that the LED should stay on for in milliseconds
unsigned long currentMillis;


const int pin_trip =  7; // Pin dedicated to ir trip 
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
        pinMode(pinNORTH, OUTPUT);
        pinMode(pinEAST, OUTPUT);
        pinMode(pinSOUTH, OUTPUT);
        pinMode(pinWEST, OUTPUT);
        
        
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

	if (pin1_val == 0 && pin2_val == 0)
		{ unitID = 1;}
	
	else if (pin1_val == 0 && pin2_val == 1)
		{ unitID = 4; }
	
	else if (pin1_val == 1 && pin2_val == 0)
		{ unitID = 3; }	

	else if (pin1_val == 1 && pin2_val == 1)
		{ unitID = 2; }	
      //Blink all of the LEDs
       for(int blinknums = 0; blinknums <=4; blinknums++) {
            digitalWrite(14, HIGH);
            digitalWrite(15, HIGH);
            digitalWrite(16, HIGH);
            digitalWrite(17, HIGH);
            delay(300);
            digitalWrite(14, LOW);
            digitalWrite(15, LOW);
            digitalWrite(16, LOW);
            digitalWrite(17, LOW);
            delay(300);
        }
      
      
        //Blink the LED of the unit that this device is
        for(int blinknums = 0; blinknums <=6; blinknums++) {
            digitalWrite(13+unitID, HIGH);
            delay(150);
            digitalWrite(13+unitID, LOW);
            delay(150);
        }
	
//------Begin radio and define communication pipes-----	
	radio.begin();
	
	const uint64_t talking_pipes[4] = { 0xF0F0F0F0D2LL, 0xF0F0F0F0C3LL, 0xF0F0F0F0B4LL, 0xF0F0F0F0A5LL };
	
	if ( unitID == 1 )
		{
		radio.openReadingPipe(1,talking_pipes[1]);
                radio.openReadingPipe(2,talking_pipes[2]);
                radio.openReadingPipe(3,talking_pipes[3]);
                
		radio.openWritingPipe(talking_pipes[0]);
		}
  
	if ( unitID == 2 )
		{
		radio.openReadingPipe(1,talking_pipes[0]);
                radio.openReadingPipe(2,talking_pipes[2]);
                radio.openReadingPipe(3,talking_pipes[3]);
                
		radio.openWritingPipe(talking_pipes[1]);
		}

	if ( unitID == 3 )
		{
		radio.openReadingPipe(1,talking_pipes[0]);
                radio.openReadingPipe(2,talking_pipes[1]);
                radio.openReadingPipe(3,talking_pipes[3]);
                
		radio.openWritingPipe(talking_pipes[2]);
		}
  
	if ( unitID == 4 )
		{
		radio.openReadingPipe(1,talking_pipes[0]);
                radio.openReadingPipe(2,talking_pipes[1]);
                radio.openReadingPipe(3,talking_pipes[2]);
                
		radio.openWritingPipe(talking_pipes[3]);
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

                //Turn on your own LED if you got tripped, and start your timer
                digitalWrite(13+unitID, HIGH);
                if (unitID == 1)
                  { timer1 = millis(); }
                else if (unitID == 2)
                  { timer2 = millis(); }
                else if (unitID == 3)
                  { timer3 = millis(); }
                else if (unitID == 4)
                  { timer4 = millis(); }

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
                //delay so that there is sufficient enough time to listen
                delay(10);
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
                //Turn on that LED
                digitalWrite(13+tripID, HIGH);
                //Start the timer of the unit that got tripped
                if (tripID == 1)
                  { timer1 = millis(); }
                else if (tripID == 2)
                  { timer2 = millis(); }
                else if (tripID == 3)
                  { timer3 = millis(); }
                else if (tripID == 4)
                  { timer4 = millis(); }
		}
         else 
               
                //If it has been [interval] since the LED went on, then we will shut it off
                currentMillis = millis();
                if (currentMillis >= (timer1 + interval))
                  { digitalWrite(14, LOW);
                    timer1 = 0; }
                if (currentMillis >= (timer2 + interval))
                  { digitalWrite(15, LOW);
                    timer2 = 0; }
                if (currentMillis >= (timer3 + interval))
                  { digitalWrite(16, LOW);
                    timer3 = 0; }
                if (currentMillis >= (timer4 + interval))
                  { digitalWrite(17, LOW);
                    timer4 = 0; }
           
}
