/*$T MATTS/cServer.cpp GC 1.140 07/01/09 21:12:09 */
#include "cServer.h"
#include "cXMLPropertySet.h"
#include <xercesc/framework/MemBufInputSource.hpp>

#define RECEIVE_BUFFER	(1024)
#define SEND_BUFFER	(1024)
#define SEND_COM_PREFIX_LEN	(sizeof (int))
#define COMMAND_MAXLEN (1024)
#define PROPERTIESXML_BUFFER	(1024*5)

void DispatchCommand (SOCKET nSock, char * szCommand, int nCommandLength) ;
void SendCommand (SOCKET nSock, char * szCommand);
void AcceptConnection (DeviceConnection * psDevice);

/*
 =======================================================================================================================
 *  Initialise the server object
 =======================================================================================================================
 */

cServer::cServer () {
	boRunning = false;
}

/*
 =======================================================================================================================
 *  Deinitialise the server object
 =======================================================================================================================
 */
cServer::~cServer () {
}

/*
 =======================================================================================================================
 *  Returns true if the server is currently running, false otherwise
 =======================================================================================================================
 */
bool cServer::GetRunning () {
	// The boRunning variable is true only when the server is running
	return boRunning;
}

/*
 =======================================================================================================================
 *  Will shut down the server when called (although it doesn't close existing connections)
 =======================================================================================================================
 */
void cServer::StopServer () {
	// Each listen cycle (approx every half second) checks the boListen variable
	// The server quits if boListen is set to false
	boListen = false;
}

/*
 =======================================================================================================================
 *  Listen for connections and spawn a DeviceConnection object thread for each one received
 =======================================================================================================================
 */
void cServer::RunServer (cWindow * psWindow) {
	SOCKET nListenSock;
	SOCKET nAcceptSock;
	int nPort;
	struct sockaddr_in sHostAddr;
	struct sockaddr_in sClientAddr;
	int nReturn;
	fd_set sListenSet;
	int nSize;
	struct timeval sTime;
	DeviceConnection * psDevice;

	boListen = true;
	nPort = 1984;

	if (!boRunning) {
		boRunning = true;
		nListenSock = socket (AF_INET, SOCK_STREAM, 0);
		if (nListenSock != SOCKET_ERROR) {
			memset (& sHostAddr, 0, sizeof (struct sockaddr));
			sHostAddr.sin_family = AF_INET;
			sHostAddr.sin_port = htons (nPort);
			sHostAddr.sin_addr.s_addr = htonl (INADDR_ANY);
			memset (&(sHostAddr.sin_zero), '\0', 8);

			nReturn = bind (nListenSock, (struct sockaddr*) & sHostAddr, sizeof (struct sockaddr));

			if (nReturn == 0) {
				nReturn = listen (nListenSock, 10);
				if (nReturn == 0) {
					while (boListen) {
						FD_ZERO (& sListenSet);
						FD_SET (nListenSock, & sListenSet);
						sTime.tv_sec = 0;
						sTime.tv_usec = 500000;
						nReturn = select ((int) nListenSock + 1, & sListenSet, NULL, NULL, & sTime);
						if (nReturn != SOCKET_ERROR) {
							if (FD_ISSET (nListenSock, & sListenSet)) {

								nAcceptSock = SOCKET_ERROR;
								nSize = sizeof (struct sockaddr_in);
								while (nAcceptSock == SOCKET_ERROR) {
									nAcceptSock = accept (nListenSock, (struct sockaddr*) & sClientAddr, & nSize);
								}
								// Start new thread to talk on the socket
								psDevice = new DeviceConnection (nAcceptSock, this, psWindow);
								// Add the connection to the server connections list
								// We need this to enable broadcasts to the clients
								lpConnections.push_front (psDevice);
								psDevice->nThread = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) AcceptConnection, psDevice, 0, NULL);
								printf ("Connected\n");
							}
						}
					}
				}
			}
		}
		closesocket (nListenSock);
	}
	boRunning = false;
}

/*
 =======================================================================================================================
 *  Initialise the DeviceConnection object
 =======================================================================================================================
 */
