/*$T MATTS/cTimer.h GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cTimer.h ;
 * Last Modified: 17/06/08 ;
 * ;
 * Purpose: Header file for cTimer ;
 */

#include "Event.h"

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
Event::~Event () {
	// Nothing to do
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool Event::Callback (unsigned int uHandle, float fCompleted) {
	return TRUE;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void Event::Save (FILE * hFile, int nIndent) {
	fprintf (hFile, "%*s<Event type=\"Generic\">\n", nIndent * 2, " ");
	fprintf (hFile, "%*s</Event>\n", nIndent * 2, " ");
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void Event::Load (char const * szProperty, char const * szType, char const * szValue) {
	printf ("%s : %s : %s\n", szProperty, szType, szValue);
	// Nothing to do
}

/*
 =======================================================================================================================
 *  Save out a property of type int to the event Timer XML file
 =======================================================================================================================
 */
void Event::SaveElement (FILE * hFile, int nIndent, char const * szName, int nValue) {
	fprintf (hFile, "%*s<Property id=\"%s\" type=\"int\">%d</Property>\n", nIndent * 2, " ", szName, nValue);
}

/*
 =======================================================================================================================
 *  Save out a property of type string to the event Timer XML file
 =======================================================================================================================
 */
void Event::SaveElement (FILE * hFile, int nIndent, char const * szName, char const * szValue) {
	fprintf (hFile, "%*s<Property id=\"%s\" type=\"string\">%s</Property>\n", nIndent * 2, " ", szName, szValue);
}

/*
 =======================================================================================================================
 *  Save out a property of type float to the event Timer XML file
 =======================================================================================================================
 */
void Event::SaveElement (FILE * hFile, int nIndent, char const * szName, float fValue) {
	fprintf (hFile, "%*s<Property id=\"%s\" type=\"float\">%f</Property>\n", nIndent * 2, " ", szName, fValue);
}

/*
 =======================================================================================================================
 *  Save out a property of type bool to the event Timer XML file
 =======================================================================================================================
 */
void Event::SaveElement (FILE * hFile, int nIndent, char const * szName, bool boValue) {
	fprintf (hFile, "%*s<Property id=\"%s\" type=\"bool\">%d</Property>\n", nIndent * 2, " ", szName, boValue);
}

/*
 =======================================================================================================================
 *  Save out a property of a given type to the event Timer XML file
 =======================================================================================================================
 */
void  Event::SaveElement (FILE * hFile, int nIndent, char const * szName, char const * szType, char const * szValue) {
	fprintf (hFile, "%*s<Property id=\"%s\" type=\"%s\">%s</Property>\n", nIndent * 2, " ", szName, szType, szValue);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool EventNewNode::Callback (unsigned int uHandle, float fCompleted) {
	if (fCompleted <= 0.0f) {
		psWindow->CreateNode (nNodeID, fXPos, fYPos, fZPos, nType);
	}

	return TRUE;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventNewNode::Save (FILE * hFile, int nIndent) {
	fprintf (hFile, "%*s<Event type=\"NewNode\">\n", nIndent * 2, " ");
	nIndent++;
	SaveElement (hFile, nIndent, "nodeid", nNodeID);
	SaveElement (hFile, nIndent, "nodetype", (int)nType);
	SaveElement (hFile, nIndent, "xpos", fXPos);
	SaveElement (hFile, nIndent, "ypos", fYPos);
	SaveElement (hFile, nIndent, "zpos", fZPos);
	nIndent--;
	fprintf (hFile, "%*s</Event>\n", nIndent * 2, " ");
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventNewNode::Load (char const * szProperty, char const * szType, char const * szValue) {
	if (_stricmp (szProperty, "nodeid") == 0) {
		nNodeID = atoi (szValue);
	}
	else if (_stricmp (szProperty, "nodetype") == 0) {
		nType = atoi (szValue);
	}
	else if (_stricmp (szProperty, "xpos") == 0) {
		fXPos = (float)atof (szValue);
	}
	else if (_stricmp (szProperty, "ypos") == 0) {
		fYPos = (float)atof (szValue);
	}
	else if (_stricmp (szProperty, "zpos") == 0) {
		fZPos = (float)atof (szValue);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool EventNewLink::Callback (unsigned int uHandle, float fCompleted) {
	if (fCompleted <= 0.0f) {
		psWindow->CreateLink (nFrom, nTo, nLinkID);
	}

	return TRUE;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventNewLink::Save (FILE * hFile, int nIndent) {
	fprintf (hFile, "%*s<Event type=\"NewLink\">\n", nIndent * 2, " ");
	nIndent++;
	SaveElement (hFile, nIndent, "nodefrom", nFrom);
	SaveElement (hFile, nIndent, "nodeto", nTo);
	SaveElement (hFile, nIndent, "linkid", nLinkID);
	nIndent--;
	fprintf (hFile, "%*s</Event>\n", nIndent * 2, " ");
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventNewLink::Load (char const * szProperty, char const * szType, char const * szValue) {
	if (_stricmp (szProperty, "nodefrom") == 0) {
		nFrom = atoi (szValue);
	}
	else if (_stricmp (szProperty, "nodeto") == 0) {
		nTo = atoi (szValue);
	}
	else if (_stricmp (szProperty, "linkid") == 0) {
		nLinkID = atoi (szValue);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool EventMoveNode::Callback (unsigned int uHandle, float fCompleted) {
	cTopologyNode*	psNode;
	float fXPos;
	float fYPos;
	float fZPos;

	psNode = psWindow->pTopology->DecodeNodeNumber (nNodeID);
	if (psNode) {
		fXPos = psNode->fXPos + ((fXPosTo - psNode->fXPos) * fCompleted);
		fYPos = psNode->fYPos + ((fYPosTo - psNode->fYPos) * fCompleted);
		fZPos = psNode->fZPos + ((fZPosTo - psNode->fZPos) * fCompleted);
		psWindow->pTopology->MoveNode (psNode, fXPos, fYPos, 0);
		psWindow->UpdateAdHocLinks (psNode);
	}

	return TRUE;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventMoveNode::Save (FILE * hFile, int nIndent) {
	fprintf (hFile, "%*s<Event type=\"MoveNode\">\n", nIndent * 2, " ");
	nIndent++;
	SaveElement (hFile, nIndent, "nodeid", nNodeID);
	SaveElement (hFile, nIndent, "xpos", fXPosTo);
	SaveElement (hFile, nIndent, "ypos", fYPosTo);
	SaveElement (hFile, nIndent, "zpos", fZPosTo);
	nIndent--;
	fprintf (hFile, "%*s</Event>\n", nIndent * 2, " ");
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventMoveNode::Load (char const * szProperty, char const * szType, char const * szValue) {
	if (_stricmp (szProperty, "nodeid") == 0) {
		nNodeID = atoi (szValue);
	}
	else if (_stricmp (szProperty, "xpos") == 0) {
		fXPosTo = (float)atof (szValue);
	}
	else if (_stricmp (szProperty, "ypos") == 0) {
		fYPosTo = (float)atof (szValue);
	}
	else if (_stricmp (szProperty, "zpos") == 0) {
		fZPosTo = (float)atof (szValue);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool EventDeleteNode::Callback (unsigned int uHandle, float fCompleted) {
	if (fCompleted <= 0.0f) {
		psWindow->DeleteNode (nNodeID);
	}

	return TRUE;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventDeleteNode::Save (FILE * hFile, int nIndent) {
	fprintf (hFile, "%*s<Event type=\"DeleteNode\">\n", nIndent * 2, " ");
	nIndent++;
	SaveElement (hFile, nIndent, "nodeid", nNodeID);
	nIndent--;
	fprintf (hFile, "%*s</Event>\n", nIndent * 2, " ");
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventDeleteNode::Load (char const * szProperty, char const * szType, char const * szValue) {
	if (_stricmp (szProperty, "nodeid") == 0) {
		nNodeID = atoi (szValue);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool EventDeleteLink::Callback (unsigned int uHandle, float fCompleted) {
	if (fCompleted <= 0.0f) {
		psWindow->DeleteLink (nNodeFromID, nLinkID);
	}

	return TRUE;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventDeleteLink::Save (FILE * hFile, int nIndent) {
	fprintf (hFile, "%*s<Event type=\"DeleteLink\">\n", nIndent * 2, " ");
	nIndent++;
	SaveElement (hFile, nIndent, "nodeid", nNodeFromID);
	SaveElement (hFile, nIndent, "linkid", nLinkID);
	nIndent--;
	fprintf (hFile, "%*s</Event>\n", nIndent * 2, " ");
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventDeleteLink::Load (char const * szProperty, char const * szType, char const * szValue) {
	if (_stricmp (szProperty, "nodeid") == 0) {
		nNodeFromID = atoi (szValue);
	}
	else if (_stricmp (szProperty, "linkid") == 0) {
		nLinkID = atoi (szValue);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool EventPropertiesNode::Callback (unsigned int uHandle, float fCompleted) {
	cTopologyNode*	psNode;
	cTopologyList*	pList;
	cTopologyNode*	psIterator;

	if (fCompleted <= 0.0f) {

		/* Find the correct node */
		psNode = NULL;
		pList = psWindow->pList;
		if (pList) {
			psIterator = pList->startIterator ();
			while ((pList->isIteratorValid (psIterator)) && (pList->getCurrentID (psIterator) != nNodeID)) {
				psIterator = pList->moveForward (psIterator);
			}
		}

		if (pList->isIteratorValid (psIterator)) {

			/* Set the properties of the node */
			psNode = pList->getCurrentNode (psIterator);

			/* Arbitrary properties */
			if (szCertProps) {
				strncpy (psNode->szCertProps, szCertProps, 1024);
			}

			/* Security max */
			psNode->nSecurityMax = nSecurityMax;

			/* Security min */
			psNode->nSecurityMin = nSecurityMin;

			/* Name */
			if (szName) {
				psNode->SetName (szName);
			}

			/* Sensitivity level */
			psNode->nSensitivityLevel = nSensitivityLevel;

			/* Encryption algorithm */
			strncpy (psNode->szEncryptAlgorithm, szEncryptAlgorithm, ENCRYPT_ALGORITHM);

			/* Staff skills */
			strncpy (psNode->szStaffSkills, szStaffSkills, STAFF_SKILLS);

			/* Firewall */
			psNode->boFirewall = boFirewall;

			/* IDS */
			psNode->boIDS = boIDS;

			/* External */
			psNode->boExternal = boExternal;

			/* Ad-hoc characteristics */
			psNode->boAdHoc = boAdHoc;
			psNode->fRangeTx = fRangeTx;
			psNode->fRangeRx = fRangeRx;

			/* Apply these properties */
			psNode->ApplyCharacteristics ();

			/* Reset all of the In and Out tables */
			psNode->NodeSecurityReset ();

			/* Sort out any ad hoc links */
			psWindow->UpdateAdHocLinks (psNode);
			psWindow->pTopology->psSelectedLink = NULL;
		}

		/* Set to analyse */
		psWindow->TopologyChanged ();
	}

	return TRUE;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
EventPropertiesNode::EventPropertiesNode () {
	szCertProps = NULL;
	szName = NULL;
	szEncryptAlgorithm = NULL;
	szStaffSkills = NULL;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
EventPropertiesNode::~EventPropertiesNode () {
	if (szCertProps) {
		free ((void*) szCertProps);
	}

	if (szName) {
		free ((void*) szName);
	}

	if (szEncryptAlgorithm) {
		free ((void*) szEncryptAlgorithm);
	}

	if (szStaffSkills) {
		free ((void*) szStaffSkills);
	}

}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventPropertiesNode::Save (FILE * hFile, int nIndent) {
	fprintf (hFile, "%*s<Event type=\"PropertiesNode\">\n", nIndent * 2, " ");
	nIndent++;
	SaveElement (hFile, nIndent, "nodeid", nNodeID);
	SaveElement (hFile, nIndent, "certprops", szCertProps);
	SaveElement (hFile, nIndent, "secmax", nSecurityMax);
	SaveElement (hFile, nIndent, "secmin", nSecurityMin);
	SaveElement (hFile, nIndent, "name", szName);
	SaveElement (hFile, nIndent, "sensitivity", nSensitivityLevel);
	SaveElement (hFile, nIndent, "encryptalgorithm", szEncryptAlgorithm);
	SaveElement (hFile, nIndent, "staffskills", szStaffSkills);
	SaveElement (hFile, nIndent, "firewall", boFirewall);
	SaveElement (hFile, nIndent, "ids", boIDS);
	SaveElement (hFile, nIndent, "external", boExternal);
	SaveElement (hFile, nIndent, "adhoc", boAdHoc);
	SaveElement (hFile, nIndent, "rangetx", fRangeTx);
	SaveElement (hFile, nIndent, "rangerx", fRangeRx);
	nIndent--;
	fprintf (hFile, "%*s</Event>\n", nIndent * 2, " ");
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventPropertiesNode::Load (char const * szProperty, char const * szType, char const * szValue) {
	if (_stricmp (szProperty, "nodeid") == 0) {
		nNodeID = atoi (szValue);
	}
	else if (_stricmp (szProperty, "certprops") == 0) {
		szCertProps = (char *)malloc (strlen (szValue) + 1);
		strcpy (szCertProps, szValue);
	}
	else if (_stricmp (szProperty, "secmax") == 0) {
		nSecurityMax = atoi (szValue);
	}
	else if (_stricmp (szProperty, "secmin") == 0) {
		nSecurityMin = atoi (szValue);
	}
	else if (_stricmp (szProperty, "name") == 0) {
		szName = (char *)malloc (strlen (szValue) + 1);
		strcpy (szName, szValue);
	}
	else if (_stricmp (szProperty, "sensitivity") == 0) {
		nSensitivityLevel = atoi (szValue);
	}
	else if (_stricmp (szProperty, "encryptalgorithm") == 0) {
		szEncryptAlgorithm = (char *)malloc (strlen (szValue) + 1);
		strcpy (szEncryptAlgorithm, szValue);
	}
	else if (_stricmp (szProperty, "staffskills") == 0) {
		szStaffSkills = (char *)malloc (strlen (szValue) + 1);
		strcpy (szStaffSkills, szValue);
	}
	else if (_stricmp (szProperty, "firewall") == 0) {
		boFirewall = (atoi (szValue) != 0);
	}
	else if (_stricmp (szProperty, "ids") == 0) {
		boIDS = (atoi (szValue) != 0);
	}
	else if (_stricmp (szProperty, "external") == 0) {
		boExternal = (atoi (szValue) != 0);
	}
	else if (_stricmp (szProperty, "adhoc") == 0) {
		boAdHoc = (atoi (szValue) != 0);
	}
	else if (_stricmp (szProperty, "rangetx") == 0) {
		fRangeTx = (float)atof (szValue);
	}
	else if (_stricmp (szProperty, "rangerx") == 0) {
		fRangeRx = (float)atof (szValue);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool EventPropertiesLink::Callback (unsigned int uHandle, float fCompleted) {
	cTopologyLink*	psLink;
	cTopologyList*	pList;
	cTopologyNode*	psIterator;

	if (fCompleted <= 0.0f) {

		/* Find the correct link */
		psLink = NULL;
		pList = psWindow->pTopology->GetpList ();
		if (pList) {
			psIterator = pList->startIterator ();
			while ((pList->isIteratorValid (psIterator)) && (nNodeFromID != pList->getCurrentID (psIterator))) {
				psIterator = pList->moveForward (psIterator);
			}

			if (pList->isIteratorValid (psIterator)) {
				psLink = pList->getLinksOut (psIterator);
				while (psLink && (psLink->nLinkID != nLinkID)) {
					psLink = psLink->pFromNext;
				}

				if (psLink) {

					/*
					 * Set the properties of the link ;
					 * Channel out
					 */
					psLink->nChannelOut = nChannelOut;

					/* Channel in */
					psLink->nChannelIn = nChannelIn;

					/* Arbitrary link output properties */
					if (szLinkOutput) {
						strncpy (psLink->szLinkOutput, szLinkOutput, 1024);
					}

					/* Arbitrary link input properties */
					if (szLinkInput) {
						strncpy (psLink->szLinkInput, szLinkInput, 1024);
					}

					/* Security level */
					psLink->nSecurity = nSecurity;

					/* Sensitivity level */
					psLink->nSensitivityLevel = nSensitivityLevel;
				}
			}
		}

		/* Set to analyse */
		psWindow->TopologyChanged ();
	}

	return TRUE;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
EventPropertiesLink::EventPropertiesLink () {
	szLinkOutput = NULL;
	szLinkInput = NULL;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
EventPropertiesLink::~EventPropertiesLink () {

	if (szLinkOutput) {
		free ((void*) szLinkOutput);
	}

	if (szLinkInput) {
		free ((void*) szLinkInput);
	}

}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventPropertiesLink::Save (FILE * hFile, int nIndent) {
	fprintf (hFile, "%*s<Event type=\"PropertiesLink\">\n", nIndent * 2, " ");
	nIndent++;
	SaveElement (hFile, nIndent, "nodeid", nNodeFromID);
	SaveElement (hFile, nIndent, "linkid", nLinkID);
	SaveElement (hFile, nIndent, "channelout", nChannelOut);
	SaveElement (hFile, nIndent, "channelin", nChannelIn);
	SaveElement (hFile, nIndent, "linkoutput", szLinkOutput);
	SaveElement (hFile, nIndent, "linkinput", szLinkInput);
	SaveElement (hFile, nIndent, "sec", nSecurity);
	SaveElement (hFile, nIndent, "sensitivity", nSensitivityLevel);
	nIndent--;
	fprintf (hFile, "%*s</Event>\n", nIndent * 2, " ");
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventPropertiesLink::Load (char const * szProperty, char const * szType, char const * szValue) {
	if (_stricmp (szProperty, "nodeid") == 0) {
		nNodeFromID = atoi (szValue);
	}
	else if (_stricmp (szProperty, "linkid") == 0) {
		nLinkID = atoi (szValue);
	}
	else if (_stricmp (szProperty, "channelout") == 0) {
		nChannelOut = atoi (szValue);
	}
	else if (_stricmp (szProperty, "channelin") == 0) {
		nChannelIn = atoi (szValue);
	}
	else if (_stricmp (szProperty, "linkoutput") == 0) {
		szLinkOutput = (char *)malloc (strlen (szValue) + 1);
		strcpy (szLinkOutput, szValue);
	}
	else if (_stricmp (szProperty, "linkinput") == 0) {
		szLinkInput = (char *)malloc (strlen (szValue) + 1);
		strcpy (szLinkInput, szValue);
	}
	else if (_stricmp (szProperty, "sec") == 0) {
		nSecurity = atoi (szValue);
	}
	else if (_stricmp (szProperty, "sensitivity") == 0) {
		nSensitivityLevel = atoi (szValue);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
EventPropertySetNode::EventPropertySetNode () {
	nNodeID = 0;
	PropertyRemoveAll ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
EventPropertySetNode::~EventPropertySetNode () {
	PropertyRemoveAll ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool EventPropertySetNode::Callback (unsigned int uHandle, float fCompleted) {
	psWindow->pTopology->DecodeNodeNumber (nNodeID)->PropertyTransfer (this);
	return true;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventPropertySetNode::Save (FILE * hFile, int nIndent) {
	PropertyMap::const_iterator sPropertyIter;
	PropertyMap const * pcPropertyMap;
	list<string>::const_iterator sOptions;

	fprintf (hFile, "%*s<Event type=\"PropertySetNode\">\n", nIndent * 2, " ");
	nIndent++;
	// Save out the node ID
	SaveElement (hFile, nIndent, "nodeid", nNodeID);

	// Save out each of the individual properties
	pcPropertyMap = GetPropertyMap ();
	for (sPropertyIter = pcPropertyMap->begin (); sPropertyIter != pcPropertyMap->end (); ++sPropertyIter) {
		switch (sPropertyIter->second.eType) {
		case PROPTYPE_BOOL:
			SaveElement (hFile, nIndent, sPropertyIter->first.c_str (), sPropertyIter->second.boValue);
			break;
		case PROPTYPE_FLOAT:
			SaveElement (hFile, nIndent, sPropertyIter->first.c_str (), sPropertyIter->second.fValue);
			break;
		case PROPTYPE_INT:
			SaveElement (hFile, nIndent, sPropertyIter->first.c_str (), sPropertyIter->second.nValue);
			break;
		case PROPTYPE_STRING:
			SaveElement (hFile, nIndent, sPropertyIter->first.c_str (), sPropertyIter->second.szValue);
			break;
		case PROPTYPE_OPTION:
			// Lots of work to do for this one
			// Save all of the possible options
			for (sOptions = sPropertyIter->second.aszValue.begin (); sOptions != sPropertyIter->second.aszValue.end (); ++sOptions) {
				SaveElement (hFile, nIndent, sPropertyIter->first.c_str (), "option", sOptions->c_str ());
			}
			// Save the currently selection option
			SaveElement (hFile, nIndent, sPropertyIter->first.c_str (), "option-set", sPropertyIter->second.szValue);
			break;
		}
	}

	nIndent--;
	fprintf (hFile, "%*s</Event>\n", nIndent * 2, " ");
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventPropertySetNode::Load (char const * szProperty, char const * szType, char const * szValue) {
	if (_stricmp (szProperty, "nodeid") == 0) {
		nNodeID = atoi (szValue);
	}
	else if (_stricmp (szType, "bool") == 0) {
		PropertySetBool (szProperty, (atoi (szValue) == 1));
	}
	else if (_stricmp (szType, "float") == 0) {
		PropertySetFloat (szProperty, (float)atof (szValue));
	}
	else if (_stricmp (szType, "int") == 0) {
		PropertySetInt (szProperty, atoi (szValue));
	}
	else if (_stricmp (szType, "string") == 0) {
		PropertySetString (szProperty, szValue);
	}
	else if (_stricmp (szType, "option") == 0) {
		PropertySetOptionAdd (szProperty, szValue);
	}
	else if (_stricmp (szType, "option-set") == 0) {
		PropertySetOption (szProperty, szValue);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
EventPropertySetClearNode::EventPropertySetClearNode () {
	nNodeID = 0;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
EventPropertySetClearNode::~EventPropertySetClearNode () {
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool EventPropertySetClearNode::Callback (unsigned int uHandle, float fCompleted) {
	psWindow->pTopology->DecodeNodeNumber (nNodeID)->PropertyRemoveAll ();
	return true;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventPropertySetClearNode::Save (FILE * hFile, int nIndent) {
	fprintf (hFile, "%*s<Event type=\"PropertySetClearNode\">\n", nIndent * 2, " ");
	nIndent++;
	// Save out the node ID
	SaveElement (hFile, nIndent, "nodeid", nNodeID);
	nIndent--;
	fprintf (hFile, "%*s</Event>\n", nIndent * 2, " ");
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventPropertySetClearNode::Load (char const * szProperty, char const * szType, char const * szValue) {
	if (_stricmp (szProperty, "nodeid") == 0) {
		nNodeID = atoi (szValue);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
EventPropertySetLink::EventPropertySetLink () {
	nLinkID = 0;
	PropertyRemoveAll ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
EventPropertySetLink::~EventPropertySetLink () {
	PropertyRemoveAll ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool EventPropertySetLink::Callback (unsigned int uHandle, float fCompleted) {
	psWindow->pTopology->DecodeLinkNumber (nLinkID)->PropertyCopy (this);
	return true;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventPropertySetLink::Save (FILE * hFile, int nIndent) {
	PropertyMap::const_iterator sPropertyIter;
	PropertyMap const * pcPropertyMap;
	list<string>::const_iterator sOptions;

	fprintf (hFile, "%*s<Event type=\"PropertySetLink\">\n", nIndent * 2, " ");
	nIndent++;
	// Save out the node ID
	SaveElement (hFile, nIndent, "linkid", nLinkID);

	// Save out each of the individual properties
	pcPropertyMap = GetPropertyMap ();
	for (sPropertyIter = pcPropertyMap->begin (); sPropertyIter != pcPropertyMap->end (); ++sPropertyIter) {
		switch (sPropertyIter->second.eType) {
		case PROPTYPE_BOOL:
			SaveElement (hFile, nIndent, sPropertyIter->first.c_str (), sPropertyIter->second.boValue);
			break;
		case PROPTYPE_FLOAT:
			SaveElement (hFile, nIndent, sPropertyIter->first.c_str (), sPropertyIter->second.fValue);
			break;
		case PROPTYPE_INT:
			SaveElement (hFile, nIndent, sPropertyIter->first.c_str (), sPropertyIter->second.nValue);
			break;
		case PROPTYPE_STRING:
			SaveElement (hFile, nIndent, sPropertyIter->first.c_str (), sPropertyIter->second.szValue);
			break;
		case PROPTYPE_OPTION:
			// Lots of work to do for this one
			// Save all of the possible options
			for (sOptions = sPropertyIter->second.aszValue.begin (); sOptions != sPropertyIter->second.aszValue.end (); ++sOptions) {
				SaveElement (hFile, nIndent, sPropertyIter->first.c_str (), "option", sOptions->c_str ());
			}
			// Save the currently selection option
			SaveElement (hFile, nIndent, sPropertyIter->first.c_str (), "option-set", sPropertyIter->second.szValue);
			break;
		}
	}

	nIndent--;
	fprintf (hFile, "%*s</Event>\n", nIndent * 2, " ");
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventPropertySetLink::Load (char const * szProperty, char const * szType, char const * szValue) {
	if (_stricmp (szProperty, "linkid") == 0) {
		nLinkID = atoi (szValue);
	}
	else if (_stricmp (szType, "bool") == 0) {
		PropertySetBool (szProperty, (atoi (szValue) == 1));
	}
	else if (_stricmp (szType, "float") == 0) {
		PropertySetFloat (szProperty, (float)atof (szValue));
	}
	else if (_stricmp (szType, "int") == 0) {
		PropertySetInt (szProperty, atoi (szValue));
	}
	else if (_stricmp (szType, "string") == 0) {
		PropertySetString (szProperty, szValue);
	}
	else if (_stricmp (szType, "option") == 0) {
		PropertySetOptionAdd (szProperty, szValue);
	}
	else if (_stricmp (szType, "option-set") == 0) {
		PropertySetOption (szProperty, szValue);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
EventPropertySetClearLink::EventPropertySetClearLink () {
	nLinkID = 0;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
EventPropertySetClearLink::~EventPropertySetClearLink () {
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool EventPropertySetClearLink::Callback (unsigned int uHandle, float fCompleted) {
	psWindow->pTopology->DecodeLinkNumber (nLinkID)->PropertyRemoveAll ();
	return true;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventPropertySetClearLink::Save (FILE * hFile, int nIndent) {
	fprintf (hFile, "%*s<Event type=\"PropertySetClearLink\">\n", nIndent * 2, " ");
	nIndent++;
	// Save out the link ID
	SaveElement (hFile, nIndent, "linkid", nLinkID);
	nIndent--;
	fprintf (hFile, "%*s</Event>\n", nIndent * 2, " ");
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventPropertySetClearLink::Load (char const * szProperty, char const * szType, char const * szValue) {
	if (_stricmp (szProperty, "linkid") == 0) {
		nLinkID = atoi (szValue);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool EventAssignData::Callback (unsigned int uHandle, float fCompleted) {
	cTopologyNode*	psNode;
	cTopologyList*	pList;
	cTopologyNode*	psIterator;

	if (fCompleted <= 0.0f) {

		/* Find the correct node */
		psNode = NULL;
		pList = psWindow->pTopology->GetpList ();
		if (pList) {
			psIterator = pList->startIterator ();
			while ((pList->isIteratorValid (psIterator)) && (pList->getCurrentID (psIterator) != nNodeID)) {
				psIterator = pList->moveForward (psIterator);
			}
		}

		if (pList->isIteratorValid (psIterator)) {
			psNode = pList->getCurrentNode (psIterator);
			strcat (psNode->szData, szDataName);
			strcat (psNode->szData, ";");
		}
	}

	return TRUE;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
EventAssignData::EventAssignData () {
	szDataName = NULL;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
EventAssignData::~EventAssignData () {
	if (szDataName) {
		free ((void*) szDataName);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventAssignData::Save (FILE * hFile, int nIndent) {
	fprintf (hFile, "%*s<Event type=\"AssignData\">\n", nIndent * 2, " ");
	nIndent++;
	SaveElement (hFile, nIndent, "nodeid", nNodeID);
	SaveElement (hFile, nIndent, "dataname", szDataName);
	nIndent--;
	fprintf (hFile, "%*s</Event>\n", nIndent * 2, " ");
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventAssignData::Load (char const * szProperty, char const * szType, char const * szValue) {
	if (_stricmp (szProperty, "nodeid") == 0) {
		nNodeID = atoi (szValue);
	}
	else if (_stricmp (szProperty, "dataname") == 0) {
		szDataName = (char *)malloc (strlen (szValue) + 1);
		strcpy (szDataName, szValue);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool EventRemoveData::Callback (unsigned int uHandle, float fCompleted) {
	cTopologyNode*	psNode;
	cTopologyList*	pList;
	char*  szTemp;
	cTopologyNode*	psIterator;

	if (fCompleted <= 0.0f) {

		/* Find the correct node */
		psNode = NULL;
		pList = psWindow->pTopology->GetpList ();
		if (pList) {
			psIterator = pList->startIterator ();
			while ((pList->isIteratorValid (psIterator)) && (pList->getCurrentID (psIterator) != nNodeID)) {
				psIterator = pList->moveForward (psIterator);
			}
		}

		if (pList->isIteratorValid (psIterator)) {
			psNode = pList->getCurrentNode (psIterator);

			szTemp = strtok (psNode->szData, ";");
			psNode->szData[0] = 0;

			while (szTemp != NULL) {
				if (strcmp (szTemp, szDataName) != 0) {
					strcat (psNode->szData, szTemp);
					strcat (psNode->szData, ";");
				}

				szTemp = strtok (NULL, ";");
			}
		}
	}

	return TRUE;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
EventRemoveData::EventRemoveData () {
	szDataName = NULL;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
EventRemoveData::~EventRemoveData () {
	if (szDataName) {
		free ((void*) szDataName);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventRemoveData::Save (FILE * hFile, int nIndent) {
	fprintf (hFile, "%*s<Event type=\"RemoveData\">\n", nIndent * 2, " ");
	nIndent++;
	SaveElement (hFile, nIndent, "nodeid", nNodeID);
	SaveElement (hFile, nIndent, "dataname", szDataName);
	nIndent--;
	fprintf (hFile, "%*s</Event>\n", nIndent * 2, " ");
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventRemoveData::Load (char const * szProperty, char const * szType, char const * szValue) {
	if (_stricmp (szProperty, "nodeid") == 0) {
		nNodeID = atoi (szValue);
	}
	else if (_stricmp (szProperty, "dataname") == 0) {
		szDataName = (char *)malloc (strlen (szValue) + 1);
		strcpy (szDataName, szValue);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool EventSendData::Callback (unsigned int uHandle, float fCompleted) {
	cTopologyNode*	psNode;
	cTopologyList*	pList;
	cTopologyNode*	psIterator;

	if (fCompleted <= 0.0f) {

		/* Find the correct node */
		psNode = NULL;
		pList = psWindow->pTopology->GetpList ();
		if (pList) {
			psIterator = pList->startIterator ();
			while ((pList->isIteratorValid (psIterator)) && (pList->getCurrentID (psIterator) != nTo)) {
				psIterator = pList->moveForward (psIterator);
			}
		}

		if (pList->isIteratorValid (psIterator)) {
			psNode = pList->getCurrentNode (psIterator);
			if (strstr (psNode->szData, szDataName) == NULL) {
				strcat (psNode->szData, szDataName);
				strcat (psNode->szData, ";");
			}
		}
	}

	return TRUE;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
EventSendData::EventSendData () {
	szDataName = NULL;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
EventSendData::~EventSendData () {
	if (szDataName) {
		free ((void*) szDataName);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventSendData::Save (FILE * hFile, int nIndent) {
	fprintf (hFile, "%*s<Event type=\"SendData\">\n", nIndent * 2, " ");
	nIndent++;
	SaveElement (hFile, nIndent, "nodeidfrom", nFrom);
	SaveElement (hFile, nIndent, "nodeidto", nTo);
	SaveElement (hFile, nIndent, "dataname", szDataName);
	nIndent--;
	fprintf (hFile, "%*s</Event>\n", nIndent * 2, " ");
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventSendData::Load (char const * szProperty, char const * szType, char const * szValue) {
	if (_stricmp (szProperty, "nodeidfrom") == 0) {
		nFrom = atoi (szValue);
	}
	else if (_stricmp (szProperty, "nodeidto") == 0) {
		nTo = atoi (szValue);
	}
	else if (_stricmp (szProperty, "dataname") == 0) {
		szDataName = (char *)malloc (strlen (szValue) + 1);
		strcpy (szDataName, szValue);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool EventChangeView::Callback (unsigned int uHandle, float fCompleted) {
	float fTheta;
	float fPhi;
	float fPsi;
	float fRadius;

	/* Calculate the camera offset based on its current position */
	fTheta = this->fTheta;
	fPhi = this->fPhi;
	fPsi = this->fPsi;
	fRadius = this->fRadius;
	psWindow->pTopology->ViewOffset (&fTheta, &fPhi, &fPsi, &fRadius);

	/* Apply the half life */
	fTheta *= fCompleted;
	fPhi *= fCompleted;
	fPsi *= fCompleted;
	fRadius *= fCompleted;

	/* Rotate the camera to its new position */
	psWindow->pTopology->ChangeView (fTheta, fPhi, fPsi, fRadius);

	return TRUE;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventChangeView::Save (FILE * hFile, int nIndent) {
	fprintf (hFile, "%*s<Event type=\"ChangeView\">\n", nIndent * 2, " ");
	nIndent++;
	SaveElement (hFile, nIndent, "theta", fTheta);
	SaveElement (hFile, nIndent, "phi", fPhi);
	SaveElement (hFile, nIndent, "psi", fPsi);
	SaveElement (hFile, nIndent, "radius", fRadius);
	nIndent--;
	fprintf (hFile, "%*s</Event>\n", nIndent * 2, " ");
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventChangeView::Load (char const * szProperty, char const * szType, char const * szValue) {
	if (_stricmp (szProperty, "theta") == 0) {
		fTheta = (float)atof (szValue);
	}
	else if (_stricmp (szProperty, "phi") == 0) {
		fPhi = (float)atof (szValue);
	}
	else if (_stricmp (szProperty, "psi") == 0) {
		fPsi = (float)atof (szValue);
	}
	else if (_stricmp (szProperty, "radius") == 0) {
		fRadius = (float)atof (szValue);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool EventTranslateView::Callback (unsigned int uHandle, float fCompleted) {
	float fX;
	float fY;
	float fZ;
	float fXAt;
	float fYAt;
	float fZAt;

	/* Calculate the camera offset based on its current position */
	psWindow->pTopology->GetCentre (&fXAt, &fYAt, &fZAt);

	fX = fXAt - this->fX;
	fY = fYAt - this->fY;
	fZ = fZAt - this->fZ;

	/* Apply the half life */
	fX *= fCompleted;
	fY *= fCompleted;
	fZ *= fCompleted;

	/* Rotate the camera to its new position */
	psWindow->pTopology->SetCentre (fXAt - fX, fYAt - fY, fZAt - fZ);

	return TRUE;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventTranslateView::Save (FILE * hFile, int nIndent) {
	fprintf (hFile, "%*s<Event type=\"TranslateView\">\n", nIndent * 2, " ");
	nIndent++;
	SaveElement (hFile, nIndent, "x", fX);
	SaveElement (hFile, nIndent, "y", fY);
	SaveElement (hFile, nIndent, "z", fZ);
	nIndent--;
	fprintf (hFile, "%*s</Event>\n", nIndent * 2, " ");
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventTranslateView::Load (char const * szProperty, char const * szType, char const * szValue) {
	if (_stricmp (szProperty, "x") == 0) {
		fX = (float)atof (szValue);
	}
	else if (_stricmp (szProperty, "y") == 0) {
		fY = (float)atof (szValue);
	}
	else if (_stricmp (szProperty, "z") == 0) {
		fZ = (float)atof (szValue);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool EventViewProperties::Callback (unsigned int uHandle, float fCompleted) {
	if (fCompleted <= 0.0f) {
		/* Set the various view properties */
		psWindow->SetOverlayStatus (boOverlay);
		psWindow->SetSpinStatus (boSpin);
		psWindow->SetNetworkViewStatus (boNetworkView);
	}

	return TRUE;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventViewProperties::Save (FILE * hFile, int nIndent) {
	fprintf (hFile, "%*s<Event type=\"ViewProperties\">\n", nIndent * 2, " ");
	nIndent++;
	SaveElement (hFile, nIndent, "spin", boSpin);
	SaveElement (hFile, nIndent, "networkview", boNetworkView);
	SaveElement (hFile, nIndent, "overlay", boOverlay);
	nIndent--;
	fprintf (hFile, "%*s</Event>\n", nIndent * 2, " ");
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void EventViewProperties::Load (char const * szProperty, char const * szType, char const * szValue) {
	if (_stricmp (szProperty, "spin") == 0) {
		boSpin = (atoi (szValue) != 0);
	}
	else if (_stricmp (szProperty, "networkview") == 0) {
		boNetworkView = (atoi (szValue) != 0);
	}
	else if (_stricmp (szProperty, "overlay") == 0) {
		boOverlay = (atoi (szValue) != 0);
	}
}

