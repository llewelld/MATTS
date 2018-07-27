/*$T MATTS/loadData.h GC 1.140 07/01/09 21:12:10 */
/*
 * ;
 * Name: loadData.h ;
 * ;
 * Purpose: Header file for loadData ;
 * Members ;
 * recordData:data structure used to store parsed XML elements ;
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
#include "dataHandler.h"

/* include <iostream.h> */
#include <windows.h>

XERCES_CPP_NAMESPACE_USE class loadData
{
public:
	loadData(dataXML[], int *nIndex);
	~ loadData ();
	void Sax2ReadXML (const char* filename);

	dataXML*  recordData;
	int*  dataIndex;
private:
};
