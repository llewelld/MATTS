/*$T MATTS/cRenderNet.h GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cRenderNet.h ;
 * Last Modified: 20/01/05 ;
 * ;
 * Purpose: Header file for cRenderNet ;
 */
#ifndef CRENDERNET_H
#define CRENDERNET_H

#include "cRender.h"
#include "cRenderNet.h"
#include "cTopologyLink.h"
#include "cTopologyList.h"
#include "cModel.h"
#include "cTimer.h"

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

class cRenderNet : public cRender
{
private:
	bool boScreenShotTemp;
	float gfX;
	float gfY;
	float gfZ;
	float gfXn;
	float gfYn;
	float gfZn;
	float gfViewRadius;
	float gfRotation;
	float gfElevation;
	float gfXCentre;
	float gfYCentre;
	float gfZCentre;
	cModel **  apsModels;
	float fRadioPulse;
	GLuint*	 aunTextureName;
	GLdouble fXDragArrowTo;
	GLdouble fYDragArrowTo;
	GLdouble fZDragArrowTo;
	GLuint guaSelectBuffer[SELBUFSIZE];
	float fChangeViewCrossFade;

	float fArrangeResistance;
	float fArrangeForce;
	float fArrangeLinklen;
	float fArrangeRigidity;
	float fArrangeCentring;

	void SetUpView ();
	void RenderStJohn ();
	void RenderLiverpool ();
	void RenderNodeHighlighted (float fXPos, float fYPos, float fZPos, float fXRot, float fYRot, float fZRot,
								float fXScale, float fYScale, float fZScale, int nCol, SHAPE eShape, int nType);
	void RenderNode (float fXPos, float fYPos, float fZPos, float fXRot, float fYRot, float fZRot, float fXScale,
					 float fYScale, float fZScale, int nCol, SHAPE eShape, int nType);
	void RenderNodeInfo (cTopologyNode* psNode);
	void RenderArrowChannels (float fXFrom, float fFrom, float fZFrom, float fXTo, float fYTo, float fZTo,
							  int nChannelOut, int nChannelIn);
	void RenderArrow (float fXFrom, float fYFrom, float fZFrom, float fXTo, float fYTo, float fZTo);
	void RenderArrowHighlighted (float fXFrom, float fYFrom, float fZFrom, float fXTo, float fYTo, float fZTo);
	void RenderArrowDragging (float fXFrom, float fYFrom, float fZFrom, float fXTo, float fYTo, float fZTo, bool boStuck);
	void RenderNodeRange (float fsXPos, float fYPos, float fZPos, float fRange, float fRadioPulse);
	void RenderTextInfo (cTopologyNode* psNode, cTopologyLink* psLink);
	void RenderLinkInfo (cTopologyLink* psLink);
	void RenderBitmapString (float fX, float fY, void* pFont, char* szString);
	void WindowPos2f (GLfloat fX, GLfloat fY);
	void RearrangeNetwork (cTopologyList* psList);
	void ApplyExpulsion (cTopologyNode* psNode, cTopologyList* psList);
	void ApplyLinkForce (cTopologyNode* psNode, cTopologyList* psList);
	void ApplyCentring (cTopologyNode* psNode, cTopologyList* psList);
	void SetRenderNodes (cTopologyList* psList);
public:
	bool boGhostLinkStuck;
	GLdouble gafModel[16];
	GLdouble gafProjection[16];
	GLint ganViewPort[4];
	bool boShowGhostLink;

	cRenderNet (cTexture * psTexture, cObject * psObject, cTopology * psTopology);
	~ cRenderNet ();
	void RenderSelectMap ();
	void PlotMain (cTopologyList* pList, cTopologyNode* psSelectedNode, cTopologyLink* psSelectedLink, float fTimeStep);
	void SetGhostLinkEnd (double fX, double fY, double fZ, bool boStick);
	SelectObj SelectMain (cTopologyList* pList, int nXPos, int nYPos);
	SelectObj SelectMap (int nXPos, int nYPos);
	void GetView (float* pfTheta, float* pfPhi, float* pfPsi, float* pfRadius);
	void ChangeView (float fTheta, float fPhi, float fPsi, float fRadius);
	void ViewOffset (float* pfTheta, float* pfPhi, float* pfPsi, float* pfRadius);
	void CentreOffset (float* pfX, float* pfY, float* pfZ);
	bool PointTowards (float fXPos, float fYPos, float fZPos, float fRadius);
	void ConvertCoords (int nXMousePos, int nYMousePos, float fZPos, double* pfX, double* pfY, double* pfZ);
	bool ChangeRenderType (cTopologyList*  pList, RENDERTYPE eRenderTypePrevious, float gfViewRadius, float gfRotation,
						   float gfElevation, float gfXCentre, float gfYCentre, float gfZCentre);
	void Update (cTopologyList* pList, unsigned long uTime);
	bool PrepareForNextRenderType ();
};

#endif /* CRENDERNET_H */
