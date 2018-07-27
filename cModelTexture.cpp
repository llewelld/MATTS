/*$T MATTS/cModelTexture.cpp GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cModelTexture.cpp ;
 * Last Modified: 07/07/08 ;
 * ;
 * Purpose: Create textures from JPEGS for OpenGL rendering ;
 */
#include "cModelTexture.h"
#include <stdio.h>

extern "C"
{
#include "jpeglib.h"
}

#include <windows.h>

/* include <setjmp.h> */
#include <gl/gl.h>

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */

cModelTexture::cModelTexture (char const* filename) {
	unsigned long width;
	unsigned long height;
	TextureSurface*	 psTexture;

	psTexture = NULL;
	GetSize (filename, &width, &height);
	if ((width > 0) && (height > 0)) {
		psTexture = (TextureSurface*) malloc (sizeof (TextureSurface));
		psTexture->format = (TexturePixelFormat*) malloc (sizeof (TexturePixelFormat));
		psTexture->format->palette = NULL;
		psTexture->w = width;
		psTexture->h = height;
		psTexture->pixels = malloc (width * height * 3);
		LoadJPEGOrig (filename, 24, psTexture->pixels);
		psTexture->format->BytesPerPixel = 3;
	}

	bitmap = psTexture;
	szName = NULL;
	valid = 0;
	tex_id = 0;
	scale_x = 1.0f;
	scale_y = 1.0f;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cModelTexture::~cModelTexture () {
	if (szName) {
		free (szName);
		szName = NULL;
	}

	if (bitmap) {
		if (bitmap->format) {
			free (bitmap->format);
			bitmap->format = NULL;
		}

		if (bitmap->pixels) {
			free (bitmap->pixels);
			bitmap->pixels = NULL;
		}

		if (valid) {
			glDeleteTextures (1, &tex_id);
		}

		free (bitmap);
		bitmap = NULL;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void* cModelTexture::DetachTexture () {
	void*  pixels;
	pixels = bitmap->pixels;

	bitmap->pixels = NULL;
	return pixels;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cModelTexture::LoadJPEGOrig (char const* filename, int bitsperpixel, void* where) {
	struct jpeg_decompress_struct cinfo;

	/*
	 * struct my_error_mgr jerr;
	 */
	FILE*  infile;		/* source file */
	JSAMPARRAY buffer;	/* Output row buffer */
	int row_stride;		/* physical row width in output buffer */
	struct jpeg_error_mgr pub;

	int y;		/* jerr.boError
				 * FALSE;
				 * */
	if ((infile = fopen (filename, "rb")) == NULL) {
		return ;
	}

	cinfo.err = jpeg_std_error (&pub);
	pub.error_exit = NULL;

	/*
	 * cinfo.err = jpeg_std_error(&jerr.pub);
	 * jerr.pub.error_exit = my_error_exit;
	 * if (jerr.boError) { ;
	 * jpeg_destroy_decompress(&cinfo);
	 * fclose(infile);
	 * return;
	 * }
	 */
	jpeg_create_decompress (&cinfo);
	jpeg_stdio_src (&cinfo, infile);
	(void) jpeg_read_header (&cinfo, TRUE);
	(void) jpeg_start_decompress (&cinfo);
	row_stride = cinfo.output_width * cinfo.output_components;
	buffer = (*cinfo.mem->alloc_sarray) ((j_common_ptr) & cinfo, JPOOL_IMAGE, row_stride, 1);

	y = 0;

	/*
	 * cinfo.output_height-1;
	 */
	while (cinfo.output_scanline < cinfo.output_height) {
		unsigned char*	super;
		(void) jpeg_read_scanlines (&cinfo, buffer, 1);

		super = *buffer;
		for (WORD x = 0; x < cinfo.output_width; x++) {
			unsigned char r;
			unsigned char g;
			unsigned char b;
			r = *super;
			super++;
			g = *super;

			super++;
			b = *super;

			super++;
			((BYTE*) where)[(x * 3) + ((y * cinfo.output_width) * 3) + 0] = r;
			((BYTE*) where)[(x * 3) + ((y * cinfo.output_width) * 3) + 1] = g;
			((BYTE*) where)[(x * 3) + ((y * cinfo.output_width) * 3) + 2] = b;
		}

		y++;	/* ;
				 * */
	}

	(void) jpeg_finish_decompress (&cinfo);
	jpeg_destroy_decompress (&cinfo);
	fclose (infile);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cModelTexture::GetSize (char const* filename, unsigned long* width, unsigned long* height) {
	struct jpeg_decompress_struct cinfo;

	/*
	 * struct my_error_mgr jerr;
	 */
	FILE*  infile;
	struct jpeg_error_mgr pub;

	if ((infile = fopen (filename, "rb")) == NULL) {
		*width = 0;
		*height = 0;
		return ;
	}

	cinfo.err = NULL;
	cinfo.err = jpeg_std_error (&pub);
	pub.error_exit = NULL;

	/*
	 * cinfo.err = jpeg_std_error(&jerr.pub);
	 * jerr.pub.error_exit = my_error_exit;
	 * if (jerr.boError) ;
	 * { ;
	 * jpeg_destroy_decompress(&cinfo);
	 * fclose(infile);
	 * return;
	 * }
	 */
	jpeg_create_decompress (&cinfo);
	jpeg_stdio_src (&cinfo, infile);
	(void) jpeg_read_header (&cinfo, TRUE);
	(void) jpeg_start_decompress (&cinfo);

	*width = cinfo.output_width;
	*height = cinfo.output_height;

	jpeg_destroy_decompress (&cinfo);
	fclose (infile);
}
