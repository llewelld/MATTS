/*$T MATTS/cMap.cpp GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cMap.cpp ;
 * Last Modified: 20/12/05 ;
 * ;
 * Purpose: Render geographical map backgrounds ;
 */

#include "cMap.h"
#include "functy/cartesian.h"
#include "functy/spherical.h"
#include "functy/textures.h"

#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glut.h>

#pragma warning(disable : 4267)
#pragma warning(disable : 4244)
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#pragma warning(default : 4267)
#pragma warning(default : 4244)

#define FILE_BUFFER_SIZE (1024)
#define ACCURACY_NEW (1.0/100.0)

static float const gfMapHeights[MAPHEIGHT_SIZE][MAPHEIGHT_SIZE] =
{
	{ 0.00f, 0.00f, 0.00f, 0.00f, 0.00f },
	{ 0.00f, 0.00f, 0.00f, 0.00f, 0.00f },
	{ 0.00f, 0.00f, 0.00f, 0.00f, 0.00f },
	{ 0.00f, 0.00f, 0.00f, 0.00f, 0.00f },
	{ 0.00f, 0.00f, 0.00f, 0.00f, 0.00f }
};

static GLfloat const MapAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
static GLfloat const MapDiffuse[] = { 0.0f, 0.0f, 0.0f, 1.0f };
static GLfloat const MapSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static GLfloat const MapShininess = 100.0f;

