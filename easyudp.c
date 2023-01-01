/* easyudp.c - An example of how to move data across a UDP/IP connection.
 * Written by Richard Kelly Wiles
 * Date 08/10/2022
 * This software is free to use as long as this comment section is left unchanged at the top of the file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#include "easyudp.h"

/* UDP is much easier to deal with than TCP but it is not reliable, so you may lose packets.
 * Packets are not normally lost on a local area network LAN but if your packets need to go over Internet
 * or over a wireless network, you will lose packets.  Wireless networks are the more likly to lose
 * packets because of radios trying to send a packet at the same time as another radio.  This causes
 * signal to be corrupted and large the packet or large number of radios in the area wanting to send data
 * at the same time will cause greater packet lose.
 * If lost packets are important then add a header to each UDP packet sent that contains
 * a sequence number.  To help with lost packets you can send each packet twice and the
 * reciever can ignore the second packet.
 *
 * This set of functions were created to send data in one direction, from sender to receiver.  This makes it
 * easier to deal with.  If you want to responsed to data being received I would use these functions again be in
 * the other direction.  This would mean you would have a server on the sender as will as the receiver side.
 *
 * Lets say you are sending data to a central server that needs to record and acknowledge the receipt of the data.
 * The server would verify that the seqNum is correct and that it has not already process this seqNum.
 * It would then send back a packet with a different seqNum but the data would comntain the seqNum it is verifing.
 * Both the sender and receiver would send the packet N times with the same seqNum, the sender and receiver would
 * ignore packets that have a seqNum they have already processed.
 * I will leave the resending of lost packets for you to handle.
 */

bool stopThread = false;
int maxMsgSize = 0;
int _seqNum = 0;
char *maxBuffer = NULL;

void *_easyUdpCapture(void *param);

// Create a UDP server.
// The ipAddr can be an IP address of a local nic card or a value of NULL means listen on all local addreesses.
// The max size of a data packet is controlled by the #define MAX_DATA_SIZE in easyudp.h.
SDI *easyUdp(char *bindIp, char *servIp, int port, int seqNumStart, void (*callback)(SDI *sdi)) {
	int sock;
	struct sockaddr_in     servaddr;

	// Creating socket.
	if ( (sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		printf("Socket creation failed. %s\n", strerror(errno));
		return(NULL);
	} 

	memset(&servaddr, 0, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	if (bindIp == NULL)
		servaddr.sin_addr.s_addr = INADDR_ANY;	// Bind to all interfaces.
	else
		servaddr.sin_addr.s_addr = inet_addr(bindIp);

	// Bind the socket.
	if ( bind(sock, (const struct sockaddr *)&servaddr,  sizeof(servaddr)) < 0 ) {
		printf("Bind failed. %s", strerror(errno));
		return(NULL);
	}

	_seqNum = seqNumStart;

	// Set up returning structure.
	SDI *sdi = (SDI *)calloc(1, sizeof(SDI));
	strcpy(sdi->bindIp, bindIp);
	strcpy(sdi->servIp, servIp);
	sdi->sock = sock;
	sdi->maxSize = MAX_DATA_SIZE;
	sdi->port = port;
	sdi->seqNumStart = seqNumStart;
	sdi->callback = callback;
	sdi->udpData.seqNum = sdi->seqNumStart;

	pthread_t thd;

	// The thread is passed needed information in the Params structure.
	pthread_create(&thd, NULL, _easyUdpCapture, (void *) sdi);

	return sdi;
}

// Only to be called by the easyUdpServer function.
void *_easyUdpCapture(void *param) {
	int n;
    unsigned int len;

	SDI *sdi = (SDI *)param;

	len = sizeof(sdi->from);

	UdpData *ud = &(sdi->udpData);

	while (stopThread == false) {
		// printf("Waiting for packet.\n");
		// Endianness is handled in callback function.
		n = recvfrom(sdi->sock, (char *)ud,
				(sdi->maxSize + (sizeof(int) * 2)),
				0,
				(struct sockaddr *) &(sdi->from), &len);

		if (n < 0) {
			printf("Recvfrom failed. %s\n", strerror(errno));
			sleep(1);
			continue;
		}

		// printf("After %d recvfrom %d seqNum %d\n", n, ud->dataSize, ud->seqNum);

		if (n > 0) {
			// call the callback function.
			(*sdi->callback)(sdi);
		}
	}
	close(sdi->sock);

	return NULL;
}

// Used by client to send data to server.
int easyUdpSend(SDI *sdi, char *dataBuffer, int dataSize) {
	struct sockaddr_in servaddr; 

	if (dataSize <= 0)
		dataSize = strlen(dataBuffer);

	if (dataSize > sdi->maxSize) {
		printf("dataSize is greater than maxSize.\n");
		return -1;
	}

	memset(&servaddr, 0, sizeof(servaddr)); 

	// Filling server information 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_port = htons(sdi->port); 
	servaddr.sin_addr.s_addr = inet_addr(sdi->servIp);

	printf("Sending packet to: (%d) %s:%d, %d\n", _seqNum, sdi->servIp, sdi->port, sdi->sendCount);

	UdpData *ud = &(sdi->udpData);

	ud->seqNum = htonl(_seqNum);
	ud->dataSize = htonl(dataSize);
	memcpy(ud->dataBuffer, dataBuffer, dataSize);

	for (int i = 0; i < sdi->sendCount; i++) {
		int r = sendto(sdi->sock, (const char *)ud,
				dataSize + (sizeof(int) * 2),
				MSG_CONFIRM, (const struct sockaddr *) &servaddr,  sizeof(servaddr));
		if (r < 0) {
			printf("sendto failed: %s\n", strerror(errno));
		} else {
			// printf("Sent %d bytes\n", r);
		}
	}

	_seqNum++;

	return 0;
}

int easyUdpRespond(SDI *sdi, char *dataBuffer, int dataSize) {

	if (dataSize <= 0)
		dataSize = strlen(dataBuffer);

	if (dataSize > sdi->maxSize) {
		printf("Repsond dataSize is greater than maxSize.\n");
		return -1;
	}

	UdpData *ud = &(sdi->udpData);

	ud->seqNum = htonl(_seqNum);
	ud->dataSize = htonl(dataSize);
	memcpy(ud->dataBuffer, dataBuffer, dataSize);

	for (int i = 0; i < sdi->sendCount; i++) {
		sendto(sdi->sock, (const char *)ud,
				dataSize + (sizeof(int) * 2),
				0, (const struct sockaddr *) &sdi->from,  sizeof(sdi->from));
	}

	_seqNum++;

	return 0;
}

void easyUdpFree(SDI *sdi) {
	if (sdi != NULL) {
		if (sdi->sock > 0)
			close(sdi->sock);
		stopThread = true;
		free(sdi);
	}
}
