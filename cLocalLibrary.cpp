/*$T MATTS/cLocalLibrary.cpp GC 1.140 07/01/09 21:12:08 */
/*
 * ;
 * Name: cLocalLibrary.cpp ;
 * Last Modified: 10/11/04 ;
 * ;
 * Purpose: Implementation file for local library class ;
 */
#include "Winsock2.h"
#include "cLocalLibrary.h"
#include "cVM.h"
#include "cConsole.h"
#include "cTopologyNode.h"

bool cLocalLibrary:: flag = false;
cLocalLibrary* cLocalLibrary:: instance = NULL;

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */

cLocalLibrary::cLocalLibrary () {
	mutexNetwork = CreateMutex (NULL, false, NULL);
	mutexSound = CreateMutex (NULL, false, NULL);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cLocalLibrary::~cLocalLibrary () {
	flag = false;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cLocalLibrary* cLocalLibrary::create () {

	/* if(!flag) */
	{
		flag = true;

		instance = new cLocalLibrary;
	}

	return instance;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cLocalLibrary::localPlaySound (long int nM2, long int nM3) {
	WaitForSingleObject (mutexSound, INFINITE);
	Beep (nM2, nM3);

	/*
	 * cConsole *pCon = cConsole::create();
	 * pCon->ConsolePrint("Not Done\n");
	 */
	ReleaseMutex (mutexSound);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cLocalLibrary::localReadImage (long int* pnM2, cVM* virtualMachine) {
	ImageInfo*	psImage;
	memset ((void*) pnM2, 0, sizeof (ImageInfo));

	psImage = (ImageInfo*) pnM2;
	psImage->fXPos = virtualMachine->psTopologyNode->fXPos;
	psImage->fYPos = virtualMachine->psTopologyNode->fYPos;
	psImage->fZPos = virtualMachine->psTopologyNode->fZPos;
	psImage->fXRot = virtualMachine->psTopologyNode->fXRot;
	psImage->fYRot = virtualMachine->psTopologyNode->fYRot;
	psImage->fZRot = virtualMachine->psTopologyNode->fZRot;
	psImage->fXScale = virtualMachine->psTopologyNode->fXScale;
	psImage->fYScale = virtualMachine->psTopologyNode->fYScale;
	psImage->fZScale = virtualMachine->psTopologyNode->fZScale;
	psImage->nCol = virtualMachine->psTopologyNode->nCol;
	psImage->nShape = (int) virtualMachine->psTopologyNode->nShape;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cLocalLibrary::localSetImage (long int* pnM2, cVM* virtualMachine) {
	ImageInfo*	psImage;

	cConsole*  pCon;
	psImage = (ImageInfo*) pnM2;
	virtualMachine->psTopologyNode->fXPos = psImage->fXPos;
	virtualMachine->psTopologyNode->fYPos = psImage->fYPos;
	virtualMachine->psTopologyNode->fZPos = psImage->fZPos;
	virtualMachine->psTopologyNode->fXRot = psImage->fXRot;
	virtualMachine->psTopologyNode->fYRot = psImage->fYRot;
	virtualMachine->psTopologyNode->fZRot = psImage->fZRot;
	virtualMachine->psTopologyNode->fXScale = psImage->fXScale;
	virtualMachine->psTopologyNode->fYScale = psImage->fYScale;
	virtualMachine->psTopologyNode->fZScale = psImage->fZScale;
	virtualMachine->psTopologyNode->nCol = psImage->nCol;
	virtualMachine->psTopologyNode->nShape = (SHAPE) psImage->nShape;

	pCon = cConsole::create ();
	pCon->ConsoleRedraw ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
os_error* cLocalLibrary::localGetFileSize (char* szFilename, long int* pnSize, cVM* virtualMachine) {
	FILE*  hFile;
	long int nSize;

	hFile = NULL;
	nSize = -1;

	hFile = fopen (szFilename, "rb");
	if (hFile) {
		fseek (hFile, 0, SEEK_END);
		nSize = ftell (hFile);
		fclose (hFile);
	} else {
		if (!virtualMachine->error) {
			virtualMachine->error = new os_error;
		}

		virtualMachine->error->errnum = 0;
		strcpy (virtualMachine->error->errmess, "File does not exist\n");
	}

	if (pnSize) {
		*pnSize = nSize;
	} else {
		if (!virtualMachine->error) {
			virtualMachine->error = new os_error;
		}

		virtualMachine->error->errnum = 0;
		strcpy (virtualMachine->error->errmess, "No return pointer passed when getting filesize\n");
	}

	return virtualMachine->error;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
os_error* cLocalLibrary::localSaveFile (char* szFilename, char* pcMemory, long int nSize, cVM* virtualMachine) {
	FILE*  hFile;
	long int nWrote;

	nWrote = 0;

	hFile = fopen (szFilename, "wb");
	if (hFile) {
		virtualMachine->ConsolePrint ("Saving...");
		fseek (hFile, 0, SEEK_SET);
		nWrote = (long int) fwrite (pcMemory, sizeof (char), nSize, hFile);

		if (nWrote != nSize) {
			if (!virtualMachine->error) {
				virtualMachine->error = new os_error;
			}

			virtualMachine->error->errnum = 0;
			sprintf (virtualMachine->error->errmess, "Only succeeded in writing %d of %d bytes\n", nWrote, nSize);
		}

		fclose (hFile);
	} else {
		if (!virtualMachine->error) {
			virtualMachine->error = new os_error;
		}

		virtualMachine->error->errnum = 0;
		strcpy (virtualMachine->error->errmess, "File could not be created\n");
	}

	return virtualMachine->error;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
os_error* cLocalLibrary::localLoadFile (char* szFilename, char* pcMemory, cVM* virtualMachine) {
	FILE*  hFile;
	long int nSize;

	hFile = NULL;
	nSize = 0;

	hFile = fopen (szFilename, "rb");
	if (hFile) {
		fseek (hFile, 0, SEEK_END);
		nSize = ftell (hFile);
		fseek (hFile, 0, SEEK_SET);
		fread (pcMemory, sizeof (char), nSize, hFile);
		fclose (hFile);
	} else {
		if (!virtualMachine->error) {
			virtualMachine->error = new os_error;
		}

		virtualMachine->error->errnum = 0;
		strcpy (virtualMachine->error->errmess, "File does not exist\n");
	}

	return virtualMachine->error;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cLocalLibrary::localCheckHost (char* szHost) {
	char szName[1024];
	struct hostent*	 psHost;
	int nReturn;
	int nHostCount;

	WaitForSingleObject (mutexNetwork, INFINITE);
	nReturn = 0;
	if (gethostname (szName, sizeof (szName)) == 0) {
		psHost = gethostbyname (szName);
		if (psHost) {
			nHostCount = 0;
			while (psHost->h_addr_list[nHostCount] != 0) {
				if (strcmp (inet_ntoa (*((struct in_addr*) psHost->h_addr_list[nHostCount])), szHost) == 0) {
					nReturn = 1;
				}

				nHostCount++;
			}
		}
	}

	ReleaseMutex (mutexNetwork);
	return nReturn;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cLocalLibrary::localSendState (char* szSendToHost, cVM* virtualMachine) {
	SOCKET nSockfd;
	struct sockaddr_in sHostAddr;
	int nReturn;
	int nResult;
	char*  pcOutputData;
	long int nOutputSize;
	int nSent;

	WaitForSingleObject (mutexNetwork, INFINITE);
	virtualMachine->ConsolePrint ("Sending agent to ");
	virtualMachine->ConsolePrint (szSendToHost);
	virtualMachine->ConsolePrint ("...\n");

	/*
	 * Send data ;
	 * (this makes us the client)
	 */
	nReturn = 0;
	nSockfd = socket (AF_INET, SOCK_STREAM, 0);

	if (nSockfd != -1) {
		sHostAddr.sin_family = AF_INET;
		sHostAddr.sin_port = htons (6432);
		sHostAddr.sin_addr.s_addr = inet_addr (szSendToHost);
		memset (&(sHostAddr.sin_zero), '\0', 8);

		nResult = connect (nSockfd, (struct sockaddr*) &sHostAddr, sizeof (struct sockaddr));

		if (nResult == 0) {
			virtualMachine->packageExecutingProgram (&pcOutputData, &nOutputSize);

			nSent = 0;
			while ((nSent < nOutputSize) && (nSent >= 0)) {
				nResult = send (nSockfd, pcOutputData + nSent, nOutputSize - nSent, 0);

				if (nResult == -1) {
					virtualMachine->ConsolePrint ("Error sending agent.\n");
					nSent = -1;
				} else {
					nSent += nResult;
				}
			}

			if (nSent == nOutputSize) {
				virtualMachine->ConsolePrint ("...Agent send completed successfuly.\n");
				virtualMachine->setLoaded (false);
				virtualMachine->setRestart (true);
				nReturn = 1;
			} else {
				virtualMachine->ConsolePrint ("Agent send unsuccessful.\n");
			}
		} else {
			char szError[255];
			nResult = WSAGetLastError ();
			sprintf (szError, "Error %d\n", nResult);
			virtualMachine->ConsolePrint (szError);
			virtualMachine->ConsolePrint ("Error connecting to socket.\n");
		}

		shutdown (nSockfd, SD_BOTH);
		closesocket (nSockfd);
	} else {
		virtualMachine->ConsolePrint ("Error creating socket.\n");
	}

	ReleaseMutex (mutexNetwork);
	return nReturn;
}
