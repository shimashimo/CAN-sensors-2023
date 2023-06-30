#include <SPI.h>
#include "mcp2515_can.h"
#include "mcp2515_can_dfs.h"

typedef unsigned char byte;
const double PI = 3.14159265358979;

// Global Variables and Constants
// -> CAN message vars
#define SPI_CS_PIN 9 // CAN specific pins
#define CAN_INT_PIN 2 // CAN interrupt pin
#define  CAN_MSG_DELAY 100
unsigned long can_timeold;
int can_delay_cycle;
int message_num = 0;

// -> wheel speed sensor vars
#define RELUCTOR 5 // number points of detection (pod) (teeth)
#define WHEEL_DIAMETER 0.5 // in meters (m)
#define CYCLE_SIZE 500 // Take calculation each 500ms
#define RIGHT_WSS_PIN 3 // signal pin for wss RIGHT wheel
#define LEFT_WSS_PIN 4 // signal pin for wss LEFT wheel
volatile int right_pulses, left_pulses;
unsigned long timeold;
int cycle;
int right_speed = 0, left_speed = 0;
int right_rpm = 0, left_rpm = 0;
byte right_wss_data[1] = {0x00};
byte left_wss_data[1] = {0x00};
byte ave_wss_data[1] = {0x00};

// -> brake pressure sensor vars
#define BPS_MIN_ADC_VAL 0 //TODO: calibrate against car setup
#define BPS_MAX_ADC_VAL 1023 //TODO: calibrate against car setup
#define BPS_PIN ##
byte bps_data[1] = {0x00};

// -> accelerator position sensor vars
#define APS_MIN_ADC_VAL 0 //TODO: calibrate against car setup
#define APS_MAX_ADC_VAL 1023 //TODO: calibrate against car setup
#define APS_PIN ##
byte aps_data[1] = {0x00};

// -> suspension travel sensor vars
#define STS_MIN_ADC_VAL 0 //TODO: calibrate against car setup
#define STS_MAX_ADC_VAL 1023 //TODO: calibrate against car setup
const int STS_ADC_RANGE = STS_MAX_ADC_VAL - STS_MIN_ADC_VAL;
const int STS_ADC_R_FACTOR = STS_ADC_RANGE / (2 * PI);
#define STS_SPOOL_RADIUS 30 //TODO: define spool radius in millimeters (mm)
#define STS_PIN ##
byte sts_data[2] = {0x00, 0x00};

// Function Declarations
void init_CAN();
void send_CAN_msg(unsigned long id, byte ext, byte len, const byte * msg_buf);
void message_cycle();

void wheel_speed_routine();
void right_wheel_pulse();
void left_wheel_pulse();
void wss_enable_I();
void wss_disable_I();

void brake_pressure_routine();

void accelerator_position_routine();

void suspension_travel_routine();

/********** Setup/Initialization ***************/

mcp2515_can CAN(SPI_CS_PIN); // set CS to pin 9

void setup() {
  // set the speed of the serial port
  Serial.begin(115200);

  init_CAN();
  can_timeold = 0;

  timeold = 0;
  right_pulses = 0;
  left_pulses - 0;
  pinMode(RIGHT_WSS_PIN, INPUT);
  pinMode(LEFT_WSS_PIN, INPUT);
  wss_enable_I();

  pinMode(BPS_PIN, INPUT);
  pinMode(APS_PIN, INPUT);
  pinMode(STS_PIN, INPUT);
}

/********** Main Loop ***************/

void loop() {
  wheel_speed_routine();
  brake_pressure_routine();
  accelerator_position_routine();

  can_delay_cycle = millis() - can_timeold;
  if (can_delay_cycle >= CAN_MSG_DELAY) message_cycle();
}

/********** Function Implementations ***************/

/* 
  Function: Initialize MCP2515 (CAN system) running with a 
            baudrate of 500kb/s. Code will not continue until 
            initialization is OK.
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
*/
void send_CAN_msg(unsigned long id, byte ext, byte len, const byte * msg_buf)
{
  byte send_status = CAN.sendMsgBuf(id, ext, len, msg_buf);

  if (send_status == MCP2515_OK) Serial.println("Message Sent Successfully!");
  else Serial.println("message Error...");
}

/*
  Function: The message_cycle() function is used to perpetually cycle through and
            send messages from the various sensors connected to the front canduino.
            Each global data array for the sensors is constantly updated from the main
            loop from which message_cycle() takes the data array and transmits the 
            message over the CAN network. 
  Params:   global sensor data buffers - The global variables are used to as containers
            to allow for indefinite updating of the sensor values. The fuction then reads
            those values and transmits them.
  Return:   NA
  Pre-conditions: CAN must first be initialized.
*/
void message_cycle()
{
  switch (message_num)
  {
    case 0:
      //wheel speed sensor
      send_CAN_msg(0x55, 0, 1, ave_wss_data);
      message_num++;
      break;
    
    case 1:
      //brake presure sensor
      send_CAN_msg(0x66, 0, 1, bps_data);
      message_num++;
      break;
    
    case 2:
      //accelerator position sensor
      send_CAN_msg(0x77, 0, 1, aps_data);
      message_num++;
      break;

    case 3:
      //suspension travel sensor
      send_CAN_msg(0x88, 0, 1, sts_data);
      message_num = 0;
      break;

    default:
      //reset message number if issue is encountered.
      message_num = 0;
  }

  can_timeold = millis();
}

