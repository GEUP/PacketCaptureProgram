#ifndef PACKETHDR_H
#define PACKETHDR_H

struct packetPointer {
   void* packet;
   int packetType;
   int len;
};

struct ipv4hdr {
   unsigned char ipheaderlen;
   unsigned char protocolType;
   unsigned char srcIP[4];
   unsigned char dstIP[4];
};

struct tcphdr {
   struct ipv4hdr* ipH;
   unsigned int srcPort;
   unsigned int dstPort;
   unsigned long long seqNum;
   unsigned long long ackNum;
   unsigned char tcpheaderlen;
   unsigned char flagAck;
   unsigned char flagSyn;
   unsigned char flagFin;
};

struct httphdr {
   struct tcphdr* tcpH;
   unsigned char msg[1500];
};
struct httpReq {
   struct tcphdr* tcpH;
   unsigned char method[50];
   unsigned char url[50];
   unsigned char version[50];
};
struct httpRep {
   struct tcphdr* tcpH;
   unsigned char version[50];
   unsigned char statCode[50];
   unsigned char statText[50];
};

struct udphdr {
   struct ipv4hdr* ipH;
   unsigned int srcPort;
   unsigned int dstPort;
};

struct dnshdr {
   struct udphdr* udpH;
   unsigned char id[2];
   unsigned char qr;
   unsigned char msg[50];
};

struct icmp {
   struct ipv4hdr* ipH;
   unsigned char type;
   unsigned char code;
   unsigned char msg[50];
};
#endif
