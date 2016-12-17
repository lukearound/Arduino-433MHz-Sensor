#include <DallasTemperature.h>
#include <SoftwareSerial.h>
#include "str_crc.h"

#define DEBUG 0                  // set to 1 to get Serial debug output

#define RESENDS 3                // how many times each message is repeated
#define RESEND_INTERVAL 1.5      // [sec] how long to wait between resend attempts 
#define SEND_INTERVAL 300        // [sec] how long to wait in between transmissions
#define MESS_MAX_LEN 63          // maximum length of 433MHz message
#define TEMP_PIN 7               // OneWire Pin to sensors (custom shield)
#define SS_TX 4                  // serial TX pin for HC-12 433MHz transmitter. Connect to HC-12 RX!!
#define SS_RX 5                  // serial RX pin for HC-12 433MHz transmitter. Connect to HC-12 TX!!
#define SS_SET 6                 // HC-12 set pin. Pull LOW to configure HC-12
#define LED_PIN 13               // pin of on board LED

// one wire addresses of temperature DS18B20 probes
DeviceAddress tprobe1 = { 0x28, 0xE3, 0x52, 0x1D, 0x07, 0x00, 0x00, 0xA7 };  //  probe 6 - 1m, carport
DeviceAddress tprobe2 = { 0x28, 0x73, 0x5D, 0x7B, 0x07, 0x00, 0x00, 0xE0 };  //  probe 7 - 5m, well pit
DeviceAddress tprobe3 = { 0x28, 0xE6, 0x31, 0x73, 0x07, 0x00, 0x00, 0x96 };  //  probe 8 - 10m, garage

SoftwareSerial     hc12(SS_RX, SS_TX); // RX, TX
OneWire            ds(TEMP_PIN);  // DS18S20 Temperaturchip i/o an pin 10
DallasTemperature  sensors(&ds);


unsigned int message_id = 0;  // used to identify messages as re-send if received multiple times by receiver
                              // set to 0 at boot time only to avoid misinterpreation at receiver side

char message[MESS_MAX_LEN+1];  // message to send over 433MHz channel to enter values into mySQL database
String Message;  // used to create message. waste of memory but easier to handle than char array

void setup()
{
  pinMode(LED_PIN, OUTPUT);    // prepare on board led to signal activity
  pinMode(SS_SET, OUTPUT); //  setPin on hc12
  
  digitalWrite(LED_PIN, LOW);  // LED off
  if(DEBUG) Serial.begin(9600);

  // setup of the hc12 module
  hc12.begin(9600); // Start the software serial port
  digitalWrite(SS_SET, LOW); // enter AT command mode
  hc12.print(F("AT+DEFAULT\r\n")); // 9600, CH1, FU3, (F) to bypass flash memory
  delay(100);
  digitalWrite(SS_SET, HIGH); // enter transparent mode

  // setup DS18B20 temperature probes
  sensors.begin();    // start up temperature sensor
  sensors.setResolution(tprobe1, 11);  // 11: 0.125°C res
  sensors.setResolution(tprobe2, 11);
  sensors.setResolution(tprobe3, 11);
}

// handling the timing
unsigned long previousTime = millis()/1000 - SEND_INTERVAL;
unsigned long currentTime = 0;
int           nomsgcount = 0;


void loop()
{
  float t1 = 0;
  float t2 = 0;
  float t3 = 0;
  int sensor_success = 0;
  unsigned long crc;

  currentTime = millis()/1000;
  if(currentTime - previousTime > SEND_INTERVAL)
  {
    previousTime = currentTime;
    // switch on LED
    digitalWrite(LED_PIN, HIGH);
  
    sensors.requestTemperatures();
    t1 = sensors.getTempC(tprobe1);    // carport
    t2 = sensors.getTempC(tprobe2);    // well pit
    t3 = sensors.getTempC(tprobe3);    // garage

    //***********************************************************************************
    // check if values make sense and transmit if they do 
    if(t1 > -50)
    {
      Message = String("mid=") + message_id + String(",loc=sensordata_carport,t=") + t1;
      crc = str_crc(Message);
      Message += String(",crc=") + String(crc);
      Message.toCharArray(message,MESS_MAX_LEN);

      // send RESEND times for redundancy
      for(int n=0; n < RESENDS; n++)
      {
        if(DEBUG) Serial.println(message);
        hc12.println(message);
        delay(RESEND_INTERVAL*1000);
      }
      if((message_id+=1) == 0) message_id = 1;  // count up, skip zero
      sensor_success+=1;
    }
   else
    {
      Message = String("mid=") + message_id + String(",loc=sensordata_carport,t=error");
      crc = str_crc(Message);
      Message += String(",crc=") + String(crc);
      Message.toCharArray(message,MESS_MAX_LEN);

      if(DEBUG) Serial.println(Message);
      hc12.println(Message);

      if((message_id+=1) == 0) message_id = 1;  // count up, skip zero
      sensor_success+=1;    
    }

    //************************************************************************************
    if(t2 > -50)
    {
      Message = String("mid=") + message_id + String(",loc=sensordata_brunnen,t=") + t2;
      crc = str_crc(Message);
      Message += String(",crc=") + String(crc);
      Message.toCharArray(message,MESS_MAX_LEN);

      // send RESEND times for redundancy    
      for(int n=0; n < RESENDS; n++)
      {
        if(DEBUG) Serial.println(message);
        hc12.println(message);
        delay(RESEND_INTERVAL*1000);
      }
      if((message_id+=1) == 0) message_id = 1;  // count up, skip zero
      sensor_success+=1;    
    }
    else
    {
      Message = String("mid=") + message_id + String(",loc=sensordata_brunnen,t=error");
      crc = str_crc(Message);
      Message += String(",crc=") + String(crc);
      Message.toCharArray(message,MESS_MAX_LEN);

      if(DEBUG) Serial.println(Message);
      hc12.println(Message);

      if((message_id+=1) == 0) message_id = 1;  // count up, skip zero
      sensor_success+=1;    
    }


    //************************************************************************************
    if(t3 > -50)
    {
      Message = String("mid=") + message_id + String(",loc=sensordata_garage,t=") + t3;
      crc = str_crc(Message);
      Message += String(",crc=") + String(crc);
      Message.toCharArray(message,MESS_MAX_LEN);

      // send RESEND times for redundancy    
      for(int n=0; n < RESENDS; n++)
      {
        if(DEBUG) Serial.println(message);
        hc12.println(message);
        delay(RESEND_INTERVAL*1000);
      }
      if((message_id+=1) == 0) message_id = 1;  // count up, skip zero
      sensor_success+=1;    
    }
    else
    {
      Message = String("mid=") + message_id + String(",loc=sensordata_garage,t=error");
      crc = str_crc(Message);
      Message += String(",crc=") + String(crc);
      Message.toCharArray(message,MESS_MAX_LEN);

      if(DEBUG) Serial.println(Message);
      hc12.println(Message);

      if((message_id+=1) == 0) message_id = 1;  // count up, skip zero
      sensor_success+=1;    
    }

    hc12.end();

    // switch LED off if both sensors were read sucessfully
    // LED will stay on if one of the sensors wasn't accessible (return value -127)
    if(sensor_success==3)
    {
      digitalWrite(LED_PIN,LOW);
    }
  }  // IF MILLIS()

  
  else
  {
    delay(1);
  }
} // LOOP










//
