#include <SPI.h>
#include "mcp2515_can.h"
#include "mcp2515_can_dfs.h"
#include "JC_Button.h"  // library for input buttons
#include "led_tachometer.h" // For RPM light strip
#include "US2066.h"
#include "dial_switch.h"

typedef unsigned char byte;

// -> CAN message vars
#define SPI_CS_PIN 9 // CAN specific pins
#define CAN_INT_PIN 2 // CAN interrupt pin
unsigned long canId = 0x00; // CAN message ID
byte msg_length = 0; // length of message from CAN
byte msg_buffer[8]; // buffer for storing message from CAN
#define  CAN_MSG_DELAY 100
unsigned long can_timeold;
int can_delay_cycle;
int message_num = 0;

// Global Variables and Constants
// Data containers for menu screen data
unsigned char wheelSpeed[8];
unsigned char engineTemp[8];
unsigned char stateOfCharge[8];
//...and more as needed

// TODO: The defines are used for the pin number for the associated
// sensor/button/component is attached to onn the canduino.
// the byte variables are used for encoding and passing CAN messages,
// If a sensor/button/component needs to send a CAN message, it
// will need a byte variable. Define any other constants or variables 
// needed that can be defined outside of a function.

// -> OLED Display Vars
US2066 OLED;
const int DISPLAY_ROWS = 4;
const int DISPLAY_ROWS = 20;
const int PRINT_COL = 9;

// -> Dial Switch Vars
const int DRIVE_DIAL_PIN = A0;
const int MENU_DIAL_PIN = A1;
Dial_Switch drive_dial(DRIVE_DIAL_PIN);
Dial_Switch menu_dial(MENU_DIAL_PIN);

// -> LED Tachometer vars
// LED PIN IS DEFINED IN led_tachometer.h AS IT NEEDS TO BE A CONST EXPRESSION
#define BRIGHTNESS 5
#define GREEN_SEC 4
#define YELLOW_SEC 0
LED_Tachometer tachometer(BRIGHTNESS, GREEN_SEC, YELLOW_SEC);

// -> shift button vars
#define SHIFT_UP_BUTTON_PIN = 2;
#define SHIFT_DOWN_BUTTON_PIN = 3;
bool shift_up_flag = false;
bool shift_down_flag = false;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 200;

/*************** GLOBAL CAN VARS AND CONTAINERS ****************************/
// CAN IDs are randomly made, change as needed
byte Brake_OK_data[1]     = {0x00};   const byte BRAKE_CAN_ID = 0x55; 
byte Fuel_Pump_OK_data[1] = {0x00};   const byte FUEL_CAN_ID  = 0x66;
byte Fan_OK_data[1]       = {0x00};   const byte FAN_CAN_ID   = 0x77;
byte Wheel_Speed_data[1]  = {0x00};   const byte WSS_CAN_ID   = 0x88;
byte Shift_Up_data[1]     = {0x00};   const byte SHIFT_UP_CAN_ID = 0x99;
byte Shift_Down_data[1]   = {0x00};   const byte SHIFT_DOWN_CAN_ID = 0x98;
/***************************************************************************/

// -> engine start vars
#define ENGINE_START_BUTTON_PIN ##
Button engine_start_button(ENGINE_START_BUTTON_PIN, 100, true, true); //btn(pin, debounceTime, pullupEnabled, logicInvert)

// -> motor start vars
#define MOTOR_START_BUTTON_PIN ##
Button motor_start_button(MOTOR_START_BUTTON_PIN, 100, true, true);

// -> shifting functionality vars
const unsigned long SHIFT_UP_CAN_ID = 0x99; // shift message CAN ID
const unsigned long SHIFT_DOWN_CAN_ID = 0x98; // shift message CAN ID

// -> throttle functionality vars
unsigned long aps_CAN_ID = 0x02; // accelerator position sensor CAN ID

// Function Declarations
void init_CAN();
void send_CAN_msg(unsigned long id, byte ext, byte len, const byte * msg_buf);
void message_cycle();
void CAN_message_handler();

// TODO: for any functions you write at the base of the file, define the
// signature of the function here for everything to compile and be happy
void shift();
void CAN_read_from_network();
void switch_data_store();
void shift_up();
void shift_down();
void debounce();
void menu_switch_case(US2066 OLED, int menu_dial_pos);
void drive_switch_case(int dial_position);

/********** Setup/Initialization ***************/

mcp2515_can CAN(SPI_CS_PIN); // set CS to pin 9

void setup() {
  // set the speed of the serial port
  Serial.begin(115200);

  init_CAN();
  can_timeold = 0;

  // TODO: Use setup for initializing the state of any variables
  // or initializing pinmodes/pin read or write. Anything that needs setup

  // engine and motor button setups
  engine_start_button.begin();
  motor_start_button.begin();

  // shift button setups
  pinMode( SHIFT_UP_BUTTON_PIN, INPUT_PULLUP );
  attachInterrupt( digitalPinToInterrupt(SHIFT_UP_BUTTON_PIN), shift_up, FALLING ); // Sending signal on press, not release for some reason.
  pinMode( SHIFT_DOWN_BUTTON_PIN, INPUT_PULLUP );
  attachInterrupt( digitalPinToInterrupt(SHIFT_DOWN_BUTTON_PIN), shift_down, FALLING ); // Sending signal on press, not release for some reason.
  
  // LED strip setups
  tachometer.begin();
}

/********** Main Loop ***************/

