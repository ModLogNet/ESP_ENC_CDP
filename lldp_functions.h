#include <EtherCard.h>
#include <Arduino.h>
#include "Packet_Data.h";
#ifndef LLDP_FUNCTIONS_H
#define LLDP_FUNCTIONS_H

unsigned int lldp_check_Packet(int plen, byte EthBuffer[], int bufSize );
PINFO lldp_packet_handler( byte cdpData[],  uint16_t plen);
String handleLLDPIPField(const byte a[], unsigned int offset, unsigned int lengtha);
String handlelldpAsciiField(byte a[], unsigned int offset, unsigned int lengtha);
void handlelldpCapabilities( const byte a[], unsigned int offset, unsigned int lengtha);
String lldp_print_binary(int v, int num_places);
String LldpCapabilities(String temp);
String lldp_print_mac(const byte a[], unsigned int offset, unsigned int length);
bool lldp_byte_array_contains(const byte a[], unsigned int offset, const byte b[], unsigned int length);
String lldp_handleCdpNumField( const byte a[], unsigned int offset, unsigned int length);

String handleportsubtype(byte cdpData[], unsigned int cdpDataIndex, unsigned int lldpFieldLength);
String handleManagementSubtype(byte cdpData[], unsigned int cdpDataIndex, unsigned int lldpFieldLength);
#endif
