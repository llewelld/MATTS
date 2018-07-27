/*$T DynamicMatts/cXMLPropertySet.cpp GC 1.140 12/27/09 14:10:04 */

#include "cXMLPropertySet.h"
#pragma warning(disable : 4267)
#pragma warning(disable : 4244)
#include <xercesc/sax2/XMLReaderFactory.hpp>
#pragma warning(default : 4267)
#pragma warning(default : 4244)

XERCES_CPP_NAMESPACE_USE using namespace std;

/*
 =======================================================================================================================
 *    Constructor
 =======================================================================================================================
 */
cXMLPropertySet::cXMLPropertySet (cWindow * psWindow) {
	boPropertyCheck = false;
	boOptionWidget = false;
	nControlCount = 0;
	nProperties = 0;
	nControlID = 150;
	pcControls = NULL;
	pcCurrent = NULL;
	this->psWindow = psWindow;
	ePropSetType = XMLPROPSET_INVALID;
	pcNodeLink = NULL;
	szPropertyName.clear ();
	szProperty.clear ();
	szOption.clear ();
	pcDefaultNodeLink = NULL;
}

/*
 =======================================================================================================================
 *    Destructor
 =======================================================================================================================
 */
cXMLPropertySet::~cXMLPropertySet () {
}

cControls * cXMLPropertySet::AddControl () {
	pcCurrent = new cControls (pcCurrent);

	if (pcControls == NULL) {
		pcControls = pcCurrent;
	}

	return pcCurrent;
}

/*
 =======================================================================================================================
 *    Implementation of the SAX DocumentHandler interface
 *    TODO: There are quite a few transcoded strings that look like they're not released here
 =======================================================================================================================
 */
