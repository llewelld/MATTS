/*$T MATTS/cRender.h GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cRender.h ;
 * Last Modified: 20/01/05 ;
 * ;
 * Purpose: Header file for cRender ;
 */
#ifndef CRENDER_H
#define CRENDER_H

#include "cTopologyLink.h"
#include "cTopologyList.h"
#include "cModel.h"
#include "cTimer.h"
#include "cTexture.h"
#include "cObject.h"

#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glut.h>

#define MGL_SCALE	(1.0f)
#define MGL_WIDTH	(0.0f)
#define MGL_HEIGHT	(0.0f)
#define MGL_DEPTH	(0.0f)
#define SELBUFSIZE	(512)
#define RANGE(VAR, MIN, MAX) \
	if (VAR < (MIN)) \
		VAR = (MIN); \
	if (VAR > (MAX)) \
		VAR = (MAX);

typedef enum _SCENE
{
	SCENE_INVALID	= -1,
	SCENE_TRON,
	SCENE_JMUMAP,
	SCENE_STJOHN,
	SCENE_LIVERPOOL,
	SCENE_NUM
} SCENE;

typedef enum
{
	RENDERTYPE_INVALID	= -1,
	RENDERTYPE_GEO,
	RENDERTYPE_NET,
	RENDERTYPE_NUM
} RENDERTYPE;

typedef struct _SelectObj
{
	int nNode;
	int nLink;
	GLfloat fZPos;
} SelectObj;

class cTopology;

class cRender
{
protected:
	int nHeight;
	int nWidth;
	cTexture * psTexture;
	cObject * psObject;
	cTopology * psTopology;
	int nNodeTextureStart;

	void BillboardBegin ();
	void BillboardEnd ();
public:
	bool boOverlay;

	cRender ();
	cRender (cTexture * psTexture, cObject * psObject, cTopology * psTopology);
	~ cRender ();
	void SetNodeTextureStart (int nNodeTextureStart);
	void Resize (int nHeight, int nWidth);
	void DrawNode (SHAPE eShape, int nType);
	virtual void GetCentre (float* pfX, float* pfY, float* pfZ);
	virtual void SetCentre (float fX, float fY, float fZ);
	virtual void GetView (float* pfTheta, float* pfPhi, float* pfPsi, float* pfRadius);
	virtual void ViewOffset (float* pfTheta, float* pfPhi, float* pfPsi, float* pfRadius);
	virtual void ChangeView (float fTheta, float fPhi, float fPsi, float fRadius);
	virtual void CentreOffset (float* pfX, float* pfY, float* pfZ);
	virtual void ConvertCoords (int nXMousePos, int nYMousePos, float fZPos, double* pfX, double* pfY, double* pfZ);
	virtual bool PointTowards (float fXPos, float fYPos, float fZPos, float fRadius);
	virtual void PlotMain (cTopologyList*  pList, cTopologyNode*  psSelectedNode, cTopologyLink*  psSelectedLink,
						   float fTimeStep);
	virtual SelectObj SelectMain (cTopologyList* pList, int nXPos, int nYPos);
	virtual SelectObj SelectMap (int nXPos, int nYPos);
	virtual bool ChangeRenderType (cTopologyList*  pList, RENDERTYPE eRenderTypePrevious, float gfViewRadius,
								   float gfRotation, float gfElevation, float gfXCentre, float gfYCentre, float gfZCentre);
	virtual void Update (cTopologyList* pList, unsigned long uTime);
	virtual bool PrepareForNextRenderType ();
};

#endif /* CRENDER_H */
