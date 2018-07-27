/*$T MATTS/cLocalLibrary.h GC 1.140 07/01/09 21:12:08 */
/*
 * ;
 * Name: cLocalLibrary.h ;
 * Last Modified: 10/11/04 ;
 * ;
 * Purpose: Header file for Local Library implementation ;
 * NOTE: This file needs to have the local functions for the library. ;
 * THIS IS FOR WINDOWS PLATFORMS ONLY!!!!!!!!!!!! ;
 * ;
 * Members ;
 * instance - Pointer to local library instance ;
 * flag - True if library created, false if not ;
 * mutexNetwork - Mutex handle for locking network code ;
 * mutexSound - Mutex handle for locking sound code ;
 * Methods ;
 * create() - Creates the library instance ;
 * localGetFileSize() - Retrieves size of a file ;
 * localLoadFile() - Loads a file from disk (local implementation) ;
 * localSaveFile() - Saves a file from disk (local implementation) ;
 * localCheckHost()- Checks if a host exists (local implementation) ;
 * localSendState()- Send the agent to another host (local implementation) ;
 * localPlaySound()- Plays a sound on client computer (local implementation) ;
 */
#ifndef CLOCALLIBRARY_H
#define CLOCALLIBRARY_H

#include <windows.h>	/* mutex */
#include "cLibrary.h"
#include <stdio.h>

class cVM;

class cLocalLibrary : public cLibrary
{
	friend class cVM;
public:
	cLocalLibrary ();
	virtual ~cLocalLibrary ();
	static cLocalLibrary*  create ();
private:
	static cLocalLibrary*  instance;
	static bool flag;
	virtual os_error*  localGetFileSize (char* szFilename, long int* pnSize, cVM* virtualMachine);
	virtual os_error*  localLoadFile (char* szFilename, char* pcMemory, cVM* virtualMachine);
	virtual os_error*  localSaveFile (char* szFilename, char* pcMemory, long int nSize, cVM* virtualMachine);
	virtual int localCheckHost (char* szHost);
	virtual int localSendState (char* szSendToHost, cVM* virtualMachine);
	virtual void localPlaySound (long int nM2, long int nM3);
	virtual void localReadImage (long int* pnM2, cVM* virtualMachine);
	virtual void localSetImage (long int* pnM2, cVM* virtualMachine);
	HANDLE mutexNetwork;
	HANDLE mutexSound;
};
#endif
