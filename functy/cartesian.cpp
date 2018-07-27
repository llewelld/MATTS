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
#include "cartesian.h"
#include "function.h"
#include "function_private.h"

#include <symbolic.h>

///////////////////////////////////////////////////////////////////
// Defines

#define FUNCTION0 "0"

///////////////////////////////////////////////////////////////////
// Structures and enumerations

struct _CartesianPersist {
  // The actual function
  Operation * psDiffWrtX;
  Operation * psDiffWrtY;
  Variable * psVariableX;
  Variable * psVariableY;
  Variable * psVariableZ;

  // Storage for the OpenGL vertex and normal data
  int nXVertices;
  int nYVertices;
};

///////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////
// Function prototypes

void CartesianPopulateVerticesColour (FuncPersist * psFuncData);
void CartesianPopulateVerticesNoColour (FuncPersist * psFuncData);

///////////////////////////////////////////////////////////////////
// Function definitions

CartesianPersist * NewCartesianPersist () {
  CartesianPersist * psCartesianData;

  psCartesianData = g_new0 (CartesianPersist, 1);

  psCartesianData->nXVertices = 0;
  psCartesianData->nYVertices = 0;

  psCartesianData->psDiffWrtX = NULL;
  psCartesianData->psDiffWrtY = NULL;
  psCartesianData->psVariableX = NULL;
  psCartesianData->psVariableY = NULL;
  psCartesianData->psVariableZ = NULL;

  return psCartesianData;
}

void DeleteCartesianPersist (CartesianPersist * psCartesianData) {
  // Free up the function data
  FreeRecursive (psCartesianData->psDiffWrtX);
  FreeRecursive (psCartesianData->psDiffWrtY);

  g_free (psCartesianData);
}

void CartesianSetFunction (char const * const szFunction, FuncPersist * psFuncData) {
  CartesianPersist * psCartesianData = psFuncData->Func.psCartesianData;
  Operation * psWRT;

  // Free up any previous function
  FreeRecursive (psFuncData->psFunction);
  FreeRecursive (psCartesianData->psDiffWrtX);
  FreeRecursive (psCartesianData->psDiffWrtY);
  psFuncData->psVariables = FreeVariables (psFuncData->psVariables);

  psFuncData->psFunction = StringToOperation (szFunction);
  psFuncData->psFunction = UberSimplifyOperation (psFuncData->psFunction);

  // Differentiate with respect to x
  psWRT = CreateVariable ("x");
  psCartesianData->psDiffWrtX = DifferentiateOperation (psFuncData->psFunction, psWRT);
  FreeRecursive (psWRT);
  psCartesianData->psDiffWrtX = UberSimplifyOperation (psCartesianData->psDiffWrtX);

  // Differentiate with respect to y
  psWRT = CreateVariable ("y");
  psCartesianData->psDiffWrtY = DifferentiateOperation (psFuncData->psFunction, psWRT);
  FreeRecursive (psWRT);
  psCartesianData->psDiffWrtY = UberSimplifyOperation (psCartesianData->psDiffWrtY);

  // Set up the variables
  psFuncData->psVariables = CreateVariables (psFuncData->psFunction, psFuncData->psVariables);
  psFuncData->psVariables = CreateVariables (psCartesianData->psDiffWrtX, psFuncData->psVariables);
  psFuncData->psVariables = CreateVariables (psCartesianData->psDiffWrtY, psFuncData->psVariables);

  // Free up any unused variables
  psFuncData->psVariables  = FreeVariables (psFuncData->psVariables);

  // Find the x, y, z and t variables if they exist
  psCartesianData->psVariableX = FindVariable (psFuncData->psVariables, "x");
  psCartesianData->psVariableY = FindVariable (psFuncData->psVariables, "y");
  psCartesianData->psVariableZ = FindVariable (psFuncData->psVariables, "z");
  psFuncData->psVariableT = FindVariable (psFuncData->psVariables, "t");

  // Set up the handy UI-related data
  g_string_assign (psFuncData->szFunction, szFunction);

  // Figure out if the functions are time dependent
  psFuncData->boTimeDependent = (psFuncData->psVariableT != NULL);
}