void cXMLPropertySet::startElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname, const Attributes&	 attrs) {
	char*  szElement;
	char*  szAttribute;
	char*  szValue;
	unsigned int uSize;
	unsigned int uAttributes;

	/*
	 * This will get element name
	 */
	szElement = XMLString::transcode (localname);
	uSize = attrs.getLength ();

	if (strcmp (szElement, "PropertyDetails") == 0) {
		// Set up the default node or link in case it's not contained in the file
		pcNodeLink = pcDefaultNodeLink;
	}
	else if (strcmp (szElement, "PropertySet") == 0) {
		// Set up the default node or link in case it's not contained in the file
		pcNodeLink = pcDefaultNodeLink;
		// Check whether this is a node, link, default etc.
		ePropSetType = XMLPROPSET_UNDEFINED;
		nID = 0;
		for (uAttributes = 0; uAttributes < uSize; uAttributes++) {
			szAttribute = XMLString::transcode (attrs.getLocalName (uAttributes));
			szValue = XMLString::transcode (attrs.getValue (uAttributes));
			if (_stricmp (szAttribute, "id") == 0) {
				// Read in the node ID
				sscanf (szValue, "%d", & nID);
			}
			else if (_stricmp (szAttribute, "type") == 0) {
				// Figure out whether this is a node, link, default properties etc.
				if (_stricmp (szValue, "node") == 0) {
					ePropSetType = XMLPROPSET_NODE;
				}
				else if (_stricmp (szValue, "link") == 0) {
					ePropSetType = XMLPROPSET_LINK;
				}
				else if (_stricmp (szValue, "default") == 0) {
					ePropSetType = XMLPROPSET_DEFAULT;
				}
			}

			// Release the strings
			XMLString::release (& szValue);
			XMLString::release (& szAttribute);
		}

		// Now find out where we're going to store the properties
		switch (ePropSetType) {
		case XMLPROPSET_NODE:
			// Find the relevant node
			pcNodeLink = psWindow->pTopology->DecodeNodeNumber (nID);
			break;
		case XMLPROPSET_LINK:
			// Find the relevant link
			pcNodeLink = psWindow->pTopology->DecodeLinkNumber (nID);
			break;
		case XMLPROPSET_DEFAULT:
			// These are the default properties for new nodes and links
			pcNodeLink = psWindow->pcDefaultProperties;
			break;
		default:
			// Use the value set external to the class
			pcNodeLink = pcDefaultNodeLink;
			break;
		}
	}
	else if (strcmp (szElement, "Option") == 0) {
		szOption.clear ();
		/* Create Control for Combo box */
		boOptionWidget = true;
	}
	else if (strcmp (szElement, "Property") == 0) {
		szProperty.clear ();
		// Start by finding the property name
		szPropertyName.clear ();
		for (uAttributes = 0; uAttributes < uSize; uAttributes++) {
			szAttribute = XMLString::transcode (attrs.getLocalName (uAttributes));
			if (strcmp (szAttribute, "id") == 0) {
				szValue = XMLString::transcode (attrs.getValue (uAttributes));
				// This is the name of the property
				szPropertyName.assign (szValue);
			}
		}
		// Release the strings
		XMLString::release (& szValue);
		XMLString::release (& szAttribute);

		/* Create control for every Property tag */
		for (uAttributes = 0; uAttributes < uSize; uAttributes++) {
			szAttribute = XMLString::transcode (attrs.getValue (uAttributes));
			if (strcmp (szAttribute, "option") == 0) {
				if (pcNodeLink) {
					pcNodeLink->PropertyRemove (szPropertyName.c_str ());
				}
				/* Create Control for an option widget */
				AddControl ();
				pcCurrent->szCaption = szAttribute;
				nControlID++;
				pcCurrent->nControlID = nControlID;		/* This will assign a Uniqe ID to each control */
				pcCurrent->szLabel = szPropertyName;
				nControlCount++;
				boPropertyCheck = true;
			}
			else if (strcmp (szAttribute, "bool") == 0) {
				/* Create Control for Check box */
				// Add the property
				if (pcNodeLink) {
					pcNodeLink->PropertySetBool (szPropertyName.c_str (), false);
				}
				// Add the control
				AddControl ();
				pcCurrent->szCaption = szAttribute;
				nControlID++;
				pcCurrent->nControlID = nControlID;		/* This will assign a Uniqe ID to each control */
				pcCurrent->szLabel = szPropertyName;
				nControlCount++;
			}
			else if (strcmp (szAttribute, "int") == 0) {
				/* Create Control for Integer Values */
				string nMinValue;
				string nMaxValue;

				// Add the property
				if (pcNodeLink) {
					pcNodeLink->PropertySetInt (szPropertyName.c_str (), 0);
				}
				// Add the control
				/* If there are four values inside the tag */
				if (uSize > 3) {
					/* New */
					AddControl ();
					pcCurrent->szCaption = szAttribute;
					nControlID++;
					pcCurrent->nControlID = nControlID;	/* This will assign a Uniqe ID to each control */
					pcCurrent->szLabel = szPropertyName;

					/* Assign Values */
					pcCurrent->nMinValue = atoi (XMLString::transcode (attrs.getValue (uAttributes - 1)));
					pcCurrent->nMaxValue = atoi (XMLString::transcode (attrs.getValue (uAttributes + 2)));
				}
				else {
					/* New */
					AddControl ();
					pcCurrent->szCaption = szAttribute;
					nControlID++;
					pcCurrent->nControlID = nControlID;	/* This will assign a Uniqe ID to each control */
					pcCurrent->szLabel = szPropertyName;

					/* Assign Values */
					pcCurrent->nMinValue = atoi (XMLString::transcode (attrs.getValue (uAttributes - 1)));
					pcCurrent->nMaxValue = 100;
				}
				nControlCount++;
			}
			else if (strcmp (szAttribute, "float") == 0) {
				/* Create Control for Floating Point Values */
				// Add the property
				if (pcNodeLink) {
					pcNodeLink->PropertySetFloat (szPropertyName.c_str (), 0.0);
				}
				// Add the control
				AddControl ();
				pcCurrent->szCaption = szAttribute;
				nControlID++;
				pcCurrent->nControlID = nControlID;		/* This will assign a Uniqe ID to each control */
				pcCurrent->szLabel = szPropertyName;
				nControlCount++;
			}
			else if (strcmp (szAttribute, "string") == 0) {
				/* Create Text Control */
				// Add the property
				if (pcNodeLink) {
					pcNodeLink->PropertySetString (szPropertyName.c_str (), "");
				}
				// Add the control
				AddControl ();
				pcCurrent->szCaption = szAttribute;
				nControlID++;
				pcCurrent->nControlID = nControlID;		/* This will assign a Uniqe ID to each control */
				pcCurrent->szLabel = szPropertyName;
				nControlCount++;
			}

			// Release the string
			XMLString::release (&szAttribute);
		}
	}

	// Release the string
	XMLString::release (& szElement);
}

/*
 =======================================================================================================================
 *    TODO:
 =======================================================================================================================
 */
cControls* cXMLPropertySet::ReturnControls () {
	return pcControls;
}

/*
 =======================================================================================================================
 *    TODO:
 =======================================================================================================================
 */
void cXMLPropertySet::characters (const XMLCh* chars, const unsigned int length) {
	char*  szMessage;
	szMessage = XMLString::transcode (chars);

	if (boOptionWidget) {
		// This is an option
		szOption.append (szMessage);
	}
	else {
		// This is something other than an option, so we'll accumulate the details
		szProperty.append (szMessage);
	}

	// Release the string
	XMLString::release (& szMessage);
}

 /*
 =======================================================================================================================
 *    Test whether a charater is whitespace or not
 =======================================================================================================================
 */
