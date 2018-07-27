/*$T MATTS/cModel.h GC 1.140 07/01/09 21:12:08 */
/*
 * ;
 * Name: cModel.h ;
 * Last Modified: 07/07/08 ;
 * ;
 * Purpose: Render 3DS models using OpenGL ;
 */
#ifndef CMODEL_H
#define CMODEL_H

#include <lib3ds/types.h>
#include <lib3ds/file.h>
#include <lib3ds/node.h>
#include <lib3ds/mesh.h>

class cModel
{
private:
	char*  szName;
public:
	cModel (char const* szName);
	~ cModel ();
	void Render ();
protected:
	void RenderNode (Lib3dsNode* psNode);
	Lib3dsFile*	 psModel;
};
#endif
