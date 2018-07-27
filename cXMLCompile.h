/*$T MATTS/cXMLCompile.h GC 1.140 07/01/09 21:12:10 */
/*
 * ;
 * Name: cXMLCompile.h ;
 * Last Modified: 15/07/08 ;
 * ;
 * Purpose: Compile Compose XML files into structures for execution ;
 */
#ifndef CXMLCOMPILE_H
#define CXMLCOMPILE_H

#include "XMLCTag.h"
#include "cOptions.h"
#include "cTopologyMirror.h"

#include <xercesc/sax2/DefaultHandler.hpp>
#pragma warning(disable : 4267)
#include <xercesc/sax2/XMLReaderFactory.hpp>
#pragma warning(default : 4267)
XERCES_CPP_NAMESPACE_USE
#define POINT_EXTERNAL	(-1)

typedef enum _XMLCLINKTYPE
{
	XMLCLINKTYPE_INVALID= -1,
	XMLCLINKTYPE_INPUT,
	XMLCLINKTYPE_OUTPUT,
	XMLCLINKTYPE_NONE,
	XMLCLINKTYPE_NUM
} XMLCLINKTYPE;

class CompileHandler;

/**
 *  cXMLCompile
 */
class cXMLCompile
{
private:
	CompileHandler*	 psHandler;
	XMLCCompose*  psRoot;
	XMLCTag*  GetSequential (XMLCTag* psCurrent);
	XMLCTag*  FindTagWithID (char const *const szID);
	bool TestExtended (XMLCTag* psCurrentNode, int nCurrentComponent);
	bool EvaluateCond (XMLCComponent* psCurrentNode);
	void GlobalStackVariables (int nCurrentComponent);
	void GlobalLinkOut (int nCurrentLinkOut);
	void ProcessScan (XMLCTag* psCurrentNode);
	bool CheckCondition (XMLCTag* psCurrentNode, char const *const szCondition);
	void ApplyAction (XMLCTag* psCurrentNode, char const *const szAction);
	comptype AnalyseComponent (char const *const szPropertyID, int nCurrentComponent, int nLinkNum, XMLCTag*	 psCurrentNode,
						  XMLCLINKTYPE eLinkType);
	bool CheckCertifiedProperties (char const *const szPropertyID, char* szLinkProperties, comptype* pxReturn);
	comptype evaluate (char const *const szEval, bool* pboError);
	comptype eval (char const *const szEval, int nLength);
	comptype GetVariableValue (char const *const szEval, int nLength);
	comptype GetInitVal (char const *const szEval, int nLength, int nIndex1, int nIndex2, char const *const szPropertyID);
	comptype SetInitVal (char const *const szEval, int nLength, int nIndex, comptype xValue);
	comptype SetVariableValue (char const *const szEval, int nLength, comptype xValue);
	int GetInOutCount (char const *const szEval, int nLength);
	bool CheckLinkInputProperties (char const *const szPropertyID, int nCurrentComponent, int nLinkNum, comptype* pxReturn);
	bool CheckLinkOutputProperties (char const *const szPropertyID, int nCurrentComponent, int nLinkNum, comptype* pxReturn);
	bool CheckNodeProperties (char const *const szPropertyID, int nCurrentComponent, comptype* pxReturn);

	/* Variables used during the analysis process */
	cTopologyMirror*  psTopologyMirror;
	cOptions*  pOptions;
	int nNodeVisited[POINTS_MAX + LINKLIST_OFFSET];
	int nRecurseLevel;
	int nCurrentLinkIn;
	int nCurrentLinkOut;
	int nCurrentComponentOn;	/* Change name */
	XMLCTag*  psEvalNode;
	bool boError;
public:
	cXMLCompile ();
	cXMLCompile (XMLCCompose* psRoot);
	~ cXMLCompile ();
	void LoadFile (char* szFilename);
	void CheckComposition (cTopologyMirror* psTopologyMirror);
	XMLCCompose*  GetCompiled (void);
	CompileHandler * cXMLCompile::GetFileHandler ();
};

/**
 =======================================================================================================================
 *  CompileHandler
 =======================================================================================================================
 */
class CompileHandler : public cLoadSave
{
public:
	CompileHandler (XMLCCompose ** ppsRoot);
	~ CompileHandler ();
	void startElement (XMLCh const *const uri, XMLCh const *const localname, XMLCh const *const qname,
					   Attributes const&  attrs);
	void characters (const XMLCh *const chars, const unsigned int length);
	void fatalError (const SAXParseException& );
	void endElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname);

	void Save (FILE * hFile, int nIndent);
	void LoadStart (char const * szChunkName);
	void LoadEnd (char const * szChunkName);
	XMLCTag * CompileHandler::GetSequential (XMLCTag * psCurrent);

	/* Important variables needed for the parsing process */
	XMLCTAG eCurrent;
	XMLCCompose **	ppsRoot;
	XMLCTag*  psCurrent;
	string szCharacters;

	/* Some things that it's useful to keep track of */
	int nSandboxes;
};
#endif