DeviceConnection::DeviceConnection (SOCKET nAcceptSock, cServer * psServer, cWindow * psWindow) {
	this->nAcceptSock = nAcceptSock;
	this->psServer = psServer;
	this->psWindow = psWindow;
	eCommand = CONCOM_INVALID;
	eState = CONSTATE_COMMAND;
	nFileRead = 0;
	nFileSize = 0;
	nNode = 0;
	// Create a cSaveXML object to load the node XML properties
	psSaveXML = new cSaveXML (psWindow);
	// Allocate some space to read in the property info
	// TODO: Change the property reading to use an input stream (smaller input buffer)
	pcProperties = (char *)malloc (PROPERTIESXML_BUFFER);
	nPropertiesSize = 0;
	pcPropertySet = (char *)malloc (PROPERTIESXML_BUFFER);
	nPropertySetSize = 0;
}

/*
 =======================================================================================================================
 *  Deinitialise the DeviceConnection object
 =======================================================================================================================
 */
DeviceConnection::~DeviceConnection () {
	// Free the cSaveXML object if there is one
	if (psSaveXML) {
		delete psSaveXML;
		psSaveXML = NULL;
	}
	// Free the properties read buffer
	if (pcProperties) {
		free (pcProperties);
		pcProperties = NULL;
	}
	// Free the property set read buffer
	if (pcPropertySet) {
		free (pcPropertySet);
		pcPropertySet = NULL;
	}
}

/*
 =======================================================================================================================
 *  Deal with any commands received on the connection
 =======================================================================================================================
 */
