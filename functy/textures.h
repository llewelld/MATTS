#ifndef TEXTURES_H
#define TEXTURES_H

typedef struct _TexPersist TexPersist;

//typedef enum _TEXNAME {
//  TEXNAME_INVALID = -1,
//  
//  TEXNAME_GRID,
//  
//  TEXNAME_NUM
//} TEXNAME;

#define TEXTURE_NONE (0)


TexPersist * NewTexPersist (int nMaxTextures);
void DeleteTexPersist (TexPersist * psTexData);
void LoadTextures (TexPersist * psTexData);
//GLuint GetTexture (TEXNAME eTexture, GLfloat afTexCoords[], TexPersist * psTexData);
//GLuint GetTexturePortion (TEXNAME eTexture, GLfloat afPortion[], GLfloat afTexCoords[], TexPersist * psTexData);
GLuint LoadTextureRaw (char const * const szFilename, int const nWidth, int const nHeight, bool boAlpha);
//inline void GetTextureCoord (TEXNAME eTexture, GLfloat fXIn, GLfloat fYIn, GLfloat * pfXOut, GLfloat * pfYOut, TexPersist * psTexData);
void UnloadTexture (GLuint uTexture);

#endif /* TEXTURES_H */

