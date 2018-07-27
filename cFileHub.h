/*$T MATTS/cFileHub.h GC 1.140 07/01/09 21:12:10 */
#ifndef CFILEHUB_H
#define CFILEHUB_H

#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <stdio.h>
#include <iostream>
#include <string>

XERCES_CPP_NAMESPACE_USE
using namespace std;

/*
 -----------------------------------------------------------------------------------------------------------------------
 *  Interface for other classes to use for loading and saving
 -----------------------------------------------------------------------------------------------------------------------
 */
class cLoadSave : public DefaultHandler
{
public:
	virtual void Save (FILE * hFile, int nIndent) {/* Do nothing */}
	virtual void LoadStart (char const * szChunkName) {/* Do nothing */}
	virtual void LoadEnd (char const * szChunkName) {/* Do nothing */}
};

/*
 -----------------------------------------------------------------------------------------------------------------------
 *  File load and save class
 -----------------------------------------------------------------------------------------------------------------------
 */

class cFileHub
{
private:
	cLoadSave * * apFileRoutine;
	char * * aszIdentifier;
	int nFileRoutines;
	int nFileRoutineAlloc;
public:
	cFileHub ();
	~cFileHub ();

	void AddFileRoutine (char const * szIdentifier, cLoadSave * psRoutine);
	void RemoveFileRoutine (char const * szIdentifier);
	void Load (char const * szFilename);
	void Save (char const * szFilename);
	cLoadSave * FindFileRoutine (char const * szIdentifier);
};

/*
 -----------------------------------------------------------------------------------------------------------------------
 *  Xerces file load handler
 -----------------------------------------------------------------------------------------------------------------------
 */

class cFileHubLoadHandler : public DefaultHandler
{
private:
  int nLoadLevel;
  string sLoadText;
	cFileHub * psFileHub;
	int nStartLevel;
	int nChunkLevel;
	cLoadSave * psChunkRoutine;
	string sChunkRoutine;

public:
	cFileHubLoadHandler (cFileHub * psFileHub);
	~cFileHubLoadHandler ();

	void startElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname,
					   const Attributes&  attrs);
	void characters (const XMLCh *const chars, const unsigned int length);
	void fatalError (const SAXParseException& );
	void endElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname);
};

#endif
