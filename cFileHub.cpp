/*$T MATTS/cFileHub.cpp GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cFileHub.cpp ;
 * Last Modified: 17/06/08 ;
 * ;
 * Purpose: Save and load everything to/from file ;
 */
#include "cFileHub.h"
#include <string>
#include <malloc.h>

#pragma warning(disable : 4267)
#pragma warning(disable : 4244)
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#pragma warning(default : 4267)
#pragma warning(default : 4244)

using namespace std;

#define BLOCKQUANTUM	(8)

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */

cFileHub::cFileHub () {
	apFileRoutine = NULL;
	aszIdentifier = NULL;
	nFileRoutines = 0;
	nFileRoutineAlloc = 0;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */

cFileHub::~cFileHub () {
	int nIdentifier;

	// Free up the file routine pointers (but not the routines!)
	if (apFileRoutine) {
		free (apFileRoutine);
		apFileRoutine = NULL;
	}

	// Free up the identifiers
	if (aszIdentifier) {
		for (nIdentifier = 0; nIdentifier < nFileRoutines; nIdentifier++) {
			if (aszIdentifier[nIdentifier]) {
				free (aszIdentifier[nIdentifier]);
				aszIdentifier[nIdentifier] = NULL;
			}
		}
		free (aszIdentifier);
		aszIdentifier = NULL;
	}
}

/*
 =======================================================================================================================
 *  Load the file in using xerces and the cFileHubLoadHandler handler
 =======================================================================================================================
 */

void cFileHub::Load (char const * szFilename) {
	SAX2XMLReader* psParser;

	XMLPlatformUtils::Initialize ();

	cFileHubLoadHandler *  psHandler;
	psParser = XMLReaderFactory::createXMLReader ();
	psHandler = new cFileHubLoadHandler (this);

	psParser->setContentHandler (psHandler);
	psParser->setErrorHandler (psHandler);

	psParser->parse (szFilename);

	delete psParser;
	delete psHandler;
	XMLPlatformUtils::Terminate ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */

void cFileHub::Save (char const * szFilename) {
	FILE * hFile;
	int nFileRoutine;
	int nIndent;

	// Open the file for writing
	hFile = fopen (szFilename, "w");
	nIndent = 0;
	if (hFile) {
		// Opening preamble
		fprintf (hFile, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n");
		fprintf (hFile, "<ComposeComposite>\n");
		nIndent++;

		// Cycle through all of the save routines to save out the data
		for (nFileRoutine = 0; nFileRoutine < nFileRoutines; nFileRoutine++) {
			fprintf (hFile, "%*s<FileHubChunk type=\"%s\">\n", nIndent * 2, " ", aszIdentifier[nFileRoutine]);
			nIndent++;
			apFileRoutine[nFileRoutine]->Save (hFile, nIndent);
			nIndent--;
			fprintf (hFile, "%*s</FileHubChunk>\n", nIndent * 2, " ");
		}

		// Closing postamble
		nIndent--;
		fprintf (hFile, "</ComposeComposite>\n");
		fclose (hFile);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cFileHub::AddFileRoutine (char const * szIdentifier, cLoadSave * psRoutine) {
	if (nFileRoutines >= nFileRoutineAlloc) {
		// Need some more space
		nFileRoutineAlloc = ((int)(nFileRoutines / BLOCKQUANTUM) + 1) * BLOCKQUANTUM;
		apFileRoutine = (cLoadSave * *)realloc (apFileRoutine, sizeof (cLoadSave *) * nFileRoutineAlloc);
		aszIdentifier = (char * *)realloc (aszIdentifier, sizeof (char *) * nFileRoutineAlloc);
	}

	// Make a copy of the pointer to the routine and the identifier
	apFileRoutine[nFileRoutines] = psRoutine;
	aszIdentifier[nFileRoutines] = (char *)malloc (strlen (szIdentifier) + 1);
	strcpy (aszIdentifier[nFileRoutines], szIdentifier);

	nFileRoutines++;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cFileHub::RemoveFileRoutine (char const * szIdentifier) {
	int nFileRoutine;
	int nShiftDown;

	// Find the file routine
	nShiftDown = 0;
	for (nFileRoutine = 0; nFileRoutine < nFileRoutines; nFileRoutine++) {
		// Copy down the data
		if (nShiftDown > 0) {
			apFileRoutine[nFileRoutine] = apFileRoutine[nFileRoutine + nShiftDown];
			aszIdentifier[nFileRoutine] = aszIdentifier[nFileRoutine + nShiftDown];
		}
		if (_stricmp (szIdentifier, aszIdentifier[nFileRoutine]) == 0) {
			// Remove the routine
			apFileRoutine[nFileRoutine] = NULL;
			free (aszIdentifier[nFileRoutine]);
			aszIdentifier[nFileRoutine] = NULL;
			// Adjust the position of things
			nShiftDown++;
			nFileRoutine--;
			nFileRoutines--;
		}
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cLoadSave * cFileHub::FindFileRoutine (char const * szIdentifier) {
	int nFileRoutine;
	cLoadSave * psFound;

	// Find the file routine
	psFound = NULL;
	for (nFileRoutine = 0; (!psFound) && (nFileRoutine < nFileRoutines); nFileRoutine++) {
		if (_stricmp (szIdentifier, aszIdentifier[nFileRoutine]) == 0) {
			// Found the routine
			psFound = apFileRoutine[nFileRoutine];
		}
	}

	return psFound;
}








/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cFileHubLoadHandler::cFileHubLoadHandler (cFileHub * psFileHub) {
	nLoadLevel = -1;
	nStartLevel = -1;
	nChunkLevel = -1;
	this->psFileHub = psFileHub;
	psChunkRoutine = NULL;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cFileHubLoadHandler::~cFileHubLoadHandler () {
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cFileHubLoadHandler::startElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname,
	const Attributes&  attrs) {
	char* szElementName;
  unsigned int uAttribute;
	char * szAttributeName;
	char * szAttributeValue;

	szElementName = XMLString::transcode (localname);
	nLoadLevel++;

	if (nChunkLevel < 0) {
		if (nStartLevel < 0) {
			if (_stricmp (szElementName, "ComposeComposite") == 0) {
				nStartLevel = nLoadLevel;
			}
		}
		else {
			if (nLoadLevel > nStartLevel) {
				if (_stricmp (szElementName, "FileHubChunk") == 0) {
					// Check the attributes
					psChunkRoutine = NULL;
					for (uAttribute = 0; uAttribute < attrs.getLength(); uAttribute++) {
						szAttributeName = XMLString::transcode (attrs.getLocalName (uAttribute));
						szAttributeValue = XMLString::transcode (attrs.getValue (uAttribute));

						if (_stricmp (szAttributeName, "type") == 0) {
							// Find the chunk type, so we know who to call
							psChunkRoutine = psFileHub->FindFileRoutine (szAttributeValue);
							sChunkRoutine = szAttributeValue;
						}

						XMLString::release (& szAttributeName);
						XMLString::release (& szAttributeValue);
					}
					if (psChunkRoutine) {
						psChunkRoutine->LoadStart (sChunkRoutine.c_str ());
					}
					nChunkLevel = nLoadLevel;
				}
			}
		}
	}
	else {
		if ((psChunkRoutine) && (nLoadLevel > nChunkLevel)) {
			// We're currently processing a chunk
			psChunkRoutine->startElement (uri, localname, qname, attrs);
		}
	}

	XMLString::release (& szElementName);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cFileHubLoadHandler::characters (const XMLCh *const chars, const unsigned int length) {
	if ((psChunkRoutine) && (nLoadLevel > nChunkLevel)) {
		// We're currently processing a chunk
		psChunkRoutine->characters (chars, length);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cFileHubLoadHandler::endElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname) {
	char* szElementName;

	szElementName = XMLString::transcode (localname);
  nLoadLevel--;

	if ((nLoadLevel == nChunkLevel) && (_stricmp (szElementName, "ComposeComposite") == 0)) {
		// End of the file portion that we're interested in
		nStartLevel = -1;
	}
	else if ((nLoadLevel == nStartLevel) && (_stricmp (szElementName, "FileHubChunk") == 0)) {
		// End of the chunk
		if (psChunkRoutine) {
			psChunkRoutine->LoadEnd (sChunkRoutine.c_str ());
		}

		nChunkLevel = -1;
		psChunkRoutine = NULL;
	}
	else if ((psChunkRoutine) && (nLoadLevel > nChunkLevel)) {
		// We're currently processing a chunk
		psChunkRoutine->endElement (uri, localname, qname);
	}

	XMLString::release (& szElementName);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cFileHubLoadHandler::fatalError (const SAXParseException& exception) {
	char*  message;

	if ((psChunkRoutine) && (nLoadLevel > nChunkLevel)) {
		psChunkRoutine->fatalError (exception);
	}
	else {
		message = XMLString::transcode (exception.getMessage ());
		cout << "Fatal Error: " << message << " at line: " << exception.getLineNumber () << endl;
		XMLString::release (& message);
	}
}