static GLfloat gMatDiffuse[] = { 0.5, 0.5, 1.0, 1.0 };
static GLfloat gBoardSpecular[] = { 0.7, 0.7, 0.7, 0.9 };
static GLfloat gBoardShininess[] = { 10.0 };
static GLfloat gGridColour[] = { 1.0, 1.0, 1.0, 1.0 };

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cMap::cMap (cTexture* psTexture, cObject* psObject) {
	int nScene;
	//FuncPersist * psFuncData;

	psHandler = new cMapLoadHandler (this);

	this->psTexture = psTexture;
	this->psObject = psObject;

  fXMin = -5.0;
  fYMin = -5.0;
  fZMin = -5.0;
  fXWidth = 10.0;
  fYWidth = 10.0;
  fZWidth = 10.0;
	boRangeChange = false;
	nFunctionNum = 0;
	nFunctionMax = 5;
	fFunctionTime = 0.0;
	apsFuncData = (FuncPersist **)malloc (sizeof (FuncPersist **) * nFunctionMax);

	LoadFile ("Resources\\Scenery.xml");
	//psFuncData = AddFunction (FUNCTYPE_CARTESIAN, ACCURACY_NEW);
	//SetFunction ("(sin(x*6)+sin(y*7))/10", psFuncData);
	//PopulateVertices (psFuncData);
	//SetFunctionColours ("(sin(x*4)+1)/2", "(sin(y*3)+1)/2", "((sin(y*5)*sin(x*4))+1)/2", "1.0", psFuncData);
	//SetFunctionRange (-5.0, -5.0, -5.0, 10.0, 10.0, 10.0, psFuncData);

	for (nScene = 0; nScene < SCENE_NUM; nScene++) {
		afMapHalfSize[nScene] = MAP_HALFSIZE_LOW;
	}

	afMapHalfSize[SCENE_TRON] = MAP_HALFSIZE_SMALL;
	afMapHalfSize[SCENE_STJOHN] = MAP_HALFSIZE_HIGH;

	aunSceneMapTexture[SCENE_TRON] = TEXNAME_TRON;
	aunSceneMapTexture[SCENE_JMUMAP] = TEXNAME_JMUMAP;
	aunSceneMapTexture[SCENE_LIVERPOOL] = TEXNAME_JMUMAP;
	aunSceneMapTexture[SCENE_STJOHN] = TEXNAME_RENDERMAP;

	for (nScene = 0; nScene < (int) SCENE_NUM; nScene++) {
		psObject->NewList ((OBJECT) ((int) OBJECT_MAPSTART + nScene));
		RenderMapInit ((SCENE) nScene);
		psObject->EndList ((OBJECT) ((int) OBJECT_MAPSTART + nScene));
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cMap::~cMap () {
	int nFunction;

	for (nFunction = 0; nFunction < nFunctionNum; nFunction++) {
    DeleteFuncPersist (apsFuncData[nFunction]);
	}
	nFunctionNum = 0;
	free (apsFuncData);
	delete psHandler;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cMapLoadHandler * cMap::GetLoadHandler () {
	return psHandler;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
FuncPersist * cMap::AddFunction (FUNCTYPE eType, double fAccuracy) {
	FuncPersist * psFuncData;

	if (nFunctionNum >= nFunctionMax) {
		nFunctionMax += 5;
		apsFuncData = (FuncPersist **)realloc (apsFuncData, sizeof (FuncPersist *) * nFunctionMax);
	}

	psFuncData = NewFuncPersist (eType, fAccuracy);
	apsFuncData[nFunctionNum] = psFuncData;
	nFunctionNum++;

	return psFuncData;
}

/*
 =======================================================================================================================
 *  Creating and rendering the map interface
 =======================================================================================================================
 */
void cMap::RenderMapInit (SCENE eScene) {
	int nXSegment;
	int nYSegment;
	float fHeight;
	double fX;
	double fY;

	glMaterialfv (GL_FRONT, GL_AMBIENT, MapAmbient);
	glMaterialfv (GL_FRONT, GL_DIFFUSE, MapDiffuse);
	glMaterialfv (GL_FRONT, GL_SPECULAR, MapSpecular);
	glMaterialf (GL_FRONT, GL_SHININESS, MapShininess);

	glBindTexture (GL_TEXTURE_2D, psTexture->GetTexture (aunSceneMapTexture[eScene]));

	/*
	 * glTexCoord2f (0.0f, 0.0f);
	 * glVertex3f (-MAP_HALFSIZE, -MAP_HALFSIZE, 0.0f);
	 * glTexCoord2f (0.0f, MAP_REPEAT);
	 * glVertex3f (-MAP_HALFSIZE, MAP_HALFSIZE, 0.0f);
	 * glTexCoord2f (MAP_REPEAT, MAP_REPEAT);
	 * glVertex3f (MAP_HALFSIZE, MAP_HALFSIZE, 0.0f);
	 * glTexCoord2f (MAP_REPEAT, 0.0f);
	 * glVertex3f (MAP_HALFSIZE, -MAP_HALFSIZE, 0.0f);
	 * ;
	 * glDisable (GL_TEXTURE_2D);
	 */
	for (nXSegment = 0; nXSegment < MAP_SEGMENTS; nXSegment++) {
		glBegin (GL_QUAD_STRIP);
		for (nYSegment = 0; nYSegment < MAP_SEGMENTS; nYSegment++) {

			/*
			 * fHeight = MAP_HEIGHT((nXSegment - (MAP_SEGMENTS / 2.0) + 1.0),(nYSegment
			 * (MAP_SEGMENTS / 2.0) + 1.0));
			 * glTexCoord2f ((MAP_REPEAT/((float)MAP_SEGMENTS))*((float)(nXSegment + 1)),
			 * (MAP_REPEAT/((float)MAP_SEGMENTS))*((float)(nYSegment + 1)));
			 * glVertex3f ((2.0f * ((float)(nXSegment + 1)) * afMapHalfSize[eScene]
			 * ((float)MAP_SEGMENTS)) - afMapHalfSize[eScene], (2.0f * ((float)(nYSegment 1))
			 * afMapHalfSize[eScene] / ((float)MAP_SEGMENTS)) - afMapHalfSize[eScene], fHeight);
			 */
			fX = (nXSegment - (MAP_SEGMENTS / 2.0) + 1.0);
			fY = (nYSegment - (MAP_SEGMENTS / 2.0));
			fHeight = MAP_HEIGHT (fX, fY);
			glTexCoord2f ((MAP_REPEAT / ((float) MAP_SEGMENTS)) * ((float) (nXSegment + 1)),
						  (MAP_REPEAT / ((float) MAP_SEGMENTS)) * ((float) nYSegment));
			glNormal3f (-MAP_NORMAL_X (fX, fY), -MAP_NORMAL_Y (fX, fY), -1.0);
			glVertex3f ((2.0f * ((float) (nXSegment + 1)) * afMapHalfSize[eScene] / ((float) MAP_SEGMENTS)) -
						afMapHalfSize[eScene], (2.0f * ((float) nYSegment) * afMapHalfSize[eScene] /
						((float) MAP_SEGMENTS)) - afMapHalfSize[eScene], fHeight);

			fX = (nXSegment - (MAP_SEGMENTS / 2.0));
			fY = (nYSegment - (MAP_SEGMENTS / 2.0));
			fHeight = MAP_HEIGHT (fX, fY);
			glTexCoord2f ((MAP_REPEAT / ((float) MAP_SEGMENTS)) * ((float) nXSegment),
						  (MAP_REPEAT / ((float) MAP_SEGMENTS)) * ((float) nYSegment));
			glNormal3f (-MAP_NORMAL_X (fX, fY), -MAP_NORMAL_Y (fX, fY), -1.0);
			glVertex3f ((2.0f * ((float) nXSegment) * afMapHalfSize[eScene] / ((float) MAP_SEGMENTS)) -
						afMapHalfSize[eScene], (2.0f * ((float) nYSegment) * afMapHalfSize[eScene] /
						((float) MAP_SEGMENTS)) - afMapHalfSize[eScene], fHeight);

			/*
			 * fHeight = MAP_HEIGHT((nXSegment - (MAP_SEGMENTS / 2.0)),(nYSegment
			 * (MAP_SEGMENTS / 2.0) + 1.0));
			 * glTexCoord2f ((MAP_REPEAT/((float)MAP_SEGMENTS))*((float)nXSegment),
			 * (MAP_REPEAT/((float)MAP_SEGMENTS))*((float)(nYSegment + 1)));
			 * glVertex3f ((2.0f * ((float)nXSegment) * afMapHalfSize[eScene]
			 * ((float)MAP_SEGMENTS)) - afMapHalfSize[eScene], (2.0f * ((float)(nYSegment 1))
			 * afMapHalfSize[eScene] / ((float)MAP_SEGMENTS)) - afMapHalfSize[eScene], fHeight);
			 */
		}

		glEnd ();
	}

	/*
	 * glEnable (GL_TEXTURE_2D);
	 */
}

/*
 =======================================================================================================================
 *  Creating and rendering the map interface
 =======================================================================================================================
 */
void cMap::RenderMap (SCENE eScene) {
  glDisable (GL_CULL_FACE);
  glEnable (GL_DEPTH_TEST);
  glLightModeli (GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

	//glScalef (1.0f, 1.0f, 0.3f);
	glRotatef (180.0, 1.0, 0.0, 0.0);
	DrawGraphs ();
	//psObject->CallList ((OBJECT) ((int) OBJECT_MAPSTART + eScene));

	glPolygonMode (GL_FRONT, GL_FILL);
  glLightModeli (GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
  glEnable (GL_CULL_FACE);
}

/*
 =======================================================================================================================
 *  Creating and rendering the map interface
 =======================================================================================================================
 */
void cMap::RenderSelectMap (SCENE eScene) {
	int nXSegment;
	int nYSegment;
	float fHeight;
  FUNCTYPE eFuncType;

	if (apsFuncData && apsFuncData[0]) {
	  eFuncType = GetFunctionType (apsFuncData[0]);
	}
	else {
	  eFuncType = FUNCTYPE_INVALID;
	}

	/* if (!boScreenShotTemp) { */
	glColor3f (1.0f, 1.0f, 1.0f);

	/*
	 * } ;
	 * else { ;
	 * glColor3f (0.3f, 0.3f, 0.3f);
	 * } ;
	 * glBindTexture (GL_TEXTURE_2D, psTexture->GetTexture (aunSceneMapTexture[eScene]));
	 * glTexCoord2f (0.0f, 0.0f);
	 * glVertex3f (-MAP_HALFSIZE, -MAP_HALFSIZE, 0.0f);
	 * glTexCoord2f (0.0f, MAP_REPEAT);
	 * glVertex3f (-MAP_HALFSIZE, MAP_HALFSIZE, 0.0f);
	 * glTexCoord2f (MAP_REPEAT, MAP_REPEAT);
	 * glVertex3f (MAP_HALFSIZE, MAP_HALFSIZE, 0.0f);
	 * glTexCoord2f (MAP_REPEAT, 0.0f);
	 * glVertex3f (MAP_HALFSIZE, -MAP_HALFSIZE, 0.0f);
	 */
	glDisable (GL_TEXTURE_2D);

	glTranslatef (0.0, 0.0, -(Z_SIZE / 1.0));

	if (eFuncType == FUNCTYPE_CARTESIAN) {
		glRotatef (180.0, 1.0, 0.0, 0.0);
		DrawGraph (apsFuncData[0]);
	}
	else {
		for (nXSegment = 0; nXSegment < MAP_SEGMENTS; nXSegment++) {
			glBegin (GL_QUAD_STRIP);
			for (nYSegment = 0; nYSegment < MAP_SEGMENTS; nYSegment++) {

				/*
				 * glTexCoord2f ((MAP_REPEAT/((float)MAP_SEGMENTS))*((float)(nXSegment + 1)),
				 * (MAP_REPEAT/((float)MAP_SEGMENTS))*((float)(nYSegment + 1)));
				 * glVertex3f ((2.0f * ((float)(nXSegment + 1)) * MAP_HALFSIZE
				 * ((float)MAP_SEGMENTS)) - MAP_HALFSIZE, (2.0f * ((float)(nYSegment + 1))
				 * MAP_HALFSIZE / ((float)MAP_SEGMENTS)) - MAP_HALFSIZE, 0.0f);
				 */
				fHeight = MAPHEIGHT_SCALE * gfMapHeights[((nXSegment + 1) % MAPHEIGHT_SIZE)][(nYSegment % MAPHEIGHT_SIZE)];

				/*
				 * glTexCoord2f ((MAP_REPEAT/((float)MAP_SEGMENTS))*((float)(nXSegment + 1)), (MAP_REPEAT/((float)MAP_SEGMENTS))*((float)nYSegment));
				 */
				glVertex3f ((2.0f * ((float) (nXSegment + 1)) * afMapHalfSize[eScene] / ((float) MAP_SEGMENTS)) -
							afMapHalfSize[eScene], (2.0f * ((float) nYSegment) * afMapHalfSize[eScene] /
							((float) MAP_SEGMENTS)) - afMapHalfSize[eScene], fHeight);

				fHeight = MAPHEIGHT_SCALE * gfMapHeights[(nXSegment % MAPHEIGHT_SIZE)][(nYSegment % MAPHEIGHT_SIZE)];

				/*
				 * glTexCoord2f ((MAP_REPEAT/((float)MAP_SEGMENTS))*((float)nXSegment), (MAP_REPEAT/((float)MAP_SEGMENTS))*((float)nYSegment));
				 */
				glVertex3f ((2.0f * ((float) nXSegment) * afMapHalfSize[eScene] / ((float) MAP_SEGMENTS)) -
							afMapHalfSize[eScene], (2.0f * ((float) nYSegment) * afMapHalfSize[eScene] /
							((float) MAP_SEGMENTS)) - afMapHalfSize[eScene], fHeight);

				/*
				 * glTexCoord2f ((MAP_REPEAT/((float)MAP_SEGMENTS))*((float)nXSegment),
				 * (MAP_REPEAT/((float)MAP_SEGMENTS))*((float)(nYSegment + 1)));
				 * glVertex3f ((2.0f * ((float)nXSegment) * MAP_HALFSIZE / ((float)MAP_SEGMENTS))
				 * - MAP_HALFSIZE, (2.0f * ((float)(nYSegment + 1)) * MAP_HALFSIZE
				 * ((float)MAP_SEGMENTS)) - MAP_HALFSIZE, 0.0f);
				 */
			}

			glEnd ();
		}
	}

	glEnable (GL_TEXTURE_2D);
}

/*
 =======================================================================================================================
 *  Render the graphs
 =======================================================================================================================
 */
void cMap::DrawGraphs () {
	int nFunction;

	//for (nFunction = (nFunctionNum - 1); nFunction >= 0; nFunction--) {
	for (nFunction = 0; nFunction < nFunctionNum; nFunction++) {
    DrawGraph (apsFuncData[nFunction]);
	}
}

/*
 =======================================================================================================================
 *  Rendering an individual graph
 =======================================================================================================================
 */
void cMap::DrawGraph (FuncPersist * psFuncData) {
  int nXIndex;
  GLfloat * afVertices;
  GLfloat * afNormals;
  GLushort * auIndices;
  GLfloat * afColours;
  GLfloat * afTextureCoords;
  int nYVertices;
  int nXVertices;
  GLfloat afGraphColour[4];
  bool boColours;
  FUNCTYPE eFuncType;
  GLuint uTexture;
  static GLdouble afClipEquations[4][4] = {{-1.0, 0.0, 0.0, AXIS_XHSIZE},
    {1.0, 0.0, 0.0, AXIS_XHSIZE},
    {0.0, -1.0, 0.0, AXIS_YHSIZE},
    {0.0, 1.0, 0.0, AXIS_YHSIZE}};

  eFuncType = GetFunctionType (psFuncData);

  afVertices = GetVertices (psFuncData);
  afNormals = GetNormals (psFuncData);
  auIndices = GetIndices (psFuncData);
  GetVertexDimensions (& nXVertices, & nYVertices, psFuncData);

  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, gBoardSpecular);
  glMaterialfv (GL_FRONT_AND_BACK, GL_SHININESS, gBoardShininess);
  glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, gMatDiffuse);

  glPushMatrix();

  if (eFuncType == FUNCTYPE_SPHERICAL) {
    // Enable the spherical clipping planes
    glClipPlane (GL_CLIP_PLANE0, afClipEquations[0]);
    glClipPlane (GL_CLIP_PLANE1, afClipEquations[1]);
    glClipPlane (GL_CLIP_PLANE2, afClipEquations[2]);
    glClipPlane (GL_CLIP_PLANE3, afClipEquations[3]);
    glEnable (GL_CLIP_PLANE0);
    glEnable (GL_CLIP_PLANE1);
    glEnable (GL_CLIP_PLANE2);
    glEnable (GL_CLIP_PLANE3);

    RecentreGraph (psFuncData);
  }

  boColours = GetColour (afGraphColour, psFuncData);

	uTexture = GetFunctionTexture (psFuncData);
	if (uTexture != TEXTURE_NONE) {
		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable (GL_TEXTURE_2D);
	  glBindTexture (GL_TEXTURE_2D, uTexture);
    glEnableClientState (GL_TEXTURE_COORD_ARRAY);
	}
  afTextureCoords = GetTextureCoords (psFuncData);

  glEnableClientState (GL_VERTEX_ARRAY);
  glEnableClientState (GL_NORMAL_ARRAY);

  if (boColours) {
    glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, afGraphColour);
		glColor4fv (afGraphColour);

    for (nXIndex = 0; nXIndex < nXVertices - 1; nXIndex++) {
      glVertexPointer (3, GL_FLOAT, 0, afVertices 
        + (nXIndex * (nYVertices * 3)));
      glNormalPointer (GL_FLOAT, 0, afNormals 
        + (nXIndex * (nYVertices * 3)));
			if (uTexture != TEXTURE_NONE) {
		    glTexCoordPointer (2, GL_FLOAT, 0, afTextureCoords
		      + (nXIndex * (nYVertices * 2)));
		  }
      glDrawElements (GL_TRIANGLE_STRIP, nYVertices * 2, 
        GL_UNSIGNED_SHORT, auIndices);
    }
  }
  else {
    afColours = GetColours (psFuncData);
    glEnable (GL_COLOR_MATERIAL);
    glEnableClientState (GL_COLOR_ARRAY);

    for (nXIndex = 0; nXIndex < nXVertices - 1; nXIndex++) {
      glVertexPointer (3, GL_FLOAT, 0, afVertices 
        + (nXIndex * (nYVertices * 3)));
      glNormalPointer (GL_FLOAT, 0, afNormals 
        + (nXIndex * (nYVertices * 3)));
      glColorPointer (4, GL_FLOAT, 0, afColours
        + (nXIndex * (nYVertices * 4)));
			if (uTexture != TEXTURE_NONE) {
		    glTexCoordPointer (2, GL_FLOAT, 0, afTextureCoords
		      + (nXIndex * (nYVertices * 2)));
		  }
      glDrawElements (GL_TRIANGLE_STRIP, nYVertices * 2, 
        GL_UNSIGNED_SHORT, auIndices);
    }
    glDisableClientState (GL_COLOR_ARRAY);

    glDisable (GL_COLOR_MATERIAL);
  }
  glDisableClientState (GL_VERTEX_ARRAY);
  glDisableClientState (GL_NORMAL_ARRAY);

	if (uTexture != TEXTURE_NONE) {
    glDisableClientState (GL_TEXTURE_COORD_ARRAY);
		glDisable (GL_TEXTURE_2D);
		glDisable (GL_BLEND);
	}

  if (eFuncType == FUNCTYPE_SPHERICAL) {
    // Disable the spherical clipping planes
    glDisable (GL_CLIP_PLANE0);
    glDisable (GL_CLIP_PLANE1);
    glDisable (GL_CLIP_PLANE2);
    glDisable (GL_CLIP_PLANE3);
  }

  glPopMatrix ();

  glDisable (GL_BLEND);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cMap::RecentreGraph (FuncPersist * psFuncData) {
  GLdouble afCentre[3];
  GLdouble fXMove;
  GLdouble fYMove;
  GLdouble fZMove;
  double afRange[6];

  SphericalGetCentre (afCentre, psFuncData);
  GetFunctionRange (afRange, psFuncData);

  fXMove = (((afCentre[0] - afRange[0]) * (2 * AXIS_XHSIZE)) / (afRange[3]));
  fYMove = (((afCentre[1] - afRange[1]) * (2 * AXIS_YHSIZE)) / (afRange[4]));
  fZMove = (((afCentre[2] - afRange[2]) * (2 * AXIS_ZHSIZE)) / (afRange[5]));

  glTranslated (fXMove, fYMove, fZMove);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cMap::TransferFunctionRange (FuncPersist * psFuncData) {
  SetFunctionRange (fXMin, fYMin, fZMin, fXWidth, fYWidth, fZWidth, psFuncData);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cMap::GetVisRange (double * afRange) {
  if (afRange) {
    afRange[0] = fXMin;
    afRange[1] = fYMin;
    afRange[2] = fZMin;
    afRange[3] = fXWidth;
    afRange[4] = fYWidth;
    afRange[5] = fZWidth;
  }
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cMap::SetVisRange (double * afRange) {
	int nFunction;

  if (afRange) {
    fXMin = afRange[0];
    fYMin = afRange[1];
    fZMin = afRange[2];
    fXWidth = afRange[3];
    fYWidth = afRange[4];
    fZWidth = afRange[5];

    boRangeChange = TRUE;

    // We need to set the position every function
		for (nFunction = 0; nFunction < nFunctionNum; nFunction++) {
			DrawGraph (apsFuncData[nFunction]);
			TransferFunctionRange (apsFuncData[nFunction]);
		}
	}
}

void cMap::Update (unsigned long uTime) {
  FUNCTYPE eFuncType;
	int nFunction;
	double fTimeChange;

	fTimeChange = (((double)uTime) / 1000.0) - fFunctionTime;
  fFunctionTime = ((double)uTime) / 1000.0;
  if (fTimeChange > 0.005f) {
    // Check all of the functions in case they need animating
		for (nFunction = 0; nFunction < nFunctionNum; nFunction++) {
      eFuncType = GetFunctionType (apsFuncData[nFunction]);

      if (eFuncType == FUNCTYPE_SPHERICAL) {
        SphericalSetFunctionTime (fFunctionTime, apsFuncData[nFunction]);
        SphericalUpdateCentre (apsFuncData[nFunction]);
      }

      if (GetTimeDependent (apsFuncData[nFunction])) {
        SetFunctionTime (fFunctionTime, apsFuncData[nFunction]);
        PopulateVertices (apsFuncData[nFunction]);
      }
		}
  }
}

/*
 =======================================================================================================================
 *  Delete all of the existing functions
 =======================================================================================================================
 */
void cMap::RemoveAllFunctions () {
	int nFunction;

	for (nFunction = 0; nFunction < nFunctionNum; nFunction++) {
    DeleteFuncPersist (apsFuncData[nFunction]);
	}
	nFunctionNum = 0;
	if (apsFuncData) {
		free (apsFuncData);
	}

	nFunctionNum = 0;
	nFunctionMax = 5;
	fFunctionTime = 0.0;
	apsFuncData = (FuncPersist **)malloc (sizeof (FuncPersist **) * nFunctionMax);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cMap::LoadFile (const char *szFilename) {
	SAX2XMLReader* psParser;

	XMLPlatformUtils::Initialize ();

	psParser = XMLReaderFactory::createXMLReader ();

	psParser->setContentHandler (psHandler);
	psParser->setErrorHandler (psHandler);

	psParser->parse (szFilename);

	delete psParser;
	XMLPlatformUtils::Terminate ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
double cMap::GetFunctionZ (double fX, double fY) {
	double fZScreen = 0.0f;
	FUNCTYPE eFuncType;

	if (apsFuncData && apsFuncData[0]) {
		eFuncType = GetFunctionType (apsFuncData[0]);

		if (eFuncType == FUNCTYPE_CARTESIAN) {
			fZScreen = CartesianGetFunctionZ (fX, fY, apsFuncData[0]);
		}
	}

	return fZScreen;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
FuncPersist ** cMap::GetFunctionList () {
	return apsFuncData;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cMap::GetFunctionNum () {
	return nFunctionNum;
}










/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cMapLoadHandler::cMapLoadHandler (cMap * psMap) {
	sLoadText = "";
	nLoadLevel = -1;
	this->psMap = psMap;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cMapLoadHandler::~cMapLoadHandler () {
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cMapLoadHandler::Save (FILE * hFile, int nIndent) {
	bool boSuccess = TRUE;
	FuncPersist ** apsFuncData;
	int nFunctionNum;
	int nFunction;

  //bool boValue;
  double fValue;
  double afRange[6];
  char const * szValue;
  //float fRadius;
  //float fRotation;
  //float fElevation;
  char * szEscaped;
  FUNCTYPE eFuncType;

  // Preamble
  fprintf (hFile, "%*s<functy>\n", nIndent * 2, " ");
	nIndent++;

  //// System info
  //fprintf (hFile, "%*s<settings>\n", nIndent * 2, " ");
  //boValue = psMap-> GetDrawAxes (psFunctyData->psVisData);
  //fprintf (hFile, "%*s<show-axes bool=\"%d\"/>\n", nIndent * 2, " ", boValue);
  //boValue = GetClearWhite (psFunctyData->psVisData);
  //fprintf (hFile, "%*s<invert bool=\"%d\"/>\n", nIndent * 2, " ", boValue);
  //boValue = GetWireframe (psFunctyData->psVisData);
  //fprintf (hFile, "%*s<wireframe bool=\"%d\"/>\n", nIndent * 2, " ", boValue);

  //if (boConfigure) {
  //  boValue = GetSpin (psFunctyData->psVisData);
  //  fprintf (hFile, "%*s<spin bool=\"%d\"/>\n", nIndent * 2, " ", boValue);
  //  boValue = GetFullScreen (psFunctyData->psVisData);
  //  fprintf (hFile, "%*s<fullscreen bool=\"%d\"/>\n", nIndent * 2, " ", boValue);

  //  GetView (& fRadius, & fRotation, & fElevation, psFunctyData->psVisData);
  //  fprintf (hFile, "%*s<radius double=\"%f\"/>\n", nIndent * 2, " ", fRadius);
  //  fprintf (hFile, "%*s<rotation double=\"%f\"/>\n", nIndent * 2, " ", fRotation);
  //  fprintf (hFile, "%*s<elevation double=\"%f\"/>\n", nIndent * 2, " ", fElevation);
  //}
  //fprintf (hFile, "%*s</settings>\n", nIndent * 2, " ");

  // Range
  fprintf (hFile, "%*s<range>\n", nIndent * 2, " ");
	nIndent++;
  psMap->GetVisRange (afRange);

  fprintf (hFile, "%*s<min>\n", nIndent * 2, " ");
	nIndent++;
  fprintf (hFile, "%*s<coord x=\"%f\" y=\"%f\" z=\"%f\"/>\n", nIndent * 2, " ", afRange[0], afRange[1], afRange[2]);
	nIndent--;
  fprintf (hFile, "%*s</min>\n", nIndent * 2, " ");
  fprintf (hFile, "%*s<width>\n", nIndent * 2, " ");
	nIndent++;
  fprintf (hFile, "%*s<coord x=\"%f\" y=\"%f\" z=\"%f\"/>\n", nIndent * 2, " ", afRange[3], afRange[4], afRange[5]);
	nIndent--;
  fprintf (hFile, "%*s</width>\n", nIndent * 2, " ");
	nIndent--;
  fprintf (hFile, "%*s</range>\n", nIndent * 2, " ");

  // Functions
	apsFuncData = psMap->GetFunctionList ();
	nFunctionNum = psMap->GetFunctionNum ();

	for (nFunction = 0; nFunction < nFunctionNum; nFunction++) {
    eFuncType = GetFunctionType (apsFuncData[nFunction]);
    switch (eFuncType) {
    case FUNCTYPE_CARTESIAN:
      fprintf (hFile, "%*s<function type=\"cartesian\">\n", nIndent * 2, " ");
      break;
    case FUNCTYPE_SPHERICAL:
      fprintf (hFile, "%*s<function type=\"spherical\">\n", nIndent * 2, " ");
      break;
    default:
      fprintf (hFile, "%*s<function>\n", nIndent * 2, " ");
      break;
    }
		nIndent++;

    szValue = GetFunctionString (apsFuncData[nFunction]);
		szEscaped =  g_markup_escape (szValue);
    fprintf (hFile, "%*s<equation>%s</equation>\n", nIndent * 2, " ", szEscaped);
    g_free (szEscaped);

    szValue = GetRedString (apsFuncData[nFunction]);
    szEscaped = g_markup_escape (szValue);
    fprintf (hFile, "%*s<red>%s</red>\n", nIndent * 2, " ", szEscaped);
    g_free (szEscaped);

    szValue = GetGreenString (apsFuncData[nFunction]);
    szEscaped = g_markup_escape (szValue);
    fprintf (hFile, "%*s<green>%s</green>\n", nIndent * 2, " ", szEscaped);
    g_free (szEscaped);

    szValue = GetBlueString (apsFuncData[nFunction]);
    szEscaped = g_markup_escape (szValue);
    fprintf (hFile, "%*s<blue>%s</blue>\n", nIndent * 2, " ", szEscaped);
    g_free (szEscaped);

    szValue = GetAlphaString (apsFuncData[nFunction]);
    szEscaped = g_markup_escape (szValue);
    fprintf (hFile, "%*s<alpha>%s</alpha>\n", nIndent * 2, " ", szEscaped);
    g_free (szEscaped);

    fValue = GetFunctionAccuracy (apsFuncData[nFunction]);
    fprintf (hFile, "%*s<accuracy double=\"%f\"/>\n", nIndent * 2, " ", fValue);

    if (eFuncType == FUNCTYPE_SPHERICAL) {
      szValue = SphericalGetXCentreString (apsFuncData[nFunction]);
      szEscaped = g_markup_escape (szValue);
      fprintf (hFile, "%*s<centre-x>%s</centre-x>\n", nIndent * 2, " ", szEscaped);
      g_free (szEscaped);

      szValue = SphericalGetYCentreString (apsFuncData[nFunction]);
      szEscaped = g_markup_escape (szValue);
      fprintf (hFile, "%*s<centre-y>%s</centre-y>\n", nIndent * 2, " ", szEscaped);
      g_free (szEscaped);

      szValue = SphericalGetZCentreString (apsFuncData[nFunction]);
      szEscaped = g_markup_escape (szValue);
      fprintf (hFile, "%*s<centre-z>%s</centre-z>\n", nIndent * 2, " ", szEscaped);
      g_free (szEscaped);
    }

		szValue = GetTexFileString (apsFuncData[nFunction]);
    szEscaped = g_markup_escape (szValue);
    fprintf (hFile, "%*s<texture-file>%s</texture-file>\n", nIndent * 2, " ", szEscaped);
		g_free (szEscaped);

		szValue = GetTexXScaleString (apsFuncData[nFunction]);
    szEscaped = g_markup_escape (szValue);
    fprintf (hFile, "%*s<texture-x-scale>%s</texture-x-scale>\n", nIndent * 2, " ", szEscaped);
		g_free (szEscaped);

		szValue = GetTexYScaleString (apsFuncData[nFunction]);
    szEscaped = g_markup_escape (szValue);
    fprintf (hFile, "%*s<texture-y-scale>%s</texture-y-scale>\n", nIndent * 2, " ", szEscaped);
		g_free (szEscaped);

		szValue = GetTexXOffsetString (apsFuncData[nFunction]);
    szEscaped = g_markup_escape (szValue);
    fprintf (hFile, "%*s<texture-x-offset>%s</texture-x-offset>\n", nIndent * 2, " ", szEscaped);
		g_free (szEscaped);

		szValue = GetTexYOffsetString (apsFuncData[nFunction]);
    szEscaped = g_markup_escape (szValue);
    fprintf (hFile, "%*s<texture-y-offset>%s</texture-y-offset>\n", nIndent * 2, " ", szEscaped);
		g_free (szEscaped);

		nIndent--;
    fprintf (hFile, "%*s</function>\n", nIndent * 2, " ");
  }

  // Conclusion
	nIndent--;
  fprintf (hFile, "%*s</functy>\n", nIndent * 2, " ");
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cMapLoadHandler::LoadStart (char const * szChunkName) {
	sLoadText = "";
	nLoadLevel = -1;
	psMap->RemoveAllFunctions ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cMapLoadHandler::LoadEnd (char const * szChunkName) {
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cMapLoadHandler::startElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname,
	const Attributes&  attrs) {
	char* szElementName;
	szElementName = XMLString::transcode (localname);

	nLoadLevel++;

	sLoadText = "";

  if ((nLoadLevel >= 0) && (nLoadLevel < MAX_LEVELS)) {
    if (_stricmp (szElementName, "functy") == 0) {
      nLoadLevel = -1;
      eLoadTag[0] = FILESAVETAG_INVALID;
    }
    else if (_stricmp (szElementName, "settings") == 0) {
      eLoadTag[nLoadLevel] = FILESAVETAG_SETTINGS;
    }
    else if (_stricmp (szElementName, "range") == 0) {
      eLoadTag[nLoadLevel] = FILESAVETAG_RANGE;
    }
    else if (_stricmp (szElementName, "function") == 0) {
      eLoadTag[nLoadLevel] = FILESAVETAG_FUNCTION;
    }
    else if (_stricmp (szElementName, "min") == 0) {
      eLoadTag[nLoadLevel] = FILESAVETAG_MIN;
    }
    else if (_stricmp (szElementName, "width") == 0) {
      eLoadTag[nLoadLevel] = FILESAVETAG_WIDTH;
    }
    else if (_stricmp (szElementName, "show-axes") == 0) {
      eLoadTag[nLoadLevel] = FILESAVETAG_SHOWAXES;
    }
    else if (_stricmp (szElementName, "spin") == 0) {
      eLoadTag[nLoadLevel] = FILESAVETAG_SPIN;
    }
    else if (_stricmp (szElementName, "invert") == 0) {
      eLoadTag[nLoadLevel] = FILESAVETAG_INVERT;
    }
    else if (_stricmp (szElementName, "wireframe") == 0) {
      eLoadTag[nLoadLevel] = FILESAVETAG_WIREFRAME;
    }
    else if (_stricmp (szElementName, "fullscreen") == 0) {
      eLoadTag[nLoadLevel] = FILESAVETAG_FULLSCREEN;
    }
    else if (_stricmp (szElementName, "radius") == 0) {
      eLoadTag[nLoadLevel] = FILESAVETAG_RADIUS;
    }
    else if (_stricmp (szElementName, "rotation") == 0) {
      eLoadTag[nLoadLevel] = FILESAVETAG_ROTATION;
    }
    else if (_stricmp (szElementName, "elevation") == 0) {
      eLoadTag[nLoadLevel] = FILESAVETAG_ELEVATION;
    }
    else if (_stricmp (szElementName, "equation") == 0) {
      eLoadTag[nLoadLevel] = FILESAVETAG_EQUATION;
    }
    else if (_stricmp (szElementName, "red") == 0) {
      eLoadTag[nLoadLevel] = FILESAVETAG_RED;
    }
    else if (_stricmp (szElementName, "green") == 0) {
      eLoadTag[nLoadLevel] = FILESAVETAG_GREEN;
    }
    else if (_stricmp (szElementName, "blue") == 0) {
      eLoadTag[nLoadLevel] = FILESAVETAG_BLUE;
    }
    else if (_stricmp (szElementName, "alpha") == 0) {
      eLoadTag[nLoadLevel] = FILESAVETAG_ALPHA;
    }
    else if (_stricmp (szElementName, "accuracy") == 0) {
      eLoadTag[nLoadLevel] = FILESAVETAG_ACCURACY;
    }
    else if (_stricmp (szElementName, "centre-x") == 0) {
      eLoadTag[nLoadLevel] = FILESAVETAG_CENTREX;
    }
    else if (_stricmp (szElementName, "centre-y") == 0) {
      eLoadTag[nLoadLevel] = FILESAVETAG_CENTREY;
    }
    else if (_stricmp (szElementName, "centre-z") == 0) {
      eLoadTag[nLoadLevel] = FILESAVETAG_CENTREZ;
    }
    else if (_stricmp (szElementName, "coord") == 0) {
      eLoadTag[nLoadLevel] = FILESAVETAG_COORD;
    }
    else if (_stricmp (szElementName, "texture-file") == 0) {
      eLoadTag[nLoadLevel] = FILESAVETAG_TEXFILE;
    }
    else if (_stricmp (szElementName, "texture-x-scale") == 0) {
      eLoadTag[nLoadLevel] = FILESAVETAG_TEXXSCALE;
    }
    else if (_stricmp (szElementName, "texture-y-scale") == 0) {
      eLoadTag[nLoadLevel] = FILESAVETAG_TEXYSCALE;
    }
    else if (_stricmp (szElementName, "texture-x-offset") == 0) {
      eLoadTag[nLoadLevel] = FILESAVETAG_TEXXOFFSET;
    }
    else if (_stricmp (szElementName, "texture-y-offset") == 0) {
      eLoadTag[nLoadLevel] = FILESAVETAG_TEXYOFFSET;
    }
    else {
      eLoadTag[nLoadLevel] = FILESAVETAG_OTHER;
    }
  }

  switch (eLoadTag[0]) {
  case FILESAVETAG_SETTINGS:
    StartElementSettings (eLoadTag[1], attrs);
    break;
  case FILESAVETAG_RANGE:
    if (eLoadTag[2] == FILESAVETAG_COORD) {
      StartElementRange (eLoadTag[1], attrs);
    }
    break;
  case FILESAVETAG_FUNCTION:
    StartElementFunction (eLoadTag[1], attrs);
    break;
  default:
    // Do nothing
    break;
  }

	XMLString::release (& szElementName);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cMapLoadHandler::characters (const XMLCh *const chars, const unsigned int length) {
	char*  szMessage;

	szMessage = XMLString::transcode (chars);
	sLoadText += szMessage;
	XMLString::release (& szMessage);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cMapLoadHandler::endElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname) {
  if ((nLoadLevel >= 0) && (nLoadLevel < MAX_LEVELS)) {
    switch (eLoadTag[0]) {
    case FILESAVETAG_FUNCTION:
      if (nLoadLevel == 0) {
        psMap->TransferFunctionRange (psLoadFuncData);
      }
      if (nLoadLevel == 1) {
        EndElementFunction (eLoadTag[1]);
      }
      break;
    default:
      // Do nothing
      break;
    }

    eLoadTag[nLoadLevel] = FILESAVETAG_INVALID;
  }
  nLoadLevel--;

	sLoadText = "";
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cMapLoadHandler::fatalError (const SAXParseException& exception) {
	char*  message;
	message = XMLString::transcode (exception.getMessage ());

	cout << "Fatal Error: " << message << " at line: " << exception.getLineNumber () << endl;

	XMLString::release (& message);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cMapLoadHandler::StartElementSettings (FILESAVETAG eTag, const Attributes & attrs) {
  bool boBoolFound;
  bool boValue = FALSE;
  unsigned int uAttribute;
  bool boDoubleFound;
  double fValue = 0.0f;
  //float fRadius;
  //float fRotation;
  //float fElevation;
	char * szAttributeName;
	char * szAttributeValue;

	boBoolFound = FALSE;
  boDoubleFound = FALSE;
	for (uAttribute = 0; uAttribute < attrs.getLength(); uAttribute++) {
		szAttributeName = XMLString::transcode (attrs.getLocalName (uAttribute));
		szAttributeValue = XMLString::transcode (attrs.getValue (uAttribute));

		if (_stricmp (szAttributeName, "bool") == 0) {
      boValue = (atoi (szAttributeValue) != 0);
      boBoolFound = TRUE;
    }
    if (_stricmp (szAttributeName, "double") == 0) {
      fValue = atof (szAttributeValue);
      boDoubleFound = TRUE;
    }
		XMLString::release (& szAttributeName);
		XMLString::release (& szAttributeValue);
	}

  //if (boBoolFound) {
  //  switch (eTag) {
  //  case FILESAVETAG_SHOWAXES:
  //    SetDrawAxes (boValue, psFilesaveData->psFunctyData->psVisData);
  //    break;
  //  case FILESAVETAG_SPIN:
  //    if (psFilesaveData->boConfigure) {
  //      SetSpin (boValue, psFilesaveData->psFunctyData->psVisData);
  //    }
  //    break;
  //  case FILESAVETAG_INVERT:
  //    SetClearWhite (boValue, psFilesaveData->psFunctyData->psVisData);
  //    break;
  //  case FILESAVETAG_WIREFRAME:
  //    SetWireframe (boValue, psFilesaveData->psFunctyData->psVisData);
  //    break;
  //  case FILESAVETAG_FULLSCREEN:
  //    if (psFilesaveData->boConfigure) {
  //      SetFullScreenWindow (boValue, psFilesaveData->psFunctyData);
  //    }
  //    break;
  //  default:
  //    // Do nothing
  //    break;
  //  }
  //}

  //if (boDoubleFound) {
  //  GetView (& fRadius, & fRotation, & fElevation, psFilesaveData->psFunctyData->psVisData);
  //  switch (eTag) {
  //  case FILESAVETAG_RADIUS:
  //    if (psFilesaveData->boConfigure) {
  //      SetView (fValue, fRotation, fElevation, psFilesaveData->psFunctyData->psVisData);
  //    }
  //    break;
  //  case FILESAVETAG_ROTATION:
  //    if (psFilesaveData->boConfigure) {
  //      SetView (fRadius, fValue, fElevation, psFilesaveData->psFunctyData->psVisData);
  //    }
  //    break;
  //  case FILESAVETAG_ELEVATION:
  //    if (psFilesaveData->boConfigure) {
  //      SetView (fRadius, fRotation, fValue, psFilesaveData->psFunctyData->psVisData);
  //    }
  //    break;
  //  default:
  //    // Do nothing
  //    break;
  //  }
  //}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cMapLoadHandler::StartElementRange (FILESAVETAG eTag, const Attributes & attrs) {
  double afRange[6];
  unsigned int uAttribute;
	char * szAttributeName;
	char * szAttributeValue;

  psMap->GetVisRange (afRange);

  switch (eTag) {
  case FILESAVETAG_MIN:
		for (uAttribute = 0; uAttribute < attrs.getLength(); uAttribute++) {
			szAttributeName = XMLString::transcode (attrs.getLocalName (uAttribute));
			szAttributeValue = XMLString::transcode (attrs.getValue (uAttribute));

      if (_stricmp (szAttributeName, "x") == 0) {
        afRange[0] = atof (szAttributeValue);
      }
      if (_stricmp (szAttributeName, "y") == 0) {
        afRange[1] = atof (szAttributeValue);
      }
      if (_stricmp (szAttributeName, "z") == 0) {
        afRange[2] = atof (szAttributeValue);
      }

			XMLString::release (& szAttributeName);
			XMLString::release (& szAttributeValue);
		}
    break;
  case FILESAVETAG_WIDTH:
		for (uAttribute = 0; uAttribute < attrs.getLength(); uAttribute++) {
			szAttributeName = XMLString::transcode (attrs.getLocalName (uAttribute));
			szAttributeValue = XMLString::transcode (attrs.getValue (uAttribute));

      if (_stricmp (szAttributeName, "x") == 0) {
        afRange[3] = atof (szAttributeValue);
      }
      if (_stricmp (szAttributeName, "y") == 0) {
        afRange[4] = atof (szAttributeValue);
      }
      if (_stricmp (szAttributeName, "z") == 0) {
        afRange[5] = atof (szAttributeValue);
      }

			XMLString::release (& szAttributeName);
			XMLString::release (& szAttributeValue);
		}
    break;
  default:
    // Do nothing
    break;
  }

  psMap->SetVisRange (afRange);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cMapLoadHandler::StartElementFunction (FILESAVETAG eTag, const Attributes & attrs) {
  bool boDoubleFound;
  double fValue = 0.0f;
  unsigned int uAttribute;
  FUNCTYPE eFuncType = FUNCTYPE_CARTESIAN;
	char * szAttributeName;
	char * szAttributeValue;

  if (eTag == FILESAVETAG_INVALID) {
		for (uAttribute = 0; uAttribute < attrs.getLength(); uAttribute++) {
			szAttributeName = XMLString::transcode (attrs.getLocalName (uAttribute));
			szAttributeValue = XMLString::transcode (attrs.getValue (uAttribute));

			if (_stricmp (szAttributeName, "type") == 0) {
				if (_stricmp (szAttributeValue, "cartesian") == 0) {
					eFuncType = FUNCTYPE_CARTESIAN;
				}
				else if (_stricmp (szAttributeValue, "spherical") == 0) {
					eFuncType = FUNCTYPE_SPHERICAL;
				}
			}

			XMLString::release (& szAttributeName);
			XMLString::release (& szAttributeValue);
		}

		// Create a new function
		psLoadFuncData = psMap->AddFunction (eFuncType, ACCURACY_NEW);
  }

  if (psLoadFuncData) {
    boDoubleFound = FALSE;
		for (uAttribute = 0; uAttribute < attrs.getLength(); uAttribute++) {
			szAttributeName = XMLString::transcode (attrs.getLocalName (uAttribute));
			szAttributeValue = XMLString::transcode (attrs.getValue (uAttribute));

      if (_stricmp (szAttributeName, "double") == 0) {
        fValue = atof (szAttributeValue);
        boDoubleFound = TRUE;
      }

			XMLString::release (& szAttributeName);
			XMLString::release (& szAttributeValue);
		}

    if (boDoubleFound) {
      switch (eTag) {
      case FILESAVETAG_ACCURACY:
        SetFunctionAccuracy (fValue, psLoadFuncData);
        break;
      default:
        // Do nothing
        break;
      }
    }
  }
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cMapLoadHandler::EndElementFunction (FILESAVETAG eTag) {
  char const * szRed;
  char const * szGreen;
  char const * szBlue;
  char const * szAlpha;
  char const * szXCentre = NULL;
  char const * szYCentre = NULL;
  char const * szZCentre = NULL;
  char const * szTexFile = NULL;
  char const * szTexXScale = NULL;
  char const * szTexYScale = NULL;
  char const * szTexXOffset = NULL;
  char const * szTexYOffset = NULL;
  FUNCTYPE eFuncType;

  eFuncType = GetFunctionType (psLoadFuncData);
  szRed = GetRedString (psLoadFuncData);
  szGreen = GetGreenString (psLoadFuncData);
  szBlue = GetBlueString (psLoadFuncData);
  szAlpha = GetAlphaString (psLoadFuncData);
  if (eFuncType == FUNCTYPE_SPHERICAL) {
    szXCentre = SphericalGetXCentreString (psLoadFuncData);
    szYCentre = SphericalGetYCentreString (psLoadFuncData);
    szZCentre = SphericalGetZCentreString (psLoadFuncData);
  }
	szTexFile = GetTexFileString (psLoadFuncData);
	szTexXScale = GetTexXScaleString (psLoadFuncData);
	szTexYScale = GetTexYScaleString (psLoadFuncData);
	szTexXOffset = GetTexXOffsetString (psLoadFuncData);
	szTexYOffset = GetTexYOffsetString (psLoadFuncData);

  if (psLoadFuncData) {
    switch (eTag) {
    case FILESAVETAG_EQUATION:
      SetFunction (sLoadText.c_str (), psLoadFuncData);
      break;
    case FILESAVETAG_RED:
      SetFunctionColours (sLoadText.c_str (), szGreen, szBlue, szAlpha, psLoadFuncData);
      break;
    case FILESAVETAG_GREEN:
      SetFunctionColours (szRed, sLoadText.c_str (), szBlue, szAlpha, psLoadFuncData);
      break;
    case FILESAVETAG_BLUE:
      SetFunctionColours (szRed, szGreen, sLoadText.c_str (), szAlpha, psLoadFuncData);
      break;
    case FILESAVETAG_ALPHA:
      SetFunctionColours (szRed, szGreen, szBlue, sLoadText.c_str (), psLoadFuncData);
      break;
    case FILESAVETAG_CENTREX:
      if (eFuncType == FUNCTYPE_SPHERICAL) {
        SphericalSetFunctionCentre (sLoadText.c_str (), szYCentre, szZCentre, psLoadFuncData);
      }
      break;
    case FILESAVETAG_CENTREY:
      if (eFuncType == FUNCTYPE_SPHERICAL) {
        SphericalSetFunctionCentre (szXCentre, sLoadText.c_str (), szZCentre, psLoadFuncData);
      }
      break;
    case FILESAVETAG_CENTREZ:
      if (eFuncType == FUNCTYPE_SPHERICAL) {
        SphericalSetFunctionCentre (szXCentre, szYCentre, sLoadText.c_str (), psLoadFuncData);
      }
      break;
    case FILESAVETAG_TEXFILE:
			SetTextureValues (sLoadText.c_str (), szTexXScale, szTexYScale, szTexXOffset, szTexYOffset, psLoadFuncData);
      break;
    case FILESAVETAG_TEXXSCALE:
			SetTextureValues (szTexFile, sLoadText.c_str (), szTexYScale, szTexXOffset, szTexYOffset, psLoadFuncData);
      break;
    case FILESAVETAG_TEXYSCALE:
			SetTextureValues (szTexFile, szTexXScale, sLoadText.c_str (), szTexXOffset, szTexYOffset, psLoadFuncData);
      break;
    case FILESAVETAG_TEXXOFFSET:
			SetTextureValues (szTexFile, szTexXScale, szTexYScale, sLoadText.c_str (), szTexYOffset, psLoadFuncData);
      break;
    case FILESAVETAG_TEXYOFFSET:
			SetTextureValues (szTexFile, szTexXScale, szTexYScale, szTexXOffset, sLoadText.c_str (), psLoadFuncData);
      break;
    default:
      // Do nothing
      break;
    }
  }
}

