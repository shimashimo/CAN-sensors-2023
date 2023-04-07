// CAN sensor encoding test

#include <SPI.h>
#include "mcp2515_can.h"
#include "mcp2515_can_dfs.h"

const int SPI_CS_PIN = 9;
const int CAN_INT_PIN = 2;

const int trigPin = 5;
const int echoPin = 6;
long duration;
int distance;

// --- Function Declaration ---
void init_CAN();
void ping();
// ----------------------------

mcp2515_can CAN(SPI_CS_PIN); // set CS to pin 9

void setup() {
  Serial.begin(115200);
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  init_CAN();

  Serial.println("\n-- SEND --\n");
}

// test data to transmit onto CAN network
unsigned char data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
char msgString[128];

void loop() {
  ping();

  // Assign
  data[0] = (distance & 0xff);
  data[1] = ((distance >> 2) & 0xff);

  sprintf(msgString,"message: 0x%.2X  0x%.2X", data[0], data[1]);
  Serial.println(msgString);


  unsigned char send_status = CAN.sendMsgBuf(0x1e, 0, 8, data);
  if (send_status == MCP2515_OK) 
  {
    Serial.println("Message Sent Successfully!");
  } else {Serial.println("message Error...");}

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

void ping()
{
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Sets the trigPin on HIGH state for 10 micro-sec
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, stores the sound wave travel time in micro-sec to dur.
  duration = pulseIn(echoPin, HIGH);

  // Calculate the distance
  distance = (duration * 0.034) / 2;

  Serial.print("Distance: ");
  Serial.println(distance);
}