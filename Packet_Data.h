#include <EtherCard.h>
#include <Arduino.h>

#ifndef PACKET_DATA_H
#define PACKET_DATA_H

struct PINFO {
  String Proto;
  String ProtoVer;
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
  String VTP;
String Dup;
};

#endif
