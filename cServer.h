/*$T MATTS/cServer.h GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cServer.h ;
 * Last Modified: 3/5/09 ;
 * ;
 * Purpose: Header file for Server code ;
 * NOTE: none ;
 * Members ;
 * Methods ;
 */
#ifndef CSERVER_H
#define CSERVER_H

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
#include "cXMLCompile.h"
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
	CONCOM_INVALID = -1,

	CONCOM_HELLO,
	CONCOM_GOODBYE,
	CONCOM_NODELIST,
	CONCOM_CREATELINK,
	CONCOM_REMOVELINK,
	CONCOM_SENDFILE,
	CONCOM_SENDPROPERTYSET,
	CONCOM_SENDPROPERTIES,

	CONCOM_NUM
} CONCOM;

typedef enum {
	CONSTATE_INVALID = -1,

	CONSTATE_COMMAND,
	CONSTATE_FILE,

	CONSTATE_NUM
} CONSTATE;

class cServer;

class DeviceConnection {
private:
	void DispatchCommand (SOCKET nSock, char * szCommand, int nCommandLength);
	bool GetParameter (char const * const szCommand, int const nLength, int const nParameter, char const * * const pszParameter, int * const pnLength);
	int nNodeID;
	void SendCommand (char const * szCommand);
	void SendCommand (void const * pCommand, int nLength);
	void SendCommandParams (char const * szTemplate, ...);
	int CommandStack (char * szReceive, int nRecBuffEnd);
	int FileReceive (char * szReceive, int nRecBuffEnd);
	void FileChunkReceived (char * szBuffer, int nSize, CONCOM eCommand);
	void FileReceived (CONCOM eCommand);
	void ParseProperties ();
	void ParsePropertySet ();

	CONSTATE eState;
	CONCOM eCommand;
	int nFileRead;
	int nFileSize;
	int nNode;
	cSaveXML * psSaveXML;
	char * pcProperties;
	int nPropertiesSize;
	char * pcPropertySet;
	int nPropertySetSize;
public:
	DeviceConnection (SOCKET nAcceptSock, cServer * psServer, cWindow * psWindow);
	~DeviceConnection ();
	HANDLE nThread;
	SOCKET nAcceptSock;
	cWindow * psWindow;
	cServer * psServer;
	void AcceptConnection ();
};

class cServer
{
private:
	//static void AcceptConnection (DeviceConnection * psDevice);
	// List of device connections
	list<DeviceConnection *> lpConnections;
	bool boRunning;
	bool boListen;
public:
	bool GetRunning ();
	void StopServer ();
	cServer ();
	~cServer ();

	void RunServer (cWindow * psWindow);
};
#endif
