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
#include "function.h"
#include "function_private.h"
#include "textures.h"

///////////////////////////////////////////////////////////////////
// Defines

#define FUNCTION0 "1"

///////////////////////////////////////////////////////////////////
// Structures and enumerations

///////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////
// Function prototypes

void GenerateVertices (FuncPersist * psFuncData);

///////////////////////////////////////////////////////////////////
// Function definitions

FuncPersist * NewFuncPersist (FUNCTYPE eType, double fAccuracy) {
  FuncPersist * psFuncData;

  psFuncData = g_new0 (FuncPersist, 1);

  psFuncData->eType = eType;
  switch (eType) {
  case FUNCTYPE_CARTESIAN:
    psFuncData->Func.psCartesianData = NewCartesianPersist ();
    break;
  case FUNCTYPE_SPHERICAL:
    psFuncData->Func.psSphericalData = NewSphericalPersist ();
    break;
  default:
    psFuncData->Func.psNone = NULL;
    break;
  }

  psFuncData->fXMin = -5.0;
  psFuncData->fYMin = -5.0;
  psFuncData->fZMin = -20.0;
  psFuncData->fXWidth = 10.0;
  psFuncData->fYWidth = 10.0;
  psFuncData->fZWidth = 40.0;

  psFuncData->fAccuracy = fAccuracy;
  psFuncData->afVertices = NULL;
  psFuncData->afNormals = NULL;
  psFuncData->auIndices = NULL;
  psFuncData->afColours = NULL;
  psFuncData->afTextureCoords = NULL;

  psFuncData->psFunction = NULL;
  psFuncData->psVariables = NULL;

  psFuncData->boColourFunction = FALSE;
  psFuncData->fRed = 0.5f;
  psFuncData->fGreen = 0.5f;
  psFuncData->fBlue = 0.5f;
  psFuncData->fAlpha = 0.5f;
  psFuncData->psRed = NULL;
  psFuncData->psGreen = NULL;
  psFuncData->psBlue = NULL;
  psFuncData->psAlpha = NULL;

	psFuncData->uTexture = TEXTURE_NONE;
  psFuncData->szTexFile = g_string_new ("");
  psFuncData->szTexXScale = g_string_new ("1.0");
  psFuncData->szTexYScale = g_string_new ("1.0");
  psFuncData->szTexXOffset = g_string_new ("0.0");
  psFuncData->szTexYOffset = g_string_new ("0.0");

  psFuncData->szFunction = g_string_new ("");
  psFuncData->szRed = g_string_new ("1.0");
  psFuncData->szGreen = g_string_new ("0.5");
  psFuncData->szBlue = g_string_new ("0.5");
  psFuncData->szAlpha = g_string_new ("0.9");

  psFuncData->boTimeDependent = FALSE;

  SetFunction (FUNCTION0, psFuncData);

  GenerateVertices (psFuncData);
  PopulateVertices (psFuncData);

  return psFuncData;
}

FUNCTYPE GetFunctionType (FuncPersist * psFuncData) {
  return psFuncData->eType;
}

void DeleteFuncPersist (FuncPersist * psFuncData) {
  // Free up the subtype
  switch (psFuncData->eType) {
  case FUNCTYPE_CARTESIAN:
    DeleteCartesianPersist (psFuncData->Func.psCartesianData);
    psFuncData->Func.psCartesianData = NULL;
    break;
  case FUNCTYPE_SPHERICAL:
    DeleteSphericalPersist (psFuncData->Func.psSphericalData);
    psFuncData->Func.psSphericalData = NULL;
    break;
  default:
    // Do absolutely nothing
    break;
  }

  // Free up the function data
  FreeRecursive (psFuncData->psFunction);

  // Free up the colour data
  FreeRecursive (psFuncData->psRed);
  FreeRecursive (psFuncData->psGreen);
  FreeRecursive (psFuncData->psBlue);

  psFuncData->psVariables = FreeVariables (psFuncData->psVariables);

  // Free up the handy UI-related data
  if (psFuncData->szFunction) {
    g_string_free (psFuncData->szFunction, TRUE);
  }
  if (psFuncData->szRed) {
    g_string_free (psFuncData->szRed, TRUE);
  }
  if (psFuncData->szGreen) {
    g_string_free (psFuncData->szGreen, TRUE);
  }
  if (psFuncData->szBlue) {
    g_string_free (psFuncData->szBlue, TRUE);
  }
  if (psFuncData->szAlpha) {
    g_string_free (psFuncData->szAlpha, TRUE);
  }


  if (psFuncData->szTexFile) {
    g_string_free (psFuncData->szTexFile, TRUE);
  }
  if (psFuncData->szTexXScale) {
    g_string_free (psFuncData->szTexXScale, TRUE);
  }
  if (psFuncData->szTexYScale) {
    g_string_free (psFuncData->szTexYScale, TRUE);
  }
  if (psFuncData->szTexXOffset) {
    g_string_free (psFuncData->szTexXOffset, TRUE);
  }
  if (psFuncData->szTexYOffset) {
    g_string_free (psFuncData->szTexYOffset, TRUE);
  }

  // Free up the OpenGL vertex and normal datas
  if (psFuncData->afVertices) {
    g_free (psFuncData->afVertices);
  }
  if (psFuncData->afNormals) {
    g_free (psFuncData->afNormals);
  }
  if (psFuncData->auIndices) {
    g_free (psFuncData->auIndices);
  }
  if (psFuncData->afColours) {
    g_free (psFuncData->afColours);
  }
  if (psFuncData->afTextureCoords) {
    g_free (psFuncData->afTextureCoords);
  }
  
  // Free up any texture
  if (psFuncData->uTexture != TEXTURE_NONE) {
  	UnloadTexture (psFuncData->uTexture);
  }

  g_free (psFuncData);
}

