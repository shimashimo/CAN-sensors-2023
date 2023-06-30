// CAN Send Testing

#include <SPI.h>
#include "mcp2515_can.h"
#include "mcp2515_can_dfs.h"

const int SPI_CS_PIN = 9;
const int CAN_INT_PIN = 2;

// Function Declaration
void init_CAN();
mcp2515_can CAN(SPI_CS_PIN); // set CS to pin 9

void setup() {
  Serial.begin(115200);
  init_CAN();

  Serial.println("\n-- SEND --\n");
}

// test data to transmit onto CAN network
unsigned char data[8] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x5d, 0x06, 0x07};

void loop() {
  unsigned char send_status = CAN.sendMsgBuf(0x1e, 0, 8, data);
  if (send_status == MCP2515_OK) 
  {
    Serial.println("Message Sent Successfully!");
  } else Serial.println("uh oh...");

  delay(100);
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