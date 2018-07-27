/*$T MATTS/cModel.cpp GC 1.140 07/01/09 21:12:08 */
/*
 * ;
 * Name: cModel.cpp ;
 * Last Modified: 07/07/08 ;
 * ;
 * Purpose: Render 3DS models using OpenGL ;
 */
#include "cModel.h"
#include "cModelTexture.h"

#include <windows.h>

#include <lib3ds/material.h>
#include <lib3ds/matrix.h>
#include <lib3ds/vector.h>
#define _USE_MATH_DEFINES	1
#include <math.h>

#include <gl/gl.h>
#include <gl/glu.h>

#define MODELPATH	"Resources\\Models"

#define TEX_XSIZE	(1024)
#define TEX_YSIZE	(1024)

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */

cModel::cModel (char const* szName) {
	char szFilename[MAX_PATH];
	psModel = NULL;
	this->szName = (char*) malloc (strlen (szName) + 1);
	strcpy (this->szName, szName);
	_snprintf (szFilename, sizeof (szFilename), "%s\\%s\\%s.3ds", MODELPATH, szName, szName);
	psModel = lib3ds_file_load (szFilename);
	if (psModel == NULL) {
		printf ("Failed to load model %s\n", szFilename);
	} else {
		lib3ds_file_eval (psModel, 1.0f);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cModel::~cModel () {
	if (szName) {
		free (szName);
		szName = NULL;
	}

	if (psModel) {
		lib3ds_file_free (psModel);
		psModel = NULL;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cModel::Render () {
	Lib3dsNode*	 psNode;

	for (psNode = psModel->nodes; psNode != 0; psNode = psNode->next) {
		RenderNode (psNode);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cModel::RenderNode (Lib3dsNode* psNode) {
	ASSERT (psModel);
	{
		Lib3dsNode*	 psChildNode;

		/* Recursively render the nodes */
		for (psChildNode = psNode->childs; psChildNode != NULL; psChildNode = psChildNode->next) {
			RenderNode (psChildNode);
		}
	}

	if (psNode->type == LIB3DS_OBJECT_NODE) {
		Lib3dsMesh*	 psMesh;

		if (strcmp (psNode->name, "$$$DUMMY") == 0) {
			return ;
		}

		psMesh = lib3ds_file_mesh_by_name (psModel, psNode->data.object.morph);
		if (psMesh == NULL)
			psMesh = lib3ds_file_mesh_by_name (psModel, psNode->name);

		if (psMesh == NULL)
			return ;

		if (!psMesh->user.d) {
			ASSERT (psMesh);
			if (!psMesh) {
				return ;
			}

			psMesh->user.d = glGenLists (1);
			glNewList (psMesh->user.d, GL_COMPILE);
			{
				unsigned p;
				Lib3dsVector*  normalL;
				Lib3dsMaterial*	 oldmat;
				normalL = (Lib3dsVector*) malloc (3 * sizeof (Lib3dsVector) * psMesh->faces);
				oldmat = (Lib3dsMaterial*) -1;
				{
					Lib3dsMatrix M;
					lib3ds_matrix_copy (M, psMesh->matrix);
					lib3ds_matrix_inv (M);
					glMultMatrixf (&M[0][0]);
				}

				lib3ds_mesh_calculate_normals (psMesh, normalL);

				for (p = 0; p < psMesh->faces; ++p) {
					Lib3dsFace*	 f;
					Lib3dsMaterial*	 mat;

					cModelTexture*	pt;
					int tex_mode;

					f = &psMesh->faceL[p];
					mat = 0;
					pt = NULL;
					tex_mode = 0;

					if (f->material[0]) {
						mat = lib3ds_file_material_by_name (psModel, f->material);
					}

					if (mat != oldmat) {
						if (mat) {

							/*
							 * if( mat->two_sided ) ;
							 * glDisable(GL_CULL_FACE);
							 * else ;
							 * glEnable(GL_CULL_FACE);
							 * ;
							 * glDisable(GL_CULL_FACE);
							 */
							if (mat->texture1_map.name[0]) {

								/* texture map? */
								Lib3dsTextureMap*  tex;
								tex = &mat->texture1_map;
								if (!tex->user.p) {

									/* no player texture yet? */
									char szFilename[MAX_PATH];
									_snprintf (szFilename, sizeof (szFilename), "%s\\%s\\%s", MODELPATH, szName,
											   tex->name);

									/*
									 * pt = (Player_texture *)malloc(sizeof(Player_texture));
									 */
									pt = new cModelTexture (szFilename);
									tex->user.p = pt;

									/*
									 * snprintf(texname, sizeof(texname), "%s/%s", datapath, tex->name);
									 * strcpy(texname, datapath);
									 * strcat(texname, "/");
									 * strcat(texname, tex->name);
									 */
									printf ("Loading texture map, name %s\n", tex->name);

									/*
									 * pt->bitmap = NULL;
									 * //IMG_Load(tex->name);
									 * pt->bitmap = LoadJPEG (szFilename);
									 */
									if ((pt->bitmap) && (pt->bitmap->w <= TEX_XSIZE) && (pt->bitmap->h <= TEX_YSIZE)) {

										/*
										 * could image be loaded ? ;
										 * this OpenGL texupload code is incomplete format-wise! to make it complete,
										 * examine SDL_surface->format and tell us @lib3ds.sf.net about your
										 * improvements :-)
										 */
										int upload_format;	/* safe choice, shows errors */

										int bytespp;
										void*  pixel;
										unsigned int uSize;
										upload_format = GL_RED;
										bytespp = pt->bitmap->format->BytesPerPixel;
										pixel = NULL;

										glGenTextures (1, &pt->tex_id);

										printf ("Uploading texture to OpenGL, id %d, at %d bytepp\n", pt->tex_id,
												bytespp);

										/*
										 * if (pt->bitmap->format->palette) { ;
										 * pixel = convert_to_RGB_Surface(pt->bitmap);
										 * upload_format = GL_RGBA;
										 * } ;
										 * else {
										 */
										pixel = pt->bitmap->pixels;

										/* e.g. this could also be a color palette */
										if (bytespp == 1)
											upload_format = GL_LUMINANCE;
										else if (bytespp == 3)
											upload_format = GL_RGB;
										else if (bytespp == 4)
											upload_format = GL_RGBA;

										/* } */
										glBindTexture (GL_TEXTURE_2D, pt->tex_id);

										/*
										 * Calculate texture size ;
										 * Used to be fixed at TEX_XSIZE and TEX_YSIZE
										 */
										if (pt->bitmap->w > pt->bitmap->h) {
											uSize = 2 << ((unsigned int) (log ((float) pt->bitmap->w + 1.0f) / M_LN2));
										} else {
											uSize = 2 << ((unsigned int) (log ((float) pt->bitmap->h + 1.0f) / M_LN2));
										}

										glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, uSize, uSize, 0, GL_RGBA,
													  GL_UNSIGNED_BYTE, NULL);

										/*
										 * glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, pt->bitmap->w, pt->bitmap->h, 0,
										 * GL_RGB, GL_UNSIGNED_BYTE, pixel);
										 */
										glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
										glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
										glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
										glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
										glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
										glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, pt->bitmap->w, pt->bitmap->h,
														 upload_format, GL_UNSIGNED_BYTE, pixel);
										pt->scale_x = (float) pt->bitmap->w / (float) uSize;
										pt->scale_y = (float) pt->bitmap->h / (float) uSize;
										glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

										pt->valid = 1;
									} else {
										fprintf (stderr,
												 "Load of texture %s did not succeed ""(format not supported !)\n", tex->name);
										pt->valid = 0;
									}
								} else {
									pt = (cModelTexture*) tex->user.p;
								}

								tex_mode = pt->valid;
							} else {
								tex_mode = 0;
							}

							/*/
							 */
							glMaterialfv (GL_FRONT, GL_AMBIENT, mat->ambient);
							glMaterialfv (GL_FRONT, GL_DIFFUSE, mat->diffuse);
							glMaterialfv (GL_FRONT, GL_SPECULAR, mat->specular);
							glMaterialf (GL_FRONT, GL_SHININESS, (GLfloat) pow (2, 10.0 * mat->shininess));
						} else {
							static const Lib3dsRgba a = { 0.7f, 0.7f, 0.7f, 1.0f };
							static const Lib3dsRgba d = { 0.7f, 0.7f, 0.7f, 1.0f };
							static const Lib3dsRgba s = { 1.0f, 1.0f, 1.0f, 1.0f };
							glMaterialfv (GL_FRONT, GL_AMBIENT, a);
							glMaterialfv (GL_FRONT, GL_DIFFUSE, d);
							glMaterialfv (GL_FRONT, GL_SPECULAR, s);
							glMaterialf (GL_FRONT, GL_SHININESS, (GLfloat) pow (2, 10.0 * 0.5));
						}

						oldmat = mat;
					} else if (mat != NULL && mat->texture1_map.name[0]) {
						Lib3dsTextureMap*  tex;
						tex = &mat->texture1_map;

						if (tex != NULL && tex->user.p != NULL) {
							pt = (cModelTexture*) tex->user.p;
							tex_mode = pt->valid;
						}
					}

				{
					int i;

					if (tex_mode) {

						/*
						 * printf("Binding texture %d\n", pt->tex_id);
						 */
						glEnable (GL_TEXTURE_2D);
						glBindTexture (GL_TEXTURE_2D, pt->tex_id);
					}

					glBegin (GL_TRIANGLES);
					glNormal3fv (f->normal);
					for (i = 0; i < 3; ++i) {
						glNormal3fv (normalL[3 * p + i]);

						if (tex_mode) {
							glTexCoord2f (psMesh->texelL[f->points[i]][0] * pt->scale_x,
										  pt->scale_y - psMesh->texelL[f->points[i]][1] * pt->scale_y);
						}

						glVertex3fv (psMesh->pointL[f->points[i]].pos);
					}

					glEnd ();

					if (tex_mode)
						glDisable (GL_TEXTURE_2D);
				}
				}

				free (normalL);
			}

			glEndList ();
		}

		if (psMesh->user.d) {
			Lib3dsObjectData*  d;

			glPushMatrix ();
			d = &psNode->data.object;
			glMultMatrixf (&psNode->matrix[0][0]);
			glTranslatef (-d->pivot[0], -d->pivot[1], -d->pivot[2]);
			glCallList (psMesh->user.d);

			/*
			 * glutSolidSphere (0.1, 20, 20);
			 */
			glPopMatrix ();
		}
	}
}