void DeviceConnection::DispatchCommand (SOCKET nSock, char * szCommand, int nCommandLength) {
	EventNewNode*  psData;
	EventDeleteNode*  psDeleteData;
	//EventPropertiesNode * psNodePropData;
	EventNewLink*  psNewLink;
	EventDeleteLink * psDeleteLink;
	cTopologyList * pList;
	pList = psWindow->getList ();
	char szText[EVENT_NAME_LEN];
	cTopology * psTopology;
	char * szNodeList;
	int nLength;
	char const * szCommandType;
	int nLengthType;
	bool boParameterFound;
	int nLenParam;
	char const * szParameter;
	int nLinkTo;
	int nLinkID;
	int nNodeFrom;
	int nNodeTo;
	char const * szFilename;
	int nFilenameLen;

	psTopology = psWindow->pTopology;

	boParameterFound = GetParameter (szCommand, nCommandLength, 0, & szCommandType, & nLengthType);

	// Figure out what the command is
	eCommand = CONCOM_INVALID;
	if (boParameterFound) {
		if (strncmp (szCommandType, "HELLO", nLengthType) == 0) {
			eCommand = CONCOM_HELLO;
		}
		else if (strncmp (szCommandType, "GOODBYE", nLengthType) == 0) {
			eCommand = CONCOM_GOODBYE;
		}
		else if (strncmp (szCommandType, "REQ_NODE_LIST", nLengthType) == 0) {
			eCommand = CONCOM_NODELIST;
		}
		else if (strncmp (szCommandType, "CREATE_LINK", nLengthType) == 0) {
			eCommand = CONCOM_CREATELINK;
		}
		else if (strncmp (szCommandType, "REMOVE_LINK", nLengthType) == 0) {
			eCommand = CONCOM_REMOVELINK;
		}
		else if (strncmp (szCommandType, "SEND_FILE", nLengthType) == 0) {
			eCommand = CONCOM_SENDFILE;
		}
		else if (strncmp (szCommandType, "SEND_PROPERTYSET", nLengthType) == 0) {
			eCommand = CONCOM_SENDPROPERTYSET;
		}
		else if (strncmp (szCommandType, "SEND_PROPERTIES", nLengthType) == 0) {
			eCommand = CONCOM_SENDPROPERTIES;
		}
	}

	// Process parameters based on the command
	switch (eCommand) {
	case CONCOM_HELLO:
		// Create the node
		psData = new EventNewNode ();
		psData->nNodeID = pList->getNextID ();
		nNodeID = psData->nNodeID;
		psData->fXPos = ((float)(rand () % 1000) / 1000.0f) - 0.5f;//(float) (SCREEN_CENTRE_X - (SCREEN_WIDTH / 2) + (rand () % SCREEN_WIDTH));
		psData->fYPos = ((float)(rand () % 1000) / 1000.0f) - 0.5f;//(float) (SCREEN_CENTRE_Y - (SCREEN_HEIGHT / 2) + (rand () % SCREEN_HEIGHT));
		psData->fZPos = 0.0f;
		psData->nType = 0;

		psData->psWindow = psWindow;
		_snprintf (szText, EVENT_NAME_LEN, "Create node %d", psData->nNodeID);
		psTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NEXT, 0, psData);

		SendCommandParams ("NODE_ID %d", nNodeID);
		break;
	case CONCOM_GOODBYE:
		// Remove the node
		psDeleteData = new EventDeleteNode ();
		psDeleteData->nNodeID = nNodeID;
		psDeleteData->psWindow = psWindow;
		_snprintf (szText, EVENT_NAME_LEN, "Delete node %d", psDeleteData->nNodeID);
		psTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NEXT, 0, psDeleteData);

		// Sending a GOODBYE back will cause the client to close the connection
		SendCommand ("GOODBYE");
		break;
	case CONCOM_NODELIST:
		// The client wants us to send us a list of the current nodes
		szNodeList = (char *)malloc (1024 * 5);
		nLength = psWindow->PopulateNodeList (szNodeList, (1024 * 5));
		SendCommand ("NODE_LIST");
		SendCommand (szNodeList, nLength);
		free (szNodeList);
		break;
	case CONCOM_CREATELINK:
		// The client is requesting that a link be created between it and another node
		boParameterFound = GetParameter (szCommand, nCommandLength, 1, & szParameter, & nLenParam);
		if (boParameterFound) {
			_snscanf (szParameter, nLenParam, "%d", & nLinkTo);
			psNewLink = new EventNewLink ();
			psNewLink->nLinkID = psTopology->GetNextLinkID ();
			psNewLink->nFrom = nNodeID;
			psNewLink ->nTo = nLinkTo;
			psNewLink->psWindow = psWindow;
			_snprintf (szText, EVENT_NAME_LEN, "Create link from %d to %d", psNewLink->nFrom, psNewLink->nTo);
			psTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NEXT, 0, psNewLink);

			SendCommandParams ("CREATE_LINK %d", psNewLink->nLinkID);
		}
		break;
	case CONCOM_REMOVELINK:
		// The client is requesting that one of its links be removed
		boParameterFound = GetParameter (szCommand, nCommandLength, 1, & szParameter, & nLenParam);
		if (boParameterFound) {
			_snscanf (szParameter, nLenParam, "%d", & nLinkID);
			psDeleteLink = new EventDeleteLink ();
			psDeleteLink->nLinkID = nLinkID;
			psDeleteLink->nNodeFromID = nNodeID;
			psDeleteLink->psWindow = psWindow;
			_snprintf (szText, EVENT_NAME_LEN, "Delete link %d from node %d", psDeleteLink->nLinkID, psDeleteLink->nNodeFromID);
			psTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NEXT, 0, psDeleteLink);

			SendCommandParams ("REMOVE_LINK %d", psDeleteLink->nLinkID);
		}
		break;
	case CONCOM_SENDFILE:
		// The client is about to send a file to another node (via us)
		boParameterFound = GetParameter (szCommand, nCommandLength, 1, & szParameter, & nLenParam);
		if (boParameterFound) {
			_snscanf (szParameter, nLenParam, "%d", & nNodeFrom);
			boParameterFound = GetParameter (szCommand, nCommandLength, 2, & szParameter, & nLenParam);
		}
		if (boParameterFound) {
			_snscanf (szParameter, nLenParam, "%d", & nNodeTo);
			boParameterFound = GetParameter (szCommand, nCommandLength, 3, & szFilename, & nFilenameLen);
		}
		if (boParameterFound) {
			boParameterFound = GetParameter (szCommand, nCommandLength, 4, & szParameter, & nLenParam);
		}
		if (boParameterFound) {
			_snscanf (szParameter, nLenParam, "%d", & nFileSize);
			eState = CONSTATE_FILE;
			nFileRead = 0;
		}
		if (boParameterFound) {
			// Now receive the file
			SendCommandParams ("FILE_SEND_READY");
		}
		break;
	case CONCOM_SENDPROPERTYSET:
		// The client is about to send a property set file
		boParameterFound = GetParameter (szCommand, nCommandLength, 1, & szParameter, & nLenParam);
		if (boParameterFound) {
			_snscanf (szParameter, nLenParam, "%d", & nNode);
			boParameterFound = GetParameter (szCommand, nCommandLength, 2, & szParameter, & nLenParam);
		}
		if (boParameterFound) {
			_snscanf (szParameter, nLenParam, "%d", & nFileSize);
			eState = CONSTATE_FILE;
			nFileRead = 0;
			nPropertySetSize = 0;
		}
		if (boParameterFound) {
			// Now receive the property set file
			SendCommandParams ("PROPERTYSET_SEND_READY");
		}
		break;
	case CONCOM_SENDPROPERTIES:
		// The client is about to send a properties file
		boParameterFound = GetParameter (szCommand, nCommandLength, 1, & szParameter, & nLenParam);
		if (boParameterFound) {
			_snscanf (szParameter, nLenParam, "%d", & nNode);
			boParameterFound = GetParameter (szCommand, nCommandLength, 2, & szParameter, & nLenParam);
		}
		if (boParameterFound) {
			_snscanf (szParameter, nLenParam, "%d", & nFileSize);
			eState = CONSTATE_FILE;
			nFileRead = 0;
			nPropertiesSize = 0;
		}
		if (boParameterFound) {
			// Now receive the properties file
			SendCommandParams ("PROPERTIES_SEND_READY");
		}
		break;
	}
}

