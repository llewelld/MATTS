/*$T MATTS/cTopology.cpp GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cTopology.cpp ;
 * Last Modified: 20/12/05 ;
 * ;
 * Purpose: Deal with network topology between agents ;
 */
#include "cTopology.h"
#include "cTopologyNode.h"
#include "cTopologyLink.h"
#include <stdio.h>
#include <windows.h>
#include <windowsx.h>
#include <Mmsystem.h>
#include "resource.h"
#include <commctrl.h>
#include "math.h"
#include "cOptions.h"
#include "cModelTexture.h"
#include "Event.h"

#include <lib3ds/camera.h>
#include <lib3ds/matrix.h>

#define Z_SCREEN								(10.0f)
#define REDRAW_GAP								(58)
#define LINKID_START							(0)
#define ANALYSIS_BLOB_COLOUR_HALFLIFE_FACTOR	(1.2f)
#define BUTTONPOS_FIRSTX						(256)
#define BUTTONPOS_CENTREDX						((nWidth - (BUTTONPOS_WIDTH * BUTTON_NUM)) / 2)
#define BUTTONPOS_FIRSTY						(20)
#define BUTTONPOS_WIDTH							(72)
#define BUTTONPOS_HEIGHT						(72)
#define BUTTON_HEIGHT							(64)
#define BUTTON_WIDTH							(64)
#define NODEBUTTONROTATE_SPEED					(0.02f)
#define BUTTON_UNSTICKTIME						(100)

