#include <Arduino.h>

void setup() {
  Serial.begin(9600);
  Serial.println("TX Setup completed \n");
}

void loop() {
  static unsigned long counter = 0;
  Serial.print("Counter = \n");
  Serial.println(++counter);
  delay(1000);
}
