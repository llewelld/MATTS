/*$T MATTS/dataHandler.cpp GC 1.140 07/01/09 21:12:10 */
/*
 * Name: dataHandler ;
 * Purpose:parse the Data.xml file into data structure recordData[]. ;
 */
#include "dataHandler.h"
#include <iostream>

using namespace std;

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */

dataHandler::dataHandler (dataXML tempDataRecord[], int* nIndex) {
	recordData = tempDataRecord;
	dataIndex = nIndex;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void dataHandler::startElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname,
								const Attributes&  attrs) {
	char*  message;
	message = XMLString::transcode (localname);
	if (!strcmp (message, "File"))
		(*dataIndex)++;
	else if (!strcmp (message, "Name"))
		propFlag = 'N';
	else if (!strcmp (message, "SensitivityLevel"))
		propFlag = 'S';
	else
		propFlag = '0';

	XMLString::release (&message);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void dataHandler::characters (const XMLCh *const chars, const unsigned int length) {
	char*  message;

	message = XMLString::transcode (chars);

	if (message[0] != 10) {

		/* ignore the whitespaces created by XMLUni::fgDOMWRTFormatPrettyPrint feature */
		switch (propFlag) {
		case 'N':	/* save name */
			strcpy (recordData[*dataIndex].szName, message);
			break;
		case 'S':	/* save sensitivity level */
			recordData[*dataIndex].nSensitivityLevel = atoi (message);
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
void dataHandler::endElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname) {

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
void dataHandler::fatalError (const SAXParseException& exception) {
	char*  message;
	message = XMLString::transcode (exception.getMessage ());

	cout << "Fatal Error: " << message << " at line: " << exception.getLineNumber () << endl;
}
