#include <EtherCard.h>
#include <Arduino.h>
#include "Packet_data.h";

#ifndef CDP_FUNCTIONS_H
#define CDP_FUNCTIONS_H

//Main fuctions
unsigned int cdp_check_Packet(int plen, byte EthBuffer[], int bufSize );
PINFO  cdp_packet_handler( byte cdpData[],  size_t   plen);

//Packet Handling Functions
String handleCdpCapabilities( const byte a[], unsigned int offset, unsigned int lengtha);
String handleCdpNumField( const byte a[], unsigned int offset, unsigned int length);
String handleCdpVoiceVLAN( const byte a[], unsigned int offset, unsigned int length);
String handleCdpAddresses(const byte a[], unsigned int offset, unsigned int length);
String handleCdpDuplex(const byte a[], unsigned int offset, unsigned int length);
String handleCdpAsciiField(byte a[], unsigned int offset, unsigned int lengtha);

//Supporting Functions
bool   byte_array_contains(const byte a[], unsigned int offset, const byte b[], unsigned int length);
String print_mac(const byte a[], unsigned int offset, unsigned int length);
String CdpCapabilities(String temp);
String print_binary(int v, int num_places);
String print_ip(const byte a[], unsigned int offset, unsigned int length);
String print_mac(const byte a[], unsigned int offset, unsigned int length);
char   val2dec(byte b);
String cdp_getHEX(const byte a[], unsigned int offset, unsigned int length) ;

#endif
