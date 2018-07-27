/*$T MATTS/cTexture.cpp GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cRender.cpp ;
 * Last Modified: 20/12/05 ;
 * ;
 * Purpose: Render the topology ;
 */
#include "cTexture.h"
#include "cModel.h"
#include "cModelTexture.h"
#include <png.h>

#define PNG_BYTES_TO_CHECK	(4)

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cTexture::cTexture () {
	nFloatingNum = 0;

	/* Load the textures */
	aunTextureName[TEXNAME_TRON] = LoadTextureJPEG ("Resources\\Tron.jpg");
	aunTextureName[TEXNAME_JMUMAP] = LoadTextureJPEG ("Resources\\jmumap.jpg");
	aunTextureName[TEXNAME_RENDERMAP] = LoadTextureJPEG ("Resources\\RenderMap.jpg");
	aunTextureName[TEXNAME_REDBLOB] = LoadTexturePNG ("Resources\\AnalyseRed.png");
	aunTextureName[TEXNAME_AMBERBLOB] = LoadTexturePNG ("Resources\\AnalyseAmber.png");
	aunTextureName[TEXNAME_GREENBLOB] = LoadTexturePNG ("Resources\\AnalyseGreen.png");
	aunTextureName[TEXNAME_RADIORANGE1] = LoadTexturePNG ("Resources\\RangeCore.png");
	aunTextureName[TEXNAME_RADIORANGE2] = LoadTexturePNG ("Resources\\RangeEdge.png");

	/* HUD Buttons */
	aunTextureName[TEXNAME_BUTTONSELECT] = LoadTexturePNG ("Resources\\ButtonSelect.png");
	aunTextureName[TEXNAME_BUTTONSELECTD] = LoadTexturePNG ("Resources\\ButtonSelectD.png");
	aunTextureName[TEXNAME_BUTTONLINKS] = LoadTexturePNG ("Resources\\ButtonLinks.png");
	aunTextureName[TEXNAME_BUTTONLINKSD] = LoadTexturePNG ("Resources\\ButtonLinksD.png");
	aunTextureName[TEXNAME_BUTTONSPIN] = LoadTexturePNG ("Resources\\ButtonSpin.png");
	aunTextureName[TEXNAME_BUTTONSPIND] = LoadTexturePNG ("Resources\\ButtonSpinD.png");
	aunTextureName[TEXNAME_BUTTONNET] = LoadTexturePNG ("Resources\\ButtonNet.png");
	aunTextureName[TEXNAME_BUTTONNETD] = LoadTexturePNG ("Resources\\ButtonNetD.png");
	aunTextureName[TEXNAME_BUTTONDETAILS] = LoadTexturePNG ("Resources\\ButtonDetails.png");
	aunTextureName[TEXNAME_BUTTONDETAILSD] = LoadTexturePNG ("Resources\\ButtonDetailsD.png");
	aunTextureName[TEXNAME_BUTTONLEFT] = LoadTexturePNG ("Resources\\ButtonLeft.png");
	aunTextureName[TEXNAME_BUTTONLEFTD] = LoadTexturePNG ("Resources\\ButtonLeftD.png");
	aunTextureName[TEXNAME_BUTTONBLANK] = LoadTexturePNG ("Resources\\ButtonBlank.png");
	aunTextureName[TEXNAME_BUTTONBLANKD] = LoadTexturePNG ("Resources\\ButtonBlankD.png");
	aunTextureName[TEXNAME_BUTTONRIGHT] = LoadTexturePNG ("Resources\\ButtonRight.png");
	aunTextureName[TEXNAME_BUTTONRIGHTD] = LoadTexturePNG ("Resources\\ButtonRightD.png");
	aunTextureName[TEXNAME_LEVEL] = LoadTexturePNG ("Resources\\Level.png");
	//aunTextureName[TEXNAME_LOGO] = LoadTexturePNG ("Resources\\logo-128x128.png");
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cTexture::~cTexture () {
	/* Delete the textures */
	glDeleteTextures (TEXNAME_NUM, aunTextureName);

	glDeleteTextures (nFloatingNum, aunTextureName + TEXNAME_NUM);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cTexture::LoadFloatingTexture (char const * szFilename) {
	int nTexture;

	if (nFloatingNum < FLOATING_MAX) {
		nTexture = TEXNAME_NUM + nFloatingNum;
		aunTextureName[nTexture] = LoadTexturePNG (szFilename);
		nFloatingNum++;
	}
	else {
		nTexture = 0;
	}

	return (nTexture);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cTexture::GetFloatingNum () {
	return nFloatingNum;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
GLuint cTexture::LoadTextureRaw (char const *const szFilename, int const nWidth, int const nHeight, bool boAlpha) {
	GLuint unTextureName;
	void*  cData;
	FILE*  hFile;
	int nComponents;
	int uFormat;

	unTextureName = 0;
	if (boAlpha) {
		nComponents = 4;
		uFormat = GL_RGBA;
	} else {
		nComponents = 3;
		uFormat = GL_RGB;
	}

	hFile = fopen (szFilename, "rb");
	if (hFile) {
		cData = malloc (nWidth * nHeight * nComponents);
		if (cData) {
			fread (cData, nWidth * nHeight * nComponents, 1, hFile);
			fclose (hFile);
			glGenTextures (1, &unTextureName);

			glBindTexture (GL_TEXTURE_2D, unTextureName);
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			gluBuild2DMipmaps (GL_TEXTURE_2D, nComponents, nWidth, nHeight, uFormat, GL_UNSIGNED_BYTE, cData);
			free (cData);
		}
	}

	return unTextureName;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
GLuint cTexture::LoadTextureJPEG (char const *const szFilename) {
	GLuint unTextureName;

	cModelTexture*	psTexture;
	unTextureName = 0;
	psTexture = new cModelTexture (szFilename);

	if ((psTexture) && (psTexture->bitmap) && (psTexture->bitmap->pixels)) {
		glGenTextures (1, &unTextureName);

		glBindTexture (GL_TEXTURE_2D, unTextureName);
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		gluBuild2DMipmaps (GL_TEXTURE_2D, 3, psTexture->bitmap->w, psTexture->bitmap->h, GL_RGB, GL_UNSIGNED_BYTE,
						   psTexture->bitmap->pixels);
	}

	if (psTexture) {
		delete psTexture;
	}

	return unTextureName;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
GLuint cTexture::LoadTexturePNG (char const *const szFilename) {
	GLuint unTextureName;
	png_structp psPng;
	png_infop psInfo;
	unsigned int uSigRead;
	png_uint_32 uWidth;
	png_uint_32 uHeight;
	int nBitDepth;
	int nColourType;
	int nInterlaceType;
	png_byte acTestBuffer[PNG_BYTES_TO_CHECK];
	void*  cData;
	FILE*  hFile;
	float fScreenGamma; /* Apparently a good guess for a PC monitors in a dimly lit room */
	int nIntent;
	double fImageGamma;
	unsigned int uRow;
	png_bytep*	apRowPointers;

	unTextureName = 0;
	psPng = NULL;
	psInfo = NULL;
	uSigRead = 0;
	fScreenGamma = 2.2;

	hFile = fopen (szFilename, "rb");
	if (hFile) {
		acTestBuffer[0] = 0;
		uSigRead = (unsigned int) fread (acTestBuffer, 1, PNG_BYTES_TO_CHECK, hFile);

		/* Check if this is a PNG file */
		if (png_sig_cmp (acTestBuffer, (png_size_t) 0, uSigRead) == 0) {

			/* Set up the PNG structure */
			psPng = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
			if (psPng) {

				/* Set up image information structure */
				psInfo = png_create_info_struct (psPng);
			}

			/* Check the structures have been set up and set the error handler */
			if (psPng && psInfo && (!setjmp (png_jmpbuf (psPng)))) {

				/* Use standard C stream as I/O method */
				png_init_io (psPng, hFile);

				/* Skip the bit we already read in */
				png_set_sig_bytes (psPng, uSigRead);

				/* Read all of the info about the PNG from the file */
				png_read_info (psPng, psInfo);

				png_get_IHDR (psPng, psInfo, &uWidth, &uHeight, &nBitDepth, &nColourType, &nInterlaceType, NULL, NULL);

				/* Reduce 16 bits per colour down to 8 bits per colour if necessary */
				png_set_strip_16 (psPng);

				/* Expand paletted colours to RGB if necessary */
				if (nColourType == PNG_COLOR_TYPE_PALETTE) {
					png_set_palette_to_rgb (psPng);
				}

				/* Expand greyscale images to 8 bits per colour if necessary */
				if ((nColourType == PNG_COLOR_TYPE_GRAY) && (nBitDepth < 8)) {
					png_set_gray_to_rgb (psPng);
				}

				/* Expand image to be in RGBA quartets if necessary */
				if (png_get_valid (psPng, psInfo, PNG_INFO_tRNS)) {
					png_set_tRNS_to_alpha (psPng);
				}

				/* Deal with Gamma correction and stuff like that */
				if (png_get_sRGB (psPng, psInfo, &nIntent)) {
					png_set_gamma (psPng, fScreenGamma, 0.45455);
				} else {
					if (png_get_gAMA (psPng, psInfo, &fImageGamma)) {
						png_set_gamma (psPng, fScreenGamma, fImageGamma);
					} else {
						png_set_gamma (psPng, fScreenGamma, 0.45455);
					}
				}

				png_read_update_info (psPng, psInfo);

				/* Allocate memory for the row pointers */
				apRowPointers = (png_bytep*) malloc (uHeight * sizeof (png_bytep));

				/* Allocate memory for the pixel data */
				cData = png_malloc (psPng, (uHeight * png_get_rowbytes (psPng, psInfo)));

				if (cData) {

					/* Set up row data pointers to allocated memory */
					for (uRow = 0; uRow < uHeight; uRow++) {
						apRowPointers[uRow] = (png_bytep) cData + (uRow * png_get_rowbytes (psPng, psInfo));
					}

					/* Read the image into the memory */
					png_read_image (psPng, apRowPointers);

					/* Read anything left over */
					png_read_end (psPng, psInfo);

					/* We don't need the row pointers anymore */
					free (apRowPointers);

					/* Now convert the data into an OpenGL texture */
					glGenTextures (1, &unTextureName);

					glBindTexture (GL_TEXTURE_2D, unTextureName);
					glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
					glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
					glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
					glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

					gluBuild2DMipmaps (GL_TEXTURE_2D, 4, uWidth, uHeight, GL_RGBA, GL_UNSIGNED_BYTE, cData);

					/* Don't need the pixel data anymore */
					png_free (psPng, cData);
				}
			}

			/* Clean things up and free allocated memory */
			png_destroy_read_struct (&psPng, &psInfo, NULL);
		}

		fclose (hFile);
	}

	return unTextureName;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
GLuint cTexture::GetTexture (TEXNAME eTexture) {
	return aunTextureName[eTexture];
}
