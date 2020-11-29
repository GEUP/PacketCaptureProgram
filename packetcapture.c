#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/types.h>
#include "packethdr.h"
#include <stdlib.h>

#define BUF_LEN 100000
#define LIST_LEN 100000
int start = 0;
int fpwrite;
int sumFd;
int isCapture = 1;
struct packetPointer packetList[LIST_LEN];
int listNum = 0;
int display_packet(unsigned char* data, int len);
void signalhandler(int sig);
void orignStorePacket(unsigned char* data, int len, const int idx);
void packetCategory(unsigned char* data, int len, const int idx);
void printSummary(struct packetPointer* pP, const int idx, const int mode);
int httpPasing(unsigned char* msg, int end);
int isSameIP(unsigned char IP1[], unsigned char IP2[]);
int isConnected(unsigned char* ssIP, unsigned char* sdIP, unsigned int ssPort, unsigned int sdPort,
	unsigned char* psIP, unsigned char* pdIP, unsigned int psPort, unsigned int pdPort);

void httpFlowSearch(void);
void icmpFlowSearch(void);
void dnsFlowSearch(void);
int ipsearch(int nindex, const char nip[]);
int portsearch(int nindex, unsigned int nport);

int main(char argc, char* argv[])
{
	int sock, len;
	char UCinput;
	unsigned char buf[BUF_LEN];
	sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	int idx = 0;

	signal(SIGINT, signalhandler);

	fpwrite = open("orign.txt", O_CREAT | O_TRUNC | O_RDWR, 0666);
	sumFd = open("summary.txt", O_CREAT | O_TRUNC | O_RDWR, 0666);

	while (1) {

		printf("캡쳐시작   (y/n) : ");
		fflush(stdin);
		scanf("%c", &UCinput);
		getchar();

		if ((UCinput == 'n') || (UCinput == 'N')) {
			return 0;
		}
		else if ((UCinput == 'y') || (UCinput == 'Y')) {
			start = 1;
			printf("\n패킷 캡쳐 시작\n");
			while (start) {
				len = recvfrom(sock, buf, BUF_LEN, 0, NULL, NULL);
				//패킷 수집
				orignStorePacket(buf, len, idx);
				packetCategory(buf, len, idx++);
				listNum = idx;

			}
		}
		else {
			printf("잘못된 입력\n");
		}//n 또는 y 이외 다른 입력 들어오면 루프

	}
	printf("프로그램종료\n");
	return 0;
}

int display_packet(unsigned char* data, int len)
{
	int i;

	printf("\n------------------------------------------------------------------------\npacketsize: %d\n", len);
	for (i = 0; i < len; i++) {
		if ((i > 0) && ((i % 16) == 0)) {
			printf("\t");
			for (int j = i - 16; j < i; j++) {
				if (data[j] == 0x0c)
					printf("\\n ");
				else if (data[j] == 0)
					printf("0 ");
				else
					printf("%c ", data[j]);
			}
			printf("\n");
		}
		printf("%02x ", data[i]);
	}


	for (int j = 0; j <= 5 - (len % 16) / 4; j++)
		printf("\t");
	for (int j = i - (len % 16); j < len; j++) {
		if (data[j] == 0x0c)
			printf("\\n ");
		else if (data[j] == 0)
			printf("0 ");
		else
			printf("%c ", data[j]);
	}
	printf("\n");
}

