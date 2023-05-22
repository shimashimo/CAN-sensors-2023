#include <SPI.h>
#include <mcp_can.h>

// Car definitions
#define RELUCTOR 5        // Number of teeth(points of detection)
#define WHEEL_DIAMETER .5 // In meter (m)

// Program definitions
#define CYCLE_SIZE 500 // Take calculated each 500ms
#define WHEEL_INPUT 2   // Digital pin 2 (interrupt enabled)

volatile int pulses;
int speed = 0;
int rpm = 0;
unsigned long timeold;

//The interrupt function that allows for the arduino to
//stop and read the input from the sensor and counts the pulse
void wheelPulse() { pulses++; }

void enableInterrupts()
{
	//initializing where the interrupt (what pin on the arduino)
	//is taking place and under what condition it will trigger. In
	//this case it is looking for when the signal rises
  attachInterrupt(digitalPinToInterrupt(WHEEL_INPUT), wheelPulse, RISING);
}

void disableInterrupts()
{
	//creating the ability to stop the interrupt so that
	//other actions can be performed with the data
	//thats been recorded
  detachInterrupt(digitalPinToInterrupt(WHEEL_INPUT));
}

void setup()
{
	//using 115200 allows for faster back/forth between sensor and board
	//9600 is simply a slower rate
  Serial.begin(115200);
  
  timeold = 0;
  pulses = 0;
  pinMode(WHEEL_INPUT, INPUT);

  enableInterrupts();  
}

int cycle;

void loop()
{
	//Calculating the amount of time that has passed
	//while actively using "interrupt" to count the
	//number of pulses from the sensor of that timespan
  cycle = millis() - timeold;

	//If we reach the defined period of time for a cycle
	//use that cycle period to calculate the rpm and speed
  if (cycle >= CYCLE_SIZE)
  {
    //Disables interrupt so that the number of pulses doesnt change
	//while we are trying to use pulses to perform a calculation
    disableInterrupts();
        
	//1minute=60000milliseconds
    rpm = (60000 * pulses / cycle) / RELUCTOR;
	//1km=1000meters
    speed = (WHEEL_DIAMETER / 1000) * PI * rpm * 60; // KMH
	
	//Reset the pulse counter so that we can count
	//a new set of pulses over a new time period (cycle)
    pulses = 0;
    
    Serial.println(rpm, DEC);
    Serial.print(speed, DEC);
    Serial.println("Km/h\n");

	//Define the new starting time, effectively resetting the
	//stop watch for a new cycle
    timeold = millis();
	
    //Re-enable interrupts so that we can count the number of
	//pulses over our new interval
    enableInterrupts();
  }
}