bool cXMLPropertySet::IsWhitespace (char cCharacter) {
	bool boIsWhitespace;

	switch (cCharacter) {
	case ' ':
	case '\t':
	case '\n':
	case '\r':
		boIsWhitespace = true;
		break;
	default:
		boIsWhitespace = false;
		break;
	}

	return boIsWhitespace;
}
 
 /*
 =======================================================================================================================
 *    Strip the whitespace from the start and end of a string
 =======================================================================================================================
 */
string cXMLPropertySet::StripWhitespaceAround (string szSurrounded) {
	int nStart;
	int nEnd;
	int nLength;
	string szTidied;

	// Strip the spaces from around the string
	nLength = (int)szSurrounded.length ();
	nStart = 0;
	while ((nStart < nLength) && (IsWhitespace (szSurrounded[nStart]))) {
		nStart++;
	}
	if (nStart < nLength) {
		szTidied = szSurrounded.substr (nStart, nLength);
	}
	else {
		szTidied = "";
	}
	nLength = (int)szTidied.length ();
	nEnd = nLength;
	while ((nEnd > 0) && (IsWhitespace (szTidied[nEnd - 1]))) {
		nEnd--;
	}
	if (nEnd > 0) {
		szTidied = szTidied.substr (0, nEnd);
	}
	else {
		szTidied = "";
	}

	return szTidied;
}
 
 /*
 =======================================================================================================================
 *    TODO:
 =======================================================================================================================
 */
void cXMLPropertySet::endElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname) {
	char*  szString;
	PROPTYPE ePropType;
	bool boValue;
	float fValue;
	int nValue;
	string szTidied;

	szString = XMLString::transcode (localname);

	if (strcmp (szString, "PropertySet") == 0) {
		// Do nothing
	}
	else if (strcmp (szString, "Option") == 0) {
		/* Create control for every Property tag */
		// Add the property
		if (boOptionWidget) {
			if (pcNodeLink) {
				pcNodeLink->PropertySetOptionAdd (szPropertyName.c_str (), szOption.c_str ());
			}
			if (pcCurrent) {
				pcCurrent->cValue.push_back (szOption.c_str ());
			}
			szOption.clear ();
			nProperties++;
		}
		boOptionWidget = false;
	}
	else if (strcmp (szString, "Property") == 0) {
		if (pcNodeLink) {
			// Tidy up the property value
			szTidied = StripWhitespaceAround (szProperty);

			// Store the value for the property if there is one
			ePropType = pcNodeLink->PropertyGetType (szPropertyName.c_str ());
			switch (ePropType) {
			case PROPTYPE_BOOL:
				sscanf (szTidied.c_str (), "%d", & boValue);
				pcNodeLink->PropertySetBool (szPropertyName.c_str (), boValue);
				break;
			case PROPTYPE_FLOAT:
				sscanf (szTidied.c_str (), "%f", & fValue);
				pcNodeLink->PropertySetFloat (szPropertyName.c_str (), fValue);
				break;
			case PROPTYPE_INT:
				sscanf (szTidied.c_str (), "%d", & nValue);
				pcNodeLink->PropertySetInt (szPropertyName.c_str (), nValue);
				break;
			case PROPTYPE_OPTION:
				pcNodeLink->PropertySetOption (szPropertyName.c_str (), szTidied.c_str ());
				break;
			case PROPTYPE_STRING:
				pcNodeLink->PropertySetString (szPropertyName.c_str (), szProperty.c_str ());
				break;
			}
		}
		szProperty.clear ();
		if (boPropertyCheck) {
			boPropertyCheck = false;
			nProperties = 0;
		}
	}
	else if (strcmp (szString, "Node") == 0) {
		ePropSetType = XMLPROPSET_INVALID;
		pcNodeLink = NULL;
	}
	else if (strcmp (szString, "Link") == 0) {
		ePropSetType = XMLPROPSET_INVALID;
		pcNodeLink = NULL;
	}
}

/*
 =======================================================================================================================
 *  Save property set to the XML file
 =======================================================================================================================
 */
