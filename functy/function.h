///////////////////////////////////////////////////////////////////
// Functy
// 3D graph drawing utility
//
// David Llewellyn-Jones
// http://www.flypig.co.uk
//
// Spring 2009
///////////////////////////////////////////////////////////////////

#ifndef FUNCTION_H
#define FUNCTION_H

///////////////////////////////////////////////////////////////////
// Includes

#include "utils.h"

///////////////////////////////////////////////////////////////////
// Defines

#define AXIS_XSIZE          (40.0)
#define AXIS_YSIZE          (40.0)
#define AXIS_ZSIZE          (40.0)
#define AXIS_XHSIZE         (AXIS_XSIZE / 2.0)
#define AXIS_YHSIZE         (AXIS_YSIZE / 2.0)
#define AXIS_ZHSIZE         (AXIS_ZSIZE / 2.0)

///////////////////////////////////////////////////////////////////
// Structures and enumerations

typedef struct _FuncPersist FuncPersist;

typedef enum _FUNCTYPE {
  FUNCTYPE_INVALID = -1,

  FUNCTYPE_CARTESIAN,
  FUNCTYPE_SPHERICAL,

  FUNCTYPE_NUM
} FUNCTYPE;

///////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////
// Function prototypes

FuncPersist * NewFuncPersist (FUNCTYPE eType, double fAccuracy);
void DeleteFuncPersist (FuncPersist * psFuncData);

FUNCTYPE GetFunctionType (FuncPersist * psFuncData);
void SetFunction (char const * const szFunction, FuncPersist * psFuncData);
void SetFunctionColours (char const * const szRed, char const * const szGreen, char const * const szBlue, char const * const szAlpha, FuncPersist * psFuncData);
char const * GetFunctionString (FuncPersist * psFuncData);
char const * GetRedString (FuncPersist * psFuncData);
char const * GetGreenString (FuncPersist * psFuncData);
char const * GetBlueString (FuncPersist * psFuncData);
char const * GetAlphaString (FuncPersist * psFuncData);
char const * GetTexFileString (FuncPersist * psFuncData);
char const * GetTexXOffsetString (FuncPersist * psFuncData);
char const * GetTexYOffsetString (FuncPersist * psFuncData);
char const * GetTexXScaleString (FuncPersist * psFuncData);
char const * GetTexYScaleString (FuncPersist * psFuncData);
void SetFunctionRange (double fXMin, double fYMin, double fZMin, double fXWidth, double fYWidth, double fZWidth, FuncPersist * psFuncData);
void GetFunctionRange (double * afRange, FuncPersist * psFuncData);
void SetFunctionPosition (double fXMin, double fYMin, double fZMin, FuncPersist * psFuncData);
void SetFunctionAccuracy (double fAccuracy, FuncPersist * psFuncData);
double GetFunctionAccuracy (FuncPersist * psFuncData);
GLfloat * GetVertices (FuncPersist * psFuncData);
GLfloat * GetNormals (FuncPersist * psFuncData);
GLushort * GetIndices (FuncPersist * psFuncData);
GLfloat * GetColours (FuncPersist * psFuncData);
GLfloat * GetTextureCoords (FuncPersist * psFuncData);
bool GetColour (float * afGraphColour, FuncPersist * psFuncData);
void GetVertexDimensions (int * pnXVertices, int * pnYVertices, FuncPersist * psFuncData);
void PopulateVertices (FuncPersist * psFuncData);
bool GetTimeDependent (FuncPersist * psFuncData);
void SetFunctionTime (double fTime, FuncPersist * psFuncData);
void SetFunctionTexture (GLuint uTexture, FuncPersist * psFuncData);
GLuint GetFunctionTexture (FuncPersist * psFuncData);
void SetTextureValues (char const * const szFilename, char const * const szXScale, char const * const szYScale, char const * const szXOffset, char const * const szYOffset, FuncPersist * psFuncData);


///////////////////////////////////////////////////////////////////
// Function definitions

#endif /* FUNCTION_H */


