///////////////////////////////////////////////////////////////////
// Functy
// 3D graph drawing utility
//
// David Llewellyn-Jones
// http://www.flypig.co.uk
//
// Spring 2009
///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
// Includes

//#include "vis.h"
#include "spherical.h"
#include "function.h"
#include "function_private.h"

#include <symbolic.h>

///////////////////////////////////////////////////////////////////
// Defines

#define FUNCTION0 "1"

///////////////////////////////////////////////////////////////////
// Structures and enumerations

struct _SphericalPersist {
  // The actual function
  Operation * psDiffWrtA;
  Operation * psDiffWrtP;
  Variable * psVariableA;
  Variable * psVariableP;
  Variable * psVariableR;

  // Centre information
  float fXCentre;
  float fYCentre;
  float fZCentre;
  Operation * psXCentre;
  Operation * psYCentre;
  Operation * psZCentre;
  Variable * psCentreVariables;
  Variable * psCentreVariableT;
  bool boCentreTimeDependent;

  // Handy data to help with the UI
  GString * szXCentre;
  GString * szYCentre;
  GString * szZCentre;

  // Storage for the OpenGL vertex and normal data
  int nAVertices;
  int nPVertices;
};

///////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////
// Function prototypes

void SphericalPopulateVerticesColour (FuncPersist * psFuncData);
void SphericalPopulateVerticesNoColour (FuncPersist * psFuncData);

///////////////////////////////////////////////////////////////////
// Function definitions

SphericalPersist * NewSphericalPersist () {
  SphericalPersist * psSphericalData;

  psSphericalData = g_new0 (SphericalPersist, 1);

  psSphericalData->nAVertices = 0;
  psSphericalData->nPVertices = 0;

  psSphericalData->psDiffWrtA = NULL;
  psSphericalData->psDiffWrtP = NULL;
  psSphericalData->psVariableA = NULL;
  psSphericalData->psVariableP = NULL;
  psSphericalData->psVariableR = NULL;

  psSphericalData->fXCentre = 0.0;
  psSphericalData->fYCentre = 0.0;
  psSphericalData->fZCentre = 0.0;
  psSphericalData->psXCentre = NULL;
  psSphericalData->psYCentre = NULL;
  psSphericalData->psZCentre = NULL;
  psSphericalData->psCentreVariables = NULL;
  psSphericalData->psCentreVariableT = NULL;
  psSphericalData->boCentreTimeDependent = FALSE;

  psSphericalData->szXCentre = g_string_new ("0.0");
  psSphericalData->szYCentre = g_string_new ("0.0");
  psSphericalData->szZCentre = g_string_new ("0.0");

  return psSphericalData;
}

void DeleteSphericalPersist (SphericalPersist * psSphericalData) {
  // Free up the function data
  FreeRecursive (psSphericalData->psDiffWrtA);
  FreeRecursive (psSphericalData->psDiffWrtP);

  if (psSphericalData->szXCentre) {
    g_string_free (psSphericalData->szXCentre, TRUE);
  }
  if (psSphericalData->szYCentre) {
    g_string_free (psSphericalData->szYCentre, TRUE);
  }
  if (psSphericalData->szZCentre) {
    g_string_free (psSphericalData->szZCentre, TRUE);
  }

  // Free up the centre data
  FreeRecursive (psSphericalData->psXCentre);
  FreeRecursive (psSphericalData->psYCentre);
  FreeRecursive (psSphericalData->psZCentre);

  psSphericalData->psCentreVariables = FreeVariables (psSphericalData->psCentreVariables);

  g_free (psSphericalData);
}

