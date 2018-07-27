///////////////////////////////////////////////////////////////////
// Functy
// 3D graph drawing utility
//
// David Llewellyn-Jones
// http://www.flypig.co.uk
//
// Spring 2009
///////////////////////////////////////////////////////////////////

#ifndef FUNCTION_PRIVATE_H
#define FUNCTION_PRIVATE_H

///////////////////////////////////////////////////////////////////
// Includes

#include "cartesian.h"
#include "spherical.h"

#include <symbolic.h>

///////////////////////////////////////////////////////////////////
// Defines

///////////////////////////////////////////////////////////////////
// Structures and enumerations

struct _FuncPersist {
  FUNCTYPE eType;
  union {
    CartesianPersist * psCartesianData;
    SphericalPersist * psSphericalData;
    void * psNone;
  } Func;

  // The following relate to graph coordinates
  double fXMin;
  double fYMin;
  double fZMin;
  double fXWidth;
  double fYWidth;
  double fZWidth;

  // The following relate to screen coordinates
  double fAccuracy;

  // The actual function
  Operation * psFunction;
  Variable * psVariables;
  Variable * psVariableT;

  // Colour information
  bool boColourFunction;
  float fRed;
  float fGreen;
  float fBlue;
  float fAlpha;
  Operation * psRed;
  Operation * psGreen;
  Operation * psBlue;
  Operation * psAlpha;
  
  // Texture information
  GLuint uTexture;
  GString * szTexFile;
  GString * szTexXScale;
  GString * szTexYScale;
  GString * szTexXOffset;
  GString * szTexYOffset;

  // Handy data to help with the UI
  GString * szFunction;
  GString * szRed;
  GString * szGreen;
  GString * szBlue;
  GString * szAlpha;

  // Storage for the OpenGL vertex and normal data
  GLfloat * afVertices;
  GLfloat * afNormals;
  GLushort * auIndices;
  GLfloat * afColours;
  GLfloat * afTextureCoords;

  // Time related variables
  bool boTimeDependent;
};

///////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////
// Function prototypes

///////////////////////////////////////////////////////////////////
// Function definitions

#endif /* FUNCTION_PRIVATE_H */


