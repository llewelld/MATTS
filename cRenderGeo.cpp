/*$T MATTS/cRenderGeo.cpp GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cRenderGeo.cpp ;
 * Last Modified: 20/12/05 ;
 * ;
 * Purpose: Render the topology geographically ;
 */
#define _USE_MATH_DEFINES

#include "cTopology.h"
#include "cRenderGeo.h"
#include "cOptions.h"
#include "math.h"
#include "cTexture.h"
#include "cMap.h"

#define BOX_WIDTH			(80)
#define BOX_HEIGHT			(60)
#define BOX_BORDER			(2)
#define ARROW_WIDTH			(6)
#define ARROW_HEADLENGTH	(16)
#define ARROW_HEADWIDTH		(14)
#define ARROW_PLOTGAP		(8)
#define ARROW_SHRINKFROM	(5)
#define ARROW_SHRINKTO		(5)
#define ARROW_SHRINKFROMHL	(30)
#define ARROW_SHRINKTOHL	(0)
#define LINE_SMALL			(0.1f)
#define LINE_CLOSE			(8.0f)
#define ARROW_SHRINKFROMCH	(60)
#define ARROW_SHRINKTOCH	(60)
#define ARROW_SHIFT			(7)
#define ARROWHEAD_STUCK		(ARROWHEAD_HEIGHT + NODE_SIZE)
#define INFOTEXT_TOP		(174)
#define INFOTEXT_LEFT		(40)
#define INFOTEXT_LINE		(16)
#define INFOTEXT_FONT		(GLUT_BITMAP_8_BY_13)
#define INFOTEXT_COLOUR		1.0, 0.2, 0.2
#define NODETEXT_FONT		(GLUT_BITMAP_HELVETICA_10)
#define NODETEXT_FONTSCREENSHOT1 (GLUT_BITMAP_HELVETICA_18)
#define NODETEXT_FONTSCREENSHOT2 (GLUT_BITMAP_HELVETICA_12)
#define NODETEXT_LINE		(12)
#define NODETEXT_LINESCREENSHOT1		(18)
#define NODETEXT_LINESCREENSHOT2		(12)
#define NODETEXT_XOFF		(20)
#define NODETEXT_YOFF		(20)
#define NODETEXT_COLOUR		0.3, 1.0, 0.3
#define LINKTEXT_FONT		(GLUT_BITMAP_HELVETICA_10)
#define LINKTEXT_LINE		(12)
#define LINKTEXT_XOFF		(-4)
#define LINKTEXT_YOFF		(4)
#define LINKTEXT_COLOUR		0.5, 1.0, 0.5

#define ICON_XOFFSET (17)
#define ICON_YOFFSET (27 - 16)
#define ICON_XSIZE (16)
#define ICON_YSIZE (16)
#define ICON_XSHIFT (18)
#define ICON_YSHIFT (0)

typedef enum _MODEL
{
	MODEL_INVALID	= -1,
	MODEL_LIVERPOOL,
	MODEL_ARENA,
	MODEL_PORT,
	MODEL_NUM
} MODEL;

static GLfloat const ambientMaterial[] = { 0.2f, 0.2f, 0.2f, 1.0f };
static GLfloat const diffuseMaterial[] = { 0.9f, 0.9f, 0.9f, 1.0f };
static GLfloat const mat_specular[] = { 0.3f, 0.5f, 0.1f, 1.0f };

static char const *const gszModels[] = { "StJohns", "Arena", "Port" };

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */

cRenderGeo::cRenderGeo (cTexture* psTexture, cObject* psObject, cTopology * psTopology) {
	int nModel;

	this->psTexture = psTexture;
	this->psObject = psObject;
	this->psTopology = psTopology;
	boScreenShotTemp = false;
	gfX = 0.0f;
	gfY = 0.0f;
	gfZ = -1.0f;
	gfXn = 0.0f;
	gfYn = 1.0f;
	gfZn = 0.0f;
	gfXCentre = 0.0f;
	gfYCentre = 0.0f;
	gfZCentre = 0.0f;
	gfRotation = (3.0f * 3.14159265 / 2.0f);
	gfElevation = (3.14159265 / 2.0f);
	gfViewRadius = 6.2f;
	boShowGhostLink = FALSE;
	boGhostLinkStuck = FALSE;
	fRadioPulse = 0.0f;
	boOverlay = true;
	fChangeViewCrossFade = 0.0f;
	psMap = new cMap (psTexture, psObject);

	/* Load in the models for the various scenes */
	apsModels = (cModel**) malloc (sizeof (cModel *) * (int) MODEL_NUM);
	for (nModel = 0; nModel < MODEL_NUM; nModel++) {
		apsModels[nModel] = new cModel (gszModels[nModel]);
	}

	eScene = SCENE_JMUMAP;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cRenderGeo::~cRenderGeo () {
	int nModel;

	/* Destroy all of the models */
	apsModels = (cModel**) malloc (sizeof (cModel *) * (int) MODEL_NUM);
	for (nModel = 0; nModel < (int) MODEL_NUM; nModel++) {
		if (apsModels[nModel]) {
			delete apsModels[nModel];
		}
	}

	if (psMap) {
		delete psMap;
		psMap = NULL;
	}

	free (apsModels);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cMapLoadHandler * cRenderGeo::GetLoadHandler () {
	return psMap->GetLoadHandler ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRenderGeo::PlotMain (cTopologyList*  pList, cTopologyNode*  psSelectedNode, cTopologyLink*  psSelectedLink,
							float fTimeStep) {
	cTopologyLink*	pLinks;
	float fPulse;
	int nCol;
	cTopologyNode*	psIterator;
	double fZScreen;
	float fFogStart;
	float fFogEnd;

	cOptions*  pOptions;
	pOptions = cOptions::create ();
	boScreenShotTemp = pOptions->getScreenShotMode ();

	glLoadIdentity ();

	glEnable (GL_NORMALIZE);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	SetUpView ();

	/*
	 * glPushMatrix ();
	 */
	glGetDoublev (GL_MODELVIEW_MATRIX, gafModel);
	glGetDoublev (GL_PROJECTION_MATRIX, gafProjection);
	glGetIntegerv (GL_VIEWPORT, ganViewPort);

	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	glColorMaterial (GL_FRONT, GL_DIFFUSE);
	glEnable (GL_COLOR_MATERIAL);

	/* Render appropriate static models in the scene */
	switch (eScene) {
	case SCENE_LIVERPOOL:

		/* Render some Liverpool landmarks */
		RenderLiverpool ();
		break;
	case SCENE_STJOHN:

		/* Render the St John Centre model */
		RenderStJohn ();
		break;
	case SCENE_TRON:
	case SCENE_JMUMAP:
	default:

		/* No models to render */
		break;
	}

	glMaterialfv (GL_FRONT, GL_AMBIENT, ambientMaterial);
	glMaterialfv (GL_FRONT, GL_DIFFUSE, diffuseMaterial);
	glMaterialfv (GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialf (GL_FRONT, GL_SHININESS, 15.0f);

	glEnable (GL_TEXTURE_2D);
	glColorMaterial (GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	/*
	 * glPopMatrix ();
	 * Plot a box for each VM
	 */
	if (pList) {
		psIterator = pList->startIterator ();

		while (pList->isIteratorValid (psIterator)) {
			fZScreen = psMap->GetFunctionZ (pList->getXPos (psIterator), pList->getYPos (psIterator));
			pList->getCurrentNode (psIterator)->fZPos = - (Z_SIZE / 2.0) - fZScreen;

			if (psSelectedNode == pList->getCurrentNode (psIterator)) {
				RenderNodeHighlighted (pList->getXPos (psIterator), pList->getYPos (psIterator),
										pList->getZPos (psIterator), pList->getXRot (psIterator),
										pList->getYRot (psIterator), pList->getZRot (psIterator),
										pList->getXScale (psIterator), pList->getYScale (psIterator),
										pList->getZScale (psIterator), pList->getCol (psIterator),
										(SHAPE) (pList->getShape (psIterator)), (pList->getType (psIterator)));
			} else {
				nCol = pList->getAnalyseCol (psIterator);
				if (nCol == 0) {
					nCol = pList->getCol (psIterator);
				}

				RenderNode (pList->getXPos (psIterator), pList->getYPos (psIterator), pList->getZPos (psIterator),
							pList->getXRot (psIterator), pList->getYRot (psIterator), pList->getZRot (psIterator),
							pList->getXScale (psIterator), pList->getYScale (psIterator), pList->getZScale (psIterator),
							nCol, (SHAPE) (pList->getShape (psIterator)), (pList->getType (psIterator)));
			}

			if (boOverlay) {
				RenderNodeInfo (pList->getCurrentNode (psIterator));
			}

			psIterator = pList->moveForward (psIterator);
		}
	}

	glDisable (GL_TEXTURE_2D);
	glDisable (GL_COLOR_MATERIAL);

	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	glColorMaterial (GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glEnable (GL_COLOR_MATERIAL);
	glMaterialfv (GL_FRONT, GL_AMBIENT, ambientMaterial);
	glMaterialfv (GL_FRONT, GL_DIFFUSE, diffuseMaterial);
	glMaterialfv (GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialf (GL_FRONT, GL_SHININESS, 15.0f);

	/* Plot all of the links between the VMs */
	psIterator = pList->startIterator ();

	while (pList->isIteratorValid (psIterator)) {
		pLinks = pList->getLinksOut (psIterator);
		while (pLinks) {
			if (pLinks == psSelectedLink) {
				RenderArrowHighlighted (pList->getXPos (psIterator), pList->getYPos (psIterator),
										pList->getZPos (psIterator), pLinks->psLinkTo->fXPos, pLinks->psLinkTo->fYPos,
										pLinks->psLinkTo->fZPos);
				RenderArrowChannels (pList->getXPos (psIterator), pList->getYPos (psIterator),
										pList->getZPos (psIterator), pLinks->psLinkTo->fXPos, pLinks->psLinkTo->fYPos,
										pLinks->psLinkTo->fZPos, pLinks->nChannelOut, pLinks->nChannelIn);
			} else {
				RenderArrow (pList->getXPos (psIterator), pList->getYPos (psIterator), pList->getZPos (psIterator),
										pLinks->psLinkTo->fXPos, pLinks->psLinkTo->fYPos, pLinks->psLinkTo->fZPos);
			}

			if (boOverlay) {
				RenderLinkInfo (pLinks);
			}

			pLinks = pLinks->pFromNext;
		}

		psIterator = pList->moveForward (psIterator);
	}

	if (boShowGhostLink) {
		fZScreen = psMap->GetFunctionZ (fXDragArrowTo, fYDragArrowTo);

		/* Drag the current temporary link */
		RenderArrowDragging (psSelectedNode->fXPos, psSelectedNode->fYPos, psSelectedNode->fZPos, fXDragArrowTo,
								fYDragArrowTo, -fZScreen, boGhostLinkStuck);
		//RenderArrowDragging (psSelectedNode->fXPos, psSelectedNode->fYPos, psSelectedNode->fZPos, fXDragArrowTo,
		//						fYDragArrowTo, fZDragArrowTo, boGhostLinkStuck);
	}

	glColorMaterial (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	glPushMatrix ();

	/* Plot the background map */
	if (fChangeViewCrossFade > 0.0f) {
		//glEnable (GL_BLEND);
		//glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glColor4f (1.0f, 1.0f, 1.0f, (1.0f + cos (fChangeViewCrossFade)) / 2.0f);
		glPushAttrib (GL_FOG_BIT);
		fFogStart = (35.0 * (0.0 + cos (fChangeViewCrossFade)));
		fFogEnd = (20.0 * (1.0 + cos (fChangeViewCrossFade)));
		RANGE (fFogStart, 0.0, 35.0);
		RANGE (fFogEnd, 0.0, 40.0);
		glFogf (GL_FOG_END, fFogEnd);
		glFogf (GL_FOG_START, fFogStart);
		glTranslatef (0.0f, 0.0f, (1.0f - cos (fChangeViewCrossFade)) * 2.0f);
		psMap->RenderMap (eScene);
		glPopAttrib ();
		//glDisable (GL_BLEND);
	} else {
		glColor4f (1.0f, 1.0f, 1.0f, 1.0f);
		psMap->RenderMap (eScene);
	}

	glPopMatrix ();


	/*
	 * Plot the radio ranges ;
	 * Plot a box for each VM
	 */
	fRadioPulse += fTimeStep * 0.0003f;
	if (fRadioPulse > 1.0f) {
		fRadioPulse = 0.0f;
	}

	glDisable (GL_LIGHTING);
	glEnable (GL_TEXTURE_2D);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask (GL_FALSE);
	glDisable (GL_DEPTH_TEST);

	if (pList) {
		psIterator = pList->startIterator ();

		while (pList->isIteratorValid (psIterator)) {
			if (pList->getCurrentNode (psIterator)->boAdHoc) {
				fPulse = fRadioPulse + ((pList->getCurrentNode (psIterator)->getID () % 10) / 10.0f);
				if (fPulse > 1.0f) {
					fPulse -= 1.0f;
				}

				RenderNodeRange (pList->getXPos (psIterator), pList->getYPos (psIterator), pList->getZPos (psIterator),
								 pList->getCurrentNode (psIterator)->fRangeTx, 0.75f + (0.25f * fPulse));
			}

			psIterator = pList->moveForward (psIterator);
		}
	}

	glEnable (GL_DEPTH_TEST);
	glDisable (GL_BLEND);
	glDepthMask (GL_TRUE);
	glDisable (GL_TEXTURE_2D);
	glEnable (GL_LIGHTING);

	/* Plot some text */
	//RenderTextInfo (psSelectedNode, psSelectedLink);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRenderGeo::SetUpView (void) {
	/*
	 * static float gfViewTheta = 0.0f;
	 * static float gfViewPhi = 0.0f;
	 * static float gfViewRadius = 5.0f;
	 */
	glLoadIdentity ();

	/*
	 * gluLookAt (gfViewRadius * sin (gfViewTheta), 0.0, gfViewRadius * cos
	 * (gfViewTheta), 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	 * gluLookAt (gfViewRadius * sin (gfViewTheta) * cos (gfViewPhi), gfViewRadius sin
	 * (gfViewPhi), gfViewRadius * cos (gfViewTheta) * cos (gfViewPhi), 0.0, 0.0, 0.0,
	 * 0.0f, 1.0f, 0.0f);
	 * gluLookAt (gfViewRadius * cos (gfViewTheta) * cos (gfViewPhi), gfViewRadius sin
	 * (gfViewPhi), gfViewRadius * sin (gfViewTheta) * cos (gfViewPhi), 0.0, 0.0, 0.0,
	 * -cos (gfViewTheta) * sin (gfViewPhi), cos (gfViewPhi), -sin (gfViewTheta) sin (gfViewPhi));
	 * gluLookAt (gfViewRadius * cos (gfViewTheta) * cos (gfViewPhi), gfViewRadius sin
	 * (gfViewPhi), gfViewRadius * sin (gfViewTheta) * cos (gfViewPhi), 0.0, 0.0, 0.0,
	 * 0.0f, 1.0f, 0.0f);
	 * ;
	 * gluLookAt (-gfViewRadius * cos (gfViewPhi) * sin (gfViewTheta), -gfViewRadius
	 * sin (gfViewPhi), gfViewRadius * cos (gfViewPhi) * cos (gfViewTheta), 0.0, 0.0,
	 * 0.0, -sin (gfViewPsi) * cos (gfViewTheta) - cos (gfViewPsi) * sin (gfViewPhi)
	 * sin (gfViewTheta), cos (gfViewPsi) * cos (gfViewPhi), -sin (gfViewPsi) * sin
	 * (gfViewTheta) + cos (gfViewPsi) * sin (gfViewPhi) * cos (gfViewTheta));
	 */
	gluLookAt (gfXCentre + (gfViewRadius * gfX), gfYCentre + (gfViewRadius * gfY), gfZCentre + (gfViewRadius * gfZ), gfXCentre, gfYCentre, gfZCentre, gfXn, gfYn, gfZn);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRenderGeo::SetScene (SCENE eScene) {
	this->eScene = eScene;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRenderGeo::RenderStJohn (void) {
	float fX;
	float fY;
	float fZ;

	glColor3f (1.0f, 1.0f, 1.0f);
	glPushMatrix ();
	fX = (-MGL_WIDTH / 2.0f + 572.0f) / MGL_SCALE;
	fY = (MGL_HEIGHT / 2.0f - 341.0f) / MGL_SCALE;
	fZ = (-MGL_DEPTH / 2.0f - 0.01f) / MGL_SCALE;

	glTranslatef (fX, fY, fZ);
	glRotatef (270.0f, 0.0, 0.0f, 1.0);
	glScalef (0.000426f, 0.000426f, 0.000426f);
	glRotatef (180.0f, 1.0, 0.0f, 0.0);
	apsModels[MODEL_LIVERPOOL]->Render ();
	glPopMatrix ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRenderGeo::RenderLiverpool (void) {
	float fX;
	float fY;
	float fZ;

	glColor3f (1.0f, 1.0f, 1.0f);
	glPushMatrix ();

	fX = (-MGL_WIDTH / 2.0f - 000.0f) / MGL_SCALE;
	fY = (MGL_HEIGHT / 2.0f - 300.0f) / MGL_SCALE;
	fZ = (-MGL_DEPTH / 2.0f - 0.0f) / MGL_SCALE;

	glTranslatef (fX, fY, fZ);
	glScalef (0.000015f, 0.000015f, 0.000015f);
	glRotatef (180.0f, 1.0f, 0.0f, 0.0f);
	apsModels[MODEL_PORT]->Render ();
	glPopMatrix ();
	glPushMatrix ();

	fX = (-MGL_WIDTH / 2.0f + 600.0f) / MGL_SCALE;
	fY = (MGL_HEIGHT / 2.0f - 200.0f) / MGL_SCALE;
	fZ = (-MGL_DEPTH / 2.0f - 0.0f) / MGL_SCALE;

	glTranslatef (fX, fY, fZ);
	glScalef (0.009f, 0.009f, 0.009f);

	glRotatef (110.0f, 0.0f, 0.0f, 1.0f);
	glRotatef (180.0f, 1.0f, 0.0f, 0.0f);
	apsModels[MODEL_ARENA]->Render ();
	glPopMatrix ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRenderGeo::RenderNodeHighlighted (float fXPos, float fYPos, float fZPos, float fXRot, float fYRot, float fZRot,
										float fXScale, float fYScale, float fZScale, int nCol, SHAPE eShape, int nType) {
	glColor3f (2.0, 0.3 * (((float) GetGValue (nCol)) / 255.0f), 0.3 * (((float) GetBValue (nCol)) / 255.0f));

	glPushMatrix ();

	glTranslatef ((-MGL_WIDTH / 2.0f + fXPos) / MGL_SCALE, (MGL_HEIGHT / 2.0f - fYPos) / MGL_SCALE,
				  (-MGL_DEPTH / 2.0f + fZPos) / MGL_SCALE);

	BillboardBegin ();

	glRotatef (fZRot / ROT_SCALE, 0.0, 0.0, 1.0);
	glRotatef (fYRot / ROT_SCALE, 0.0, 1.0, 0.0);
	glRotatef (fXRot / ROT_SCALE, 1.0, 0.0, 0.0);

	glScalef (fXScale / SCALE_SCALE, fYScale / SCALE_SCALE, fZScale / SCALE_SCALE);
	glRotatef (270.0f, 1.0, 0.0, 0.0);

	DrawNode (eShape, nType);

	BillboardEnd ();

	glPopMatrix ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRenderGeo::RenderNode (float fXPos, float fYPos, float fZPos, float fXRot, float fYRot, float fZRot, float fXScale,
							 float fYScale, float fZScale, int nCol, SHAPE eShape, int nType) {

	/*
	 * glColor3f (1.0, 1.0, 1.0);
	 */
	if (boScreenShotTemp) {
		glColor3f (((float) GetRValue (nCol)) / 155.0f, ((float) GetGValue (nCol)) / 155.0f,
				   ((float) GetBValue (nCol)) / 155.0f);
	} else {
		glColor3f (((float) GetRValue (nCol)) / 255.0f, ((float) GetGValue (nCol)) / 255.0f,
				   ((float) GetBValue (nCol)) / 255.0f);
	}

	glPushMatrix ();

	glTranslatef ((-MGL_WIDTH / 2.0f + fXPos) / MGL_SCALE, (MGL_HEIGHT / 2.0f - fYPos) / MGL_SCALE,
				  (-MGL_DEPTH / 2.0f + fZPos) / MGL_SCALE);

	BillboardBegin ();

	glRotatef (fZRot / ROT_SCALE, 0.0, 0.0, 1.0);
	glRotatef (fYRot / ROT_SCALE, 0.0, 1.0, 0.0);
	glRotatef (fXRot / ROT_SCALE, 1.0, 0.0, 0.0);

	glScalef (fXScale / SCALE_SCALE, fYScale / SCALE_SCALE, fZScale / SCALE_SCALE);
	glRotatef (270.0f, 1.0, 0.0, 0.0);

	DrawNode (eShape, nType);

	BillboardEnd ();

	glPopMatrix ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRenderGeo::RenderNodeInfo (cTopologyNode* psNode) {
	GLdouble afModel[16];
	GLdouble afProjection[16];
	GLint anViewpoert[4];
	GLdouble fX;
	GLdouble fY;
	GLdouble fZ;
	GLdouble fXn;
	GLdouble fYn;
	GLdouble fZn;
	char szText[1024];
	float fAspectRatio;
	GLdouble fXicon;
	GLdouble fYicon;
	GLdouble fIconHeight;

	if (nHeight <= 0) {
		nHeight = 1;
	}
	fAspectRatio = (float) nWidth / (float) nHeight;

	if (psNode) {
		glGetDoublev (GL_MODELVIEW_MATRIX, afModel);
		glGetDoublev (GL_PROJECTION_MATRIX, afProjection);
		glGetIntegerv (GL_VIEWPORT, anViewpoert);

		fXn = (-MGL_WIDTH / 2.0f + psNode->fXPos) / MGL_SCALE;
		fYn = (MGL_HEIGHT / 2.0f - psNode->fYPos) / MGL_SCALE;
		fZn = (-MGL_DEPTH / 2.0f + psNode->fZPos) / MGL_SCALE;

		gluProject (fXn, fYn, fZn, afModel, afProjection, anViewpoert, &fX, &fY, &fZ);

		glDisable (GL_LIGHTING);
		glColor3f (NODETEXT_COLOUR);
		fX += NODETEXT_XOFF;
		fY += NODETEXT_YOFF;

		if (!boScreenShotTemp) {
			_snprintf (szText, sizeof (szText), "%s node %d", psNode->GetName (), psNode->getID ());
			RenderBitmapString ((float) fX, (float) fY, NODETEXT_FONT, szText);
			fY -= NODETEXT_LINE;

			if (psNode->szCertProps[0] != 0) {
				_snprintf (szText, sizeof (szText), "Prop: %s", psNode->szCertProps);
				RenderBitmapString ((float) fX, (float) fY, NODETEXT_FONT, szText);
				fY -= NODETEXT_LINE;
			}
		} else {
			_snprintf (szText, sizeof (szText), "%s", psNode->GetName (), (psNode->getID () + 1));
			RenderBitmapString ((float) fX, (float) fY, NODETEXT_FONTSCREENSHOT1, szText);
			fY -= NODETEXT_LINESCREENSHOT1;

			_snprintf (szText, sizeof (szText), "         %s", psNode->PropertyGetOption ("CommandLevel"));
			//_snprintf (szText, sizeof (szText), "Auth: %d", psNode->nSensitivityLevel);
			RenderBitmapString ((float) fX, (float) fY, NODETEXT_FONTSCREENSHOT2, szText);
			fY -= NODETEXT_LINESCREENSHOT2;


			/* Set up an orthogonal view to overlay the existing 3D model */
			glMatrixMode (GL_PROJECTION);
			glPushMatrix ();
			glLoadIdentity ();
			glOrtho (0, nWidth, 0, nHeight, -100, 100);
			glScalef (1, -1, 1);
			glTranslatef (0.0, (GLfloat)-nHeight, 0.0);
			glMatrixMode (GL_MODELVIEW);
			glPushMatrix ();
			glLoadIdentity ();

			glEnable (GL_BLEND);
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable (GL_TEXTURE_2D);
			glBindTexture (GL_TEXTURE_2D, psTexture->GetTexture (TEXNAME_LEVEL));
			glColor4f (1.0, 1.0, 1.0, 1.0);

			fXicon = fX + ICON_XOFFSET;
			fYicon = nHeight - fY - ICON_YOFFSET;

			fIconHeight = 1.0f - ((psNode->PropertyGetInt ("Authorisation") + 1) / 10.0f);
			if (fIconHeight > 1.0f) {
				fIconHeight = 1.0f;
			}
			if (fIconHeight < 0.0f) {
				fIconHeight = 0.0f;
			}
			glBegin (GL_QUADS);
			glTexCoord2f (1, 0);
			glVertex3i (fXicon, fYicon - (ICON_YSIZE * fIconHeight), 1);
			glTexCoord2f (0, 0);
			glVertex3i (fXicon - ICON_XSIZE, fYicon - (ICON_YSIZE * fIconHeight), 1);
			glTexCoord2f (0, fIconHeight);
			glVertex3i (fXicon - ICON_XSIZE, fYicon, 1);
			glTexCoord2f (1, fIconHeight);
			glVertex3i (fXicon, fYicon, 1);
			glEnd ();

			fXicon += ICON_XSHIFT;
			fYicon += ICON_YSHIFT;

			fIconHeight = 1.0f - ((psNode->PropertyGetInt ("Sensitivity") + 1) / 10.0f);
			if (fIconHeight > 1.0f) {
				fIconHeight = 1.0f;
			}
			if (fIconHeight < 0.0f) {
				fIconHeight = 0.0f;
			}
			glBegin (GL_QUADS);
			glTexCoord2f (1, 0);
			glVertex3i (fXicon, fYicon - (ICON_YSIZE * fIconHeight), 1);
			glTexCoord2f (0, 0);
			glVertex3i (fXicon - ICON_XSIZE, fYicon - (ICON_YSIZE * fIconHeight), 1);
			glTexCoord2f (0, fIconHeight);
			glVertex3i (fXicon - ICON_XSIZE, fYicon, 1);
			glTexCoord2f (1, fIconHeight);
			glVertex3i (fXicon, fYicon, 1);
			glEnd ();

			/* Switch back to the original view */
			glMatrixMode (GL_PROJECTION);
			glPopMatrix ();
			glMatrixMode (GL_MODELVIEW);
			glPopMatrix ();

			glEnable (GL_DEPTH_TEST);
			//glDepthMask (GL_TRUE);
		}

		glEnable (GL_LIGHTING);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRenderGeo::RenderArrowChannels (float fXFrom, float fYFrom, float fZFrom, float fXTo, float fYTo, float fZTo,
									  int nChannelOut, int nChannelIn) {

	/* Don't plot anything */
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRenderGeo::RenderArrow (float fXFrom, float fYFrom, float fZFrom, float fXTo, float fYTo, float fZTo) {
	float fLength;
	float fAngle;

	float fXMid;
	float fYMid;
	float fZMid;

	fXMid = (fXTo + fXFrom) / 2.0f;
	fYMid = (fYTo + fYFrom) / 2.0f;
	fZMid = (fZTo + fZFrom) / 2.0f;
	fLength = sqrt (((((fXFrom - fXTo) * (fXFrom - fXTo))) + (((fYFrom - fYTo) * (fYFrom - fYTo))) +
						(((fZFrom - fZTo) * (fZFrom - fZTo)))));

	/*
	 * if (fLength < (float)(ARROW_HEADLENGTH + ARROW_SHRINKFROM + ARROW_SHRINKTO)) ;
	 * { ;
	 * fLength = (float)(ARROW_HEADLENGTH + ARROW_SHRINKFROM + ARROW_SHRINKTO);
	 * }
	 */
	if (boScreenShotTemp) {
		glColor4f (0.0, 2.0, 2.0, 1.0);
	} else {
		glColor4f (0.0, 1.0, 1.0, 1.0);
	}

	glPushMatrix ();

	glTranslatef ((-MGL_WIDTH / 2.0f + fXMid) / MGL_SCALE, (MGL_HEIGHT / 2.0f - fYMid) / MGL_SCALE,
				  (-MGL_DEPTH / 2.0f + fZMid) / MGL_SCALE);

	fAngle = -360.0f * acos ((fXTo - fXFrom) / sqrt ((((fXTo - fXFrom) * (fXTo - fXFrom)) + ((fZTo - fZFrom) * (fZTo - fZFrom))))) / (2.0f * 3.1415926538);
	if (fZTo < fZFrom) {
		fAngle = -fAngle;
	}

	glRotatef (fAngle, 0.0, 1.0, 0.0);

	fAngle = -360.0f * acos ((((fXTo - fXFrom) * (fXTo - fXFrom)) + ((fZTo - fZFrom) * (fZTo - fZFrom))) / sqrt (
								 ((((fXTo - fXFrom) * (fXTo - fXFrom)) + ((fZTo - fZFrom) * (fZTo - fZFrom)))) * ((
								 ((fXTo - fXFrom) * (fXTo - fXFrom)) + ((fYTo - fYFrom) * (fYTo - fYFrom)) +
								 ((fZTo - fZFrom) * (fZTo - fZFrom)))))) / (2.0f * 3.1415926538);
	if (fYTo < fYFrom) {
		fAngle = -fAngle;
	}

	glRotatef (fAngle, 0.0, 0.0, 1.0);

	/*
	 * fAngle = -360.0f * atan2 ((float)(nYTo - nYFrom), (float)(nXTo - nXFrom)) (2.0f 3.1415926538);
	 * glRotatef (fAngle, 0.0, 0.0, 1.0);
	 */
	glTranslatef ((fLength / (2.0f * MGL_SCALE)) - ARROWHEAD_STUCK, 0.0, 0.0);
	glRotatef (90.0f, 0.0, 1.0, 0.0);
	psObject->CallList (OBJECT_ARROWHEAD);
	glRotatef (-90.0f, 0.0, 1.0, 0.0);
	glTranslatef (-(fLength / (2.0f * MGL_SCALE)) + ARROWHEAD_STUCK, 0.0, 0.0);

	glTranslatef (-ARROWHEAD_HEIGHT, 0.0, 0.0);

	glScalef ((fLength / MGL_SCALE) - ARROWHEAD_HEIGHT, 0.07, 0.07);

	glTranslatef (-0.5, 0.0, 0.0);
	glRotatef (90.0f, 0.0, 1.0, 0.0);

	/*
	 * glutSolidCone (0.3, 1.0, 10, 10);
	 */
	psObject->CallList (OBJECT_CYLINDER);

	glPopMatrix ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRenderGeo::RenderArrowHighlighted (float fXFrom, float fYFrom, float fZFrom, float fXTo, float fYTo, float fZTo) {
	float fLength;
	float fAngle;

	float fXMid;
	float fYMid;
	float fZMid;

	fXMid = (fXTo + fXFrom) / 2.0f;
	fYMid = (fYTo + fYFrom) / 2.0f;
	fZMid = (fZTo + fZFrom) / 2.0f;
	fLength = sqrt (((((fXFrom - fXTo) * (fXFrom - fXTo))) + (((fYFrom - fYTo) * (fYFrom - fYTo))) +
						(((fZFrom - fZTo) * (fZFrom - fZTo)))));

	/*
	 * if (fLength < (float)(ARROW_HEADLENGTH + ARROW_SHRINKFROM + ARROW_SHRINKTO)) ;
	 * { ;
	 * fLength = (float)(ARROW_HEADLENGTH + ARROW_SHRINKFROM + ARROW_SHRINKTO);
	 * }
	 */
	fAngle = -360.0f * atan2 ((fYTo - fYFrom), (fXTo - fXFrom)) / (2.0f * 3.1415926538);

	glColor3f (2.0, 0.0, 0.0);

	glPushMatrix ();

	glTranslatef ((-MGL_WIDTH / 2.0f + fXMid) / MGL_SCALE, (MGL_HEIGHT / 2.0f - fYMid) / MGL_SCALE,
				  (-MGL_DEPTH / 2.0f + fZMid) / MGL_SCALE);

	fAngle = -360.0f * acos ((fXTo - fXFrom) / sqrt ((((fXTo - fXFrom) * (fXTo - fXFrom)) + ((fZTo - fZFrom) * (fZTo - fZFrom))))) / (2.0f * 3.1415926538);
	if (fZTo < fZFrom) {
		fAngle = -fAngle;
	}

	glRotatef (fAngle, 0.0, 1.0, 0.0);

	fAngle = -360.0f * acos ((((fXTo - fXFrom) * (fXTo - fXFrom)) + ((fZTo - fZFrom) * (fZTo - fZFrom))) / sqrt (
								 ((((fXTo - fXFrom) * (fXTo - fXFrom)) + ((fZTo - fZFrom) * (fZTo - fZFrom)))) * ((
								 ((fXTo - fXFrom) * (fXTo - fXFrom)) + ((fYTo - fYFrom) * (fYTo - fYFrom)) +
								 ((fZTo - fZFrom) * (fZTo - fZFrom)))))) / (2.0f * 3.1415926538);
	if (fYTo < fYFrom) {
		fAngle = -fAngle;
	}

	glRotatef (fAngle, 0.0, 0.0, 1.0);

	glTranslatef ((fLength / (2.0f * MGL_SCALE)) - ARROWHEAD_STUCK, 0.0, 0.0);
	glRotatef (90.0f, 0.0, 1.0, 0.0);
	psObject->CallList (OBJECT_ARROWHEAD);
	glRotatef (-90.0f, 0.0, 1.0, 0.0);
	glTranslatef (-(fLength / (2.0f * MGL_SCALE)) + ARROWHEAD_STUCK, 0.0, 0.0);

	glTranslatef (-ARROWHEAD_HEIGHT, 0.0, 0.0);

	glScalef ((fLength / MGL_SCALE) - ARROWHEAD_HEIGHT, 0.07, 0.07);

	glTranslatef (-0.5, 0.0, 0.0);
	glRotatef (90.0f, 0.0, 1.0, 0.0);

	/*
	 * glutSolidCone (0.3, 1.0, 10, 10);
	 */
	psObject->CallList (OBJECT_CYLINDER);

	glPopMatrix ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRenderGeo::RenderArrowDragging (float fXFrom, float fYFrom, float fZFrom, float fXTo, float fYTo, float fZTo,
									  bool boStuck) {
	float fLength;
	float fAngle;

	float fXMid;
	float fYMid;
	float fZMid;

	fXMid = (fXTo + fXFrom) / 2.0f;
	fYMid = (fYTo + fYFrom) / 2.0f;
	fZMid = (fZTo + fZFrom) / 2.0f;
	fLength = sqrt (((((fXFrom - fXTo) * (fXFrom - fXTo))) + (((fYFrom - fYTo) * (fYFrom - fYTo))) +
						(((fZFrom - fZTo) * (fZFrom - fZTo)))));

	/*
	 * if (fLength < (float)(ARROW_HEADLENGTH + ARROW_SHRINKFROM + ARROW_SHRINKTO)) ;
	 * { ;
	 * fLength = (float)(ARROW_HEADLENGTH + ARROW_SHRINKFROM + ARROW_SHRINKTO);
	 * }
	 */
	fAngle = -360.0f * atan2 ((fYTo - fYFrom), (fXTo - fXFrom)) / (2.0f * 3.1415926538);

	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor4f (1.5, 0.2, 0.2, 0.6);

	glPushMatrix ();

	glTranslatef ((-MGL_WIDTH / 2.0f + fXMid) / MGL_SCALE, (MGL_HEIGHT / 2.0f - fYMid) / MGL_SCALE,
				  (-MGL_DEPTH / 2.0f + fZMid) / MGL_SCALE);

	fAngle = -360.0f * acos ((fXTo - fXFrom) / sqrt ((((fXTo - fXFrom) * (fXTo - fXFrom)) + ((fZTo - fZFrom) * (fZTo - fZFrom))))) / (2.0f * 3.1415926538);
	if (fZTo < fZFrom) {
		fAngle = -fAngle;
	}

	glRotatef (fAngle, 0.0, 1.0, 0.0);

	fAngle = -360.0f * acos ((((fXTo - fXFrom) * (fXTo - fXFrom)) + ((fZTo - fZFrom) * (fZTo - fZFrom))) / sqrt (
								 ((((fXTo - fXFrom) * (fXTo - fXFrom)) + ((fZTo - fZFrom) * (fZTo - fZFrom)))) * ((
								 ((fXTo - fXFrom) * (fXTo - fXFrom)) + ((fYTo - fYFrom) * (fYTo - fYFrom)) +
								 ((fZTo - fZFrom) * (fZTo - fZFrom)))))) / (2.0f * 3.1415926538);
	if (fYTo < fYFrom) {
		fAngle = -fAngle;
	}

	glRotatef (fAngle, 0.0, 0.0, 1.0);

	if (boStuck) {
		glTranslatef ((fLength / (2.0f * MGL_SCALE)) - ARROWHEAD_STUCK, 0.0, 0.0);
	} else {
		glTranslatef ((fLength / (2.0f * MGL_SCALE)) - ARROWHEAD_HEIGHT, 0.0, 0.0);
	}

	glRotatef (90.0f, 0.0, 1.0, 0.0);
	psObject->CallList (OBJECT_ARROWHEAD);
	glRotatef (-90.0f, 0.0, 1.0, 0.0);
	glTranslatef (-(fLength / (2.0f * MGL_SCALE)) + ARROWHEAD_STUCK, 0.0, 0.0);

	glTranslatef (-ARROWHEAD_HEIGHT, 0.0, 0.0);

	/*
	 * glScalef ((fLength / MGL_SCALE) - 0.25f, 0.07, 0.07);
	 */
	glScalef ((fLength / MGL_SCALE) - ARROWHEAD_HEIGHT, 0.07, 0.07);

	glTranslatef (-0.5, 0.0, 0.0);
	glRotatef (90.0f, 0.0, 1.0, 0.0);

	/*
	 * glutSolidCone (0.3, 1.0, 10, 10);
	 */
	psObject->CallList (OBJECT_CYLINDER);

	glPopMatrix ();

	glDisable (GL_BLEND);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRenderGeo::RenderNodeRange (float fXPos, float fYPos, float fZPos, float fRange, float fRadioPulse) {
	if (fRange > 0.0f) {
		glPushMatrix ();

		glTranslatef ((-MGL_WIDTH / 2.0f + fXPos) / MGL_SCALE, (MGL_HEIGHT / 2.0f - fYPos) / MGL_SCALE,
					  (-MGL_DEPTH / 2.0f + fZPos) / MGL_SCALE);

		glScalef (fRadioPulse * fRange * 2.0f, fRadioPulse * fRange * 2.0f, 1.0f);
		glColor4f (0.0, 0.3, 1.0, 1.0f * (1.0f - fRadioPulse));
		glBindTexture (GL_TEXTURE_2D, psTexture->GetTexture (TEXNAME_RADIORANGE2));
		psObject->CallList (OBJECT_RADIORANGE);

		/*
		 * glutSolidSphere (1.0f, 30, 5);
		 */
		glColor4f (0.0, 0.3, 1., 0.25);
		glBindTexture (GL_TEXTURE_2D, psTexture->GetTexture (TEXNAME_RADIORANGE1));
		psObject->CallList (OBJECT_RADIORANGE);

		glPopMatrix ();
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRenderGeo::RenderTextInfo (cTopologyNode* psNode, cTopologyLink* psLink) {

	/* Render the info text in the corner */
	int nTextY;
	char szText[1024];

	glDisable (GL_LIGHTING);
	glColor3f (INFOTEXT_COLOUR);
	nTextY = nHeight - INFOTEXT_TOP;

	_snprintf (szText, sizeof (szText), "Time: %u", psTopology->GetTimer ()->GetTime ());
	RenderBitmapString (INFOTEXT_LEFT, nTextY, INFOTEXT_FONT, szText);
	nTextY -= INFOTEXT_LINE;

	if (psNode) {
		_snprintf (szText, sizeof (szText), "%s node %d", psNode->GetName (), psNode->getID ());
		RenderBitmapString (INFOTEXT_LEFT, nTextY, INFOTEXT_FONT, szText);
		nTextY -= INFOTEXT_LINE;
		if (psNode->szCertProps[0] != 0) {
			_snprintf (szText, sizeof (szText), "Properties: %s", psNode->szCertProps);
			RenderBitmapString (INFOTEXT_LEFT, nTextY, INFOTEXT_FONT, szText);
			nTextY -= INFOTEXT_LINE;
		}

		nTextY -= INFOTEXT_LINE;
	}

	if (psLink) {
		//_snprintf (szText, sizeof (szText), "Link %d to %d", psLink->psLinkFrom->getID (), psLink->psLinkTo->getID ());
		//RenderBitmapString (INFOTEXT_LEFT, nTextY, INFOTEXT_FONT, szText);
		//nTextY -= INFOTEXT_LINE;
		_snprintf (szText, sizeof (szText), "Channels %d to %d", psLink->nChannelIn, psLink->nChannelOut);
		RenderBitmapString (INFOTEXT_LEFT, nTextY, INFOTEXT_FONT, szText);
		nTextY -= INFOTEXT_LINE;
		if (psLink->szLinkOutput[0] != 0) {
			_snprintf (szText, sizeof (szText), "Output properties: %s", psLink->szLinkOutput);
			RenderBitmapString (INFOTEXT_LEFT, nTextY, INFOTEXT_FONT, szText);
			nTextY -= INFOTEXT_LINE;
		}

		if (psLink->szLinkInput[0] != 0) {
			_snprintf (szText, sizeof (szText), "Input properties: %s", psLink->szLinkInput);
			RenderBitmapString (INFOTEXT_LEFT, nTextY, INFOTEXT_FONT, szText);
			nTextY -= INFOTEXT_LINE;
		}

		//_snprintf (szText, sizeof (szText), "Sensitivity Level %d", psLink->nSensitivityLevel);
		//RenderBitmapString (INFOTEXT_LEFT, nTextY, INFOTEXT_FONT, szText);
		//nTextY -= INFOTEXT_LINE;
	}

	glEnable (GL_LIGHTING);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRenderGeo::RenderLinkInfo (cTopologyLink* psLink) {
	GLdouble afModel[16];
	GLdouble afProjection[16];
	GLint anViewpoert[4];
	GLdouble fX;
	GLdouble fY;
	GLdouble fZ;
	GLdouble fXn;
	GLdouble fYn;
	GLdouble fZn;
	char szText[1024];

	if ((psLink) && (!boScreenShotTemp)) {
		glGetDoublev (GL_MODELVIEW_MATRIX, afModel);
		glGetDoublev (GL_PROJECTION_MATRIX, afProjection);
		glGetIntegerv (GL_VIEWPORT, anViewpoert);

		fXn = (psLink->psLinkFrom->fXPos + psLink->psLinkTo->fXPos) / 2.0f;
		fYn = (psLink->psLinkFrom->fYPos + psLink->psLinkTo->fYPos) / 2.0f;
		fZn = (psLink->psLinkFrom->fZPos + psLink->psLinkTo->fZPos) / 2.0f;

		fXn = (-MGL_WIDTH / 2.0f + (float) fXn) / MGL_SCALE;
		fYn = (MGL_HEIGHT / 2.0f - (float) fYn) / MGL_SCALE;
		fZn = (-MGL_DEPTH / 2.0f + (float) fZn) / MGL_SCALE;

		gluProject (fXn, fYn, fZn, afModel, afProjection, anViewpoert, &fX, &fY, &fZ);

		glDisable (GL_LIGHTING);
		glColor3f (LINKTEXT_COLOUR);
		fX += LINKTEXT_XOFF;
		fY += LINKTEXT_YOFF;

		//_snprintf (szText, sizeof (szText), "Link %d to %d", psLink->psLinkFrom->getID (), psLink->psLinkTo->getID ());
		//RenderBitmapString ((float) fX, (float) fY, LINKTEXT_FONT, szText);
		//fY -= LINKTEXT_LINE;

		_snprintf (szText, sizeof (szText), "Chan %d to %d", psLink->nChannelIn, psLink->nChannelOut);
		RenderBitmapString ((float) fX, (float) fY, LINKTEXT_FONT, szText);
		fY -= LINKTEXT_LINE;

		if (psLink->szLinkOutput[0] != 0) {
			_snprintf (szText, sizeof (szText), "Out prop: %s", psLink->szLinkOutput);
			RenderBitmapString ((float) fX, (float) fY, LINKTEXT_FONT, szText);
			fY -= LINKTEXT_LINE;
		}

		if (psLink->szLinkInput[0] != 0) {
			_snprintf (szText, sizeof (szText), "In prop: %s", psLink->szLinkInput);
			RenderBitmapString ((float) fX, (float) fY, LINKTEXT_FONT, szText);
			fY -= LINKTEXT_LINE;
		}

		glEnable (GL_LIGHTING);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRenderGeo::SetGhostLinkEnd (double fX, double fY, double fZ, bool boStick) {
	fXDragArrowTo = fX;
	fYDragArrowTo = fY;
	fZDragArrowTo = fZ;
	boGhostLinkStuck = boStick;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRenderGeo::RenderBitmapString (float fX, float fY, void* pFont, char* szString) {
	int nPos;

	WindowPos2f (fX, fY);
	nPos = 0;
	while (szString[nPos] >= 32) {
		glutBitmapCharacter (pFont, szString[nPos]);
		nPos++;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRenderGeo::WindowPos2f (GLfloat fX, GLfloat fY) {
	GLfloat fXn;

	GLfloat fYn;

	glPushAttrib (GL_TRANSFORM_BIT | GL_VIEWPORT_BIT);
	glMatrixMode (GL_PROJECTION);
	glPushMatrix ();
	glLoadIdentity ();
	glMatrixMode (GL_MODELVIEW);
	glPushMatrix ();
	glLoadIdentity ();

	glDepthRange (0, 0);
	glViewport ((int) fX - 1, (int) fY - 1, 2, 2);
	fXn = fX - (int) fX;
	fYn = fY - (int) fY;
	glRasterPos4f (fXn, fYn, 0.0, 1);
	glPopMatrix ();
	glMatrixMode (GL_PROJECTION);
	glPopMatrix ();

	glPopAttrib ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
SelectObj cRenderGeo::SelectMain (cTopologyList* pList, int nXPos, int nYPos) {
	cTopologyLink*	pLinks;

	int nHits;
	HDC sHDC;
	int nCount;
	int nBufferPos;
	int nNamesNum;
	SelectObj sObjectsSelected;
	unsigned int uNodeOrder;
	unsigned int uLinkOrder;
	float fAspectRatio;
	GLfloat fZPos;
	cTopologyNode*	psIterator;

	sHDC = 0;

	glSelectBuffer (SELBUFSIZE, guaSelectBuffer);
	glRenderMode (GL_SELECT);

	glMatrixMode (GL_PROJECTION);
	glPushMatrix ();
	glLoadIdentity ();

	/*
	 * glGetIntegerv (GL_VIEWPORT, anViewport);
	 */
	gluPickMatrix (nXPos, ganViewPort[3] - nYPos, 1, 1, ganViewPort);

	/*
	 * glViewport (0, 0, (GLsizei)(nWidth), (GLsizei)(nHeight));
	 */
	if (nHeight <= 0) {
		nHeight = 1;
	}

	fAspectRatio = (float) nWidth / (float) nHeight;
	gluPerspective (30.0, fAspectRatio, 1.5, 250.0);

	glMatrixMode (GL_MODELVIEW);
	glInitNames ();

	/*
	 * glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	 */
	SetUpView ();

	glPushName (OBJECTTYPE_NODE);
	nCount = 0;

	/* Plot a box for each VM */
	if (pList) {
		psIterator = pList->startIterator ();

		while (pList->isIteratorValid (psIterator)) {
			glPushName ((unsigned int) pList->getCurrentID (psIterator));
			RenderNode (pList->getXPos (psIterator), pList->getYPos (psIterator), pList->getZPos (psIterator),
						pList->getXRot (psIterator), pList->getYRot (psIterator), pList->getZRot (psIterator),
						pList->getXScale (psIterator), pList->getYScale (psIterator), pList->getZScale (psIterator),
						pList->getCol (psIterator), (SHAPE) (pList->getShape (psIterator)),
						(pList->getType (psIterator)));
			psIterator = pList->moveForward (psIterator);
			glPopName ();
			nCount++;
		}
	}

	glPopName ();

	/* Plot all of the links between the VMs */
	glPushName (OBJECTTYPE_LINK);
	nCount = 0;
	psIterator = pList->startIterator ();

	while (pList->isIteratorValid (psIterator)) {
		pLinks = pList->getLinksOut (psIterator);
		while (pLinks) {
			glPushName ((unsigned int)pLinks->nLinkID);
			RenderArrow (pList->getXPos (psIterator), pList->getYPos (psIterator), pList->getZPos (psIterator),
						 pLinks->psLinkTo->fXPos, pLinks->psLinkTo->fYPos, pLinks->psLinkTo->fZPos);
			pLinks = pLinks->pFromNext;
			glPopName ();
			nCount++;
		}

		psIterator = pList->moveForward (psIterator);
	}

	glPopName ();

	glMatrixMode (GL_PROJECTION);
	glPopMatrix ();
	glMatrixMode (GL_MODELVIEW);
	glFlush ();

	/* returning to normal rendering mode */
	nHits = glRenderMode (GL_RENDER);

	sObjectsSelected.nNode = -1;
	sObjectsSelected.nLink = -1;
	sObjectsSelected.fZPos = 1.0f;
	uNodeOrder = 0;
	uLinkOrder = 0;

	if (nHits != 0) {
		nBufferPos = 0;
		for (nCount = 0; nCount < nHits; nCount++) {
			nNamesNum = guaSelectBuffer[nBufferPos];
			nBufferPos += 3;

			if (guaSelectBuffer[nBufferPos] == OBJECTTYPE_NODE) {
				if ((guaSelectBuffer[nBufferPos - 1] >= uNodeOrder) && (nNamesNum > 1)) {
					sObjectsSelected.nNode = guaSelectBuffer[nBufferPos + 1];
					uNodeOrder = guaSelectBuffer[nBufferPos - 1];
				}
			}

			if (guaSelectBuffer[nBufferPos] == OBJECTTYPE_LINK) {
				if ((guaSelectBuffer[nBufferPos - 1] >= uLinkOrder) && (nNamesNum > 1)) {
					sObjectsSelected.nLink = guaSelectBuffer[nBufferPos + 1];
					uLinkOrder = guaSelectBuffer[nBufferPos - 1];
				}
			}

			nBufferPos += nNamesNum;
		}

		glReadPixels (nXPos, ganViewPort[3] - nYPos, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &fZPos);
		sObjectsSelected.fZPos = fZPos;
	}

	return sObjectsSelected;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
SelectObj cRenderGeo::SelectMap (int nXPos, int nYPos) {

	/*
	 * int nHits;
	 */
	SelectObj sObjectsSelected;
	float fAspectRatio;
	GLfloat fZPos;

	/*
	 * glSelectBuffer (SELBUFSIZE, guaSelectBuffer);
	 * glRenderMode (GL_SELECT);
	 */
	glMatrixMode (GL_PROJECTION);
	glPushMatrix ();
	glLoadIdentity ();

	/* Create a small viewport for the selection */
	glViewport (nXPos, ganViewPort[3] - nYPos, 1, 1);

	/*
	 * glGetIntegerv (GL_VIEWPORT, anViewport);
	 */
	gluPickMatrix (nXPos, ganViewPort[3] - nYPos, 1, 1, ganViewPort);

	if (nHeight <= 0) {
		nHeight = 1;
	}

	fAspectRatio = (float) nWidth / (float) nHeight;
	gluPerspective (30.0, fAspectRatio, 1.5, 250.0);

	glMatrixMode (GL_MODELVIEW);

	glClear (GL_DEPTH_BUFFER_BIT);

	SetUpView ();

	/*
	 * glPushName (OBJECTTYPE_MAP);
	 */
	psMap->RenderSelectMap (eScene);

	/*
	 * glPopName ();
	 */
	glMatrixMode (GL_PROJECTION);
	glPopMatrix ();
	glMatrixMode (GL_MODELVIEW);
	glFlush ();

	/*/
	 *  Returning to normal rendering mode ;
	 *  nHits = glRenderMode (GL_RENDER);
	 */
	/*
	 * sObjectsSelected.nNode = -1;
	 * sObjectsSelected.nLink = -1;
	 * sObjectsSelected.fZPos = 0.7f;
	 * ;
	 * if (nHits != 0) {
	 */
	glReadPixels (nXPos, ganViewPort[3] - nYPos, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &fZPos);
	sObjectsSelected.fZPos = fZPos;

	/*
	 * } ;
	 * Restore the original viewport
	 */
	glViewport (0, 0, (GLsizei) (nWidth), (GLsizei) (nHeight));

	return sObjectsSelected;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRenderGeo::GetView (float* pfTheta, float* pfPhi, float* pfPsi, float* pfRadius) {
	if (pfTheta) {
		*pfTheta = gfRotation;
	}

	if (pfPhi) {
		*pfPhi = gfElevation;
	}

	/* We don't currently use pfPsi */
	if (pfPsi) {
		*pfPsi = 0.0f;
	}

	if (pfRadius) {
		*pfRadius = gfViewRadius;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRenderGeo::ViewOffset (float* pfTheta, float* pfPhi, float* pfPsi, float* pfRadius) {
	if (pfTheta) {
		*pfTheta -= gfRotation;
	}

	if (pfPhi) {
		*pfPhi -= gfElevation;
	}

	/* We don't currently use pfPsi */
	if (pfRadius) {
		*pfRadius -= gfViewRadius;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRenderGeo::ChangeView (float fTheta, float fPhi, float fPsi, float fRadius) {
	gfViewRadius += fRadius;

	RANGE (gfViewRadius, 4.1, 30.5)

	/* Rotation */
	gfRotation += fTheta;

	/* Elevation */
	gfElevation += fPhi;
	if (gfElevation < (3.14159265 / 20.0f))
		gfElevation = (3.14159265 / 20.0f);
	if (gfElevation > (3.14159265 / 2.0f))
		gfElevation = (3.14159265 / 2.0f);

	/* Calculate camera position */
	gfX = cos (gfElevation) * cos (gfRotation);
	gfY = cos (gfElevation) * sin (gfRotation);
	gfZ = -sin (gfElevation);

	/* Calculate camera up direction */
	gfXn = cos ((3.14159265 / 2.0f) - gfElevation) * cos ((3.14159265) + gfRotation);
	gfYn = cos ((3.14159265 / 2.0f) - gfElevation) * sin ((3.14159265) + gfRotation);
	gfZn = -sin ((3.14159265 / 2.0f) - gfElevation);

	/*
	 * Normalise vectors (they should already be, but we make sure to avoid ;
	 * cumulative rounding errors) ;
	 * Normalise (& gfX, & gfY, & gfZ);
	 * Normalise (& gfXn, & gfYn, & gfZn);
	 */
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRenderGeo::CentreOffset (float* pfX, float* pfY, float* pfZ) {
	if (pfX) {
		*pfX-= gfXCentre;
	}

	if (pfY) {
		*pfY-= gfYCentre;
	}

	if (pfZ) {
		*pfZ-= gfZCentre;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRenderGeo::GetCentre (float* pfX, float* pfY, float* pfZ) {
	if (pfX) {
		*pfX = gfXCentre;
	}
	if (pfY) {
		*pfY = gfYCentre;
	}
	if (pfZ) {
		*pfZ = gfZCentre;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRenderGeo::SetCentre (float fX, float fY, float fZ) {
	gfXCentre = fX;
	gfYCentre = fY;
	gfZCentre = fZ;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRenderGeo::ConvertCoords (int nXMousePos, int nYMousePos, float fZPos, double* pfX, double* pfY, double* pfZ) {
	GLdouble fX;
	GLdouble fY;
	GLdouble fZ;

	gluUnProject ((GLdouble) nXMousePos, (GLdouble) (ganViewPort[3] - nYMousePos), (GLdouble) fZPos, gafModel,
				  gafProjection, ganViewPort, &fX, &fY, &fZ);
	if (pfX) {
		*pfX = (fX * MGL_SCALE) + (MGL_WIDTH / 2.0f);
	}

	if (pfY) {
		*pfY = -((fY * MGL_SCALE) - (MGL_HEIGHT / 2.0f));
	}

	if (pfZ) {
		*pfZ = ((fZ * MGL_SCALE) + (MGL_DEPTH / 2.0f));
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cRenderGeo::PointTowards (float fXPos, float fYPos, float fZPos, float fRadius) {
	float fTheta;

	/*
	 * float fPhi;
	 */
	float fPsi;

	/*
	 * float fX;
	 * float fY;
	 * float fZ;
	 */
	float fXp;
	float fYp;
	float fZp;

	/*
	 * float fXn;
	 * float fYn;
	 * float fZn;
	 */
	float fHalfLife;
	float fL;
	cOptions*  pOptions;
	bool boArrived;

	pOptions = cOptions::create ();

	fHalfLife = pOptions->getRotateHalfLife ();

	boArrived = true;

	fXp = (-MGL_WIDTH / 2.0f + fXPos) / MGL_SCALE;
	fYp = (MGL_HEIGHT / 2.0f - fYPos) / MGL_SCALE;
	fZp = (-MGL_DEPTH / 2.0f + fZPos) / MGL_SCALE;

	/*
	 * fTheta = DotProdAngle (fXp, fZp, gfX, gfZ) * fHalfLife;
	 * if (abs (fTheta) > POINTTO_MINMOVE) ;
	 * { ;
	 * fX = (gfX * cos (fTheta)) + (gfZ * sin (fTheta));
	 * fZ = - (gfX * sin (fTheta)) + (gfZ * cos (fTheta));
	 * fXn = (gfXn * cos (fTheta)) + (gfZn * sin (fTheta));
	 * fZn = - (gfXn * sin (fTheta)) + (gfZn * cos (fTheta));
	 * gfX = fX;
	 * gfZ = fZ;
	 * gfXn = fXn;
	 * gfZn = fZn;
	 * boArrived = false;
	 * } ;
	 * fPhi = DotProdAngle (fYp, fZp, gfY, gfZ) * fHalfLife;
	 * if (abs(fPhi) > POINTTO_MINMOVE) ;
	 * { ;
	 * fY = (gfY * cos (fPhi)) + (gfZ * sin (fPhi));
	 * fZ = - (gfY * sin (fPhi)) + (gfZ * cos (fPhi));
	 * fYn = (gfYn * cos (fPhi)) + (gfZn * sin (fPhi));
	 * fZn = - (gfYn * sin (fPhi)) + (gfZn * cos (fPhi));
	 * gfY = fY;
	 * gfZ = fZ;
	 * gfYn = fYn;
	 * gfZn = fZn;
	 * boArrived = false;
	 * }
	 */
	fTheta = 0.0f;
	fL = sqrt (((fXp) * (fXp)) + ((fYp) * (fYp)));
	fTheta = (3.14159265 / 20.0f) + (((3.14159265 / 2.0f) - (3.14159265 / 20.0f)) / (fL + 1.0f));
	if (fTheta < (3.14159265 / 20.0f))
		fTheta = (3.14159265 / 20.0f);
	if (fTheta > (3.14159265 / 2.0f))
		fTheta = (3.14159265 / 2.0f);
	fTheta = (fTheta - gfElevation) * fHalfLife;

	fPsi = cTopology::DotProdAngle (-fXp, -fYp, gfX, gfY) * fHalfLife;

	if ((abs (fPsi) > POINTTO_MINMOVE) || (abs (fTheta) > POINTTO_MINMOVE)) {
		ChangeView (-fPsi, fTheta, 0.0f, 0.0f);

		/*
		 * fX = (gfX * cos (fPsi)) + (gfY * sin (fPsi));
		 * fY = - (gfX * sin (fPsi)) + (gfY * cos (fPsi));
		 * fXn = (gfXn * cos (fPsi)) + (gfYn * sin (fPsi));
		 * fYn = - (gfXn * sin (fPsi)) + (gfYn * cos (fPsi));
		 * gfX = fX;
		 * gfY = fY;
		 * gfXn = fXn;
		 * gfYn = fYn;
		 */
		boArrived = false;
	}

	/*
	 * gfY = (gfY * cos (fPhi)) + (gfZ * sin (fPhi));
	 * gfZ = - (gfY * sin (fPhi)) + (gfZ * cos (fPhi));
	 * ;
	 * gfX = (gfX * cos (fPsi)) + (gfY * sin (fPsi));
	 * gfY = - (gfX * sin (fPsi)) + (gfY * cos (fPsi));
	 * ;
	 * Normalise (& gfX, & gfY, & gfZ);
	 * RedrawTopology ();
	 */
	return boArrived;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cRenderGeo::ChangeRenderType (cTopologyList*  pList, RENDERTYPE eRenderTypePrevious, float gfViewRadius,
								   float gfRotation, float gfElevation, float gfXCentre, float gfYCentre, float gfZCentre) {
	this->gfViewRadius = gfViewRadius;
	this->gfRotation = gfRotation;
	this->gfElevation = gfElevation;

	/*
	 * fChangeViewCrossFade = 0.0f;
	 */
	return TRUE;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cRenderGeo::PrepareForNextRenderType () {
	fChangeViewCrossFade += 0.2f;

	return (fChangeViewCrossFade >= M_PI);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRenderGeo::Update (cTopologyList* pList, unsigned long uTime) {
	if (fChangeViewCrossFade > 0.0f) {
		fChangeViewCrossFade -= 0.1f;
	}

	psMap->Update (uTime);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cRenderGeo::LoadFile (const char *szFilename) {
	psMap->RemoveAllFunctions ();
	psMap->LoadFile (szFilename);
}