/*
  Function: Handles the calculation of the rpm and speed in KMH of the wheels, then
            transmits the data over the can network.
*/
void wheel_speed_routine()
{
  // Current time delta. Checks against CYCLE_SIZE.
  cycle = millis() - timeold;

  if (cycle >= CYCLE_SIZE)
  {
    // stop counting pulses
    wss_disable_I();

    // ( 60000.ms * (#pulses / cycle.ms) ) / #pod
    right_rpm = (60000 * right_pulses / cycle) / RELUCTOR;
    left_rpm = (60000 * left_pulses / cycle) / RELUCTOR;

    // diam.m * (1.km / 1000.m) * PI * rpm * (60.min / 1.h)
    right_speed = (WHEEL_DIAMETER / 1000) * PI * right_rpm * 60; // KMH
    left_speed = (WHEEL_DIAMETER / 1000) * PI * left_rpm * 60; // KMH

    int ave_speed = (right_speed + left_speed) / 2;

    ave_wss_data[0] = ave_speed & 0xff;
    // ave_wss_data[1] = ((ave_speed >> 2) & 0xff); note: shouldnt need more than 0-255

    left_wss_data[0] = left_speed & 0xff;
    right_wss_data[0] = right_speed & 0xff;
   
    // send_CAN_msg(0x02,0,8,left_wss_data);

    // reset counter and cycle. re-enable interrupts.
    right_pulses = 0;
    left_pulses = 0;
    timeold = millis();
    wss_enable_I();
  }
}

/*
  Function: ISR for counting wheel pulses on signal detection. When the hall effect
            sensor detects a rising edge, trigger the ISR to increment pulses.
*/
void right_wheel_pulse() { right_pulses++; }
void left_wheel_pulse() { left_pulses++; }

/*
  Function: Enables the interrupt for pulse count. Attaches signal pin from WSS
            to trigger ISR on a rising signal.
*/
void wss_enable_I() 
{ 
  attachInterrupt(digitalPinToInterrupt(RIGHT_WSS_PIN), right_wheel_pulse, RISING);
  attachInterrupt(digitalPinToInterrupt(LEFT_WSS_PIN), left_wheel_pulse, RISING); 
}

/*
  Function: Disables the interrupt for pulse count.
*/
void wss_disable_I() 
{ 
  detachInterrupt(digitalPinToInterrupt(RIGHT_WSS_PIN)); 
  detachInterrupt(digitalPinToInterrupt(LEFT_WSS_PIN)); 
}

/*
  Function: Brake pressure routine reads in the analog signal
            from the brake pressure sensor, taking the value
            returned from the analog to digital converter (ADC)
            and converting the it to a percentage out of 100 
            based on where the value exists between the 
            MIN and MAX sensor values.
*/
void brake_pressure_routine()
{
  int bps_reading = analogRead(BPS_PIN);
  int bps_corr_reading = bps_reading - BPS_MIN_ADC_VAL;

  int percentage = (100 * bps_corr_reading) / BPS_MAX_ADC_VAL;

  bps_data[0] = percentage & 0xff;
}

/*
  Function: Accelerator position routine reads in the analog signal
            from the accelerator potentiometer sensor, taking the value
            returned from the analog to digital converter (ADC)
            and converting the it to a percentage out of 100 
            based on where the value exists between the 
            MIN and MAX sensor values.
*/
void accelerator_position_routine()
{
  // TODO: using two sensors, determine whether sensors are reasonably equal
  int aps_reading = analogRead(APS_PIN);
  int aps_corr_reading = aps_reading - APS_MIN_ADC_VAL;

  int percentage = (100 * aps_corr_reading) / APS_MAX_ADC_VAL;

  aps_data[0] = percentage & 0xff;
}

/*
  Function: Suspension travel sensor reads in an analog signal from
            the suspension potentiometer sensor. The reading is then
            subtracted from the minimum sensor value calculating a
            "digital" arc length. From the digital arc length we can
            find the angle of rotation and thus calculate the arc length
            the spool of line (attached to the suspension) traveled
            calculating the resulting travel amount.

            Travel is calculated in millimeters (mm) of travel.
*/
void suspension_travel_routine()
{
  int sts_reading = analogRead(STS_PIN);

  double travel_angle = (sts_reading - STS_MIN_ADC_VAL) / STS_ADC_R_FACTOR;

  int travel_mm = travel_angle * STS_SPOOL_RADIUS;

  sts_data[0] = travel_mm & 0xff;
  sts_data[1] = (travel_mm >> 8) & 0xff;
}












