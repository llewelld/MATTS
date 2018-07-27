/*$T MATTS/cConsole.cpp GC 1.140 07/01/09 21:12:08 */
/*
 * ;
 * Name: cConsole.cpp ;
 * Last Modified: 10/11/04 ;
 * ;
 * Purpose: Implementation file for console class ;
 */
#include "cWindow.h"
#include "cConsole.h"
#include "cOptions.h"
#include <stdio.h>
#include "resource.h"
#include <uxtheme.h>

bool cConsole:: flag = false;
cConsole* cConsole:: instance = NULL;

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */

cConsole::cConsole () {
	scrXPos = 0;
	bInput = false;
	pVM = NULL;
	cInput = 0;
	pCurrentVM = NULL;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cConsole::~cConsole () {
	flag = false;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cConsole* cConsole::create () {
	if (!flag) {
		flag = true;

		instance = new cConsole;
	}

	return instance;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cConsole::createWindow (HINSTANCE hinstance, LPSTR title, int iWidth, int iHeight, unsigned int iStyle) {
	HMENU hMenu;
	HBRUSH hBrush;

	MENUINFO sMenuInfo;
	INITCOMMONCONTROLSEX*  temp;
	hInstance = hinstance;

	/* Create menu */
	hMenu = LoadMenu (hInstance, MAKEINTRESOURCE (IDR_MENU4));

	/* Create window */
	wndClass.hCursor = LoadCursor (NULL, IDC_ARROW);
	wndClass.hIcon = (HICON) LoadIcon (hinstance, MAKEINTRESOURCE (IDI_MAIN));
	wndClass.hIconSm = (HICON) LoadIcon (hinstance, MAKEINTRESOURCE (IDI_SMACTIVE));
	wndClass.hbrBackground = (HBRUSH) GetStockObject (NULL_BRUSH);
	wndClass.hInstance = hInstance;
	wndClass.lpfnWndProc = wndProc;
	wndClass.lpszClassName = "console";

	wndClass.cbClsExtra = NULL;
	wndClass.cbSize = sizeof (WNDCLASSEX);
	wndClass.cbWndExtra = NULL;
	wndClass.lpszMenuName = NULL;
	wndClass.style = CS_DBLCLKS;

	if (!(RegisterClassEx (&wndClass))) {
		return false;
	}

	hWindow = CreateWindowEx ((WS_CLIPCHILDREN), "console", title, iStyle, CW_USEDEFAULT, CW_USEDEFAULT, iWidth,
							  iHeight, NULL, NULL, hInstance, (void*) this);
	SetMenu (hWindow, hMenu);
	SetWindowText (hWindow, title);

	hBrush = GetThemeSysColorBrush (NULL, COLOR_MENU);
	sMenuInfo.cbSize = sizeof (MENUINFO);
	sMenuInfo.fMask = MIM_BACKGROUND;
	sMenuInfo.hbrBack = hBrush;
	SetMenuInfo (hMenu, &sMenuInfo);
	DeleteObject (hBrush);

	/* Window created. get window rect for control size */
	GetWindowRect (hWindow, &winRect);

	/* Initialise tab control */
	temp = new INITCOMMONCONTROLSEX;
	temp->dwSize = sizeof (INITCOMMONCONTROLSEX);
	temp->dwICC = ICC_TAB_CLASSES;

	InitCommonControlsEx (temp);

	/* Create the vm tab list */
	vmTabList = CreateWindowEx (0, WC_TABCONTROL, "", WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE, 0, TAB_YOFFSET,
								winRect.right - 1, 30, hWindow, NULL, hInstance, NULL);

	if (vmTabList == NULL) {
		return NULL;
	}

	/*
	 * Uncomment the following line to open the console window on start-up ;
	 * ShowWindow(hWindow, SW_SHOWNORMAL);
	 */
	return true;
}

/*
 =======================================================================================================================
 *  void cConsole::ShowTopologyTab (void) ;
 *  { ;
 *  setVM (NULL);
 *  TabCtrl_SetCurSel (vmTabList, 0);
 *  activeID = TabCtrl_GetCurSel (vmTabList);
 *  changeTab ();
 *  } ;
 =======================================================================================================================
 */
void cConsole::OpenConsole () {
	ShowWindow (hWindow, SW_SHOWNORMAL);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cConsole::ConsoleInitialise (HINSTANCE hinstance, HWND hwnd, cWindow* psWin) {
	int nRow;

	createWindow (hinstance, "MATTS Console", 700, 500, (WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL));

	for (nRow = 0; nRow < 32; nRow++) {
		strcpy (console[nRow], "");
	}

	strcpy (string, "");

	currentRow = 0;
	scrXPos = 0;
	strcpy (string, "");

	psWindow = psWin;

	/*
	 * RedrawWindow(hWindow, NULL, NULL, (RDW_INVALIDATE | RDW_ERASE));
	 */
	WindowRedraw ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
LRESULT CALLBACK cConsole::wndProc (HWND window, UINT msg, WPARAM wparam, LPARAM lparam) {
	cConsole*  pWin;
	bool bProcessed;

	pWin = NULL;
	bProcessed = false;

	switch (msg) {
	case WM_NCCREATE:

		/* We don't want this to produce a warning every time we compile */
#pragma warning (disable : 4311)
		SetWindowLong (window, GWL_USERDATA, (long) ((LPCREATESTRUCT (lparam))->lpCreateParams));
#pragma warning (default : 4311)
		break;
	default:

		/* We don't want this to produce a warning every time we compile */
#pragma warning (disable : 4312)
		pWin = (cConsole*) GetWindowLong (window, GWL_USERDATA);
#pragma warning (default : 4312)
		if (pWin != NULL) {
			bProcessed = pWin->messageHandler (msg, wparam, lparam);
		}

		break;
	}

	if (bProcessed == false) {
		return DefWindowProc (window, msg, wparam, lparam);
	}

	return 0;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cConsole::messageHandler (UINT msg, WPARAM wparam, LPARAM lparam) {
	cConsole*  pCon;
	cOptions*  opt;

	pCon = cConsole::create ();
	opt = cOptions::create ();

	switch (msg) {
	case WM_CLOSE:
		cleanup ();
		return true;
		break;
	case WM_SIZE:
		resize (wparam, lparam);
		return true;
		break;
	case WM_COMMAND:
		switch (LOWORD (wparam)) {
		case ID_CURRENTMACHINE_CLEARCONSOLE:
			pCurrentVM->clearBuffer ();
			clear ();
			return true;
			break;
		case ID_CURRENTMACHINE_CLEARVM:
			DeleteVMConsole ();
			return true;
			break;
		case ID_VIEW_CLEARCONSOLE:
			clear ();
			return true;
			break;
		case ID_MENU_EXIT:
			cleanup ();
			break;
			return true;
		}

		return true;
		break;
	case WM_NOTIFY:
		switch (HIWORD (wparam)) {
		case 0:
			if (((NMHDR *) (lparam))->hwndFrom == vmTabList) {
				changeTab ();
				SetForegroundWindow (hWindow);
				return true;
			}

			break;
		}

		return true;
		break;
	case WM_PAINT:
		redraw ();
		return true;
		break;
	case WM_ERASEBKGND:
		return true;
		break;
	default:
		return false;
		break;
	}

	return false;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cConsole::cleanup () {
	ShowWindow (hWindow, SW_HIDE);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cConsole::redraw () {
	PAINTSTRUCT ps;
	HDC hdc;

	hdc = BeginPaint (hWindow, &ps);

	plotMain (&ps, hdc);

	EndPaint (hWindow, &ps);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cConsole::resize (WPARAM wparam, LPARAM lparam) {

	/* Window created. Get window rect for control size */
	GetClientRect (hWindow, &winRect);

	SetWindowPos (vmTabList, NULL, NULL, NULL, winRect.right - 1, 30, SWP_NOMOVE);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cConsole::NewVMConsole (bool boSetAsActive) {
	TCITEM sTabItem;
	int nTabID;
	char szTitle[100];

	/* Create Tab for VM */
	memset (&sTabItem, 0, sizeof (sTabItem));
	sTabItem.mask = TCIF_TEXT;

	nTabID = psWindow->getList ()->getCurrentID (psWindow->psTabIterator);

	sprintf (szTitle, "Node %d", nTabID);
	sTabItem.pszText = szTitle;

	/* Add tab to tab list */
	if (TabCtrl_InsertItem (vmTabList, nTabID, &sTabItem) == -1) {
		DestroyWindow (vmTabList);
		return false;
	}

	/* Select new VM */
	if (boSetAsActive) {
		SetActiveTab (nTabID);
	}

	WindowRedraw ();

	return true;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cConsole::DeleteVMConsole () {
	TabCtrl_DeleteItem (vmTabList, activeID);
	if (activeID > 0) {
		activeID--;
		TabCtrl_SetCurSel (vmTabList, activeID);
	} else {
		TabCtrl_SetCurSel (vmTabList, activeID);
	}

	WindowRedraw ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cConsole::changeTab () {
	cTopologyNode*	psIterator;

	if (pCurrentVM) {
		pCurrentVM->setActive (false);
	}

	activeID = TabCtrl_GetCurSel (vmTabList);

	/* Find the current machine in the list */
	psIterator = psWindow->getList ()->startIterator ();

	while
	(
		(psWindow->getList ()->getCurrentID (psIterator) != activeID)
	&&	(psWindow->getList ()->isIteratorValid (psIterator))
	) {
		psIterator = psWindow->getList ()->moveForward (psIterator);
	}

	if (psWindow->getList ()->getCurrentID (psIterator) != activeID) {

		/* Not there? */
		MessageBox (hWindow, "Cannot find machine in list", "Error", MB_OK | MB_ICONWARNING);
		return ;
	} else {
		pCurrentVM = psWindow->getList ()->getCurrentItem (psIterator);
		pCurrentVM->setActive (true);
		setVM (pCurrentVM);

		psWindow->SetMenuTick (hWindow, ID_CURRENTMACHINE_PAUSEPROGRAM, (pCurrentVM->threadPaused ()));
	}

	/* Redraw */
	WindowRedraw ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cConsole::SetTabName (char* szTitle) {
	TCITEM sTabItem;
	memset (&sTabItem, 0, sizeof (sTabItem));
	sTabItem.mask = TCIF_TEXT;

	sTabItem.pszText = szTitle;
	TabCtrl_SetItem (vmTabList, activeID, &sTabItem);

	WindowRedraw ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cConsole::SetActiveTab (int nTabID) {
	TabCtrl_SetCurSel (vmTabList, nTabID);
	activeID = TabCtrl_GetCurSel (vmTabList);

	EnableMenuItem (GetMenu (hWindow), ID_CURRENTMACHINE_LOADPROGRAM, (MF_BYCOMMAND | MF_ENABLED));
	EnableMenuItem (GetMenu (hWindow), ID_CURRENTMACHINE_SAVEPROGRAM, (MF_BYCOMMAND | MF_ENABLED));
	EnableMenuItem (GetMenu (hWindow), ID_CURRENTMACHINE_CLEARCONSOLE, (MF_BYCOMMAND | MF_ENABLED));
	EnableMenuItem (GetMenu (hWindow), ID_CURRENTMACHINE_CLOSEMACHINE, (MF_BYCOMMAND | MF_ENABLED));
	EnableMenuItem (GetMenu (hWindow), ID_CURRENTMACHINE_ENDPROGRAM, (MF_BYCOMMAND | MF_ENABLED));
	pCurrentVM->setActive (true);
	setVM (pCurrentVM);

	WindowRedraw ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cConsole::clear () {
	int nRow;
	for (nRow = 0; nRow < 32; nRow++) {
		strcpy (console[nRow], "");
	}

	currentRow = 0;
	scrXPos = 0;

	/*
	 * RedrawWindow(hWindow, NULL, NULL, (RDW_INVALIDATE | RDW_ERASE));
	 */
	ConsoleRedraw ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cConsole::scroll (void) {
	currentRow++;
	if (currentRow >= 32) {
		currentRow = 0;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cConsole::ConsolePrint (char* szString) {
	COLORREF uCol;
	int nStrPos;
	int nMaxLen;
	int nCopyPos;
	char cChar;

	uCol = RGB (0, 0, 0);

	nStrPos = 0;

	while (szString[nStrPos] > 0) {
		if (scrXPos >= 80) {

			/* Scroll the window */
			scroll ();
			scrXPos = 0;
		}

		nMaxLen = 80 - scrXPos;

		/*
		 * Copy the string over ;
		 * Until nMaxLen is reached, or a newline
		 */
		nCopyPos = 0;
		while ((nCopyPos < nMaxLen) && (szString[nStrPos] >= 0x20)) {
			cChar = szString[nStrPos++];
			console[currentRow][scrXPos + nCopyPos++] = cChar;
		}

		console[currentRow][scrXPos + nCopyPos] = 0;

		/* Print the line */
		scrXPos += nCopyPos;

		if ((szString[nStrPos] == 0x0d) || (szString[nStrPos] == 0x0a)) {

			/* Newline */
			scrXPos = 80;
			nStrPos++;
		}
	}

	ConsoleRedraw ();

	/*
	 * RedrawWindow(hWindow, NULL, NULL, (RDW_INVALIDATE | RDW_ERASE));
	 */
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cConsole::plotMain (PAINTSTRUCT* psPaint, HDC sHDC) {
	COLORREF uPrevFore;
	COLORREF uPrevBack;
	HFONT sPrevFont;
	HFONT sFont;
	LOGFONT sFontInfo;
	int nRow;
	int nIndex;
	char szInput[sizeof (string) + 3];

	uPrevFore = SetTextColor (sHDC, RGB (0x00, 0x00, 0x00));
	uPrevBack = SetBkColor (sHDC, RGB (0xff, 0xff, 0xff));

	/* Clear the area */
	FillRect (sHDC, &psPaint->rcPaint, (HBRUSH) (GetStockObject (WHITE_BRUSH)));

	memset (&sFontInfo, 0, sizeof (LOGFONT));
	sFontInfo.lfHeight = 16;
	strcpy (sFontInfo.lfFaceName, "Courier New");
	sFont = ::CreateFontIndirect (&sFontInfo);
	sPrevFont = (HFONT)::SelectObject (sHDC, sFont);

	/* Font and colours set up, so now do the plotting */
	nIndex = currentRow;
	for (nRow = 0; nRow < 32; nRow++) {
		SetTextColor (sHDC, RGB (0, 0, 0));
		ExtTextOut (sHDC, 5, ((16 * 32) - (16 * nRow)) + 15, 0, NULL, console[nIndex], (int) strlen (console[nIndex]),
					NULL);
		nIndex--;
		if (nIndex < 0) {
			nIndex = 32 - 1;
		}
	}

	if (pVM != NULL) {
		bInput = pVM->isInput ();
	} else {
		bInput = false;
	}

	if (bInput) {
		SetTextColor (sHDC, RGB (0xff, 0x00, 0x00));
		sprintf (szInput, "> %s", pVM->getInBuffer ());
		ExtTextOut (sHDC, 4, (16 * (32 + 2)), 0, NULL, szInput, (int) strlen (szInput), NULL);
	}

	/* Font and colours are about to be restored, so finish all plotting. */
	::SelectObject (sHDC, sPrevFont);
	DeleteObject (sFont);
	SetTextColor (sHDC, uPrevFore);
	SetBkColor (sHDC, uPrevBack);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cConsole::setVM (cVM* vm) {
	pVM = vm;
	clear ();
	if (vm != NULL) {
		vm->getBuffer (console, &scrXPos, &currentRow, string, &nPos);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
char* cConsole::ConsoleInputString (void) {
	cVM*  currentInputVM;

	currentInputVM = pVM;

	bInput = true;

	/*
	 * RedrawWindow(hWindow, NULL, NULL, (RDW_INVALIDATE | RDW_ERASE));
	 */
	ConsoleRedraw ();
	currentInputVM->clearInput ();

	do
	{

		/* may be inactive machine, so susspend input for 1/2 sec and check again */
		while (!currentInputVM->getActive ()) {
			Sleep (500);
		}

		switch (cInput) {
		case 8: /* Delete */
			currentInputVM->inputChar (8);
			cInput = 0;

			/*
			 * RedrawWindow(hWindow, NULL, NULL, (RDW_INVALIDATE | RDW_ERASE));
			 */
			ConsoleRedraw ();
			break;
		default:
			if (cInput >= 0x20) {
				currentInputVM->inputChar (cInput);
				cInput = 0;

				/*
				 * RedrawWindow(hWindow, NULL, NULL, (RDW_INVALIDATE | RDW_ERASE));
				 */
				ConsoleRedraw ();
			}

			break;
		}
	} while ((cInput != 0x0d) && (pVM != NULL) && (!pVM->getRestart ()));

	cInput = 0;
	currentInputVM->ConsolePrint ("> ");
	strcpy (string, currentInputVM->getInBuffer ());
	currentInputVM->ConsolePrint (string);
	currentInputVM->ConsolePrint ("\n");

	/*
	 * RedrawWindow(hWindow, NULL, NULL, (RDW_INVALIDATE | RDW_ERASE));
	 */
	ConsoleRedraw ();

	bInput = false;
	currentInputVM->clearInput ();

	return string;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cConsole::ConsoleInputNumber (void) {
	cVM*  currentInputVM;

	long int nDigit;
	long int nNumber;

	currentInputVM = pVM;

	bInput = true;

	/*
	 * RedrawWindow(hWindow, NULL, NULL, (RDW_INVALIDATE | RDW_ERASE));
	 */
	ConsoleRedraw ();
	currentInputVM->clearInput ();

	do
	{

		/* may be inactive machine, so susspend input for 1/2 sec and check again */
		while (!currentInputVM->getActive ()) {
			Sleep (500);
		}

		switch (cInput) {
		case 8: /* Delete */
			currentInputVM->inputChar (8);
			cInput = 0;

			/*
			 * RedrawWindow(hWindow, NULL, NULL, (RDW_INVALIDATE | RDW_ERASE));
			 */
			ConsoleRedraw ();
			break;
		default:
			if (cInput >= 0x20) {
				currentInputVM->inputChar (cInput);
				cInput = 0;

				/*
				 * RedrawWindow(hWindow, NULL, NULL, (RDW_INVALIDATE | RDW_ERASE));
				 */
				ConsoleRedraw ();
			}

			break;
		}
	} while ((cInput != 0x0d) && (pVM != NULL) && (!pVM->getRestart ()));
	cInput = 0;
	currentInputVM->ConsolePrint ("> ");
	strcpy (string, currentInputVM->getInBuffer ());
	currentInputVM->ConsolePrint (string);
	currentInputVM->ConsolePrint ("\n");

	/*
	 * RedrawWindow(hWindow, NULL, NULL, (RDW_INVALIDATE | RDW_ERASE));
	 */
	ConsoleRedraw ();

	bInput = false;

	nDigit = 0;
	nNumber = 0;

	if (string[0] == '-') {
		nDigit++;
	}

	while ((string[nDigit] >= '0') && (string[nDigit] <= '9')) {
		nNumber *= 10;
		nNumber += (string[nDigit] - '0');
		nDigit++;
	}

	if (string[0] == '-') {
		nNumber = -nNumber;
	}

	currentInputVM->clearInput ();

	return nNumber;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cConsole::ConsoleRedraw (void) {
	RECT sRect;

	GetClientRect (hWindow, &sRect);
	sRect.top += CONTROLS_YOFFSET;
	RedrawWindow (hWindow, &sRect, NULL, (RDW_INVALIDATE | RDW_ERASE));
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cConsole::WindowRedraw (void) {
	RECT sRect;

	GetClientRect (hWindow, &sRect);
	RedrawWindow (hWindow, &sRect, NULL, (RDW_INVALIDATE | RDW_ERASE));
}
