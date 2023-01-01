
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#define _GNU_SOURCE
#include <search.h>

#include "dts.h"
#include "easyudp.h"

#define null			NULL
#define MAX_SEQ_NUMS	10
#define DEFAULT_PORT	8383

SDI *sdi = NULL;

void *dtsTree = NULL;
int idx = 0;
int lastSeqNums[MAX_SEQ_NUMS];
int udpPort = DEFAULT_PORT;
void callback(SDI *sdi);

int main (int argc, char *argv[]) {

	memset(lastSeqNums, 0, sizeof(lastSeqNums));

	sdi = easyUdp("192.168.0.15", "192.168.0.15", udpPort, 1, &callback);
	if( sdi == NULL) {
		printf("Can not create UDP socket.\n");
		exit(1);
	}

	sdi->sendCount = 2;

	dtsInit();

	while(true) {
		sleep(5);
	}

	easyUdpFree(sdi);

	return 0;
}

void callback(SDI *sdi) {
	bool seenFlag = false;
	int sn = 0;
	int ds = 0;

	sn = ntohl(sdi->udpData.seqNum);
	ds = ntohl(sdi->udpData.dataSize);

	// Check if we have seen this seqNum before.
	for (int i = 0; i < MAX_SEQ_NUMS; i++) {
		if (lastSeqNums[i] == sn) {
			// we have seen this seqnum before.
			seenFlag = true;
		}
	}

	if (seenFlag == true)
		return;

	if (idx >= MAX_SEQ_NUMS)
		idx = 0;

	lastSeqNums[idx++] = sn;

	// printf("seqNum: %d\n", sn);

	sdi->udpData.dataBuffer[ds] = '\0';
	char *packet = dtsParse(sdi->udpData.dataBuffer, ds);
	if (packet == NULL) {
		printf("Error in parsing DTS data.\n");
	} else {
		printf("Server: %s\n", packet);
		easyUdpRespond(sdi, packet, 0);
	}
}
