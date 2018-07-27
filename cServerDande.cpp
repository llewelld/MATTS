/*$T MATTS/cServerDande.cpp GC 1.140 07/01/09 21:12:09 */
#include "cServerDande.h"
#include "cProperties.h"

#define DEFAULT_PORT	(4972)
#define RECEIVE_BUFFER	(1024)
#define SEND_BUFFER	(1024)
#define SEND_COM_PREFIX_LEN	(sizeof (int))
#define COMMAND_MAXLEN (1024)
#define DANDE_RANDOMAREA (5.0f)

void DispatchCommand (SOCKET nSock, char * szCommand, int nCommandLength) ;
void SendCommand (SOCKET nSock, char * szCommand);
void AcceptConnection (DeviceConnectionDande * psDevice);

/*
 =======================================================================================================================
 *  Initialise the server object
 =======================================================================================================================
 */

cServerDande::cServerDande () {
	boRunning = false;
}

/*
 =======================================================================================================================
 *  Deinitialise the server object
 =======================================================================================================================
 */
cServerDande::~cServerDande () {
}

/*
 =======================================================================================================================
 *  Returns true if the server is currently running, false otherwise
 =======================================================================================================================
 */
bool cServerDande::GetRunning () {
	// The boRunning variable is true only when the server is running
	return boRunning;
}

/*
 =======================================================================================================================
 *  Will shut down the server when called (although it doesn't close existing connections)
 =======================================================================================================================
 */
void cServerDande::StopServer () {
	// Each listen cycle (approx every half second) checks the boListen variable
	// The server quits if boListen is set to false
	boListen = false;
}

/*
 =======================================================================================================================
 *  Listen for connections and spawn a DeviceConnectionDande object thread for each one received
 =======================================================================================================================
 */
