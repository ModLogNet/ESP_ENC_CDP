#include <Arduino.h>
#include "lldp_functions.h";
#include "Packet_data.h";
PINFO info;

int lldpIPaddr = 0;
int eightOtwo = 0;
byte lldp_mac[] = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x0e};
byte lldp_llc_bytes[] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x0c, 0x20, 0x00};
byte lldp_encbuff[1500];


unsigned int lldp_check_Packet(int plen, byte lldp_encbuff[1500], int bufSize ) {


  if ( plen > 0 ) {
    if (plen > sizeof(lldp_encbuff)) {

      plen = sizeof(lldp_encbuff);
    }
    unsigned int cdpDataIndex = 0;

    if (lldp_byte_array_contains(lldp_encbuff, cdpDataIndex, lldp_mac, sizeof(lldp_mac))) {
      cdpDataIndex += sizeof(lldp_mac); // Increment index the length of the source MAC address

      //LDDP Packet found and is now getting processed
      Serial.println("LLDP Packet Recieved");

      //Get source MAC Address
      byte* macFrom = lldp_encbuff + cdpDataIndex;
      info.MAC = lldp_print_mac(macFrom, 0, 6);

      cdpDataIndex += sizeof(lldp_mac); // Increment index the length of the MAC address

      unsigned int packet_length = (lldp_encbuff[cdpDataIndex] << 8) | lldp_encbuff[cdpDataIndex + 1];
      cdpDataIndex += 2;
      return cdpDataIndex;
    }
    else {
      return (0);
    }
  }


}

bool lldp_byte_array_contains(const byte a[], unsigned int offset, const byte b[], unsigned int length) {
  for (unsigned int i = offset, j = 0; j < length; ++i, ++j) {
    if (a[i] != b[j]) {
      return false;
    }
  }
  return true;
}

PINFO lldp_packet_handler( byte cdpData[], size_t plen) {

  //if (byte_array_contains(cdpData, 0, lldp_mac, sizeof(lldp_mac))) {

  //CDP Packet found and is now getting processed
  //  Protocal = "LLDP";
  //  Serial.println("LLPD Packet Recieved");
  info.Proto = "LLDP";
  byte* macFrom = cdpData + sizeof(lldp_mac);
  lldp_print_mac(macFrom, 0, 6);

  unsigned int cdpDataIndex = 14;

  while (cdpDataIndex < plen) { // read all remaining TLV fields
    unsigned int lldpFieldType = cdpData[cdpDataIndex];
    cdpDataIndex += 1;
    unsigned int lldpFieldLength = cdpData[cdpDataIndex];
    /*   Serial.print(" type:");
       Serial.print(lldpFieldType, HEX);
       Serial.print(" Length:");
       Serial.print(lldpFieldLength); */
    cdpDataIndex += 1;

    switch (lldpFieldType) {
         //Chassis ID
      case 0x0002:
        info.ChassisID = handleportsubtype( cdpData, cdpDataIndex , lldpFieldLength );
        break;

      //Port Name
      case 0x0004:
        info.Port = handleportsubtype( cdpData, cdpDataIndex , lldpFieldLength );

        break;

      //TTL
      case 0x0006:
        info.TTL = lldp_handleCdpNumField(cdpData, cdpDataIndex , lldpFieldLength);
        break;

      //Port Description
      case 0x0008:
        info.PortDesc = handlelldpAsciiField( cdpData, cdpDataIndex , lldpFieldLength);
        break;

      //Device Name
      case 0x000a:
        info.Name = handlelldpAsciiField( cdpData, cdpDataIndex , lldpFieldLength);
        break;

      //Model Name
      case 0x000c:
        info.Model = handlelldpAsciiField( cdpData, cdpDataIndex, lldpFieldLength);

        break;

      //Capabilities
      case 0x000e:
        handlelldpCapabilities( cdpData, cdpDataIndex + 2, lldpFieldLength - 2);
        break;

      //Management IP Address
      case 0x0010:
        info.IP = handleLLDPIPField(cdpData, cdpDataIndex + 2, 4);
        break;

      //  MAC/PHY Configuration
      case 0x00fe:
        //Port VLAN ID
        if (eightOtwo == 0) {
          //  Serial.println(lldp_print_mac(cdpData, cdpDataIndex +4 , lldpFieldLength-4));
          info.VLAN = lldp_handleCdpNumField(cdpData, cdpDataIndex + 4 , lldpFieldLength - 4);
        }

        eightOtwo++;
        break;

    }
    cdpDataIndex += lldpFieldLength;
  }
  //drawscreen();
  // Serial.println("DEBUG: Clear buffer");
  //memset(cdpData, 0, sizeof(cdpData));

  return info;
}

