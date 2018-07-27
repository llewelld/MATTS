/*$T MATTS/cModelTexture.h GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cModelTexture.h ;
 * Last Modified: 07/07/08 ;
 * ;
 * Purpose: Create textures from JPEGS for OpenGL rendering ;
 */
#ifndef CMODELTEXTURE_H
#define CMODELTEXTURE_H

typedef struct _TexturePixelFormat
{
	void*  palette;
	unsigned char BytesPerPixel;
} TexturePixelFormat;

/*
 -----------------------------------------------------------------------------------------------------------------------
 *  TextureSurface is here to replace SDL_Surface
 -----------------------------------------------------------------------------------------------------------------------
 */
typedef struct _TextureSurface
{
	TexturePixelFormat*	 format;	/* Read-only */
	int w, h;		/* Read-only */
	void*  pixels;	/* Read-write */
} TextureSurface;

class cModelTexture
{
private:
	void GetSize (char const* filename, unsigned long* width, unsigned long* height);
	void LoadJPEGOrig (char const* filename, int bitsperpixel, void* where);
public:
	cModelTexture (char const* szName);
	~ cModelTexture ();
	void*  DetachTexture ();

	char*  szName;
	int valid;		/* was the loading attempt successful ? */
	TextureSurface*	 bitmap;
	unsigned int tex_id;	/* OpenGL texture ID */
	float scale_x;

	/* OpenGL texture ID */
	float scale_y;			/* scale the texcoords, as OpenGL thinks in TEX_XSIZE and TEX_YSIZE */
};
#endif