void cServerDande::RunServer (cWindow * psWindow) {
	SOCKET nListenSock;
	SOCKET nAcceptSock;
	int nPort;
	struct sockaddr_in sHostAddr;
	struct sockaddr_in sClientAddr;
	int nReturn;
	fd_set sListenSet;
	int nSize;
	struct timeval sTime;
	DeviceConnectionDande * psDevice;

	boListen = true;
	nPort = DEFAULT_PORT;

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
								psDevice = new DeviceConnectionDande (nAcceptSock, this, psWindow);
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
 *  Initialise the DeviceConnectionDande object
 =======================================================================================================================
 */
DeviceConnectionDande::DeviceConnectionDande (SOCKET nAcceptSock, cServerDande * psServer, cWindow * psWindow) {
	this->nAcceptSock = nAcceptSock;
	this->psServer = psServer;
	this->psWindow = psWindow;
	eCommand = DANDECOM_INVALID;
	eState = DANDECONSTATE_COMMAND;
	nFileRead = 0;
	nFileSize = 0;
	// Create a cSaveXML object to load the node XML properties
	psSaveXML = new cSaveXML (psWindow);
}

/*
 =======================================================================================================================
 *  Deinitialise the DeviceConnectionDande object
 =======================================================================================================================
 */
DeviceConnectionDande::~DeviceConnectionDande () {
	// Free the cSaveXML object if there is one
	if (psSaveXML) {
		delete psSaveXML;
		psSaveXML = NULL;
	}
}

/*
 =======================================================================================================================
 *  Deal with any commands received on the connection
 =======================================================================================================================
 */
void DeviceConnectionDande::DispatchCommand (SOCKET nSock, char * szCommand, int nCommandLength) {
	EventNewNode*  psData;
	EventDeleteNode*  psDeleteData;
	EventPropertySetNode * psNodePropData;
	EventPropertySetLink * psLinkPropData;
	EventNewLink*  psNewLink;
	EventDeleteLink * psDeleteLink;
	cTopologyList * pList;
	cTopologyNode * psNode;
	cTopologyLink* psLink;
	pList = psWindow->getList ();
	char szText[EVENT_NAME_LEN];
	cTopology * psTopology;
	char const * szCommandType;
	int nLengthType;
	bool boParameterFound;
	string szParamString;
	string szPropertyName;
	string szPropertyValue;
	int nLinkID;
	int nNodeFrom;
	int nNodeTo;
	int nNodeID;
	int nValue;
	float fValue;
	bool boLinkHasName;

	psTopology = psWindow->pTopology;

	boParameterFound = GetParameter (szCommand, nCommandLength, 0, & szCommandType, & nLengthType);

	// Figure out what the command is
	eCommand = DANDECOM_INVALID;
	if (boParameterFound) {
		if (strncmp (szCommandType, "ADDNODE", nLengthType) == 0) {
			eCommand = DANDECOM_ADDNODE;
		}
		else if (strncmp (szCommandType, "SUBNODE", nLengthType) == 0) {
			eCommand = DANDECOM_SUBNODE;
		}
		else if (strncmp (szCommandType, "MOVENODE", nLengthType) == 0) {
			eCommand = DANDECOM_MOVENODE;
		}
		else if (strncmp (szCommandType, "SETNODECOLOUR", nLengthType) == 0) {
			eCommand = DANDECOM_SETNODECOLOUR;
		}
		else if (strncmp (szCommandType, "ADDLINK", nLengthType) == 0) {
			eCommand = DANDECOM_ADDLINK;
		}
		else if (strncmp (szCommandType, "SUBLINK", nLengthType) == 0) {
			eCommand = DANDECOM_SUBLINK;
		}
		else if (strncmp (szCommandType, "ADDLINKBI", nLengthType) == 0) {
			eCommand = DANDECOM_ADDLINKBI;
		}
		else if (strncmp (szCommandType, "SUBLINKBI", nLengthType) == 0) {
			eCommand = DANDECOM_SUBLINKBI;
		}
		else if (strncmp (szCommandType, "SETLINKCOLOUR", nLengthType) == 0) {
			eCommand = DANDECOM_SETLINKCOLOUR;
		}
		else if (strncmp (szCommandType, "ADDNODEPROP", nLengthType) == 0) {
			eCommand = DANDECOM_ADDNODEPROP;
		}
		else if (strncmp (szCommandType, "SUBNODEPROP", nLengthType) == 0) {
			eCommand = DANDECOM_SUBNODEPROP;
		}
		else if (strncmp (szCommandType, "ADDLINKPROP", nLengthType) == 0) {
			eCommand = DANDECOM_ADDLINKPROP;
		}
		else if (strncmp (szCommandType, "SUBLINKPROP", nLengthType) == 0) {
			eCommand = DANDECOM_SUBLINKPROP;
		}
		else if (strncmp (szCommandType, "CHANGELINK", nLengthType) == 0) {
			eCommand = DANDECOM_CHANGELINK;
		}
		else if (strncmp (szCommandType, "CLEAR", nLengthType) == 0) {
			eCommand = DANDECOM_CLEAR;
		}
		else if (strncmp (szCommandType, "QUIT", nLengthType) == 0) {
			eCommand = DANDECOM_QUIT;
		}
		else if (strncmp (szCommandType, "OKAY", nLengthType) == 0) {
			eCommand = DANDECOM_OKAY;
		}
		else if (strncmp (szCommandType, "FAIL", nLengthType) == 0) {
			eCommand = DANDECOM_FAIL;
		}
	}

	// Process parameters based on the command
	switch (eCommand) {
	case DANDECOM_ADDNODE:
	  // ADDNODE <name>
	  // Add a node to the network visualisation
		boParameterFound = GetParameter (szCommand, nCommandLength, 1, & szParamString);
		if (boParameterFound) {
			// Create the node
			psData = new EventNewNode ();
			psData->nNodeID = pList->getNextID ();
			nNodeID = psData->nNodeID;
			psData->fXPos = (((float)(rand () % 10000) / 10000.0f) - 0.5f) * DANDE_RANDOMAREA;//(float) (SCREEN_CENTRE_X - (SCREEN_WIDTH / 2) + (rand () % SCREEN_WIDTH));
			psData->fYPos = (((float)(rand () % 10000) / 10000.0f) - 0.5f) * DANDE_RANDOMAREA;//(float) (SCREEN_CENTRE_Y - (SCREEN_HEIGHT / 2) + (rand () % SCREEN_HEIGHT));
			psData->fZPos = 0.0f;
			psData->nType = 0;
			psData->psWindow = psWindow;
			_snprintf (szText, EVENT_NAME_LEN, "Create node %d", psData->nNodeID);
			psTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NEXT, 0, psData);

			// Set the node name
			psNodePropData = new EventPropertySetNode ();
			psNodePropData->nNodeID = nNodeID;
			psNodePropData->PropertySetString ("Name", szParamString.c_str ());
			psNodePropData->psWindow = psWindow;
			_snprintf (szText, EVENT_NAME_LEN, "Set node %d name", nNodeID);
			psTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NEXT, 0, psNodePropData);

			SendCommand ("OKAY");
		}
		break;
	case DANDECOM_SUBNODE:
		// SUBNODE <name>
		// Remove the node from the network visualisation
		boParameterFound = GetParameter (szCommand, nCommandLength, 1, & szParamString);
		if (boParameterFound) {
			// Find the node
			// TODO: This isn't thread safe, and it needs to be made so
			psNode = psTopology->DecodeNodeName (szParamString.c_str ());
			if (psNode) {
				nNodeID = psNode->getID ();
				// Remove the node
				psDeleteData = new EventDeleteNode ();
				psDeleteData->nNodeID = nNodeID;
				psDeleteData->psWindow = psWindow;
				_snprintf (szText, EVENT_NAME_LEN, "Delete node %d", psDeleteData->nNodeID);
				psTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NEXT, 0, psDeleteData);

				SendCommand ("OKAY");
			}
			else {
				SendCommand ("FAIL");
			}
		}
		break;
	case DANDECOM_MOVENODE:
		// MOVENODE <name> <x> <y> <z>
		// Move the node in the given direction in the network visualisation
		// TODO: Act on this command
		break;
	case DANDECOM_SETNODECOLOUR:
		// SETNODECOLOUR <name> <red> <green> <blue>
		// Set the node colour
		// TODO: Act on this command
		break;
	case DANDECOM_ADDLINK:
		// ADDLINK <From> <To>
		// Add a link to the network visualisation
		// ADDLINK <Name> <From> <To>
		// Add a named link to the network visualisation
		// Create link between two nodes
		boLinkHasName = FALSE;
		boParameterFound = GetParameter (szCommand, nCommandLength, 3, & szParamString);
		if (boParameterFound) {
			// Find the 'to' node
			// TODO: This isn't thread safe, and it needs to be made so
			psNode = psTopology->DecodeNodeName (szParamString.c_str ());
			if (psNode) {
				nNodeTo = psNode->getID ();
				boParameterFound = GetParameter (szCommand, nCommandLength, 2, & szParamString);
				if (boParameterFound) {
					// Find the 'from' node
					// TODO: This isn't thread safe, and it needs to be made so
					psNode = psTopology->DecodeNodeName (szParamString.c_str ());
					if (psNode) {
						nNodeFrom = psNode->getID ();
						boLinkHasName = TRUE;
					}
					else {
						boParameterFound = FALSE;
					}
				}
			}
			else {
				boParameterFound = FALSE;
			}
		}
		else {
			boParameterFound = GetParameter (szCommand, nCommandLength, 2, & szParamString);
			if (boParameterFound) {
				// Find the 'to' node
				// TODO: This isn't thread safe, and it needs to be made so
				psNode = psTopology->DecodeNodeName (szParamString.c_str ());
				if (psNode) {
					nNodeTo = psNode->getID ();
					boParameterFound = GetParameter (szCommand, nCommandLength, 1, & szParamString);
					if (boParameterFound) {
						// Find the 'from' node
						// TODO: This isn't thread safe, and it needs to be made so
						psNode = psTopology->DecodeNodeName (szParamString.c_str ());
						if (psNode) {
							nNodeFrom = psNode->getID ();
						}
						else {
							boParameterFound = FALSE;
						}
					}
				}
				else {
					boParameterFound = FALSE;
				}
			}
		}

		if (boParameterFound) {
			psNewLink = new EventNewLink ();
			nLinkID = psTopology->GetNextLinkID ();
			psNewLink->nLinkID = nLinkID;
			psNewLink->nFrom = nNodeFrom;
			psNewLink ->nTo = nNodeTo;
			psNewLink->psWindow = psWindow;
			_snprintf (szText, EVENT_NAME_LEN, "Create link from %d to %d", psNewLink->nFrom, psNewLink->nTo);
			psTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NEXT, 0, psNewLink);

			if (boLinkHasName) {
				boParameterFound = GetParameter (szCommand, nCommandLength, 1, & szParamString);
				if (boParameterFound) {

					// Set the link name
					psLinkPropData = new EventPropertySetLink ();
					psLinkPropData->nLinkID = nLinkID;
					psLinkPropData->PropertySetString ("Name", szParamString.c_str ());
					psLinkPropData->psWindow = psWindow;
					_snprintf (szText, EVENT_NAME_LEN, "Set link %d name", nLinkID);
					psTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NEXT, 0, psLinkPropData);
				}
			}
			SendCommand ("OKAY");
		}
		else {
			SendCommand ("FAIL");
		}
		break;
	case DANDECOM_SUBLINK:
		// SUBLINK <From> <To>
		// Remove the link from the network visualisation
		// SUBLINK <Name>
		// Remove the named link from the network visualisation
		boParameterFound = GetParameter (szCommand, nCommandLength, 1, & szParamString);
		if (boParameterFound) {
			// Find the link
			// TODO: This isn't thread safe, and it needs to be made so
			psLink = psTopology->DecodeLinkName (szParamString.c_str ());
			if (psLink) {
				nLinkID = psLink->nLinkID;
				// Remove the link
				psDeleteLink = new EventDeleteLink ();
				psDeleteLink->nLinkID = nLinkID;
				psDeleteLink->nNodeFromID = psLink->psLinkFrom->getID ();
				psDeleteLink->psWindow = psWindow;
				_snprintf (szText, EVENT_NAME_LEN, "Delete link %d", nLinkID);
				psTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NEXT, 0, psDeleteLink);

				SendCommand ("OKAY");
			}
			else {
				SendCommand ("FAIL");
			}
		}
		break;
	case DANDECOM_ADDLINKBI:
		// ADDLINKBI <From> <To>
		// Add a link to the network visualisation
		// ADDLINKBI <Name> <From> <To>
		// Add a named link to the network visualisation
		// Create link between two nodes
		boLinkHasName = FALSE;
		boParameterFound = GetParameter (szCommand, nCommandLength, 3, & szParamString);
		if (boParameterFound) {
			// Find the 'to' node
			// TODO: This isn't thread safe, and it needs to be made so
			psNode = psTopology->DecodeNodeName (szParamString.c_str ());
			if (psNode) {
				nNodeTo = psNode->getID ();
				boParameterFound = GetParameter (szCommand, nCommandLength, 2, & szParamString);
				if (boParameterFound) {
					// Find the 'from' node
					// TODO: This isn't thread safe, and it needs to be made so
					psNode = psTopology->DecodeNodeName (szParamString.c_str ());
					if (psNode) {
						nNodeFrom = psNode->getID ();
						boLinkHasName = TRUE;
					}
					else {
						boParameterFound = FALSE;
					}
				}
			}
			else {
				boParameterFound = FALSE;
			}
		}
		else {
			boParameterFound = GetParameter (szCommand, nCommandLength, 2, & szParamString);
			if (boParameterFound) {
				// Find the 'to' node
				// TODO: This isn't thread safe, and it needs to be made so
				psNode = psTopology->DecodeNodeName (szParamString.c_str ());
				if (psNode) {
					nNodeTo = psNode->getID ();
					boParameterFound = GetParameter (szCommand, nCommandLength, 1, & szParamString);
					if (boParameterFound) {
						// Find the 'from' node
						// TODO: This isn't thread safe, and it needs to be made so
						psNode = psTopology->DecodeNodeName (szParamString.c_str ());
						if (psNode) {
							nNodeFrom = psNode->getID ();
						}
						else {
							boParameterFound = FALSE;
						}
					}
				}
				else {
					boParameterFound = FALSE;
				}
			}
		}

		if (boParameterFound) {
			psNewLink = new EventNewLink ();
			nLinkID = psTopology->GetNextLinkID ();
			psNewLink->nLinkID = nLinkID;
			psNewLink->nFrom = nNodeFrom;
			psNewLink ->nTo = nNodeTo;
			psNewLink->psWindow = psWindow;
			_snprintf (szText, EVENT_NAME_LEN, "Create link from %d to %d", psNewLink->nFrom, psNewLink->nTo);
			psTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NEXT, 0, psNewLink);

			if (boLinkHasName) {
				boParameterFound = GetParameter (szCommand, nCommandLength, 1, & szParamString);
				if (boParameterFound) {

					// Set the link name
					psLinkPropData = new EventPropertySetLink ();
					psLinkPropData->nLinkID = nLinkID;
					psLinkPropData->PropertySetString ("Name", szParamString.c_str ());
					psLinkPropData->psWindow = psWindow;
					_snprintf (szText, EVENT_NAME_LEN, "Set link %d name", nLinkID);
					psTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NEXT, 0, psLinkPropData);
				}
			}
			SendCommand ("OKAY");
		}
		else {
			SendCommand ("FAIL");
		}
		break;
	case DANDECOM_SUBLINKBI:
		// SUBLINKBI <From> <To>
		// Remove the link from the network visualisation
		// SUBLINKBI <Name>
		// Remove the named link from the network visualisation
		boParameterFound = GetParameter (szCommand, nCommandLength, 1, & szParamString);
		if (boParameterFound) {
			// Find the link
			// TODO: This isn't thread safe, and it needs to be made so
			psLink = psTopology->DecodeLinkName (szParamString.c_str ());
			if (psLink) {
				nLinkID = psLink->nLinkID;
				// Remove the link
				psDeleteLink = new EventDeleteLink ();
				psDeleteLink->nLinkID = nLinkID;
				psDeleteLink->nNodeFromID = psLink->psLinkFrom->getID ();
				psDeleteLink->psWindow = psWindow;
				_snprintf (szText, EVENT_NAME_LEN, "Delete link %d", nLinkID);
				psTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NEXT, 0, psDeleteLink);

				SendCommand ("OKAY");
			}
			else {
				SendCommand ("FAIL");
			}
		}
		break;
	case DANDECOM_SETLINKCOLOUR:
		// SETLINKCOLOUR <name> <red> <green> <blue>
		// Set the link colour
		// TODO: Act on this command
		break;
	case DANDECOM_ADDNODEPROP:
		// ADDNODEPROP <Node> <Property> <Type> <Value>
		// Add a property to a node
		boParameterFound = GetParameter (szCommand, nCommandLength, 1, & szParamString);
		if (boParameterFound) {
			psNode = psTopology->DecodeNodeName (szParamString.c_str ());
			if (psNode) {
				// Set the node property
				psNodePropData = new EventPropertySetNode ();
				nNodeID = psNode->getID ();
				psNodePropData->nNodeID = nNodeID;

				boParameterFound = GetParameter (szCommand, nCommandLength, 3, & szParamString);
				boParameterFound &= GetParameter (szCommand, nCommandLength, 2, & szPropertyName);
				boParameterFound &= GetParameter (szCommand, nCommandLength, 2, & szPropertyValue);
				if (boParameterFound) {
					if (_stricmp (szParamString.c_str (), "int") == 0) {
						_snscanf_s (szPropertyValue.c_str (), szPropertyValue.length (), "%d", & nValue);
						psNodePropData->PropertySetInt (szPropertyName.c_str (), nValue);
					}
					else if (_stricmp (szParamString.c_str (), "float") == 0) {
						_snscanf_s (szPropertyValue.c_str (), szPropertyValue.length (), "%f", & fValue);
						psNodePropData->PropertySetFloat (szPropertyName.c_str (), fValue);
					}
					else if (_stricmp (szParamString.c_str (), "string") == 0) {
						psNodePropData->PropertySetString (szPropertyName.c_str (), szPropertyValue.c_str ());
					}

					psNodePropData->psWindow = psWindow;
					_snprintf (szText, EVENT_NAME_LEN, "Set node %d property", nNodeID);
					psTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NEXT, 0, psNodePropData);

					SendCommand ("OKAY");
				}
				else {
					SendCommand ("FAIL");
				}
			}
			else {
				SendCommand ("FAIL");
			}
		}
		break;
	case DANDECOM_SUBNODEPROP:
		// SUBNODEPROP <Node> <Property>
		// Remove a property from a node
		// TODO: Act on this command
		break;
	case DANDECOM_ADDLINKPROP:
		// ADDLINKPROP <Link> <Property> <Type> <Value>
		// Add a property to a link
		boParameterFound = GetParameter (szCommand, nCommandLength, 1, & szParamString);
		if (boParameterFound) {
			psLink = psTopology->DecodeLinkName (szParamString.c_str ());
			if (psLink) {
				// Set the link property
				psLinkPropData = new EventPropertySetLink ();
				nLinkID = psLink->nLinkID;
				psLinkPropData->nLinkID = nLinkID;

				boParameterFound = GetParameter (szCommand, nCommandLength, 3, & szParamString);
				boParameterFound &= GetParameter (szCommand, nCommandLength, 2, & szPropertyName);
				boParameterFound &= GetParameter (szCommand, nCommandLength, 2, & szPropertyValue);
				if (boParameterFound) {
					if (_stricmp (szParamString.c_str (), "int") == 0) {
						_snscanf_s (szPropertyValue.c_str (), szPropertyValue.length (), "%d", & nValue);
						psLinkPropData->PropertySetInt (szPropertyName.c_str (), nValue);
					}
					else if (_stricmp (szParamString.c_str (), "float") == 0) {
						_snscanf_s (szPropertyValue.c_str (), szPropertyValue.length (), "%f", & fValue);
						psLinkPropData->PropertySetFloat (szPropertyName.c_str (), fValue);
					}
					else if (_stricmp (szParamString.c_str (), "string") == 0) {
						psLinkPropData->PropertySetString (szPropertyName.c_str (), szPropertyValue.c_str ());
					}

					psLinkPropData->psWindow = psWindow;
					_snprintf (szText, EVENT_NAME_LEN, "Set link %d property", nLinkID);
					psTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NEXT, 0, psLinkPropData);

					SendCommand ("OKAY");
				}
				else {
					SendCommand ("FAIL");
				}
			}
			else {
				SendCommand ("FAIL");
			}
		}
		break;
	case DANDECOM_SUBLINKPROP:
		// SUBLINKPROP <Node> <Property>
		// Remove a property from a link
		// TODO: Act on this command
		break;
	case DANDECOM_CHANGELINK:
		// CHANGELINK <OldFrom> <OldTo> <NewFrom> <NewTo>
		// A link has been moved to a new node via the network visualisation
		// TODO: Act on this command
		break;
	case DANDECOM_CLEAR:
		// CLEAR
		// Remove all nodes and linsk from the network visualisation
		// TODO: Act on this command
		break;
	case DANDECOM_QUIT:
		// QUIT
		// Okay to quit
		// TODO: Act on this command
		break;
	case DANDECOM_OKAY:
		// TODO: Act on this command
		break;
	case DANDECOM_FAIL:
		// TODO: Act on this command
		break;
	}
}

