#include <DHT11.h>
#include <IRremote.hpp>  // include the library

//Declare pin functions on RedBoard
#define stp 2
#define dir 3
#define MS1 4
#define MS2 5
#define EN 6

//declare button pins for up down
#define upBTN 12
#define downBTN 7

//Declare variables for functions
char user_input;
int x;
int y;
int state;
const unsigned long dT = 5;  // dT being an abbreviation of delayTime
DHT11 dht11(8);
int temperature = 0;
int humidity = 0;
int dhtResult;
int receiver = 10;
unsigned long prevMillis = 0;
const long interval = 60000;  // 60.000 ms == 1 minute

IRrecv irrecv(receiver);  // create instance of 'irrecv'
decode_results results;   // create instance of 'decode_results'


void setup() {
  Serial.begin(9600);  
  Serial.println("Begin motor control");
  Serial.println();
  pinMode(stp, OUTPUT);
  pinMode(dir, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(EN, OUTPUT);
  pinMode(upBTN, INPUT_PULLUP);
  pinMode(downBTN, INPUT_PULLUP);

  resetEDPins();

  IrReceiver.begin(10, ENABLE_LED_FEEDBACK);

  printActiveIRProtocols(&Serial);


  x = 0;
  y = 0;
}

void loop() {
  unsigned long currentMillis = millis();

  digitalWrite(EN, LOW);  //Pull enable pin low to allow motor control




  if (digitalRead(upBTN) == LOW) {
    move(LOW);  // move forward with the motor

    x++;
  } else {
    x = 0;
  }
  if (digitalRead(downBTN) == LOW) {
    move(HIGH);  // move in reverse with the motor

    y++;
  } else {
    y = 0;
  }

  if (currentMillis - prevMillis >= interval) {
    prevMillis = currentMillis;
    checkTemperature();
  }


  translateIR();


  resetEDPins();
}

void move(int dirState) {

  digitalWrite(dir, dirState);  // when dir is LOW, motor moves forward

  for (int i = 0; i < 10; i++) {
    digitalWrite(stp, HIGH);  //Trigger one step
    delay(dT);
    digitalWrite(stp, LOW);  //Pull step pin low so it can be triggered again
    delay(dT);
  }
}

void checkTemperature() {
  temperature = 0;
  humidity = 0;

  // Read the temperature and humidity via the dht11 module, the temperature and humidity are stored in the variables temperature and humidity. the read result is in dhtResult
  dhtResult = dht11.readTemperatureHumidity(temperature, humidity);

  // Check the readResult, if it is 0, reading is successfull
  if (dhtResult == 0) {

    if (temperature >= 30) {
      for (int i = 0; i < 50; i++) {
        move(HIGH);
      }
    }
  } else {
    // Print error message based on the error code.
    Serial.println(DHT11::getErrorString(dhtResult));
  }
}

void translateIR() {
  if (IrReceiver.decode()) {

    /*
         * Print a summary of received data
         */
    if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
      Serial.println(F("Received noise or an unknown (or not yet enabled) protocol"));
      // We have an unknown protocol here, print extended info
      IrReceiver.printIRResultRawFormatted(&Serial, true);
      IrReceiver.resume();  // Do it here, to preserve raw data for printing with printIRResultRawFormatted()
    } else {
      IrReceiver.resume();  // Early enable receiving of the next IR frame
      // IrReceiver.printIRResultShort(&Serial);
      // IrReceiver.printIRSendUsage(&Serial);
    }

    /*
         * Finally, check the received data and perform actions according to the received command
         */
    if (IrReceiver.decodedIRData.command == 0x7) {
      move(HIGH);
    } else if (IrReceiver.decodedIRData.command == 0x9) {
      move(LOW);
    } else {
      IrReceiver.decodedIRData.command = 0x10;
    }
  }
}

//Reset Easy Driver pins to default states
void resetEDPins() {
  digitalWrite(stp, LOW);
  digitalWrite(dir, LOW);
  digitalWrite(MS1, LOW);
  digitalWrite(MS2, LOW);
  digitalWrite(EN, HIGH);
}
