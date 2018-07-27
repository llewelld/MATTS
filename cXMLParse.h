/*$T MATTS/cXMLParse.h GC 1.140 07/01/09 21:12:10 */
/*
 * ;
 * Name: cXMLParse.h ;
 * Last Modified: 20/01/05 ;
 * ;
 * Purpose: Header file for cXMLParse ;
 * Members ;
 * gXmlFile - Filename of the Compose XML file to use for testing ;
 * Methods ;
 * Test () - Tests a composition ;
 */
#ifndef CXMLPARSE_H
#define CXMLPARSE_H

#include "cTopology.h"
#include "cOptions.h"
#include "cXMLCompile.h"
#include "cTopologyMirror.h"

/* include <xercesc/util/PlatformUtils.hpp> */
#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMWriter.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/util/XMLStringTokenizer.hpp>
#include <xercesc/util/XMLChar.hpp>

XERCES_CPP_NAMESPACE_USE

/* define PERFORMANCE_TIME (1000) */
#undef PERFORMANCE_TIME
#define POINTS_MAX			(1024)
#define LINKLIST_OFFSET		(1)
#define POINT_INVALID		(-1)
#define POINT_EXTERNAL		(-1)
#define PROPERTY_SIZE		(1024)
#define VARIABLE_SIZE		(1024)
#define RESULTLIST_LEN		(64)
#define SELECTCHANGE_WAIT	(500)
#define COLOURCHANGE_WAIT	(300)

typedef enum _CYCLE
{
	CYCLE_INVALID	= -1,
	CYCLE_REQUIRED,
	CYCLE_DISALLOW,
	CYCLE_OPTIONAL,
	CYCLE_NUM
} CYCLE;

typedef enum _FOLLOW
{
	FOLLOW_INVALID	= -1,
	FOLLOW_YES,
	FOLLOW_NO,
	FOLLOW_FRESH,
	FOLLOW_NUM
} FOLLOW;

typedef enum _LINKTYPE
{
	LINKTYPE_INVALID= -1,
	LINKTYPE_INPUT,
	LINKTYPE_OUTPUT,
	LINKTYPE_NONE,
	LINKTYPE_NUM
} LINKTYPE;

class cXMLParse
{
private:
	char gXmlFile[1024];
	char gszLogFile[1024];
	XercesDOMParser::ValSchemes gValScheme;
	int gnBoxVisited[POINTS_MAX + LINKLIST_OFFSET];

	XercesDOMParser*  gpDomParser;
	char gszResultList[RESULTLIST_LEN][32];
	bool gboResultList[RESULTLIST_LEN];
	int gnResultListNum;
	bool gboError;
	DOMNode*  gpdnEvalNode;
	char gszResultDesc[RESULTLIST_LEN][64];

	int gnCurrentComponent;
	int gnCurrentLinkOut;
	int gnCurrentLinkIn;
	int gnCurrentChannelIn;
	int gnCurrentChannelOut;
	int gnRecurseLevel;

	HANDLE ghThreadHandle;

	float gfPerformanceTime;

	FILE*  gfhLogFile;
	bool gboLog;

	cOptions*  pOptions;
	cXMLCompile*  psXMLCompile;
	cTopology*	gpcTopology;
	cTopologyMirror*  psTopologyMirror;

	bool Test (DOMNode* pdnCurrentNode, int nCurrentComponent);
	bool TestExtended (DOMNode* pdnCurrentNode, int nCurrentComponent);
	bool CheckCondition (DOMNode* pdnCurrentNode, char* szCondition);
	void ApplyAction (DOMNode* pdnCurrentNode, char* szAction);
	bool EvaluateCond (DOMNode* pdnCurrentNode);
	void ProcessScan (DOMNode* pdnCurrentNode);
	int AnalyseComponent (XMLCh*  xszPropertyID, int nCurrentComponent, int nLinkNum, DOMNode*	pdnCurrentNode,
						  LINKTYPE eLinkType);
	bool CheckCertifiedProperties (char* szPropertyID, char* szLinkProperties, int* pnReturn);
	bool CheckLinkOutputProperties (char* szPropertyID, int nCurrentComponent, int nLinkNum, int* pnReturn);
	bool CheckLinkInputProperties (char* szPropertyID, int nCurrentComponent, int nLinkNum, int* pnReturn);
	bool CheckNodeProperties (char* szPropertyID, int nCurrentComponent, int* pnReturn);

	int evaluate (char* szEval, bool* pboError);
	int eval (char* szEval, int nLength);
	int GetVariableValue (char* szEval, int nLength);
	int SetVariableValue (char* szEval, int nLength, int nValue);
	int GetInOutCount (char* szEval, int nLength);
	int GetInitVal (char* szEval, int nLength, int nIndex1, int nIndex2, char* szPropertyID);
	int SetInitVal (char* szEval, int nLength, int nIndex, int nValue);
	void GlobalStackVariables (int nCurrentComponent);
	void GlobalLinkOut (int nCurrentLinkOut);
public:
	cXMLParse (cTopology* pcTopology);
	~ cXMLParse ();

	void ParseStart (HWND hWnd);
	void LoadXMLFile (char* szFilename);
	void CheckComposition (void);
	void CheckCompositionCompiled (void);
	bool XMLFileLoaded (void);
	int ResultListNum (void);
	char*  ResultListItem (int nItem);
	bool ResultListItemResult (int nItem);
	char*  ResultListDesc (int nItem);
	void cXMLParse:: LogInitialise (void);
	void cXMLParse:: LogDeinitialise (void);
	void LogString (char* szString);
	void LogXString (XMLCh const* xszString);
	void LogValue (char* szString, int nValue);
	void LogNode (DOMNode const* pdnNode);
	float PerformanceTiming ();
	CompileHandler * GetFileHandler ();

	bool gboCompleted;
	bool gboResult;
	HWND ghWnd;
};
#endif
