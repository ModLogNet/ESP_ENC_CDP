#include <Arduino.h>
#include "DHCPOptions.h";

extern DHCP_DATA DHCP_info[255];
extern int OptionCount;
#define DHCP_INFINITE_LEASE  0xffffffff


void DHCPOption(uint8_t option, const byte* data, uint8_t len) {
  //likely to be an IPv4 address/Subnet
  switch (option) {
    case 1:
      IPv4("MASK", data, len);
      break;

    case 3:
      IPv4("GW", data, len);
      break;

    case 4:
      if (len == 4) {
        IPv4("TIME:", data, len);
      }
      else {
        DHCP_Text("TIME", data, len - 4);
      }
      break;

    case 6:
      if (len > 0) {
        int a = 0;
        for ( int i = 0; i <   (len / 4) ; ++i ) {
          IPv4("DNS" +  String(i + 1), data + (i * 4) , 4 );
          a = a + 4;
          if (((i * 4) + 4) != len) {
            OptionCount++;
          }
        }
      }
      break;

    case 15:
      DHCP_Text("DOMAIN", data, len);
      break;

    case 42:
      if (len == 4) {
        IPv4("NTP:", data, len);
      }
      else {
        DHCP_Text("NTP", data, len - 4);
      }
      break;

    case 44:
      IPv4("NETBIOS", data, len);
      break;

    case 51:
      DHCP_Time("LEASE", data, len);
      break;

    case 54:
      IPv4("DHCP", data, len);
      break;

    case 53:
      //skip DHCP_Text("DHCP Msg Type ", data, len);
      break;

    case 58:
      // Skip DHCP_Time("RENEWAL", data, len);
      break;

    case 59:
      // Skip DHCP_Time("REBIND", data, len);
      break;

    case 66:
      DHCP_Text("TFTP", data, len);
      break;

    case 67:
      DHCP_Text("PXE FILE", data, len);
      break;

    case 77:
      // Skip option
      break;

    case 119:
      DHCP_Search("SEARCH DOMAIN", data, len);
      break;

    case 255:
      // Skip option
      break;

    default: {
        byte i;
        char temp [len + 1] ;
        int a = 0;
        String temp1;
        for ( i = 0; i < ( len ); ++i , ++a) {
          temp[a] = data[i];
        }
        temp[len] = '\0';
        temp1  = temp;
        Serial.println("OPT: " + String(option) + " LEN:" +  String(len) + " Data:" + temp1 + '\n');
      }
  }
  OptionCount++;
}


void IPv4( String optlabel, const byte* data, uint8_t len) {
  byte address[4];
  String temp;
  for (unsigned int j = 0; j < len; ++j) {
    address[j] = data[j];
    temp += address[j] ;
    if (j < 3) {
      temp += ".";
    }
  }
  Serial.println(optlabel + ":" + temp);
  DHCP_info[OptionCount].Option[0] = optlabel;
  DHCP_info[OptionCount].Option[1] = temp;

}
void DHCP_Text(String optlabel, const byte* data, uint8_t len) {
  byte i;
  char temp [len + 1] ;
  int a = 0;
  String temp1;
  for ( i = 0; i < ( len ); ++i , ++a) {
    temp[a] = data[i];
  }
  temp[len] = '\0';
  temp1  = temp;
  Serial.println(optlabel + ":" + temp1);
  DHCP_info[OptionCount].Option[0] = optlabel;
  DHCP_info[OptionCount].Option[1] = temp1;
}

void DHCP_Search(String optlabel, const byte* data, uint8_t len) {
  char temp [len ] ;
  int a = 0;
  int b = 0;
  String temp1;

  unsigned int DataIndex = 0;
  while (DataIndex < len ) {
    unsigned int Searchlen = data[DataIndex];
    if (Searchlen == 0) {
      temp1 += "\n";
      DataIndex++;
    }
    else {
      DataIndex++;
      for (int  i = 0; i < Searchlen; ++i , ++a) {
        temp[a] = data[i + DataIndex ];
      }
      temp[Searchlen] = '\0';
      temp1 += temp;
      if ((DataIndex + Searchlen) != len && data[DataIndex + Searchlen] != 0) {
        temp1 += ".";
      }
      DataIndex += Searchlen;
      a = 0;
    }
  }
  Serial.println(optlabel + ":" + temp1);
  DHCP_info[OptionCount].Option[0] = optlabel;
  DHCP_info[OptionCount].Option[1] = temp1;
}

void DHCP_Time(String optlabel, const byte * data, uint8_t len) {
  unsigned long num = 0;
  int temp1;
  for (unsigned int i = 0; i < len; ++i) {
    num <<= 8;
    num += data[i];
  }
  temp1 =  (int)num;
  temp1 = ((temp1 / 60) / 60);
  Serial.println(optlabel + ":" + String(temp1) + "hrs");
  DHCP_info[OptionCount].Option[0] = optlabel;
  DHCP_info[OptionCount].Option[1] = String(temp1) + "hrs";
}
