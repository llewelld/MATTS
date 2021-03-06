///////////////////////////////////////////////////////////////////
// Utils
// Generally useful definitions, structures, functions, etc.
//
// David Llewellyn-Jones
// http://www.flypig.co.uk
//
// Spring 2008
///////////////////////////////////////////////////////////////////

#ifndef UTILS_H
#define UTILS_H

///////////////////////////////////////////////////////////////////
// Includes

#define _USE_MATH_DEFINES
#include <math.h>
#ifndef M_TWOPI
#define M_TWOPI (M_PI * 2.0)
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#endif

#include "GString.h"
#include <GL/glut.h>
//#include <gtk/gtk.h>
#include <stdio.h>

///////////////////////////////////////////////////////////////////
// Defines

#if defined (WIN32) && !defined (NAN)
static const unsigned long nan[2] = {0xffffffff, 0x7fffffff};
#define NAN (*(const double *) nan)
#define isnan _isnan
#endif

///////////////////////////////////////////////////////////////////
// Structures and enumerations

typedef struct _Vector3 {
  GLfloat fX;
  GLfloat fY;
  GLfloat fZ;
} Vector3;

typedef struct _Matrix3 {
  GLfloat fA1;
  GLfloat fA2;
  GLfloat fA3;
  GLfloat fB1;
  GLfloat fB2;
  GLfloat fB3;
  GLfloat fC1;
  GLfloat fC2;
  GLfloat fC3;
} Matrix3;

///////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////
// Function prototypes

float absf (float fValue);
Vector3 Normal (Vector3 * v1, Vector3 * v2);
void Normalise (Vector3 * v1);
void Normalise3f (float * pfX, float * pfY, float * pfZ);
float Length (Vector3 * v1);
Matrix3 Invert (Matrix3 * m1);
float Determinant (Matrix3 * m1);
float DotProdAngle (float fX1, float fY1, float fX2, float fY2);
Vector3 MultMatrixVector (Matrix3 * m1, Vector3 * v1);
void PrintMatrix (Matrix3 * m1);
void PrintVector (Vector3 * v1);

///////////////////////////////////////////////////////////////////
// Function definitions

#endif /* UTILS_H */