void signalhandler(int sig) {
	char a;
	int e;
	char nip[20] = "-1";
	nip[19] = '\n';

	unsigned int nport = -1;


	fflush(stdout);

	if (!start)
		return;
	while (1) {
		printf("\n\nq. 프로그램 종료\n0. 검색 종료\n1. 일반검색\n2. 특수검색\n> ");
		fflush(stdin);
		scanf("%c", &a);
		getchar();
		if (a == 'q')
			exit(0);
		if (a == '0') {
			printf("\n패킷 캡쳐 시작\n");
			return;
		}
		else if (a == '1') { //일반검색

			int willprintList[LIST_LEN] = { 0, };
			int nindex = -1, nprotocol = -1;
			printf("\n\n------------------------일반검색------------------------\n");
			printf("검색방법 : index번호, ip번호, port번호, 프로토콜 종류 중 \n검색하고 싶은 항목의 정보를 입력하고 \n검색에서 제외할 항목은 '-1'을 입력해주세요.\n--------------------------------------------------------\n\n");

			printf("index 번호(ex >> 400 ) \n>> ");
			fflush(stdin);
			scanf("%d", &nindex);
			getchar();
			if (nindex != -1) {//index 검색 하는경우
				if (nindex >= listNum) {
					printf("잘못된 입력입니다.\n");
				}
				else
				{
					printSummary(&packetList[nindex], nindex, 1);
				}


			}
			else {// packetType = (0 = struct icmp/  1 = struct httphdr/ 3 = struct dnshdr/ 4= struct tcphdr/ 5= struct udphdr);

				printf("프로토콜 종류(ICMP=0, HTTP=1, DNS=3, TCP=4, UDP=5)\n>> ");
				fflush(stdin);
				scanf("%d", &nprotocol);
				getchar();

				printf("IP 번호(ex >> 127.0.0.1 )\n>> ");
				fflush(stdin);
				scanf("%s", nip);
				getchar();
				// 255이상일 경우 error 처리. 형식X일경우 error처리


				printf("PORT 번호(ex >> 80 )\n>> ");
				fflush(stdin);
				scanf("%u", &nport);
				getchar();

				for (int i = 0; i < listNum - 1; i++) {

					if (nprotocol == -1) {
						willprintList[i] = 1;
					}
					else   if (nprotocol == packetList[i].packetType) {
						willprintList[i] = 1;
					}
					else
					{
						willprintList[i] = 0;
					}


					if (!(strcmp(nip, "-1"))) {
					}
					else if (ipsearch(i, nip)) {
						willprintList[i] = willprintList[i] && 1;
					}
					else {
						willprintList[i] = willprintList[i] && 0;
					}


					if (nport == -1) {
					}
					else if (portsearch(i, nport)) {
						willprintList[i] = willprintList[i] && 1;
					}
					else {

						willprintList[i] = willprintList[i] && 0;
					}

				}

				for (int i = 0; i < listNum - 1; i++) {
					if (willprintList[i])
					{
						printSummary(&packetList[i], i, 1);
					}
				}

			}
		}
		else if (a == '2') {
			printf("특수검색 시작\n");
			printf("1. HTTP\n2.ICMP\n3.DNS\n>");
			fflush(stdin);
			scanf("%c", &a);
			getchar();
			if (a == '1') {
				printf("HTTP Packet\n----------------------------------------------------\n");
				httpFlowSearch();

			}
			else if (a == '2') {
				printf("ICMP Packet\n----------------------------------------------------\n");
				icmpFlowSearch();

			}
			else if (a == '3') {
				printf("DNS Packet\n----------------------------------------------------\n");
				dnsFlowSearch();
			}
			else {
				printf("잘못된 입력\n");
			}
		}
		else {
			printf("잘못된 입력\n");
		}
	}
	start = 0;
	return;

}

void orignStorePacket(unsigned char* data, int len, const int idx) {

	char type[5];
	char newdata[BUF_LEN];
	char orignData[BUF_LEN + 100];
	char* p;

	p = &newdata[0];

	//sprintf(type,"%02x%02x",data[12],data[13]);
	//printf("%s\n",type);
	for (int i = 0; i < len; i++) {
		p += sprintf(p, "%02x", data[i]);
	}
	sprintf(orignData, "%7d|%7d|%s\n", idx, len, newdata);
	write(fpwrite, orignData, strlen(orignData));

}

