/*$T DynamicMatts/cXMLPropertySet.hpp GC 1.140 12/27/09 14:10:04 */
#ifndef CXMLPROPERTYSET_H
#define CXMLPROPERTYSET_H

#pragma warning(disable : 4267)
#pragma warning(disable : 4244)
#include <xercesc/sax2/DefaultHandler.hpp>
#pragma warning(default : 4267)
#pragma warning(default : 4244)
#include "cWindow.h"
#include "cControls.h"

XERCES_CPP_NAMESPACE_USE

typedef enum {
	XMLPROPSET_INVALID = -1,

	XMLPROPSET_NODE,
	XMLPROPSET_LINK,
	XMLPROPSET_DEFAULT,
	XMLPROPSET_UNDEFINED,

	XMLPROPSET_NUM
} XMLPROPSET;

/*
 =======================================================================================================================
 *    Xerces handler for reading the PropertySet XML file
 =======================================================================================================================
 */
class cXMLPropertySet : public cLoadSave
{
public:
	/*
	 * Constructors and Destructor
	 */
	cXMLPropertySet (cWindow * psWindow);
	~cXMLPropertySet ();
	cControls*	ReturnControls ();

	/*
	 * Handlers for the SAX ContentHandler interface
	 */
	void Save (FILE * hFile, int nIndent);
	void LoadStart (char const * szChunkName);
	void LoadEnd (char const * szChunkName);
	void startElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname, const Attributes&  attrs);
	void endElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname);
	void characters (const XMLCh *const chars, const unsigned int length);
	void ChangeDefaults (cProperties * pcDefaultNodeLink);

	void createDialog ();
private:
	cControls * AddControl ();
	void SaveNode (FILE * hFile, int nIndent, cTopologyNode * psNode);
	void SaveLink (FILE * hFile, int nIndent, cTopologyLink * psLink);
	void SavePropertySet (FILE * hFile, int nIndent, cProperties * pcProperties, XMLPROPSET eType, int nID);
	static bool IsWhitespace (char cCharacter);
	static string StripWhitespaceAround (string szSurrounded);
	bool boPropertyCheck;
	bool boOptionWidget;
	int nControlCount;
	int nProperties;
	int nControlID;
	cControls * pcControls;
	cControls * pcCurrent;
	cWindow * psWindow;

	cProperties * pcNodeLink;
	XMLPROPSET ePropSetType;
	int nID;
	string szPropertyName;
	string szProperty;
	string szOption;
	cProperties * pcDefaultNodeLink;
};
#endif
