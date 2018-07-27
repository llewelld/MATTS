/*$T MATTS/DlClientWinMt.cpp GC 1.140 07/01/09 21:12:10 */
/*
 * DlClientWinMt.cpp Created on 19 July 2008 Multithreaded Dandelion Client for
 * Windows
 */
#include "DlClientWinMt.h"
#include <iostream>

#define ADDRESS_LEN_MAX		(1024)
#define RECEIVE_BUFFER_MAX	(1024)
#define FINISH_TIMEOUT		(10 * 1000)

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */

DlClientWinMt::DlClientWinMt (char const* szAddress, unsigned int uPort) {
	int uAddressLen;
	//int nResult;
	uAddressLen = (unsigned int) strnlen (szAddress, ADDRESS_LEN_MAX);
	this->szAddress = (char*) malloc (uAddressLen + 1);
	strncpy (this->szAddress, szAddress, uAddressLen);
	this->szAddress[uAddressLen] = '\0';
	this->uPort = uPort;
	_snprintf (szPort, 32, "%u", uPort);
	sBuffer = "";
	boActive = false;
	sDandeSocket = INVALID_SOCKET;

	//nResult = WSAStartup (MAKEWORD (1, 1), & sWSAData);
	//if (nResult != 0) {
	//	printf ("Error starting up networking\n");
	//}

	hBufferMutex = CreateMutex (NULL, FALSE, NULL);
	hBufferEvent = CreateEvent (NULL, FALSE, FALSE, NULL);

	hNetwork = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) Run, this, 0, NULL);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