void packetCategory(unsigned char* data, int len, const int idx) {

	int protocol;
	char t;
	char ipheadelen;
	ipheadelen = data[14] & (0x0f);
	t = 14 + ipheadelen * 4;
	char str[BUF_LEN];
	if ((int)data[12] != 8) {
		printf("%7d|%7d|it is not IPv4\n", idx, len);
		return;
	}
	//출력, 사용은 16진수 %02x
	//ethernet
	//data[0] ~ data[13] : ethernet 정보 (14byte)

	//ip
	//data[14] : 상위 4bit : version - 저장X
	//           하위 4bit : ip 헤더 길이 정보 - ipheaderlen 변수에 저장 /*ipheaderlen = data[14.5]*4*/
	//data[23] : 프로토콜 타입 : ICMP - 1, TCP - 6, UDP - 17
	//data[25] ~ data[28] : 발신지 ip주소
	//data[29] ~ data[33] : 수신지 ip주소
	struct ipv4hdr* ipP = (struct ipv4hdr*)malloc(sizeof(struct ipv4hdr));
	ipP->ipheaderlen = ipheadelen;
	ipP->protocolType = data[23];
	for (int i = 0; i < 4; i++) {
		ipP->srcIP[i] = data[26 + i];
		ipP->dstIP[i] = data[30 + i];
	}

	protocol = (int)data[23];
	switch (protocol) {
	case 1:
	{
		//icmp
		//data[t]  : type 
		//data[t+1] : code
		//8-0일 경우 icmp acho request 메시지
		//0-0일 경우 icmp acho reply 메시지 두 정보만 저장

		struct icmp* packet = (struct icmp*)malloc(sizeof(struct icmp));
		packet->ipH = ipP;

		packet->type = data[t];
		packet->code = data[t + 1];
		if ((packet->type == 8) && (packet->code == 0))
			sprintf(packet->msg, "icmp acho request");
		else if ((packet->type == 0) && (packet->code == 0))
			sprintf(packet->msg, "icmp acho reply");
		else
			sprintf(packet->msg, "icmp msg");

		//printf("%s", str);
		packetList[idx].packet = packet;
		packetList[idx].packetType = 0;
		packetList[idx].len = len;
		printSummary(&packetList[idx], idx, 0);


		break;
	}
	case 6:
	{
		//tcp일때
		   //t=14+ipheaderlen;
		   //data[t] ~ data[t+1] : 발신지 port번호
		   //data[t+2] ~ data[t+3] : 수신지 port번호
		   //data[t+4] ~ data[t+7] : seq num
		   //data[t+8] ~ data[t+11] :ack num
		   //data[t+12]&11110000 : 상위 4bit : tcp 헤더 길이 - tcpheaderlen 변수에 저장
		   //data[t+13]&00010000 : ack 플래그
		   //data[t+13]&00000010 : syn 플래그
		   //data[t+13]&00000001 : fin 플래그

		struct tcphdr* tcpP = (struct tcphdr*)malloc(sizeof(struct tcphdr));
		tcpP->ipH = ipP;

		tcpP->srcPort = ((int)data[t] << 8 | (int)data[t + 1]);
		tcpP->dstPort = ((int)data[t + 2] << 8 | (int)data[t + 3]);
		tcpP->seqNum = ((long long)data[t + 4] << 24 | (long long)data[t + 5] << 16 | (long long)data[t + 6] << 8 | (long long)data[t + 7]);
		tcpP->ackNum = ((long long)data[t + 8] << 24 | (long long)data[t + 9] << 16 | (long long)data[t + 10] << 8 | (long long)data[t + 11]);

		tcpP->tcpheaderlen = (data[t + 12] & 0xF0) >> 4;
		tcpP->flagAck = (data[t + 13] & 0x1F) >> 4;
		tcpP->flagSyn = (data[t + 13] & 0x02) >> 1;
		tcpP->flagFin = data[t + 13] & 0x01;
		if (tcpP->dstPort == 80 || tcpP->srcPort == 80) {//http 패킷
			int ht = t + tcpP->tcpheaderlen * 4;
			struct httphdr* httpP = (struct httphdr*)malloc(sizeof(struct httphdr));
			httpP->tcpH = tcpP;
			unsigned char* msgPointer = &data[ht];
			int n = httpPasing(&data[ht], len - ht - 1);
			if (n != -1) {
				strncpy(httpP->msg, msgPointer, n);
				packetList[idx].packet = httpP;
				packetList[idx].packetType = 1;
				packetList[idx].len = len;
				printSummary(&packetList[idx], idx, 0);

			}
			else {
				packetList[idx].packet = tcpP;
				packetList[idx].packetType = 4;
				packetList[idx].len = len;
				printSummary(&packetList[idx], idx, 0);

			}
			//http
			//ht = t +tcpP->tcpheaderlen*4
			//응답 - HTTP/1.1 부터 나오는거
			//data[ht] 시작점 ~/r/n 까지 :HTTP/1.1 200 OK
			//그 다음줄 ~ /r/n 까지 : 날찌/시간
			//그 다음줄 ~ /r/n 까지 : content-Type: text/css
			//content-Length: 17284
			//

			//요청 - GET 부터 나오는거
		}
		else {
			packetList[idx].packet = tcpP;
			packetList[idx].packetType = 4;
			packetList[idx].len = len;
			printSummary(&packetList[idx], idx, 0);

		}
		break;
	}
	case 17:
	{
		//udp
		//t=14+ipheaderlen
		//data[t] ~ data[t+1] : 발신지 port번호
		//data[t+2] ~ data[t+3] : 수신지 port번호

		struct udphdr* udpP = (struct udphdr*)malloc(sizeof(struct udphdr));
		udpP->ipH = ipP;
		udpP->srcPort = ((int)data[t] << 8 | (int)data[t + 1]);
		udpP->dstPort = ((int)data[t + 2] << 8 | (int)data[t + 3]);

		if (udpP->srcPort == 53 || udpP->dstPort == 53) { //dns 구분
			int dt = t + 8;
			struct dnshdr* dnsP = (struct dnshdr*)malloc(sizeof(struct dnshdr));
			dnsP->udpH = udpP;
			dnsP->id[0] = data[dt];
			dnsP->id[1] = data[dt + 1];
			dnsP->qr = (data[dt + 2] & 0x80) >> 7;
			packetList[idx].packet = dnsP;
			packetList[idx].packetType = 3;
			packetList[idx].len = len;
			printSummary(&packetList[idx], idx, 0);
		}
		else {
			packetList[idx].packet = udpP;
			packetList[idx].packetType = 5;
			packetList[idx].len = len;
			printSummary(&packetList[idx], idx, 0);
		}

		break;
	}
	default:
	{
		packetList[idx].packet = NULL;
		packetList[idx].packetType = -1;
		packetList[idx].len = len;
		printSummary(&packetList[idx], idx, 0);
		break;

	}
	}








	//dns
	//만약 udp 발신, 수신 포트번호 중 하나가 53일경우 dns로 판별



}

