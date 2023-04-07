// CAN Receive Testing

#include <SPI.h>
#include "mcp2515_can.h"
#include "mcp2515_can_dfs.h"

long unsigned int rxId; // message ID
unsigned char len = 0; // message Length
unsigned char rxBuf[8]; // buffer for storing message
char msgString[128];

const int CAN_INT_PIN = 2;
const int SPI_CS_PIN = 9;

// Function Definitions
void init_CAN();
mcp2515_can CAN(SPI_CS_PIN); // set CS to pin 9

void setup() 
{
  Serial.begin(115200);
  init_CAN();

  Serial.println("\n-- RECEIVE --\n");
}

void loop() 
{  
  // check if we are able to see messages on the CAN network.
  if (CAN_MSGAVAIL == CAN.checkReceive())
  {
    unsigned long canId = CAN.getCanId();

    Serial.println("-----------------------------");
    Serial.print("get data from ID: 0x");
    Serial.println(canId, HEX);
    Serial.println();
    
    CAN.readMsgBuf(&len, rxBuf);

    Serial.print("message: ");

    for (byte i=0;i<len;i++) { 
      sprintf(msgString," 0x%.2X",rxBuf[i]); 
      Serial.print(msgString);
      Serial.print("  ");

    }
    Serial.println();
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
