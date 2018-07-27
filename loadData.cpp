/*$T MATTS/loadData.cpp GC 1.140 07/01/09 21:12:10 */
/*
 * Name: loadData ;
 * Purpose:handler to load the Data.xml file and parse it into data structure
 * recordData[]. ;
 * will be called when initialising 'Manage Data' dialog and loading the
 * sensitivity level of transfered data.
 */
#include "loadData.h"

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */

XERCES_CPP_NAMESPACE_USE loadData::loadData (dataXML tempDataRecord[], int* nIndex) {
	XMLPlatformUtils::Initialize ();
	recordData = tempDataRecord;	/* data structure used to store the elements of XML file */
	dataIndex = nIndex;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
loadData::~loadData () {
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void loadData::Sax2ReadXML (const char* filename) {

	/* create SAX2 parser and read the basic building block setting file */
	SAX2XMLReader*	parser;

	/* create handler that will parse the XML elements */
	dataHandler*  handler;
	parser = XMLReaderFactory::createXMLReader ();
	handler = new dataHandler (recordData, dataIndex);

	parser->setContentHandler (handler);
	parser->setErrorHandler (handler);

	try
	{
		parser->parse (filename);
	}

	catch (const XMLException&toCatch) {
		char*  message;

		message = XMLString::transcode (toCatch.getMessage ());
		XMLString::release (&message);
	}

	catch (const SAXParseException&toCatch) {
		char*  message;

		message = XMLString::transcode (toCatch.getMessage ());

		XMLString::release (&message);
	}

	catch (...) {
	}

	delete parser;
	delete handler;
	XMLPlatformUtils::Terminate ();
}
