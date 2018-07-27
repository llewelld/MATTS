/*$T MATTS/functy/GString.cpp GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: GString.cpp ;
 * Last Modified: 20/12/05 ;
 * ;
 * Purpose: Replacement for GString functions from glib ;
 */

#include "GString.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define GSTRING_ALLOC_CHUNKSIZE  (64)

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
GString * g_string_new (char const * init) {
	GString * psString;
	unsigned long uLength;
	unsigned long uAllocated;

	psString = (GString *)malloc (sizeof (GString));

	uLength = (unsigned)strlen (init);
	uAllocated = ((unsigned int)((uLength) / GSTRING_ALLOC_CHUNKSIZE) + 1) * GSTRING_ALLOC_CHUNKSIZE;

	psString->str = (char *)calloc (1, uAllocated);
	memcpy (psString->str, init, uLength + 1);
	psString->length = uLength;
	psString->allocated_len = uAllocated;

	return psString;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
char * g_string_free (GString * string, bool free_segment) {
	char * szReturn = NULL;
	if (free_segment) {
		free (string->str);
	}
	else {
		szReturn = string->str;
	}
	free (string);

	return szReturn;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
GString * g_string_assign (GString * string, const char * rval) {
	unsigned long uLength;
	unsigned long uAllocated;

	uLength = (unsigned)strlen (rval);
	uAllocated = ((unsigned int)((uLength) / GSTRING_ALLOC_CHUNKSIZE) + 1) * GSTRING_ALLOC_CHUNKSIZE;

	if (uAllocated != string->allocated_len) {
		string->str = (char *)realloc (string->str, uAllocated);
		string->allocated_len = uAllocated;
	}

	memcpy (string->str, rval, uLength + 1);
	string->length = uLength;

	return string;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
char * g_markup_escape (char const * szText) {
	int nPos;
	int nEscapePos;
	int nLength;
	char * szResult;

	nLength = (int)strlen (szText);
	nEscapePos = 0;
	// Count the number of additional characters
	for (nPos = 0; nPos < nLength; nPos++) {
		switch (szText[nPos]) {
		case '<':
			nEscapePos += 4;
			break;
		case '>':
			nEscapePos += 4;
			break;
		case '&':
			nEscapePos += 5;
			break;
		default:
			nEscapePos++;
			break;
		}
	}

	szResult = (char *)malloc (nEscapePos + 1);
	nEscapePos = 0;
	for (nPos = 0; nPos < nLength; nPos++) {
		switch (szText[nPos]) {
		case '<':
			strcpy (szResult + nEscapePos, "&lt;");
			nEscapePos += 4;
			break;
		case '>':
			strcpy (szResult + nEscapePos, "&gt;");
			nEscapePos += 4;
			break;
		case '&':
			strcpy (szResult + nEscapePos, "&amp;");
			nEscapePos += 5;
			break;
		default:
			szResult[nEscapePos] = szText[nPos];
			nEscapePos++;
			break;
		}
	}
	szResult[nEscapePos] = 0;

	return szResult;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void * g_malloc (int const size) {
	return malloc (size);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int g_strcmp0 (char const * szText1, char const * szText2) {
	return strcmp (szText1, szText2);
}