void CartesianSetFunctionColours (char const * const szRed, char const * const szGreen, char const * const szBlue, char const * const szAlpha, FuncPersist * psFuncData) {
  CartesianPersist * psCartesianData = psFuncData->Func.psCartesianData;
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

  // Find the x, y, z and t variables if they exist
  psCartesianData->psVariableX = FindVariable (psFuncData->psVariables, "x");
  psCartesianData->psVariableY = FindVariable (psFuncData->psVariables, "y");
  psCartesianData->psVariableZ = FindVariable (psFuncData->psVariables, "z");
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

void CartesianGetVertexDimensions (int * pnXVertices, int * pnYVertices, FuncPersist * psFuncData) {
  CartesianPersist * psCartesianData = psFuncData->Func.psCartesianData;

  if (pnXVertices) {
    * pnXVertices = psCartesianData->nXVertices;
  }
  if (pnYVertices) {
    * pnYVertices = psCartesianData->nXVertices;
  }
}

void CartesianPopulateVertices (FuncPersist * psFuncData) {
  if (psFuncData->boColourFunction) {
    CartesianPopulateVerticesColour (psFuncData);
  }
  else {
    CartesianPopulateVerticesNoColour (psFuncData);
  }
}

void CartesianPopulateVerticesNoColour (FuncPersist * psFuncData) {
  CartesianPersist * psCartesianData = psFuncData->Func.psCartesianData;
  int nVertex;
  double fX;
  double fY;
  double fXFunc;
  double fYFunc;
  double fZFunc;
  double fZScreen;
  double x;
  double y;
  Vector3 vX;
  Vector3 vY;
  Vector3 vN;
  double fEnd;
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
  for (fX = 0.0; fX < fEnd; fX += psFuncData->fAccuracy) {

    fXFunc = (fX * psFuncData->fXWidth) + psFuncData->fXMin;

    for (fY = 0.0; fY < fEnd; fY += psFuncData->fAccuracy) {
      fYFunc = (fY * psFuncData->fYWidth) + psFuncData->fYMin;

      x = fXFunc;
      y = fYFunc;

      if (psCartesianData->psVariableX) {
        SetVariable (psCartesianData->psVariableX, fXFunc);
      }
      if (psCartesianData->psVariableY) {
        SetVariable (psCartesianData->psVariableY, fYFunc);
      }

      fZFunc = ApproximateOperation (psFuncData->psFunction);

      vX.fX = 1.0f;
      vX.fY = 0.0f;
      vX.fZ = ApproximateOperation (psCartesianData->psDiffWrtX);

      vY.fX = 0.0f;
      vY.fY = 1.0f;
      vY.fZ = ApproximateOperation (psCartesianData->psDiffWrtY);

      vN = Normal (& vX, & vY);

      psFuncData->afNormals[(nVertex * 3)] = vN.fX;
      psFuncData->afNormals[(nVertex * 3) + 1] = vN.fY;
      psFuncData->afNormals[(nVertex * 3) + 2] = vN.fZ;

      fZScreen = (((fZFunc - psFuncData->fZMin) / psFuncData->fZWidth) * AXIS_ZSIZE) - AXIS_ZHSIZE;

			// Calculate vertex texture coordinate
      psFuncData->afTextureCoords[(nVertex * 2)] = (fXFunc * fTexXScale) + fTexXOffset;
      psFuncData->afTextureCoords[(nVertex * 2) + 1] = (fYFunc * fTexYScale) + fTexYOffset;

      psFuncData->afVertices[(nVertex * 3) + 2] = fZScreen;

      nVertex++;
    }
  }
}

void CartesianPopulateVerticesColour (FuncPersist * psFuncData) {
  CartesianPersist * psCartesianData = psFuncData->Func.psCartesianData;
  int nVertex;
  double fX;
  double fY;
  double fXFunc;
  double fYFunc;
  double fZFunc;
  double fZScreen;
  double x;
  double y;
  Vector3 vX;
  Vector3 vY;
  Vector3 vN;
  double fEnd;
  float fColour;
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
  for (fX = 0.0; fX < fEnd; fX += psFuncData->fAccuracy) {

    fXFunc = (fX * psFuncData->fXWidth) + psFuncData->fXMin;

    for (fY = 0.0; fY < fEnd; fY += psFuncData->fAccuracy) {
      fYFunc = (fY * psFuncData->fYWidth) + psFuncData->fYMin;

      x = fXFunc;
      y = fYFunc;

      if (psCartesianData->psVariableX) {
        SetVariable (psCartesianData->psVariableX, fXFunc);
      }
      if (psCartesianData->psVariableY) {
        SetVariable (psCartesianData->psVariableY, fYFunc);
      }

      fZFunc = ApproximateOperation (psFuncData->psFunction);

      vX.fX = 1.0f;
      vX.fY = 0.0f;
      vX.fZ = ApproximateOperation (psCartesianData->psDiffWrtX);

      vY.fX = 0.0f;
      vY.fY = 1.0f;
      vY.fZ = ApproximateOperation (psCartesianData->psDiffWrtY);

      vN = Normal (& vX, & vY);

      psFuncData->afNormals[(nVertex * 3)] = vN.fX;
      psFuncData->afNormals[(nVertex * 3) + 1] = vN.fY;
      psFuncData->afNormals[(nVertex * 3) + 2] = vN.fZ;

      fZScreen = (((fZFunc - psFuncData->fZMin) / psFuncData->fZWidth) * AXIS_ZSIZE) - AXIS_ZHSIZE;

      if (psCartesianData->psVariableZ) {
        SetVariable (psCartesianData->psVariableZ, fZFunc);
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
      psFuncData->afTextureCoords[(nVertex * 2)] = (fXFunc * fTexXScale) + fTexXOffset;
      psFuncData->afTextureCoords[(nVertex * 2) + 1] = (fYFunc * fTexYScale) + fTexYOffset;

      psFuncData->afVertices[(nVertex * 3) + 2] = fZScreen;

      nVertex++;
    }
  }
}

void CartesianGenerateVertices (FuncPersist * psFuncData) {
  CartesianPersist * psCartesianData = psFuncData->Func.psCartesianData;
  int nXVertices;
  int nYVertices;
  int nVertex;
  double fX;
  double fY;
  double fXScreen;
  double fYScreen;
  int nIndex;
  double fEnd;

  nXVertices = floor (1.0 / psFuncData->fAccuracy) + 1;
  nYVertices = floor (1.0 / psFuncData->fAccuracy) + 1;

  psCartesianData->nXVertices = nXVertices;
  psCartesianData->nYVertices = nYVertices;

  psFuncData->afVertices = g_new0 (GLfloat, nXVertices * nYVertices * 3);
  psFuncData->afNormals = g_new0 (GLfloat, nXVertices * nYVertices * 3);
  psFuncData->auIndices = g_new (GLushort, nYVertices * 2);
  psFuncData->afColours = g_new0 (GLfloat, nXVertices * nYVertices * 4);
  psFuncData->afTextureCoords = g_new0 (GLfloat, nXVertices * nYVertices * 2);

  // Generate the index data
  for (nIndex = 0; nIndex < nYVertices; nIndex++) {
    psFuncData->auIndices[(nIndex * 2)] = nIndex;
    psFuncData->auIndices[(nIndex * 2) + 1] = nIndex + nYVertices;
  }

  // Generate the x,y vertex data (z values are populated dynamically)
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
}

void CartesianSetFunctionPosition (double fXMin, double fYMin, double fZMin, FuncPersist * psFuncData) {
  psFuncData->fXMin = fXMin;
  psFuncData->fYMin = fYMin;
  psFuncData->fZMin = fZMin;

  PopulateVertices (psFuncData);
}

double CartesianGetFunctionZ (double fX, double fY, FuncPersist * psFuncData) {
  CartesianPersist * psCartesianData = psFuncData->Func.psCartesianData;
	double fXFunc;
	double fYFunc;
	double fZFunc;
	double fZScreen;

	fXFunc = (((fX + AXIS_XHSIZE) / AXIS_XSIZE) * psFuncData->fXWidth) + psFuncData->fXMin;
	fYFunc = (((fY + AXIS_YHSIZE) / AXIS_YSIZE) * psFuncData->fYWidth) + psFuncData->fYMin;

	if (psCartesianData->psVariableX) {
		SetVariable (psCartesianData->psVariableX, fXFunc);
	}
	if (psCartesianData->psVariableY) {
		SetVariable (psCartesianData->psVariableY, fYFunc);
	}

  fZFunc = ApproximateOperation (psFuncData->psFunction);

  fZScreen = (((fZFunc - psFuncData->fZMin) / psFuncData->fZWidth) * AXIS_ZSIZE) - AXIS_ZHSIZE;

	return fZScreen;
}
