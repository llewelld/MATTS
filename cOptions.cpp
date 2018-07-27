/*$T MATTS/cOptions.cpp GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cOptions.cpp ;
 * Last Modified: 10/11/04 ;
 * ;
 * Purpose: Implementation file for options class ;
 */
#include "cOptions.H"
#include "cVMTypes.h"
#include <stdio.h>
#include <string>

bool cOptions:: flag = false;
cOptions* cOptions:: instance = NULL;

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */

cOptions::cOptions () {
	listenHandle = NULL;
	listenControlHandle = NULL;
	listenPDAServerHandle = NULL;
	stack = 32000;
	machineLimit = 100;
	allowMinimise = false;
	allowAgents = false;
	autoCreate = true;
	allowControl = false;
	selectSpeed = 500;
	colourSpeed = 300;
	rotHalfLife = 0.1f;
	rotSelected = true;
	demoCycle = false;
	fSoundVolume = 1.0f;
	screenShotMode = false;
	dandelion = false;
	dandeServer = "localhost";
	viewEvents = true;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cOptions::~cOptions () {
	flag = false;
	listenHandle = NULL;
	listenControlHandle = NULL;
	listenPDAServerHandle = NULL;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cOptions* cOptions::create () {
	if (!flag) {
		flag = true;

		instance = new cOptions;
	}

	return instance;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cOptions::Save (FILE * hFile, int nIndent) {
	fprintf (hFile, "%*s<Options>\n", nIndent * 2, " ");
	nIndent++;

	/* Write machine limit */
	fprintf (hFile, "%*s<MachineLimit type=\"int\">%d</MachineLimit>\n", nIndent * 2, " ", machineLimit);

	/* Write stack size */
	fprintf (hFile, "%*s<StackSize type=\"long\">%ld</StackSize>\n", nIndent * 2, " ", stack);

	/* Write allow minimise */
	fprintf (hFile, "%*s<AllowMinimise type=\"bool\">%d</AllowMinimise>\n", nIndent * 2, " ", allowMinimise);

	/* write allow agents */
	fprintf (hFile, "%*s<AllowAgents type=\"bool\">%d</AllowAgents>\n", nIndent * 2, " ", allowAgents);

	/* Write allow auto create */
	fprintf (hFile, "%*s<AutoCreate type=\"bool\">%d</AutoCreate>\n", nIndent * 2, " ", autoCreate);

	/* Write allow control */
	fprintf (hFile, "%*s<AllowControl type=\"bool\">%d</AllowControl>\n", nIndent * 2, " ", allowControl);

	/* Write select speed */
	fprintf (hFile, "%*s<SelectSpeed type=\"int\">%d</SelectSpeed>\n", nIndent * 2, " ", selectSpeed);

	/* Write colour speed */
	fprintf (hFile, "%*s<ColourSpeed type=\"int\">%d</ColourSpeed>\n", nIndent * 2, " ", colourSpeed);

	/* Write rotate half-life */
	fprintf (hFile, "%*s<RotateHalfLife type=\"float\">%f</RotateHalfLife>\n", nIndent * 2, " ", rotHalfLife);

	/* Write rotate selected */
	fprintf (hFile, "%*s<RotateSelected type=\"bool\">%d</RotateSelected>\n", nIndent * 2, " ", rotSelected);

	/* Write sound volume */
	fprintf (hFile, "%*s<SoundVolume type=\"float\">%f</SoundVolume>\n", nIndent * 2, " ", fSoundVolume);

	/* Write screen shot mode */
	fprintf (hFile, "%*s<ScreenshotMode type=\"bool\">%d</ScreenshotMode>\n", nIndent * 2, " ", screenShotMode);

	/* Write dandelion use */
	fprintf (hFile, "%*s<Dandelion type=\"bool\">%d</Dandelion>\n", nIndent * 2, " ", dandelion);

	/* Write dandelion server address */
	fprintf (hFile, "%*s<DandelionServer type=\"string\">%s</DandelionServer>\n", nIndent * 2, " ", dandeServer.c_str ());

	/* Write record view events */
	fprintf (hFile, "%*s<RecordViewEvents type=\"bool\">%d</RecordViewEvents>\n", nIndent * 2, " ", viewEvents);

	nIndent--;
	fprintf (hFile, "%*s</Options>\n", nIndent * 2, " ");
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cOptions::LoadStart (char const * szChunkName) {
	eTag = OPTIONSTAG_INVALID;
	sOption = "";
	sLoadText = "";
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cOptions::LoadEnd (char const * szChunkName) {
	eTag = OPTIONSTAG_INVALID;
	sOption = "";
	sLoadText = "";
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cOptions::startElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname, const Attributes&  attrs) {
	char* szElementName;

	szElementName = XMLString::transcode (localname);
	sLoadText = "";

	switch (eTag) {
		case OPTIONSTAG_INVALID:
			if (_stricmp (szElementName, "Options") == 0) {
				eTag = OPTIONSTAG_OPTIONS;
			}
			break;
		case OPTIONSTAG_OPTIONS:
			sOption.assign (szElementName);
			eTag = OPTIONSTAG_OPTION;
			break;
	}
	XMLString::release (& szElementName);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cOptions::characters (const XMLCh *const chars, const unsigned int length) {
	char*  szMessage;

	szMessage = XMLString::transcode (chars);
	sLoadText.append (szMessage);
	XMLString::release (& szMessage);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cOptions::endElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname) {
	char * szElementName;

	szElementName = XMLString::transcode (localname);

	switch (eTag) {
	case OPTIONSTAG_OPTIONS:
		if (_stricmp (szElementName, "Options") == 0) {
			eTag = OPTIONSTAG_INVALID;
		}
		break;
	case OPTIONSTAG_OPTION:
		if (_stricmp (szElementName, sOption.c_str ()) == 0) {
			if (_stricmp (szElementName, "MachineLimit") == 0) {
				machineLimit = atoi (sLoadText.c_str ());
			}
			else if (_stricmp (szElementName, "StackSize") == 0) {
				stack = strtol (sLoadText.c_str (), NULL, 10);
			}
			else if (_stricmp (szElementName, "AllowMinimise") == 0) {
				allowMinimise = (atoi (sLoadText.c_str ()) != 0);
			}
			else if (_stricmp (szElementName, "AllowAgents") == 0) {
				allowAgents = (atoi (sLoadText.c_str ()) != 0);
			}
			else if (_stricmp (szElementName, "AutoCreate") == 0) {
				autoCreate = (atoi (sLoadText.c_str ()) != 0);
			}
			else if (_stricmp (szElementName, "AllowControl") == 0) {
				allowControl = (atoi (sLoadText.c_str ()) != 0);
			}
			else if (_stricmp (szElementName, "SelectSpeed") == 0) {
				selectSpeed = atoi (sLoadText.c_str ());
			}
			else if (_stricmp (szElementName, "ColourSpeed") == 0) {
				colourSpeed = atoi (sLoadText.c_str ());
			}
			else if (_stricmp (szElementName, "RotateHalfLife") == 0) {
				rotHalfLife = (float)atof (sLoadText.c_str ());
			}
			else if (_stricmp (szElementName, "RotateSelected") == 0) {
				rotSelected = (atoi (sLoadText.c_str ()) != 0);
			}
			else if (_stricmp (szElementName, "SoundVolume") == 0) {
				fSoundVolume = (float)atof (sLoadText.c_str ());
			}
			else if (_stricmp (szElementName, "ScreenshotMode") == 0) {
				screenShotMode = (atoi (sLoadText.c_str ()) != 0);
			}
			else if (_stricmp (szElementName, "Dandelion") == 0) {
				dandelion = (atoi (sLoadText.c_str ()) != 0);
			}
			else if (_stricmp (szElementName, "DandelionServer") == 0) {
				dandeServer.assign (sLoadText);
			}
			else if (_stricmp (szElementName, "RecordViewEvents") == 0) {
				viewEvents = (atoi (sLoadText.c_str ()) != 0);
			}

			eTag = OPTIONSTAG_OPTIONS;
		}
		break;
	}

	XMLString::release (& szElementName);
}
