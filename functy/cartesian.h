///////////////////////////////////////////////////////////////////
// Functy
// 3D graph drawing utility
//
// David Llewellyn-Jones
// http://www.flypig.co.uk
//
// Spring 2009
///////////////////////////////////////////////////////////////////

#ifndef CARTESIAN_H
#define CARTESIAN_H

///////////////////////////////////////////////////////////////////
// Includes

#include "function.h"

///////////////////////////////////////////////////////////////////
// Defines

///////////////////////////////////////////////////////////////////
// Structures and enumerations

typedef struct _CartesianPersist CartesianPersist;

///////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////
// Function prototypes

CartesianPersist * NewCartesianPersist ();
void DeleteCartesianPersist (CartesianPersist * psCartesianData);

void CartesianSetFunction (char const * const szFunction, FuncPersist * psFuncData);
void CartesianSetFunctionColours (char const * const szRed, char const * const szGreen, char const * const szBlue, char const * const szAlpha, FuncPersist * psFuncData);
void CartesianPopulateVertices (FuncPersist * psFuncData);
void CartesianGetVertexDimensions (int * pnXVertices, int * pnYVertices, FuncPersist * psFuncData);
void CartesianGenerateVertices (FuncPersist * psFuncData);
void CartesianSetFunctionPosition (double fXMin, double fYMin, double fZMin, FuncPersist * psFuncData);
double CartesianGetFunctionZ (double fX, double fY, FuncPersist * psFuncData);

///////////////////////////////////////////////////////////////////
// Function definitions

#endif /* CARTESIAN_H */