/*
 =======================================================================================================================
 *  Parse a command and parameters using space as a delimiter
 =======================================================================================================================
 */
bool DeviceConnectionDande::GetParameter (char const * const szCommand, int const nLength, int const nParameter, char const * * const pszParameter, int * const pnLength) {
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
 *  Parse a command and parameters using space as a delimiter
 =======================================================================================================================
 */
bool DeviceConnectionDande::GetParameter (char const * const szCommand, int const nLength, int const nParameter, string * pszParameter) {
	int nParamLength;
	bool boFound;
	char const * szParameter;

	boFound = GetParameter (szCommand, nLength, nParameter, & szParameter, & nParamLength);
	if (boFound) {
		pszParameter->assign (szParameter, nParamLength);
	}

	return boFound;
}

/*
 =======================================================================================================================
 *  Send a command with parameters
 =======================================================================================================================
 */
void DeviceConnectionDande::SendCommandParams (char const * szTemplate, ...) {
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
		szCommand = (char *)malloc (nLength + 2);

		// Construct the command
		va_start (ap, szTemplate);
		_vsnprintf (szCommand, nLength, szTemplate, ap);
		va_end (ap);

		// Send the command
		//nSent = send (nAcceptSock, (char *)(& nLength), SEND_COM_PREFIX_LEN, 0);
		nSent = send (nAcceptSock, szCommand, nLength, 0);
		// Send the trailing newline
		nSent = send (nAcceptSock, "\n", 1, 0);

		// Free up the allocated memory
		free (szCommand);
	}
}

