/*$T MATTS/SHandler.cpp GC 1.140 07/01/09 21:12:11 */
/*
 * Name: SHandler ;
 * Purpose: handler to parse the saved network topology (XML file) into data
 * structures which can be accessed in the main program. ;
 */
#include "SHandler.h"
#include <iostream>

using namespace std;

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */

SHandler::SHandler (nodeXML tempNodeRecord[], linkXML tempLinkRecord[], int* nIndex, int* lIndex) {
	recordNode = tempNodeRecord;
	recordLink = tempLinkRecord;
	nodeIndex = nIndex;
	linkIndex = lIndex;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void SHandler::startElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname,
							 const Attributes&	attrs) {
	char*  message;
	message = XMLString::transcode (localname);

	/*
	 * Upper-case cletters represent the nodes, ;
	 * Lower-case letters represent the links
	 */
	if (!_stricmp (message, "Node"))
		(*nodeIndex)++;
	else if (!_stricmp (message, "Id"))
		propFlag = 'I';
	else if (!_stricmp (message, "Name"))
		propFlag = 'A';
	else if (!_stricmp (message, "SensitivityLevel"))
		propFlag = 'S';
	else if (!_stricmp (message, "EncryptionAlgorithm"))
		propFlag = 'E';
	else if (!_stricmp (message, "StaffSkills"))
		propFlag = 'K';
	else if (!_stricmp (message, "Firewall"))
		propFlag = 'F';
	else if (!_stricmp (message, "IDS"))
		propFlag = 'D';
	else if (!_stricmp (message, "External"))
		propFlag = 'L';
	else if (!_stricmp (message, "SecurityMax"))
		propFlag = 'C';
	else if (!_stricmp (message, "SecurityMin"))
		propFlag = 'R';
	else if (!_stricmp (message, "ExtraProperty"))
		propFlag = 'X';
	else if (!_stricmp (message, "Type"))
		propFlag = 'T';
	else if (!_stricmp (message, "Position")) {
		int i;
		i = 0;
		message = XMLString::transcode (attrs.getValue (i));
		recordNode[*nodeIndex].fXPos = (float) atof (message);
		i++;
		message = XMLString::transcode (attrs.getValue (i));
		recordNode[*nodeIndex].fYPos = (float) atof (message);
		i++;
		message = XMLString::transcode (attrs.getValue (i));
		recordNode[*nodeIndex].fZPos = (float) atof (message);
	} else if (!_stricmp (message, "Link")) (*linkIndex)++;
	else if (!_stricmp (message, "OutId"))
		propFlag = 'o';
	else if (!_stricmp (message, "InId"))
		propFlag = 'i';
	else if (!_stricmp (message, "ChannelOut"))
		propFlag = 'c';
	else if (!_stricmp (message, "ChannelIn"))
		propFlag = 'h';
	else if (!_stricmp (message, "OutProperty"))
		propFlag = 'u';
	else if (!_stricmp (message, "InProperty"))
		propFlag = 'n';
	else if (!_stricmp (message, "SecurityLevel"))
		propFlag = 's';
	else if (!_stricmp (message, "Sensitivity"))
		propFlag = 'e';
	else
		propFlag = '0';

	XMLString::release (&message);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void SHandler::characters (const XMLCh *const chars, const unsigned int length) {
	char*  message;

	message = XMLString::transcode (chars);

	if (message[0] != 10) {

		/* ignore the whitespaces created by XMLUni::fgDOMWRTFormatPrettyPrint feature */
		switch (propFlag) {
		case 'I':	/* node's Id */
			recordNode[*nodeIndex].nId = atoi (message);
			break;
		case 'A':	/* node's name */
			strcpy (recordNode[*nodeIndex].szName, message);
			break;
		case 'S':	/* node's sensitivity level */
			recordNode[*nodeIndex].nSensitivityLevel = atoi (message);
			break;
		case 'E':	/* node's encryption algorithm */
			strcpy (recordNode[*nodeIndex].szEncryptAlgorithm, message);
			break;
		case 'K':	/* node's staff skill */
			strcpy (recordNode[*nodeIndex].szStaffSkills, message);
			break;
		case 'F':	/* node's firewall */
			if (!strcmp (message, "Yes"))
				recordNode[*nodeIndex].boFirewall = true;
			else
				recordNode[*nodeIndex].boFirewall = false;
			break;
		case 'D':	/* node's ids */
			if (!strcmp (message, "Yes"))
				recordNode[*nodeIndex].boIDS = true;
			else
				recordNode[*nodeIndex].boIDS = false;
			break;
		case 'L':	/* node is external or not */
			if (!strcmp (message, "Yes"))
				recordNode[*nodeIndex].boExternal = true;
			else
				recordNode[*nodeIndex].boExternal = false;
			break;
		case 'C':	/* node's security max */
			recordNode[*nodeIndex].cSecurityMax = message[0];
			break;
		case 'R':	/* node's security min */
			recordNode[*nodeIndex].cSecurityMin = message[0];
			break;
		case 'X':	/* node's certain property */
			strcpy (recordNode[*nodeIndex].szCertProps, message);
			break;
		case 'T':	/* node's type */
			strcpy (recordNode[*nodeIndex].szType, message);
			break;
		case 'o':	/* links's out id */
			recordLink[*linkIndex].nOutId = atoi (message);
			break;
		case 'i':	/* links in id */
			recordLink[*linkIndex].nInId = atoi (message);
			break;
		case 'c':	/* link's channel out */
			recordLink[*linkIndex].nChannelOut = atoi (message);
			break;
		case 'h':	/* link's channel in */
			recordLink[*linkIndex].nChannelIn = atoi (message);
			break;
		case 'u':	/* link's out property */
			strcpy (recordLink[*linkIndex].szOutProperty, message);
			break;
		case 'n':	/* link's in property */
			strcpy (recordLink[*linkIndex].szInProperty, message);
			break;
		case 's':	/* link's security level */
			recordLink[*linkIndex].cSecurityLevel = message[0];
			break;
		case 'e':	/* link's sensitivity level */
			recordLink[*linkIndex].nSensitivityLevel = atoi (message);
			break;
		default:
			break;
		}
	}	/* end of if */

	XMLString::release (&message);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void SHandler::endElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname) {

	/*
	 * char* message = XMLString::transcode(localname);
	 * cout << "I saw element: "<< message << endl;
	 * XMLString::release(&message);
	 */
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void SHandler::fatalError (const SAXParseException& exception) {
	char*  message;
	message = XMLString::transcode (exception.getMessage ());

	cout << "Fatal Error: " << message << " at line: " << exception.getLineNumber () << endl;
}