void SetFunction (char const * const szFunction, FuncPersist * psFuncData) {
  switch (psFuncData->eType) {
  case FUNCTYPE_CARTESIAN:
    CartesianSetFunction (szFunction, psFuncData);
    break;
  case FUNCTYPE_SPHERICAL:
    SphericalSetFunction (szFunction, psFuncData);
    break;
  default:
    // Do absolutely nothing
    break;
  }
}

void SetFunctionColours (char const * const szRed, char const * const szGreen, char const * const szBlue, char const * const szAlpha, FuncPersist * psFuncData) {
  switch (psFuncData->eType) {
  case FUNCTYPE_CARTESIAN:
    CartesianSetFunctionColours (szRed, szGreen, szBlue, szAlpha, psFuncData);
    break;
  case FUNCTYPE_SPHERICAL:
    SphericalSetFunctionColours (szRed, szGreen, szBlue, szAlpha, psFuncData);
    break;
  default:
    // Do absolutely nothing
    break;
  }
}

char const * GetFunctionString (FuncPersist * psFuncData) {
  return psFuncData->szFunction->str;
}

char const * GetRedString (FuncPersist * psFuncData) {
  return psFuncData->szRed->str;
}

char const * GetGreenString (FuncPersist * psFuncData) {
  return psFuncData->szGreen->str;
}

char const * GetBlueString (FuncPersist * psFuncData) {
  return psFuncData->szBlue->str;
}

char const * GetAlphaString (FuncPersist * psFuncData) {
  return psFuncData->szAlpha->str;
}

char const * GetTexFileString (FuncPersist * psFuncData) {
  return psFuncData->szTexFile->str;
}

char const * GetTexXOffsetString (FuncPersist * psFuncData) {
  return psFuncData->szTexXOffset->str;
}

char const * GetTexYOffsetString (FuncPersist * psFuncData) {
  return psFuncData->szTexYOffset->str;
}

char const * GetTexXScaleString (FuncPersist * psFuncData) {
  return psFuncData->szTexXScale->str;
}

char const * GetTexYScaleString (FuncPersist * psFuncData) {
  return psFuncData->szTexYScale->str;
}

void SetFunctionRange (double fXMin, double fYMin, double fZMin, double fXWidth, double fYWidth, double fZWidth, FuncPersist * psFuncData) {
  psFuncData->fXMin = fXMin;
  psFuncData->fYMin = fYMin;
  psFuncData->fZMin = fZMin;

  psFuncData->fXWidth = fXWidth;
  psFuncData->fYWidth = fYWidth;
  psFuncData->fZWidth = fZWidth;

  PopulateVertices (psFuncData);
}

void GetFunctionRange (double * afRange, FuncPersist * psFuncData) {
  if (afRange) {
    afRange[0] = psFuncData->fXMin;
    afRange[1] = psFuncData->fYMin;
    afRange[2] = psFuncData->fZMin;
    afRange[3] = psFuncData->fXWidth;
    afRange[4] = psFuncData->fYWidth;
    afRange[5] = psFuncData->fZWidth;
  }
}

void SetFunctionPosition (double fXMin, double fYMin, double fZMin, FuncPersist * psFuncData) {
  switch (psFuncData->eType) {
  case FUNCTYPE_CARTESIAN:
    CartesianSetFunctionPosition (fXMin, fYMin, fZMin, psFuncData);
    break;
  case FUNCTYPE_SPHERICAL:
    SphericalSetFunctionPosition (fXMin, fYMin, fZMin, psFuncData);
    break;
  default:
    // Do absolutely nothing
    break;
  }
}

