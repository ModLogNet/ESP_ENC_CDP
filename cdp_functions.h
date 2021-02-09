#include <EtherCard.h>
#include <Arduino.h>
#include "Packet_data.h";

#ifndef CDP_FUNCTIONS_H
#define CDP_FUNCTIONS_H

struct cdp_info {


  String Name;
  String MAC;
  String Port;
  String PortDesc;
  String Model;
  String VLAN;
  String IP;
  String VoiceVLAN;
  String Cap;
  String SWver;
  String TTL;

};


unsigned int cdp_check_Packet(int plen, byte EthBuffer[1500], int bufSize );
PINFO cdp_packet_handler( byte cdpData[], size_t cdpDataIndex, size_t plen);
String print_mac(const byte a[], unsigned int offset, unsigned int length);
bool byte_array_contains(const byte a[], unsigned int offset, const byte b[], unsigned int length);
String CdpCapabilities(String temp);
String handleCdpCapabilities( const byte a[], unsigned int offset, unsigned int lengtha);
String print_binary(int v, int num_places);
String handleCdpNumField( const byte a[], unsigned int offset, unsigned int length);
void handleCdpVoiceVLAN( const byte a[], unsigned int offset, unsigned int length);
boolean handleCdpAddresses(const byte a[], unsigned int offset, unsigned int length);
String handleCdpDuplex(const byte a[], unsigned int offset, unsigned int length);
String handleCdpAsciiField(byte a[], unsigned int offset, unsigned int lengtha);
String print_ip(const byte a[], unsigned int offset, unsigned int length);
void print_byte(byte a[], int psize);
char val2dec(byte b);
void cdp_getHEX(const byte a[], unsigned int offset, unsigned int length) ;
#endif