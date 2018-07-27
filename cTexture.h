/*$T MATTS/cTexture.h GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cTexture.h ;
 * Last Modified: 20/01/05 ;
 * ;
 * Purpose: Header file for cTexture ;
 */
#ifndef CTEXTURE_H
#define CTEXTURE_H

#include <windows.h>
#include <windowsx.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glut.h>

#define FLOATING_MAX (64)

typedef enum _TEXNAME
{
	TEXNAME_INVALID			= -1,
	TEXNAME_LEVEL,
	//TEXNAME_LOGO,
	TEXNAME_TRON,
	TEXNAME_JMUMAP,
	TEXNAME_RENDERMAP,
	TEXNAME_REDBLOB,
	TEXNAME_AMBERBLOB,
	TEXNAME_GREENBLOB,
	TEXNAME_RADIORANGE1,
	TEXNAME_RADIORANGE2,
	TEXNAME_BUTTONSELECT,
	TEXNAME_BUTTONSELECTD,
	TEXNAME_BUTTONLINKS,
	TEXNAME_BUTTONLINKSD,
	TEXNAME_BUTTONSPIN,
	TEXNAME_BUTTONSPIND,
	TEXNAME_BUTTONNET,
	TEXNAME_BUTTONNETD,
	TEXNAME_BUTTONDETAILS,
	TEXNAME_BUTTONDETAILSD,
	TEXNAME_BUTTONLEFT,
	TEXNAME_BUTTONLEFTD,
	TEXNAME_BUTTONBLANK,
	TEXNAME_BUTTONBLANKD,
	TEXNAME_BUTTONRIGHT,
	TEXNAME_BUTTONRIGHTD,
	TEXNAME_NUM
} TEXNAME;

class cTexture
{
private:
	GLuint aunTextureName[TEXNAME_NUM + FLOATING_MAX];
	int nFloatingNum;
public:
	cTexture ();
	~ cTexture ();
	int LoadFloatingTexture (char const * szFilename);
	int GetFloatingNum ();
	GLuint LoadTextureRaw (char const *const szFilename, int const nWidth, int const nHeight, bool boAlpha);
	GLuint LoadTextureJPEG (char const *const szFilename);
	GLuint LoadTexturePNG (char const *const szFilename);
	GLuint GetTexture (TEXNAME eTexture);
};
#endif /* CTEXTURE_H */