DlClientWinMt::~DlClientWinMt () {
	boActive = false;

	sBuffer.clear ();
	SetEvent (hBufferEvent);
	WaitForSingleObject (hNetwork, FINISH_TIMEOUT);

	/*
	 * Not good, but something went wrong wo we're going to forcefully ;
	 * terminate the thread
	 */
	TerminateThread (hNetwork, NULL);
	CloseHandle (hNetwork);

	/*
	 * WSACleanup ();
	 */
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void DlClientWinMt::AddNode (char* szName) {
	WaitForSingleObject (hBufferMutex, INFINITE);

	sBuffer += "ADDNODE ";
	sBuffer += szName;
	sBuffer += '\n';

	ReleaseMutex (hBufferMutex);
	SetEvent (hBufferEvent);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void DlClientWinMt::SubNode (char* szName) {
	WaitForSingleObject (hBufferMutex, INFINITE);

	sBuffer += "SUBNODE ";
	sBuffer += szName;
	sBuffer += '\n';

	ReleaseMutex (hBufferMutex);
	SetEvent (hBufferEvent);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void DlClientWinMt::AddLink (char* szNodeFrom, char* szNodeTo) {
	WaitForSingleObject (hBufferMutex, INFINITE);

	sBuffer += "ADDLINK ";
	sBuffer += szNodeFrom;
	sBuffer += ' ';
	sBuffer += szNodeTo;
	sBuffer += '\n';

	ReleaseMutex (hBufferMutex);
	SetEvent (hBufferEvent);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void DlClientWinMt::SubLink (char* szNodeFrom, char* szNodeTo) {
	WaitForSingleObject (hBufferMutex, INFINITE);

	sBuffer += "SUBLINK ";
	sBuffer += szNodeFrom;
	sBuffer += ' ';
	sBuffer += szNodeTo;
	sBuffer += '\n';

	ReleaseMutex (hBufferMutex);
	SetEvent (hBufferEvent);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void DlClientWinMt::AddLinkBi (char* szNode1, char* szNode2) {
	WaitForSingleObject (hBufferMutex, INFINITE);

	sBuffer += "ADDLINKBI ";
	sBuffer += szNode1;
	sBuffer += ' ';
	sBuffer += szNode2;
	sBuffer += '\n';

	ReleaseMutex (hBufferMutex);
	SetEvent (hBufferEvent);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void DlClientWinMt::SubLinkBi (char* szNode1, char* szNode2) {
	WaitForSingleObject (hBufferMutex, INFINITE);

	sBuffer += "SUBLINKBI ";
	sBuffer += szNode1;
	sBuffer += ' ';
	sBuffer += szNode2;
	sBuffer += '\n';

	ReleaseMutex (hBufferMutex);
	SetEvent (hBufferEvent);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void DlClientWinMt::Clear () {
	WaitForSingleObject (hBufferMutex, INFINITE);

	sBuffer += "CLEAR\n";

	ReleaseMutex (hBufferMutex);
	SetEvent (hBufferEvent);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void DlClientWinMt::Quit () {
	WaitForSingleObject (hBufferMutex, INFINITE);

	sBuffer += "QUIT\n";

	ReleaseMutex (hBufferMutex);
	SetEvent (hBufferEvent);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void DlClientWinMt::Run (DlClientWinMt* psThis) {
	int nFirstLine;
	string sFirstLine;
	int nResult;
	char szReceiveBuffer[RECEIVE_BUFFER_MAX];

	psThis->boActive = true;

	while (psThis->boActive) {

		/* Wait for something to add data to the buffer and notify us */
		WaitForSingleObject (psThis->hBufferEvent, INFINITE);

		if (psThis->sBuffer.length () > 0) {

			/* Open the connection if it's not already */
			if (psThis->sDandeSocket == INVALID_SOCKET) {
				memset (&psThis->sAddrHints, 0, sizeof (addrinfo));
				psThis->sAddrHints.ai_family = AF_UNSPEC;
				psThis->sAddrHints.ai_socktype = SOCK_STREAM;
				psThis->sAddrHints.ai_protocol = IPPROTO_TCP;

				/* Resolve server address */
				nResult = getaddrinfo (psThis->szAddress, psThis->szPort, &psThis->sAddrHints, &psThis->psAddrInfo);
				if (nResult != 0) {
					printf ("Error finding host\n");
				}

				/* Create the socket */
				psThis->sDandeSocket = INVALID_SOCKET;
				psThis->sDandeSocket = socket (psThis->psAddrInfo->ai_family, psThis->psAddrInfo->ai_socktype,
											   psThis->psAddrInfo->ai_protocol);
				if (psThis->sDandeSocket == INVALID_SOCKET) {
					printf ("Error creating socket\n");;
				}

				/* Connect to the Dandelion server */
				nResult = connect (psThis->sDandeSocket, psThis->psAddrInfo->ai_addr,
								   (int) psThis->psAddrInfo->ai_addrlen);
				if (nResult == SOCKET_ERROR) {
					closesocket (psThis->sDandeSocket);
					psThis->sDandeSocket = INVALID_SOCKET;
				}

				/* Don't need the address info any more, so we can release it */
				freeaddrinfo (psThis->psAddrInfo);
			}

			if (psThis->sDandeSocket == INVALID_SOCKET) {
				printf ("Failed to connect\n");
			}

			/* Send any data in the buffer a line at a time */
			while ((psThis->sBuffer.length () > 0) && (psThis->sDandeSocket != INVALID_SOCKET)) {

				/* Figure out the first command in the buffer to send */
				WaitForSingleObject (psThis->hBufferMutex, INFINITE);
				nFirstLine = (int) psThis->sBuffer.find ('\n') + 1;
				if (nFirstLine <= 0) {
					nFirstLine = (int) psThis->sBuffer.length ();
				}

				sFirstLine = psThis->sBuffer.substr (0, nFirstLine);
				psThis->sBuffer.erase (0, nFirstLine);
				ReleaseMutex (psThis->hBufferMutex);

				/* Now we're ready to actually send the command */
				nResult = send (psThis->sDandeSocket, sFirstLine.c_str (), (int) sFirstLine.length (), 0);
				if (nResult == SOCKET_ERROR) {
					printf ("Error sending command\n");
					closesocket (psThis->sDandeSocket);
					psThis->sDandeSocket = INVALID_SOCKET;
				}

				/* Read in the reply */
				nResult = recv (psThis->sDandeSocket, szReceiveBuffer, RECEIVE_BUFFER_MAX, 0);
				if (nResult <= 0) {

					/* End of the socket */
				}
			}
		}
	}

	if (psThis->sDandeSocket != INVALID_SOCKET) {

		/* Close the socket */
		nResult = shutdown (psThis->sDandeSocket, SD_SEND);
		closesocket (psThis->sDandeSocket);
		psThis->sDandeSocket = INVALID_SOCKET;
	}

	/* And we're done with this thread */
}
