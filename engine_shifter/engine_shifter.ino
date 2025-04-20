#define ENCODER_OPTIMIZE_INTERRUPTS // For optimized interrupts in encoder
#include <Encoder.h>
#include <Servo.h>
#include <PID_v1.h>
const int PWM_PIN = 44;  // The PWM output pin to the SPARK MAX
const int SHIFT_UP_PIN = 49;
const int SHIFT_DOWN_PIN = 46;
const int SHIFT_NEUTRAL_PIN = 48;
const int MOTOR_ON_PIN = 47;

// Previous button states
bool prevShiftUpState = HIGH;
bool prevShiftDownState = HIGH;
bool prevNeutralState = HIGH;
bool prevMotorOnState = HIGH;


const int UP_SHIFT_SETPOINT = 600;
const int DOWN_SHIFT_SETPOINT = -600;
const int NEUTRAL_SETPOINT = 0;

// Define the pins connected to Channel A and B of the encoder
Encoder myEnc(2, 3);  // Replace with your actual pin numbers
Servo sparkmax;

// Define Variables we'll be connecting to for PID
double Setpoint, Input, Output;
const double Kp = 2.3, Ki = 0.07, Kd = 0.005;

// Specify the links and initial tuning parameters for PID
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

double oldPosition = -99;

void setup() {
  Serial.begin(9600);
  pinMode(PWM_PIN, OUTPUT);
  sparkmax.attach(PWM_PIN);

  // Need setpoints for Forward, Neutral and Reverse
  Setpoint = 0.0;  // Set desired position for the motor

  myPID.SetOutputLimits(-5000.0, 5000.0);
  myPID.SetMode(AUTOMATIC);

  // Shifter Paddles
  pinMode(SHIFT_UP_PIN, INPUT_PULLUP);
  pinMode(SHIFT_DOWN_PIN, INPUT_PULLUP);
  pinMode(SHIFT_NEUTRAL_PIN, INPUT_PULLUP);
  pinMode(MOTOR_ON_PIN, INPUT_PULLUP);

  attachPinChangeInterrupt(digitalPinToPCINT(SHIFT_UP_PIN), shiftUpISR, CHANGE);
  attachPinChangeInterrupt(digitalPinToPCINT(SHIFT_DOWN_PIN), shiftDownISR, CHANGE);
  attachPinChangeInterrupt(digitalPinToPCINT(SHIFT_NEUTRAL_PIN), neutralISR, CHANGE);
  attachPinChangeInterrupt(digitalPinToPCINT(MOTOR_ON_PIN), motorOnISR, CHANGE);
}

// Reverse Mode increases encoder value
//  

void loop() {
  // Update PID input with the current encoder position
  Input = myEnc.read();
  
  // bool shiftUpState = digitalRead(SHIFT_UP_PIN);
  // bool shiftDownState = digitalRead(SHIFT_DOWN_PIN);
  // bool neutralState = digitalRead(SHIFT_NEUTRAL_PIN);
  // bool motorOnState = digitalRead(MOTOR_ON_PIN);

  // if (Input != oldPosition) {
  //   oldPosition = Input;
  //   Serial.print("Encoder Position: ");
  //   Serial.println(Input);
  // }

  // Compute PID output
  myPID.Compute();
  Serial.print("Output Value: ");
  Serial.println(Output);
  // Output is distance from setpoint - i.e error


  if (shiftUpState == LOW && prevShiftUpState == HIGH) {
    Serial.println("Shift Up Pressed");
    Setpoint = UP_SHIFT_SETPOINT;
  }
  if (shiftDownState == LOW && prevShiftDownState == HIGH) {
    Serial.println("Shift Down Pressed");
    Setpoint = DOWN_SHIFT_SETPOINT;
  }

  if (neutralState == LOW && prevNeutralState == HIGH) {
    Serial.println("Neutral Pressed");
    Setpoint = NEUTRAL_SETPOINT;
  }

  // Update previous button states
  prevShiftUpState = shiftUpState;
  prevShiftDownState = shiftDownState;
  prevNeutralState = neutralState;
  prevMotorOnState = motorOnState;

  delay(20);  // Button debounce


  PID_Control();
  Setpoint = NEUTRAL_SETPOINT;
  PID_Control();

}

void PID_Control() {
  unsigned long neutralStartTime = 0;  // Time when we first entered neutral state
  bool inNeutralState = false;  // Track if we're in neutral state
  const unsigned long NEUTRAL_TIMEOUT = 200;  // Time in milliseconds to wait before returning
  
  while (1) {
    Input = myEnc.read();
    myPID.Compute();
    Serial.print("Output Value: ");
    Serial.println(Output);

    if(Output < -700) { // Error is large = position - setpoint. Need to increase encoder value, which is reversing motor
      // Full Reverse: p <= 1000
      Serial.println("FULL REVERSE");
      sparkmax.writeMicroseconds(1000);
      inNeutralState = false;
    } 
    else if(Output >= -700 && Output <= -10) {
      // Proportional Reverse: 1000 < p < 1475
      Serial.println("PROP REVERSE");
      int speed = map(Output, -10, -700, 1450, 1100);
      sparkmax.writeMicroseconds(speed);
      inNeutralState = false;
    } 
    else if(Output > 700) { 
      // Full Forward: p >= 2000
      Serial.println("FULL FORWARD");
      sparkmax.writeMicroseconds(2500);
      inNeutralState = false;
    }
    else if(Output >= 10 && Output <= 700) {
      Serial.println("PROP FORWARD");
      // Proportional Forward: 1525 < p < 2000
      int speed = map(Output, 700, 10, 1900, 1550);
      sparkmax.writeMicroseconds(speed);
      inNeutralState = false;
    }
    // else if (Output == 0) {
    else if(Output > -10 && Output < 10) { // Set Neutral
    // else {
      // Neutral: 1475 <= p <= 1525
      Serial.println("Neutral");
      sparkmax.writeMicroseconds(1500);
      if (!inNeutralState) {
        // First time entering neutral state, record the time
        neutralStartTime = millis();
        inNeutralState = true;
      }
      else if ((millis() - neutralStartTime) >= NEUTRAL_TIMEOUT) {
        // We've been in neutral state for the required time
        Serial.println("Neutral timeout reached, returning");
        return;
      }
    }
  }
}