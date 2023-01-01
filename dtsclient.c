
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include "dts.h"
#include "easyudp.h"

#define MAX_SEQ_NUMS		10

int idx = 0;
int lastSeqNums[MAX_SEQ_NUMS];
SDI *sdi = NULL;

void clientCallback(SDI *sdi);

int main(int argc, char *argv[]) {

	sdi = easyUdp("192.168.0.24", "192.168.0.15", 8383, 2, clientCallback);
	if( sdi == NULL) {
        printf("Can not create UDP socket.\n");
        exit(1);
    }

	sdi->sendCount = 2;		// Set send number count, how many times the packet is sent.

	sleep(2);

	easyUdpSend(sdi, "kelly:number:set 12378", 0);
	easyUdpSend(sdi, "wiles:number:set 2345;richard:string:set 'Kelly Wiles'", 0);

	easyUdpSend(sdi, "kelly:number:get", 0);
	easyUdpSend(sdi, "wiles:number:get;richard:string:get", 0);

	while (true) {
		sleep(2);
	}

	easyUdpFree(sdi);

	return 0;
}

void clientCallback(SDI *sdi) {
	 bool seenFlag = false;

	 int sn = ntohl(sdi->udpData.seqNum);
	 int ds = ntohl(sdi->udpData.dataSize);

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

	sdi->udpData.dataBuffer[ds] = '\0';
	printf("client: %s, %d\n", sdi->udpData.dataBuffer, ds);
}
