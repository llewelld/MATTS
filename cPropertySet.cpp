/*$T DynamicMatts/cPropertySet.cpp GC 1.140 12/27/09 14:10:04 */

#include "cXMLPropertySet.h"
#pragma warning(disable : 4267)
#pragma warning(disable : 4244)
#include <xercesc/sax2/XMLReaderFactory.hpp>
#pragma warning(default : 4267)
#pragma warning(default : 4244)
#include "cPropertySet.h"
#include "resource.h"

XERCES_CPP_NAMESPACE_USE

/*
 =======================================================================================================================
 *    Defines
 =======================================================================================================================
 */
#define WIDGET_STARTX	(6)
#define WIDGET_STARTY	(10)
#define WIDGET_WIDTH	(84-6)
#define LABEL_WIDTH	(120)
#define WIDGET_HEIGHT	(20)
#define WIDGET_DROPLISTHEIGHT	(260)
#define WIDGET_NUMHEIGHT	(50)
#define WIDGET_GAPY	(6)
#define WIDGET_GAPX	(12)
#define WIDGET_ENDGAPX	(44)
#define WIDGET_ENDGAPY	(58)

#define CLASSNAME_MAXLEN	(256)

 /*
 =======================================================================================================================
 *    Constructor
 =======================================================================================================================
 */
cPropertySet::cPropertySet (cWindow * psWindow) {
	this->psWindow = psWindow;
	pcDefaultNodeLink = NULL;
}

/*
 =======================================================================================================================
 *    Destructor
 =======================================================================================================================
 */
cPropertySet::~cPropertySet () {
}

/*
 =======================================================================================================================
 *    Load the controls from the XML file
 =======================================================================================================================
 */
cControls* cPropertySet::LoadXML (char* FileName) {
	SAX2XMLReader*	pcParser;
	cXMLPropertySet * pcHandler;
	cControls*	pcControls;

	XMLPlatformUtils::Initialize ();
	pcParser = XMLReaderFactory::createXMLReader ();
	pcHandler = new cXMLPropertySet (psWindow);
	pcHandler->ChangeDefaults (pcDefaultNodeLink);

	pcParser->setContentHandler (pcHandler);
	pcParser->parse (FileName);

	pcControls = pcHandler->ReturnControls ();

	delete pcParser;

	/* And call the termination method */
	XMLPlatformUtils::Terminate ();

	return pcControls;
}	/* End of loadXML */

/*
 =======================================================================================================================
 *    Create the window based on the controls
 =======================================================================================================================
 */
