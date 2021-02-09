#include <Arduino.h>
#include "cdp_functions.h";
#include "Packet_data.h";
byte cdp_mac[] = {0x01, 0x00, 0x0c, 0xcc, 0xcc, 0xcc};
byte llc_bytes[] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x0c, 0x20, 0x00};

PINFO cdpinfo;

byte encbuff[1500];

unsigned int cdp_check_Packet(int plen, byte encbuff[1500], int bufSize ) {

  if ( plen > 0 ) {
    if (plen > sizeof(encbuff)) {

      plen = sizeof(encbuff);
    }
    unsigned int cdpDataIndex = 0;
    if (byte_array_contains(encbuff, cdpDataIndex, cdp_mac, sizeof(cdp_mac))) {
      cdpDataIndex += sizeof(cdp_mac);
      //CDP Packet found and is now getting processed


      Serial.println("CDP Packet Recieved");

      //Get source MAC Address
      byte* macFrom = encbuff + cdpDataIndex;
      print_mac(macFrom, 0, 6);
      // Serial.println();
      //////////////////////////////////
      cdpDataIndex += sizeof(cdp_mac); // received from, MAC address = 6 bytes
      //cdp_getHEX(encbuff, cdpDataIndex, sizeof(llc_bytes)) ;
      unsigned int packet_length = (encbuff[cdpDataIndex] << 8) | encbuff[cdpDataIndex + 1];
      cdpDataIndex += 2;
      if (packet_length != plen - cdpDataIndex) {

        cdpDataIndex += sizeof(llc_bytes);

        return cdpDataIndex;
      }
      else {

        Serial.println("wrong packet length");
      }
      if (!byte_array_contains(encbuff, cdpDataIndex, llc_bytes, sizeof(llc_bytes))) {

        cdpDataIndex += sizeof(llc_bytes);

        return cdpDataIndex;
      }
      else {
        Serial.println("no llc bytes");
      }


    }
  }

  return (1);
}

PINFO cdp_packet_handler( byte cdpData[], size_t cdpDataIndex, size_t plen) {
  /*unsigned long secs = millis() / 1000;
    int min = secs / 60;
    int sec = secs % 60;
    //Serial.print(min); Serial.print(':');
    if (sec < 10) Serial.print('0');
    //Serial.print(sec);
    //Serial.println();
  */
  cdpinfo.Proto = "CDP";
  int cdpVersion = cdpData[cdpDataIndex++];
  if (cdpVersion != 0x02) {
    Serial.print(F("Version: "));
    Serial.println(cdpVersion);
  }
  cdpinfo.ProtoVer = cdpVersion;

  cdpDataIndex = 26;
  //Serial.println ("cdpDataIndex");
  //Serial.println (cdpDataIndex);
  //Cycle through the frame reading the TLV fields
  while (cdpDataIndex < plen) {
    unsigned int cdpFieldType = (cdpData[cdpDataIndex] << 8) | cdpData[cdpDataIndex + 1];
    cdpDataIndex += 2;
    unsigned int cdpFieldLength = (cdpData[cdpDataIndex] << 8) | cdpData[cdpDataIndex + 1];
    cdpDataIndex += 2;
    cdpFieldLength -= 4;

    switch (cdpFieldType) {

      //Get the device name
      case 0x0001:

        cdpinfo.Name = handleCdpAsciiField(cdpData, cdpDataIndex, cdpFieldLength);

        break;

      //IP address
      case 0x0002:
        handleCdpAddresses(cdpData, cdpDataIndex, cdpFieldLength );

        break;

      //Port Name
      case 0x0003:

        cdpinfo.Port = handleCdpAsciiField(cdpData, cdpDataIndex, cdpFieldLength);

        break;

      case 0x0004:
        cdpinfo.Cap = handleCdpCapabilities(cdpData, cdpDataIndex + 2, cdpFieldLength - 2);
        break;

      case 0x0005:

        // if (cdpFieldLength > 50) {    cdpFieldLength = 50;}
        cdpinfo.SWver = handleCdpAsciiField( encbuff, cdpDataIndex, cdpFieldLength);

        break;

      //CDP Model Name
      case 0x0006:
        cdpinfo.Model = handleCdpAsciiField(cdpData, cdpDataIndex, cdpFieldLength);

        break;

      case 0x0009:
        cdpinfo.VTP = handleCdpAsciiField( encbuff, cdpDataIndex, cdpFieldLength);
        break;

      //CDP VLAN #
      case 0x000a:
        cdpinfo.VLAN = handleCdpNumField(cdpData, cdpDataIndex, cdpFieldLength);
        break;

      case 0x000b:
        cdpinfo.Dup = handleCdpDuplex(cdpData, cdpDataIndex, cdpFieldLength);
        break;


      //CDP VLAN voice#
      case 0x000e:
        handleCdpVoiceVLAN(cdpData, cdpDataIndex + 2, cdpFieldLength - 2);
        break;






    }
    cdpDataIndex += cdpFieldLength;
  }

  return cdpinfo;
}


bool byte_array_contains(const byte a[], unsigned int offset, const byte b[], unsigned int length) {
  for (unsigned int i = offset, j = 0; j < length; ++i, ++j) {
    if (a[i] != b[j]) {
      return false;
    }
  }
  return true;
}


String print_mac(const byte a[], unsigned int offset, unsigned int length) {
  String Mac;
  char temp [40];

  for (unsigned int i = offset; i < offset + length; ++i) {

    if (i > offset) {

      Mac = Mac + ':';
    }
    if (a[i] < 0x10) {
      Mac = Mac + '0';

    }
    Mac = Mac + String (a[i], HEX);
  }
  cdpinfo.MAC =  Mac;

  return Mac;
}


