/*$T MATTS/cSaveXML.cpp GC 1.140 07/01/09 21:12:09 */
/*
 * Name: cSaveXML ;
 * Purpose:save the curretn scenarion including nodes and links into an xml fie
 * for future usage. ;
 */
#include "cSaveXML.h"
#include "cTopology.h"
#include "cTimer.h"
#include "Event.h"

XERCES_CPP_NAMESPACE_USE

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */

cSaveXML::cSaveXML (cWindow * psWindow) {
	this->psWindow = psWindow;
	boCreateNodes = true;
	nDefaultNodeID = 0;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cSaveXML::~cSaveXML () {
}

/*
 =======================================================================================================================
 *  Change the default behaviour for specific circumstances
 =======================================================================================================================
 */
void cSaveXML::ChangeDefaults (bool boCreateNodes, int nDefaultNodeID) {
	this->boCreateNodes = boCreateNodes;
	this->nDefaultNodeID = nDefaultNodeID;
}

/*
 =======================================================================================================================
 *  Save node properties to the XML file
 =======================================================================================================================
 */
void cSaveXML::SaveNode (FILE * hFile, int nIndent, cTopologyNode * psNode) {
	fprintf (hFile, "%*s<Node>\n", nIndent * 2, " ");
	nIndent++;
	fprintf (hFile, "%*s<Id type=\"int\">%d</Id>\n", nIndent * 2, " ", psNode->getID ());
	fprintf (hFile, "%*s<Position X=\"%f\" Y=\"%f\" Z=\"%f\" />\n", nIndent * 2, " ", psNode->fXPos, psNode->fYPos, psNode->fZPos);
	fprintf (hFile, "%*s<Name type=\"string\">%s</Name>\n", nIndent * 2, " ", psNode->GetName ());
	fprintf (hFile, "%*s<Type type=\"enum\">%d</Type>\n", nIndent * 2, " ", psNode->nType);
	fprintf (hFile, "%*s<SensitivityLevel type=\"int\">%d</SensitivityLevel>\n", nIndent * 2, " ", psNode->nSensitivityLevel);
	fprintf (hFile, "%*s<EncryptionAlgorithm type=\"string\">%s</EncryptionAlgorithm>\n", nIndent * 2, " ", psNode->szEncryptAlgorithm);
	fprintf (hFile, "%*s<StaffSkills type=\"string\">%s</StaffSkills>\n", nIndent * 2, " ", psNode->szStaffSkills);
	fprintf (hFile, "%*s<Firewall type=\"bool\">%d</Firewall>\n", nIndent * 2, " ", psNode->boFirewall);
	fprintf (hFile, "%*s<IDS type=\"bool\">%d</IDS>\n", nIndent * 2, " ", psNode->boIDS);
	fprintf (hFile, "%*s<SecurityMax type=\"int\">%d</SecurityMax>\n", nIndent * 2, " ", psNode->nSecurityMax);
	fprintf (hFile, "%*s<SecurityMin type=\"int\">%d</SecurityMin>\n", nIndent * 2, " ", psNode->nSecurityMin);
	fprintf (hFile, "%*s<ExtraProperty type=\"string\">%s</ExtraProperty>\n", nIndent * 2, " ", psNode->szCertProps);
	fprintf (hFile, "%*s<External type=\"bool\">%d</External>\n", nIndent * 2, " ", psNode->boExternal);
	fprintf (hFile, "%*s<AdHoc type=\"bool\">%d</AdHoc>\n", nIndent * 2, " ", psNode->boAdHoc);
	fprintf (hFile, "%*s<RangeTx type=\"float\">%f</RangeTx>\n", nIndent * 2, " ", psNode->fRangeTx);
	fprintf (hFile, "%*s<RangeRx type=\"float\">%f</RangeRx>\n", nIndent * 2, " ", psNode->fRangeRx);
	nIndent--;
	fprintf (hFile, "%*s</Node>\n", nIndent * 2, " ");
}

/*
 =======================================================================================================================
 *  Save link properties to the XML file
 =======================================================================================================================
 */
void cSaveXML::SaveLink (FILE * hFile, int nIndent, cTopologyLink * psLink) {
	fprintf (hFile, "%*s<Link>\n", nIndent * 2, " ");
	nIndent++;
	fprintf (hFile, "%*s<OutId type=\"int\">%d</OutId>\n", nIndent * 2, " ", psLink->psLinkFrom->getID ());
	fprintf (hFile, "%*s<InId type=\"int\">%d</InId>\n", nIndent * 2, " ", psLink->psLinkTo->getID ());
	fprintf (hFile, "%*s<LinkId type=\"int\">%d</LinkId>\n", nIndent * 2, " ", psLink->nLinkID);
	fprintf (hFile, "%*s<ChannelOut type=\"int\">%d</ChannelOut>\n", nIndent * 2, " ", psLink->nChannelOut);
	fprintf (hFile, "%*s<ChannelIn type=\"int\">%d</ChannelIn>\n", nIndent * 2, " ", psLink->nChannelIn);
	fprintf (hFile, "%*s<OutProperty type=\"string\">%s</OutProperty>\n", nIndent * 2, " ", psLink->szLinkOutput);
	fprintf (hFile, "%*s<InProperty type=\"string\">%s</InProperty>\n", nIndent * 2, " ", psLink->szLinkInput);
	fprintf (hFile, "%*s<SecurityLevel type=\"int\">%d</SecurityLevel>\n", nIndent * 2, " ", psLink->nSecurity);
	fprintf (hFile, "%*s<Sensitivity type=\"int\">%d</Sensitivity>\n", nIndent * 2, " ", psLink->nSensitivityLevel);
	nIndent--;
	fprintf (hFile, "%*s</Link>\n", nIndent * 2, " ");
}

/*
 =======================================================================================================================
 *  Save the elements to the XML file
 =======================================================================================================================
 */
void cSaveXML::Save (FILE * hFile, int nIndent) {
	cTopologyList * pList;
	cTopologyNode * psIterator;
	cTopologyLink * pLinks;
	float fTheta;
	float fPhi;
	float fPsi;
	float fRadius;
	float fX;
	float fY;
	float fZ;

	fprintf (hFile, "%*s<Topology>\n", nIndent * 2, " ");
	nIndent++;

	// Save out the camera position
	psWindow->pTopology->GetView (& fTheta, & fPhi, & fPsi, & fRadius);
	fprintf (hFile, "%*s<Camera theta=\"%f\" phi=\"%f\" psi=\"%f\" radius=\"%f\"/>\n", nIndent * 2, " ", fTheta, fPhi, fPsi, fRadius);

	// Save out the centre position
	psWindow->pTopology->GetCentre (& fX, & fY, & fZ);
	fprintf (hFile, "%*s<Centre X=\"%f\" Y=\"%f\" Z=\"%f\"/>\n", nIndent * 2, " ", fX, fY, fZ);

	// Save out the nodes
	pList = psWindow->pTopology->GetpList ();

	psIterator = pList->startIterator ();
	while (pList->isIteratorValid (psIterator)) {
		SaveNode (hFile, nIndent, psIterator);
		psIterator = pList->moveForward (psIterator);
	}

	// Save out the links
	psIterator = pList->startIterator ();
	while (pList->isIteratorValid (psIterator)) {
		pLinks = pList->getLinksOut (psIterator);
		while (pLinks) {
			SaveLink (hFile, nIndent, pLinks);
			pLinks = pLinks->pFromNext;
		}
		psIterator = pList->moveForward (psIterator);
	}

	nIndent--;
	fprintf (hFile, "%*s</Topology>\n", nIndent * 2, " ");
}

/*
 =======================================================================================================================
 *  Start the routines that load the elements from the XML file
 =======================================================================================================================
 */
void cSaveXML::LoadStart (char const * szChunkName) {
	sLoadText = "";
	eObject = LOADSAVETAG_INVALID;
	eProperty = LOADSAVETAG_INVALID;
	psNewNode = NULL;
	psNewLink = NULL;
	psPropertiesNode = NULL;
	psPropertiesLink = NULL;
}

/*
 =======================================================================================================================
 *  End the routines that load the elements from the XML file
 =======================================================================================================================
 */
void cSaveXML::LoadEnd (char const * szChunkName) {
	sLoadText = "";
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cSaveXML::startElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname, const Attributes&	attrs) {
	char * szElementName;
  unsigned int uAttribute;
	char * szAttributeName;
	char * szAttributeValue;
	float fXPos = 0.0f;
	float fYPos = 0.0f;
	float fZPos = 0.0f;
	float fTheta = 0.0f;
	float fPhi = 0.0f;
	float fPsi = 0.0f;
	float fRadius = 0.0f;

	szElementName = XMLString::transcode (localname);

	// Check the attributes
	for (uAttribute = 0; uAttribute < attrs.getLength(); uAttribute++) {
		szAttributeName = XMLString::transcode (attrs.getLocalName (uAttribute));
		szAttributeValue = XMLString::transcode (attrs.getValue (uAttribute));

		if (_stricmp (szAttributeName, "X") == 0) {
			fXPos = (float)atof (szAttributeValue);
		}
		else if (_stricmp (szAttributeName, "Y") == 0) {
			fYPos = (float)atof (szAttributeValue);
		}
		else if (_stricmp (szAttributeName, "Z") == 0) {
			fZPos = (float)atof (szAttributeValue);
		}
		else if (_stricmp (szAttributeName, "theta") == 0) {
			fTheta = (float)atof (szAttributeValue);
		}
		else if (_stricmp (szAttributeName, "phi") == 0) {
			fPhi = (float)atof (szAttributeValue);
		}
		else if (_stricmp (szAttributeName, "psi") == 0) {
			fPsi = (float)atof (szAttributeValue);
		}
		else if (_stricmp (szAttributeName, "radius") == 0) {
			fRadius = (float)atof (szAttributeValue);
		}

		XMLString::release (& szAttributeName);
		XMLString::release (& szAttributeValue);
	}

	// Check the element name
	if (eObject == LOADSAVETAG_INVALID) {
		if (_stricmp (szElementName, "Topology") == 0) {
			eObject = LOADSAVETAG_TOPOLOGY;
		}
	}
	else if (eObject == LOADSAVETAG_TOPOLOGY) {
		if (_stricmp (szElementName, "Node") == 0) {
			eObject = LOADSAVETAG_NODE;
			psNewNode = new EventNewNode ();
			psNewNode->psWindow = psWindow;
			psNewNode->nNodeID = nDefaultNodeID;
			psPropertiesNode = new EventPropertiesNode ();
			psPropertiesNode->psWindow = psWindow;
			psPropertiesNode->nNodeID = nDefaultNodeID;
			psPropertiesNode->fRangeTx = 0.0f;
			psPropertiesNode->fRangeRx = 0.0f;
		}
		else if (_stricmp (szElementName, "Link") == 0) {
			eObject = LOADSAVETAG_LINK;
			psNewLink = new EventNewLink ();
			psNewLink->psWindow = psWindow;
			psPropertiesLink = new EventPropertiesLink ();
			psPropertiesLink->psWindow = psWindow;
		}
		else if (_stricmp (szElementName, "Camera") == 0) {
			eObject = LOADSAVETAG_CAMERA;
			psWindow->pTopology->ViewOffset (&fTheta, &fPhi, &fPsi, &fRadius);
			psWindow->pTopology->ChangeView (fTheta, fPhi, fPsi, fRadius);
		}
		else if (_stricmp (szElementName, "Centre") == 0) {
			eObject = LOADSAVETAG_CENTRE;
			psWindow->pTopology->SetCentre (fXPos, fYPos, fZPos);
		}
	}
	else if (eObject == LOADSAVETAG_NODE) {
		if (_stricmp (szElementName, "Id") == 0) {
			eProperty = LOADSAVETAG_ID;
		}
		else if (_stricmp (szElementName, "Position") == 0) {
			eProperty = LOADSAVETAG_POSITION;
			psNewNode->fXPos = fXPos;
			psNewNode->fYPos = fYPos;
			psNewNode->fZPos = fZPos;
		}
		else if (_stricmp (szElementName, "Name") == 0) {
			eProperty = LOADSAVETAG_NAME;
		}
		else if (_stricmp (szElementName, "Type") == 0) {
			eProperty = LOADSAVETAG_TYPE;
		}
		else if (_stricmp (szElementName, "SensitivityLevel") == 0) {
			eProperty = LOADSAVETAG_SENSITIVITY;
		}
		else if (_stricmp (szElementName, "EncryptionAlgorithm") == 0) {
			eProperty = LOADSAVETAG_ENCRYPTION;
		}
		else if (_stricmp (szElementName, "StaffSkills") == 0) {
			eProperty = LOADSAVETAG_STAFFSKILLS;
		}
		else if (_stricmp (szElementName, "Firewall") == 0) {
			eProperty = LOADSAVETAG_FIREWALL;
		}
		else if (_stricmp (szElementName, "IDS") == 0) {
			eProperty = LOADSAVETAG_IDS;
		}
		else if (_stricmp (szElementName, "SecurityMax") == 0) {
			eProperty = LOADSAVETAG_SECMAX;
		}
		else if (_stricmp (szElementName, "SecurityMin") == 0) {
			eProperty = LOADSAVETAG_SECMIN;
		}
		else if (_stricmp (szElementName, "ExtraProperty") == 0) {
			eProperty = LOADSAVETAG_PROPERTIES;
		}
		else if (_stricmp (szElementName, "External") == 0) {
			eProperty = LOADSAVETAG_EXTERNAL;
		}
		else if (_stricmp (szElementName, "AdHoc") == 0) {
			eProperty = LOADSAVETAG_ADHOC;
		}
		else if (_stricmp (szElementName, "RangeTx") == 0) {
			eProperty = LOADSAVETAG_RANGETX;
		}
		else if (_stricmp (szElementName, "RangeRx") == 0) {
			eProperty = LOADSAVETAG_RANGERX;
		}
	}
	else if (eObject == LOADSAVETAG_LINK) {
		if (_stricmp (szElementName, "OutId") == 0) {
			eProperty = LOADSAVETAG_OUTID;
		}
		else if (_stricmp (szElementName, "InId") == 0) {
			eProperty = LOADSAVETAG_INID;
		}
		else if (_stricmp (szElementName, "LinkId") == 0) {
			eProperty = LOADSAVETAG_LINKID;
		}
		else if (_stricmp (szElementName, "ChannelOut") == 0) {
			eProperty = LOADSAVETAG_CHANNELOUT;
		}
		else if (_stricmp (szElementName, "ChannelIn") == 0) {
			eProperty = LOADSAVETAG_CHANNELIN;
		}
		else if (_stricmp (szElementName, "OutProperty") == 0) {
			eProperty = LOADSAVETAG_OUTPROPERTY;
		}
		else if (_stricmp (szElementName, "InProperty") == 0) {
			eProperty = LOADSAVETAG_INPROPERTY;
		}
		else if (_stricmp (szElementName, "SecurityLevel") == 0) {
			eProperty = LOADSAVETAG_SECURITY;
		}
		else if (_stricmp (szElementName, "Sensitivity") == 0) {
			eProperty = LOADSAVETAG_SENSITIVITY;
		}
	}

	sLoadText = "";

	XMLString::release (& szElementName);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cSaveXML::characters (const XMLCh *const chars, const unsigned int length) {
	char*  szMessage;

	szMessage = XMLString::transcode (chars);
	sLoadText.append (szMessage);
	XMLString::release (& szMessage);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cSaveXML::endElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname) {
	char * szElementName;
	unsigned int uEventHandle;
	cTimer * psTimer;

	szElementName = XMLString::transcode (localname);

	switch (eObject) {
		case LOADSAVETAG_TOPOLOGY:
			if (_stricmp (szElementName, "Topology") == 0) {
				eObject = LOADSAVETAG_INVALID;
			}
			break;
		case LOADSAVETAG_NODE:
			if (_stricmp (szElementName, "Node") == 0) {
				psTimer = psWindow->pTopology->GetTimer ();
				if (boCreateNodes) {
					// Create the node
					uEventHandle = psTimer->AddEvent ("Load node", TIMER_TIME_NOW, 0, psNewNode);
					// Clean up the event
					psTimer->RemoveEvent (uEventHandle);
					psNewNode = NULL;
					// Set the node properties
					uEventHandle = psTimer->AddEvent ("Load node properties", TIMER_TIME_NOW, 0, psPropertiesNode);
					// Clean up the event
					psTimer->RemoveEvent (uEventHandle);
					psPropertiesNode = NULL;
				}
				else {
					// We don't need to create the node
					delete psNewNode;
					psNewNode = NULL;
					// Set the node properties
					uEventHandle = psTimer->AddEvent ("Load node properties", TIMER_TIME_NEXT, 0, psPropertiesNode);
					psPropertiesNode = NULL;
				}
				eObject = LOADSAVETAG_TOPOLOGY;
			}
			else {
				switch (eProperty) {
				case LOADSAVETAG_ID:
					if (_stricmp (szElementName, "Id") == 0) {
						// If we're just setting the properties we should ignore the node ID
						// TODO: Remove the ID from the data sent by the client and remove the condition
						if (boCreateNodes) {
							psNewNode->nNodeID = atoi (sLoadText.c_str ());
							psPropertiesNode->nNodeID = atoi (sLoadText.c_str ());
						}
						eProperty = LOADSAVETAG_INVALID;
					}
					break;
				case LOADSAVETAG_POSITION:
					if (_stricmp (szElementName, "Position") == 0) {
						eProperty = LOADSAVETAG_INVALID;
					}
					break;
				case LOADSAVETAG_NAME:
					if (_stricmp (szElementName, "Name") == 0) {
						psPropertiesNode->szName = (char *)malloc (strlen (sLoadText.c_str ()) + 1);
						strcpy (psPropertiesNode->szName, sLoadText.c_str ());
						eProperty = LOADSAVETAG_INVALID;
					}
					break;
				case LOADSAVETAG_TYPE:
					if (_stricmp (szElementName, "Type") == 0) {
						psNewNode->nType = atoi (sLoadText.c_str ());
						eProperty = LOADSAVETAG_INVALID;
					}
					break;
				case LOADSAVETAG_SENSITIVITY:
					if (_stricmp (szElementName, "SensitivityLevel") == 0) {
						psPropertiesNode->nSensitivityLevel = atoi (sLoadText.c_str ());
						eProperty = LOADSAVETAG_INVALID;
					}
					break;
				case LOADSAVETAG_ENCRYPTION:
					if (_stricmp (szElementName, "EncryptionAlgorithm") == 0) {
						psPropertiesNode->szEncryptAlgorithm = (char *)malloc (strlen (sLoadText.c_str ()) + 1);
						strcpy (psPropertiesNode->szEncryptAlgorithm, sLoadText.c_str ());
						eProperty = LOADSAVETAG_INVALID;
					}
					break;
				case LOADSAVETAG_STAFFSKILLS:
					if (_stricmp (szElementName, "StaffSkills") == 0) {
						psPropertiesNode->szStaffSkills = (char *)malloc (strlen (sLoadText.c_str ()) + 1);
						strcpy (psPropertiesNode->szStaffSkills, sLoadText.c_str ());
						eProperty = LOADSAVETAG_INVALID;
					}
					break;
				case LOADSAVETAG_FIREWALL:
					if (_stricmp (szElementName, "Firewall") == 0) {
						psPropertiesNode->boFirewall = (atoi (sLoadText.c_str ()) != 0);
						eProperty = LOADSAVETAG_INVALID;
					}
					break;
				case LOADSAVETAG_IDS:
					if (_stricmp (szElementName, "IDS") == 0) {
						psPropertiesNode->boIDS = (atoi (sLoadText.c_str ()) != 0);
						eProperty = LOADSAVETAG_INVALID;
					}
					break;
				case LOADSAVETAG_SECMAX:
					if (_stricmp (szElementName, "SecurityMax") == 0) {
						psPropertiesNode->nSecurityMax = atoi (sLoadText.c_str ());
						eProperty = LOADSAVETAG_INVALID;
					}
					break;
				case LOADSAVETAG_SECMIN:
					if (_stricmp (szElementName, "SecurityMin") == 0) {
						psPropertiesNode->nSecurityMin = atoi (sLoadText.c_str ());
						eProperty = LOADSAVETAG_INVALID;
					}
					break;
				case LOADSAVETAG_PROPERTIES:
					if (_stricmp (szElementName, "ExtraProperty") == 0) {
						psPropertiesNode->szCertProps = (char *)malloc (strlen (sLoadText.c_str ()) + 1);
						strcpy (psPropertiesNode->szCertProps, sLoadText.c_str ());
						eProperty = LOADSAVETAG_INVALID;
					}
					break;
				case LOADSAVETAG_EXTERNAL:
					if (_stricmp (szElementName, "External") == 0) {
						psPropertiesNode->boExternal = (atoi (sLoadText.c_str ()) != 0);
						eProperty = LOADSAVETAG_INVALID;
					}
					break;
				case LOADSAVETAG_ADHOC:
					if (_stricmp (szElementName, "AdHoc") == 0) {
						psPropertiesNode->boAdHoc = (atoi (sLoadText.c_str ()) != 0);
						eProperty = LOADSAVETAG_INVALID;
					}
					break;
				case LOADSAVETAG_RANGERX:
					if (_stricmp (szElementName, "RangeRx") == 0) {
						psPropertiesNode->fRangeRx = (float)atof (sLoadText.c_str ());
						eProperty = LOADSAVETAG_INVALID;
					}
					break;
				case LOADSAVETAG_RANGETX:
					if (_stricmp (szElementName, "RangeTx") == 0) {
						psPropertiesNode->fRangeTx = (float)atof (sLoadText.c_str ());
						eProperty = LOADSAVETAG_INVALID;
					}
					break;
				}
			}
			break;
		case LOADSAVETAG_LINK:
			if (_stricmp (szElementName, "Link") == 0) {
				psTimer = psWindow->pTopology->GetTimer ();
				if (boCreateNodes) {
					// Create the node
					uEventHandle = psTimer->AddEvent ("Load link", TIMER_TIME_NOW, 0, psNewLink);
					// Clean up the event
					psTimer->RemoveEvent (uEventHandle);
					psNewLink = NULL;
					// Set the node properties
					uEventHandle = psTimer->AddEvent ("Load link properties", TIMER_TIME_NOW, 0, psPropertiesLink);
					// Clean up the event
					psTimer->RemoveEvent (uEventHandle);
					psPropertiesLink = NULL;
				}
				else {
					// We don't need to create the link
					delete psNewLink;
					psNewLink = NULL;
					// Set the node properties
					uEventHandle = psTimer->AddEvent ("Load link properties", TIMER_TIME_NEXT, 0, psPropertiesLink);
					psPropertiesLink = NULL;
				}
				eObject = LOADSAVETAG_TOPOLOGY;
			}
			else {
				switch (eProperty) {
				case LOADSAVETAG_OUTID:
					if (_stricmp (szElementName, "OutId") == 0) {
						psNewLink->nFrom = atoi (sLoadText.c_str ());
						psPropertiesLink->nNodeFromID = atoi (sLoadText.c_str ());
						eProperty = LOADSAVETAG_INVALID;
					}
					break;
				case LOADSAVETAG_INID:
					if (_stricmp (szElementName, "InId") == 0) {
						psNewLink->nTo = atoi (sLoadText.c_str ());
						eProperty = LOADSAVETAG_INVALID;
					}
					break;
				case LOADSAVETAG_LINKID:
					if (_stricmp (szElementName, "LinkId") == 0) {
						psNewLink->nLinkID = atoi (sLoadText.c_str ());
						psPropertiesLink->nLinkID = atoi (sLoadText.c_str ());
						eProperty = LOADSAVETAG_INVALID;
					}
					break;
				case LOADSAVETAG_CHANNELOUT:
					if (_stricmp (szElementName, "ChannelOut") == 0) {
						psPropertiesLink->nChannelOut = atoi (sLoadText.c_str ());
						eProperty = LOADSAVETAG_INVALID;
					}
					break;
				case LOADSAVETAG_CHANNELIN:
					if (_stricmp (szElementName, "ChannelIn") == 0) {
						psPropertiesLink->nChannelIn = atoi (sLoadText.c_str ());
						eProperty = LOADSAVETAG_INVALID;
					}
					break;
				case LOADSAVETAG_OUTPROPERTY:
					if (_stricmp (szElementName, "OutProperty") == 0) {
						psPropertiesLink->szLinkOutput = (char *)malloc (strlen (sLoadText.c_str ()) + 1);
						strcpy (psPropertiesLink->szLinkOutput, sLoadText.c_str ());
						eProperty = LOADSAVETAG_INVALID;
					}
					break;
				case LOADSAVETAG_INPROPERTY:
					if (_stricmp (szElementName, "InProperty") == 0) {
						psPropertiesLink->szLinkInput = (char *)malloc (strlen (sLoadText.c_str ()) + 1);
						strcpy (psPropertiesLink->szLinkInput, sLoadText.c_str ());
						eProperty = LOADSAVETAG_INVALID;
					}
					break;
				case LOADSAVETAG_SECURITY:
					if (_stricmp (szElementName, "SecurityLevel") == 0) {
						psPropertiesLink->nSecurity = atoi (sLoadText.c_str ());
						eProperty = LOADSAVETAG_INVALID;
					}
					break;
				case LOADSAVETAG_SENSITIVITY:
					if (_stricmp (szElementName, "Sensitivity") == 0) {
						psPropertiesLink->nSensitivityLevel = atoi (sLoadText.c_str ());
						eProperty = LOADSAVETAG_INVALID;
					}
					break;
				}
			}
			break;
		case LOADSAVETAG_CAMERA:
			if (_stricmp (szElementName, "Camera") == 0) {
				eObject = LOADSAVETAG_TOPOLOGY;
			}
			break;
		case LOADSAVETAG_CENTRE:
			if (_stricmp (szElementName, "Centre") == 0) {
				eObject = LOADSAVETAG_TOPOLOGY;
			}
			break;
		default:
			// Do nothing
			break;
	}

	XMLString::release (& szElementName);
}
