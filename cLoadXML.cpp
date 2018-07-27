/*$T MATTS/cLoadXML.cpp GC 1.140 07/01/09 21:12:08 */
/*
 * Name: cLoadXML ;
 * Purpose:parse the saved network topology (XML file) into data structures which
 * can be accessed in the main program. ;
 */
#include "cLoadXML.h"

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */

XERCES_CPP_NAMESPACE_USE cLoadXML::cLoadXML (nodeXML tempNodeRecord[], linkXML tempLinkRecord[], int*  nIndex,
											 int*  lIndex) {
	XMLPlatformUtils::Initialize ();
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
cLoadXML::~cLoadXML () {
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cLoadXML::Sax2ReadXML (const char* filename) {

	/* create SAX2 parser and read the basic building block setting file */
	SAX2XMLReader*	parser;

	/* create handler that will parse the XML elements */
	SHandler*  handler;
	parser = XMLReaderFactory::createXMLReader ();
	handler = new SHandler (recordNode, recordLink, nodeIndex, linkIndex);

	parser->setContentHandler (handler);
	parser->setErrorHandler (handler);

	try
	{
		parser->parse (filename);
	}

	catch (const XMLException&toCatch) {
		char*  message;
		message = XMLString::transcode (toCatch.getMessage ());

		/*
		 * err << "Exception message is: \n" ;
		 * << message << "\n";
		 */
		XMLString::release (&message);
	}

	catch (const SAXParseException&toCatch) {
		char*  message;
		message = XMLString::transcode (toCatch.getMessage ());

		/*
		 * err << "Exception message is: \n" ;
		 * << message << "\n";
		 */
		XMLString::release (&message);
	}

	catch (...) {

		/*
		 * err << "Unexpected Exception \n" ;
		 */
	}

	delete parser;
	delete handler;
	XMLPlatformUtils::Terminate ();
}
