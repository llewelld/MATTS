/*$T MATTS/cObject.cpp GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cObject.cpp ;
 * Last Modified: 20/12/05 ;
 * ;
 * Purpose: Render objects using OpenGL call lists ;
 */
#include "cObject.h"

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */

cObject::cObject () {
	GLUquadricObj*	sQobj;
	GLUquadric*	 psSphereTextureQuad;

	psSphereTextureQuad = gluNewQuadric ();
	gluQuadricTexture (psSphereTextureQuad, GL_TRUE);
	sQobj = gluNewQuadric ();
	gluQuadricDrawStyle (sQobj, GLU_FILL);
	gluQuadricNormals (sQobj, GLU_SMOOTH);

	uStartList = glGenLists (OBJECT_NUM);

	glNewList (uStartList + OBJECT_CYLINDER, GL_COMPILE);
	gluCylinder (sQobj, 0.2, 0.2, 1.0, 5, 1);
	glEndList ();

	glNewList (uStartList + OBJECT_ARROWHEAD, GL_COMPILE);
	glutSolidCone (ARROWHEAD_BASE, ARROWHEAD_HEIGHT, 7, 1);
	glEndList ();

	glNewList (uStartList + OBJECT_SPHERE, GL_COMPILE);
	gluSphere (psSphereTextureQuad, 1.0f, 10, 10);
	glEndList ();

	gluDeleteQuadric (sQobj);
	gluDeleteQuadric (psSphereTextureQuad);

	glNewList (uStartList + OBJECT_RADIORANGE, GL_COMPILE);
	glBegin (GL_QUADS);
	glTexCoord2f (1, 1);
	glVertex3f (0.5f, 0.5f, -0.002f);
	glTexCoord2f (1, 0);
	glVertex3f (0.5f, -0.5f, -0.002f);
	glTexCoord2f (0, 0);
	glVertex3f (-0.5f, -0.5f, -0.002f);
	glTexCoord2f (0, 1);
	glVertex3f (-0.5f, 0.5f, -0.002f);
	glEnd ();
	glEndList ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cObject::~cObject () {
	glDeleteLists (uStartList, OBJECT_NUM);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cObject::CallList (OBJECT eObject) {
	glCallList (uStartList + eObject);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cObject::NewList (OBJECT eObject) {
	glNewList (uStartList + eObject, GL_COMPILE);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cObject::EndList (OBJECT eObject) {
	glEndList ();
}
