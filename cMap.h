/*$T MATTS/cMap.h GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cMap.h ;
 * Last Modified: 20/01/05 ;
 * ;
 * Purpose: Header file for cMap ;
 */
#ifndef CMAP_H
#define CMAP_H

#include "cTopology.h"
#include "cRender.h"
#include "functy/function.h"
#include "cFileHub.h"

#include <iostream>
#include <string>

#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/Attributes.hpp>

XERCES_CPP_NAMESPACE_USE
using namespace std;

#define MAPHEIGHT_SCALE		(-0.3f)
#define MAP_HALFSIZE_SMALL	(8.0f)
#define MAP_HALFSIZE_LOW	(15.0f)
#define MAP_HALFSIZE_HIGH	(20.0f)
#define MAP_REPEAT			(5.0f)
#define MAP_SEGMENTS		(64)
#define MAP_HEIGHT(X, Y)	(0.0)
#define MAP_NORMAL_X(X, Y)	(0.0)
#define MAP_NORMAL_Y(X, Y)	(0.0)
#define MAPHEIGHT_SIZE		(5)
#define MAX_LEVELS (4)

class cMapLoadHandler;

typedef enum {
  FILESAVETAG_INVALID = -1,

  FILESAVETAG_SETTINGS,
  FILESAVETAG_RANGE,
  FILESAVETAG_FUNCTION,

  FILESAVETAG_SHOWAXES,
  FILESAVETAG_SPIN,
  FILESAVETAG_INVERT,
  FILESAVETAG_WIREFRAME,
  FILESAVETAG_FULLSCREEN,

  FILESAVETAG_RADIUS,
  FILESAVETAG_ROTATION,
  FILESAVETAG_ELEVATION,

  FILESAVETAG_MIN,
  FILESAVETAG_WIDTH,
  FILESAVETAG_COORD,

  FILESAVETAG_EQUATION,
  FILESAVETAG_RED,
  FILESAVETAG_GREEN,
  FILESAVETAG_BLUE,
  FILESAVETAG_ALPHA,
  FILESAVETAG_ACCURACY,

  FILESAVETAG_TEXFILE,
  FILESAVETAG_TEXXSCALE,
  FILESAVETAG_TEXYSCALE,
  FILESAVETAG_TEXXOFFSET,
  FILESAVETAG_TEXYOFFSET,

	FILESAVETAG_CENTREX,
  FILESAVETAG_CENTREY,
  FILESAVETAG_CENTREZ,

  FILESAVETAG_OTHER,

  FILESAVETAG_NUM
} FILESAVETAG;

class cMap
{
private:
	cTexture* psTexture;
	cObject* psObject;
	float afMapHalfSize[SCENE_NUM];
	TEXNAME aunSceneMapTexture[SCENE_NUM];
	FuncPersist ** apsFuncData;
	int nFunctionNum;
	int nFunctionMax;
  double fXMin;
  double fYMin;
  double fZMin;
  double fXWidth;
  double fYWidth;
  double fZWidth;
	bool boRangeChange;
	double fFunctionTime;
	cMapLoadHandler *  psHandler;

	void RenderMapInit (SCENE eScene);
	void DrawGraph (FuncPersist * psFuncData);
	void RecentreGraph (FuncPersist * psFuncData);
	void DrawGraphs ();

public:
	cMap (cTexture* psTexture, cObject* psObject);
	~cMap ();

	FuncPersist * AddFunction (FUNCTYPE eType, double fAccuracy);
	void TransferFunctionRange (FuncPersist * psFuncData);
	void GetVisRange (double * afRange);
	void SetVisRange (double * afRange);
	void Update (unsigned long uTime);
	void RenderMap (SCENE eScene);
	void RenderSelectMap (SCENE eScene);
	void LoadFile (char const * szFilename);
	double GetFunctionZ (double fX, double fY);
	void RemoveAllFunctions ();
	FuncPersist ** GetFunctionList ();
	int GetFunctionNum ();
	cMapLoadHandler * GetLoadHandler ();
};

class cMapLoadHandler : public cLoadSave
{
private:
	FILESAVETAG eLoadTag[MAX_LEVELS];
  int nLoadLevel;
  string sLoadText;
  FuncPersist * psLoadFuncData;
	cMap * psMap;

	void StartElementSettings (FILESAVETAG eTag, const Attributes & attrs);
	void StartElementRange (FILESAVETAG eTag, const Attributes & attrs);
	void StartElementFunction (FILESAVETAG eTag, const Attributes & attrs);
	void EndElementFunction (FILESAVETAG eTag);

public:
	cMapLoadHandler (cMap * psMap);
	~cMapLoadHandler ();

	void Save (FILE * hFile, int nIndent);
	void LoadStart (char const * szChunkName);
	void LoadEnd (char const * szChunkName);

	void startElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname,
					   const Attributes&  attrs);
	void characters (const XMLCh *const chars, const unsigned int length);
	void fatalError (const SAXParseException& );
	void endElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname);
};

#endif /* CMAP_H */
