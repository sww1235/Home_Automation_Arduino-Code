#include <Ethernet.h>
#include <EthernetClient.h>
#include <Wire.h>
const int ShiftPWM_latchPin = 8;
const bool ShiftPWM_invertOutputs = false;
const bool ShiftPWM_balanceLoad = false;
#include <CShiftPWM.h>
#include <pins_arduino_compile_time.h>
#include <ShiftPWM.h>
#include <SPI.h>
#include <avr/pgmspace.h>


//Designed for use with Arduino Nano

/*
Each button press should establish a EthernetClient connection to an
EthernetServer, which is running on the appropriate relay or device.
It should also establish an EthernetClient connection to a similar server
listening on a command and control server which will update the database with
the current state of the client device.



Basically the idea is to have devices that control things
(the ardunios (servers) that are connected to the physical devices) and
clients (ardunios) which are the push button panels and other human interfaces.


Each arduino (either client or server) is running a tftp bootloader(freetronics).
See references.txt for linky. This allows for the storage of IP and MAC in
EEPROM. This needs static IPs.

ALternatively, can use arduino-netboot bootloader which allows for DHCP config
but requires DNS and DHCP setup.

since KISS should always be in effect, probably use static IPs and freetronics
bootloader. Since on own subnet anyways, static addressing is not an issue.

MAC addresses are generated randomly using locally administered addressing,
and compared to previously assigned addresses in database before being burned
into the EEPROM of the device.

Locally administered addresses are unicast and of the form:
x2-xx-xx-xx-xx-xx
x6-xx-xx-xx-xx-xx
xA-xx-xx-xx-xx-xx
xE-xx-xx-xx-xx-xx

Where x is any hex value.

Each server has an IP address and can have multiple controlled devices. Query
strings will be customized for specific controlled devices.

This code is a template that will be automagically filled in for a
specific device by the command and control server.


This code is for the RGB LED button panel clients.

Remember that there are 8 buttons on the device plus an additional 2
external buttons.
*/
const int resetEthernetPin = 4;
const int rxSense = 3;
//Function Prototypes
int connectToServer(IPAddress address, const char request[]);
int connectToServerAndRetrieveData(IPAddress address, const char request[]);
void ethernetReset();
//Definitions for LED control
unsigned char maxBrightness = 255;
unsigned char pwmFrequency = 75;
unsigned int numRegisters = 6;
unsigned int numOutputs = numRegisters*8;
unsigned int numRGBLeds = numRegisters*8/3;
unsigned int fadingMode = 0; //start with all LED's off.
unsigned long startTime = 0; // start time for the chosen fading mode


//make sure to move all string constants to progmem to save space, especially
//the query strings.

//change to nonstandard port once decided upon.
const int port = 80;
//need to fill in with generated mac address (6 byte array)
const byte mac[] = { , , , , , };
//static ip address of local device (client)
const IPAddress ip( , , , );
IPAddress dns( , , , ); //IP address of DNS server
IPAddress gateway( , , , ); //Router's gateway address
IPAddress subnet( , , , ); //Subnet mask

//Duplicate Server IP address if it is accessed by more than one button.

//static ip address of device to be controlled by button 1
const IPAddress server1( , , , );
//static ip address of device to be controlled by button 2
const IPAddress server2( , , , );
//static ip address of device to be controlled by button 3
const IPAddress server3( , , , );
//static ip address of device to be controlled by button 4
const IPAddress server4( , , , );
//static ip address of device to be controlled by button 5
const IPAddress server5( , , , );
//static ip address of device to be controlled by button 6
const IPAddress server6( , , , );
//static ip address of device to be controlled by button 7
const IPAddress server7( , , , );
//static ip address of device to be controlled by button 8
const IPAddress server8( , , , );
//static ip address of device to be controlled by button 9  (external switch)
const IPAddress server9( , , , );
//static ip address of device to be controlled by button 10 (external switch)
const IPAddress server0( , , , );
//static ip address of command and control server
const IPAddress ccServer( , , , );
//Will always have 10 queries, even if they are duplicates.
//message string format: tbd //TODO figure out a good message string format
const PROGMEM char query1[] = "";
const PROGMEM char query2[] = "";
const PROGMEM char query3[] = "";
const PROGMEM char query4[] = "";
const PROGMEM char query5[] = "";
const PROGMEM char query6[] = "";
const PROGMEM char query7[] = "";
const PROGMEM char query8[] = "";
const PROGMEM char query9[] = "";
const PROGMEM char query0[] = "";
const PROGMEM char ccQuery[] = ""; //querry to get data back from command and control server
EthernetClient client; //main ethernet client

//EthernetServer server(port); //if we want a server

byte GPIOA, GPIOB;
boolean button1, button2, button3, button4, button5, button6, button7, button8, button9, button0;
// length of response from server (only requesting data from
//command and control server will use this, shrink if needed.
char incString[100];
void(* resetFunc) (void) = 0; //declare reset function @ address 0