void SphericalSetFunction (char const * const szFunction, FuncPersist * psFuncData) {
  SphericalPersist * psSphericalData = psFuncData->Func.psSphericalData;
  Operation * psWRT;

  // Free up any previous function
  FreeRecursive (psFuncData->psFunction);
  FreeRecursive (psSphericalData->psDiffWrtA);
  FreeRecursive (psSphericalData->psDiffWrtP);
  psFuncData->psVariables = FreeVariables (psFuncData->psVariables);

  psFuncData->psFunction = StringToOperation (szFunction);
  psFuncData->psFunction = UberSimplifyOperation (psFuncData->psFunction);

  // Differentiate with respect to a
  psWRT = CreateVariable ("a");
  psSphericalData->psDiffWrtA = DifferentiateOperation (psFuncData->psFunction, psWRT);
  FreeRecursive (psWRT);
  psSphericalData->psDiffWrtA = UberSimplifyOperation (psSphericalData->psDiffWrtA);

  // Differentiate with respect to p
  psWRT = CreateVariable ("p");
  psSphericalData->psDiffWrtP = DifferentiateOperation (psFuncData->psFunction, psWRT);
  FreeRecursive (psWRT);
  psSphericalData->psDiffWrtP = UberSimplifyOperation (psSphericalData->psDiffWrtP);

  // Set up the variables
  psFuncData->psVariables = CreateVariables (psFuncData->psFunction, psFuncData->psVariables);
  psFuncData->psVariables = CreateVariables (psSphericalData->psDiffWrtA, psFuncData->psVariables);
  psFuncData->psVariables = CreateVariables (psSphericalData->psDiffWrtP, psFuncData->psVariables);

  // Free up any unused variables
  psFuncData->psVariables  = FreeVariables (psFuncData->psVariables);

  // Find the a, p, r and t variables if they exist
  psSphericalData->psVariableA = FindVariable (psFuncData->psVariables, "a");
  psSphericalData->psVariableP = FindVariable (psFuncData->psVariables, "p");
  psSphericalData->psVariableR = FindVariable (psFuncData->psVariables, "r");
  psFuncData->psVariableT = FindVariable (psFuncData->psVariables, "t");

  // Set up the handy UI-related data
  g_string_assign (psFuncData->szFunction, szFunction);

  // Figure out if the functions are time dependent
  psFuncData->boTimeDependent = (psFuncData->psVariableT != NULL);
}

void SphericalSetFunctionColours (char const * const szRed, char const * const szGreen, char const * const szBlue, char const * const szAlpha, FuncPersist * psFuncData) {
  SphericalPersist * psSphericalData = psFuncData->Func.psSphericalData;
  double fApproximate;

  // Free up any previous function
  FreeRecursive (psFuncData->psRed);
  FreeRecursive (psFuncData->psGreen);
  FreeRecursive (psFuncData->psBlue);
  FreeRecursive (psFuncData->psAlpha);
  psFuncData->psVariables = FreeVariables (psFuncData->psVariables);

  psFuncData->psRed = StringToOperation (szRed);
  psFuncData->psRed = UberSimplifyOperation (psFuncData->psRed);
  psFuncData->psGreen = StringToOperation (szGreen);
  psFuncData->psGreen = UberSimplifyOperation (psFuncData->psGreen);
  psFuncData->psBlue = StringToOperation (szBlue);
  psFuncData->psBlue = UberSimplifyOperation (psFuncData->psBlue);
  psFuncData->psAlpha = StringToOperation (szAlpha);
  psFuncData->psAlpha = UberSimplifyOperation (psFuncData->psAlpha);

  // Check if we can avoid recalculating the colour for every vertex
  psFuncData->boColourFunction = FALSE;
  fApproximate = ApproximateOperation (psFuncData->psRed);
  psFuncData->fRed = fApproximate;
  if (isnan (fApproximate)) {
    psFuncData->boColourFunction = TRUE;
  }
  fApproximate = ApproximateOperation (psFuncData->psGreen);
  psFuncData->fGreen = fApproximate;
  if (isnan (fApproximate)) {
    psFuncData->boColourFunction = TRUE;
  }
  fApproximate = ApproximateOperation (psFuncData->psBlue);
  psFuncData->fBlue = fApproximate;
  if (isnan (fApproximate)) {
    psFuncData->boColourFunction = TRUE;
  }
  fApproximate = ApproximateOperation (psFuncData->psAlpha);
  psFuncData->fAlpha = fApproximate;
  if (isnan (fApproximate)) {
    psFuncData->boColourFunction = TRUE;
  }

  // Set up the variables
  psFuncData->psVariables = CreateVariables (psFuncData->psRed, psFuncData->psVariables);
  psFuncData->psVariables = CreateVariables (psFuncData->psGreen, psFuncData->psVariables);
  psFuncData->psVariables = CreateVariables (psFuncData->psBlue, psFuncData->psVariables);
  psFuncData->psVariables = CreateVariables (psFuncData->psAlpha, psFuncData->psVariables);

  // Find the a, p, r and t variables if they exist
  psSphericalData->psVariableA = FindVariable (psFuncData->psVariables, "a");
  psSphericalData->psVariableP = FindVariable (psFuncData->psVariables, "p");
  psSphericalData->psVariableR = FindVariable (psFuncData->psVariables, "r");
  psFuncData->psVariableT = FindVariable (psFuncData->psVariables, "t");

  // Free up any unused variables
  psFuncData->psVariables  = FreeVariables (psFuncData->psVariables);

  // Set up the handy UI-related data
  g_string_assign (psFuncData->szRed, szRed);
  g_string_assign (psFuncData->szGreen, szGreen);
  g_string_assign (psFuncData->szBlue, szBlue);
  g_string_assign (psFuncData->szAlpha, szAlpha);

  // Figure out if the functions are time dependent
  psFuncData->boTimeDependent = (psFuncData->psVariableT != NULL);
}