String CdpCapabilities(String temp) {

  String output;
  //Serial.print(temp.substring(26,27));
  if (temp.substring(15, 16) == "1") {
    output = output + "Router ";
  }
  if (temp.substring(14, 15) == "1") {
    output = output + "Trans_Bridge ";
  }
  if (temp.substring(13, 14) == "1") {
    output = output + "Route_Bridge,";
  }
  if (temp.substring(12, 13) == "1") {
    output = output + "Switch ";
  }
  if (temp.substring(11, 12) == "1") {
    output = output + "Host ";
  }
  if (temp.substring(10, 11) == "1") {
    output = output + "IGMP ";
  }
  if (temp.substring(9, 10) == "1") {
    output = output + "Repeater ";
  }

  return output;
}



String handleCdpCapabilities( const byte a[], unsigned int offset, unsigned int lengtha) {
  int j = 0;
  String temp;

  for (unsigned int i = offset; i < ( offset + lengtha ); ++i , ++j) {
    temp  =  temp + print_binary(a[i], 8);
  }
  return  CdpCapabilities(temp);
}



String print_binary(int v, int num_places)
{
  String output;
  int mask = 0, n;
  for (n = 1; n <= num_places; n++)
  {
    mask = (mask << 1) | 0x0001;
  }
  v = v & mask;  // truncate v to specified number of places

  while (num_places)
  {
    if (v & (0x0001 << num_places - 1))
    {
      output = output + "1" ;
    }
    else
    {
      output = output + "0" ;
    }
    --num_places;
  }
  return output;
}
String handleCdpNumField( const byte a[], unsigned int offset, unsigned int length) {
  unsigned long num = 0;
  String temp;
  for (unsigned int i = 0; i < length; ++i) {
    num <<= 8;
    num += a[offset + i];
  }
  temp = "" + String(num, DEC);
  return temp;
}

void handleCdpVoiceVLAN( const byte a[], unsigned int offset, unsigned int length) {
  unsigned long num = 0;
  for (unsigned int i = offset; i < ( offset + length ); ++i) {
    num <<= 8;
    // Serial.print(a[i]);
    num += a[i];
  }
  cdpinfo.VoiceVLAN = String(num, DEC);
}



boolean handleCdpAddresses(const byte a[], unsigned int offset, unsigned int length) {
  unsigned long numOfAddrs = (a[offset] << 24) | (a[offset + 1] << 16) | (a[offset + 2] << 8) | a[offset + 3];
  offset += 4;
  //  Serial.print("number of addresses: " );
  //    Serial.print(numOfAddrs);
  for (unsigned long i = 0; i < numOfAddrs; ++i) {
    unsigned int protoType = a[offset++];
    unsigned int protoLength = a[offset++];
    byte proto[8];
    for (unsigned int j = 0; j < protoLength; ++j) {
      proto[j] = a[offset++];
    }
    unsigned int addressLength = (a[offset] << 8) | a[offset + 1];

    offset += 2;
    byte address[4];
    if (addressLength != 4) {
      Serial.print(F(" Corrupt Packet Received: "));
      Serial.print (addressLength);
      return (0);

    }
    //if (addressLength != 4)     break;
    cdpinfo.IP = "";
    for (unsigned int j = 0; j < addressLength; ++j) {
      address[j] = a[offset++];

      cdpinfo.IP = cdpinfo.IP + address[j] ;
      if (j < 3) {
        cdpinfo.IP = cdpinfo.IP + ".";
      }



    }
    if (addressLength != 4)     break;
  }

}



String handleCdpDuplex(const byte a[], unsigned int offset, unsigned int length) {
  String temp;
  //Serial.print(F("Duplex: "));
  if (a[offset]) {
    //   Serial.println(F("Full"));
    temp = "Full";
  }
  else {
    //   Serial.println(F("Half"));
    temp = "Half";
  }
  return temp;
}


String handleCdpAsciiField(byte a[], unsigned int offset, unsigned int lengtha) {
  int j = 0;

  char temp [lengtha + 1] ;
  for (unsigned int i = offset; i < ( offset + lengtha ); ++i , ++j) {
    temp[j] = a[i];
  }
  temp[lengtha  ] = '\0';

  String temp1  = temp;
  // Serial.print(temp1);
  return temp1;
}

String print_ip(const byte a[], unsigned int offset, unsigned int length) {
  String ip;
  for (unsigned int i = offset; i < offset + length; ++i) {
    //    if(i>offset) Serial.print('.');
    //   Serial.print(a[i], DEC);
    if (i > offset) ip = ip + '.';
    ip = ip + String (a[i]);
  }
  int iplentgh;
  return ip;
}


void print_byte(byte a[], int psize) {
  for (unsigned int i = 0; i < psize; ++i) {
  }
}

char val2dec(byte b) {
  switch (b) {
    case 0:
      return '0';
    case 1:
      return '1';
    case 2:
      return '2';
    case 3:
      return '3';
    case 4:
      return '4';
    case 5:
      return '5';
    case 6:
      return '6';
    case 7:
      return '7';
    case 8:
      return '8';
    case 9:
      return '9';
  }
}

void cdp_getHEX(const byte a[], unsigned int offset, unsigned int length) {
  String Mac;
  char temp [40];

  for (unsigned int i = offset; i < offset + length; ++i) {

    if (i > offset) {

      Mac = Mac + ':';
    }
    if (a[i] < 0x10) {
      Mac = Mac + '0';

    }
    Mac = Mac + String (a[i], HEX);
  }

  Serial.print(Mac);

}