/*
 =======================================================================================================================
 *  Send a command provided as a null-terminated string
 =======================================================================================================================
 */
void DeviceConnectionDande::SendCommand (char const * szCommand) {
	int nSent;
	int nLength;

	nLength = (int)strlen (szCommand);
	//nSent = send (nAcceptSock, (char *)(& nLength), SEND_COM_PREFIX_LEN, 0);
	nSent = send (nAcceptSock, szCommand, nLength, 0);
	// Send the trailing newline
	nSent = send (nAcceptSock, "\n", 1, 0);
}

/*
 =======================================================================================================================
 *  Send a command provided as a data chunk with the given size
 =======================================================================================================================
 */
void DeviceConnectionDande::SendCommand (void const * pCommand, int nLength) {
	int nSent;

	//nSent = send (nAcceptSock, (char *)(& nLength), SEND_COM_PREFIX_LEN, 0);
	nSent = send (nAcceptSock, (char *)pCommand, nLength, 0);
	// Send the trailing newline
	nSent = send (nAcceptSock, "\n", 1, 0);
}

/*
 =======================================================================================================================
 *  Helper function for spawning the given DeviceConnectionDande's AcceptConnection method in a new thread
 =======================================================================================================================
 */
void AcceptConnection (DeviceConnectionDande * psDevice) {
	psDevice->AcceptConnection ();
}