/*
 =======================================================================================================================
 *  Parse a command and parameters using space as a delimiter
 =======================================================================================================================
 */
bool DeviceConnection::GetParameter (char const * const szCommand, int const nLength, int const nParameter, char const * * const pszParameter, int * const pnLength) {
	int nToken;
	int nStart;
	int nEnd;
	bool boFound;
	char const * szParameter;

	boFound = false;
	if (szCommand) {
		szParameter = szCommand;
		nToken = 0;
		for (nStart = 0; (nToken < nParameter) && (nStart < nLength); nStart++) {
			if (szCommand[nStart] == ' ') {
				nToken++;
			}
		}
		if (nStart < nLength) {
			// We've found the token
			boFound = true;
			// Now we need to find the end of it

			for (nEnd = (nStart + 1); (nEnd < nLength) && (szCommand[nEnd] != ' '); nEnd++) {
				// Nothing to do in here, just iterate away
			}
		}
	}
	if (pszParameter) {
		if (boFound) {
			*pszParameter = szParameter + nStart;
		}
	}
	if (pnLength) {
		if (boFound) {
			*pnLength = (nEnd - nStart);
		}
	}

	return boFound;
}

/*
 =======================================================================================================================
 *  Send a command with parameters
 =======================================================================================================================
 */
void DeviceConnection::SendCommandParams (char const * szTemplate, ...) {
	va_list ap;
	char * szCommand;
	int nSent;
	int nLength;

	// Find out how long the command will be
	va_start (ap, szTemplate);
	nLength = _vsnprintf (NULL, 0, szTemplate, ap);
	va_end (ap);

	if (nLength > 0) {
		// Allocate an appropriate amount of memory to construct the command
		szCommand = (char *)malloc (nLength + 1);

		// Construct the command
		va_start (ap, szTemplate);
		_vsnprintf (szCommand, nLength, szTemplate, ap);
		va_end (ap);

		// Send the command
		nSent = send (nAcceptSock, (char *)(& nLength), SEND_COM_PREFIX_LEN, 0);
		nSent = send (nAcceptSock, szCommand, nLength, 0);

		// Free up the allocated memory
		free (szCommand);
	}
}

/*
 =======================================================================================================================
 *  Send a command provided as a null-terminated string
 =======================================================================================================================
 */
void DeviceConnection::SendCommand (char const * szCommand) {
	int nSent;
	int nLength;

	nLength = (int)strlen (szCommand);
	nSent = send (nAcceptSock, (char *)(& nLength), SEND_COM_PREFIX_LEN, 0);
	nSent = send (nAcceptSock, szCommand, nLength, 0);
}

/*
 =======================================================================================================================
 *  Send a command provided as a data chunk with the given size
 =======================================================================================================================
 */
void DeviceConnection::SendCommand (void const * pCommand, int nLength) {
	int nSent;

	nSent = send (nAcceptSock, (char *)(& nLength), SEND_COM_PREFIX_LEN, 0);
	nSent = send (nAcceptSock, (char *)pCommand, nLength, 0);
}

