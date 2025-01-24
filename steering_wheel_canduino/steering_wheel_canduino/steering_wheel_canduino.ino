/* steering_wheel_canduino_fixed.ino */

#include <SPI.h>
#include <Wire.h>
#include "mcp2515_can.h"
#include "mcp2515_can_dfs.h"
#include "JC_Button.h"       
#include "led_tachometer.h"  
#include "US2066.h"          
#include "dial_switch.h"     

// --------------------- Constants & Globals -----------------------
typedef unsigned char byte;

// CAN
#define SPI_CS_PIN     9
#define CAN_INT_PIN    2
mcp2515_can CAN(SPI_CS_PIN);

bool canActive = false;
unsigned long canId        = 0;
byte          msg_length   = 0;
byte          msg_buffer[8];
#define       CAN_MSG_DELAY 100
unsigned long can_timeold;
int           can_delay_cycle;
int           message_num  = 0;

// Example variables / placeholders
int  RPM = 0;  

byte Brake_OK_data[1]     = {0x00};  const unsigned long BRAKE_CAN_ID      = 0x55; 
byte Fuel_Pump_OK_data[1] = {0x00};  const unsigned long FUEL_CAN_ID       = 0x66;
byte Fan_OK_data[1]       = {0x00};  const unsigned long FAN_CAN_ID        = 0x77;
byte Wheel_Speed_data[1]  = {0x00};  const unsigned long WSS_CAN_ID        = 0x88;
byte Shift_Up_data[1]     = {0x00};  const unsigned long SHIFT_UP_CAN_ID   = 0x99;
byte Shift_Down_data[1]   = {0x00};  const unsigned long SHIFT_DOWN_CAN_ID = 0x98;

// Display
US2066 OLED;
const int DISPLAY_ROWS = 4;
const int DISPLAY_COLS = 20;

// Dial pins
const int DRIVE_DIAL_PIN = A0;
const int MENU_DIAL_PIN  = A1;

// Create Dial_Switch objects
Dial_Switch drive_dial(DRIVE_DIAL_PIN);
Dial_Switch menu_dial(MENU_DIAL_PIN);

// Tachometer
#define BRIGHTNESS 5
#define GREEN_SEC  4
#define YELLOW_SEC 0
LED_Tachometer tachometer(BRIGHTNESS, GREEN_SEC, YELLOW_SEC);

// Shift buttons
#define SHIFT_UP_BUTTON_PIN   2
#define SHIFT_DOWN_BUTTON_PIN 3
volatile bool shift_up_flag   = false;
volatile bool shift_down_flag = false;
unsigned long lastDebounceTime= 0;
unsigned long debounceDelay   = 200;

// Engine & Motor start
#define ENGINE_START_BUTTON_PIN 7
#define MOTOR_START_BUTTON_PIN  8
Button engine_start_button(ENGINE_START_BUTTON_PIN, 100, true, true);
Button motor_start_button(MOTOR_START_BUTTON_PIN, 100, true, true);

// Example
int globalGearNumber = 1;

// --------------------- Declarations -----------------------
void try_init_CAN();
void send_CAN_msg(unsigned long id, byte ext, byte len, const byte *msg_buf);
void CAN_message_handler();
void CAN_read_from_network();
void switch_data_store(unsigned long CAN_ID, unsigned char *&data_container);
void message_cycle();

bool debounce();
void shift_up();
void shift_down();

void drive_switch_case(int dial_position);
void menu_switch_case(US2066 &oled, int dial_position);

void diagnoseDisplay();

