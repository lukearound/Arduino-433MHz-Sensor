/*
 * Arduino skectch for an 433MHz reciver, translating the wireless messages of
 * wireless temperature probe into a http request, executing a mySQL entry
 * 
 * 19.11.2016
 * 
 * HC-12: Pins 50, 51 connected to TX, RX   (RX must be connected to Serial TX!)
 *             52     connected to Set Pin
 * 
 * 
 */

#include <SPI.h>
#include <Ethernet.h>
#include <SoftwareSerial.h>
#include "support_fcns.h"


#define HC_SET_PIN 6 //52
#define HC_TX_PIN 12 //51
#define HC_RX_PIN 11 //50
#define LED_PIN 13

#define DEBUG 1                  // set to 1 to get Serial debug output
#define MESS_MAX_LEN 50          // maximum length of 433MHz message


SoftwareSerial hc12(HC_RX_PIN,HC_TX_PIN); // RX, TX

String msg, sqlTable;
String param_temp = "t=";
String param_mid = "mid=";
String param_loc = "loc=";
String emptyString = "";
String Smid, Stemp, Sdb_table;

float temperature, humidity, pressure;
String message_id = "";  // used to identify messages as re-send if received multiple times by receiver
                              // set to 0 at boot time only to avoid misinterpreation at receiver side

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xAB };
IPAddress ip(192, 168, 0, 190);
EthernetClient ethernet_client;
char server[] = "mainpi";






void setup() 
{
  if(DEBUG) Serial.begin(9600); // Start the hardware serial port
  
  // disable SD card
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  // enable Ethernet chip
  pinMode(10, OUTPUT);
  digitalWrite(10, LOW);
  
  if(DEBUG) Serial.println("setting up ethernet connection ...");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }

  if(DEBUG) Serial.println("setting up hc-12 ...");
  pinMode(HC_SET_PIN,OUTPUT); //  setPin on hc12
  pinMode(LED_PIN, OUTPUT);  // on board LED
  
  hc12.begin(9600); // Start the software serial port
 
  // setup of the hc12 module
  digitalWrite(HC_SET_PIN,LOW); // enter AT command mode
  hc12.print(F("AT+DEFAULT\r\n")); // 9600, CH1, FU3, (F) to bypass flash memory
  delay(100);
  digitalWrite(HC_SET_PIN,HIGH); // enter transparent mode

  msg.reserve(64); // allocate memory for maximum message length

  if(DEBUG) Serial.println("---------- SETUP END ----------");
}








void loop() 
{
  // look for incoming messages on wireless connection
  msg = checkSerial();

  // process wireless message if available
  if(msg.length() > 0)
  {
    if(DEBUG) Serial.print(msg);
    
    if(crcCheckMessage(msg) == true)
    {
      if(DEBUG) Serial.println("  -> CRC passed");
      extractParameter(Smid, msg, param_mid);
      extractParameter(Stemp, msg, param_temp);
      extractParameter(Sdb_table, msg, param_loc);

      // in case the sensor sends an outdates table name, it can be changed here
      adjustDBtable(Sdb_table);

      // process message if message id was not seen before
      if(Smid == message_id)
      {
        if(DEBUG) Serial.println("mid not new. message dropped");
      }
      else
      {
        message_id = Smid;
        if(DEBUG) Serial.println("new mid. processing message");

        String param_str = "";
        param_str.concat("tab=");
        param_str.concat(Sdb_table);
        param_str.concat("&temp=");
        param_str.concat(Stemp);

        if(DEBUG) Serial.println(param_str);
        
        boolean suc = httpRequest(param_str);
      }
    }
    else
    {
      if(DEBUG) Serial.println("  -> CRC failed!");
    }

    // reset message
    msg = "";
  }
}


//***************************************************************************************************  


String checkSerial(void)
{
  if(hc12.available() > 0) 
  { 
    digitalWrite(LED_PIN, HIGH);
    delay(100);  // wait for transmission to complete (at 1200 char/sec, max len=64, max transmission=53ms)
    
    while(hc12.available() > 0)
    {
      msg += (char)hc12.read();

      // remove any end of line character and return from function
      if(msg.endsWith("\n"))
      {
        msg.remove(msg.length()-1);
        return msg;
      }
    }
    digitalWrite(LED_PIN, LOW);
  }

  return msg;

}




//*************************************************************************************
boolean httpRequest(String &parameters)
{
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  ethernet_client.stop();
  delay(10);

  // if there's a successful connection:
  if(ethernet_client.connect(server, 80)) 
  {
    // send the HTTP GET request:
    // http://mainpi/cgi-bin/sqlentry.py?tab=sensordata_brunnen&temp=20
     
    //ethernet_client.print("GET /cgi-bin/sqlentry.py");
    ethernet_client.print("GET /cgi-bin/sqlentry.py?");
    ethernet_client.print(parameters.c_str());
    ethernet_client.println(" HTTP/1.1");
    ethernet_client.println("Host: mainpi");
    ethernet_client.println("User-Agent: Arduino");
    ethernet_client.println("Accept: text/html");
    ethernet_client.println("Connection: close");
    ethernet_client.println();

    Serial.print("GET /cgi-bin/sqlentry.py?");
    Serial.print(parameters.c_str());
    Serial.println(" HTTP/1.1");

    return true;
  } 
  else 
  {
    // if you couldn't make a connection:
    if(DEBUG) Serial.println("ethernet connection failed");
    return false;
  }
}














