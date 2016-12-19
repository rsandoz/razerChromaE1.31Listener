#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#define _CRT_SECURE_NO_WARNINGS 1

// Command line options:
//    receiver [-p:int] [-i:IP][-n:x] [-b:x]
//           -p:int   Local port
//           -i:IP    Local IP address to listen on
//
#include "Chroma.h"

#define DEFAULT_PORT 5568

int iPort = DEFAULT_PORT;
BOOL bInterface = FALSE;
char szInterface[32];

void usage() {
	printf("usage: receiver [-p:int] [-i:IP][-n:x] [-b:x]\n\n");
	printf("       -p:int   Local port\n");
	printf("       -i:IP    Local IP address to listen on\n");
	ExitProcess(1);
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
					bInterface = TRUE;
					strcpy_s(szInterface, &argv[i][3]);
				}
				break;
			default:
				usage();
				break;
			}
		}
	}
}

int main(int argc, char **argv) {
	Chroma impl;
	BOOL test_for_init = impl.initialize();
	if (test_for_init == TRUE) {
		validateArgs(argc, argv);

		WSADATA wsd;
		if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0) {
			printf("WSAStartup failed!\n");
			return 1;
		}
		SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
		if (s == INVALID_SOCKET) {
			printf("socket() failed; %d\n", WSAGetLastError());
			return 1;
		}
		SOCKADDR_IN local;
		local.sin_family = AF_INET;
		local.sin_port = htons((short)iPort);
		if (bInterface)
			local.sin_addr.s_addr = inet_addr(szInterface);
		else
			local.sin_addr.s_addr = htonl(INADDR_ANY);
		if (bind(s, (SOCKADDR *)&local, sizeof(local)) == SOCKET_ERROR) {
			printf("bind() failed: %d\n", WSAGetLastError());
			return 1;
		}
		e131_packet_t packet;
		// Read datagrams
		for (;;) {
			SOCKADDR_IN sender;
			DWORD dwSenderSize = sizeof(sender);
			int ret = recvfrom(s, (char*)packet.raw, 638, 0, (SOCKADDR*)&sender, (int*)&dwSenderSize);
			if (ret == SOCKET_ERROR) {
				printf("recvfrom() failed; %d\n", WSAGetLastError());
				break;
			} else if (ret == 0)
				break;
			else {
				//printf("\n[%s] received:", inet_ntoa(sender.sin_addr));
				impl.command(&packet);
			}
		}
		closesocket(s);
		WSACleanup();
	}
	else
		printf("Unable to initialize Chroma.\n");
	return 0;
}