String handleLLDPIPField(const byte a[], unsigned int offset, unsigned int lengtha) {
  int j = 0;
  String temp;
  for (unsigned int i = offset; i < ( offset + lengtha ); ++i , ++j) {
    temp  += a[i], DEC;
    if (j < 3) {
      temp  += ".";
    }
  }
  temp += '\0';
  return  temp;
}

String handlelldpAsciiField(byte a[], unsigned int offset, unsigned int lengtha) {
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

void handlelldpCapabilities( const byte a[], unsigned int offset, unsigned int lengtha) {
  int j = 0;
  String temp;

  for (unsigned int i = offset; i < ( offset + lengtha ); ++i , ++j) {
    temp  =  temp + lldp_print_binary(a[i], 8)  ;
  }
  info.Cap =  (LldpCapabilities(temp));
}

String lldp_print_binary(int v, int num_places)
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

String LldpCapabilities(String temp) {
  //Serial.print (temp);
  String output;
  //Serial.print(temp.substring(26,27));
  if (temp.substring(15, 16) == "1") {
    output = output + "Other ";
  }
  if (temp.substring(14, 15) == "1") {
    output = output + "Repeater ";
  }
  if (temp.substring(13, 14) == "1") {
    output = output + "Bridge ";
  }
  if (temp.substring(12, 13) == "1") {
    output = output + "WLAN ";
  }
  if (temp.substring(11, 12) == "1") {
    output = output + "Router ";
  }
  if (temp.substring(10, 11) == "1") {
    output = output + "Telephone ";
  }
  if (temp.substring(9, 10) == "1") {
    output = output + "DOCSIS ";
  }

  if (temp.substring(8, 9) == "1") {
    output = output + "Station ";
  }
  return output;
}

String lldp_print_mac(const byte a[], unsigned int offset, unsigned int length) {
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

  return Mac;
}

String lldp_handleCdpNumField( const byte a[], unsigned int offset, unsigned int length) {
  String temp;
  unsigned long num = 0;
  for (unsigned int i = 0; i < length; ++i) {
    num <<= 8;
    num += a[offset + i];
  }
  temp = "" + String(num, DEC);
  return temp;
}

String handleportsubtype(byte cdpData[], unsigned int cdpDataIndex, unsigned int lldpFieldLength) {
  lldpFieldLength = lldpFieldLength - 1;
  unsigned int charTemp = cdpData[cdpDataIndex];
  Serial.println("Port Sub type:" );
  Serial.print(charTemp);
  cdpDataIndex++;

  /*
        https://docs.zephyrproject.org/latest/reference/kconfig/CONFIG_NET_LLDP_PORT_ID_SUBTYPE.html
        Subtype 1 = Interface alias
        Subtype 2 = Port component
        Subtype 3 = MAC address
        Subtype 4 = Network address
        Subtype 5 = Interface name
        Subtype 6 = Agent circuit ID
        Subtype 7 = Locally assigned
  */
  switch (charTemp) {

    case 0x0001:
      //ASCII
      return handlelldpAsciiField( cdpData, cdpDataIndex , lldpFieldLength);
      break;

    case 0x0002:
      //ASCII
      return handlelldpAsciiField( cdpData, cdpDataIndex , lldpFieldLength);
      break;

    case 0x0003:
      //MAC Address
      return lldp_print_mac(cdpData, cdpDataIndex, lldpFieldLength);
      break;

    case 0x0004:
      //IP Address
      return handleLLDPIPField(cdpData, cdpDataIndex, lldpFieldLength);
      break;

    case 0x0005:
      //ASCII
      return handlelldpAsciiField( cdpData, cdpDataIndex , lldpFieldLength);
      break;

    case 0x0006:
      // ??
      break;

    case 0x0007:
      //ASCII ??
      return handlelldpAsciiField( cdpData, cdpDataIndex , lldpFieldLength);
      break;
  }
  return " ";
}
