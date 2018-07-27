#include "utils.h"
#include "textures.h"

struct _TexPersist {
  GLuint * aunTextureName;
  int nMaxTextures;
  int nTextures;
};

TexPersist * NewTexPersist (int nMaxTextures) {
  TexPersist * psTexData;

  // Allocate some memory for the new structures
  psTexData = g_new0 (TexPersist, 1);
  psTexData->aunTextureName = g_new0 (GLuint, nMaxTextures);

  // Initialise variables
  psTexData->nMaxTextures = nMaxTextures;
  psTexData->nTextures = 0;

  return psTexData;
}

void DeleteTexPersist (TexPersist * psTexData) {
  // Free up all of the textures
  glDeleteTextures (psTexData->nTextures, psTexData->aunTextureName);

  // Free up the structures
  g_free (psTexData->aunTextureName);
  g_free (psTexData);
}

void LoadTextures (TexPersist * psTexData) {
  //LoadTextureRaw (FUNCTYDIR "/Grid.raw", 256, 256, TRUE, psTexData);
  //LoadTextureRaw ("/home/flypig/Documents/Development/Projects/Functy/Star.raw", 256, 256, TRUE, psTexData);
}

//GLuint GetTexture (TEXNAME eTexture, GLfloat afTexCoords[], TexPersist * psTexData) {
//  if (afTexCoords) {
//    afTexCoords[0] = 0;
//    afTexCoords[1] = 1;
//    afTexCoords[2] = 0;
//    afTexCoords[3] = 1;
//  }
//
//  return psTexData->aunTextureName[eTexture];
//}
//
//GLuint GetTexturePortion (TEXNAME eTexture, GLfloat afPortion[], GLfloat afTexCoords[], TexPersist * psTexData) {
//  if (afPortion && afTexCoords) {
//    afTexCoords[0] = afPortion[0];
//    afTexCoords[1] = afPortion[1];
//    afTexCoords[2] = afPortion[2];
//    afTexCoords[3] = afPortion[3];
//  }
//
//  return psTexData->aunTextureName[eTexture];
//}
//
//inline void GetTextureCoord (TEXNAME eTexture, GLfloat fXIn, GLfloat fYIn, GLfloat * pfXOut, GLfloat * pfYOut, TexPersist * psTexData) {
//  if (pfXOut) {
//    *pfXOut = fXIn;
//  }
//  if (pfYOut) {
//    *pfYOut = fYIn;
//  }
//}

GLuint LoadTextureRaw (char const * const szFilename, int const nWidth, int const nHeight, bool boAlpha) {
  GLuint unTextureName = 0;
  void * cData;
  FILE * hFile;
  int nComponents;
  int uFormat;
  size_t uRead;

  if (boAlpha) {
    nComponents = 4;
    uFormat = GL_RGBA;
  }
  else {
    nComponents = 3;
    uFormat = GL_RGB;
  }

  hFile = fopen (szFilename, "rb");
  if (hFile) {
    cData = g_malloc (nWidth * nHeight * nComponents);
    if (cData) {
      uRead = fread (cData, nWidth * nHeight * nComponents, 1, hFile);
      fclose (hFile);

      if (uRead == 1) {
        glGenTextures (1, & unTextureName);

        glBindTexture (GL_TEXTURE_2D, unTextureName);
        glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        gluBuild2DMipmaps (GL_TEXTURE_2D, nComponents, nWidth, nHeight, uFormat, GL_UNSIGNED_BYTE, cData);
      }
      else {
        fprintf (stderr, "Error: Failed to fully load texture %s\n", szFilename);
      }
      g_free (cData);
    }
  }
  else {
    fprintf (stderr, "Error: Could not open texture %s\n", szFilename);
  }
  
  return unTextureName;
}

void UnloadTexture (GLuint uTexture) {
	if (uTexture > 0) {
		glDeleteTextures (1, & uTexture);
	}
}