void SphericalGetVertexDimensions (int * pnAVertices, int * pnPVertices, FuncPersist * psFuncData) {
  SphericalPersist * psSphericalData = psFuncData->Func.psSphericalData;

  if (pnAVertices) {
    * pnAVertices = psSphericalData->nAVertices;
  }
  if (pnPVertices) {
    * pnPVertices = psSphericalData->nPVertices;
  }
}

void SphericalPopulateVertices (FuncPersist * psFuncData) {
  if (psFuncData->boColourFunction) {
    SphericalPopulateVerticesColour (psFuncData);
  }
  else {
    SphericalPopulateVerticesNoColour (psFuncData);
  }
}

void SphericalPopulateVerticesNoColour (FuncPersist * psFuncData) {
  SphericalPersist * psSphericalData = psFuncData->Func.psSphericalData;
  int nVertex;
  double fA;
  double fP;
  double fAFunc;
  double fPFunc;
  double fRFunc;
  double fXScreen;
  double fYScreen;
  double fZScreen;
  Vector3 vA;
  Vector3 vP;
  Vector3 vN;
  double fEnd;
  double fDiff;
	double fTexXOffset = 0.0;
	double fTexYOffset = 0.0;
	double fTexXScale = 1.0;
	double fTexYScale = 1.0;

	sscanf (psFuncData->szTexXOffset->str, "%lf", & fTexXOffset);
	sscanf (psFuncData->szTexYOffset->str, "%lf", & fTexYOffset);
	sscanf (psFuncData->szTexXScale->str, "%lf", & fTexXScale);
	sscanf (psFuncData->szTexYScale->str, "%lf", & fTexYScale);

  fEnd = 1.0 + (psFuncData->fAccuracy / 2.0);
  nVertex = 0;
  for (fA = 0.0; fA < fEnd; fA += psFuncData->fAccuracy) {

    fAFunc = (fA * 2.0 * M_PI);

    for (fP = 0.0; fP < fEnd; fP += psFuncData->fAccuracy) {
      fPFunc = (fP * 1.0 * M_PI);

      if (psSphericalData->psVariableA) {
        SetVariable (psSphericalData->psVariableA, fAFunc);
      }
      if (psSphericalData->psVariableP) {
        SetVariable (psSphericalData->psVariableP, fPFunc);
      }

      fRFunc = ApproximateOperation (psFuncData->psFunction);

      // Calculate the normal
      fDiff = ApproximateOperation (psSphericalData->psDiffWrtA);

      vA.fX = sin (fPFunc) * ((fDiff * cos (fAFunc)) - (fRFunc * sin (fAFunc)));
      vA.fY = sin (fPFunc) * ((fDiff * sin (fAFunc)) + (fRFunc * cos (fAFunc)));
      vA.fZ = fDiff * cos (fPFunc);

      fDiff = ApproximateOperation (psSphericalData->psDiffWrtP);

      vP.fX = cos (fAFunc) * ((fDiff * sin (fPFunc)) + (fRFunc * cos (fPFunc)));
      vP.fY = sin (fAFunc) * ((fDiff * sin (fPFunc)) + (fRFunc * cos (fPFunc)));
      vP.fZ = (fDiff * cos (fPFunc)) - (fRFunc * sin (fPFunc));

      vN = Normal (& vA, & vP);

      // Since sin(p) is zero when p is zero, we need to fix the normal
      if (fPFunc == 0.0f) {
        vN.fX = 0.0;
        vN.fY = 0.0;
        vN.fZ = -1.0;
      }

      psFuncData->afNormals[(nVertex * 3)] = vN.fX;
      psFuncData->afNormals[(nVertex * 3) + 1] = vN.fY;
      psFuncData->afNormals[(nVertex * 3) + 2] = vN.fZ;

      // Calculate the cartesian position
      fXScreen = (((fRFunc * cos (fAFunc) * sin (fPFunc)) / psFuncData->fXWidth) * AXIS_XSIZE) - AXIS_XHSIZE;
      fYScreen = (((fRFunc * sin (fAFunc) * sin (fPFunc)) / psFuncData->fYWidth) * AXIS_YSIZE) - AXIS_YHSIZE;
      fZScreen = (((fRFunc * cos (fPFunc)) / psFuncData->fZWidth) * AXIS_ZSIZE) - AXIS_ZHSIZE;

			// Calculate vertex texture coordinate
      psFuncData->afTextureCoords[(nVertex * 2)] = (fAFunc * fTexXScale) + fTexXOffset;
      psFuncData->afTextureCoords[(nVertex * 2) + 1] = (fPFunc * fTexYScale) + fTexYOffset;

      psFuncData->afVertices[(nVertex * 3)] = fXScreen;
      psFuncData->afVertices[(nVertex * 3) + 1] = fYScreen;
      psFuncData->afVertices[(nVertex * 3) + 2] = fZScreen;

      nVertex++;
    }
  }
}

