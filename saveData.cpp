/*$T MATTS/saveData.cpp GC 1.140 07/01/09 21:12:11 */
/*
 * Name: saveData ;
 * Purpose:save the name and sensitivility level of data into Data.xml fie for
 * future reference. ;
 * will be called when closing 'Manage Data' dialog
 */
#include "saveData.h"

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */

XERCES_CPP_NAMESPACE_USE

/* initialisation */
saveData::saveData (char* szSelected) {
	XMLPlatformUtils::Initialize ();
	impl = DOMImplementationRegistry::getDOMImplementation (XMLString::transcode ("Core"));

	if (impl != NULL) {
		doc = impl->createDocument (0,	/* root element namespace URI. */
									XMLString::transcode ("Data"),	/* root element name */
									0); /* document type object (DTD). */

		rootElem = doc->getDocumentElement ();
		szFileName = szSelected;
	} /* (inpl != NULL) */ else {
		XERCES_STD_QUALIFIER cerr << "Requested implementation is not supported" << XERCES_STD_QUALIFIER endl;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
saveData::~saveData () {
}

/*
 =======================================================================================================================
 *  generate all elements and format the information
 =======================================================================================================================
 */
void saveData::saveFile (char* szName, int nSensitivityLevel) {
	char szTemp[256];

	DOMElement*	 fileElem;
	DOMElement*	 nameElem;
	DOMText*  nameElemVal;
	DOMElement*	 slElem;
	DOMText*  slElemVal;
	fileElem = doc->createElement (XMLString::transcode ("File"));
	rootElem->appendChild (fileElem);

	nameElem = doc->createElement (XMLString::transcode ("Name"));

	fileElem->appendChild (nameElem);
	sprintf (szTemp, "%s", szName);
	nameElemVal = doc->createTextNode (XMLString::transcode (szTemp));

	nameElem->appendChild (nameElemVal);

	slElem = doc->createElement (XMLString::transcode ("SensitivityLevel"));

	fileElem->appendChild (slElem);
	sprintf (szTemp, "%d", nSensitivityLevel);
	slElemVal = doc->createTextNode (XMLString::transcode (szTemp));

	slElem->appendChild (slElemVal);
}

/*
 =======================================================================================================================
 *  write the elements into XML file
 =======================================================================================================================
 */
void saveData::createXMLFile () {

	/* get a serializer, an instance of DOMWriter */
	DOMWriter*	theSerializer;

	XMLFormatTarget*  myFormTarget;

	DOMNode*  doc1;

	theSerializer = ((DOMImplementationLS*) impl)->createDOMWriter ();

	/* set user specified output encoding */
	theSerializer->setEncoding (0);

	/* try to make it print out a bit nicer */
	if (theSerializer->canSetFeature (XMLUni::fgDOMWRTFormatPrettyPrint, true)) {
		theSerializer->setFeature (XMLUni::fgDOMWRTFormatPrettyPrint, true);
	}

	/*
	 * ;
	 * Plug in a format target to receive the resultant ;
	 * XML stream from the serializer. ;
	 * StdOutFormatTarget prints the resultant XML stream ;
	 * to stdout once it receives any thing from the serializer. ;
	 */
	myFormTarget = new LocalFileFormatTarget (szFileName);

	/* get the DOM representation */
	doc1 = doc;

	/*
	 * ;
	 * do the serialization through DOMWriter::writeNode();
	 */
	theSerializer->writeNode (myFormTarget, *doc1);

	delete theSerializer;
	delete myFormTarget;

	doc->release ();
	XMLPlatformUtils::Terminate ();
}
