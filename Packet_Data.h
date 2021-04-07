#include <EtherCard.h>
#include <Arduino.h>

#ifndef PACKET_DATA_H
#define PACKET_DATA_H

struct PINFO {

  String ChassisID[2] = {"ChassisID", "EMPTY"};
  String Proto[2] = {"Proto", "EMPTY"};
  String ProtoVer[2] = {"ProtoVer", "EMPTY"};
  String Name[2] = {"Name", "EMPTY"};
  String MAC[2] = {"MAC", "EMPTY"};
  String Port[2] = {"Port", "EMPTY"};
  String PortDesc[2] = {"PortDesc", "EMPTY"};
  String Model[2] = {"Model", "EMPTY"};
  String VLAN[2] = {"VLAN", "EMPTY"};
  String IP[2] = {"IP", "EMPTY"};
  String VoiceVLAN[2] = {"VoiceVLAN", "EMPTY"};
  String Cap[2] = {"Cap", "EMPTY"};
  String SWver[2] = {"SWver", "EMPTY"};
  String TTL[2] = {"TTL", "EMPTY"};
  String VTP[2] = {"VTP", "EMPTY"};
  String Dup[2] = {"Dup", "EMPTY"};
  String Checksum[2] = {"Checksum", "EMPTY"};
};


#endif