void SphericalPopulateVerticesColour (FuncPersist * psFuncData) {
  SphericalPersist * psSphericalData = psFuncData->Func.psSphericalData;
  int nVertex;
  double fA;
  double fP;
  double fAFunc;
  double fPFunc;
  double fRFunc;
  double fXScreen;
  double fYScreen;
  double fZScreen;
  Vector3 vA;
  Vector3 vP;
  Vector3 vN;
  double fEnd;
  float fColour;
  double fDiff;
	double fTexXOffset = 0.0;
	double fTexYOffset = 0.0;
	double fTexXScale = 1.0;
	double fTexYScale = 1.0;

	sscanf (psFuncData->szTexXOffset->str, "%lf", & fTexXOffset);
	sscanf (psFuncData->szTexYOffset->str, "%lf", & fTexYOffset);
	sscanf (psFuncData->szTexXScale->str, "%lf", & fTexXScale);
	sscanf (psFuncData->szTexYScale->str, "%lf", & fTexYScale);

  fEnd = 1.0 + (psFuncData->fAccuracy / 2.0);
  nVertex = 0;
  for (fA = 0.0; fA < fEnd; fA += psFuncData->fAccuracy) {

    fAFunc = (fA * 2.0 * M_PI);

    for (fP = 0.0; fP < fEnd; fP += psFuncData->fAccuracy) {
      fPFunc = (fP * 1.0 * M_PI);

      if (psSphericalData->psVariableA) {
        SetVariable (psSphericalData->psVariableA, fAFunc);
      }
      if (psSphericalData->psVariableP) {
        SetVariable (psSphericalData->psVariableP, fPFunc);
      }

      fRFunc = ApproximateOperation (psFuncData->psFunction);

      // Calculate the normal
      fDiff = ApproximateOperation (psSphericalData->psDiffWrtA);

      vA.fX = sin (fPFunc) * ((fDiff * cos (fAFunc)) - (fRFunc * sin (fAFunc)));
      vA.fY = sin (fPFunc) * ((fDiff * sin (fAFunc)) + (fRFunc * cos (fAFunc)));
      vA.fZ = fDiff * cos (fPFunc);

      fDiff = ApproximateOperation (psSphericalData->psDiffWrtP);

      vP.fX = cos (fAFunc) * ((fDiff * sin (fPFunc)) + (fRFunc * cos (fPFunc)));
      vP.fY = sin (fAFunc) * ((fDiff * sin (fPFunc)) + (fRFunc * cos (fPFunc)));
      vP.fZ = (fDiff * cos (fPFunc)) - (fRFunc * sin (fPFunc));

      vN = Normal (& vA, & vP);

      // Since sin(p) is zero when p is zero, we need to fix the normal
      if (fPFunc == 0.0f) {
        vN.fX = 0.0;
        vN.fY = 0.0;
        vN.fZ = -1.0;
      }

      psFuncData->afNormals[(nVertex * 3)] = vN.fX;
      psFuncData->afNormals[(nVertex * 3) + 1] = vN.fY;
      psFuncData->afNormals[(nVertex * 3) + 2] = vN.fZ;

      // Calculate the cartesian position
      fXScreen = (((fRFunc * cos (fAFunc) * sin (fPFunc)) / psFuncData->fXWidth) * AXIS_ZSIZE) - AXIS_ZHSIZE;
      fYScreen = (((fRFunc * sin (fAFunc) * sin (fPFunc)) / psFuncData->fYWidth) * AXIS_ZSIZE) - AXIS_ZHSIZE;
      fZScreen = (((fRFunc * cos (fPFunc)) / psFuncData->fZWidth) * AXIS_ZSIZE) - AXIS_ZHSIZE;

      if (psSphericalData->psVariableR) {
        SetVariable (psSphericalData->psVariableR, fRFunc);
      }

      // Calculate vertex colour
      fColour = ApproximateOperation (psFuncData->psRed);
      psFuncData->afColours[(nVertex * 4)] = fColour;
      fColour = ApproximateOperation (psFuncData->psGreen);
      psFuncData->afColours[(nVertex * 4) + 1] = fColour;
      fColour = ApproximateOperation (psFuncData->psBlue);
      psFuncData->afColours[(nVertex * 4) + 2] = fColour;
      fColour = ApproximateOperation (psFuncData->psAlpha);
      psFuncData->afColours[(nVertex * 4) + 3] = fColour;

			// Calculate vertex texture coordinate
      psFuncData->afTextureCoords[(nVertex * 2)] = (fAFunc * fTexXScale) + fTexXOffset;
      psFuncData->afTextureCoords[(nVertex * 2) + 1] = (fPFunc * fTexYScale) + fTexYOffset;

      psFuncData->afVertices[(nVertex * 3)] = fXScreen;
      psFuncData->afVertices[(nVertex * 3) + 1] = fYScreen;
      psFuncData->afVertices[(nVertex * 3) + 2] = fZScreen;

      nVertex++;
    }
  }
}

