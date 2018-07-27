/*$T MATTS/XMLCTag.h GC 1.140 07/01/09 21:12:11 */
/*
 * ;
 * Name: XMLCTag.h ;
 * Last Modified: 15/07/08 ;
 * ;
 * Purpose: Compose XML tags for cXMLCompile ;
 */
#ifndef XMLCTAG_H
#define XMLCTAG_H

#include <string.h>
#include <iostream>
using namespace std;

/**
 =======================================================================================================================
 *  The main type for use in computations
 =======================================================================================================================
 */
// Use integer arithmetic for analysis
//typedef int comptype;
//#define compconv "%d"

// Use floating point arithmetic for analysis
//typedef float comptype;
//#define compconv "%f"

// Use double floating point arithmetic for analysis
typedef double comptype;
#define compconv "%lf"

/* Enumeration of the various tags that exist */
typedef enum _XMLCTAG
{
	XMLCTAG_INVALID			= -1,
	XMLCTAG_COMPOSE,
	XMLCTAG_SANDBOX,
	XMLCTAG_PROPERTY,
	XMLCTAG_CONFIGURATION,
	XMLCTAG_COMPONENT,
	XMLCTAG_INPUT,
	XMLCTAG_OUTPUT,
	XMLCTAG_PROCESS,
	XMLCTAG_NUM
} XMLCTAG;

char const *const szXMLCTag[] =
{
	"Compose",
	"Sandbox",
	"Property",
	"Configuration",
	"Component",
	"Input",
	"Output",
	"Process",
	"Num"
};

/* Enumeration of the types of Compose file for the compose tag */
typedef enum _XMLCTYPE
{
	XMLCTYPE_INVALID		= -1,
	XMLCTYPE_STANDARD,
	XMLCTYPE_EXTENDED,
	XMLCTYPE_NUM
} XMLCTYPE;

/* Enumeration of values for the cycle attribute of the input and output tags */
typedef enum _XMLCCYCLE
{
	XMLCCYCLE_INVALID		= -1,
	XMLCCYCLE_REQUIRED,
	XMLCCYCLE_DISALLOW,
	XMLCCYCLE_OPTIONAL,
	XMLCCYCLE_NUM
} XMLCCYCLE;

/* Enumeration of values for the follow attribute of the input and output tags */
typedef enum _XMLCFOLLOW
{
	XMLCFOLLOW_INVALID		= -1,
	XMLCFOLLOW_YES,
	XMLCFOLLOW_NO,
	XMLCFOLLOW_FRESH,
	XMLCFOLLOW_NUM
} XMLCFOLLOW;

/**
 *  XMLCNumarray  */
#define XMLCNUMARRAY_ALLOCATE	(100)

class XMLCNumarray
{
public:
	XMLCNumarray ();
	XMLCNumarray (comptype xInit);
	XMLCNumarray (string szInit);
	~ XMLCNumarray ();

	comptype GetElement (unsigned int uPos);
	void SetElement (comptype xElement, unsigned int uPos);
	unsigned int GetNumElements ();
	void CopyInto (XMLCNumarray* psFrom);
	void ClearAll ();
	void Save (FILE * hFile);
private:
	unsigned int uElements;
	unsigned int uAllocated;
	comptype*  pxElement;
};

/**
 *  XMLCTokenize  */
class XMLCTokenize
{
private:
	unsigned int uTokens;
	string **  aszToken;
public:
	XMLCTokenize (string szTokenize);
	~ XMLCTokenize ();

	unsigned int GetTokensNum ();
	string GetToken (unsigned int uTokenNum);
	bool FindAny (char const *const szToken);
};

/**
 *  XMLCTag  */
class XMLCTag
{
public:
	XMLCTag ();
	virtual ~XMLCTag ();
	XMLCTAG GetTag ();
	string GetID ();
	void SetParent (XMLCTag* psParent);
	void AddChild (XMLCTag* psNewChild);
	virtual void Reset ();
	virtual void Save (FILE * hFile, int nIndent);

	XMLCTag*  psPrev;
	XMLCTag*  psNext;
	XMLCTag*  psParent;
	XMLCTag*  psChild;
protected:
	XMLCTAG eTag;
	string szID;
private:
};

/**
 =======================================================================================================================
 *  XMLCSandbox
 =======================================================================================================================
 */
class XMLCSandbox : public XMLCTag
{
public:
	XMLCSandbox (string szID, string szDescription, string szConfig);
	void Save (FILE * hFile, int nIndent);

	string szDescription;
	string szConfig;

	bool boTestResult;
};

/**
 =======================================================================================================================
 *  XMLCCompose
 =======================================================================================================================
 */
class XMLCCompose : public XMLCTag
{
public:
	XMLCCompose (string szID, string szInit, XMLCTYPE eType);
	~ XMLCCompose ();
	void Reset ();
	void Save (FILE * hFile, int nIndent);

	XMLCTYPE eType;
	XMLCNumarray*  psInit;
	XMLCNumarray*  psInitReset;

	/* Pointers to sandbox elements */
	int nSandboxes;
	XMLCSandbox **	apsSandbox;
};

/**
 =======================================================================================================================
 *  XMLCProperty
 =======================================================================================================================
 */
class XMLCProperty : public XMLCTag
{
public:
	XMLCProperty (string szID, string szDescription);
	void Save (FILE * hFile, int nIndent);

	string szDescription;
};

/**
 =======================================================================================================================
 *  XMLCConfiguration
 =======================================================================================================================
 */
class XMLCConfiguration : public XMLCTag
{
public:
	XMLCConfiguration (string szID);
};

/**
 =======================================================================================================================
 *  XMLCComponent
 =======================================================================================================================
 */
class XMLCComponent : public XMLCTag
{
public:
	XMLCComponent (string szID, string szInit, string szCond, string szAction);
	~ XMLCComponent ();
	void Reset ();
	virtual void Save (FILE * hFile, int nIndent);

	XMLCNumarray*  psInit;
	XMLCNumarray*  psInitReset;
	string szCond;
	string szAction;
protected:
	XMLCComponent ();
};

/**
 =======================================================================================================================
 *  XMLCInput
 =======================================================================================================================
 */
class XMLCInput : public XMLCComponent
{
public:
	XMLCInput (string szID, string szInit, string szCond, string szAction, string szConfig, string szFormat,
			   XMLCCYCLE eCycle, XMLCFOLLOW eFollow);
	~ XMLCInput ();
	void Save (FILE * hFile, int nIndent);

	string szConfig;
	string szFormat;
	XMLCTokenize*  psFormat;
	XMLCCYCLE eCycle;
	XMLCFOLLOW eFollow;
};

/**
 =======================================================================================================================
 *  XMLCOuput
 =======================================================================================================================
 */
class XMLCOuput : public XMLCComponent
{
public:
	XMLCOuput (string szID, string szInit, string szCond, string szAction, string szConfig, string szFormat,
			   XMLCCYCLE eCycle, XMLCFOLLOW eFollow);
	~ XMLCOuput ();
	void Save (FILE * hFile, int nIndent);

	string szConfig;
	string szFormat;
	XMLCTokenize*  psFormat;
	XMLCCYCLE eCycle;
	XMLCFOLLOW eFollow;
};

/**
 =======================================================================================================================
 *  XMLCProcess
 =======================================================================================================================
 */
class XMLCProcess : public XMLCComponent
{
public:
	XMLCProcess (string szID, string szInit, string szCond, string szAction, string szConfig);
	void Save (FILE * hFile, int nIndent);

	string szConfig;
};
#endif /* XMLCTAG_H */
