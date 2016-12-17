/*
  returns crc of a String. 
  16.11.2016
  Taken and modified from arduino.cc example:
  https://www.arduino.cc/en/Tutorial/EEPROMCrc

*/

#ifndef STR_CRC_CPP
#define STR_CRC_CPP


unsigned long str_crc(String message);
boolean crcCheckMessage(String &msg);
void extractParameter(String &retString, String &msg, String &param);  
long string2long(String &str);
void adjustDBtable(String &Sdb_table);
#endif


