#include <stdlib.h>
#include <SPI.h>
#include "mcp2515_can.h"
#include "mcp2515_can_dfs.h"

typedef unsigned char byte;

unsigned long canId = 0x00; // CAN message ID
byte msg_length = 0; // length of message from CAN
byte msg_buffer[8]; // buffer for storing message from CAN

#define  CAN_MSG_DELAY 100
unsigned long can_timeold;
int can_delay_cycle;
int message_num = 0;

// -> Global Data Containers
byte Starter_SW_data[1] = {0x00};   const byte STARTER_CAN_ID = 0x20;
byte Brake_OK_data[1] = {0x00};     const byte BRAKE_CAN_ID = 0x21;
byte Fuel_Pump_OK_data[1] = {0x00}; const byte FUEL_CAN_ID = 0x22;
byte Fan_OK_data[1] = {0x00};       const byte FAN_CAN_ID = 0x23;
byte RTDS_data[1] = {0x00};         const byte RTDS_CAN_ID = 0x24;  // Ready to Drive Signal

const int CAN_INT_PIN = 2;
const int SPI_CS_PIN = 9;

// Function Definitions
void init_CAN();
mcp2515_can CAN(SPI_CS_PIN); // set CS to pin 9
void CAN_message_handler();
void CAN_read_from_network();
void switch_data_store(unsigned long CAN_ID, unsigned char*& data_container);

/* 
  PD7 = Starter SW
  PD8 = RTDS
  PD9 = Spare 12 V
  PD10 = Brake Light Switch
  PD3 = Fuel Pump
  PD4 = Engine Fan x2
*/

// Sensor input pins

const int BRAKE_SENSOR_PIN = A0;  // A0 = Pin 14
const int IN_RTDS_PIN = 0;

// Output pins
const int OUT_STARTER_SW_PIN = 7;
const int OUT_RTDS_PIN = 8;
const int OUT_BRAKE_LIGHT_PIN = 10;
const int OUT_FUEL_PUMP_PIN = 3;
const int OUT_FAN_PIN = 4;

// initialize sensor variables
volatile int BrakeSensorVal = 0;

void setup() 
{
  Serial.begin(115200);
  init_CAN();

  pinMode(IN_RTDS_PIN, INPUT);  // pin 0 is also RX pin so need to configure as digital input

  pinMode(OUT_BRAKE_LIGHT_PIN, OUTPUT);
  pinMode(OUT_RTDS_PIN, OUTPUT);
  // pinMode(OUT_STARTER_SW_PIN, OUTPUT);
  // pinMode(OUT_FUEL_PUMP_PIN, OUTPUT);
  // pinMode(OUT_FAN_PIN, OUTPUT);
}

void loop() 
{  
  CAN_message_handler();

  can_delay_cycle = millis() - can_timeold;
  if (can_delay_cycle >= CAN_MSG_DELAY) {
    can_timeold = millis();
  }

  /* Brakes */
  BrakeSensorVal = analogRead(BRAKE_SENSOR_PIN);
  // If brake pressure is over some threshold, turn the brake light on
  if(BrakeSensorVal > 100) {   
    digitalWrite(OUT_BRAKE_LIGHT_PIN, HIGH);  // Turn brake light on
    delay(500); // Minimum on for 500ms
  } else {
    digitalWrite(OUT_BRAKE_LIGHT_PIN, LOW);
  }

  /* Ready to Drive Signal - Input from steering wheel */
  if(digitalRead(IN_RTDS_PIN)) {
    digitalWrite(OUT_RTDS_PIN, HIGH);
    delay(3000); // On for 3 seconds
  } else {
    digitalWrite(OUT_RTDS_PIN, LOW);
  }

  /* Fuel Pump */
  /* Need to know what kind of signal Starter */

}



// Initialize MCP2515 running with a baudrate of 500kb/s.
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
  Function: The message_read() function gets the CAN ID of the current message on the
            CAN network and routes the program to the appropriate handler.
*/
void CAN_message_handler()
{
  // TODO: This function is used as the main point of executing any function associated
  // to a read CAN message. Instead of calling your functions in "loop()", add cases
  // for each can ID and call your function from within the case

  if (CAN_MSGAVAIL == CAN.checkReceive())
  {
    canId = CAN.getCanId();
    CAN.readMsgBuf(&msg_length, msg_buffer);

    /* Debugging Prints */
    Serial.print("\nCanID: ");
    Serial.println(canId);

    /* Could change this switch case to a range of CAN IDs instead of having one for each case */
    /* Like: if (canID <= 0x99 && canID >= 0x55 ) CAN_read_from_network(); */
    switch (canId)
    {
      case BRAKE_CAN_ID:
        // Serial.println("BRAKE CAN MESSAGE RECEIVED");
        // Cant this just be condensed into a global data_container, then setting the data_container to the pointer of the correct container? No need for all this overhead?
        CAN_read_from_network();
        break;

      case FUEL_CAN_ID:
        // Serial.println("FUEL CAN MESSAGE RECEIVED");
        CAN_read_from_network();
        break;

      case FAN_CAN_ID:
        // Serial.println("FAN CAN MESSAGE RECEIVED");
        CAN_read_from_network();
        break;

      default:
        // do nothing if not an accepted CAN ID
        Serial.println("Unknown CAN ID");
        Serial.print(int(canId));
        break;
    }
  }
}

/*
  Function: The CAN_read_from_network() function is used to read the current message off
            off the CAN network and route the incoming message to be stored in its respective
            container. ie reading the wheel speed ID, then storing/updating the message buffer
            to the wheel speed container variable. 
*/
void CAN_read_from_network()
{
  unsigned char* data_container = NULL;
  switch_data_store(canId, data_container);
  if (data_container == NULL) 
  {
    Serial.println("error setting the data container...\n");
    return;
  }

  for(int i=0; i < msg_length; i++)
  {
    // Serial.print(msg_buffer[i]);
    data_container[i] = msg_buffer[i];
  }
}

/*
  Function: HELPER method used in the CAN_read_from_network() function. This method
            is used to select the appropriate container for storing the data read
            from the incoming CAN message. the method returns a pointer to the data
            container that is used for storing data in the read function.
*/
void switch_data_store(unsigned long CAN_ID, unsigned char*& data_container)
{
  switch(CAN_ID)
  {
    case BRAKE_CAN_ID:
      data_container = Brake_OK_data;
      break;

    case FUEL_CAN_ID:
      //engine temp
      data_container = Fuel_Pump_OK_data;
      break;

    case FAN_CAN_ID:
      //engine temp
      data_container = Fan_OK_data;
      break;

    default:
      *data_container = NULL;
      break;
  }
}
