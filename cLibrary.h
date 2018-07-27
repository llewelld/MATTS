/*$T MATTS/cLibrary.h GC 1.140 07/01/09 21:12:08 */
/*
 * ;
 * Name: cLibrary.h ;
 * Last Modified: 10/11/04 ;
 * ;
 * Purpose: Header file for MATS Library ;
 * NOTE: Needs to be inherited from to create local library ;
 * Members ;
 * sendActive - True if sending data, false otherwise ;
 * Methods ;
 * wait() - Waits until passed condition is true ;
 * numberToString()- Converts a number to a string ;
 * executeLIB_PrintString() - Prints a string ;
 * executeLIB_PrintInteger() - Prints an integer ;
 * executeLIB_PrintNumber() - Prints a (floating point) number ;
 * executeLIB_InputString() - Input's a string ;
 * executeLIB_InputNumber() - Input's a number (int or float) ;
 * executeLIB_SendState() - Sends the agent to a new machine ;
 * executeLIB_CheckHost() - Checks if a host exists ;
 * executeLIB_PlaySound() - Plays a sound on local comp ;
 */
#ifndef CLIBRARY_H
#define CLIBRARY_H

#include "cVMTypes.h"

typedef enum _IMAGESHAPE
{
	IMAGESHAPE_INVALID,
	IMAGESHAPE_CUBE,
	IMAGESHAPE_SPHERE,
	IMAGESHAPE_CONE,
	IMAGESHAPE_TEAPOT,
	IMAGESHAPE_NUM
} IMAGESHAPE;

typedef struct _ImageInfo
{
	float fXPos;
	float fYPos;
	float fZPos;
	float fXRot;
	float fYRot;
	float fZRot;
	float fXScale;
	float fYScale;
	float fZScale;
	int nCol;
	int nShape;
} ImageInfo;

class cVM;

class cLibrary
{
	friend class cVM;
protected:
	void numberToString (long int nNumber, char* szString);
	void executeLIB_PrintString (long int* pnM2, cVM* vm);
	void executeLIB_PrintInteger (long int nM2, cVM* vm);
	void executeLIB_PrintNumber (long int nM2, cVM* vm);
	void executeLIB_InputString (long int* pnM2, long int nM3, cVM* vm);
	void executeLIB_InputNumber (long int* pnM2, cVM* vm);
	void executeLIB_SendState (long int* pnM2, cVM* vm);
	void executeLIB_CheckHost (long int* pnM2, long int* pnM3, cVM* vm);
	void executeLIB_PlaySound (long int nM2, long int nM3);
	bool executeLIB_SendInteger (long int nM2, long int nM3, cVM* vm);
	bool executeLIB_ReceiveInteger (long int* pnM2, long int nM3, cVM* vm);
	void executeLIB_HeapAlloc (long int* pnM2, long int nM3, cVM* vm);
	void executeLIB_ReadImage (long int* pnM2, cVM* vm);
	void executeLIB_SetImage (long int* pnM2, cVM* vm);
	void executeLIB_Wait (long int nM2, cVM* vm);

	virtual os_error*  localGetFileSize (char* szFilename, long int* pnSize, cVM* virtualMachine) = 0;
	virtual os_error*  localLoadFile (char* szFilename, char* pcMemory, cVM* virtualMachine) = 0;
	virtual os_error*  localSaveFile (char* szFilename, char* pcMemory, long int nSize, cVM* virtualMachine) = 0;
	virtual int localCheckHost (char* szHost) = 0;
	virtual int localSendState (char* szSendToHost, cVM* virtualMachine) = 0;
	virtual void localPlaySound (long int nM2, long int nM3) = 0;
	virtual void localReadImage (long int* pnM2, cVM* virtualMachine) = 0;
	virtual void localSetImage (long int* pnM2, cVM* virtualMachine) = 0;
	bool sendActive;
public:
	void wait (bool);
	cLibrary ();
	virtual ~cLibrary ();
};
#endif
