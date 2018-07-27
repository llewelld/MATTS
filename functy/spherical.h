///////////////////////////////////////////////////////////////////
// Functy
// 3D graph drawing utility
//
// David Llewellyn-Jones
// http://www.flypig.co.uk
//
// Spring 2009
///////////////////////////////////////////////////////////////////

#ifndef SPHERICAL_H
#define SPHERICAL_H

///////////////////////////////////////////////////////////////////
// Includes

#include "function.h"

///////////////////////////////////////////////////////////////////
// Defines

///////////////////////////////////////////////////////////////////
// Structures and enumerations

typedef struct _SphericalPersist SphericalPersist;

///////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////
// Function prototypes

SphericalPersist * NewSphericalPersist ();
void DeleteSphericalPersist (SphericalPersist * psSphericalData);

void SphericalSetFunction (char const * const szFunction, FuncPersist * psFuncData);
void SphericalSetFunctionColours (char const * const szRed, char const * const szGreen, char const * const szBlue, char const * const szAlpha, FuncPersist * psFuncData);
void SphericalPopulateVertices (FuncPersist * psFuncData);
void SphericalGetVertexDimensions (int * pnAVertices, int * pnPVertices, FuncPersist * psFuncData);
void SphericalGenerateVertices (FuncPersist * psFuncData);
char const * SphericalGetXCentreString (FuncPersist * psFuncData);
char const * SphericalGetYCentreString (FuncPersist * psFuncData);
char const * SphericalGetZCentreString (FuncPersist * psFuncData);
void SphericalSetFunctionCentre (char const * const szXCentre, char const * const szYCentre, char const * const szZCentre, FuncPersist * psFuncData);
bool SphericalGetCentreTimeDependent (FuncPersist * psFuncData);
void SphericalGetCentre (double * afCentre, FuncPersist * psFuncData);
void SphericalUpdateCentre (FuncPersist * psFuncData);
void SphericalSetFunctionTime (double fTime, FuncPersist * psFuncData);
void SphericalSetFunctionPosition (double fXMin, double fYMin, double fZMin, FuncPersist * psFuncData);

///////////////////////////////////////////////////////////////////
// Function definitions

#endif /* SPHERICAL_H */