//printSummary(&packetList[idx],idx); 사용
void printSummary(struct packetPointer* pP, const int idx, const int mode) {
	char str[BUF_LEN];

	switch (pP->packetType) {
	case 0:
	{
		struct icmp* packet = (struct icmp*)(pP->packet);

		sprintf(str, "%7d|%7d|%s|srcIP=%d.%d.%d.%d|dstIP=%d.%d.%d.%d|type=%d|code=%d|msg=\"%s\"\n", idx, pP->len, "ICMP", packet->ipH->srcIP[0], packet->ipH->srcIP[1], packet->ipH->srcIP[2], packet->ipH->srcIP[3], packet->ipH->dstIP[0], packet->ipH->dstIP[1], packet->ipH->dstIP[2], packet->ipH->dstIP[3], packet->type, packet->code, packet->msg);
		break;

	}
	case 1:
	{
		struct httphdr* packet = (struct httphdr*)(pP->packet);
		sprintf(str, "%7d|%7d|%s|srcIP=%d.%d.%d.%d|dstIP=%d.%d.%d.%d|SrcPORT=%u|DstPORT=%u|seq=%lld|ack=%lld|tcpheaderlen=%d|flagAck=%d|flagSyn=%d|flagFin=%d\n\t\t\t-----httpMsg-----\n\t\t\t\t%s\n\n",
			idx, pP->len, "HTTP",
			packet->tcpH->ipH->srcIP[0], packet->tcpH->ipH->srcIP[1], packet->tcpH->ipH->srcIP[2], packet->tcpH->ipH->srcIP[3],
			packet->tcpH->ipH->dstIP[0], packet->tcpH->ipH->dstIP[1], packet->tcpH->ipH->dstIP[2], packet->tcpH->ipH->dstIP[3],
			packet->tcpH->srcPort, packet->tcpH->dstPort, packet->tcpH->seqNum, packet->tcpH->ackNum, packet->tcpH->tcpheaderlen,
			packet->tcpH->flagAck, packet->tcpH->flagSyn, packet->tcpH->flagFin, packet->msg);


		break;
	}
	case 3:
	{
		struct dnshdr* packet = (struct dnshdr*)(pP->packet);
		sprintf(str, "%7d|%7d|%s|srcIP=%d.%d.%d.%d|dstIP=%d.%d.%d.%d|SrcPORT=%u|DstPORT=%u|dnsId :0x%02x%02x|dnsQR=%d\n", idx, pP->len, "DNS", packet->udpH->ipH->srcIP[0], packet->udpH->ipH->srcIP[1], packet->udpH->ipH->srcIP[2], packet->udpH->ipH->srcIP[3], packet->udpH->ipH->dstIP[0], packet->udpH->ipH->dstIP[1], packet->udpH->ipH->dstIP[2], packet->udpH->ipH->dstIP[3], packet->udpH->srcPort, packet->udpH->dstPort, packet->id[0], packet->id[1], packet->qr);

		break;
	}
	case 4:
	{

		struct tcphdr* packet = (struct tcphdr*)(pP->packet);
		sprintf(str, "%7d|%7d|%s|srcIP=%d.%d.%d.%d|dstIP=%d.%d.%d.%d|SrcPORT=%u|DstPORT=%u|seq=%lld|ack=%lld|tcpheaderlen=%d|flagAck=%d|flagSyn=%d|flagFin=%d\n", idx, pP->len, "TCP",
			packet->ipH->srcIP[0], packet->ipH->srcIP[1], packet->ipH->srcIP[2], packet->ipH->srcIP[3],
			packet->ipH->dstIP[0], packet->ipH->dstIP[1], packet->ipH->dstIP[2], packet->ipH->dstIP[3],
			packet->srcPort, packet->dstPort, packet->seqNum, packet->ackNum, packet->tcpheaderlen,
			packet->flagAck, packet->flagSyn, packet->flagFin
		);


		break;
	}
	case 5:
	{

		struct udphdr* packet = (struct udphdr*)(pP->packet);
		sprintf(str, "%7d|%7d|%s|srcIP=%d.%d.%d.%d|dstIP=%d.%d.%d.%d|SrcPORT=%u|DstPORT=%u\n", idx, pP->len, "UDP",
			packet->ipH->srcIP[0], packet->ipH->srcIP[1], packet->ipH->srcIP[2], packet->ipH->srcIP[3],
			packet->ipH->dstIP[0], packet->ipH->dstIP[1], packet->ipH->dstIP[2], packet->ipH->dstIP[3],
			packet->srcPort, packet->dstPort
		);



		break;
	}
	default:
	{
		printf("%7d|%7d|i don't know this packet\n", idx, pP->len);
		break;
	}
	}
	if (mode == 0) {
		write(sumFd, str, strlen(str));
	}
	if (!start)
		return;
	write(1, str, strlen(str));
	return;

}

