#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9,10); // CE, CSN

const byte address[5] = "00001";

void setup() {
  radio.begin();
  radio.openWritingPipe(address);
  //radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
}

void loop() {
  const char text[] = "Im alive!\n";
  radio.write(&text, sizeof(text));
  //isChipConnected();
  delay(1000);
}