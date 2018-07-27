/*$T MATTS/functy/GString.h GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: GString.h ;
 * Last Modified: 20/01/05 ;
 * ;
 * Purpose: Header file for GString ;
 */

#ifndef GSTRING_H
#define GSTRING_H

#include <stdio.h>
#include <stdlib.h>
#include <float.h>

/*
 =======================================================================================================================
 *  The following struct is to remove this linker warning
 *  "warning LNK4248: unresolved typeref token (01000014) for '_Variable'; image may not run"
 *  that apparently arises because the CLR requires the struct to be resolvable
 =======================================================================================================================
 */
struct _Variable {
};

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
typedef struct _GString {
	char * str;
	unsigned long length;
	unsigned long allocated_len;
} GString;

#define g_new0(STRUCT_TYPE, N_STRUCTS) (STRUCT_TYPE *)calloc (sizeof (STRUCT_TYPE), N_STRUCTS)
#define g_new(STRUCT_TYPE, N_STRUCTS) (STRUCT_TYPE *)malloc (sizeof (STRUCT_TYPE) * N_STRUCTS)
#define g_free free

GString * g_string_new (char const * init);
char * g_string_free (GString * string, bool free_segment);
GString * g_string_assign (GString * string, const char * rval);

char * g_markup_escape (char const * szText);

void * g_malloc (int const size);

int g_strcmp0 (char const * szText1, char const * szText2);

#endif /* GSTRING_H */
