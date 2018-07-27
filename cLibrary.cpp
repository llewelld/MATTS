/*$T MATTS/cLibrary.cpp GC 1.140 07/01/09 21:12:08 */
/*
 * ;
 * Name: cLibrary.cpp ;
 * Last Modified: 10/11/04 ;
 * ;
 * Purpose: Implementation file for library class ;
 */
#include "cLibrary.h"
#include "cConsole.h"
#include "cVM.h"
#include "cTopologyNode.h"

/*
 =======================================================================================================================
 *  Constructor
 =======================================================================================================================
 */
cLibrary::cLibrary () {
	sendActive = false;
}

/*
 =======================================================================================================================
 *  Destructor
 =======================================================================================================================
 */
cLibrary::~cLibrary () {
}

/*
 =======================================================================================================================
 *  LIB print string
 =======================================================================================================================
 */
void cLibrary::executeLIB_PrintString (long int* pnM2, cVM* vm) {
	if (pnM2) {
		vm->ConsolePrint ((char*) pnM2);
	}
}

/*
 =======================================================================================================================
 *  LIB print integer
 =======================================================================================================================
 */
void cLibrary::executeLIB_PrintInteger (long int nM2, cVM* vm) {
	char sNumber[16];
	numberToString (nM2, sNumber);
	vm->ConsolePrint (sNumber);
}

/*
 =======================================================================================================================
 *  LIB print nmuber
 =======================================================================================================================
 */
void cLibrary::executeLIB_PrintNumber (long int nM2, cVM* vm) {
	char sNumber[16];
	numberToString (nM2, sNumber);
	vm->ConsolePrint (sNumber);
	vm->ConsolePrint ("\n");
}

/*
 =======================================================================================================================
 *  LIB input string
 =======================================================================================================================
 */
void cLibrary::executeLIB_InputString (long int* pnM2, long int nM3, cVM* vm) {
	long int iLength;

	vm->setInput (true);
	if (pnM2) {
		cConsole*  pCon;

		pCon = cConsole::create ();
		if (vm->getActive () == true) {

			/* active vm */
			iLength = vm->storeString (pCon->ConsoleInputString (), pnM2, nM3);
			vm->checkMaxMemory (pnM2 + iLength);
		} else {

			/* not active, wait */
			wait (vm->getActive ());

			/* now active */
			iLength = vm->storeString (pCon->ConsoleInputString (), pnM2, nM3);
			vm->checkMaxMemory (pnM2 + iLength);
		}
	}

	vm->setInput (false);
}

/*
 =======================================================================================================================
 *  LIB input number
 =======================================================================================================================
 */
void cLibrary::executeLIB_InputNumber (long int* pnM2, cVM* vm) {
	vm->setInput (true);
	if (pnM2) {
		cConsole*  pCon;
		pCon = cConsole::create ();

		if (vm->getActive () == true) {

			/* active */
			*pnM2 = pCon->ConsoleInputNumber ();
			vm->checkMaxMemory (pnM2);
		} else {

			/* inactive, wait */
			wait (vm->getActive ());
			MessageBox (NULL, "", "", MB_OK);
			*pnM2 = pCon->ConsoleInputNumber ();
			vm->checkMaxMemory (pnM2);
		}
	}

	vm->setInput (false);
}

/*
 =======================================================================================================================
 *  Execute a LIB SendState instruction
 =======================================================================================================================
 */
void cLibrary::executeLIB_SendState (long int* pnM2, cVM* vm) {

	/*
	 * if (sendActive) wait();
	 * sendActive = true;
	 */
	if (pnM2) {
		if (localCheckHost ((char*) pnM2) == 0) {
			localSendState ((char*) pnM2, vm);
		}
	}

	/*
	 * sendActive = false;
	 */
}

/*/
 =======================================================================================================================
 *  Execute a LIB GetHost instruction
 =======================================================================================================================
 */
void cLibrary::executeLIB_CheckHost (long int* pnM2, long int* pnM3, cVM* vm) {

	/*
	 * if (sendActive) wait();
	 * sendActive = true;
	 */
	if (pnM2) {
		*pnM2 = localCheckHost ((char*) pnM3);
		vm->checkMaxMemory (pnM2);
	}

	/*
	 * sendActive = false;
	 */
}

/*
 =======================================================================================================================
 *  LIB Play Sound
 =======================================================================================================================
 */
void cLibrary::executeLIB_PlaySound (long int nM2, long int nM3) {
	localPlaySound (nM2, nM3);
}

/*
 =======================================================================================================================
 *  LIB send integer
 =======================================================================================================================
 */
bool cLibrary::executeLIB_SendInteger (long int nM2, long int nM3, cVM* vm) {
	return vm->psTopologyNode->findLinkWithChannelOut (nM3)->sendData (nM2);
}

/*
 =======================================================================================================================
 *  LIB receive integer
 =======================================================================================================================
 */
bool cLibrary::executeLIB_ReceiveInteger (long int* pnM2, long int nM3, cVM* vm) {
	return vm->psTopologyNode->findLinkWithChannelIn (nM3)->receiveData (pnM2);
}

/*
 =======================================================================================================================
 *  LIB heap alloc
 =======================================================================================================================
 */
void cLibrary::executeLIB_HeapAlloc (long int* pnM2, long int nM3, cVM* vm) {
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cLibrary::executeLIB_ReadImage (long int* pnM2, cVM* vm) {
	if (pnM2) {
		localReadImage (pnM2, vm);
		vm->checkMaxMemory (pnM2 + sizeof (ImageInfo));
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cLibrary::executeLIB_SetImage (long int* pnM2, cVM* vm) {
	if (pnM2) {
		localSetImage (pnM2, vm);
		vm->checkMaxMemory (pnM2 + sizeof (ImageInfo));
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cLibrary::executeLIB_Wait (long int nM2, cVM* vm) {
	::Sleep (nM2);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cLibrary::numberToString (long int nNumber, char* szString) {
	long int nWidth;
	long int nMultiplier;

	nWidth = 1;
	nMultiplier = 10;

	if (nNumber < 0) {
		szString[0] = '-';
		szString++;
		nNumber = -nNumber;
	}

	while (nNumber >= nMultiplier) {
		nWidth++;
		nMultiplier *= 10;
	}

	szString[nWidth] = 0;
	while (nWidth > 0) {
		nWidth--;
		szString[nWidth] = '0' + (char) (nNumber % 10);
		nNumber /= 10;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cLibrary::wait (bool condition) {
	while (!condition) {

		/* do nothing */
	}
}