/*
 =======================================================================================================================
 *  Accept the socket connection from an external client and process any data received on it
 =======================================================================================================================
 */
void DeviceConnectionDande::AcceptConnection () {
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
					case DANDECONSTATE_COMMAND:
						nProcessed = CommandStack (szReceive, nRecBuffEnd);
						break;
					case DANDECONSTATE_FILE:
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

	// Need to lodge a request with the cServerDande object to remove this DeviceConnectionDande object from the list

}

/*
 =======================================================================================================================
 *  Process any data in the buffer as structured commands
 =======================================================================================================================
 */
int DeviceConnectionDande::CommandStack (char * szReceive, int nRecBuffEnd) {
	int nCommandLength;
	char * szCommand;
	int nProcessed;
	int nPos;

	nProcessed = 0;
	nCommandLength = -1;
	for (nPos = 0; ((nPos < nRecBuffEnd) && (nCommandLength < 0)); nPos++) {
		if (szReceive[nPos] == '\n') {
			nCommandLength = nPos;
		}
	}

	// Have we received the complete command?
	if ((nCommandLength >= 0) && (nRecBuffEnd >= (nCommandLength + 1))) {
		// Deal with the command
		szCommand = szReceive;
		DispatchCommand (nAcceptSock, szCommand, nCommandLength);
		nProcessed = (nCommandLength + 1);
	}

	return nProcessed;
}

