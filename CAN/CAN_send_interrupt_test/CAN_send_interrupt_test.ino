// CAN Send Testing with Interrupt while receiving (Closest to Steering Canduino behaviour)
// Receives updated sensor values over can while also sending an interrupt occasionally with a push button
// Connect D7 on the circle Canduino to one side of pushbutton and ground to the other.
// Connect all canduinos with CAN_H on 1 rail and CAN_L on the other rail. 
// IMPORTANT NOTE?: the circle canduino is the only canduino without termination resistor soldered so should be in the middle 
#include <SPI.h>
#include "mcp2515_can.h"
#include "mcp2515_can_dfs.h"

typedef unsigned char byte;

// const int SPI_CS_PIN = 9;
// const int CAN_INT_PIN = 2;
#define SPI_CS_PIN 9 // CAN specific pins
#define CAN_INT_PIN 2 // CAN interrupt pin
unsigned long canId = 0x00; // CAN message ID
byte msg_length = 0; // length of message from CAN
byte msg_buffer[8]; // buffer for storing message from CAN
#define CAN_MSG_DELAY 1000  // time between message sends in millis
unsigned long can_timeold;
int can_delay_cycle;
int message_num = 0;

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 200;
const int SHIFT_UP_BUTTON_PIN = 7;
bool shift_up_flag = false;

/*************** GLOBAL CAN VARS AND CONTAINERS ****************************/
byte Brake_OK_data[1]     = {0x00};   const byte BRAKE_CAN_ID = 0x55; 
byte Fuel_Pump_OK_data[1] = {0x00};   const byte FUEL_CAN_ID  = 0x66;
byte Fan_OK_data[1]       = {0x00};   const byte FAN_CAN_ID   = 0x77;
byte Wheel_Speed_data[1]  = {0x00};   const byte WSS_CAN_ID   = 0x88;
byte Shift_Up_data[1]     = {0x00};   const byte SHIFT_UP_CAN_ID = 0x99;
/***************************************************************************/

// Function Definitions
void init_CAN();
mcp2515_can CAN(SPI_CS_PIN); // set CS to pin 9
void CAN_message_handler();
void CAN_read_from_network();
void switch_data_store(unsigned long, unsigned char*&);

void setup() {
  Serial.begin(115200);
  init_CAN();
  can_timeold = 0;

  pinMode( SHIFT_UP_BUTTON_PIN, INPUT_PULLUP);

  // Using pin change interrupts for testing cuz no access to hardware interrupt pins right now.
  // Enable PCIE2 Bit3 = 1 (Port D)
  PCICR |= B00000100;
  // Select PCINT23 Bit7 = 1 (Pin D7)
  PCMSK2 |= B10000000;


  Serial.println("\n-- SEND --\n");
}

// test data to transmit onto CAN network
// unsigned char data[8] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x5d, 0x06, 0x07};

void loop() 
{  
  CAN_message_handler();

  // can_delay_cycle = millis() - can_timeold;
  // if (can_delay_cycle >= CAN_MSG_DELAY) {
  //   can_timeold = millis();
  // }
  if(shift_up_flag) {
    Serial.println("INTERRUPT SHIFTING");
    Shift_Up_data[0] = 0x01;
    send_CAN_msg(0x99, 0, 1, Shift_Up_data);
    shift_up_flag = false;
  }
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
    // Serial.print("\nCandID: "); Serial.println(canId);
    // for (int i=0; i < msg_length; i++) {
    //   Serial.println(msg_buffer[i]);
    // }

    /* Could change this switch case to a range of CAN IDs instead of having one for each case */
    /* Like: if (canID <= 0x99 && canID >= 0x55 ) CAN_read_from_network(); */
    switch (canId)
    {
      case BRAKE_CAN_ID:
        Serial.println("BRAKE CAN MESSAGE RECEIVED");
        CAN_read_from_network();
        break;

      case FUEL_CAN_ID:
        Serial.println("FUEL CAN MESSAGE RECEIVED");
        CAN_read_from_network();
        break;

      case FAN_CAN_ID:
        Serial.println("FAN CAN MESSAGE RECEIVED");
        CAN_read_from_network();
        break;

      case WSS_CAN_ID:
        Serial.println("WSS CAN MESSAGE RECEIVED");
        CAN_read_from_network();
        break;
/*
      case some_CAN_ID:
        some_event_handler_function();
        break;
*/
      default:
        // do nothing if not an accepted CAN ID
        break;
    }
    Serial.println("=== Printing Data Containers");
    Serial.print("Brake Data: "); Serial.print(Brake_OK_data[0]); Serial.println("");
    Serial.print("Fuel Data: "); Serial.print(Fuel_Pump_OK_data[0]); Serial.println("");
    Serial.print("Fan Data: "); Serial.print(Fan_OK_data[0]); Serial.println("");
    Serial.print("WheelSpeed Data: "); Serial.print(Wheel_Speed_data[0]); Serial.println("\n\n");
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

    case WSS_CAN_ID:
      //engine temp
      data_container = Wheel_Speed_data;
      break;

    //...

    default:
      *data_container = NULL;
      break;
  }
}


ISR (PCINT2_vect)
{
  if(debounce()) shift_up_flag = true;
}

bool debounce()
{
  // check to see if enough time has passed
  // since the last signal to ignore any bounce:
  if ((millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis();
    return true;
  }
  return false;
}