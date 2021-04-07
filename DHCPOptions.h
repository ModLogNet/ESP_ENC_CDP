#include <EtherCard.h>
#include <Arduino.h>
#include "Packet_data.h";


#ifndef DHCPOPTIONS_H
#define DHCPOPTIONS_H

struct DHCP_DATA {
  String Option[2]={"EMPTY","EMPTY"};
};

void DHCPOption(uint8_t option, const byte* data, uint8_t len);
void IPv4( String optlabel, const byte* data, uint8_t len);
void DHCP_Text(String optlabel, const byte* data, uint8_t len);
void DHCP_Time(String optlabel, const byte* data, uint8_t len);
void DHCP_Search(String optlabel, const byte* data, uint8_t len);


#endif
