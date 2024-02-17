/*
* this code is responsible for handling the RC transmitter module
* the program reads the input from two joysticks and send it 
* through the NRF transmission module
*
* !!! this program will not be responsible for receving signals !!!
*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>
#include <printf.h>

// lez go bois

#define Left_X A2
#define Left_Y A1
#define Right_X A3
#define Right_y A4

// note that the range of analog input form the joystick
// is 0 - 1024, therefore the middle point is around (512,512)
// im so proud of myself for figuring this out
#define JoStick_Upper_TH 91
#define JoStick_Lower_TH 89

// initiallizing the CE and CSN pin for RC transmission
RF24 radio(7, 8); // CE, CSN
const byte address[][6] = {"00001","00002","00003","00004"};
/*
* Channel Look Up Table:
* 00001 address[0] --> Xpos_Left
* 00002 address[1] --> Ypos_Left
* 00003 address[2] --> Xpos_Right
* 00004 address[3] --> Ypos_Right
*/

// note that these variables will be in degrees
int Xpos_Left;
int Ypos_Left;
int Xpos_Right;
int Ypos_Right;

void setup() {
  radio.begin();
  radio.openWritingPipe(address[0]); // open transmitting pipe 00001
  radio.openWritingPipe(address[1]); 
  // open second transmitting pipe for the
  // second joystick

  // setting the power amplifier level, set to low if the range is close
  // however whem set to high later make sure to have a bypass capacitor
  radio.setPALevel(RF24_PA_MIN);


  // setting up the joy stick inputs
  // pinMode(Left_X, INPUT);
  // pinMode(Left_Y, INPUT);
  // pinMode(Right_X, INPUT);
  // pinMOde(Right_Y, OUTPUT);

}

void loop() {
  // initiallizing the NRF module as a transmitter
  radio.stopListening();

  // NOTE THAT ALL THE READ VALUES ARE CONVETED INTO 180 DEGREES RANGES
  // reading the value of the left hand joystick
  Xpos_Left = map(analogRead(Left_X), 0, 1023, 0, 180);
  Ypos_Left = map(analogRead(Left_Y), 0, 1023, 0, 180);
  // reading the value of the rght hand joy stick
  Xpos_Right = map(analogRead(Right_X),0 ,1023, 0, 180);
  Ypos_Right = map(analogRead(Right_y),0 , 1023, 0, 180);

  // checking if the joysticks pass a threshold value so we dont end
  // up sending  a lot of garbage values around 90 degrees
  if (Xpos_Left > JoStick_Upper_TH || Xpos_Left < JoStick_Lower_TH){
    // sending the Xpos_left though the NRF
    // selecting channel 1 for transmitting Xpos_Left
    radio.setChannel(address[0]);
    radio.write(&Xpos_Left, sizeof(Xpos_Left));
  }

  // checking the value of Ypos_Left
  if (Ypos_Left > JoStick_Upper_TH || Ypos_Left < JoStick_Lower_TH){
    // sending the Xpos_left though the NRF
    // selecting channel 1 for transmitting Xpos_Left
    radio.setChannel(address[1]);
    radio.write(&Ypos_Left, sizeof(Ypos_Left));
  }

  if (Xpos_Right > JoStick_Upper_TH || Xpos_Right < JoStick_Lower_TH){
    // sending the Xpos_left though the NRF
    // selecting channel 1 for transmitting Xpos_Left
    radio.setChannel(address[2]);
    radio.write(&Xpos_Right, sizeof(Xpos_Right));
  }

  if (Ypos_Right > JoStick_Upper_TH || Ypos_Right < JoStick_Lower_TH){
    // sending the Xpos_left though the NRF
    // selecting channel 1 for transmitting Xpos_Left
    radio.setChannel(address[0]);
    radio.write(&Ypos_Right, sizeof(Ypos_Right));
  }

}