int httpPasing(unsigned char msg[], const int end)
{
	int i;
	for (i = 0; i < end - 1; i++) {

		if ((msg[i - 1] == 0x0D) && (msg[i] == 0x0A)) {
			msg[i] = '\n';
			return i;
		}
		if (i > 100)
			return i;

	}
	return -1;
}

int isSameIP(unsigned char IP1[], unsigned char IP2[])
{
	int same = 1;
	for (int i = 0; i < 4; i++) {
		same = same && (IP1[i] == IP2[i]);
	}
	return same;
}

int isConnected(unsigned char* ssIP, unsigned char* sdIP, unsigned int ssPort, unsigned int sdPort,
	unsigned char* psIP, unsigned char* pdIP, unsigned int psPort, unsigned int pdPort)
{
	int same1 = 0; //저장된 srcIP가 목적지IP거나 출발지IP인 패킷인지 확인
	int same2 = 0; //저장된 dstIP가 목적지IP거나 출발지IP인 패킷인지 확인
	int same3 = 0; //저장된 srcPort가 목적지Port거나 출발지Port인 패킷인지 확인
	int same4 = 0; //저장된 dstPort가 목적지Port거나 출발지Port인 패킷인지 확인
	same1 = isSameIP(ssIP, psIP);
	same1 = same1 || isSameIP(ssIP, pdIP);

	same2 = isSameIP(sdIP, psIP);
	same2 = same2 || isSameIP(sdIP, pdIP);

	same3 = (ssPort == psPort);
	same3 = same3 || (ssPort == pdPort);

	same4 = (sdPort == psPort);
	same4 = same4 || (sdPort == pdPort);

	return same1 && same2 && same3 && same4;
}