//need const for the char array because it is a pass by reference so it
//could be potentially modified by the function if const is not specififed.
int connectToServer(IPAddress address, const char request[]) {
  if (client.connect(address, port)){
    client.println(PSTR(request)); //the client does not actually need to get anything back from the server.
    delay(500);
    client.flush();
    client.stop();
    return 0;
  }
  else return -1;
}
int connectToServerAndRetrieveData(IPAddress address, const char request[]) {
  //clear incString
    incString = "";
  if (client.connect(address, port)) {
    client.println(PSTR(request)); //send request for data
    delay(500);
    if (client.available() > 0) {
      int ii = client.available(); //returns number of bytes available for reading
      char c;
      for (int i = ii; i > 0; i-- )
      {
        c = client.read();
        if (c == '\n') //server data returned terminated with newline
        {
          break;
        }
        incString[i] = c;

      }

    }


    client.flush();
    client.stop();
    return 0;
  }
  else{
    return -1;
  }

}

void ethernetReset() {
  bool rxState;
  int cnt = 10, retryCount = 10, result;

  pinMode(resetEthernetPin, INPUT); //reset pin for ethernet shield
  pinMode(rxSense, INPUT); //sense pin for rx led on ethernet shield


  while (retryCount-- > 0) {
    digitalWrite(resetEthernetPin, HIGH); //enable internal pullup resistor on resetEthernetPin
    pinMode(resetEthernetPin, OUTPUT); //change pin mode to output
    digitalWrite(resetEthernetPin, LOW);//pull ehternet board reset pin low to reset
    delay(1000);
    digitalWrite(resetEthernetPin, HIGH);
    delay(2000);
    // after reset, check rx pin for constant on

    cnt = 10;
    result = 0;
    while (cnt-- != 0) { // simply count the number of times the light is on in the loop
      result += digitalRead(rxSense);
      delay(50);
    }
    if (result >= 6)     // experimentation gave me this number YMMV: confirm this number
      return;
    delay(50);

  }
  // OK, I tried 10 times to make it work, just start over.
  resetFunc();


}

void setup() {
  ethernetReset();
  Ethernet.begin(mac, ip, dns, gateway, subnet); //initilize ethernet connection
  delay(1000);
  Wire.begin(); //wake up I2C bus
    // Sets the number of 8-bit registers that are used.
  ShiftPWM.SetAmountOfRegisters(numRegisters);
  // SetPinGrouping allows flexibility in LED setup.
  // If your LED's are connected like this: RRRRGGGGBBBBRRRRGGGGBBBB, use SetPinGrouping(4).
  ShiftPWM.SetPinGrouping(8); //This is the default, but I added here to demonstrate how to use the function
  ShiftPWM.Start(pwmFrequency,maxBrightness);




}


void loop(){
  //this sets incString[] to a string value recieved from the command
  //and control server that encodes the status of the leds
  int RDsucess = connectToServerAndRetrieveData(ccServer, PSTR(ccQuery));
  //implement stuff to look at value of incString and change led values based on it.
  if (RDsucess){

  }
  //begin transmission on wire bus to address of mux, which is 0x20
  Wire.beginTransmission(0x20);
  Wire.write(0x12); //set MCP23017 memory pointer to GPIOA address
  Wire.endTransmission();
  //read one byte of data from GPIOA.
  //Status will be represented as a binary number.
  Wire.requestFrom(0x20, 1);
  GPIOA = Wire.read();
  Wire.beginTransmission(0x20);
  Wire.write(0x13);
  Wire.endTransmission();
  Wire.requestFrom(0x20, 1);
  GPIOB = Wire.read();
  if (GPIOA > 0 || GPIOB > 0){ //if a button was pressed

    if (GPIOA > 0){
      button1 = GPIOA & B00000001; //set all other bits of result to zero except first which gives us true/false
      button2 = GPIOA & B00000010;
      button3 = GPIOA & B00000100;
      button4 = GPIOA & B00001000;
      button5 = GPIOA & B00010000;
      button6 = GPIOA & B00100000;
      button7 = GPIOA & B01000000;
      button8 = GPIOA & B10000000;

      if (button1 == true){
        connectToServer(server1, query1);
      }
      if (button2 == true){
        connectToServer(server2, query2);
      }
      if (button3 == true){
        connectToServer(server3, query3);
      }
      if (button4 == true){
        connectToServer(server4, query4);
      }
      if (button5 == true){
        connectToServer(server5, query5);
      }
      if (button6 == true){
        connectToServer(server6, query6);
      }
      if (button7 == true){
        connectToServer(server7, query7);
      }
      if (button8 == true){
        connectToServer(server8, query8);
      }

    }
    if (GPIOB > 0){
      button9 = GPIOB & B00000001;
      button0 = GPIOB & B00000010;
      if (button9 == true){
        connectToServer(server9, query9);
      }
      if (button0 == true){
        connectToServer(server0, query0);
      }

    }

  }



}