// -------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("Starting setup...");

  // I2C/Display setup with checks
  Wire.begin();
  Wire.setClock(100000); // 100kHz for stability
  
  Serial.println("Initializing OLED...");
  
  // Initialize display
  OLED.init();
  delay(100);  // Power-up delay
  
  // Basic settings
  OLED.clear();
  delay(50);
  
  // Set contrast to mid-range
  OLED.contrast(80);
  delay(50);
  
  Wire.beginTransmission(0x3C);
  byte error = Wire.endTransmission();
  
  if (error == 0) {
    Serial.println("OLED found at 0x3C");
    
    // Welcome sequence with centered text
    OLED.clear();
    delay(100);
    
    // Center "UVic Hybrid 2025" on first line
    // Display is 20 chars wide, text is 15 chars, so start at position 2
    OLED.cursor(0, 2);
    OLED.print("UVic Hybrid 2025");
    delay(2000);
    
    OLED.clear();
    delay(100);
    
    // Center "Starting" on first line (8 chars)
    // Start at position 6 for centering
    OLED.cursor(0, 6);
    OLED.print("Starting");
    delay(1000);
    OLED.print(".");
    delay(1000);
    OLED.print(".");
    delay(1000);
    OLED.print(".");
    delay(1000);
    
  } else {
    Serial.println("OLED not found! Error: ");
    Serial.println(error);
  }
  
  OLED.clear();
  delay(100);

  // Rest of your setup code...
  try_init_CAN();
  can_timeold = 0;

  // Buttons
  engine_start_button.begin();
  motor_start_button.begin();

  // Tachometer
  tachometer.begin();
}

// Add a display update delay
unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_UPDATE_DELAY = 250; // 250ms between updates

void loop() {
  unsigned long currentMillis = millis();
  
  // Check for serial commands
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    
    // Display diagnostic command
    if (cmd == 'd' || cmd == 'D') {
      Serial.println("Running display diagnostic...");
      diagnoseDisplay();
    }
    // Menu selection commands
    else if (cmd >= '1' && cmd <= '6') {
      int menuChoice = cmd - '0';  // Convert char to int
      Serial.print("Switching to menu ");
      Serial.println(menuChoice);
      
      OLED.clear();
      delay(1);
      menu_switch_case(OLED, menuChoice);
    }
    // Shift commands
    else if (cmd == 'u' || cmd == 'U') {
      Serial.println("SHIFTING UP");
      Shift_Up_data[0] = 0x01;
      if (canActive) send_CAN_msg(SHIFT_UP_CAN_ID, 0, 1, Shift_Up_data);
      Shift_Up_data[0] = 0x00;
    }
    else if (cmd == 'n' || cmd == 'N') {
      Serial.println("SHIFTING DOWN");
      Shift_Down_data[0] = 0x01;
      if (canActive) send_CAN_msg(SHIFT_DOWN_CAN_ID, 0, 1, Shift_Down_data);
      Shift_Down_data[0] = 0x00;
    }
  }

  // Handle CAN if active
  if (canActive) {
    CAN_message_handler();
    can_delay_cycle = millis() - can_timeold;
    if (can_delay_cycle >= CAN_MSG_DELAY) {
      message_cycle();
    }
  }

  // Check engine/motor start
  engine_start_button.read();
  if (engine_start_button.wasReleased()) {
    Serial.println("Engine Start Button Pressed");
    // ...
  }
  motor_start_button.read();
  if (motor_start_button.wasReleased()) {
    Serial.println("Motor Start Button Pressed");
    // ...
  }

  // Potentiometer handling with rate limiting
  if (currentMillis - lastDisplayUpdate >= DISPLAY_UPDATE_DELAY) {
    // Read potentiometer values
    drive_dial.read_adc();
    menu_dial.read_adc();

    // Get current menu position
    int currentMenuPos = menu_dial.get_dial_pos();
    static int lastMenuPos = -1;  // Track last position

    // Debug output
    Serial.print("Current Menu Position: ");
    Serial.print(currentMenuPos);
    Serial.print(" Last Menu Position: ");
    Serial.println(lastMenuPos);

    // Update display if position changed
    if (currentMenuPos != lastMenuPos) {
        OLED.clear();
        delay(50);  // Give display time to clear
        menu_switch_case(OLED, currentMenuPos);
        lastMenuPos = currentMenuPos;
        delay(50);  // Give display time to update
    }

    lastDisplayUpdate = currentMillis;
  }

  // Update tachometer
  tachometer.setLEDs(RPM);
  FastLED.show();
}

