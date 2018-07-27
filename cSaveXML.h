/*$T MATTS/cSaveXML.h GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cSaveXML.h ;
 * ;
 * Purpose: Header file for cSaveXML ;
 * Members ;
 * szFileName - Filename of the XML file to use for saving the network topology or
 * scenarios ;
 * Methods ;
 * saveNode(): generate and save all elements for the nodes ;
 * saveLink(): generate and save all elements for the links ;
 * createXMLFile(): write the elements into XML file ;
 */
#ifndef CSAVEXML_H
#define CSAVEXML_H

#define WIN32_LEAN_AND_MEAN 1

#include "cTopologyNode.h"
#include "cFileHub.h"

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

/*
 * The current tag used for loading the XML events
 */
typedef enum {
  LOADSAVETAG_INVALID = -1,

	LOADSAVETAG_TOPOLOGY,

	LOADSAVETAG_CAMERA,
	LOADSAVETAG_CENTRE,
	LOADSAVETAG_NODE,
	LOADSAVETAG_LINK,

	LOADSAVETAG_ID,
	LOADSAVETAG_POSITION,
	LOADSAVETAG_NAME,
	LOADSAVETAG_TYPE,
	LOADSAVETAG_SENSITIVITY,
	LOADSAVETAG_ENCRYPTION,
	LOADSAVETAG_STAFFSKILLS,
	LOADSAVETAG_FIREWALL,
	LOADSAVETAG_IDS,
	LOADSAVETAG_SECMAX,
	LOADSAVETAG_SECMIN,
	LOADSAVETAG_PROPERTIES,
	LOADSAVETAG_EXTERNAL,
	LOADSAVETAG_ADHOC,
	LOADSAVETAG_RANGETX,
	LOADSAVETAG_RANGERX,

	LOADSAVETAG_OUTID,
	LOADSAVETAG_INID,
	LOADSAVETAG_LINKID,
	LOADSAVETAG_CHANNELOUT,
	LOADSAVETAG_CHANNELIN,
	LOADSAVETAG_OUTPROPERTY,
	LOADSAVETAG_INPROPERTY,
	LOADSAVETAG_SECURITY,

  LOADSAVETAG_NUM
} LOADSAVETAG;

class cWindow;
class EventNewNode;
class EventNewLink;
class EventPropertiesNode;
class EventPropertiesLink;

XERCES_CPP_NAMESPACE_USE

class cSaveXML : public cLoadSave
{
public:
	cSaveXML (cWindow * psWindow);
	~ cSaveXML ();

	void Save (FILE * hFile, int nIndent);
	void LoadStart (char const * szChunkName);
	void LoadEnd (char const * szChunkName);
	void startElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname, const Attributes&	attrs);
	void endElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname);
	void characters (const XMLCh *const chars, const unsigned int length);
	void ChangeDefaults (bool boCreateNodes, int nDefaultNodeID);

private:
	cWindow * psWindow;
  string sLoadText;
	LOADSAVETAG eObject;
	LOADSAVETAG eProperty;
	EventNewNode * psNewNode;
	EventNewLink * psNewLink;
	EventPropertiesNode * psPropertiesNode;
	EventPropertiesLink * psPropertiesLink;
	bool boCreateNodes;
	int nDefaultNodeID;

	void SaveNode (FILE * hFile, int nIndent, cTopologyNode * psNode);
	void SaveLink (FILE * hFile, int nIndent, cTopologyLink * psLink);
};

#endif