void SphericalGenerateVertices (FuncPersist * psFuncData) {
  SphericalPersist * psSphericalData = psFuncData->Func.psSphericalData;
  int nAVertices;
  int nPVertices;
  int nIndex;
/*
  int nVertex;
  double fX;
  double fY;
  double fXScreen;
  double fYScreen;
  double fEnd;
*/

  nAVertices = floor (1.0 / psFuncData->fAccuracy) + 1;
  nPVertices = floor (1.0 / psFuncData->fAccuracy) + 1;

  psSphericalData->nAVertices = nAVertices;
  psSphericalData->nPVertices = nPVertices;

  psFuncData->afVertices = g_new0 (GLfloat, nAVertices * nPVertices * 3);
  psFuncData->afNormals = g_new0 (GLfloat, nAVertices * nPVertices * 3);
  psFuncData->auIndices = g_new (GLushort, nPVertices * 2);
  psFuncData->afColours = g_new0 (GLfloat, nAVertices * nPVertices * 4);
  psFuncData->afTextureCoords = g_new0 (GLfloat, nAVertices * nPVertices * 2);

  // Generate the index data
  for (nIndex = 0; nIndex < nPVertices; nIndex++) {
    psFuncData->auIndices[(nIndex * 2)] = nIndex;
    psFuncData->auIndices[(nIndex * 2) + 1] = nIndex + nPVertices;
  }

  // These don't need to be generated for spherical coordinates
  // They're all populated dynamically
/*
  fEnd = 1.0 + (psFuncData->fAccuracy / 2.0);
  nVertex = 0;
  for (fX = 0.0; fX < fEnd; fX += psFuncData->fAccuracy) {
    fXScreen = (fX * AXIS_XSIZE) - AXIS_XHSIZE;

    for (fY = 0.0; fY < fEnd; fY += psFuncData->fAccuracy) {
      fYScreen = (fY * AXIS_YSIZE) - AXIS_YHSIZE;

      psFuncData->afNormals[(nVertex * 3)] = 0.0;
      psFuncData->afNormals[(nVertex * 3) + 1] = 0.0;
      psFuncData->afNormals[(nVertex * 3) + 2] = 1.0;

      psFuncData->afVertices[(nVertex * 3)] = fXScreen;
      psFuncData->afVertices[(nVertex * 3) + 1] = fYScreen;
      psFuncData->afVertices[(nVertex * 3) + 2] = 0.0;

      nVertex++;
    }
  }
*/
}

char const * SphericalGetXCentreString (FuncPersist * psFuncData) {
  SphericalPersist * psSphericalData = psFuncData->Func.psSphericalData;

  return psSphericalData->szXCentre->str;
}

char const * SphericalGetYCentreString (FuncPersist * psFuncData) {
  SphericalPersist * psSphericalData = psFuncData->Func.psSphericalData;

  return psSphericalData->szYCentre->str;
}

