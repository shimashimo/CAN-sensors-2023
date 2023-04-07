#include <SPI.h>
#include "mcp2515_can.h"
#include "mcp2515_can_dfs.h"

// Constant Definitions
#define SPI_CS_PIN 9 // CAN specific pins
#define CAN_INT_PIN 2
#define  CAN_MSG_DELAY 100

#define RELUCTOR 48 // number points of detection (pod) (teeth)
#define WHEEL_DIAMETER 0.65 // in meters (m)
#define CYCLE_SIZE 500 // Take calculation each 500ms
#define RIGHT_WHEEL_INPUT 3 // defining signal pin for wheel speed sensor
#define LEFT_WHEEL_INPUT 4 // defining signal pin for wheel speed sensor

typedef unsigned char byte;

// Global Variables
volatile int right_pulses, left_pulses;
unsigned long timeold;
int cycle;
int right_speed = 0, left_speed = 0;
int right_rpm = 0, left_rpm = 0;
byte right_wss_data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
byte left_wss_data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
byte ave_wss_data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

unsigned long can_timeold;
int can_delay_cycle;

// Function Declarations
void init_CAN();
void send_CAN_msg(unsigned long id, byte ext, byte len, const byte * msg_buf);
void right_wheel_pulse();
void left_wheel_pulse();
void wss_enable_I(int sensor);
void wss_disable_I(int sensor);
void brake_pressure_routine();
void accelerator_position_routine();
void suspension_travel_routine();

mcp2515_can CAN(SPI_CS_PIN); // set CS to pin 9

/********** Setup/Initialization ***************/

void setup() {
  Serial.begin(115200);

  init_CAN();

  timeold = 0;
  right_pulses = 0;
  left_pulses - 0;
  pinMode(RIGHT_WHEEL_INPUT, INPUT);
  pinMode(LEFT_WHEEL_INPUT, INPUT);
  wss_enable_I(0);
  wss_enable_I(1);
}

/********** Main Loop ***************/

void loop() {
  wheel_speed_routine();

}

/********** Function Implementations ***************/

/* 
  Function: Initialize MCP2515 (CAN system) running with a 
            baudrate of 500kb/s. Code will not continue until 
            initialization is OK.
  Params:   NA
  Return:   NA
  Pre-conditions: NA
*/
void init_CAN()
{
  while (CAN_OK != CAN.begin(CAN_500KBPS))
  {
    Serial.println("CAN init fail, retry...");
    delay(100);
  }
  Serial.println("CAN init ok!");
}

/* 
  Function: Sending a CAN message over the network.
  Params:   unsigned long id - The message ID.
            byte ext - The extension code for the message. typically 0.
            byte len - The message length. (max 8 bytes)
            const byte * msg_buf - The message byte array to be transmitted over CAN.
  Return:   NA
  Pre-conditions: NA
*/
void send_CAN_msg(unsigned long id, byte ext, byte len, const byte * msg_buf)
{
  can_delay_cycle = millis() - can_timeold;

  // creating 100ms delay in message transmission without limmiting code from continuing its execution.
  // ** TODO: may need to fix if causes issues sending other values when function is called.
  // may be possible to run without delay??
  if (can_delay_cycle >= CAN_MSG_DELAY)
  {
    byte send_status = CAN.sendMsgBuf(id, ext, len, msg_buf);

    if (send_status == MCP2515_OK) Serial.println("Message Sent Successfully!");
    else Serial.println("message Error...");
    
    can_timeold = millis();
  }
}

/*
  Function: Handles the calculation of the rpm and speed in KMH of the wheels, then
            transmits the data over the can network.
  Params:   NA
  Return:   NA
  Pre-conditions: NA
*/
void wheel_speed_routine()
{
  // Current time delta. Checks against CYCLE_SIZE.
  cycle = millis() - timeold;

  if (cycle >= CYCLE_SIZE)
  {
    // stop counting pulses
    wss_disable_I(0);
    wss_disable_I(1);

    // ( 60000.ms * (#pulses / cycle.ms) ) / #pod
    right_rpm = (60000 * right_pulses / cycle) / RELUCTOR;
    left_rpm = (60000 * left_pulses / cycle) / RELUCTOR;

    // diam.m * (1.km / 1000.m) * PI * rpm * (60.min / 1.h)
    right_speed = (WHEEL_DIAMETER / 1000) * PI * right_rpm * 60; // KMH
    left_speed = (WHEEL_DIAMETER / 1000) * PI * left_rpm * 60; // KMH

    int ave_speed = (right_speed + left_speed) / 2;

    ave_wss_data[0] = (ave_speed & 0xff);
    ave_wss_data[1] = ((ave_speed >> 2) & 0xff);
    send_CAN_msg(0x01,0,8,ave_wss_data);

    // left_wss_data[0] = (left_speed & 0xff);
    // left_wss_data[1] = ((left_speed >> 2) & 0xff);
    // send_CAN_msg(0x02,0,8,left_wss_data);

    // reset counter and cycle. re-enable interrupts.
    right_pulses = 0;
    left_pulses = 0;
    timeold = millis();
    wss_enable_I(0);
    wss_enable_I(1);
  }
}

/*
  Function: ISR for counting wheel pulses on signal detection. When the hall effect
            sensor detects a rising edge, trigger the ISR to increment pulses.
  Params:   NA
  Return:   NA
  Pre-conditions: NA
*/
void right_wheel_pulse() { right_pulses++; }
void left_wheel_pulse() { left_pulses++; }

/*
  Function: Enables the interrupt for pulse count. Attaches signal pin from WSS
            to trigger ISR on a rising signal.
  Params:   int sensor - Defining the left and right sensors
            0 = right wss
            1 = left wss
  Return:   NA
  Pre-conditions: NA
*/
void wss_enable_I(int sensor) 
{ 
  if (sensor = 0) attachInterrupt(digitalPinToInterrupt(RIGHT_WHEEL_INPUT), right_wheel_pulse, RISING);
  else attachInterrupt(digitalPinToInterrupt(LEFT_WHEEL_INPUT), left_wheel_pulse, RISING); 
}

/*
  Function: Disables the interrupt for pulse count.
  Params:   int sensor - Defining the left and right sensors
            0 = right wss
            1 = left wss
  Return:   NA
  Pre-conditions: NA
*/
void wss_disable_I(int sensor) 
{ 
  if (sensor = 0) detachInterrupt(digitalPinToInterrupt(RIGHT_WHEEL_INPUT)); 
  else detachInterrupt(digitalPinToInterrupt(LEFT_WHEEL_INPUT)); 
}

/**/
void brake_pressure_routine()
{

}

/**/
void accelerator_position_routine()
{

}

/**/
void suspension_travel_routine()
{

}












