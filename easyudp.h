
#ifndef _EASYUPD_H_
#define _EASYUDP_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX_DATA_SIZE	2048

typedef struct _udpData_ {
	int dataSize;
	int seqNum;
	char dataBuffer[MAX_DATA_SIZE];
} UdpData;

typedef struct _servDataInfo {
	int sock;
	int port;
	int sendCount;
	int maxSize;
	int seqNumStart;
	void (*callback) (struct _servDataInfo *sdi);
	char servIp[16];
	char bindIp[16];
	struct sockaddr_in from;
	UdpData udpData;
} SDI;

SDI *easyUdp(char *bindIp, char *servIp, int port, int seqNumStart, void (*callback)(SDI *sdi));

int easyUdpSend(SDI *sdi, char *dataBuffer, int dataSize);
int easyUdpRespond(SDI *sdi, char *dataBuffer, int dataSize);

void easyUdpFree(SDI *sdi);

#endif