/*
 =======================================================================================================================
 *  Helper function for spawning the given DeviceConnection's AcceptConnection method in a new thread
 =======================================================================================================================
 */
void AcceptConnection (DeviceConnection * psDevice) {
	psDevice->AcceptConnection ();
}

/*
 =======================================================================================================================
 *  Accept the socket connection from an external client and process any data received on it
 =======================================================================================================================
 */
void DeviceConnection::AcceptConnection () {
	char * szReceive;
	char * szSend;
	int nRecBuffEnd;
	int nReturn;
	bool boConnected;
	bool boProcessMore;
	int nMemPos;

	szSend = (char *)malloc (SEND_BUFFER);
	szReceive = (char *)malloc (RECEIVE_BUFFER);
	nRecBuffEnd = 0;
	int nProcessed;

	boConnected = true;
	while (boConnected) {
		nReturn = recv (nAcceptSock, szReceive + nRecBuffEnd, (RECEIVE_BUFFER - nRecBuffEnd), 0);
		if (nReturn != SOCKET_ERROR) {
			if (nReturn > 0) {
				nRecBuffEnd += nReturn;

				// We've received some data, so we need to deal with it
				boProcessMore = true;
				while (boProcessMore) {
					boProcessMore = false;
					nProcessed = 0;

					// Process the data based on the current state
					switch (eState) {
					case CONSTATE_COMMAND:
						nProcessed = CommandStack (szReceive, nRecBuffEnd);
						break;
					case CONSTATE_FILE:
						nProcessed = FileReceive (szReceive, nRecBuffEnd);
						break;
					default:
						// Nothing to do
						break;
					}

					// Shift down the buffer
					for (nMemPos = nProcessed; nMemPos < nRecBuffEnd; nMemPos++) {
						szReceive[nMemPos - nProcessed] = szReceive[nMemPos];
					}
					// The buffer is now smaller
					nRecBuffEnd -= nProcessed;
					if (nProcessed > 0){
						// Something was processed, so we should go around the loop at least once more
						boProcessMore = true;
					}
				}
			}
			else {
				boConnected = false;
			}
		}
		else {
			boConnected = false;
		}
	}

	// Need to lodge a request with the cServer object to remove this DeviceConnection object from the list

}

/*
 =======================================================================================================================
 *  Process any data in the buffer as structured commands
 =======================================================================================================================
 */
int DeviceConnection::CommandStack (char * szReceive, int nRecBuffEnd) {
	int nCommandLength;
	char * szCommand;
	int nProcessed;

	nProcessed = 0;
	nCommandLength = -1;
	// Is there enough data to constitute a command length?
	if (nRecBuffEnd > 4) {
		// Read in the command length
		nCommandLength = *(int *)(szReceive);
	}
	// Have we received the complete command?
	if ((nCommandLength >= 0) && (nRecBuffEnd >= (int)(nCommandLength + SEND_COM_PREFIX_LEN))) {
		// Deal with the command
		szCommand = szReceive + SEND_COM_PREFIX_LEN;
		DispatchCommand (nAcceptSock, szCommand, nCommandLength);
		nProcessed = (nCommandLength + SEND_COM_PREFIX_LEN);
	}

	return nProcessed;
}

/*
 =======================================================================================================================
 *  Process any data in the buffer as unstructured file data
 =======================================================================================================================
 */
int DeviceConnection::FileReceive (char * szReceive, int nRecBuffEnd) {
	int nProcessed;

	// Figure out how much we should read in
	nProcessed = nRecBuffEnd;
	if ((nFileRead + nProcessed) > nFileSize) {
		nProcessed = nFileSize - nFileRead;
	}

	// Do something with the new piece of file received
	FileChunkReceived (szReceive, nProcessed, eCommand);
	nFileRead += nProcessed;

	if (nFileRead >= nFileSize) {
		// All of the file has been received
		// Do something with it and switch back to commands
		FileReceived (eCommand);
		eState = CONSTATE_COMMAND;
	}

	return nProcessed;
}

/*
 =======================================================================================================================
 *  Process the unstructured file data chunks depending on their time
 =======================================================================================================================
 */
