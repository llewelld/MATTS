/*$T MATTS/dataHandler.h GC 1.140 07/01/09 21:12:10 */
/*
 * ;
 * Name: dataHandler.h ;
 * ;
 * Purpose: Header file for dataHandler ;
 * Members ;
 * recordData - data structure used to store parsed XML elements ;
 * progFlag - a char used to represent the element has been parsed ;
 * Methods ;
 * startElement() - start parsing an element ;
 * characters() - process the string ;
 */
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/Attributes.hpp>

/* include <string> */
XERCES_CPP_NAMESPACE_USE

/*
 -----------------------------------------------------------------------------------------------------------------------
 *  data structure to map the xml file
 -----------------------------------------------------------------------------------------------------------------------
 */
struct dataXML
{
	char*  szName;
	int nSensitivityLevel;
};

class dataHandler : public DefaultHandler
{
public:
	dataHandler::dataHandler(dataXML tempDataRecord[], int *nIndex);

	void startElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname,
					   const Attributes&  attrs);
	void characters (const XMLCh *const chars, const unsigned int length);
	void fatalError (const SAXParseException& );
	void endElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname);

	dataXML*  recordData;
	char propFlag;
	int*  dataIndex;
};
