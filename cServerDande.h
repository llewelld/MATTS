/*$T MATTS/cServerDande.h GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cServerDande.h ;
 * Last Modified: 3/5/09 ;
 * ;
 * Purpose: Header file for Server code ;
 * NOTE: none ;
 * Members ;
 * Methods ;
 */
#ifndef CSERVERDANDE_H
#define CSERVERDANDE_H

#include "cWindow.h"
#include "resource.h"
#include <stdio.h>
#include <stdarg.h>
#include "cConsole.h"
#include "cOptions.h"
#include <string.h>
#include <windows.h>
#include <windowsx.h>
#include <Mmsystem.h>
#include "cAnalyse.h"
#include "cServerDande.h"
#include <cmath>
#include <uxtheme.h>
#include "cSaveXML.h"

#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glut.h>
#include <atlstr.h>
#include <fstream>
#include <iostream>
#include <list>

using namespace std;

typedef enum {
	DANDECOM_INVALID = -1,

	DANDECOM_ADDNODE,
	DANDECOM_SUBNODE,
	DANDECOM_MOVENODE,
	DANDECOM_SETNODECOLOUR,
	DANDECOM_ADDLINK,
	DANDECOM_SUBLINK,
	DANDECOM_ADDLINKBI,
	DANDECOM_SUBLINKBI,
	DANDECOM_SETLINKCOLOUR,
	DANDECOM_ADDNODEPROP,
	DANDECOM_SUBNODEPROP,
	DANDECOM_ADDLINKPROP,
	DANDECOM_SUBLINKPROP,
	DANDECOM_CHANGELINK,
	DANDECOM_CLEAR,
	DANDECOM_QUIT,
	DANDECOM_OKAY,
	DANDECOM_FAIL,

	DANDECOM_NUM
} DANDECOM;

typedef enum {
	DANDECONSTATE_INVALID = -1,

	DANDECONSTATE_COMMAND,
	DANDECONSTATE_FILE,

	DANDECONSTATE_NUM
} DANDECONSTATE;

class cServerDande;

class DeviceConnectionDande {
private:
	void DispatchCommand (SOCKET nSock, char * szCommand, int nCommandLength);
	bool GetParameter (char const * const szCommand, int const nLength, int const nParameter, char const * * const pszParameter, int * const pnLength);
	bool GetParameter (char const * const szCommand, int const nLength, int const nParameter, string * pszParameter);
	void SendCommand (char const * szCommand);
	void SendCommand (void const * pCommand, int nLength);
	void SendCommandParams (char const * szTemplate, ...);
	int CommandStack (char * szReceive, int nRecBuffEnd);
	int FileReceive (char * szReceive, int nRecBuffEnd);
	void FileChunkReceived (char * szBuffer, int nSize, DANDECOM eCommand);
	void FileReceived (DANDECOM eCommand);

	DANDECONSTATE eState;
	DANDECOM eCommand;
	int nFileRead;
	int nFileSize;
	cSaveXML * psSaveXML;
public:
	DeviceConnectionDande (SOCKET nAcceptSock, cServerDande * psServer, cWindow * psWindow);
	~DeviceConnectionDande ();
	HANDLE nThread;
	SOCKET nAcceptSock;
	cWindow * psWindow;
	cServerDande * psServer;
	void AcceptConnection ();
};

class cServerDande
{
private:
	//static void AcceptConnection (DeviceConnectionDande * psDevice);
	// List of device connections
	list<DeviceConnectionDande *> lpConnections;
	bool boRunning;
	bool boListen;
public:
	bool GetRunning ();
	void StopServer ();
	cServerDande ();
	~cServerDande ();

	void RunServer (cWindow * psWindow);
};
#endif
