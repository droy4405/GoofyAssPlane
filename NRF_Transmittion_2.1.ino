#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9,10); // CE, CSN

#define VRX_PIN  A5 // Arduino pin connected to VRX pin
#define VRY_PIN  A4 // Arduino pin connected to VRY pin

int xValueR = 0; // To store value of the X axis
int yValueR = 0; // To store value of the Y axis

int x_coord_R;
int y_coord_R;

const byte address[6] = "00001";

void setup() {
  radio.begin();
  radio.openWritingPipe(address);
  //radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
}

void loop() {

  xValueR = analogRead(VRX_PIN);
  yValueR = analogRead(VRY_PIN);

  x_coord_R = map(xValueR, 0, 1023, -90, 90);
  y_coord_R = map(yValueR, 0, 1023, -90, 90);

  radio.write(&x_coord_R, sizeof(x_coord_R));
  //isChipConnected();
  delay(70);
}