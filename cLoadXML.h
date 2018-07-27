/*$T MATTS/cLoadXML.h GC 1.140 07/01/09 21:12:08 */
/*
 * ;
 * Name: cLoadXML.h ;
 * ;
 * Purpose: Header file for cLoadXML ;
 * Members ;
 * recordNode: data structure used to store parsed nodes ;
 * recordLink: data structure used to store parsed links ;
 * nodeIndex - number of nodes ;
 * linkIndex - number of links ;
 * Methods ;
 * Sax2ReadXML(const char *filename): load XML file ;
 */
#pragma warning(disable : 4267)
#pragma warning(disable : 4244)
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#pragma warning(default : 4267)
#pragma warning(default : 4244)
#include "SHandler.h"

/* include <iostream.h> */
#include <windows.h>

XERCES_CPP_NAMESPACE_USE class cLoadXML
{
public:
	cLoadXML(nodeXML[], linkXML[], int *nIndex, int *lIndex);
	~ cLoadXML ();
	void Sax2ReadXML (const char* filename);

	nodeXML*  recordNode;
	linkXML*  recordLink;
	int*  nodeIndex;
	int*  linkIndex;
private:
};