void DeviceConnection::FileChunkReceived (char * szBuffer, int nSize, CONCOM eCommand) {
	int nCopySize;

	switch (eCommand) {
	case CONCOM_SENDFILE:
		// TODO: Should assign the data to the node it's being sent to
		printf ("Okay\n");
		break;
	case CONCOM_SENDPROPERTIES:
		// TODO: Stream the file to the xml property file reader
		// Copy the data into the Properties buffer
		nCopySize = nSize;
		if (nCopySize > (PROPERTIESXML_BUFFER - nPropertiesSize)) {
			nCopySize = (PROPERTIESXML_BUFFER - nPropertiesSize);
		}
		memcpy (pcProperties + nPropertiesSize, szBuffer, nCopySize);
		nPropertiesSize += nCopySize;
		break;
	case CONCOM_SENDPROPERTYSET:
		// TODO: Stream the file to the xml property set file reader
		// Copy the data into the PropertySet buffer
		nCopySize = nSize;
		if (nCopySize > (PROPERTIESXML_BUFFER - nPropertySetSize)) {
			nCopySize = (PROPERTIESXML_BUFFER - nPropertySetSize);
		}
		memcpy (pcPropertySet + nPropertySetSize, szBuffer, nCopySize);
		nPropertySetSize += nCopySize;
		break;
	default:
		// Don't know what to do with this, so just do nothing!
		break;
	}
}

/*
 =======================================================================================================================
 *  Process the completely received unstructured file data depending on its type
 =======================================================================================================================
 */
void DeviceConnection::FileReceived (CONCOM eCommand) {
	switch (eCommand) {
	case CONCOM_SENDFILE:
		// TODO: Should do something with the completely received file
		MessageBox (NULL, "File received", "File transfer", MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
		break;
	case CONCOM_SENDPROPERTIES:
		// TODO: Should do something with the completely received property set file
		//MessageBox (NULL, "Properties file received", "File transfer", MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
		ParseProperties ();
		break;
	case CONCOM_SENDPROPERTYSET:
		// TODO: Should do something with the completely received properties file
		//MessageBox (NULL, "PropertySet file received", "File transfer", MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
		// Clear the existing properties
		psWindow->pTopology->DecodeNodeNumber (nNode)->PropertyRemoveAll ();
		ParsePropertySet ();
		break;
	default:
		// Don't know what to do with this, so just do nothing!
		break;
	}
}

/*
 =======================================================================================================================
 *  Parse the received properties file
 =======================================================================================================================
 */
void DeviceConnection::ParseProperties () {
	SAX2XMLReader * psParser;
	MemBufInputSource * psMemBufIS;
	char szMemBufID[] = "Properties";

	XMLPlatformUtils::Initialize ();

	cSaveXML * psHandler;
	psParser = XMLReaderFactory::createXMLReader ();
	psHandler = new cSaveXML (psWindow);
	psHandler->ChangeDefaults (false, nNode);
	psMemBufIS = new MemBufInputSource ((const XMLByte *)pcProperties, nPropertiesSize, szMemBufID, false);

	psHandler->LoadStart ("");

	psParser->setContentHandler (psHandler);
	psParser->setErrorHandler (psHandler);

	psParser->parse (* psMemBufIS);

	psHandler->LoadEnd ("");
	delete psMemBufIS;
	delete psParser;
	delete psHandler;
	XMLPlatformUtils::Terminate ();
}

/*
 =======================================================================================================================
 *  Parse the received property set file
 =======================================================================================================================
 */
void DeviceConnection::ParsePropertySet () {
	SAX2XMLReader * psParser;
	MemBufInputSource * psMemBufIS;
	char szMemBufID[] = "PropertySet";

	XMLPlatformUtils::Initialize ();

	cXMLPropertySet * psHandler;
	psParser = XMLReaderFactory::createXMLReader ();
	psHandler = new cXMLPropertySet (psWindow);
	psHandler->ChangeDefaults (psWindow->pTopology->DecodeNodeNumber (nNode));
	psMemBufIS = new MemBufInputSource ((const XMLByte *)pcPropertySet, nPropertySetSize, szMemBufID, false);

	psHandler->LoadStart ("");

	psParser->setContentHandler (psHandler);
	psParser->setErrorHandler (psHandler);

	psParser->parse (* psMemBufIS);

	psHandler->LoadEnd ("");
	delete psMemBufIS;
	delete psParser;
	delete psHandler;
	XMLPlatformUtils::Terminate ();
}
