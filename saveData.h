/*$T MATTS/saveData.h GC 1.140 07/01/09 21:12:11 */
/*
 * Name: saveData.h ;
 * ;
 * Purpose: Header file for saveData ;
 * Members ;
 * szFileName - Filename of the XML file to use for referring the sensibility
 * level of data to to transfered. ;
 * Methods ;
 * saveFile(char*, int): generate all elements and format the information ;
 * createXMLFile(): write the elements into XML file ;
 */
#include <windows.h>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOM.hpp>
#if defined(XERCES_NEW_IOSTREAMS)
#include <iostream>
#else
#include <iostream.h>
#endif
#include <xercesc/util/OutOfMemoryException.hpp>

#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMWriter.hpp>

#include <xercesc/framework/StdOutFormatTarget.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#pragma warning(disable : 4244)
#include <xercesc/parsers/XercesDOMParser.hpp>
#pragma warning(default : 4244)
#include <xercesc/util/XMLUni.hpp>

#include <string.h>
#include <stdlib.h>
#include <fstream>

XERCES_CPP_NAMESPACE_USE class saveData
{
public:
	saveData (char* szSelected);
	~ saveData ();
	void createXMLFile ();
	void saveFile (char* szName, int nSensitivityLevel);
private:
	xercesc::DOMDocument * doc;
	DOMElement*	 rootElem;
	DOMImplementation*	impl;
	char*  szFileName;
};
