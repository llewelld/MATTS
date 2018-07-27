/*$T MATTS/cObject.h GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cObject.h ;
 * Last Modified: 20/01/05 ;
 * ;
 * Purpose: Header file for cObject ;
 */
#ifndef COBJECT_H
#define COBJECT_H

#include <windows.h>
#include <windowsx.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glut.h>

#define ARROWHEAD_HEIGHT	(0.12)
#define ARROWHEAD_BASE		(0.04)
typedef enum _OBJECT
{
	OBJECT_INVALID		= -1,
	OBJECT_CYLINDER,
	OBJECT_ARROWHEAD,
	OBJECT_SPHERE,
	OBJECT_RADIORANGE,
	OBJECT_MAPSTART,
	OBJECT_MAPTRON		= OBJECT_MAPSTART,
	OBJECT_MAPJMUMAP,
	OBJECT_MAPLIVERPOOL,
	OBJECT_MAPSTJOHN,
	OBJECT_NUM
} OBJECT;

class cObject
{
protected:
	GLuint uStartList;
public:
	cObject ();
	~ cObject ();

	void CallList (OBJECT eObject);
	void NewList (OBJECT eObject);
	void EndList (OBJECT eObject);
};

#endif /* COBJECT_H */
