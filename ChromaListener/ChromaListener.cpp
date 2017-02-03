#include <map>
#include <stdio.h>

#if defined(WIN32) || defined(_WIN64)
#include <wtypes.h>
#include <linux/kernel.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#endif

#include "packet.h"
#include <vector>
#include <set>
#include "RzChromaSDKTypes.h"
#include "Chroma.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS 1

// Command line options:
//    receiver [-p:int] [-i:IP][-d:1 or 0]
//           -p:int   Local port
//           -i:IP    Local IP address to listen on
//           -d:int   Debug (1 or 0) 
//

#define DEFAULT_PORT 5568

int iPort = DEFAULT_PORT;
int bInterface = 0;
char szInterface[32];
int bMulticast = 1;
Chroma impl;

void usage() {
	printf("usage: receiver [-p:int] [-i:IP][-n:x] [-b:x]\n\n");
	printf("       -p:int   Local port (default:5568)\n");
	printf("       -i:IP    Local IP address to listen on (default:any)\n");
	printf("       -u:int   Unicast (1 or 0 default:0)\n");
	printf("       -d:int   Debug (1 or 0 default:0)\n");
	exit(1);
}

void validateArgs(int argc, char **argv) {
	for (int i = 1; i < argc; i++) {
		if ((argv[i][0] == '-') || (argv[i][0] == '/')) {
			switch (tolower(argv[i][1])) {
			case 'p':   // Local port
				if (strlen(argv[i]) > 3)
					iPort = atoi(&argv[i][3]);
				break;
			case 'i':	// Interface to receive datagrams on
				if (strlen(argv[i]) > 3) {
					bInterface = 1;
					strcpy_s(szInterface, strlen(szInterface)-1, &argv[i][3]);
				}
				break;
			case 'u':	// Unicast mode
				if (strlen(argv[i]) > 3)
					bMulticast = !atoi(&argv[i][3]);
				break;
			case 'd':   // Debug
				if (strlen(argv[i]) > 3)
					bDebug = atoi(&argv[i][3]);
				break;
			default:
				usage();
				break;
			}
		}
	}
}

int listen() {

#if defined(WIN32) || defined(WIN64)
	WSADATA wsd;
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0) {
		printf("WSAStartup failed!\n");
		return 1;
	}
#endif

	//create a UDP socket
	SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
	//if (s == INVALID_SOCKET) {
	if (s < 0) {
		printf("socket() failed; %d\n", WSAGetLastError());
		return 1;
	}

	if (bMulticast) {
		//allow multiple sockets to use the same PORT number
		unsigned int yes = 1;
		if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes)) < 0) {
			perror("Reusing ADDR failed");
			return 1;
		}
	}

	//set up destination address
	sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons((short)iPort);
	if (bInterface)
		local.sin_addr.s_addr = inet_addr(szInterface);
	else
		local.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//bind to receive address
	if (bind(s, (sockaddr *)&local, sizeof(local)) == SOCKET_ERROR) {
		printf("bind() failed: %d\n", WSAGetLastError());
		return 1;
	}

	if (bMulticast)
		for (unsigned int universe : impl.universeSet) {
			unsigned char b1 = (unsigned char)(universe >> 8);
			unsigned char b2 = (unsigned char)(universe & 0xff);
			char addr[16];
			sprintf_s(addr,"239.255.%d.%d", b1, b2);
			if (bDebug)
				printf("listening on multicast address %s\n", addr);

			//use setsockopt() to request to join a multicast group
			ip_mreq mreq;
			mreq.imr_multiaddr.s_addr = inet_addr(addr); // need universe 239,255,(byte) (universe >> 8), (byte) (universe & 0xff)
			mreq.imr_interface.s_addr = htonl(INADDR_ANY);
			if (setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*)&mreq, sizeof(mreq)) < 0) {
				printf("setsockopt() failed: %d\n", WSAGetLastError());
				return 1;
			}
		}
	else
		if (bDebug)
			printf("listening in unicast mode\n");

	// Read datagrams
	for (;;) {
		sockaddr_in sender;
		unsigned long dwSenderSize = sizeof(sender);
		//int ret = recvfrom(s, (char*)impl.packet.raw, 638, 0, (SOCKADDR*)&sender, (unsigned int*)&dwSenderSize);
		int ret = recvfrom(s, (char*)impl.packet.raw, 638, 0, (SOCKADDR*)&sender, (socklen_t*)&dwSenderSize);
		//if (ret == SOCKET_ERROR) {
		if (ret <  0) {
			printf("recvfrom() failed; %d\n", WSAGetLastError());
			break;
		} else if (ret == 0)
			break;
	}

	closesocket(s);
#if defined(WIN32) || defined(WIN64)
	WSACleanup();
#endif
}



int main(int argc, char **argv) {
	int test_for_init = impl.initialize();
	if (test_for_init == 1) {
		validateArgs(argc, argv);
		impl.startThread();
		return listen();
	} else
		printf("Unable to initialize Chroma.\n");
	return 0;
}

//http://ntrg.cs.tcd.ie/undergrad/4ba2/multicast/antony/example.html