/*
 =======================================================================================================================
 *  Process any data in the buffer as unstructured file data
 =======================================================================================================================
 */
int DeviceConnectionDande::FileReceive (char * szReceive, int nRecBuffEnd) {
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
		eState = DANDECONSTATE_COMMAND;
	}

	return nProcessed;
}

/*
 =======================================================================================================================
 *  Process the unstructured file data chunks depending on their time
 =======================================================================================================================
 */
void DeviceConnectionDande::FileChunkReceived (char * szBuffer, int nSize, DANDECOM eCommand) {
	switch (eCommand) {
	case DANDECOM_INVALID:
		// Something's gone wrong: this should never happen
		MessageBox (NULL, "Error during file transfer", "Server (Dandelion)", MB_OK | MB_ICONERROR | MB_APPLMODAL);
		break;
	default:
		// TODO: Should do something with the file, depending on the command it's associated with
		// Don't know what to do with this, so just do nothing!
		break;
	}
}

/*
 =======================================================================================================================
 *  Process the completely received unstructured file data depending on its type
 =======================================================================================================================
 */
void DeviceConnectionDande::FileReceived (DANDECOM eCommand) {
	switch (eCommand) {
	case DANDECOM_INVALID:
		// Something's gone wrong: this should never happen
		MessageBox (NULL, "Error during file transfer", "Server (Dandelion)", MB_OK | MB_ICONERROR | MB_APPLMODAL);
		break;
	default:
		// TODO: Should do something with the file, depending on the command it's associated with
		// Don't know what to do with this, so just do nothing!
		break;
	}
}

