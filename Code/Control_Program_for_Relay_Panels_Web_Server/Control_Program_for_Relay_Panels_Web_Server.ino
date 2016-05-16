#include <SPI.h>
#include <Ethernet.h>
#include <EthernetServer.h>
#include <EthernetClient.h>
#include <avr/pgmspace.h>


//code based on arduino web server tutorial
//using static ip address

//Designed for use with Arduino Nano


// Basically the idea is to have devices that control things
// (the ardunios (servers) that are connected to the physical devices) and
// clients (ardunios) which are the push button panels and other human interfaces.
//
//
// Each arduino (either client or server) is running a tftp bootloader(freetronics).
// See references.txt for linky. This allows for the storage of IP and MAC in
// EEPROM. This needs static IPs.
//
// ALternatively, can use arduino-netboot bootloader which allows for DHCP config
// but requires DNS and DHCP setup.
//
// since KISS should always be in effect, probably use static IPs and freetronics
// bootloader. Since on own subnet anyways, static addressing is not an issue.
//
// MAC addresses are generated randomly using locally administered addressing,
// and compared to previously assigned addresses in database before being burned
// into the EEPROM of the device.
//
// Locally administered addresses are unicast and of the form:
// x2-xx-xx-xx-xx-xx
// x6-xx-xx-xx-xx-xx
// xA-xx-xx-xx-xx-xx
// xE-xx-xx-xx-xx-xx
//
// Where x is any hex value.
//
// Each server has an IP address and can have multiple controlled devices. Query
// strings will be customized for specific controlled devices.
//
// This code is a template that will be automagically filled in for a
// specific device by the command and control server.
//
//
// this is code for an arduino board that controls something when it receives
// a query string from an EthernetClient
//
// it also reports its status to the command and control server

//Definitions
int port = 80; //change to custom port //TODO: change to custom port
byte mac[] = { , , , , , }; //need to fill in with generated mac address (6 byte array)
IPAddress ip( , , , ); //static ip address of local device (server)
IPAddress dns( , , , ); //IP address of DNS server
IPAddress gateway( , , , ); //Router's gateway address
IPAddress subnet( , , , ); //Subnet mask
IPAddress ccServer( , , , ); //C2 server IP
// add list of query strings to respond to here
// will need two entries to turn something on and off.
const PROGMEM char query1[] = "";

EthernetServer server(80);
char incString[100]; //set max length of request string
void setup() {
  // put your setup code here, to run once:
  Ethernet.begin(mac, ip, dns, gateway, subnet);
  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  EthernetClient client = server.available();
  if (client){ //listen for incoming clients
    while (client.connected()){
      if (client.available()){
        int ii = 0;
        while ((c = client.read()) != "\n"){ //read from client
          incString[ii++] = c;
        }

        //incString now has querry string inside it.

        //only one client will ever be connected at once
        //one query string will be a status request from the C2 server. something along the lines of Status?
        //use client.write or println to respond. probably .println w/json encoded data.
        delay(1);
        //stopping client
        client.stop();
        //implement functions here that control outputs
        //define query strings at the top and then reference them in the if statements down here
        if (strcmp_PF(incString, PSTR(query1))==0){
          //do what ever, like set output high or low to trigger a relay or whatever.
        }//repeat if statements for other triggers

        incString=""; //clear string for next useage

      }
    }
  }
}