// packetType = (0 = struct icmp/  1 = struct httphdr/ 3 = struct dnshdr/ 4= struct tcphdr/ 5= struct udphdr);
void httpFlowSearch(void)
{
	int isPrinted[LIST_LEN] = { 0, };
	int oldStart = 0;
	int start = 0;
	struct tcphdr* tcpP;
	struct httphdr* httpP;
	unsigned char srcIP[4];
	unsigned char dstIP[4];
	unsigned int srcPort;
	unsigned int dstPort;

	/*
	   1. start부터 packetList를 검색하면서 tcphdr 헤더의 flagSyn 이 1인 패킷의 목적지/수신지 IP/Port 저장하고 start에 해당 패킷의 idx를 저장한다.
	   2. 1번에서 저장한 IP/Port 와 목적지/수신지IP/Port 가 동일한 패킷이 한 번도 출력된적 없다면 모두 출력하고 isPrinted[i]를 1로 set한다.
	   3. start가 증가하지 않을 때(oldStart == Start)까지 1~2를 반복한다.

	*/
	while (1) {

		for (int i = start; i < listNum; i++) {
			if (packetList[i].packetType == 4)
			{
				tcpP = packetList[i].packet;
				if ((tcpP->flagSyn == 1) && (isPrinted[i] == 0) && (tcpP->dstPort == 80)) {
					for (int j = 0; j < 4; j++) {
						srcIP[j] = tcpP->ipH->srcIP[j];
						dstIP[j] = tcpP->ipH->dstIP[j];
					}
					srcPort = tcpP->srcPort;
					dstPort = tcpP->dstPort;
					start = i;
					break;
				}
			}
		}


		for (int i = start; i < listNum; i++) {
			if (isPrinted[i] == 0) {
				if (packetList[i].packetType == 4) {
					tcpP = packetList[i].packet;
					if (isConnected(srcIP, dstIP, srcPort, dstPort, tcpP->ipH->srcIP, tcpP->ipH->dstIP, tcpP->srcPort, tcpP->dstPort)) {
						printSummary(&packetList[i], i, 1);
						isPrinted[i] = 1;
					}

				}
				else if (packetList[i].packetType == 1) {
					httpP = packetList[i].packet;

					if (isConnected(srcIP, dstIP, srcPort, dstPort, httpP->tcpH->ipH->srcIP, httpP->tcpH->ipH->dstIP, httpP->tcpH->srcPort, httpP->tcpH->dstPort)) {
						printf("HTTP Packet TEST\n");
						printSummary(&packetList[i], i, 1);
						isPrinted[i] = 1;
					}

				}
			}
		}
		if (oldStart == start)
			break;
		else {
			oldStart = start;
			printf("\n----------------------------------------------------\n");
		}
	}

	return;
}
void icmpFlowSearch(void)
{
	int isPrinted[LIST_LEN] = { 0, };
	int oldStart = 0;
	int start = 0;
	struct icmp* icmpP;
	unsigned char srcIP[4];
	unsigned char dstIP[4];
	/*
	   1. start부터 packetList를 검색하면서 icmp 패킷을 찾고, 목적지/수신지 IP를 저장하고 start에 해당 패킷의 idx를 저장한다.
	   2. 1번에서 저장한 IP 와 목적지/수신지IP 가 동일한 패킷이 한 번도 출력된적 없다면 모두 출력하고 isPrinted[i]를 1로 set한다.
	   3. start가 증가하지 않을 때(oldStart == Start)까지 1~2를 반복한다.

	*/
	while (1) {

		for (int i = start; i < listNum; i++) {
			if ((packetList[i].packetType == 0) && (isPrinted[i] == 0))
			{
				icmpP = packetList[i].packet;

				for (int j = 0; j < 4; j++) {
					srcIP[j] = icmpP->ipH->srcIP[j];
					dstIP[j] = icmpP->ipH->dstIP[j];
				}

				start = i;
				break;
			}
		}


		for (int i = start; i < listNum; i++) {
			if (isPrinted[i] == 0) {
				if (packetList[i].packetType == 0) {
					icmpP = packetList[i].packet;
					if (isConnected(srcIP, dstIP, 0, 0, icmpP->ipH->srcIP, icmpP->ipH->dstIP, 0, 0)) {
						printSummary(&packetList[i], i, 1);
						isPrinted[i] = 1;
					}

				}
			}
		}
		if (oldStart == start)
			break;
		else {
			oldStart = start;
			printf("\n----------------------------------------------------\n");
		}
	}
	return;
}
void dnsFlowSearch(void)
{
	int isPrinted[LIST_LEN] = { 0, };
	int oldStart = 0;
	int start = 0;
	struct dnshdr* dnsP;
	unsigned char srcIP[4];
	unsigned char dstIP[4];
	unsigned int srcPort;
	unsigned int dstPort;

	/*
	   1. start부터 packetList를 검색하면서 dns 패킷의 목적지/수신지 IP/Port 저장하고 start에 해당 패킷의 idx를 저장한다.
	   2. 1번에서 저장한 IP/Port 와 목적지/수신지IP/Port 가 동일한 패킷이 한 번도 출력된적 없다면 모두 출력하고 isPrinted[i]를 1로 set한다.
	   3. start가 증가하지 않을 때(oldStart == Start)까지 1~2를 반복한다.

	*/
	while (1) {

		for (int i = start; i < listNum; i++) {
			if (packetList[i].packetType == 3)
			{
				dnsP = packetList[i].packet;
				if ((isPrinted[i] == 0) && (dnsP->udpH->dstPort == 53)) {
					for (int j = 0; j < 4; j++) {
						srcIP[j] = dnsP->udpH->ipH->srcIP[j];
						dstIP[j] = dnsP->udpH->ipH->dstIP[j];
					}
					srcPort = dnsP->udpH->srcPort;
					dstPort = dnsP->udpH->dstPort;
					start = i;
					break;
				}
			}
		}


		for (int i = start; i < listNum; i++) {
			if (isPrinted[i] == 0) {
				if (packetList[i].packetType == 3) {
					dnsP = packetList[i].packet;
					if (isConnected(srcIP, dstIP, srcPort, dstPort, dnsP->udpH->ipH->srcIP, dnsP->udpH->ipH->dstIP, dnsP->udpH->srcPort, dnsP->udpH->dstPort)) {
						printSummary(&packetList[i], i, 1);
						isPrinted[i] = 1;
					}

				}
			}
		}
		if (oldStart == start)
			break;
		else {
			oldStart = start;
			printf("\n----------------------------------------------------\n");
		}
	}
	return;
}
int ipsearch(int nindex, const char nip2[]) {
	//ip 배열 생성
	int ipsametmpp = 0; //같으면 1
	unsigned char ip1[4];//입력받은 ip저장
	unsigned char ip2[4];//src ip
	unsigned char ip3[4];//dst ip            
	unsigned char* token;
	char nip[20];
	strncpy(nip, nip2, 19);
	token = strtok(nip, ".");
	for (int i = 0; i < 4; i++) {

		if (atoi(token) > 255)
		{
			printf("IP 입력 값이 너무 큽니다.\n");
			exit(-1);
		}
		ip1[i] = (unsigned char)atoi(token);
		token = strtok(NULL, ".");

	}


	switch (packetList[nindex].packetType) {
	case 0://icmp일 경우
	{
		struct icmp* icmppa = (struct icmp*)(packetList[nindex].packet);
		icmppa = packetList[nindex].packet;
		for (int i = 0; i < 4; i++) {//비교할 ip 저장
			ip2[i] = icmppa->ipH->srcIP[i];
			ip3[i] = icmppa->ipH->dstIP[i];
		}
		ipsametmpp = isSameIP(ip1, ip2) || isSameIP(ip1, ip3);
		break;
	}
	case 1://httphdr
	{
		struct httphdr* httppa = (struct httphdr*)(packetList[nindex].packet);
		httppa = packetList[nindex].packet;
		for (int i = 0; i < 4; i++) {//비교할 ip 저장
			ip2[i] = httppa->tcpH->ipH->srcIP[i];
			ip3[i] = httppa->tcpH->ipH->dstIP[i];
		}
		ipsametmpp = isSameIP(ip1, ip2) || isSameIP(ip1, ip3);
		break;
	}
	case 3://dns
	{
		struct dnshdr* dnspa = (struct dnshdr*)(packetList[nindex].packet);
		dnspa = packetList[nindex].packet;
		for (int i = 0; i < 4; i++) {//비교할 ip 저장
			ip2[i] = dnspa->udpH->ipH->srcIP[i];
			ip3[i] = dnspa->udpH->ipH->dstIP[i];
		}
		ipsametmpp = isSameIP(ip1, ip2) || isSameIP(ip1, ip3);
		break;
	}
	case 4://tcp
	{
		struct tcphdr* tcppa = (struct tcphdr*)(packetList[nindex].packet);
		tcppa = packetList[nindex].packet;
		for (int i = 0; i < 4; i++) {//비교할 ip 저장
			ip2[i] = tcppa->ipH->srcIP[i];
			ip3[i] = tcppa->ipH->dstIP[i];
		}
		ipsametmpp = isSameIP(ip1, ip2) || isSameIP(ip1, ip3);
		break;
	}
	case 5: //udp
	{
		struct udphdr* udppa = (struct udphdr*)(packetList[nindex].packet);
		udppa = packetList[nindex].packet;
		for (int i = 0; i < 4; i++) {//비교할 ip 저장
			ip2[i] = udppa->ipH->srcIP[i];
			ip3[i] = udppa->ipH->dstIP[i];
		}
		ipsametmpp = isSameIP(ip1, ip2) || isSameIP(ip1, ip3);
		break;
	}
	default:
	{
		ipsametmpp = 0;
		break;
	}
	}//switch 종료
	return ipsametmpp;

}

