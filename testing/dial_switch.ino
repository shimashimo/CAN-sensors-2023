/*
  Modified code of ReadAnalogVoltage example and dial_switch_case_concept.md

  Kinda jank but enough for testing the dial switches
*/
int dial_steps = 12;
int prev_value = 0;
int voltage_boundary = (int)(1024/dial_steps - 2);  // 1024 / 12 = 85.33 (the interval with which the voltage increases -> [i * 85v] )
int dial_pin;
#define ROTARY_SWITCH_PIN A0

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

// the loop routine runs over and over again forever:
void loop() {
  // read the input on analog pin 0:
  int sensorValue = analogRead(A0);
  // Analog reading goes from 0 - 1023

  /*Can probably do this better but idk rn*/
  // To reduce redundant checks
  if(sensorValue != prev_value){
    // Reversing as voltage is greatest at position 1
    for(int i=12; i*voltage_boundary > 0; i--) {
      if(sensorValue >= (i*voltage_boundary)){
        dial_pin = 13-i;
        break;
      }
    }
  }
  Serial.println(dial_pin); // Print

  switch (dial_pin){
    case 1:
      Serial.println("Race Mode");
      break;
      
    case 2:
      Serial.println("ECO Mode");
      break;
      
    case 3:
      Serial.println("3rd Mode");
      break;
    case 4:
      Serial.println("4th Mode");
      break;
    default:
      Serial.println("Default");
  }
  delay(200); //Just to slow down output prints
}