void SetFunctionAccuracy (double fAccuracy, FuncPersist * psFuncData) {
  // Don't do anything if the accuracy hasn't changed
  if (psFuncData->fAccuracy != fAccuracy) {
    psFuncData->fAccuracy = fAccuracy;

    // Need to regenerate the vertices
    if (psFuncData->afVertices) {
      g_free (psFuncData->afVertices);
      psFuncData->afVertices = NULL;
    }
    // Need to regenerate the normal data
    if (psFuncData->afNormals) {
      g_free (psFuncData->afNormals);
      psFuncData->afNormals = NULL;
    }
    // Need to regenerate the index data
    if (psFuncData->auIndices) {
      g_free (psFuncData->auIndices);
      psFuncData->auIndices = NULL;
    }

    // May need to regenerate the colour data
    if (psFuncData->afColours) {
      g_free (psFuncData->afColours);
      psFuncData->afColours = NULL;
    }

    // Need to regenerate the texture data
    if (psFuncData->afTextureCoords) {
      g_free (psFuncData->afTextureCoords);
      psFuncData->afTextureCoords = NULL;
    }

    GenerateVertices (psFuncData);
    PopulateVertices (psFuncData);
  }
}

double GetFunctionAccuracy (FuncPersist * psFuncData) {
  return psFuncData->fAccuracy;
}

GLfloat * GetVertices (FuncPersist * psFuncData) {
  return psFuncData->afVertices;
}

GLfloat * GetNormals (FuncPersist * psFuncData) {
  return psFuncData->afNormals;
}

GLushort * GetIndices (FuncPersist * psFuncData) {
  return psFuncData->auIndices;
}

GLfloat * GetColours (FuncPersist * psFuncData) {
  return psFuncData->afColours;
}

GLfloat * GetTextureCoords (FuncPersist * psFuncData) {
  return psFuncData->afTextureCoords;
}

bool GetColour (float * afGraphColour, FuncPersist * psFuncData) {
  if (!psFuncData->boColourFunction) {
    afGraphColour[0] = psFuncData->fRed;
    afGraphColour[1] = psFuncData->fGreen;
    afGraphColour[2] = psFuncData->fBlue;
    afGraphColour[3] = psFuncData->fAlpha;
  }

  return !psFuncData->boColourFunction;
}

void GetVertexDimensions (int * pnV1Vertices, int * pnV2Vertices, FuncPersist * psFuncData) {
  switch (psFuncData->eType) {
  case FUNCTYPE_CARTESIAN:
    CartesianGetVertexDimensions (pnV1Vertices, pnV2Vertices, psFuncData);
    break;
  case FUNCTYPE_SPHERICAL:
    SphericalGetVertexDimensions (pnV1Vertices, pnV2Vertices, psFuncData);
    break;
  default:
    // Do absolutely nothing
    break;
  }
}

void PopulateVertices (FuncPersist * psFuncData) {
  switch (psFuncData->eType) {
  case FUNCTYPE_CARTESIAN:
    CartesianPopulateVertices (psFuncData);
    break;
  case FUNCTYPE_SPHERICAL:
    SphericalPopulateVertices (psFuncData);
    break;
  default:
    // Do absolutely nothing
    break;
  }
}

void GenerateVertices (FuncPersist * psFuncData) {
  switch (psFuncData->eType) {
  case FUNCTYPE_CARTESIAN:
    CartesianGenerateVertices (psFuncData);
    break;
  case FUNCTYPE_SPHERICAL:
    SphericalGenerateVertices (psFuncData);
    break;
  default:
    // Do absolutely nothing
    break;
  }
}

bool GetTimeDependent (FuncPersist * psFuncData) {
  return psFuncData->boTimeDependent;
}

void SetFunctionTime (double fTime, FuncPersist * psFuncData) {
  if (psFuncData->psVariableT) {
    SetVariable (psFuncData->psVariableT, fTime);
  }
}

void SetFunctionTexture (GLuint uTexture, FuncPersist * psFuncData) {
	psFuncData->uTexture = uTexture;
}

GLuint GetFunctionTexture (FuncPersist * psFuncData) {
	return psFuncData->uTexture;
}

void LoadTexture (char const * const szFilename, FuncPersist * psFuncData) {
  // Free up any texture
  if (psFuncData->uTexture != TEXTURE_NONE) {
  	UnloadTexture (psFuncData->uTexture);
  	psFuncData->uTexture = TEXTURE_NONE;
  }

	psFuncData->uTexture = LoadTextureRaw (szFilename, 2048, 2048, true);
}

void SetTextureValues (char const * const szFilename, char const * const szXScale, char const * const szYScale, char const * const szXOffset, char const * const szYOffset, FuncPersist * psFuncData) {
	// Load a new texture file if the filename has changed
	if (g_strcmp0 (szFilename, psFuncData->szTexFile->str) != 0) {
	  g_string_assign (psFuncData->szTexFile, szFilename);
		if (psFuncData->uTexture != TEXTURE_NONE) {
			UnloadTexture (psFuncData->uTexture);
			psFuncData->uTexture = TEXTURE_NONE;
		}
		if (g_strcmp0 (psFuncData->szTexFile->str, "") != 0) {
			LoadTexture (szFilename, psFuncData);
		}
	}

	g_string_assign (psFuncData->szTexXScale, szXScale);
	g_string_assign (psFuncData->szTexYScale, szYScale);
	g_string_assign (psFuncData->szTexXOffset, szXOffset);
	g_string_assign (psFuncData->szTexYOffset, szYOffset);
}