int portsearch(int nindex, unsigned int nport) {
	//ip 배열 생성
	int portsametmpp = 0; //같으면 1

	unsigned int nsrcPort;//src p
	unsigned int ndstPort;//dst p            

	switch (packetList[nindex].packetType) {
	case 0://icmp일 경우
	{
		portsametmpp = 0;
		break;
	}
	case 1://httphdr
	{
		struct httphdr* httppa = (struct httphdr*)(packetList[nindex].packet);
		httppa = packetList[nindex].packet;
		portsametmpp = (nport == httppa->tcpH->srcPort) || (nport == httppa->tcpH->dstPort);
		break;
	}
	case 3://dns
	{
		struct dnshdr* dnspa = (struct dnshdr*)(packetList[nindex].packet);
		dnspa = packetList[nindex].packet;
		portsametmpp = (nport == dnspa->udpH->srcPort) || (nport == dnspa->udpH->dstPort);
		break;
	}
	case 4://tcp
	{
		struct tcphdr* tcppa = (struct tcphdr*)(packetList[nindex].packet);
		tcppa = packetList[nindex].packet;
		portsametmpp = (nport == tcppa->srcPort) || (nport == tcppa->dstPort);
		break;
	}
	case 5: //udp
	{
		struct udphdr* udppa = (struct udphdr*)(packetList[nindex].packet);
		udppa = packetList[nindex].packet;
		portsametmpp = (nport == udppa->srcPort) || (nport == udppa->dstPort);
		break;
	}
	default:
	{
		portsametmpp = 0;
		break;
	}
	}//switch 종료
	return portsametmpp;

}