char const * SphericalGetZCentreString (FuncPersist * psFuncData) {
  SphericalPersist * psSphericalData = psFuncData->Func.psSphericalData;

  return psSphericalData->szZCentre->str;
}

void SphericalSetFunctionCentre (char const * const szXCentre, char const * const szYCentre, char const * const szZCentre, FuncPersist * psFuncData) {
  SphericalPersist * psSphericalData = psFuncData->Func.psSphericalData;

  // Free up any previous function
  FreeRecursive (psSphericalData->psXCentre);
  FreeRecursive (psSphericalData->psYCentre);
  FreeRecursive (psSphericalData->psZCentre);
  psSphericalData->psCentreVariables = FreeVariables (psSphericalData->psCentreVariables);

  psSphericalData->psXCentre = StringToOperation (szXCentre);
  psSphericalData->psXCentre = UberSimplifyOperation (psSphericalData->psXCentre);
  psSphericalData->psYCentre = StringToOperation (szYCentre);
  psSphericalData->psYCentre = UberSimplifyOperation (psSphericalData->psYCentre);
  psSphericalData->psZCentre = StringToOperation (szZCentre);
  psSphericalData->psZCentre = UberSimplifyOperation (psSphericalData->psZCentre);

  // Check if we can avoid recalculating the centre for every frame
  psSphericalData->fXCentre = ApproximateOperation (psSphericalData->psXCentre);
  psSphericalData->fYCentre = ApproximateOperation (psSphericalData->psYCentre);
  psSphericalData->fZCentre = ApproximateOperation (psSphericalData->psZCentre);

  // Set up the variables
  // Note that we keep these variables separate from the main function
  // in order to keep the animations separate
  psSphericalData->psCentreVariables = CreateVariables (psSphericalData->psXCentre, psSphericalData->psCentreVariables);
  psSphericalData->psCentreVariables = CreateVariables (psSphericalData->psYCentre, psSphericalData->psCentreVariables);
  psSphericalData->psCentreVariables = CreateVariables (psSphericalData->psZCentre, psSphericalData->psCentreVariables);

  // Find the t variable if it exists
  psSphericalData->psCentreVariableT = FindVariable (psSphericalData->psCentreVariables, "t");

  // Free up any unused variables
  psSphericalData->psCentreVariables = FreeVariables (psSphericalData->psCentreVariables);

  // Set up the handy UI-related data
  g_string_assign (psSphericalData->szXCentre, szXCentre);
  g_string_assign (psSphericalData->szYCentre, szYCentre);
  g_string_assign (psSphericalData->szZCentre, szZCentre);

  // Figure out if the functions are time dependent
  psSphericalData->boCentreTimeDependent = (psSphericalData->psCentreVariableT != NULL);
}

bool SphericalGetCentreTimeDependent (FuncPersist * psFuncData) {
  SphericalPersist * psSphericalData = psFuncData->Func.psSphericalData;

  return psSphericalData->boCentreTimeDependent;
}

void SphericalGetCentre (double * afCentre, FuncPersist * psFuncData) {
  SphericalPersist * psSphericalData = psFuncData->Func.psSphericalData;

  afCentre[0] = psSphericalData->fXCentre;
  afCentre[1] = psSphericalData->fYCentre;
  afCentre[2] = psSphericalData->fZCentre;
}

void SphericalUpdateCentre (FuncPersist * psFuncData) {
  SphericalPersist * psSphericalData = psFuncData->Func.psSphericalData;

  if (psSphericalData->boCentreTimeDependent) {
    psSphericalData->fXCentre = ApproximateOperation (psSphericalData->psXCentre);
    psSphericalData->fYCentre = ApproximateOperation (psSphericalData->psYCentre);
    psSphericalData->fZCentre = ApproximateOperation (psSphericalData->psZCentre);
  }
}

void SphericalSetFunctionTime (double fTime, FuncPersist * psFuncData) {
  SphericalPersist * psSphericalData = psFuncData->Func.psSphericalData;

  if (psSphericalData->psCentreVariableT) {
    SetVariable (psSphericalData->psCentreVariableT, fTime);
  }
}

void SphericalSetFunctionPosition (double fXMin, double fYMin, double fZMin, FuncPersist * psFuncData) {
  psFuncData->fXMin = fXMin;
  psFuncData->fYMin = fYMin;
  psFuncData->fZMin = fZMin;

  // It's not necessary to regenerate the vertices for the spherical function
  //PopulateVertices (psFuncData);
}