void loop() {
  CAN_message_handler();

  can_delay_cycle = millis() - can_timeold;
  if (can_delay_cycle >= CAN_MSG_DELAY) message_cycle();

  /* Engine/Motor Start Buttons*/
  engine_start_button.read();

  // send CAN message if start engine button has been pushed
  if(engine_start_button.wasReleased()) //returns true on falling edge, when there's a change in read() value
  {
    // send engine start CAN message
    // send_CAN_msg(); //TODO: set up messageID, 
  }
  // send CAN message if start motor button has been pushed
  if(motor_start_button.wasReleased())
  {
    // send motor start CAN message
    // send_CAN_msg(); //TODO: set up messageID, 
  }

  /*Shift messages*/
  // TODO: Implement rpm checking to know if shift up should occur (is rpm > Min_value_rpm)
  if(shift_up_flag)
  {
    Serial.println("INTERRUPT SHIFTING");
    Shift_Up_data[0] = 0x01;
    send_CAN_msg(0x99, 0, 1, Shift_Up_data);
    shift_up_flag = false;
    Shift_up_date[0] = 0x00;
  }
  if(shift_down_flag)
  {
    // TODO: Implement rpm checking to know if shift down should occur (is rpm < Max_value_rpm)
    Serial.println("INTERRUPT SHIFTING");
    Shift_Down_data[0] = 0x01;
    send_CAN_msg(0x98, 0, 1, Shift_Down_data);
    shift_down_flag = false;
    Shift_Down_date[0] = 0x00;
  }

  // Read from ADC to get dial position
  drive_dial.read_adc();
  menu_dial.read_adc();
  // Todo: Error checking for dial position

  // Drive Mode
  drive_switch_case(drive_dial.get_dial_pos());

  if(menu_dial.check_change()) {
    OLED.clear();
    menu_dial.reset_change();
  }
  menu_switch_case(OLED, menu_dial.get_dial_pos())
  
  tachometer.setLEDs( RPM );
  FastLED.show();
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
  Function: The message_read() function gets the CAN ID of the current message on the
            CAN network and routes the program to the appropriate handler.
*/
void CAN_message_handler()
{
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
    // Serial.print("\nCandID: ");
    // Serial.println(canId);
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
void switch_data_store(unsigned long CAN_ID, char*& data_container)
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
/*
  Function: Use the data containers being sent for various sensors to display information to the dash screen
  TODO: Replace DataContainer with appropriate data container to display data
*/
void menu_switch_case(US2066 OLED, int menu_dial_pos) {
  switch (dial_position) { 
    case 1: // Main Display
      // det_drive_mode(DriveModeData); /** Determine drive mode then update screen **/
      char SpeedBuf[8];
      char RPMbuf[8];
      char TempBuf[8]; 
      OLED.print(0, 0, "Speed:");       // print(StartingRow, Starting Column, StringToDisplay)
      itoa(DataContainer, SpeedBuf, 10);   // Requires itoa() as the oled print function uses String type
      OLED.print(0, PRINT_COL, SpeedBuf);

      OLED.print(1, 0, "SOCa: ");
      itoa(DataContainer, RPMbuf, 10);       // Replace DataCont
      OLED.print(1, PRINT_COL, RPMbuf);
      
      OLED.print(2, 0, "Drive: ");

      OLED.print(3, 0, "ENGTemp:");
      itoa(DataContainer, TempBuf, 10);
      OLED.print(3,PRINT_COL, TempBuf);
      break;
      
    case 2: // Battery Status
      OLED.print(0, 0, "SOC: ");
      itoa(wheelSpeed)
      OLED.print(1, 0, " A");
      OLED.print(2, 0, " A")
      break;
      
    case 3: // Accumulator Cell Voltages?
      char CellBuf[24]; // I think there were 24 cells?
      for(int i=0; i < 24; i++) { // Change the cell data to strings
        // itoa(CellDataContainer[i], CellBuf[i]);
      }
      OLED.print(0, 0, CellBuff[0]);
      break;

    case 4: // Acuumulator Temperature
      Serial.println("Accumulator Temperatures");
      break;

    case 5: // Brake Pressure
      Serial.println("Brake Pressure");
      break;

    case 6: // Suspension Travel
      Serial.println("Suspension Travel");
      break;

    default: // Error if getting here
      Serial.println("Default");
      break;
  }
}

/*
  Function to send can msg to put car into various drive modes, from dial input
*/
void drive_switch_case(int dial_position) {
  /* IMPLEMENT DRIVE MODE STUFF HERE */
  switch (dial_position){
    case 1: // Drive Mode
      Serial.println("Drive Mode");
      // Send CAN msg to put into drive mode?
      break;
      
    case 2: // Eco Mode
      Serial.println("ECO Mode");
      // Send CAN msg
      break;
      
    case 3:
      Serial.println("3rd Mode");
      break;

    case 4:
      Serial.println("4th Mode");
      break;

    default:
      Serial.println("Default");
      break;
  }
}

/*
  Function: The shift_up function is an ISR that sends a CAN message from the steering wheel canduino,
            to the mid canduino to shift the gear up.
*/
void shift_up()
{
  if(debounce()) shift_up_flag = true;
}

/*
  Function: The shift_up function is an ISR that sends a CAN message from the steering wheel canduino,
            to the mid canduino to shift the gear down.
*/
void shift_down()
{
  if(debounce()) shift_down_flag = true;
}

/*
  Function: The debounce() function debounces the buttons for the shift up and down ISR functions. 
            The ISRs are still called on each "bounce", but only when debounced time has passed will it complete an action.
*/
bool debounce()
{
  // check to see if enough time has passed
  // since the last signal to ignore any bounce:
  if ((millis() - lastDebounceTime) > bounceDelay) {
    lastDebounceTime = millis();
    return true;
  }
  return false;
}