static GLfloat light_position1[] = { BUTTONPOS_FIRSTX + (5 * BUTTONPOS_WIDTH), BUTTONPOS_FIRSTY, -80.0f, 1.0f };
static GLfloat const light_ambient1[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static GLfloat const light_diffuse1[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static GLfloat const light_specular1[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static GLfloat light_position0[] = { 0.5f, 0.5f, 1.0f, 0.0f };
static GLfloat const light_ambient0[] = { 0.0f, 0.0f, 0.0f, 1.0f };
static GLfloat const light_diffuse0[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static GLfloat const light_specular0[] = { 0.0f, 0.0f, 0.0f, 1.0f };
static GLfloat const LightModelAmbient[] = { 0.5f, 0.5f, 0.5f, 1.0f };

static GLfloat const ambientMaterial[] = { 0.2f, 0.2f, 0.2f, 1.0f };
static GLfloat const diffuseMaterial[] = { 0.9f, 0.9f, 0.9f, 1.0f };
static GLfloat const mat_specular[] = { 0.3f, 0.5f, 0.1f, 1.0f };

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */

cTopology::cTopology (cTopologyList* pTopologyList, HWND hwnd, cFileHub * psFileHub, cWindow * psWindow) {
	RECT sWinRect;

	this->psFileHub = psFileHub;
	this->psWindow = psWindow;
	CreateTimer ();

	uTime = psTimer->GetTime ();
	uTimePrev = uTime;
	fTimeStep = 0.0f;
	pList = pTopologyList;
	hWindow = hwnd;
	GetWindowRect (hWindow, &sWinRect);
	nHeight = sWinRect.bottom - sWinRect.top;
	nWidth = sWinRect.right - sWinRect.left;

	psTexture = new cTexture ();
	psObject = new cObject ();
	psRenderGeo = new cRenderGeo (psTexture, psObject, this);
	psRenderNet = new cRenderNet (psTexture, psObject, this);
	eRenderType = RENDERTYPE_GEO;
	psRenderCurrent = psRenderGeo;

	psSelectedNode = NULL;
	psSelectedLink = NULL;
	psPointToNode = NULL;
	boScreenShotTemp = false;
	nNextLinkID = LINKID_START;
	boAnalysisCompleted = false;
	boAnalysisResult = true;
	fAnalysisRed = 1.0f;
	fAnalysisGreen = 0.5f;

	ChangeView (0.0f, 0.0f, 0.0f, 0.0f);

	uTime = psTimer->GetTime ();
	uTimePrev = uTime;
	fTimeStep = 0.0f;

	for (int nButton = 0; nButton < BUTTON_NUM; nButton++) {
		aboButtonPressed[nButton] = false;
	}

	fNodeButtonRotate = 0.0f;
	nNodeButtonNodeType = 0;
	uNodeButtonLeftTimeClicked = 0;
	uNodeButtonRightTimeClicked = 0;

	glEnable (GL_CULL_FACE);
	glLightModeli (GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
	glEnable (GL_NORMALIZE);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cTopology::~cTopology () {
	if (pList) {
		delete pList;
		pList = NULL;
	}

	if (psTimer) {
		DeleteTimer ();
	}

	if (psRenderGeo) {
		delete psRenderGeo;
		psRenderGeo = NULL;
	}

	if (psRenderNet) {
		delete psRenderNet;
		psRenderNet = NULL;
	}

	if (psTexture) {
		delete psTexture;
		psTexture = NULL;
	}

	if (psObject) {
		delete psObject;
		psObject = NULL;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cTopology::PrepareForNextRenderType () {
	bool boChange;
	boChange = psRenderCurrent->PrepareForNextRenderType ();

	if (boChange) {
		NextRenderType ();
	}

	return boChange;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::NextRenderType () {
	RENDERTYPE eRenderTypePrevious;
	RENDERTYPE eRenderTypeNext;
	cRender*  psRenderNext;
	bool boSuccess;
	float fViewRadius;
	float fRotation;
	float fElevation;
	float fXCentre;
	float fYCentre;
	float fZCentre;

	psRenderNext = psRenderCurrent;
	eRenderTypePrevious = eRenderType;
	eRenderTypeNext = (RENDERTYPE) ((int) eRenderType + 1);
	if (eRenderTypeNext >= RENDERTYPE_NUM) {
		eRenderTypeNext = (RENDERTYPE) 0;
	}

	switch (eRenderTypeNext) {
	default:
	case RENDERTYPE_GEO:
		psRenderNext = psRenderGeo;
		psRenderCurrent->GetView (&fRotation, &fElevation, NULL, &fViewRadius);
		psRenderCurrent->GetCentre (& fXCentre, & fYCentre, & fZCentre);
		boSuccess = psRenderGeo->ChangeRenderType (pList, eRenderTypePrevious, fViewRadius, fRotation, fElevation, fXCentre, fYCentre, fZCentre);
		break;
	case RENDERTYPE_NET:
		psRenderNext = psRenderNet;
		psRenderCurrent->GetView (&fRotation, &fElevation, NULL, &fViewRadius);
		psRenderCurrent->GetCentre (& fXCentre, & fYCentre, & fZCentre);
		boSuccess = psRenderNet->ChangeRenderType (pList, eRenderTypePrevious, fViewRadius, fRotation, fElevation, fXCentre, fYCentre, fZCentre);
		break;
	}

	if (boSuccess) {
		psRenderNext->boOverlay = psRenderCurrent->boOverlay;
		eRenderType = eRenderTypeNext;
		psRenderCurrent = psRenderNext;
		psRenderCurrent->Resize (nHeight, nWidth);
		psRenderCurrent->ChangeView (0.0f, 0.0f, 0.0f, 0.0f);
	}
}

/*
 =======================================================================================================================
 *  void cTopology::RenderMap2 (SCENE eScene) Creating and rendaring the map interface ;
 *  { ;
 *  int nXSegment;
 *  int nYSegment;
 *  float fHeight;
 *  ;
 *  if (!boScreenShotTemp) { ;
 *  glColor3f (1.0f, 1.0f, 1.0f);
 *  } ;
 *  else { ;
 *  glColor3f (0.3f, 0.3f, 0.3f);
 *  } ;
 *  glBindTexture (GL_TEXTURE_2D, psTexture->GetTexture (aunSceneMapTexture[eScene]));
 *  ;
 *  //glTexCoord2f (0.0f, 0.0f);
 *  //glVertex3f (-MAP_HALFSIZE, -MAP_HALFSIZE, 0.0f);
 *  //glTexCoord2f (0.0f, MAP_REPEAT);
 *  //glVertex3f (-MAP_HALFSIZE, MAP_HALFSIZE, 0.0f);
 *  //glTexCoord2f (MAP_REPEAT, MAP_REPEAT);
 *  //glVertex3f (MAP_HALFSIZE, MAP_HALFSIZE, 0.0f);
 *  //glTexCoord2f (MAP_REPEAT, 0.0f);
 *  //glVertex3f (MAP_HALFSIZE, -MAP_HALFSIZE, 0.0f);
 *  ;
 *  for (nXSegment = 0;
 *  nXSegment < MAP_SEGMENTS;
 *  nXSegment ++) ;
 *  { ;
 *  glBegin (GL_QUAD_STRIP);
 *  for (nYSegment = 0;
 *  nYSegment < MAP_SEGMENTS;
 *  nYSegment ++) ;
 *  { ;
 *  //glTexCoord2f ((MAP_REPEAT/((float)MAP_SEGMENTS))*((float)(nXSegment + 1)),
 *  (MAP_REPEAT/((float)MAP_SEGMENTS))*((float)(nYSegment + 1)));
 *  //glVertex3f ((2.0f * ((float)(nXSegment + 1)) * MAP_HALFSIZE / ((float)MAP_SEGMENTS)) - MAP_HALFSIZE, (2.0f
 *  ((float)(nYSegment + 1)) * MAP_HALFSIZE / ((float)MAP_SEGMENTS)) - MAP_HALFSIZE, 0.0f);
 *  fHeight = MAPHEIGHT_SCALE * gfMapHeights[((nXSegment + 1) % MAPHEIGHT_SIZE)][(nYSegment % MAPHEIGHT_SIZE)];
 *  glTexCoord2f ((MAP_REPEAT/((float)MAP_SEGMENTS))*((float)(nXSegment + 1)),
 *  (MAP_REPEAT/((float)MAP_SEGMENTS))*((float)nYSegment));
 *  glVertex3f ((2.0f * ((float)(nXSegment + 1)) * afMapHalfSize[eScene] / ((float)MAP_SEGMENTS))
 *  afMapHalfSize[eScene], (2.0f * ((float)nYSegment) * afMapHalfSize[eScene] / ((float)MAP_SEGMENTS))
 *  afMapHalfSize[eScene], fHeight);
 *  ;
 *  fHeight = MAPHEIGHT_SCALE * gfMapHeights[(nXSegment % MAPHEIGHT_SIZE)][(nYSegment % MAPHEIGHT_SIZE)];
 *  glTexCoord2f ((MAP_REPEAT/((float)MAP_SEGMENTS))*((float)nXSegment),
 *  (MAP_REPEAT/((float)MAP_SEGMENTS))*((float)nYSegment));
 *  glVertex3f ((2.0f * ((float)nXSegment) * afMapHalfSize[eScene] / ((float)MAP_SEGMENTS)) - afMapHalfSize[eScene],
 *  (2.0f * ((float)nYSegment) * afMapHalfSize[eScene] / ((float)MAP_SEGMENTS)) - afMapHalfSize[eScene], fHeight);
 *  //glTexCoord2f ((MAP_REPEAT/((float)MAP_SEGMENTS))*((float)nXSegment),
 *  (MAP_REPEAT/((float)MAP_SEGMENTS))*((float)(nYSegment + 1)));
 *  //glVertex3f ((2.0f * ((float)nXSegment) * MAP_HALFSIZE / ((float)MAP_SEGMENTS)) - MAP_HALFSIZE, (2.0f
 *  ((float)(nYSegment + 1)) * MAP_HALFSIZE / ((float)MAP_SEGMENTS)) - MAP_HALFSIZE, 0.0f);
 *  } ;
 *  glEnd ();
 *  } ;
 *  }
 =======================================================================================================================
 */
void cTopology::RenderHUD () {
	float fAspectRatio;

	/* Unpress buttons that don't stick */
	if ((uNodeButtonLeftTimeClicked > 0) && (uTime > (uNodeButtonLeftTimeClicked + BUTTON_UNSTICKTIME))) {
		aboButtonPressed[BUTTON_LEFT] = false;
		uNodeButtonLeftTimeClicked = 0;
	}

	if ((uNodeButtonRightTimeClicked > 0) && (uTime > (uNodeButtonRightTimeClicked + BUTTON_UNSTICKTIME))) {
		aboButtonPressed[BUTTON_RIGHT] = false;
		uNodeButtonRightTimeClicked = 0;
	}

	/* Set up an orthogonal view to overlay the existing 3D model */
	if (nHeight <= 0) {
		nHeight = 1;
	}

	fAspectRatio = (float) nWidth / (float) nHeight;
	glMatrixMode (GL_PROJECTION);
	glPushMatrix ();
	glLoadIdentity ();
	glOrtho (0, nWidth, 0, nHeight, -100, 100);
	glScalef (1, -1, 1);
	glTranslatef (0.0, (GLfloat)-nHeight, 0.0);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	glDisable (GL_DEPTH_TEST);
	glDepthMask (GL_FALSE);
	glDisable (GL_LIGHTING);

	/*
	 * glDisable (GL_LIGHT0);
	 * glEnable (GL_LIGHT1);
	 */
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable (GL_TEXTURE_2D);

	/*
	 * Now do any rendering needed for the HUD Render the status blob ;
	 * if (boAnalysisCompleted) { ;
	 * if (boAnalysisResult) { ;
	 * fAnalysisRed = (fAnalysisRed / ANALYSIS_BLOB_COLOUR_HALFLIFE_FACTOR);
	 * fAnalysisGreen = 1.0f - ((1.0f - fAnalysisGreen) / ANALYSIS_BLOB_COLOUR_HALFLIFE_FACTOR);
	 * glColor3f (fAnalysisRed, fAnalysisGreen, 0.0);
	 * } ;
	 * else { ;
	 * fAnalysisRed = 1.0f - ((1.0f - fAnalysisRed) / ANALYSIS_BLOB_COLOUR_HALFLIFE_FACTOR);
	 * fAnalysisGreen = (fAnalysisGreen / ANALYSIS_BLOB_COLOUR_HALFLIFE_FACTOR);
	 * glColor3f (fAnalysisRed, fAnalysisGreen, 0.0);
	 * } ;
	 * } ;
	 * else { ;
	 * fAnalysisRed = 1.0f - ((1.0f - fAnalysisRed) / ANALYSIS_BLOB_COLOUR_HALFLIFE_FACTOR);
	 * fAnalysisGreen = 0.5f - ((0.5f - fAnalysisGreen) / ANALYSIS_BLOB_COLOUR_HALFLIFE_FACTOR);
	 * glColor3f (fAnalysisRed, fAnalysisGreen, 0.0);
	 * }
	 */
	if (boAnalysisCompleted) {
		if (boAnalysisResult) {
			glBindTexture (GL_TEXTURE_2D, psTexture->GetTexture (TEXNAME_GREENBLOB));
		} else {
			glBindTexture (GL_TEXTURE_2D, psTexture->GetTexture (TEXNAME_REDBLOB));
		}
	} else {
		glBindTexture (GL_TEXTURE_2D, psTexture->GetTexture (TEXNAME_AMBERBLOB));
	}

	glColor4f (1.0, 1.0, 1.0, 1.0);

	glBegin (GL_QUADS);
	glTexCoord2f (1, 0);
	glVertex3i (nWidth - 40, BUTTONPOS_FIRSTY, 0);
	glTexCoord2f (0, 0);
	glVertex3i (nWidth - 72, BUTTONPOS_FIRSTY, 0);
	glTexCoord2f (0, 1);
	glVertex3i (nWidth - 72, BUTTONPOS_FIRSTY + 32, 0);
	glTexCoord2f (1, 1);
	glVertex3i (nWidth - 40, BUTTONPOS_FIRSTY + 32, 0);
	glEnd ();

	PlotButton (BUTTONPOS_CENTREDX + (0 * BUTTONPOS_WIDTH), BUTTONPOS_FIRSTY, TEXNAME_BUTTONSELECT,
				TEXNAME_BUTTONSELECTD, aboButtonPressed[BUTTON_SELECT]);
	PlotButton (BUTTONPOS_CENTREDX + (1 * BUTTONPOS_WIDTH), BUTTONPOS_FIRSTY, TEXNAME_BUTTONLINKS, TEXNAME_BUTTONLINKSD,
				aboButtonPressed[BUTTON_LINKS]);
	PlotButton (BUTTONPOS_CENTREDX + (2 * BUTTONPOS_WIDTH), BUTTONPOS_FIRSTY, TEXNAME_BUTTONSPIN, TEXNAME_BUTTONSPIND,
				aboButtonPressed[BUTTON_SPIN]);
	PlotButton (BUTTONPOS_CENTREDX + (3 * BUTTONPOS_WIDTH), BUTTONPOS_FIRSTY, TEXNAME_BUTTONNET,
				TEXNAME_BUTTONNETD, aboButtonPressed[BUTTON_NET]);
	PlotButton (BUTTONPOS_CENTREDX + (4 * BUTTONPOS_WIDTH), BUTTONPOS_FIRSTY, TEXNAME_BUTTONDETAILS,
				TEXNAME_BUTTONDETAILSD, aboButtonPressed[BUTTON_DETAILS]);
	PlotButton (BUTTONPOS_CENTREDX + (5 * BUTTONPOS_WIDTH), BUTTONPOS_FIRSTY, TEXNAME_BUTTONLEFT, TEXNAME_BUTTONLEFTD,
				aboButtonPressed[BUTTON_LEFT]);
	PlotButton (BUTTONPOS_CENTREDX + (6 * BUTTONPOS_WIDTH), BUTTONPOS_FIRSTY, TEXNAME_BUTTONBLANK, TEXNAME_BUTTONBLANKD,
				aboButtonPressed[BUTTON_NODES]);
	PlotButton (BUTTONPOS_CENTREDX + (7 * BUTTONPOS_WIDTH), BUTTONPOS_FIRSTY, TEXNAME_BUTTONRIGHT, TEXNAME_BUTTONRIGHTD,
				aboButtonPressed[BUTTON_RIGHT]);

	/*
	// Plot the logo
	glBindTexture (GL_TEXTURE_2D, psTexture->GetTexture (TEXNAME_LOGO));
	glColor4f (1.0, 1.0, 1.0, 1.0);
	glBegin (GL_QUADS);
	glTexCoord2f (1, 0);
	glVertex3i (168, BUTTONPOS_FIRSTY, 0);
	glTexCoord2f (0, 0);
	glVertex3i (40, BUTTONPOS_FIRSTY, 0);
	glTexCoord2f (0, 1);
	glVertex3i (40, BUTTONPOS_FIRSTY + 128, 0);
	glTexCoord2f (1, 1);
	glVertex3i (168, BUTTONPOS_FIRSTY + 128, 0);
	glEnd ();
	// Finished plotting the logo
	*/



	glEnable (GL_LIGHTING);
	glDisable (GL_LIGHT0);
	glEnable (GL_LIGHT1);
	glPushMatrix ();
	glTranslatef ((GLfloat)(BUTTONPOS_CENTREDX + (6 * BUTTONPOS_WIDTH) + (BUTTON_WIDTH / 2)), (GLfloat)(BUTTONPOS_FIRSTY + (BUTTON_HEIGHT / 2)),
				  0.0);
	glScalef (24, 24, 24);
	if (fTimeStep > 100.0f) {
		fNodeButtonRotate += (100.0f * NODEBUTTONROTATE_SPEED);
	}
	else {
		fNodeButtonRotate += fTimeStep * NODEBUTTONROTATE_SPEED;
	}
	if (fNodeButtonRotate > 360.0f) {
		fNodeButtonRotate = fmod (fNodeButtonRotate, 360.0f);
	}

	glColorMaterial (GL_FRONT, GL_DIFFUSE);

	glMaterialfv (GL_FRONT, GL_AMBIENT, ambientMaterial);
	glMaterialfv (GL_FRONT, GL_DIFFUSE, diffuseMaterial);
	glMaterialfv (GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialf (GL_FRONT, GL_SHININESS, 15.0f);

	glRotatef (60.0f, 1.0f, 0.0f, 0.0f);
	glRotatef (fNodeButtonRotate, 0.0f, 1.0f, 0.0f);
	psRenderCurrent->DrawNode (SHAPE_SPHERE, nNodeButtonNodeType);
	glPopMatrix ();
	glDisable (GL_LIGHT1);
	glEnable (GL_LIGHT0);
	glDisable (GL_LIGHTING);

	glDisable (GL_TEXTURE_2D);
	glDisable (GL_BLEND);

	/* Switch back to the original view */
	glMatrixMode (GL_PROJECTION);
	glPopMatrix ();
	glMatrixMode (GL_MODELVIEW);
	glEnable (GL_LIGHTING);
	glEnable (GL_DEPTH_TEST);
	glDepthMask (GL_TRUE);

	/*
	 * glDisable (GL_LIGHT1);
	 * glEnable (GL_LIGHT0);
	 */
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::PlotButton (int nXPos, int nYPos, unsigned int uTexture, unsigned int uTextureDown, bool boDown) {
	if (boDown) {
		glBindTexture (GL_TEXTURE_2D, psTexture->GetTexture ((TEXNAME) uTextureDown));
	} else {
		glBindTexture (GL_TEXTURE_2D, psTexture->GetTexture ((TEXNAME) uTexture));
	}

	glBegin (GL_QUADS);
	glTexCoord2f (1, 0);
	glVertex3i (nXPos + 64, nYPos, 0);
	glTexCoord2f (0, 0);
	glVertex3i (nXPos, nYPos, 0);
	glTexCoord2f (0, 1);
	glVertex3i (nXPos, nYPos + 64, 0);
	glTexCoord2f (1, 1);
	glVertex3i (nXPos + 64, nYPos + 64, 0);
	glEnd ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cTopology::ClickButton (int nXPos, int nYPos) {
	bool boClicked;

	/* Set up an orthogonal view to overlay the existing 3D model */
	BUTTON eButton;
	LRESULT nResult;

	boClicked = false;
	if ((nYPos > BUTTONPOS_FIRSTY) && (nYPos < BUTTONPOS_FIRSTY + BUTTON_HEIGHT)) {
		eButton = (BUTTON) ((int) BUTTON_INVALID + 1);
		while ((eButton < BUTTON_NUM) && (!boClicked)) {
			if ((nXPos > (BUTTONPOS_CENTREDX + (eButton * BUTTONPOS_WIDTH)))
			&& (nXPos < (BUTTONPOS_CENTREDX + BUTTON_WIDTH + (eButton * BUTTONPOS_WIDTH)))) {
				boClicked = true;
			} else {
				eButton = (BUTTON) ((int) eButton + 1);
			}
		}
	}

	if (boClicked) {
		switch (eButton) {
		case BUTTON_SELECT:
			nResult = SendMessage (hWindow, WM_COMMAND, ID_VIEW_SELECT, 0);
			break;
		case BUTTON_LINKS:
			nResult = SendMessage (hWindow, WM_COMMAND, ID_VIEW_CREATELINK, 0);
			break;
		case BUTTON_SPIN:
			nResult = SendMessage (hWindow, WM_COMMAND, ID_VIEW_SPIN, 0);
			break;
		case BUTTON_NET:
			nResult = SendMessage (hWindow, WM_COMMAND, ID_VIEW_RENDERTYPE, 0);
			break;
		case BUTTON_DETAILS:
			nResult = SendMessage (hWindow, WM_COMMAND, ID_VIEW_OVERLAY, 0);
			break;
		case BUTTON_LEFT:
			nNodeButtonNodeType = (nNodeButtonNodeType - 1);
			if (nNodeButtonNodeType < 0) {
				nNodeButtonNodeType = nNodeTypes - 1;
			}

			if (aboButtonPressed[BUTTON_NODES]) {
				SendNodeButtonNodeType ();
			}

			aboButtonPressed[BUTTON_LEFT] = true;
			uNodeButtonLeftTimeClicked = uTime;
			break;
		case BUTTON_NODES:
			SendNodeButtonNodeType ();
			break;
		case BUTTON_RIGHT:
			nNodeButtonNodeType = (nNodeButtonNodeType + 1);
			if (nNodeButtonNodeType >= nNodeTypes) {
				nNodeButtonNodeType = 0;
			}

			if (aboButtonPressed[BUTTON_NODES]) {
				SendNodeButtonNodeType ();
			}

			aboButtonPressed[BUTTON_RIGHT] = true;
			uNodeButtonRightTimeClicked = uTime;
			break;
		default:

			/* Do nothing */
			break;
		}
	}

	return boClicked;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::SendNodeButtonNodeType () {
	LRESULT nResult;

	nResult = SendMessage (hWindow, WM_COMMAND, ID_VIEW_CREATE_HOSPITAL, 0);

	//switch (eNodeButtonNodeType) {
	//default:
	//case TYPE_HOSPITAL:
	//	nResult = SendMessage (hWindow, WM_COMMAND, ID_VIEW_CREATE_HOSPITAL, 0);
	//	break;
	//case TYPE_POLICE:
	//	nResult = SendMessage (hWindow, WM_COMMAND, ID_VIEW_CREATE_POLICE, 0);
	//	break;
	//case TYPE_BANK:
	//	nResult = SendMessage (hWindow, WM_COMMAND, ID_VIEW_CREATE_BANK, 0);
	//	break;
	//case TYPE_EVENT:
	//	nResult = SendMessage (hWindow, WM_COMMAND, ID_VIEW_CREATE_EVENT, 0);
	//	break;
	//case TYPE_MOBILE:
	//	nResult = SendMessage (hWindow, WM_COMMAND, ID_VIEW_CREATE_MOBILE, 0);
	//	break;
	//case TYPE_RESCUE:
	//	nResult = SendMessage (hWindow, WM_COMMAND, ID_VIEW_CREATE_RESCUE, 0);
	//	break;
	//case TYPE_FIRE:
	//	nResult = SendMessage (hWindow, WM_COMMAND, ID_VIEW_CREATE_FIRE, 0);
	//	break;
	//case TYPE_AMBULANCE:
	//	nResult = SendMessage (hWindow, WM_COMMAND, ID_VIEW_CREATE_AMBULANCE, 0);
	//	break;
	//case TYPE_TRANSPORT:
	//	nResult = SendMessage (hWindow, WM_COMMAND, ID_VIEW_CREATE_TRANSPORT, 0);
	//	break;
	//case TYPE_EXTERNAL:
	//	nResult = SendMessage (hWindow, WM_COMMAND, ID_VIEW_CREATE_EXTERNAL, 0);
	//	break;
	//}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
SelectObj cTopology::SelectMain (int nXPos, int nYPos) {
	return psRenderCurrent->SelectMain (pList, nXPos, nYPos);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
SelectObj cTopology::SelectMap (int nXPos, int nYPos) {
	return psRenderCurrent->SelectMap (nXPos, nYPos);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::Normalise (float* pfX, float* pfY, float* pfZ) {
	float fL;

	fL = sqrt (((*pfX) * (*pfX)) + ((*pfY) * (*pfY)) + ((*pfZ) * (*pfZ)));

	if (fL != 0) {
		*pfX /= fL;
		*pfY /= fL;
		*pfZ /= fL;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
float cTopology::DotProdAngle (float fX1, float fY1, float fX2, float fY2) {
	float fAngle;
	float fScaler;

	float fY;
	float fRot;

	fRot = atan2 (fY1, fX1);
	fY = -(fX2 * sin (fRot)) + (fY2 * cos (fRot));

	fScaler = sqrt ((fX1 * fX1) + (fY1 * fY1)) * sqrt ((fX2 * fX2) + (fY2 * fY2));
	fAngle = acos (((fX1 * fX2) + (fY1 * fY2)) / fScaler);

	if (fY < 0) {
		fAngle = -fAngle;
	}

	return fAngle;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::GetCentre (float* pfX, float* pfY, float* pfZ) {
	psRenderCurrent->GetCentre (pfX, pfY, pfZ);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::SetCentre (float fX, float fY, float fZ) {
	psRenderCurrent->SetCentre (fX, fY, fZ);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::MoveCentre (float fXDelta, float fYDelta, float fZDelta) {
	float fX;
	float fY;
	float fZ;
	float fTheta;

	GetView (& fTheta, NULL, NULL, NULL);

	psRenderCurrent->GetCentre (& fX, & fY, & fZ);
	fX += (fXDelta * cos (fTheta)) - (fYDelta * sin (fTheta));
	fY += (fYDelta * cos (fTheta)) + (fXDelta * sin (fTheta));
	fZ += fZDelta;
	psRenderCurrent->SetCentre (fX, fY, fZ);
}

/*
 =======================================================================================================================
 *  void cTopology::ChangeView (float fTheta, float fPhi, float fPsi, float fRadius) ;
 *  { ;
 *  float fA;
 *  float fB;
 *  float fX;
 *  float fY;
 *  float fZ;
 *  float fXn;
 *  float fYn;
 *  float fZn;
 *  ;
 *  float fXv;
 *  float fYv;
 *  float fZv;
 *  ;
 *  gfViewRadius += fRadius;
 *  ;
 *  // Phi ;
 *  fA = cos (fPhi);
 *  fB = sin (fPhi);
 *  ;
 *  fX = (fA * gfX) + (fB * gfXn);
 *  fY = (fA * gfY) + (fB * gfYn);
 *  fZ = (fA * gfZ) + (fB * gfZn);
 *  ;
 *  fXn = - (fB * gfX) + (fA * gfXn);
 *  fYn = - (fB * gfY) + (fA * gfYn);
 *  fZn = - (fB * gfZ) + (fA * gfZn);
 *  ;
 *  gfX = fX;
 *  gfY = fY;
 *  gfZ = fZ;
 *  ;
 *  gfXn = fXn;
 *  gfYn = fYn;
 *  gfZn = fZn;
 *  ;
 *  // Theta ;
 *  fXv = (gfY * gfZn) - (gfZ * gfYn);
 *  fYv = (gfZ * gfXn) - (gfX * gfZn);
 *  fZv = (gfX * gfYn) - (gfY * gfXn);
 *  ;
 *  fA = cos (fTheta);
 *  fB = sin (fTheta);
 *  ;
 *  fX = (fA * gfX) + (fB * fXv);
 *  fY = (fA * gfY) + (fB * fYv);
 *  fZ = (fA * gfZ) + (fB * fZv);
 *  ;
 *  gfX = fX;
 *  gfY = fY;
 *  gfZ = fZ;
 *  ;
 *  // Psi ;
 *  fA = cos (fPsi);
 *  fB = sin (fPsi);
 *  ;
 *  fXv = (gfY * gfZn) - (gfZ * gfYn);
 *  fYv = (gfZ * gfXn) - (gfX * gfZn);
 *  fZv = (gfX * gfYn) - (gfY * gfXn);
 *  ;
 *  fXn = (fA * fXn) - (fB * fXv);
 *  fYn = (fA * fYn) - (fB * fYv);
 *  fZn = (fA * fZn) - (fB * fZv);
 *  ;
 *  gfXn = fXn;
 *  gfYn = fYn;
 *  gfZn = fZn;
 *  ;
 *  // Normalise vectors (they should already be, but we make sure to avoid ;
 *  // cumulative rounding errors) ;
 *  Normalise (& gfX, & gfY, & gfZ);
 *  Normalise (& gfXn, & gfYn, & gfZn);
 *  ;
 *  RedrawTopology ();
 *  } ;
 *  bool cTopology::PointTowards (int nXPos, int nYPos, int nZPos, float fRadius) ;
 *  { ;
 *  float fTheta;
 *  float fPhi;
 *  float fPsi;
 *  ;
 *  float fX;
 *  float fY;
 *  float fZ;
 *  float fXp;
 *  float fYp;
 *  float fZp;
 *  float fXn;
 *  float fYn;
 *  float fZn;
 *  float fHalfLife;
 *  cOptions * pOptions = cOptions::create();
 *  fHalfLife = pOptions->getRotateHalfLife ();
 *  ;
 *  bool boArrived = true;
 *  ;
 *  fXp = (-MGL_WIDTH/2.0f + (float)nXPos) / MGL_SCALE;
 *  fYp = (MGL_HEIGHT/2.0f - (float)nYPos) / MGL_SCALE;
 *  fZp = (-MGL_DEPTH/2.0f + (float)nZPos) / MGL_SCALE;
 *  ;
 *  fTheta = DotProdAngle (fXp, fZp, gfX, gfZ) * fHalfLife;
 *  if (abs (fTheta) > POINTTO_MINMOVE) ;
 *  { ;
 *  fX = (gfX * cos (fTheta)) + (gfZ * sin (fTheta));
 *  fZ = - (gfX * sin (fTheta)) + (gfZ * cos (fTheta));
 *  fXn = (gfXn * cos (fTheta)) + (gfZn * sin (fTheta));
 *  fZn = - (gfXn * sin (fTheta)) + (gfZn * cos (fTheta));
 *  gfX = fX;
 *  gfZ = fZ;
 *  gfXn = fXn;
 *  gfZn = fZn;
 *  boArrived = false;
 *  } ;
 *  fPhi = DotProdAngle (fYp, fZp, gfY, gfZ) * fHalfLife;
 *  if (abs(fPhi) > POINTTO_MINMOVE) ;
 *  { ;
 *  fY = (gfY * cos (fPhi)) + (gfZ * sin (fPhi));
 *  fZ = - (gfY * sin (fPhi)) + (gfZ * cos (fPhi));
 *  fYn = (gfYn * cos (fPhi)) + (gfZn * sin (fPhi));
 *  fZn = - (gfYn * sin (fPhi)) + (gfZn * cos (fPhi));
 *  gfY = fY;
 *  gfZ = fZ;
 *  gfYn = fYn;
 *  gfZn = fZn;
 *  boArrived = false;
 *  } ;
 *  fPsi = DotProdAngle (fXp, fYp, gfX, gfY) * fHalfLife;
 *  if (abs(fPsi) > POINTTO_MINMOVE) ;
 *  { ;
 *  fX = (gfX * cos (fPsi)) + (gfY * sin (fPsi));
 *  fY = - (gfX * sin (fPsi)) + (gfY * cos (fPsi));
 *  fXn = (gfXn * cos (fPsi)) + (gfYn * sin (fPsi));
 *  fYn = - (gfXn * sin (fPsi)) + (gfYn * cos (fPsi));
 *  gfX = fX;
 *  gfY = fY;
 *  gfXn = fXn;
 *  gfYn = fYn;
 *  boArrived = false;
 *  } ;
 *  //gfY = (gfY * cos (fPhi)) + (gfZ * sin (fPhi));
 *  //gfZ = - (gfY * sin (fPhi)) + (gfZ * cos (fPhi));
 *  ;
 *  //gfX = (gfX * cos (fPsi)) + (gfY * sin (fPsi));
 *  //gfY = - (gfX * sin (fPsi)) + (gfY * cos (fPsi));
 *  ;
 *  Normalise (& gfX, & gfY, & gfZ);
 *  ;
 *  RedrawTopology ();
 *  ;
 *  return boArrived;
 *  }
 =======================================================================================================================
 */
void cTopology::GetView (float* pfTheta, float* pfPhi, float* pfPsi, float* pfRadius) {
	psRenderCurrent->GetView (pfTheta, pfPhi, pfPsi, pfRadius);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::ViewOffset (float* pfTheta, float* pfPhi, float* pfPsi, float* pfRadius) {
	psRenderCurrent->ViewOffset (pfTheta, pfPhi, pfPsi, pfRadius);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::CentreOffset (float* pfX, float* pfY, float* pfZ) {
	psRenderCurrent->CentreOffset (pfX, pfY, pfZ);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::ChangeView (float fTheta, float fPhi, float fPsi, float fRadius) {
	psRenderCurrent->ChangeView (fTheta, fPhi, fPsi, fRadius);
	RedrawTopology ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cTopology::PointTowards (float fXPos, float fYPos, float fZPos, float fRadius) {
	return psRenderCurrent->PointTowards (fXPos, fYPos, fZPos, fRadius);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::RedrawTopology (void) {
	RECT sRect;

	GetClientRect (hWindow, &sRect);

	/*
	 * sRect.top += REDRAW_GAP+200;
	 * sRect.top = 300;
	 * sRect.bottom += 300;
	 */
	RedrawWindow (hWindow, &sRect, NULL, (RDW_INVALIDATE));
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::CalculateLinkList (void) {
	int nCount;
	int nPoints;
	cTopologyLink*	pLinks;
	cTopologyNode*	psIterator;

	if (pList) {
		nPoints = pList->getCount ();

		for (nCount = -LINKLIST_OFFSET; nCount < nPoints; nCount++) {
			gnBoxLinkInCount[nCount + LINKLIST_OFFSET] = 0;
			gnBoxLinkOutCount[nCount + LINKLIST_OFFSET] = 0;
			gpsBox[nCount + LINKLIST_OFFSET] = NULL;
		}

		/* Give each node an enumeration number */
		nCount = 0;
		psIterator = pList->startIterator ();
		while (pList->isIteratorValid (psIterator)) {
			pList->getCurrentNode (psIterator)->nEnumerationTemp = nCount;
			gpsBox[nCount + LINKLIST_OFFSET] = pList->getCurrentNode (psIterator);
			nCount++;
			psIterator = pList->moveForward (psIterator);
		}

		/* Enumerate all of the links between the VMs */
		nCount = 0;
		psIterator = pList->startIterator ();
		while (pList->isIteratorValid (psIterator)) {

			/* Enumerate all of the links out */
			pLinks = pList->getLinksOut (psIterator);
			while (pLinks) {
				if (gnBoxLinkOutCount[nCount + LINKLIST_OFFSET] < 20) {
					gpsBoxLinkBoxOut[nCount + LINKLIST_OFFSET][gnBoxLinkOutCount[nCount + LINKLIST_OFFSET]] = pLinks;
					pLinks->nLinkOutIndexTemp = gnBoxLinkOutCount[nCount + LINKLIST_OFFSET];
					gnBoxLinkOutCount[nCount + LINKLIST_OFFSET]++;
				}

				pLinks = pLinks->pFromNext;
			}

			/* Enumerate all of the links in */
			pLinks = pList->getLinksIn (psIterator);
			while (pLinks) {
				if (gnBoxLinkInCount[nCount + LINKLIST_OFFSET] < 20) {
					gpsBoxLinkBoxIn[nCount + LINKLIST_OFFSET][gnBoxLinkInCount[nCount + LINKLIST_OFFSET]] = pLinks;
					pLinks->nLinkInIndexTemp = gnBoxLinkInCount[nCount + LINKLIST_OFFSET];
					gnBoxLinkInCount[nCount + LINKLIST_OFFSET]++;
				}

				pLinks = pLinks->pToNext;
			}

			nCount++;
			psIterator = pList->moveForward (psIterator);
		}
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cTopology::LinksInCount (int nPoint) {
	return gnBoxLinkInCount[nPoint + LINKLIST_OFFSET];
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cTopology::LinksOutCount (int nPoint) {
	return gnBoxLinkOutCount[nPoint + LINKLIST_OFFSET];
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
char* cTopology::GetLinkInput (int nPoint, int nLinkNum) {
	return gpsBoxLinkBoxIn[nPoint + LINKLIST_OFFSET][nLinkNum]->szLinkInput;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
char* cTopology::GetLinkOutput (int nPoint, int nLinkNum) {
	return gpsBoxLinkBoxOut[nPoint + LINKLIST_OFFSET][nLinkNum]->szLinkOutput;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cTopology::LinkTo (int nPoint, int nLinkNum) {
	return gpsBoxLinkBoxOut[nPoint + LINKLIST_OFFSET][nLinkNum]->psLinkTo->nEnumerationTemp;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cTopology::LinkFrom (int nPoint, int nLinkNum) {
	return gpsBoxLinkBoxIn[nPoint + LINKLIST_OFFSET][nLinkNum]->psLinkFrom->nEnumerationTemp;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
char* cTopology::GetComponentProperties (int nPoint) {
	return gpsBox[nPoint + LINKLIST_OFFSET]->szCertProps;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cTopology::GetPredefinedProperty (int nPoint, PREPROP ePreProp) {
	int nReturn;
	cTopologyNode*	psBox;

	nReturn = 0;
	psBox = gpsBox[nPoint + LINKLIST_OFFSET];

	switch (ePreProp) {
	case PREPROP_SENSITIVITYLEVEL:
		nReturn = psBox->nSensitivityLevel;
		break;
	case PREPROP_ENCRYPTIONSTRENGTH:
		if (strcmp (psBox->szEncryptAlgorithm, "Unspecified") == 0) {
			nReturn = 1;
		} else if (strcmp (psBox->szEncryptAlgorithm, "WEP-40") == 0) {
			nReturn = 2;
		} else if (strcmp (psBox->szEncryptAlgorithm, "RC2-40") == 0) {
			nReturn = 3;
		} else if (strcmp (psBox->szEncryptAlgorithm, "RC4-40") == 0) {
			nReturn = 4;
		} else if (strcmp (psBox->szEncryptAlgorithm, "WEP-114") == 0) {
			nReturn = 5;
		} else if (strcmp (psBox->szEncryptAlgorithm, "DES-56") == 0) {
			nReturn = 6;
		} else if (strcmp (psBox->szEncryptAlgorithm, "RC5-56") == 0) {
			nReturn = 7;
		} else if (strcmp (psBox->szEncryptAlgorithm, "RC2-128") == 0) {
			nReturn = 8;
		} else if (strcmp (psBox->szEncryptAlgorithm, "RC4-128") == 0) {
			nReturn = 9;
		} else if (strcmp (psBox->szEncryptAlgorithm, "RC5-128") == 0) {
			nReturn = 10;
		} else if (strcmp (psBox->szEncryptAlgorithm, "TDES-168") == 0) {
			nReturn = 11;
		} else if (strcmp (psBox->szEncryptAlgorithm, "IDEA-128") == 0) {
			nReturn = 12;
		} else if (strcmp (psBox->szEncryptAlgorithm, "Skipjack-80") == 0) {
			nReturn = 13;
		}

		break;
	case PREPROP_STAFFSKILLS:
		if (strcmp (psBox->szStaffSkills, "Low") == 0) {
			nReturn = 1;
		} else if (strcmp (psBox->szStaffSkills, "Mid") == 0) {
			nReturn = 2;
		} else if (strcmp (psBox->szStaffSkills, "High") == 0) {
			nReturn = 3;
		}

		break;
	case PREPROP_FIREWALL:
		nReturn = psBox->boFirewall;
		break;
	case PREPROP_IDS:
		nReturn = psBox->boIDS;
		break;
	case PREPROP_EXTERNAL:
		nReturn = psBox->boExternal;
		break;
	default:
		nReturn = 0;
		break;
	}

	return nReturn;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cTopology::GetPredefinedProperty (int nPoint, int nLinkNum, PREPROP ePreProp) {
	int nReturn;
	cTopologyNode*	psBox;
	nReturn = 0;
	psBox = gpsBox[nPoint + LINKLIST_OFFSET];

	/*
	 * cTopologyLink * psBoxLink = gpsBoxLinkBoxOut[nPoint + LINKLIST_OFFSET][nLinkNum];
	 */
	switch (ePreProp) {
	case PREPROP_SENSITIVITYLEVEL:
		nReturn = gpsBoxLinkBoxOut[nPoint + LINKLIST_OFFSET][nLinkNum]->nSensitivityLevel;
		break;
	case PREPROP_CHANNELOUT:
		nReturn = gpsBoxLinkBoxOut[nPoint + LINKLIST_OFFSET][nLinkNum]->nChannelOut;
		break;
	default:
		nReturn = 0;
		break;
	}

	return nReturn;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cVM* cTopology::GetComponentVM (int nPoint) {
	return gpsBox[nPoint + LINKLIST_OFFSET]->virtualMachine;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cTopology::GetLinkInputChannelIn (int nPoint, int nLinkNum) {
	return gpsBoxLinkBoxIn[nPoint + LINKLIST_OFFSET][nLinkNum]->nChannelIn;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cTopology::GetLinkInputChannelOut (int nPoint, int nLinkNum) {
	return gpsBoxLinkBoxIn[nPoint + LINKLIST_OFFSET][nLinkNum]->nChannelOut;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cTopology::GetLinkOutputChannelIn (int nPoint, int nLinkNum) {
	return gpsBoxLinkBoxOut[nPoint + LINKLIST_OFFSET][nLinkNum]->nChannelIn;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cTopology::GetLinkOutputChannelOut (int nPoint, int nLinkNum) {
	return gpsBoxLinkBoxOut[nPoint + LINKLIST_OFFSET][nLinkNum]->nChannelOut;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cTopology::GetLinkInputOutputIndex (int nPoint, int nLinkNum) {
	return gpsBoxLinkBoxIn[nPoint + LINKLIST_OFFSET][nLinkNum]->nLinkOutIndexTemp;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cTopology::GetLinkOutputInputIndex (int nPoint, int nLinkNum) {
	return gpsBoxLinkBoxOut[nPoint + LINKLIST_OFFSET][nLinkNum]->nLinkInIndexTemp;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::Resize (RECT const* psNewWinRect) {
	float fAspectRatio;
  static GLfloat afLightAmbient[] = {0.3, 0.3, 0.3, 1.0, 0.5, 0.5, 0.5, 1.0};
  static GLfloat afLightDiffuse[] = {0.5, 0.5, 0.5, 1.0, 0.5, 0.3, 0.0, 1.0};
  static GLfloat afLightSpecular[] = {0.2, 0.2, 0.2, 1.0, 0.3, 0.5, 1.0, 0.0};
  static GLfloat afLightPosition[] = {0.0, 10.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0.0};
	static GLfloat afFog[] = {BGCOL_RED, BGCOL_GREEN, BGCOL_BLUE, 1.0};

	nHeight = psNewWinRect->bottom - psNewWinRect->top;
	nWidth = psNewWinRect->right - psNewWinRect->left;
	if (nHeight <= 0) {
		nHeight = 1;
	}

	fAspectRatio = (float) nWidth / (float) nHeight;

	glViewport (0, 0, nWidth, nHeight);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective (30.0, fAspectRatio, 1.5, 250.0);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();

	glClearColor (BGCOL_RED, BGCOL_GREEN, BGCOL_BLUE, 0.0f);
	glEnable (GL_FOG);
	glFogi (GL_FOG_MODE, GL_LINEAR);
	glFogfv (GL_FOG_COLOR, afFog);
	glFogf (GL_FOG_DENSITY, 0.04);
	glHint (GL_FOG_HINT, GL_DONT_CARE);
	glFogf (GL_FOG_START, 35.0);
	glFogf (GL_FOG_END, 40.0);

	psRenderCurrent->Resize (nHeight, nWidth);

	//glShadeModel (GL_SMOOTH);
	glEnable (GL_DEPTH_TEST);

  glLightfv (GL_LIGHT0, GL_DIFFUSE, afLightDiffuse);
  glLightfv (GL_LIGHT0, GL_SPECULAR, afLightSpecular);
  glLightfv (GL_LIGHT0, GL_POSITION, afLightPosition);
  //glLightfv (GL_LIGHT0, GL_AMBIENT, light_ambient);
  glLightModelfv (GL_LIGHT_MODEL_AMBIENT, afLightAmbient);
  glLightModeli (GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);


//static GLfloat light_position1[] = { BUTTONPOS_FIRSTX + (5 * BUTTONPOS_WIDTH), BUTTONPOS_FIRSTY, -80.0f, 1.0f };
//static GLfloat const light_ambient1[] = { 1.0f, 1.0f, 1.0f, 1.0f };
//static GLfloat const light_diffuse1[] = { 1.0f, 1.0f, 1.0f, 1.0f };
//static GLfloat const light_specular1[] = { 1.0f, 1.0f, 1.0f, 1.0f };
//static GLfloat light_position0[] = { 0.5f, 0.5f, 1.0f, 0.0f };
//static GLfloat const light_ambient0[] = { 0.0f, 0.0f, 0.0f, 1.0f };
//static GLfloat const light_diffuse0[] = { 1.0f, 1.0f, 1.0f, 1.0f };
//static GLfloat const light_specular0[] = { 0.0f, 0.0f, 0.0f, 1.0f };
//static GLfloat const LightModelAmbient[] = { 0.5f, 0.5f, 0.5f, 1.0f };


	//glLightModelfv (GL_LIGHT_MODEL_AMBIENT, LightModelAmbient);
	//glLightfv (GL_LIGHT0, GL_POSITION, light_position0);
	//glLightfv (GL_LIGHT0, GL_AMBIENT, light_ambient0);
	//glLightfv (GL_LIGHT0, GL_DIFFUSE, light_diffuse0);
	//glLightfv (GL_LIGHT0, GL_SPECULAR, light_specular0);
	glEnable (GL_LIGHTING);
	glEnable (GL_LIGHT0);

	light_position1[0] = (GLfloat)(BUTTONPOS_CENTREDX + (5 * BUTTONPOS_WIDTH));
	glLightfv (GL_LIGHT1, GL_POSITION, light_position1);
	glLightfv (GL_LIGHT1, GL_AMBIENT, light_ambient1);
	glLightfv (GL_LIGHT1, GL_DIFFUSE, light_diffuse1);
	glLightfv (GL_LIGHT1, GL_SPECULAR, light_specular1);
	glDisable (GL_LIGHT1);

	RedrawTopology ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cTopologyNode* cTopology::DecodeNodeName (char const * szName) {
	cTopologyNode*	psNode;
	cTopologyNode*	psIterator;
	char const * szNameCheck;

	psNode = NULL;
	if ((pList) && (szName)) {
		psIterator = pList->startIterator ();

		while (pList->isIteratorValid (psIterator) && (psNode == NULL)) {
			szNameCheck = pList->getName (psIterator);
			if (szNameCheck && (strcmp (szNameCheck, szName) == 0)) {
				psNode = pList->getCurrentNode (psIterator);
			}

			psIterator = pList->moveForward (psIterator);
		}
	}

	return psNode;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cTopologyNode* cTopology::DecodeNodeNumber (int nNode) {
	cTopologyNode*	psNode;
	cTopologyNode*	psIterator;

	psNode = NULL;
	if ((pList) && (nNode >= 0)) {
		psIterator = pList->startIterator ();

		while (pList->isIteratorValid (psIterator) && (psNode == NULL)) {
			if (pList->getCurrentID (psIterator) == nNode) {
				psNode = pList->getCurrentNode (psIterator);
			}

			psIterator = pList->moveForward (psIterator);
		}
	}

	return psNode;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cTopologyLink* cTopology::DecodeLinkName (char const * szName) {
	cTopologyLink*	psLink;
	cTopologyLink*	pLinks;
	cTopologyNode*	psIterator;
	char const * szNameCheck;

	psLink = NULL;
	if ((pList) && (szName)) {
		psIterator = pList->startIterator ();

		while (pList->isIteratorValid (psIterator) && (psLink == NULL)) {
			pLinks = pList->getLinksOut (psIterator);
			while (pLinks && (psLink == NULL)) {
				szNameCheck = pLinks->PropertyGetString ("Name");
				if (szNameCheck && (strcmp (szNameCheck, szName) == 0)) {
					psLink = pLinks;
				}
				pLinks = pLinks->pFromNext;
			}
			psIterator = pList->moveForward (psIterator);
		}
	}

	return psLink;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cTopologyLink* cTopology::DecodeLinkNumber (int nLink) {
	cTopologyLink*	psLink;
	cTopologyLink*	pLinks;
	cTopologyNode*	psIterator;

	psLink = NULL;
	if ((pList) && (nLink >= 0)) {
		psIterator = pList->startIterator ();

		while (pList->isIteratorValid (psIterator) && (psLink == NULL)) {
			pLinks = pList->getLinksOut (psIterator);
			while (pLinks && (psLink == NULL)) {
				if (pLinks->nLinkID == nLink) {
					psLink = pLinks;
				}
				pLinks = pLinks->pFromNext;
			}
			psIterator = pList->moveForward (psIterator);
		}
	}

	return psLink;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::ConvertCoords (int nXMousePos, int nYMousePos, float fZPos, double* pfX, double* pfY, double* pfZ) {
	psRenderCurrent->ConvertCoords (nXMousePos, nYMousePos, fZPos, pfX, pfY, pfZ);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cTopology::CreateLink (int nFrom, int nTo, int nLinkID) {
	cTopologyNode*	psNodeFrom;
	cTopologyNode*	psNodeTo;
	CASCADE eCascadeCheck;

	psNodeFrom = DecodeNodeNumber (nFrom);
	psNodeTo = DecodeNodeNumber (nTo);

	/* Connect the nodes together */
	psSelectedLink = new cTopologyLink (psNodeFrom, psNodeTo, nLinkID);
	boLinkCancelled = false;

	/* Check the link for cascade vulnerabilities */
	eCascadeCheck = psSelectedLink->CheckCascadeLink ();
	switch (eCascadeCheck) {
	case CASCADE_LINKOUTSIDERANGE:
		MessageBox (NULL, "Link is not within range of nodes", "Cascade vulnerability check", MB_OK);
		boLinkCancelled = true;
		break;
	case CASCADE_VULNERABILITY:
		MessageBox (NULL, "Cascade vulnerability failure.", "Cascade vulnerability check", MB_OK);
		boLinkCancelled = true;
		break;
	default:
		break;
	}

	if (boLinkCancelled) {
		delete psSelectedLink;
		psSelectedLink = NULL;
	} else {
		psSelectedLink->UpdateCascadeTables ();
	}

	RedrawTopology ();

	return !boLinkCancelled;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::DeleteLink (cTopologyLink* psLink) {
	cTopologyNode*	psNodeFrom;
	cTopologyNode*	psNodeTo;

	/*
	 * char szFrom[TOPOLOGYNODE_NAMLEN];
	 * char szTo[TOPOLOGYNODE_NAMLEN];
	 */
	if (psLink) {
		psNodeFrom = psLink->psLinkFrom;
		psNodeTo = psLink->psLinkTo;
		SetSelectedLink (NULL);
		delete psLink;

		RedrawTopology ();

		/* Update the security Output tables */
		psNodeFrom->UpdateCascadeOutputTableRecurse ();

		/* Update the security Input tables */
		psNodeTo->UpdateCascadeInputTableRecurse ();

		/*/
		 *  Notify Dandelion ;
		 *  _snprintf (szFrom, sizeof (szFrom), "%d", psNodeFrom->getID ());
		 *  _snprintf (szTo, sizeof (szTo), "%d", psNodeTo->getID ());
		 *  if (psDlClient) { ;
		 *  psDlClient->SubLink (szFrom, szTo);
		 *  }
		 */
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::MoveNode (cTopologyNode* psNode, float fXPos, float fYPos, float fZPos) {
	psNode->fXPos = fXPos;
	psNode->fYPos = fYPos;
	psNode->fZPos = fZPos;
}

/*
 =======================================================================================================================
 *  void cTopology::TransformBasis (float fXScreen, float fYScreen, float fZScreen, float * pfX, float * pfY, float
 *  pfZ) { float fA;
 *  float fB;
 *  float fC;
 *  float fXv;
 *  float fYv;
 *  float fZv;
 *  fXv = (gfY * gfZn) - (gfZ * gfYn);
 *  fYv = (gfZ * gfXn) - (gfX * gfZn);
 *  fZv = (gfX * gfYn) - (gfY * gfXn);
 *  fA = ((((gfXn * fYScreen) - (fZScreen * gfYn)) * ((gfZn * fYv) - (fZv * gfYn))) - (((fXv * gfYn) - (gfXn * fYv))
 *  ((gfZn * fYScreen) - (fZScreen * gfYn)))) / ((((gfX * gfYn) - (gfXn * gfY)) * ((gfZn * fYv) - (fZv * gfYn))) (((fXv
 *  gfYn) - (gfXn * fYv)) * ((gfZn * gfY) - (gfZ * gfYn))));
 *  fB = ((((fXScreen * gfY) - (gfX * fYScreen)) * ((fYv * gfZ) - (gfY * fZv))) - (((fXv * gfY) - (gfX * fYv))
 *  ((fYScreen * gfZ) - (gfY * fZScreen)))) / ((((gfXn * gfY) - (gfX * gfYn)) * ((fYv * gfZ) - (gfY * fZv))) - (((fXv
 *  gfY) - (gfX * fYv)) * ((gfYn * gfZ) - (gfY * gfZn))));
 *  fC = ((((fXScreen * gfY) - (gfX * fYScreen)) * ((gfYn * gfZ) - (gfY * gfZn))) - (((gfXn * gfY) - (gfX * gfYn))
 *  ((fYScreen * gfZ) - (gfY * fZScreen)))) / ((((fXv * gfY) - (gfX * fYv)) * ((gfYn * gfZ) - (gfY * gfZn))) - (((gfXn
 *  gfY) - (gfX * gfYn)) * ((fYv * gfZ) - (gfY * fZv))));
 *  pfX = -fC;
 *  pfY = fB;
 *  pfZ = fA;
 *  }
 =======================================================================================================================
 */
cTopologyLink* cTopology::GetSelectedLink (void) {
	return psSelectedLink;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::SetSelectedLink (cTopologyLink* psSetSelectedLink) {
	psSelectedLink = psSetSelectedLink;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cTopologyNode* cTopology::GetSelectedNode (void) {

	/*
	 * boSetActive();
	 * pCurrentVm->setActive(true);
	 */
	return psSelectedNode;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cTopologyList* cTopology::GetpList (void) {
	return pList;
}

/*
 =======================================================================================================================
 *  new function to get node to be deleted
 =======================================================================================================================
 */
cTopologyNode* cTopology::GetNodeToDelete (void) {
	return psSelectedNode;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::SetSelectedNode (cTopologyNode* psSetSelectedNode) {
	psSelectedNode = psSetSelectedNode;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void glutSolidTube (double fRadius, double fHeight, int nSlices, int nStacks) {
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cTopologyLink* cTopology::FindCommandLink (int nCommandIDFrom, int nCommandIDTo) {
	cTopologyLink*	pLinks;
	cTopologyLink*	pFound;
	cTopologyNode*	psIterator;

	/* Iterate through all of the links */
	psIterator = pList->startIterator ();
	pFound = NULL;
	while (pList->isIteratorValid (psIterator)) {
		if (pList->getCurrentNode (psIterator)->nControlID == nCommandIDFrom) {
			pLinks = pList->getLinksOut (psIterator);
			while (pLinks) {
				if (pLinks->psLinkTo->nControlID == nCommandIDTo) {
					pFound = pLinks;
				}

				pLinks = pLinks->pFromNext;
			}
		}

		psIterator = pList->moveForward (psIterator);
	}

	return pFound;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::Update (unsigned long uTime) {
	Point ();
	psRenderCurrent->Update (pList, uTime);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::Point (void) {
	bool boArrived;

	boArrived = true;

	if (psPointToNode) {
		boArrived = PointTowards (psPointToNode->fXPos, psPointToNode->fYPos, psPointToNode->fZPos, 0.0f);
		if (boArrived) {
			psPointToNode = NULL;
		}
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cTopology::SetNodeAnalyseColour (int nPoint, int nColour) {
	cTopologyNode*	psNode;
	cTopologyNode*	psIterator;

	psNode = gpsBox[nPoint + LINKLIST_OFFSET];

	if (nColour == 0) {
		cOptions*  pOptions;
		pOptions = cOptions::create ();
		if (pOptions->getRotateSelected ()) {
			psPointToNode = psNode;
		}

		SetSelectedNode (psNode);

		if (pList) {
			psIterator = pList->startIterator ();

			while (pList->isIteratorValid (psIterator) && (pList->getCurrentNode (psIterator) != psNode)) {
				psIterator = pList->moveForward (psIterator);
			}

			psSelectedNode = pList->getCurrentNode (psIterator);
		}

		RedrawTopology ();
	} else {
		psNode->nAnalyseCol = nColour;
		RedrawTopology ();
	}

	return 1;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::PointToSelected (void) {
	psPointToNode = psSelectedNode;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::ResetAnalyseView (void) {
	cTopologyNode*	psIterator;

	psIterator = pList->startIterator ();

	while (pList->isIteratorValid (psIterator)) {
		pList->getCurrentNode (psIterator)->nAnalyseCol = 0;
		psIterator = pList->moveForward (psIterator);
	}

	RedrawTopology ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::SelectNextNode (void) {
	cTopologyNode*	psIterator;

	if (pList) {
		psIterator = pList->startIterator ();

		while (pList->isIteratorValid (psIterator) && (psSelectedNode != pList->getCurrentNode (psIterator))) {
			psIterator = pList->moveForward (psIterator);
		}

		if (pList->isIteratorValid (psIterator)) {
			psIterator = pList->moveForward (psIterator);
		}

		if (!pList->isIteratorValid (psIterator)) {
			psIterator = pList->startIterator ();
		}

		if (pList->isIteratorValid (psIterator)) {
			psSelectedNode = pList->getCurrentNode (psIterator);
		}

		psSelectedLink = NULL;
		RedrawTopology ();
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::SelectPreviousNode (void) {
	cTopologyNode*	psIterator;

	if (pList) {
		psIterator = pList->startIterator ();

		while (pList->isIteratorValid (psIterator) && (psSelectedNode != pList->getCurrentNode (psIterator))) {
			psIterator = pList->moveForward (psIterator);
		}

		if (pList->isIteratorValid (psIterator)) {
			psIterator = pList->moveBackward (psIterator);
		}

		if (!pList->isIteratorValid (psIterator)) {
			psIterator = pList->startIterator ();

			while (pList->isIteratorValid (psIterator)) {
				psSelectedNode = pList->getCurrentNode (psIterator);
				psIterator = pList->moveForward (psIterator);
			}
		}

		if (pList->isIteratorValid (psIterator)) {
			psSelectedNode = pList->getCurrentNode (psIterator);
		}

		psSelectedLink = NULL;
		RedrawTopology ();
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::SelectNextLink (void) {
	cTopologyNode*	pNode;
	cTopologyLink*	pLink;
	cTopologyNode*	psIterator;

	pNode = NULL;
	pLink = NULL;

	if (psSelectedLink) {
		pNode = psSelectedLink->psLinkFrom;
		pLink = psSelectedLink;
	} else {
		if (psSelectedNode) {
			pNode = psSelectedNode;
			pLink = pNode->pLinksOut;
		} else {
			psIterator = pList->startIterator ();
			pNode = pList->getCurrentNode (psIterator);
			pLink = pNode->pLinksOut;
		}
	}

	if (pNode) {
		if (pLink) {
			pLink = pLink->pFromNext;
			if (!pLink) {
				pLink = pLink = pNode->pLinksOut;
			}
		}

		psSelectedLink = pLink;

		psIterator = pList->startIterator ();
		while (pList->isIteratorValid (psIterator) && (psSelectedNode != pList->getCurrentNode (psIterator))) {
			psIterator = pList->moveForward (psIterator);
		}

		if (pList->isIteratorValid (psIterator)) {
			psSelectedNode = pList->getCurrentNode (psIterator);
		}

		RedrawTopology ();
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::SelectPreviousLink (void) {
	cTopologyNode*	pNode;
	cTopologyLink*	pLink;
	cTopologyLink*	pLinkCycle;
	cTopologyNode*	psIterator;

	pNode = NULL;
	pLink = NULL;
	pLinkCycle = NULL;

	if (psSelectedLink) {
		pNode = psSelectedLink->psLinkFrom;
		pLink = psSelectedLink;
	} else {
		if (psSelectedNode) {
			pNode = psSelectedNode;
			pLink = pNode->pLinksOut;
		} else {
			psIterator = pList->startIterator ();
			pNode = pList->getCurrentNode (psIterator);
			pLink = pNode->pLinksOut;
		}
	}

	if (pNode) {
		if (pLink) {
			pLink = pLink->pFromPrevious;
			if (!pLink) {
				pLinkCycle = pNode->pLinksOut;
				while (pLinkCycle) {
					pLink = pLinkCycle;
					pLinkCycle = pLink->pFromNext;
				}
			}
		}

		psIterator = pList->startIterator ();
		while (pList->isIteratorValid (psIterator) && (psSelectedNode != pList->getCurrentNode (psIterator))) {
			psIterator = pList->moveForward (psIterator);
		}

		if (pList->isIteratorValid (psIterator)) {
			psSelectedNode = pList->getCurrentNode (psIterator);
		}

		psSelectedLink = pLink;
		RedrawTopology ();
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::ClearSelected (void) {
	psSelectedNode = NULL;
	psSelectedLink = NULL;
	RedrawTopology ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cTopology::GetOverlayStatus (void) {
	return psRenderCurrent->boOverlay;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::SetOverlayStatus (bool boStatus) {
	if (boStatus != psRenderCurrent->boOverlay) {
		psRenderCurrent->boOverlay = boStatus;
		RedrawTopology ();
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::ToggleOverlayStatus (void) {
	psRenderCurrent->boOverlay = !psRenderCurrent->boOverlay;
	RedrawTopology ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
SEC cTopology::ConvertSecurityToInt (char cSecurity) {
	SEC eSecurity;

	switch (cSecurity) {
	case 'U':
	case 'u':
		eSecurity = SEC_U;
		break;
	case 'N':
	case 'n':
		eSecurity = SEC_N;
		break;
	case 'C':
	case 'c':
		eSecurity = SEC_C;
		break;
	case 'S':
	case 's':
		eSecurity = SEC_S;
		break;
	case 'T':
	case 't':
		eSecurity = SEC_TS;
		break;
	case '1':
		eSecurity = SEC_1C;
		break;
	case 'M':
	case 'm':
		eSecurity = SEC_MC;
		break;
	default:
		eSecurity = SEC_INVALID;
		break;
	}

	return eSecurity;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
char cTopology::ConvertIntToSecurity (SEC eSecurity) {
	char cSecurity;

	switch (eSecurity) {
	case SEC_U:
		cSecurity = 'U';
		break;
	case SEC_N:
		cSecurity = 'N';
		break;
	case SEC_C:
		cSecurity = 'C';
		break;
	case SEC_S:
		cSecurity = 'S';
		break;
	case SEC_TS:
		cSecurity = 'T';
		break;
	case SEC_1C:
		cSecurity = '1';
		break;
	case SEC_MC:
		cSecurity = 'M';
		break;
	default:
		cSecurity = 'x';
		break;
	}

	return cSecurity;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cTimer* cTopology::GetTimer (void) {
	return psTimer;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cTopology::GetNextLinkID (void) {
	return nNextLinkID++;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::ResetNextID (void) {
	pList->resetNextID ();
	nNextLinkID = LINKID_START;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::ClearTimerEvents (void) {
	DeleteTimer ();
	CreateTimer ();
}

void cTopology::CreateTimer (void) {
	psTimer = new cTimer (psWindow);

	// Register the load and save routines
	this->psFileHub = psFileHub;
	psFileHub->AddFileRoutine ("Timer", psTimer);

	// Register the various events
	psTimer->RegisterLoadEvent ("NewNode", EventNewNode::Kahlo);
	psTimer->RegisterLoadEvent ("NewLink", EventNewLink::Kahlo);
	psTimer->RegisterLoadEvent ("MoveNode", EventMoveNode::Kahlo);
	psTimer->RegisterLoadEvent ("DeleteNode", EventDeleteNode::Kahlo);
	psTimer->RegisterLoadEvent ("DeleteLink", EventDeleteLink::Kahlo);
	psTimer->RegisterLoadEvent ("PropertiesNode", EventPropertiesNode::Kahlo);
	psTimer->RegisterLoadEvent ("PropertiesLink", EventPropertiesLink::Kahlo);
	psTimer->RegisterLoadEvent ("AssignData", EventAssignData::Kahlo);
	psTimer->RegisterLoadEvent ("RemoveData", EventRemoveData::Kahlo);
	psTimer->RegisterLoadEvent ("SendData", EventSendData::Kahlo);
	psTimer->RegisterLoadEvent ("ChangeView", EventChangeView::Kahlo);
	psTimer->RegisterLoadEvent ("TranslateView", EventTranslateView::Kahlo);
	psTimer->RegisterLoadEvent ("ViewProperties", EventViewProperties::Kahlo);
	psTimer->RegisterLoadEvent ("PropertySetNode", EventPropertySetNode::Kahlo);
	psTimer->RegisterLoadEvent ("PropertySetClearNode", EventPropertySetClearNode::Kahlo);
	psTimer->RegisterLoadEvent ("PropertySetLink", EventPropertySetNode::Kahlo);
	psTimer->RegisterLoadEvent ("PropertySetClearLink", EventPropertySetClearNode::Kahlo);
}

void cTopology::DeleteTimer (void) {
	// Deregister the load and save routines
	psFileHub->RemoveFileRoutine ("Timer");

	delete psTimer;
	psTimer = NULL;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::UpdateAdHocLinks (cTopologyNode* psNode) {
	float fX1;
	float fY1;
	float fZ1;
	float fX2;
	float fY2;
	float fZ2;
	float fDistSquared;
	cTopologyLink*	psLink;
	cTopologyLink*	psLinkFound;
	cTopologyNode*	psNodeLinked;
	cTopologyNode*	psIterator;
	int nLinkID;

	if (pList) {
		fX1 = psNode->fXPos;
		fY1 = psNode->fYPos;
		fZ1 = psNode->fZPos;

		if (psNode->boAdHoc) {
			psIterator = pList->startIterator ();

			while (pList->isIteratorValid (psIterator)) {
				psNodeLinked = pList->getCurrentNode (psIterator);
				if (psNodeLinked->boAdHoc) {
					fX2 = pList->getXPos (psIterator);
					fY2 = pList->getYPos (psIterator);
					fZ2 = pList->getZPos (psIterator);

					fDistSquared = ((fX1 - fX2) * (fX1 - fX2)) + ((fY1 - fY2) * (fY1 - fY2)) + ((fZ1 - fZ2) * (fZ1 - fZ2));

					if (psNodeLinked != psNode) {
						if (((fDistSquared) < (psNode->fRangeTx * psNode->fRangeTx))
							|| ((fDistSquared) < (psNodeLinked->fRangeRx * psNodeLinked->fRangeRx))) {

							/* Add new link if there isn't one already */
							psLink = psNode->pLinksOut;
							psLinkFound = NULL;
							while ((psLink) && (psLinkFound == NULL)) {
								if ((psLink->boAdHoc) && (psLink->psLinkTo == psNodeLinked)) {
									psLinkFound = psLink;
								}

								psLink = psLink->pFromNext;
							}

							if (psLinkFound == NULL) {

								/* Create a new link */
								nLinkID = GetNextLinkID ();

								/* Note that we don't add this in to the timer */
								CreateLink (psNode->getID (), psNodeLinked->getID (), nLinkID);
								if (psSelectedLink) {
									psSelectedLink->boAdHoc = true;
								}
							}
						} else {

							/*
							 * Remove any existing ad hoc links ;
							 * Add new link if there isn't one already
							 */
							psLink = psNode->pLinksOut;
							while (psLink) {
								if ((psLink->boAdHoc) && (psLink->psLinkTo == psNodeLinked)) {

									/* Remove the link */
									psLinkFound = psLink;
									psLink = psLink->pFromNext;
									DeleteLink (psLinkFound);
								} else {
									psLink = psLink->pFromNext;
								}
							}
						}

						if (((fDistSquared) < (psNodeLinked->fRangeTx * psNodeLinked->fRangeTx))
							|| ((fDistSquared) < (psNode->fRangeRx * psNode->fRangeRx))) {

							/* Add new link if there isn't one already */
							psLink = psNodeLinked->pLinksOut;
							psLinkFound = NULL;
							while ((psLink) && (psLinkFound == NULL)) {
								if ((psLink->boAdHoc) && (psLink->psLinkTo == psNode)) {
									psLinkFound = psLink;
								}

								psLink = psLink->pFromNext;
							}

							if (psLinkFound == NULL) {

								/* Create a new link */
								nLinkID = GetNextLinkID ();

								/* Note that we don't add this in to the timer */
								CreateLink (psNodeLinked->getID (), psNode->getID (), nLinkID);
								if (psSelectedLink) {
									psSelectedLink->boAdHoc = true;
								}
							}
						} else {

							/*
							 * Remove any existing ad hoc links ;
							 * Add new link if there isn't one already
							 */
							psLink = psNodeLinked->pLinksOut;
							while (psLink) {
								if ((psLink->boAdHoc) && (psLink->psLinkTo == psNode)) {

									/* Remove the link */
									psLinkFound = psLink;
									psLink = psLink->pFromNext;
									DeleteLink (psLinkFound);
								} else {
									psLink = psLink->pFromNext;
								}
							}
						}
					}
				}

				psIterator = pList->moveForward (psIterator);
			}
		} else {

			/* Remove any ad-hoc links */
			psLink = psNode->pLinksOut;
			while (psLink) {
				if (psLink->boAdHoc) {

					/* Remove the link */
					psLinkFound = psLink;
					psLink = psLink->pFromNext;
					DeleteLink (psLinkFound);
				} else {
					psLink = psLink->pFromNext;
				}
			}

			psLink = psNode->pLinksIn;
			while (psLink) {
				if (psLink->boAdHoc) {

					/* Remove the link */
					psLinkFound = psLink;
					psLink = psLink->pFromNext;
					DeleteLink (psLinkFound);
				} else {
					psLink = psLink->pFromNext;
				}
			}
		}
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::PlotMain (HDC sHDC) {
	uTimePrev = uTime;
	uTime = psTimer->GetTime ();
	fTimeStep = (float) (uTime - uTimePrev);

	psRenderCurrent->PlotMain (pList, psSelectedNode, psSelectedLink, fTimeStep);

	/* Plot the HUD */
	RenderHUD ();

#ifdef _DEBUG
	GLenum eError;
	eError = glGetError ();
	if (eError != GL_NO_ERROR) {
		printf ("Error %d\n", eError);
	}

#endif
	glFlush ();

	SwapBuffers (sHDC);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopology::SetNodeTextureInfo (int nNodeTextureStart, int nNodeTypes) {
	this->nNodeTypes = nNodeTypes;

	psRenderGeo->SetNodeTextureStart (nNodeTextureStart);
	psRenderNet->SetNodeTextureStart (nNodeTextureStart);
}