HWND cPropertySet::GenerateWindow (cControls* acControls) {
	HWND hMyDialog;
	HWND hControl;
	int nPositionX;
	int nPositionY;
	cControls * pcCurrent;
	nPositionX = WIDGET_STARTX;
	nPositionY = WIDGET_STARTY;
	int nWidth;
	int nHeight;
	char szNumber[5];

	// Create the window
	hMyDialog = CreateWindowEx (0, WC_DIALOG, "Properties",
							   WS_TABSTOP | WS_OVERLAPPEDWINDOW | WS_VISIBLE | ES_AUTOVSCROLL | ES_AUTOHSCROLL, 200,
							   100, 50, 50, NULL, NULL, NULL, NULL);

	pcCurrent = acControls;
	while (pcCurrent) {
		/*
		 * TextBox Controls
		 * Create the label
		 */
		CreateWindowEx (0, "STATIC", pcCurrent->szLabel.c_str (), WS_CHILD | WS_VISIBLE | WS_TABSTOP, nPositionX,
						nPositionY, WIDGET_GAPX, WIDGET_HEIGHT, hMyDialog, NULL, NULL, NULL);
		if (strcmp (pcCurrent->szCaption.c_str (), "Text") == 0) {
			/* create the textbox */
			hControl = CreateWindowEx (WS_EX_CLIENTEDGE,	/* special border style for textbox */
										   "EDIT", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP, nPositionX + WIDGET_GAPX, nPositionY,
										   WIDGET_WIDTH, WIDGET_HEIGHT, hMyDialog, (HMENU) pcCurrent->nControlID, NULL, NULL);
			nPositionY += (WIDGET_HEIGHT + WIDGET_GAPY);
		}
		else if (strcmp (pcCurrent->szCaption.c_str (), "Option") == 0) {
			/* ComboBox Controls */
			/* Create Combo */
			hControl = CreateWindowEx (0, "COMBOBOX", NULL,
										   WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST,
											 nPositionX + WIDGET_GAPX, nPositionY, WIDGET_WIDTH, WIDGET_DROPLISTHEIGHT, hMyDialog, (HMENU) pcCurrent->nControlID,
										   NULL, NULL);
			/* Assign values to Combo */
			list<string>::iterator cValue = pcCurrent->cValue.begin ();
			while (cValue != pcCurrent->cValue.end()) {
				SendMessage (hControl, CB_ADDSTRING, 0,
					reinterpret_cast<LPARAM> ((LPCTSTR) (*cValue).c_str ()));
				++cValue;
			}
			nPositionY += (WIDGET_HEIGHT + WIDGET_GAPY);
		}
		else if (strcmp (pcCurrent->szCaption.c_str (), "Integer") == 0) {
			/* Integer Controls */
			/* Create Combo */
			hControl = CreateWindowEx (0, "COMBOBOX", NULL,
										   WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_AUTOHSCROLL,
										   nPositionX + 120, nPositionY, WIDGET_WIDTH, WIDGET_NUMHEIGHT, hMyDialog, (HMENU) pcCurrent->nControlID,
										   NULL, NULL);
			/* Assign values to Combo */
			for (int j = pcCurrent->nMinValue; j <= pcCurrent->nMaxValue; j++) {
				_snprintf (szNumber, 5, "%d", j);
				szNumber[4] = 0;
				SendMessage (hControl, /* hWndComboBox */ CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ((LPCTSTR) szNumber));
			}
			nPositionY += (WIDGET_NUMHEIGHT + WIDGET_GAPY);
		}
		else if (strcmp (pcCurrent->szCaption.c_str (), "Floating Point") == 0) {
			/* Floating Point Control */
			/* Create the textbox */
			hControl = CreateWindowEx (WS_EX_CLIENTEDGE,	/* special border style for textbox */
										   "EDIT", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP, nPositionX + WIDGET_GAPX, nPositionY,
										   WIDGET_WIDTH, WIDGET_HEIGHT, hMyDialog, (HMENU) pcCurrent->nControlID, NULL, NULL);
			nPositionY += (WIDGET_HEIGHT + WIDGET_GAPY);
		}
		else if (strcmp (pcCurrent->szCaption.c_str (), "Binary") == 0) {
			/* Binary Control */
			/* Create the checkbox */
			hControl = CreateWindowEx (0, "BUTTON", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
										   nPositionX + WIDGET_GAPX, nPositionY, WIDGET_WIDTH, WIDGET_HEIGHT, hMyDialog, (HMENU) pcCurrent->nControlID,
										   NULL, NULL);
			nPositionY += (WIDGET_HEIGHT + WIDGET_GAPY);
		}
		pcCurrent = pcCurrent->GetNext ();
	}

	/* Save Button */
	CreateWindowEx (0, "BUTTON", "Save", WS_CHILD | WS_VISIBLE | WS_TABSTOP, nPositionX + WIDGET_GAPX, nPositionY, WIDGET_WIDTH, WIDGET_HEIGHT,
					hMyDialog, (HMENU) 128, NULL, NULL);
	nPositionY += (WIDGET_HEIGHT + WIDGET_GAPY);

	// Set the window size
	nWidth = (WIDGET_STARTX + WIDGET_GAPX + WIDGET_WIDTH + WIDGET_ENDGAPX);
	nHeight = (nPositionY + WIDGET_GAPY + WIDGET_ENDGAPY);
	SetWindowPos (hMyDialog, HWND_TOP, 0, 0, nWidth, nHeight, (SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER));

	return hMyDialog;
}

/*
 =======================================================================================================================
 *    Create the window based on the controls
 =======================================================================================================================
 */