void cXMLPropertySet::SavePropertySet (FILE * hFile, int nIndent, cProperties * pcProperties, XMLPROPSET eType, int nID) {
	PropertyMap const * pcPropertyMap;
	PropertyMap::const_iterator cPropertyIter;
	Property const * psProperty;
	string szName;
	list<string>::const_iterator sOptionIter;
	int nOptionNum;

	switch (eType) {
	case XMLPROPSET_NODE:
		fprintf (hFile, "%*s<PropertySet type=\"node\" id=\"%d\">\n", nIndent * 2, " ", nID);
		break;
	case XMLPROPSET_LINK:
		fprintf (hFile, "%*s<PropertySet type=\"link\" id=\"%d\">\n", nIndent * 2, " ", nID);
		break;
	case XMLPROPSET_DEFAULT:
		fprintf (hFile, "%*s<PropertySet type=\"default\">\n", nIndent * 2, " ");
		break;
	default:
		fprintf (hFile, "%*s<PropertySet>\n", nIndent * 2, " ");
		break;
	}
	nIndent++;

	pcPropertyMap = pcProperties->GetPropertyMap ();
	for (cPropertyIter = pcPropertyMap->begin (); cPropertyIter != pcPropertyMap->end (); ++cPropertyIter) {
		szName = cPropertyIter->first;
		psProperty = & cPropertyIter->second;
		switch (psProperty->eType) {
		case PROPTYPE_INT:
			fprintf (hFile, "%*s<Property id=\"%s\" type=\"int\">%d</Property>\n", nIndent * 2, " ", szName.c_str (), psProperty->nValue);
			break;
		case PROPTYPE_STRING:
			fprintf (hFile, "%*s<Property id=\"%s\" type=\"string\">%s</Property>\n", nIndent * 2, " ", szName.c_str (), psProperty->szValue);
			break;
		case PROPTYPE_FLOAT:
			fprintf (hFile, "%*s<Property id=\"%s\" type=\"float\">%f</Property>\n", nIndent * 2, " ", szName.c_str (), psProperty->fValue);
			break;
		case PROPTYPE_BOOL:
			fprintf (hFile, "%*s<Property id=\"%s\" type=\"bool\">%d</Property>\n", nIndent * 2, " ", szName.c_str (), psProperty->boValue);
			break;
		case PROPTYPE_OPTION:
			fprintf (hFile, "%*s<Property id=\"%s\" type=\"option\">\n", nIndent * 2, " ", szName.c_str ());
			nIndent++;
			// Save out the options
			nOptionNum = 1;
			for (sOptionIter = psProperty->aszValue.begin (); sOptionIter != psProperty->aszValue.end (); ++sOptionIter) {
				fprintf (hFile, "%*s<Option enum=\"%d\">%s</Option>\n", nIndent * 2, " ", nOptionNum, sOptionIter->c_str ());
				nOptionNum++;
			}
			if (psProperty->szValue) {
				fprintf (hFile, "%*s%s\n", nIndent * 2, " ", psProperty->szValue);
			}
			nIndent--;
			fprintf (hFile, "%*s</Property>\n", nIndent * 2, " ");
			break;
		default:
			// Do nothing
			break;
		}
	}

	nIndent--;
	fprintf (hFile, "%*s</PropertySet>\n", nIndent * 2, " ");
}

/*
 =======================================================================================================================
 *  Save node property set to the XML file
 =======================================================================================================================
 */
void cXMLPropertySet::SaveNode (FILE * hFile, int nIndent, cTopologyNode * psNode) {
	SavePropertySet (hFile, nIndent, psNode, XMLPROPSET_NODE, psNode->getID ());
}

/*
 =======================================================================================================================
 *  Save link property set to the XML file
 =======================================================================================================================
 */
void cXMLPropertySet::SaveLink (FILE * hFile, int nIndent, cTopologyLink * psLink) {
	SavePropertySet (hFile, nIndent, psLink, XMLPROPSET_LINK, psLink->nLinkID);
}

/*
 =======================================================================================================================
 *    Save the elements to the XML file
 =======================================================================================================================
 */
void cXMLPropertySet::Save (FILE * hFile, int nIndent) {
	cTopologyList * pList;
	cTopologyNode * psIterator;
	cTopologyLink * pLinks;

	fprintf (hFile, "%*s<PropertyDetails>\n", nIndent * 2, " ");
	nIndent++;

	// Save the default properties
	SavePropertySet (hFile, nIndent, psWindow->pcDefaultProperties, XMLPROPSET_DEFAULT, 0);

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
	fprintf (hFile, "%*s</PropertyDetails>\n", nIndent * 2, " ");
}

void cXMLPropertySet::LoadStart (char const * szChunkName) {
}

void cXMLPropertySet::LoadEnd (char const * szChunkName) {
}

/*
 =======================================================================================================================
 *    Set up the default properties for when loading a Property Set file
 =======================================================================================================================
 */
void cXMLPropertySet::ChangeDefaults (cProperties * pcDefaultNodeLink) {
	this->pcDefaultNodeLink = pcDefaultNodeLink;
}
