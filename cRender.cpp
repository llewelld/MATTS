/*$T MATTS/cRender.cpp GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cRender.cpp ;
 * Last Modified: 20/12/05 ;
 * ;
 * Purpose: Render the topology ;
 */
#include "cTopology.h"
#include "cRender.h"
#include "cOptions.h"
#include "math.h"

#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glut.h>

static GLfloat const ambientMaterial[] = { 0.2f, 0.2f, 0.2f, 1.0f };
static GLfloat const diffuseMaterial[] = { 0.9f, 0.9f, 0.9f, 1.0f };
static GLfloat const mat_specular[] = { 0.3f, 0.5f, 0.1f, 1.0f };

static GLfloat const MapAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
static GLfloat const MapDiffuse[] = { 0.0f, 0.0f, 0.0f, 1.0f };
static GLfloat const MapSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static GLfloat const MapShininess = 100.0f;

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */

cRender::cRender () {
	this->psTexture = NULL;
	this->psObject = NULL;
	this->psTopology = NULL;
	this->nNodeTextureStart = 0;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cRender::cRender (cTexture* psTexture, cObject* psObject, cTopology * psTopology) {
	this->psTexture = psTexture;
	this->psObject = psObject;
	this->psTopology = psTopology;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cRender::~cRender () {

	/* Do nothing... yet */
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRender::Resize (int nHeight, int nWidth) {
	this->nHeight = nHeight;
	this->nWidth = nWidth;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRender::GetCentre (float* pfX, float* pfY, float* pfZ) {

	/* Do nothing */
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRender::SetCentre (float fX, float fY, float fZ) {

	/* Do nothing */
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRender::GetView (float* pfTheta, float* pfPhi, float* pfPsi, float* pfRadius) {

	/* Do nothing */
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRender::ViewOffset (float* pfTheta, float* pfPhi, float* pfPsi, float* pfRadius) {

	/* Do nothing */
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRender::CentreOffset (float* pfX, float* pfY, float* pfZ) {

	/* Do nothing */
}


/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRender::ChangeView (float fTheta, float fPhi, float fPsi, float fRadius) {

	/* Do nothing */
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRender::ConvertCoords (int nXMousePos, int nYMousePos, float fZPos, double* pfX, double* pfY, double* pfZ) {

	/* Do nothing */
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cRender::PointTowards (float fXPos, float fYPos, float fZPos, float fRadius) {

	/* Do nothing */
	return FALSE;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRender::PlotMain (cTopologyList*	pList, cTopologyNode*  psSelectedNode, cTopologyLink*  psSelectedLink,
						float fTimeStep) {

	/* Do nothing */
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
SelectObj cRender::SelectMain (cTopologyList* pList, int nXPos, int nYPos) {
	SelectObj sObjectsSelected;

	sObjectsSelected.nNode = -1;
	sObjectsSelected.nLink = -1;
	sObjectsSelected.fZPos = 0.0f;

	return sObjectsSelected;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
SelectObj cRender::SelectMap (int nXPos, int nYPos) {
	SelectObj sObjectsSelected;

	sObjectsSelected.nNode = -1;
	sObjectsSelected.nLink = -1;
	sObjectsSelected.fZPos = 0.0f;

	return sObjectsSelected;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRender::DrawNode (SHAPE eShape, int nType) {
	glPushMatrix ();

	glBindTexture (GL_TEXTURE_2D, psTexture->GetTexture ((TEXNAME)(nNodeTextureStart + nType)));

	switch (eShape) {
	default:
	case SHAPE_CUBE:
		glutSolidCube (1.0);
		break;
	case SHAPE_SPHERE:
		psObject->CallList (OBJECT_SPHERE);
		break;
	case SHAPE_CONE:
		glutSolidCone (1.0f, 1.0f, 10, 10);
		break;
	case SHAPE_TORUS:
		glutSolidTorus (0.5f, 1.0f, 10, 10);
		break;
	case SHAPE_TEAPOT:
		glutSolidTeapot (1.0f);
		break;
	}

	glPopMatrix ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cRender::ChangeRenderType (cTopologyList*	pList, RENDERTYPE eRenderTypePrevious, float gfViewRadius,
								float gfRotation, float gfElevation, float gfXCentre, float gfYCentre, float gfZCentre) {
	return TRUE;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRender::Update (cTopologyList* pList, unsigned long uTime) {
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cRender::PrepareForNextRenderType () {
	return TRUE;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRender::BillboardBegin () {
	float aModelview[16];
	int nXPos;
	int nYPos;

	// Save the modelview matrix
	glPushMatrix();

	// Get the modelview matrix
	glGetFloatv(GL_MODELVIEW_MATRIX , aModelview);

	// Undo rotations
	for (nXPos = 0; nXPos < 3; nXPos++) {
		for (nYPos = 0; nYPos < 3; nYPos++) {
			if (nXPos == nYPos) {
				aModelview[nXPos * 4 + nYPos] = 1.0;
			}
			else {
				aModelview[nXPos * 4 + nYPos] = 0.0;
			}
		}
	}

	// Set the new modelview matrix
	glLoadMatrixf(aModelview);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRender::BillboardEnd () {
	// Restore previous modelview matrix
	glPopMatrix();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRender::SetNodeTextureStart (int nNodeTextureStart) {
	this->nNodeTextureStart = nNodeTextureStart;
}