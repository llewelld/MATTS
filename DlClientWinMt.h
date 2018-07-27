/*$T MATTS/DlClientWinMt.h GC 1.140 07/01/09 21:12:10 */
/*
 * DlClientWinMt.h Created on 19 July 2008 Multithreaded Dandelion Client for
 * Windows
 */
#ifndef DLCLIENTWINMT_H
#define DLCLIENTWINMT_H

#include "Winsock2.h"
#include "Ws2tcpip.h"
#include <windows.h>
#include <string>
using namespace std;

class DlClientWinMt
{
public:
	DlClientWinMt (char const* szAddress, unsigned int uPort);
	~ DlClientWinMt ();

	void AddNode (char* szName);
	void SubNode (char* szName);
	void AddLink (char* szNodeFrom, char* szNodeTo);
	void SubLink (char* szNodeFrom, char* szNodeTo);
	void AddLinkBi (char* szNode1, char* szNode2);
	void SubLinkBi (char* szNode1, char* szNode2);
	void Clear ();
	void Quit ();
private:
	char*  szAddress;
	unsigned int uPort;
	char szPort[32];
	string sBuffer;
	bool boActive;
	HANDLE hActive;
	HANDLE hNetwork;
	SOCKET sDandeSocket;
	struct addrinfo*  psAddrInfo;
	struct addrinfo sAddrHints;
	HANDLE hBufferMutex;
	HANDLE hBufferEvent;
	WSADATA sWSAData;
	static void Run (DlClientWinMt* psThis);
};
#endif /* DLCLIENTWINMT */