int cPropertySet::AddPropertySetToWindow (HWND hMyDialog, int nYStart, cProperties * pcProperties) {
	PropertyMap const * pcPropertyMap;
	PropertyMap::const_iterator cPropertyIter;
	Property const * psProperty;
	string szName;
	int nPositionX;
	int nPositionY;
	HWND hOldWidget;
	HWND hNewWidget;
	int nWidth;
	int nHeight;
	char szClassName[CLASSNAME_MAXLEN];
	LRESULT hFont;
	WINDOWINFO sWindowInfo;
	list<string>::const_iterator cOptionIter;
	RECT sPos;
	POINT sPoint;
	char szTemp[256];

	// Find the start position from the OK button
	hOldWidget = GetDlgItem (hMyDialog, IDOK);
	GetWindowRect (hOldWidget, & sPos);
	sPoint.x = sPos.left;
	sPoint.y = sPos.top;
	ScreenToClient (hMyDialog, & sPoint);
	nYStart = sPoint.y;

	sWindowInfo.cbSize = sizeof (sWindowInfo);
	hFont = SendMessage (hMyDialog, WM_GETFONT, 0, 0);

	nPositionX = WIDGET_STARTX;
	nPositionY = nYStart;
	pcPropertyMap = pcProperties->GetPropertyMap ();
	for (cPropertyIter = pcPropertyMap->begin (); cPropertyIter != pcPropertyMap->end (); ++cPropertyIter) {
		szName = cPropertyIter->first;
		psProperty = & cPropertyIter->second;

		// Add the label
		hOldWidget = GetDlgItem (hMyDialog, IDC_TEMPLATELABEL);
		GetClassName (hOldWidget, szClassName, sizeof (szClassName));
		hNewWidget = CreateWindowEx (WS_EX_RIGHT, szClassName, szName.c_str (), WS_CHILD | WS_VISIBLE | WS_TABSTOP, nPositionX, nPositionY, LABEL_WIDTH, WIDGET_HEIGHT, hMyDialog, NULL, NULL, NULL);
		SendMessage (hNewWidget, WM_SETFONT, hFont, 0);

		switch (psProperty->eType) {
		case PROPTYPE_INT:
			// Create the control
			hOldWidget = GetDlgItem (hMyDialog, IDC_TEMPLATEINT);
			GetClassName (hOldWidget, szClassName, sizeof (szClassName));
			GetWindowInfo (hOldWidget, & sWindowInfo);
			nWidth = sWindowInfo.rcWindow.right - sWindowInfo.rcWindow.left;
			nHeight = sWindowInfo.rcWindow.bottom - sWindowInfo.rcWindow.top;
			hNewWidget = CreateWindowEx (WS_EX_CLIENTEDGE, szClassName, "", WS_CHILD | WS_VISIBLE | WS_TABSTOP, nPositionX + LABEL_WIDTH + WIDGET_GAPX, nPositionY, nWidth, nHeight, hMyDialog, NULL, NULL, NULL);
			SendMessage (hNewWidget, WM_SETFONT, hFont, 0);
			nPositionY += (WIDGET_HEIGHT + WIDGET_GAPY);
			// Set its value
			_snprintf (szTemp, sizeof (szTemp), "%d", psProperty->nValue);
			SendMessage (hNewWidget, WM_SETTEXT, 0, reinterpret_cast<LPARAM> ((LPCTSTR)szTemp));
			// Add the control to the controls list
			AddControl (szName.c_str (), hNewWidget);
			break;
		case PROPTYPE_BOOL:
			// Create the control
			hOldWidget = GetDlgItem (hMyDialog, IDC_TEMPLATEBOOL);
			GetClassName (hOldWidget, szClassName, sizeof (szClassName));
			GetWindowInfo (hOldWidget, & sWindowInfo);
			nWidth = sWindowInfo.rcWindow.right - sWindowInfo.rcWindow.left;
			nHeight = sWindowInfo.rcWindow.bottom - sWindowInfo.rcWindow.top;
			hNewWidget = CreateWindowEx (0, szClassName, "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, nPositionX + LABEL_WIDTH + WIDGET_GAPX, nPositionY, nWidth, nHeight, hMyDialog, NULL, NULL, NULL);
			SendMessage (hNewWidget, WM_SETFONT, hFont, 0);
			nPositionY += (WIDGET_HEIGHT + WIDGET_GAPY);
			// Set its value
			SendMessage (hNewWidget, BM_SETCHECK, (psProperty->boValue ? BST_CHECKED : BST_UNCHECKED), 0);
			// Add the control to the controls list
			AddControl (szName.c_str (), hNewWidget);
			break;
		case PROPTYPE_FLOAT:
			// Create the control
			hOldWidget = GetDlgItem (hMyDialog, IDC_TEMPLATEFLOAT);
			GetClassName (hOldWidget, szClassName, sizeof (szClassName));
			GetWindowInfo (hOldWidget, & sWindowInfo);
			nWidth = sWindowInfo.rcWindow.right - sWindowInfo.rcWindow.left;
			nHeight = sWindowInfo.rcWindow.bottom - sWindowInfo.rcWindow.top;
			hNewWidget = CreateWindowEx (WS_EX_CLIENTEDGE, szClassName, "", WS_CHILD | WS_VISIBLE | WS_TABSTOP, nPositionX + LABEL_WIDTH + WIDGET_GAPX, nPositionY, nWidth, nHeight, hMyDialog, NULL, NULL, NULL);
			SendMessage (hNewWidget, WM_SETFONT, hFont, 0);
			nPositionY += (WIDGET_HEIGHT + WIDGET_GAPY);
			// Set its value
			_snprintf (szTemp, sizeof (szTemp), "%f", psProperty->fValue);
			SendMessage (hNewWidget, WM_SETTEXT, 0, reinterpret_cast<LPARAM> ((LPCTSTR)szTemp));
			// Add the control to the controls list
			AddControl (szName.c_str (), hNewWidget);
			break;
		case PROPTYPE_OPTION:
			// Create the control
			hOldWidget = GetDlgItem (hMyDialog, IDC_TEMPLATEOPTION);
			GetClassName (hOldWidget, szClassName, sizeof (szClassName));
			GetWindowInfo (hOldWidget, & sWindowInfo);
			nWidth = sWindowInfo.rcWindow.right - sWindowInfo.rcWindow.left;
			nHeight = sWindowInfo.rcWindow.bottom - sWindowInfo.rcWindow.top;
			hNewWidget = CreateWindowEx (WS_EX_CLIENTEDGE, szClassName, "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST, nPositionX + LABEL_WIDTH + WIDGET_GAPX, nPositionY, nWidth, nHeight, hMyDialog, NULL, NULL, NULL);
			SendMessage (hNewWidget, WM_SETFONT, hFont, 0);
			nPositionY += (WIDGET_HEIGHT + WIDGET_GAPY);
			/* Assign values to Combo */
			cOptionIter = psProperty->aszValue.begin ();
			while (cOptionIter != psProperty->aszValue.end()) {
				SendMessage (hNewWidget, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ((LPCTSTR) (*cOptionIter).c_str ()));
				++cOptionIter;
			}
			// Set its value
			if (psProperty->szValue) {
				SendMessage (hNewWidget, CB_SELECTSTRING, -1, reinterpret_cast<LPARAM> ((LPCTSTR)psProperty->szValue));
			}
			// Add the control to the controls list
			AddControl (szName.c_str (), hNewWidget);
			break;
		case PROPTYPE_STRING:
			// Create the control
			hOldWidget = GetDlgItem (hMyDialog, IDC_TEMPLATESTRING);
			GetClassName (hOldWidget, szClassName, sizeof (szClassName));
			GetWindowInfo (hOldWidget, & sWindowInfo);
			nWidth = sWindowInfo.rcWindow.right - sWindowInfo.rcWindow.left;
			nHeight = sWindowInfo.rcWindow.bottom - sWindowInfo.rcWindow.top;
			hNewWidget = CreateWindowEx (WS_EX_CLIENTEDGE, szClassName, "", WS_CHILD | WS_VISIBLE | WS_TABSTOP, nPositionX + LABEL_WIDTH + WIDGET_GAPX, nPositionY, nWidth, nHeight, hMyDialog, NULL, NULL, NULL);
			SendMessage (hNewWidget, WM_SETFONT, hFont, 0);
			nPositionY += (WIDGET_HEIGHT + WIDGET_GAPY);
			// Set its value
			_snprintf (szTemp, sizeof (szTemp), "%s", psProperty->szValue);
			SendMessage (hNewWidget, WM_SETTEXT, 0, reinterpret_cast<LPARAM> ((LPCTSTR)szTemp));
			// Add the control to the controls list
			AddControl (szName.c_str (), hNewWidget);
			break;
		default:
			// Do nothing
			break;
		}
	}

	// Relocate the OK and Cancel buttons
	// OK button
	hOldWidget = GetDlgItem (hMyDialog, IDOK);
	GetWindowRect (hOldWidget, & sPos);
	nWidth = sPos.right- sPos.left;
	nHeight = sPos.bottom - sPos.top;
	sPoint.x = sPos.left;
	sPoint.y = sPos.top;
	ScreenToClient (hMyDialog, & sPoint);
	SetWindowPos (hOldWidget, HWND_TOP, sPoint.x, nPositionY, nWidth, nPositionY + nHeight, (SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER));
	// Cancel button
	hOldWidget = GetDlgItem (hMyDialog, IDCANCEL);
	GetWindowRect (hOldWidget, & sPos);
	nWidth = sPos.right- sPos.left;
	nHeight = sPos.bottom - sPos.top;
	sPoint.x = sPos.left;
	sPoint.y = sPos.top;
	ScreenToClient (hMyDialog, & sPoint);
	SetWindowPos (hOldWidget, HWND_TOP, sPoint.x, nPositionY, nWidth, nPositionY + nHeight, (SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER));

	// Set the window size
	GetWindowRect (hMyDialog, & sPos);
	nWidth = (sPos.right - sPos.left);
	if (nWidth < (WIDGET_STARTX + WIDGET_GAPX + WIDGET_WIDTH + WIDGET_ENDGAPX)) {
		nWidth = (WIDGET_STARTX + WIDGET_GAPX + WIDGET_WIDTH + WIDGET_ENDGAPX);
	}
	nHeight = (nPositionY + WIDGET_GAPY + WIDGET_ENDGAPY);
	SetWindowPos (hMyDialog, HWND_TOP, 0, 0, nWidth, nHeight, (SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER));

	return nPositionY;
}

/*
 =======================================================================================================================
 *    Process windows messages
 =======================================================================================================================
 */
BOOL WINAPI cPropertySet::ProcessMessage (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	//static cControls * pcCurrent = NULL;

	if (message == WM_CLOSE) {
		PostQuitMessage (0);
	}

	//pcCurrent = pcControls;
	//while (pcCurrent) {
	//	if (strcmp (pcCurrent->szCaption.c_str (), "Binary") == 0) {

	//		/* For Check Box */
	//		if ((message == WM_COMMAND) && (LOWORD (wParam) == pcCurrent->nControlID)) {

	//			/* First get handle of checkbox by casting lParam to HWND */
	//			HWND hCheckboxHandle;
	//			hCheckboxHandle = (HWND) lParam;

	//			/* send message BM_GETCHECK to check box to retrieve its current state */
	//			LRESULT nState;
	//			nState = SendMessage (hCheckboxHandle, BM_GETCHECK, 0, 0);

	//			if (nState == BST_CHECKED)
	//				MessageBox (hwnd, "It hs checked", pcCurrent->szLabel.c_str (), 0);
	//			if (nState == BST_UNCHECKED)
	//				MessageBox (hwnd, "It hs Unchecked", pcCurrent->szLabel.c_str (), 0);
	//		}
	//	}
	//	pcCurrent = pcCurrent->GetNext ();
	//}

	//if (message == WM_COMMAND && LOWORD (wParam) == 128) {
	//	FILE*  hFile;
	//	char*  szString;
	//	hFile = fopen ("myfile.xml", "w");
	//	szString = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>";
	//	fprintf (hFile, "%s", szString);
	//	fprintf (hFile, "\n%s<Node>\n", " ");

	//	fprintf (hFile, "%s</Node>", " ");
	//	fclose (hFile);
	//	pcCurrent = pcControls;
	//	while (pcCurrent) {
	//		char szDialogueText[128];
	//		if (strcmp (pcCurrent->szCaption.c_str (), "Binary") == 0) {

	//			/* First get handle of checkbox by casting lParam to HWND */
	//			if (SendMessage (GetDlgItem (hMyDialog, pcCurrent->nControlID), BM_GETCHECK, 0, 0) == BST_CHECKED)
	//				MessageBox (hwnd, "First It hs checked", "Message", 0);
	//			else
	//				MessageBox (hwnd, "First It has not checked", "Message", 0);
	//		} else {
	//			GetDlgItemText (hMyDialog, pcCurrent->nControlID, szDialogueText, 128);

	//			MessageBox (hwnd, szDialogueText, NULL, NULL);
	//		}
	//		pcCurrent = pcCurrent->GetNext ();
	//	}
	//}

	return false;
}

/*
 =======================================================================================================================
 *    Find the control handle from the given property name
 =======================================================================================================================
 */
HWND cPropertySet::ControlHandleFromName (char const * szName) {
	ControlList::iterator sControlIter;
	HWND hReturn;
	bool boFound;

	hReturn = 0;
	boFound = false;
	for (sControlIter = sControls.begin (); ((sControlIter != sControls.end ()) && (!boFound)); ++sControlIter) {
		if (strcmp (sControlIter->szProperty, szName) == 0) {
			hReturn = sControlIter->hControl;
			boFound = true;
		}
	}

	return hReturn;
}

/*
 =======================================================================================================================
 *    Find the property name from the given control handle
 =======================================================================================================================
 */
char const * cPropertySet::ControlNameFromHandle (HWND hControl) {
	ControlList::iterator sControlIter;
	char const * szReturn;
	bool boFound;

	szReturn = NULL;
	boFound = false;
	for (sControlIter = sControls.begin (); ((sControlIter != sControls.end ()) && (!boFound)); ++sControlIter) {
		if (sControlIter->hControl == hControl) {
			szReturn = sControlIter->szProperty;
			boFound = true;
		}
	}

	return szReturn;
}

/*
 =======================================================================================================================
 *    Clear the controls list
 =======================================================================================================================
 */
void cPropertySet::ClearControls () {
	sControls.clear ();
}

/*
 =======================================================================================================================
 *    Add a control handle, name pair to the controls list
 =======================================================================================================================
 */
void cPropertySet::AddControl (char const * szProperty, HWND hControl) {
	DynamicHandles sDynamicHandle (szProperty, hControl);
	sControls.push_front (sDynamicHandle);
}

/*
 =======================================================================================================================
 *    Returns the control list (useful for iterating)
 =======================================================================================================================
 */
ControlList const cPropertySet::GetControlList () {
	return sControls;
}

/*
 =======================================================================================================================
 *    Set up the default properties for when loading a Property Set file
 =======================================================================================================================
 */
void cPropertySet::ChangeDefaults (cProperties * pcDefaultNodeLink) {
	this->pcDefaultNodeLink = pcDefaultNodeLink;
}
/*
 =======================================================================================================================
 *    Constructor
 =======================================================================================================================
 */
DynamicHandles::DynamicHandles (char const * szProperty, HWND hControl) {
	this->hControl = hControl;
	if (szProperty) {
		this->szProperty = (char *)malloc (strlen (szProperty) + 1);
		strcpy (this->szProperty, szProperty);
	}
	else {
		szProperty = NULL;
	}
}

/*
 =======================================================================================================================
 *    Destructor
 =======================================================================================================================
 */
DynamicHandles::~DynamicHandles () {
	free (szProperty);
}

/*
 =======================================================================================================================
 *    Copy constructor
 =======================================================================================================================
 */
DynamicHandles::DynamicHandles (DynamicHandles const & sCopyFrom) {
	hControl = sCopyFrom.hControl;
	if (sCopyFrom.szProperty) {
		szProperty = (char *)malloc (strlen (sCopyFrom.szProperty) + 1);
		strcpy (szProperty, sCopyFrom.szProperty);
	}
	else {
		szProperty = NULL;
	}
}

