int sensorPin = 2; //Pin used to read digital output from sensor
volatile int pulseCounter = 0; //Variable to count the pulses

void pulseInterrupt() { pulseCounter++; }

void setup() {
  pinMode(sensorPin, INPUT);

  attachInterrupt(digitalPinToInterrupt(2),pulseInterrupt,FALLING);
  Serial.begin(9600);
}

void loop() {
  Serial.print("sensor Val: ");
  Serial.println(digitalRead(sensorPin));

  if(pulseCounter > 0) {
    Serial.println("Pulse Counter: ");
    Serial.println(pulseCounter);
  }
}
