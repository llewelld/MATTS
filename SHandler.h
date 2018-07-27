/*$T MATTS/SHandler.h GC 1.140 07/01/09 21:12:11 */
/*
 * ;
 * Name: SHandler.h ;
 * ;
 * Purpose: Header file for SHandler ;
 * Members ;
 * recordNode - data structure used to store the parsed nodes ;
 * recordLink - data structure used to store the parsed links ;
 * nodeIndex - number of nodes ;
 * linkIndex - number of links ;
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
 *  data structure to map the nodes in xml file
 -----------------------------------------------------------------------------------------------------------------------
 */
struct nodeXML
{
	int nId;
	char*  szName;
	int nSensitivityLevel;
	char*  szEncryptAlgorithm;
	char*  szCertProps;
	char cSecurityMax;
	char cSecurityMin;
	char*  szStaffSkills;
	bool boFirewall;
	bool boIDS;
	float fXPos;
	float fYPos;
	float fZPos;
	char*  szType;
	bool boExternal;
};

/*
 -----------------------------------------------------------------------------------------------------------------------
 *  data structure to map the links in xml file
 -----------------------------------------------------------------------------------------------------------------------
 */
struct linkXML
{
	int nOutId;
	int nInId;
	int nChannelOut;
	int nChannelIn;
	char*  szOutProperty;
	char*  szInProperty;
	char cSecurityLevel;
	int nSensitivityLevel;
};

class SHandler : public DefaultHandler
{
public:
	SHandler::SHandler(nodeXML tempNodeRecord[], linkXML tempLinkRecord[], int *nIndex, int *lIndex);

	void startElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname,
					   const Attributes&  attrs);
	void characters (const XMLCh *const chars, const unsigned int length);
	void fatalError (const SAXParseException& );
	void endElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname);

	nodeXML*  recordNode;
	linkXML*  recordLink;
	char propFlag;
	int*  nodeIndex;
	int*  linkIndex;
};
