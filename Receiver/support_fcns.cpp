/*
	returns crc of a String. 
	16.11.2016
	Taken and modified from arduino.cc example:
	https://www.arduino.cc/en/Tutorial/EEPROMCrc

*/


#include "Arduino.h"

#define DEBUG 0


unsigned long str_crc(String message) {

  const unsigned long crc_table[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
  };

  unsigned long crc = ~0L;
  
  
  for (int index = 0 ; index < message.length()  ; ++index) {
    crc = crc_table[(crc ^ message[index]) & 0x0f] ^ (crc >> 4);
    crc = crc_table[(crc ^ (message[index] >> 4)) & 0x0f] ^ (crc >> 4);
    crc = ~crc;
  }
  return crc;
}


//*****************************************************************************************************
boolean crcCheckMessage(String &msg)
{
  unsigned long crc = 0;

  int icrc = msg.indexOf(",crc=");
  unsigned long lcrc     = 0;
  unsigned long sub_lcrc = 0;

  if(icrc <= 0)
  {
    if(DEBUG) Serial.println("no crc found! discard message.");
    msg = "";
    return false;
  }
  else
  {
    // store crc
    String submitted_crc = msg.substring(icrc+5,msg.length());
    
    // remove crc from message (including ",crc=" part)
    String tempMsg = msg.substring(0,icrc);

    // calculate and compare crc
    String crc = String(str_crc(tempMsg));

    if(DEBUG) Serial.println();
    if(DEBUG) Serial.print("transmitted crc: -->");
    if(DEBUG) Serial.print(submitted_crc);
    if(DEBUG) Serial.println("<--");
    if(DEBUG) Serial.print("calculated crc:  -->");
    if(DEBUG) Serial.print(crc);
    if(DEBUG) Serial.println("<--");

    for (int i = 0; i < crc.length(); i++) {
      char c = crc.charAt(i);
      if (c < '0' || c > '9') break;
        lcrc *= 10;
        lcrc += (c - '0');
    }

    for (int i = 0; i < submitted_crc.length(); i++) {
      char c = submitted_crc.charAt(i);
      if (c < '0' || c > '9') break;
        sub_lcrc *= 10;
        sub_lcrc += (c - '0');
    }

    //if(crc.equalsIgnoreCase(submitted_crc))
    if(lcrc == sub_lcrc)
    {
      // crc of message equals transmitted crc
      return true;
    }
    else
    {
      // crc of message differs from transmitted crc
      return false;
    }
  }
}




//****************************************************************************************


// returns requested parameter from message string
void extractParameter(String &retString, String &msg, String &param)
{
  // search for string
  int n = msg.indexOf(param,0);

  if(DEBUG) Serial.print(n);
  if(DEBUG) Serial.print(" -> ");
  
  // check if parameter was found
  if (n==-1)
  {
    // not found
    retString = "";
    return;
  }
  else
  {
    // search for ',' following the parameter value
    int m = msg.indexOf(",", n);
    if(DEBUG) Serial.println(m);
    if(DEBUG) Serial.println(msg.substring(n,m));

    // return value following the parameter
    retString = msg.substring(n+param.length(),m);
    return;
  }
}


//***********************************************************
long string2long(String &str)
{
  Serial.println(str);
  
  long lval = 0;

  for (int i = 0; i < str.length(); i++) {
      char c = str.charAt(i);
      if (c < '0' || c > '9') break;
        lval *= 10;
        lval += (c - '0');
  }

  return lval;
}



//*************************************************************

void adjustDBtable(String &Sdb_table)
{
  if(Sdb_table == "sensordata_brunnen")
  {
    Sdb_table = "sensordata_carport";
  }
  else if(Sdb_table == "sensordata_carport")
  {
   Sdb_table = "sensordata_brunnen";
  }
}
















//
