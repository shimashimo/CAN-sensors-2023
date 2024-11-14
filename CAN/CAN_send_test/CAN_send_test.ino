// CAN Send Testing
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

// -> Example Global Vars
byte Brake_OK_data[1] = {0x00}; byte BRAKE_CAN_ID = 0x55; 
byte Fuel_Pump_OK_data[1] = {0x00}; byte FUEL_CAN_ID = 0x66;
byte Fan_OK_data[1] = {0x00}; byte FAN_CAN_ID = 0x77;
byte Wheel_Speed_data[1] = {0x00}; byte WSS_CAN_ID = 0x88;
int iRandNum;
// Function Declaration
void init_CAN();
mcp2515_can CAN(SPI_CS_PIN); // set CS to pin 9

void setup() {
  Serial.begin(115200);
  init_CAN();
  can_timeold = 0;

  Serial.println("\n-- SEND --\n");
}

// test data to transmit onto CAN network
// unsigned char data[8] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x5d, 0x06, 0x07};

void loop() {
  // iRandNum = rand() % (max - min + 1) + min;
  Brake_OK_data[0] = random(50, 59);
  Fuel_Pump_OK_data[0] = random(60, 69);
  Fan_OK_data[0] = random(70, 79);
  Wheel_Speed_data[0] = random(80, 89);
  

  can_delay_cycle = millis() - can_timeold;
  if (can_delay_cycle >= CAN_MSG_DELAY) {
    Serial.print("PRinting BRAKE OK STATUS");
    Serial.println(Brake_OK_data[0]);
    
    can_timeold = millis();
    message_cycle();
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

void send_CAN_msg(unsigned long id, byte ext, byte len, const byte * msg_buf)
{
  Serial.print("Sending ID: "); Serial.print(id); Serial.print("\n");
  byte send_status = CAN.sendMsgBuf(id, ext, len, msg_buf);

  if (send_status == MCP2515_OK) Serial.println("Message Sent Successfully!\n");
  else Serial.println("message Error...\n");
}

void message_cycle()
{
  switch (message_num)
  {
    case 0:
      Serial.print("Sending Brake: ");
      Serial.println(Brake_OK_data[0]);
      send_CAN_msg(0x55, 0, 1, Brake_OK_data);
      message_num++;
      break;
    
    case 1:
      Serial.print("Sending Fuel: ");
      Serial.println(Fuel_Pump_OK_data[0]);
      send_CAN_msg(0x66, 0, 1, Fuel_Pump_OK_data);
      message_num++;
      break;
    
    case 2:
      Serial.print("Sending FAN: ");
      Serial.println(Fan_OK_data[0]);
      send_CAN_msg(0x77, 0, 1, Fan_OK_data);
      message_num++;
      break;

    case 3:
      Serial.print("Sending WheelSpeed: ");
      Serial.println(Wheel_Speed_data[1]);
      send_CAN_msg(0x88, 0, 1, Wheel_Speed_data);
      message_num = 0;
      break;

    default:
      //reset message number if issue is encountered.
      message_num = 0;
  }

  can_timeold = millis();
}
