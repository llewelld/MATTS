/*$T MATTS/XMLCTag.cpp GC 1.140 07/01/09 21:12:11 */
/*
 * ;
 * Name: XMLCTag.cpp ;
 * Last Modified: 15/07/08 ;
 * ;
 * Purpose: Compose XML tags for cXMLCompile ;
 */
#include "XMLCTag.h"
#include "functy/GString.h"

/**
 =======================================================================================================================
 *  XMLCNumarray
 =======================================================================================================================
 */
XMLCNumarray::XMLCNumarray () {
	uElements = 0;
	uAllocated = 0;
	pxElement = (comptype*) calloc (1, sizeof (comptype));
	if (pxElement) {
		uElements = 1;
		uAllocated = 1;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
XMLCNumarray::XMLCNumarray (comptype xInit) {
	uElements = 0;
	uAllocated = 0;
	pxElement = (comptype*) malloc (sizeof (comptype));
	if (pxElement) {
		pxElement[0] = xInit;
		uElements = 1;
		uAllocated = 1;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
XMLCNumarray::XMLCNumarray (string szInit) {
	unsigned int uValues;
	int nPosStart;
	int nPosEnd;
	unsigned int uNewSize;
	comptype xValue;
	int nRead;

	uElements = 0;
	uAllocated = 0;

	/* Find out how many values there are */
	uValues = 0;
	nPosStart = 0;
	while (nPosStart >= 0) {
		uValues++;
		nPosStart = (int) szInit.find_first_of (" ", nPosStart + 1);
	}

	/* Allocate memory for the values */
	if (uValues <= 1) {
		uNewSize = 1;
	} else {
		uNewSize = ((uValues / XMLCNUMARRAY_ALLOCATE) + 1) * XMLCNUMARRAY_ALLOCATE;
	}

	pxElement = (comptype*) calloc (uNewSize, sizeof (comptype));
	if (pxElement) {
		uAllocated = uNewSize;
		uElements = uValues;

		/* Store the values in the array */
		nPosStart = 0;
		uValues = 0;
		while (uValues < uElements) {
			nPosEnd = (int) szInit.find_first_of (" ", nPosStart + 1);

			if (nPosEnd < 0) {
				nRead = sscanf (szInit.substr (nPosStart).c_str (), compconv, & xValue);
				if (nRead > 0) {
					pxElement[uValues] = xValue;
				}
			} else {
				nRead = sscanf (szInit.substr (nPosStart, nPosEnd - nPosStart).c_str (), compconv, & xValue);
				if (nRead > 0) {
					pxElement[uValues] = xValue;
				}
			}

			nPosStart = nPosEnd + 1;
			uValues++;
		}
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
XMLCNumarray::~XMLCNumarray () {
	free (pxElement);
	pxElement = NULL;
	uAllocated = 0;
	uElements = 0;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
comptype XMLCNumarray::GetElement (unsigned int uPos) {
	comptype xElement;
	xElement = 0;
	if (uPos < uElements) {
		xElement = pxElement[uPos];
	} else {
		xElement = 0;

		/*
		 * if (uElements > 0) { ;
		 * nElement = pnElement[0];
		 * }
		 */
	}

	return xElement;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void XMLCNumarray::SetElement (comptype xElement, unsigned int uPos) {
	unsigned int uNewSize;
	comptype*  pxNewElement;

	if (uPos < uAllocated) {

		/* Within bounds, so we just assign the value */
		pxElement[uPos] = xElement;
		if (uPos >= uElements) {
			uElements = uPos + 1;
		}
	} else {

		/* These elements don't yet exist, so we need to allocate space */
		uNewSize = ((uPos / XMLCNUMARRAY_ALLOCATE) + 1) * XMLCNUMARRAY_ALLOCATE;
		pxNewElement = (comptype*) calloc (uNewSize, sizeof (comptype));
		if (pxNewElement) {

			/* Copy the existing numbers across */
			memcpy (pxNewElement, pxElement, uElements * sizeof (comptype));
			free (pxElement);
			uAllocated = uNewSize;
			pxElement = pxNewElement;

			/* Store the new value */
			pxElement[uPos] = xElement;
			uElements = uPos + 1;
		}
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
unsigned int XMLCNumarray::GetNumElements () {
	return uElements;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void XMLCNumarray::CopyInto (XMLCNumarray* psFrom) {
	unsigned int uElementsFrom;

	if (pxElement) {
		free (pxElement);
	}

	uElementsFrom = psFrom->GetNumElements ();
	pxElement = (comptype*) malloc (uElementsFrom * sizeof (comptype));
	if (pxElement) {
		uAllocated = uElementsFrom;

		/* At the end of the loop, we have (uElements == uElementsFrom) as required */
		for (uElements = 0; uElements < uElementsFrom; uElements++) {
			pxElement[uElements] = psFrom->GetElement (uElements);
		}
	} else {
		uAllocated = 0;
		uElements = 0;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void XMLCNumarray::ClearAll () {
	if (pxElement) {
		free (pxElement);
	}

	uElements = 0;
	uAllocated = 0;
	pxElement = (comptype*) calloc (1, sizeof (comptype));
	if (pxElement) {
		uElements = 1;
		uAllocated = 1;
	}
}

/*
 =======================================================================================================================
 *  XMLCNumarray save routine
 =======================================================================================================================
 */
void XMLCNumarray::Save (FILE * hFile) {
	unsigned int uPos;

	for (uPos = 0; uPos < uElements; uPos++) {
		fprintf (hFile, compconv, pxElement[uPos]);
		if (uPos + 1 < uElements) {
			fprintf (hFile, " ");
		}
	}
}

/**
 =======================================================================================================================
 *  XMLCTokenize
 =======================================================================================================================
 */
XMLCTokenize::XMLCTokenize (string szTokenize) {
	int nPosStart;
	int nPosEnd;
	unsigned int uToken;

	/* Find out how many tokens there are */
	uTokens = 0;
	nPosStart = 0;
	if (szTokenize.length () > 0) {
		while (nPosStart >= 0) {
			uTokens++;
			nPosStart = (int) szTokenize.find_first_of (" ", nPosStart + 1);
		}

		/* Allocate memory for the token strings */
		aszToken = (string**) calloc (uTokens, sizeof (string *));

		/* Store the tokens strings in the array */
		nPosStart = 0;
		uToken = 0;
		while (uToken < uTokens) {
			nPosEnd = (int) szTokenize.find_first_of (" ", nPosStart + 1);
			aszToken[uToken] = new string ();

			if (nPosEnd < 0) {
				*aszToken[uToken] = szTokenize.substr (nPosStart);
			} else {
				*aszToken[uToken] = szTokenize.substr (nPosStart, nPosEnd - nPosStart);
			}

			nPosStart = nPosEnd + 1;
			uToken++;
		}
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
XMLCTokenize::~XMLCTokenize () {
	unsigned int uToken;

	/* Delete all of the token strings */
	for (uToken = 0; uToken < uTokens; uToken++) {
		delete aszToken[uToken];
	}

	free (aszToken);
	aszToken = NULL;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
unsigned int XMLCTokenize::GetTokensNum () {
	return uTokens;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
string XMLCTokenize::GetToken (unsigned int uTokenNum) {
	string szReturn;

	szReturn = "";

	if (uTokenNum < uTokens) {
		szReturn = *aszToken[uTokenNum];
	}

	return szReturn;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool XMLCTokenize::FindAny (char const *const szToken) {
	unsigned int uToken;
	bool boReturn;

	boReturn = false;

	/* Delete all of the token strings */
	for (uToken = 0; (uToken < uTokens) && !boReturn; uToken++) {
		boReturn = (aszToken[uToken]->compare (szToken) == 0);
	}

	return boReturn;
}

/**
 =======================================================================================================================
 *  XMLCTag
 =======================================================================================================================
 */
XMLCTag::XMLCTag () {
	eTag = XMLCTAG_INVALID;
	szID = "";
	psPrev = NULL;
	psNext = NULL;
	psParent = NULL;
	psChild = NULL;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
XMLCTag::~XMLCTag () {
	if (psPrev) {
		psPrev->psNext = NULL;
	}

	if (psNext) {
		delete psNext;
	}

	if (psParent && !psPrev) {
		psParent->psChild = NULL;
	}

	if (psChild) {
		delete psChild;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
XMLCTAG XMLCTag::GetTag () {
	return eTag;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
string XMLCTag::GetID () {
	return szID;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void XMLCTag::SetParent (XMLCTag* psNewParent) {
	XMLCTag*  psTag;
	psParent = psNewParent;
	if (psParent) {
		psTag = psParent->psChild;
		if (psTag) {
			while (psTag->psNext) {
				psTag = psTag->psNext;
			}

			psPrev = psTag;
			psTag->psNext = this;
		} else {
			psPrev = NULL;
			psParent->psChild = this;
		}
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void XMLCTag::AddChild (XMLCTag* psNewChild) {
	XMLCTag*  psTag;
	if (psNewChild) {
		psNewChild->psParent = this;

		if (psChild) {
			psTag = psChild;
			while (psTag->psNext) {
				psTag = psTag->psNext;
			}

			psTag->psNext = psNewChild;
			psNewChild->psPrev = psTag;
		} else {
			psChild = psNewChild;
			psNewChild->psPrev = NULL;
		}
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void XMLCTag::Reset () {
	if (psChild) {
		psChild->Reset ();
	}

	if (psNext) {
		psNext->Reset ();
	}
}

/*
 =======================================================================================================================
 *  XMLCTag save routine
 =======================================================================================================================
 */
void XMLCTag::Save (FILE * hFile, int nIndent) {
	XMLCTag * psCurrent;

	if (psChild) {
		fprintf (hFile, "%*s<%s", nIndent * 2, " ", szXMLCTag[eTag]);
		if (!szID.empty ()) {
			fprintf (hFile, " id=\"%s\"", szID.c_str ());
		}
		fprintf (hFile, ">\n");

		nIndent++;
		// And recurse
		psChild->Save (hFile, nIndent);
		nIndent--;
		fprintf (hFile, "%*s</%s>\n", nIndent * 2, " ", szXMLCTag[eTag]);
	}
	else {
		fprintf (hFile, "%*s<%s", nIndent * 2, " ", szXMLCTag[eTag]);
		if (!szID.empty ()) {
			fprintf (hFile, " id=\"%s\"", szID.c_str ());
		}
		fprintf (hFile, "/>\n");
	}

	if (psPrev == NULL) {
		// Cycle through all of the following tags
		psCurrent = psNext;
		while (psCurrent) {
			psCurrent->Save (hFile, nIndent);
			psCurrent = psCurrent->psNext;
		}
	}
}

/**
 =======================================================================================================================
 *  XMLCCompose
 =======================================================================================================================
 */
XMLCCompose::XMLCCompose (string szID, string szInit, XMLCTYPE eType) {
	eTag = XMLCTAG_COMPOSE;
	this->szID = szID;
	psInit = new XMLCNumarray (szInit);
	psInitReset = new XMLCNumarray ();
	psInitReset->CopyInto (psInit);
	this->eType = eType;

	nSandboxes = 0;
	apsSandbox = NULL;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
XMLCCompose::~XMLCCompose () {
	if (psInit) {
		delete psInit;
		psInit = NULL;
	}

	if (psInitReset) {
		delete psInitReset;
		psInitReset = NULL;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void XMLCCompose::Reset () {
	if ((psInit) && (psInitReset)) {
		psInit->CopyInto (psInitReset);
	}

	if (psChild) {
		psChild->Reset ();
	}

	if (psNext) {
		psNext->Reset ();
	}
}

/*
 =======================================================================================================================
 *  XMLCCompose save routine
 =======================================================================================================================
 */
void XMLCCompose::Save (FILE * hFile, int nIndent) {
	XMLCTag * psCurrent;

	if (psChild) {
		fprintf (hFile, "%*s<%s", nIndent * 2, " ", szXMLCTag[eTag]);
		if (!szID.empty ()) {
			fprintf (hFile, " id=\"%s\"", szID.c_str ());
		}
		switch (eType) {
		default:
			fprintf (hFile, " type=\"standard\"");
			break;
		case XMLCTYPE_EXTENDED:
			fprintf (hFile, " type=\"extended\"");
			break;
		}
		fprintf (hFile, " init=\"");
		psInitReset->Save (hFile);
		fprintf (hFile, "\"");
		fprintf (hFile, ">\n");

		nIndent++;
		// And recurse
		psChild->Save (hFile, nIndent);
		nIndent--;
		fprintf (hFile, "%*s</%s>\n", nIndent * 2, " ", szXMLCTag[eTag]);
	}
	else {
		fprintf (hFile, "%*s<%s", nIndent * 2, " ", szXMLCTag[eTag]);
		if (!szID.empty ()) {
			fprintf (hFile, " id=\"%s\"", szID.c_str ());
		}
		switch (eType) {
		default:
			fprintf (hFile, " type=\"standard\"");
			break;
		case XMLCTYPE_EXTENDED:
			fprintf (hFile, " type=\"extended\"");
			break;
		}
		fprintf (hFile, " init=\"");
		psInitReset->Save (hFile);
		fprintf (hFile, "\"");
		fprintf (hFile, "/>\n");
	}

	if (psPrev == NULL) {
		// Cycle through all of the following tags
		psCurrent = psNext;
		while (psCurrent) {
			psCurrent->Save (hFile, nIndent);
			psCurrent = psCurrent->psNext;
		}
	}
}

/**
 =======================================================================================================================
 *  XMLCSandbox
 =======================================================================================================================
 */
XMLCSandbox::XMLCSandbox (string szID, string szDescription, string szConfig) {
	eTag = XMLCTAG_SANDBOX;
	this->szID = szID;
	this->szDescription = szDescription;
	this->szConfig = szConfig;
	boTestResult = false;
}

/*
 =======================================================================================================================
 *  XMLCSandbox save routine
 =======================================================================================================================
 */
void XMLCSandbox::Save (FILE * hFile, int nIndent) {
	XMLCTag * psCurrent;
	char * szEscaped;

	fprintf (hFile, "%*s<%s", nIndent * 2, " ", szXMLCTag[eTag]);
	if (!szID.empty ()) {
		fprintf (hFile, " id=\"%s\"", szID.c_str ());
	}
	if (!szConfig.empty ()) {
		fprintf (hFile, " config=\"%s\"", szConfig.c_str ());
	}
	fprintf (hFile, ">");

	szEscaped = g_markup_escape (szDescription.c_str ());
	fprintf (hFile, szEscaped);
	g_free (szEscaped);

	if (psChild) {
		nIndent++;
		// And recurse
		psChild->Save (hFile, nIndent);
		nIndent--;
	}
	fprintf (hFile, "</%s>\n", szXMLCTag[eTag]);

	if (psPrev == NULL) {
		// Cycle through all of the following tags
		psCurrent = psNext;
		while (psCurrent) {
			psCurrent->Save (hFile, nIndent);
			psCurrent = psCurrent->psNext;
		}
	}
}

/**
 =======================================================================================================================
 *  XMLCProperty
 =======================================================================================================================
 */
XMLCProperty::XMLCProperty (string szID, string szDescription) {
	eTag = XMLCTAG_PROPERTY;
	this->szID = szID;
	this->szDescription = szDescription;
}

/*
 =======================================================================================================================
 *  XMLCProperty save routine
 =======================================================================================================================
 */
void XMLCProperty::Save (FILE * hFile, int nIndent) {
	XMLCTag * psCurrent;
	char * szEscaped;

	fprintf (hFile, "%*s<%s", nIndent * 2, " ", szXMLCTag[eTag]);
	if (!szID.empty ()) {
		fprintf (hFile, " id=\"%s\"", szID.c_str ());
	}
	fprintf (hFile, ">");

	szEscaped = g_markup_escape (szDescription.c_str ());
	fprintf (hFile, szEscaped);
	g_free (szEscaped);

	if (psChild) {
		nIndent++;
		// And recurse
		psChild->Save (hFile, nIndent);
		nIndent--;
	}
	fprintf (hFile, "</%s>\n", szXMLCTag[eTag]);

	if (psPrev == NULL) {
		// Cycle through all of the following tags
		psCurrent = psNext;
		while (psCurrent) {
			psCurrent->Save (hFile, nIndent);
			psCurrent = psCurrent->psNext;
		}
	}
}

/**
 =======================================================================================================================
 *  XMLCConfiguration
 =======================================================================================================================
 */
XMLCConfiguration::XMLCConfiguration (string szID) {
	eTag = XMLCTAG_CONFIGURATION;
	this->szID = szID;
}

/**
 =======================================================================================================================
 *  XMLCComponent
 =======================================================================================================================
 */
XMLCComponent::XMLCComponent (string szID, string szInit, string szCond, string szAction) {
	eTag = XMLCTAG_COMPONENT;
	this->szID = szID;
	psInit = new XMLCNumarray (szInit);
	psInitReset = new XMLCNumarray ();
	psInitReset->CopyInto (psInit);
	this->szCond = szCond;
	this->szAction = szAction;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
XMLCComponent::XMLCComponent () {
	eTag = XMLCTAG_COMPONENT;
	szID = "";
	psInit = new XMLCNumarray ();
	psInitReset = new XMLCNumarray ();
	szCond = "";
	szAction = "";
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
XMLCComponent::~XMLCComponent () {
	if (psInit) {
		delete psInit;
		psInit = NULL;
	}

	if (psInitReset) {
		delete psInitReset;
		psInitReset = NULL;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void XMLCComponent::Reset () {
	if ((psInit) && (psInitReset)) {
		psInit->CopyInto (psInitReset);
	}

	if (psChild) {
		psChild->Reset ();
	}

	if (psNext) {
		psNext->Reset ();
	}
}

/*
 =======================================================================================================================
 *  XMLCComponent save routine
 =======================================================================================================================
 */
void XMLCComponent::Save (FILE * hFile, int nIndent) {
	XMLCTag * psCurrent;
	char * szEscaped;

	if (psChild) {
		fprintf (hFile, "%*s<%s", nIndent * 2, " ", szXMLCTag[eTag]);
		if (!szID.empty ()) {
			fprintf (hFile, " id=\"%s\"", szID.c_str ());
		}
		fprintf (hFile, " init=\"");
		psInitReset->Save (hFile);
		fprintf (hFile, "\"");
		if (!szCond.empty ()) {
			szEscaped = g_markup_escape (szCond.c_str ());
			fprintf (hFile, " cond=\"%s\"", szEscaped);
			g_free (szEscaped);
		}
		if (!szAction.empty ()) {
			szEscaped = g_markup_escape (szAction.c_str ());
			fprintf (hFile, " action=\"%s\"", szEscaped);
			g_free (szEscaped);
		}
		fprintf (hFile, ">\n");

		nIndent++;
		// And recurse
		psChild->Save (hFile, nIndent);
		nIndent--;
		fprintf (hFile, "%*s</%s>\n", nIndent * 2, " ", szXMLCTag[eTag]);
	}
	else {
		fprintf (hFile, "%*s<%s", nIndent * 2, " ", szXMLCTag[eTag]);
		if (!szID.empty ()) {
			fprintf (hFile, " id=\"%s\"", szID.c_str ());
		}
		fprintf (hFile, " init=\"");
		psInitReset->Save (hFile);
		fprintf (hFile, "\"");
		if (!szCond.empty ()) {
			szEscaped = g_markup_escape (szCond.c_str ());
			fprintf (hFile, " cond=\"%s\"", szEscaped);
			g_free (szEscaped);
		}
		if (!szAction.empty ()) {
			szEscaped = g_markup_escape (szAction.c_str ());
			fprintf (hFile, " action=\"%s\"", szEscaped);
			g_free (szEscaped);
		}
		fprintf (hFile, "/>\n");
	}

	if (psPrev == NULL) {
		// Cycle through all of the following tags
		psCurrent = psNext;
		while (psCurrent) {
			psCurrent->Save (hFile, nIndent);
			psCurrent = psCurrent->psNext;
		}
	}
}

/**
 =======================================================================================================================
 *  XMLCInput
 =======================================================================================================================
 */
XMLCInput::XMLCInput (string szID, string szInit, string szCond, string szAction, string szConfig, string szFormat,
					  XMLCCYCLE eCycle, XMLCFOLLOW eFollow) {
	eTag = XMLCTAG_INPUT;
	this->szID = szID;
	psInit = new XMLCNumarray (szInit);
	psInitReset = new XMLCNumarray ();
	psInitReset->CopyInto (psInit);
	this->szCond = szCond;
	this->szAction = szAction;
	this->szConfig = szConfig;
	this->szFormat = szFormat;
	this->psFormat = new XMLCTokenize (szFormat);
	this->eCycle = eCycle;
	this->eFollow = eFollow;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
XMLCInput::~XMLCInput () {
	if (psInit) {
		delete psInit;
		psInit = NULL;
	}

	if (psInitReset) {
		delete psInitReset;
		psInitReset = NULL;
	}

	if (psFormat) {
		delete psFormat;
		psFormat = NULL;
	}
}

/*
 =======================================================================================================================
 *  XMLCInput save routine
 =======================================================================================================================
 */
void XMLCInput::Save (FILE * hFile, int nIndent) {
	XMLCTag * psCurrent;
	char * szEscaped;

	if (psChild) {
		fprintf (hFile, "%*s<%s", nIndent * 2, " ", szXMLCTag[eTag]);
		if (!szID.empty ()) {
			fprintf (hFile, " id=\"%s\"", szID.c_str ());
		}
		fprintf (hFile, " init=\"");
		psInitReset->Save (hFile);
		fprintf (hFile, "\"");
		if (!szCond.empty ()) {
			szEscaped = g_markup_escape (szCond.c_str ());
			fprintf (hFile, " cond=\"%s\"", szEscaped);
			g_free (szEscaped);
		}
		if (!szAction.empty ()) {
			szEscaped = g_markup_escape (szAction.c_str ());
			fprintf (hFile, " action=\"%s\"", szEscaped);
			g_free (szEscaped);
		}
		if (!szConfig.empty ()) {
			szEscaped = g_markup_escape (szConfig.c_str ());
			fprintf (hFile, " config=\"%s\"", szEscaped);
			g_free (szEscaped);
		}
		if (!szFormat.empty ()) {
			szEscaped = g_markup_escape (szFormat.c_str ());
			fprintf (hFile, " format=\"%s\"", szEscaped);
			g_free (szEscaped);
		}
		switch (eCycle) {
		case XMLCCYCLE_DISALLOW:
			fprintf (hFile, " cycle=\"disallow\"");
			break;
		case XMLCCYCLE_OPTIONAL:
			fprintf (hFile, " cycle=\"optional\"");
			break;
		case XMLCCYCLE_REQUIRED:
			fprintf (hFile, " cycle=\"required\"");
			break;
		}
		switch (eFollow) {
		case XMLCFOLLOW_YES:
			fprintf (hFile, " follow=\"yes\"");
			break;
		case XMLCFOLLOW_NO:
			fprintf (hFile, " follow=\"no\"");
			break;
		case XMLCFOLLOW_FRESH:
			fprintf (hFile, " follow=\"fresh\"");
			break;
		}
		fprintf (hFile, ">\n");

		nIndent++;
		// And recurse
		psChild->Save (hFile, nIndent);
		nIndent--;
		fprintf (hFile, "%*s</%s>\n", nIndent * 2, " ", szXMLCTag[eTag]);
	}
	else {
		fprintf (hFile, "%*s<%s", nIndent * 2, " ", szXMLCTag[eTag]);
		if (!szID.empty ()) {
			fprintf (hFile, " id=\"%s\"", szID.c_str ());
		}
		fprintf (hFile, " init=\"");
		psInitReset->Save (hFile);
		fprintf (hFile, "\"");
		if (!szCond.empty ()) {
			szEscaped = g_markup_escape (szCond.c_str ());
			fprintf (hFile, " cond=\"%s\"", szEscaped);
			g_free (szEscaped);
		}
		if (!szAction.empty ()) {
			szEscaped = g_markup_escape (szAction.c_str ());
			fprintf (hFile, " action=\"%s\"", szEscaped);
			g_free (szEscaped);
		}
		if (!szConfig.empty ()) {
			szEscaped = g_markup_escape (szConfig.c_str ());
			fprintf (hFile, " config=\"%s\"", szEscaped);
			g_free (szEscaped);
		}
		if (!szFormat.empty ()) {
			szEscaped = g_markup_escape (szFormat.c_str ());
			fprintf (hFile, " format=\"%s\"", szEscaped);
			g_free (szEscaped);
		}
		switch (eCycle) {
		case XMLCCYCLE_DISALLOW:
			fprintf (hFile, " cycle=\"disallow\"");
			break;
		case XMLCCYCLE_OPTIONAL:
			fprintf (hFile, " cycle=\"optional\"");
			break;
		case XMLCCYCLE_REQUIRED:
			fprintf (hFile, " cycle=\"required\"");
			break;
		}
		switch (eFollow) {
		case XMLCFOLLOW_YES:
			fprintf (hFile, " follow=\"yes\"");
			break;
		case XMLCFOLLOW_NO:
			fprintf (hFile, " follow=\"no\"");
			break;
		case XMLCFOLLOW_FRESH:
			fprintf (hFile, " follow=\"fresh\"");
			break;
		}
		fprintf (hFile, "/>\n");
	}

	if (psPrev == NULL) {
		// Cycle through all of the following tags
		psCurrent = psNext;
		while (psCurrent) {
			psCurrent->Save (hFile, nIndent);
			psCurrent = psCurrent->psNext;
		}
	}
}

/**
 =======================================================================================================================
 *  XMLCOuput
 =======================================================================================================================
 */
XMLCOuput::XMLCOuput (string szID, string szInit, string szCond, string szAction, string szConfig, string szFormat,
					  XMLCCYCLE eCycle, XMLCFOLLOW eFollow) {
	eTag = XMLCTAG_OUTPUT;
	this->szID = szID;
	psInit = new XMLCNumarray (szInit);
	psInitReset = new XMLCNumarray ();
	psInitReset->CopyInto (psInit);
	this->szCond = szCond;
	this->szAction = szAction;
	this->szConfig = szConfig;
	this->szFormat = szFormat;
	this->psFormat = new XMLCTokenize (szFormat);
	this->eCycle = eCycle;
	this->eFollow = eFollow;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
XMLCOuput::~XMLCOuput () {
	if (psInit) {
		delete psInit;
		psInit = NULL;
	}

	if (psInitReset) {
		delete psInitReset;
		psInitReset = NULL;
	}

	if (psFormat) {
		delete psFormat;
		psFormat = NULL;
	}
}

/*
 =======================================================================================================================
 *  XMLCOuput save routine
 =======================================================================================================================
 */
void XMLCOuput::Save (FILE * hFile, int nIndent) {
	XMLCTag * psCurrent;
	char * szEscaped;

	if (psChild) {
		fprintf (hFile, "%*s<%s", nIndent * 2, " ", szXMLCTag[eTag]);
		if (!szID.empty ()) {
			fprintf (hFile, " id=\"%s\"", szID.c_str ());
		}
		fprintf (hFile, " init=\"");
		psInitReset->Save (hFile);
		fprintf (hFile, "\"");
		if (!szCond.empty ()) {
			szEscaped = g_markup_escape (szCond.c_str ());
			fprintf (hFile, " cond=\"%s\"", szEscaped);
			g_free (szEscaped);
		}
		if (!szAction.empty ()) {
			szEscaped = g_markup_escape (szAction.c_str ());
			fprintf (hFile, " action=\"%s\"", szEscaped);
			g_free (szEscaped);
		}
		if (!szConfig.empty ()) {
			szEscaped = g_markup_escape (szConfig.c_str ());
			fprintf (hFile, " config=\"%s\"", szEscaped);
			g_free (szEscaped);
		}
		if (!szFormat.empty ()) {
			szEscaped = g_markup_escape (szFormat.c_str ());
			fprintf (hFile, " format=\"%s\"", szEscaped);
			g_free (szEscaped);
		}
		switch (eCycle) {
		case XMLCCYCLE_DISALLOW:
			fprintf (hFile, " cycle=\"disallow\"");
			break;
		case XMLCCYCLE_OPTIONAL:
			fprintf (hFile, " cycle=\"optional\"");
			break;
		case XMLCCYCLE_REQUIRED:
			fprintf (hFile, " cycle=\"required\"");
			break;
		}
		switch (eFollow) {
		case XMLCFOLLOW_YES:
			fprintf (hFile, " follow=\"yes\"");
			break;
		case XMLCFOLLOW_NO:
			fprintf (hFile, " follow=\"no\"");
			break;
		case XMLCFOLLOW_FRESH:
			fprintf (hFile, " follow=\"fresh\"");
			break;
		}
		fprintf (hFile, ">\n");

		nIndent++;
		// And recurse
		psChild->Save (hFile, nIndent);
		nIndent--;
		fprintf (hFile, "%*s</%s>\n", nIndent * 2, " ", szXMLCTag[eTag]);
	}
	else {
		fprintf (hFile, "%*s<%s", nIndent * 2, " ", szXMLCTag[eTag]);
		if (!szID.empty ()) {
			fprintf (hFile, " id=\"%s\"", szID.c_str ());
		}
		fprintf (hFile, " init=\"");
		psInitReset->Save (hFile);
		fprintf (hFile, "\"");
		if (!szCond.empty ()) {
			szEscaped = g_markup_escape (szCond.c_str ());
			fprintf (hFile, " cond=\"%s\"", szEscaped);
			g_free (szEscaped);
		}
		if (!szAction.empty ()) {
			szEscaped = g_markup_escape (szAction.c_str ());
			fprintf (hFile, " action=\"%s\"", szEscaped);
			g_free (szEscaped);
		}
		if (!szConfig.empty ()) {
			szEscaped = g_markup_escape (szConfig.c_str ());
			fprintf (hFile, " config=\"%s\"", szEscaped);
			g_free (szEscaped);
		}
		if (!szFormat.empty ()) {
			szEscaped = g_markup_escape (szFormat.c_str ());
			fprintf (hFile, " format=\"%s\"", szEscaped);
			g_free (szEscaped);
		}
		switch (eCycle) {
		case XMLCCYCLE_DISALLOW:
			fprintf (hFile, " cycle=\"disallow\"");
			break;
		case XMLCCYCLE_OPTIONAL:
			fprintf (hFile, " cycle=\"optional\"");
			break;
		case XMLCCYCLE_REQUIRED:
			fprintf (hFile, " cycle=\"required\"");
			break;
		}
		switch (eFollow) {
		case XMLCFOLLOW_YES:
			fprintf (hFile, " follow=\"yes\"");
			break;
		case XMLCFOLLOW_NO:
			fprintf (hFile, " follow=\"no\"");
			break;
		case XMLCFOLLOW_FRESH:
			fprintf (hFile, " follow=\"fresh\"");
			break;
		}
		fprintf (hFile, "/>\n");
	}

	if (psPrev == NULL) {
		// Cycle through all of the following tags
		psCurrent = psNext;
		while (psCurrent) {
			psCurrent->Save (hFile, nIndent);
			psCurrent = psCurrent->psNext;
		}
	}
}

/**
 =======================================================================================================================
 *  XMLCProcess
 =======================================================================================================================
 */
XMLCProcess::XMLCProcess (string szID, string szInit, string szCond, string szAction, string szConfig) {
	eTag = XMLCTAG_PROCESS;
	this->szID = szID;
	psInit = new XMLCNumarray (szInit);
	psInitReset = new XMLCNumarray ();
	psInitReset->CopyInto (psInit);
	this->szCond = szCond;
	this->szAction = szAction;
	this->szConfig = szConfig;
}

/*
 =======================================================================================================================
 *  XMLCProcess save routine
 =======================================================================================================================
 */
void XMLCProcess::Save (FILE * hFile, int nIndent) {
	XMLCTag * psCurrent;
	char * szEscaped;

	if (psChild) {
		fprintf (hFile, "%*s<%s", nIndent * 2, " ", szXMLCTag[eTag]);
		if (!szID.empty ()) {
			fprintf (hFile, " id=\"%s\"", szID.c_str ());
		}
		fprintf (hFile, " init=\"");
		psInitReset->Save (hFile);
		fprintf (hFile, "\"");
		if (!szCond.empty ()) {
			szEscaped = g_markup_escape (szCond.c_str ());
			fprintf (hFile, " cond=\"%s\"", szEscaped);
			g_free (szEscaped);
		}
		if (!szAction.empty ()) {
			szEscaped = g_markup_escape (szAction.c_str ());
			fprintf (hFile, " action=\"%s\"", szEscaped);
			g_free (szEscaped);
		}
		if (!szConfig.empty ()) {
			szEscaped = g_markup_escape (szConfig.c_str ());
			fprintf (hFile, " config=\"%s\"", szEscaped);
			g_free (szEscaped);
		}
		fprintf (hFile, ">\n");

		nIndent++;
		// And recurse
		psChild->Save (hFile, nIndent);
		nIndent--;
		fprintf (hFile, "%*s</%s>\n", nIndent * 2, " ", szXMLCTag[eTag]);
	}
	else {
		fprintf (hFile, "%*s<%s", nIndent * 2, " ", szXMLCTag[eTag]);
		if (!szID.empty ()) {
			fprintf (hFile, " id=\"%s\"", szID.c_str ());
		}
		fprintf (hFile, " init=\"");
		psInitReset->Save (hFile);
		fprintf (hFile, "\"");
		if (!szCond.empty ()) {
			szEscaped = g_markup_escape (szCond.c_str ());
			fprintf (hFile, " cond=\"%s\"", szEscaped);
			g_free (szEscaped);
		}
		if (!szAction.empty ()) {
			szEscaped = g_markup_escape (szAction.c_str ());
			fprintf (hFile, " action=\"%s\"", szEscaped);
			g_free (szEscaped);
		}
		if (!szConfig.empty ()) {
			szEscaped = g_markup_escape (szConfig.c_str ());
			fprintf (hFile, " config=\"%s\"", szEscaped);
			g_free (szEscaped);
		}
		fprintf (hFile, "/>\n");
	}

	if (psPrev == NULL) {
		// Cycle through all of the following tags
		psCurrent = psNext;
		while (psCurrent) {
			psCurrent->Save (hFile, nIndent);
			psCurrent = psCurrent->psNext;
		}
	}
}