// -------------------------------------------------------------------
// Attempt CAN init once
void try_init_CAN() {
  if (CAN_OK == CAN.begin(CAN_500KBPS)) {
    canActive = true;
    Serial.println("CAN init OK!");
  } else {
    canActive = false;
    Serial.println("No CAN module found, skipping CAN code.");
  }
}

// -------------------------------------------------------------------
// SHIFT / Debounce
void shift_up() {
  if (debounce()) {
    Serial.println("INTERRUPT SHIFTING UP");
    shift_up_flag = true;
  }
}

void shift_down() {
  if (debounce()) {
    Serial.println("INTERRUPT SHIFTING DOWN");
    shift_down_flag = true;
  }
}

bool debounce() {
  if ((millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis();
    return true;
  }
  return false;
}

// -------------------------------------------------------------------
void drive_switch_case(int dial_position) {
  switch (dial_position) {
    case 1:
      Serial.println("Drive Mode 1");
      globalGearNumber = 1;
      break;
    case 2:
      Serial.println("Drive Mode 2");
      globalGearNumber = 2;
      break;
    case 3:
      Serial.println("Drive Mode 3");
      globalGearNumber = 3;
      break;
    case 4:
      Serial.println("Drive Mode 4");
      globalGearNumber = 4;
      break;
    case 5:
      Serial.println("Drive Mode 5");
      globalGearNumber = 5;
      break;
    case 6:
      Serial.println("Drive Mode 6");
      globalGearNumber = 6;
      break;
    default:
      break;
  }
}

// -------------------------------------------------------------------
void menu_switch_case(US2066 &oled, int dial_position) {
    oled.clear();
    delay(10);  // Ensure clear completes

    switch (dial_position) {
        case 1:
            oled.cursor(0, 0);
            oled.print("=== Main Display ===");
            delay(5);
            oled.cursor(1, 0);
            oled.print("Speed:");
            delay(5);
            oled.cursor(2, 0);
            oled.print("RPM:");
            delay(5);
            oled.cursor(3, 0);
            oled.print("Gear:");
            break;
        case 2:
            oled.cursor(0, 0);
            oled.print("=== Battery Status ===");
            delay(5);
            oled.cursor(1, 0);
            oled.print("Voltage: ");
            oled.cursor(1, 12);
            oled.print("Amps: ");
            delay(5);
            oled.cursor(2, 0);
            oled.print("Temp: ");
            oled.cursor(2, 12);
            oled.print("SOC: ");
            delay(5);
            oled.cursor(3, 0);
            oled.print("Health: ");
            oled.cursor(3, 12);
            oled.print("Cycles: ");
            break;
        case 3:
            oled.cursor(0, 0);
            oled.print("=== Brake & Pedals ===");
            delay(5);
            oled.cursor(1, 0);
            oled.print("Front: ");
            oled.cursor(1, 12);
            oled.print("Rear: ");
            delay(5);
            oled.cursor(2, 0);
            oled.print("Bias: ");
            oled.cursor(2, 12);
            oled.print("Press: ");
            delay(5);
            oled.cursor(3, 0);
            oled.print("Pedal: ");
            oled.cursor(3, 12);
            oled.print("Travel: ");
            break;
        case 4:
            oled.cursor(0, 0);
            oled.print("=== Suspension ===");
            delay(5);
            oled.cursor(1, 0);
            oled.print("FL: ");
            oled.cursor(1, 6);
            oled.print("FR: ");
            delay(5);
            oled.cursor(2, 0);
            oled.print("RL: ");
            oled.cursor(2, 6);
            oled.print("RR: ");
            break;
        case 5:
            oled.cursor(0, 0);
            oled.print("=== Accumulator ===");
            delay(5);
            oled.cursor(1, 0);
            oled.print("Temperature: ");
            delay(5);
            oled.cursor(2, 0);
            oled.print("GLV Status: ");
            break;
        case 6:
            oled.cursor(0, 0);
            oled.print("=== Cell Voltages ===");
            delay(5);
            oled.cursor(1, 0);
            oled.print("Cell 1: ");
            oled.cursor(1, 12);
            oled.print("Cell 2: ");
            delay(5);
            oled.cursor(2, 0);
            oled.print("Cell 3: ");
            oled.cursor(2, 12);
            oled.print("Cell 4: ");
            break;
        default:
            oled.cursor(0, 0);
            oled.print("Invalid Menu Option");
            delay(5);
            oled.cursor(1, 0);
            oled.print("Please select 1-6");
            break;
    }
}

// -------------------------------------------------------------------
// CAN Functions (will do nothing if canActive=false)
void CAN_message_handler() {
  if (CAN_MSGAVAIL == CAN.checkReceive()) {
    canId = CAN.getCanId();
    CAN.readMsgBuf(&msg_length, msg_buffer);

    switch (canId) {
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
      default:
        // unrecognized ID
        break;
    }

    // Debug
    Serial.println("=== Data Containers ===");
    Serial.print("Brake: ");      Serial.println(Brake_OK_data[0]);
    Serial.print("Fuel: ");       Serial.println(Fuel_Pump_OK_data[0]);
    Serial.print("Fan : ");       Serial.println(Fan_OK_data[0]);
    Serial.print("Wheel: ");      Serial.println(Wheel_Speed_data[0]);
    Serial.println();
  }
}

void CAN_read_from_network() {
  unsigned char *data_container = nullptr;
  switch_data_store(canId, data_container);
  if (!data_container) {
    Serial.println("Error: data_container=null");
    return;
  }
  for(int i = 0; i < msg_length; i++) {
    data_container[i] = msg_buffer[i];
  }
}

void switch_data_store(unsigned long CAN_ID, unsigned char *&data_container) {
  switch (CAN_ID) {
    case BRAKE_CAN_ID:
      data_container = Brake_OK_data;
      break;
    case FUEL_CAN_ID:
      data_container = Fuel_Pump_OK_data;
      break;
    case FAN_CAN_ID:
      data_container = Fan_OK_data;
      break;
    case WSS_CAN_ID:
      data_container = Wheel_Speed_data;
      break;
    default:
      data_container = nullptr;
      break;
  }
}

void message_cycle() {
  can_timeold = millis();
  byte exampleData[2];
  message_num++;
  exampleData[0] = (byte)(message_num & 0xFF);
  exampleData[1] = (byte)((message_num >> 8) & 0xFF);
  send_CAN_msg(0x123, 0, 2, exampleData);
}

void send_CAN_msg(unsigned long id, byte ext, byte len, const byte *msg_buf) {
  if (!canActive) return; 
  byte send_status = CAN.sendMsgBuf(id, ext, len, msg_buf);
  if (send_status == MCP2515_OK) {
    Serial.println("Message Sent Successfully!");
  } else {
    Serial.println("Message Error...");
  }
}

void diagnoseDisplay() {
  Serial.println("Starting OLED diagnostic...");
  
  // Check I2C connection
  Wire.beginTransmission(0x3C);
  byte error = Wire.endTransmission();
  
  Serial.print("I2C Status (0=OK): ");
  Serial.println(error);
  
  // Test display with different patterns
  OLED.clear();
  delay(10);
  
  // Test each line
  OLED.cursor(0, 0);
  OLED.print("Line 1 Test 123");
  delay(500);
  OLED.cursor(1, 0);
  OLED.print("Line 2 Test 456");
  delay(500);
  OLED.cursor(2, 0);
  OLED.print("Line 3 Test 789");
  delay(500);
  OLED.cursor(3, 0);
  OLED.print("Line 4 Test ABC");
  delay(500);
  
  // Test contrast levels
  for (int contrast = 0; contrast < 256; contrast += 64) {
    OLED.contrast(contrast);
    OLED.clear();
    OLED.cursor(0, 0);
    OLED.print("Contrast: ");
    Serial.print("Setting contrast to: ");
    Serial.println(contrast);
    delay(500);
  }
  
  // Reset to middle contrast
  OLED.contrast(127);
  
  // Final status
  OLED.clear();
  OLED.cursor(0, 0);
  OLED.print("Diagnostic Complete");
  Serial.println("OLED diagnostic complete");
}