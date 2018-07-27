/*$T MATTS/cWindow.cpp GC 1.140 07/01/09 21:12:10 */
/*
 * ;
 * Name: cWindow.cpp ;
 * Last Modified: 10/11/04 ;
 * ;
 * Purpose: Implementation file for window class ;
 * ;
 * TODO
 */
#define VERSION				3.00
#define WM_APP_NOTIFYICON	(WM_APP + 0x01)
#include "cWindow.h"
#include "resource.h"
#include <stdio.h>
#include "cConsole.h"
#include "cOptions.h"
#include <string.h>
#include <windows.h>
#include <windowsx.h>
#include <Mmsystem.h>
#include "cAnalyse.h"
#include "cXMLCompile.h"
#include "cServer.h"
#include "cServerDande.h"
#include <cmath>
#include <uxtheme.h>
#include "cMap.h"
#include "cPropertySet.h"
#include "cXMLPropertySet.h"

#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glut.h>
#include <atlstr.h>
#include <fstream>
#include <iostream>

/*
 * Socket notification commands. ;
 * int port = 1984;
 */

#define ADHOC_SCALE					(25.0f)
#define EVENT_COLUMN_TIME_WIDTH		(48)
#define EVENT_COLUMN_EVENT_WIDTH	(152)
#define SPINMOMENTUM_STOPX			(0.002f)
#define SPINMOMENTUM_STOPY			(0.002f)
#define SPINMOMENTUM_STOPZ			(0.002f)
#define SPINMOMENTUM_RESISTANCE		(0.90f)
#define TRANSLATEMOMENTUM_STOPX			(0.002f)
#define TRANSLATEMOMENTUM_STOPY			(0.002f)
#define TRANSLATEMOMENTUM_STOPZ			(0.002f)
#define TRANSLATEMOMENTUM_RESISTANCE		(0.90f)
#define SCROLLWHEEL_EVENTDELAY		(500)
#define ADHOC_SLIDER_FACTOR			(2)
#define VKEY_DOWN								(0xF0)

/*
 * glTranslatef ((-MGL_WIDTH/2.0f + (float)nXPos) / MGL_SCALE, (MGL_HEIGHT/2.0f
 * (float)nYPos) / MGL_SCALE, (-MGL_DEPTH/2.0f + (float)nZPos) / MGL_SCALE);
 */
#define MGL_SCALE_PREV			(218.0f)
#define MGL_WIDTH_PREV			(600.0f)
#define MGL_HEIGHT_PREV			(620.0f)
#define MGL_DEPTH_PREV			(0.0f)
#define LOAD_RESCALE_THRESHOLD	(MGL_SCALE_PREV * 2)

/* Function prototypes */
void threadProc (cVM* pCurrentVM);
void listenProc (cWindow* pWin);
void listenControlProc (cWindow* pWin);
void LeftButtonDown (int nXMousePos, int nYMousePos);
void LeftButtonUp (int nXMousePos, int nYMousePos);
void PopulateEventListCallback (unsigned int uStart, unsigned int uEnd, char const* szEvent, void* pData);
void runServer (cWindow* pWin);
void runServerDande (cWindow* pWin);

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */

cWindow::cWindow () {
	pConsole = NULL;
	pList = NULL;
	psTabIterator = NULL;
	pTopology = NULL;
	trayActive = false;
	pXMLParse = NULL;
	geViewState = VIEWSTATE_SELECT;
	gboSpin = FALSE;
	boFreezeRedraw = false;
	geDemoCycle = DEMOCYCLE_STOPPED;
	eDragType = DRAGTYPE_INVALID;
	fXDragOffset = 0;
	fYDragOffset = 0;
	fZDragOffset = 0;
	uDragStartTime = TIMER_TIME_MIN;
	fSpinMomentum = 0.0f;
	fSpinMomentumXComponent = 0.0f;
	fSpinMomentumYComponent = 0.0f;
	fSpinMomentumZComponent = 0.0f;
	fTranslateMomentum = 0.0f;
	fTranslateMomentumXComponent = 0.0f;
	fTranslateMomentumYComponent = 0.0f;
	fTranslateMomentumZComponent = 0.0f;
	uScrollWheelTimeStart = 0u;
	uScrollWheelTimeLast = 0u;
	psDlClient = NULL;
	nCurID = -1;
	nCurDeleteID = -1;
	szNameList = (char*) malloc (1024);
	szNameList[0] = 0;
	psServer = new cServer ();
	psServerDande = new cServerDande ();
	boNextRenderType = FALSE;
	boFullScreen = false;
	psFileHub = new cFileHub ();
	pcDefaultProperties = new cProperties ();
	boDrag = false;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cWindow::~cWindow () {
	if (pList != NULL) {
		delete pList;
		pList = NULL;
		psTabIterator = NULL;
	}

	if (pConsole != NULL) {
		delete pConsole;
		pConsole = NULL;
	}

	if (pTopology != NULL) {
		delete pTopology;
		pTopology = NULL;
	}

	if (pXMLParse != NULL) {
		delete pXMLParse;
		pXMLParse = NULL;
	}

	if (psDlClient) {
		delete psDlClient;
		psDlClient = NULL;
	}

	if (psServer) {
		delete psServer;
		psServer = NULL;
	}

	if (psServerDande) {
		delete psServerDande;
		psServerDande = NULL;
	}

	if (psFileHub) {
		delete psFileHub;
		psFileHub = NULL;
	}

	if (psSaveXML) {
		delete psSaveXML;
		psSaveXML = NULL;
	}

	if (pcDefaultProperties) {
		delete pcDefaultProperties;
		pcDefaultProperties = NULL;
	}

	/* clean up sockets */
	WSACleanup ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cWindow::create (HINSTANCE hinstance, LPSTR title, int iWidth, int iHeight, unsigned int iStyle) {
	INITCOMMONCONTROLSEX*  psControls;
	int nButton;
	INITCOMMONCONTROLSEX sICCex;
	HBRUSH hBrush;

	MENUINFO sMenuInfo;
	WSAData sWSAData;

	TBBUTTON asButtons[16];
	cOptions*  opt;
	hInstance = hinstance;

	::FILETIME sFileTime;
	GetSystemTimeAsFileTime (&sFileTime);
	srand (sFileTime.dwLowDateTime);

	/* Create menu */
	hMainMenuTop = LoadMenu (hInstance, MAKEINTRESOURCE (IDR_MENU1));

	/* Create window */
	wndClass.hCursor = LoadCursor (NULL, IDC_ARROW);
	wndClass.hIcon = (HICON) LoadIcon (hinstance, MAKEINTRESOURCE (IDI_MAIN));
	wndClass.hIconSm = (HICON) LoadIcon (hinstance, MAKEINTRESOURCE (IDI_SMACTIVE));
	wndClass.hbrBackground = (HBRUSH) GetStockObject (NULL_BRUSH);
	wndClass.hInstance = hInstance;
	wndClass.lpfnWndProc = wndProc;
	wndClass.lpszClassName = "win";

	wndClass.cbClsExtra = NULL;
	wndClass.cbSize = sizeof (WNDCLASSEX);
	wndClass.cbWndExtra = NULL;
	wndClass.lpszMenuName = NULL;	/* MAKEINTRESOURCE
									 * (IDR_MENU1);
									 * */
	wndClass.style = CS_DBLCLKS;

	if (!(RegisterClassEx (&wndClass))) {
		return false;
	}

	hwnd = CreateWindowEx (0, "win", title, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
						   CW_USEDEFAULT, iWidth, iHeight, NULL, NULL, hInstance, (void*) this);
	//hwnd = CreateWindowEx (0, "win", title, WS_POPUPWINDOW, 0,
	//	0, GetSystemMetrics (SM_CXSCREEN), GetSystemMetrics (SM_CYSCREEN), NULL, NULL, hInstance, (void*) this);

	SetMenu (hwnd, hMainMenuTop);
	SetWindowText (hwnd, title);

	/* Initialise OpenGL */
	EnableOpenGL (hwnd, &hDC, &hRC);

	opt = cOptions::create ();
	psFileHub->AddFileRoutine ("Options", opt);

	/* Create the vm list */
	pList = new cTopologyList;
	psTabIterator = pList->startIterator ();

	pTopology = new cTopology (pList, hwnd, psFileHub, this);
	LoadNodeTextures ();
	pTopology->SetNodeTextureInfo (nNodeTextureStart, nNodeTypes);
	psSaveXML = new cSaveXML (this);
	pXMLParse = new cXMLParse (pTopology);
	psXMLPropertySet = new cXMLPropertySet (this);
	psFileHub->AddFileRoutine ("Topology", psSaveXML);
	psFileHub->AddFileRoutine ("Compose", pXMLParse->GetFileHandler ());
	psFileHub->AddFileRoutine ("Functy", pTopology->psRenderGeo->GetLoadHandler ());
	psFileHub->AddFileRoutine ("PropertySet", psXMLPropertySet);

	hBrush = GetThemeSysColorBrush (NULL, COLOR_MENU);
	sMenuInfo.cbSize = sizeof (MENUINFO);
	sMenuInfo.fMask = MIM_BACKGROUND;
	sMenuInfo.hbrBack = hBrush;
	SetMenuInfo (hMainMenuTop, &sMenuInfo);
	DeleteObject (hBrush);

	/* Window created. get window rect for control size */
	GetClientRect (hwnd, & winRect);
	GetWindowRect (hwnd, & sNotFullRect);

	pTopology->Resize (& winRect);
	GetWindowRect (hwnd, & winRect);

	/* Initialise tab control */
	psControls = new INITCOMMONCONTROLSEX;
	psControls->dwSize = sizeof (INITCOMMONCONTROLSEX);
	psControls->dwICC = ICC_TAB_CLASSES;

	InitCommonControlsEx (psControls);

	/*/
	 *  Create the vm tab list ;
	 *  vmTabList = CreateWindowEx (0, WC_TABCONTROL, "", WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE, ;
	 *  0, TAB_YOFFSET, winRect.right - 1, 30, hwnd, NULL, hInstance, NULL );
	 */
	/*
	 * if (vmTabList == NULL) ;
	 * { ;
	 * return NULL;
	 * ;
	 * }
	 */

	/*
	 * pSaveXML=new cSaveXML();
	 */
	if (pConsole == NULL) {
		pConsole = cConsole::create ();
	}

	pConsole->ConsoleInitialise (hInstance, hwnd, this);

	if (WSAStartup (MAKEWORD (1, 1), &sWSAData) != 0) {
		pConsole->ConsolePrint ("WSAStartup failed. Unable to access other machines\n");
		return 0;
	}

	SetViewTick ();

	sICCex.dwSize = sizeof (INITCOMMONCONTROLSEX);
	sICCex.dwICC = ICC_BAR_CLASSES;
	InitCommonControlsEx (&sICCex);

	nButton = 0;

	asButtons[nButton].iBitmap = 0;
	asButtons[nButton].idCommand = ID_VIEW_SELECT;
	asButtons[nButton].fsState = TBSTATE_ENABLED | TBSTATE_CHECKED;
	asButtons[nButton].fsStyle = TBSTYLE_BUTTON | TBSTYLE_CHECK;
	asButtons[nButton].dwData = 0;
	asButtons[nButton].iString = NULL;
	nButton++;

	asButtons[nButton].iBitmap = 1;
	asButtons[nButton].idCommand = ID_VIEW_CREATELINK;
	asButtons[nButton].fsState = TBSTATE_ENABLED;
	asButtons[nButton].fsStyle = TBSTYLE_BUTTON | TBSTYLE_CHECK;
	asButtons[nButton].dwData = 1;
	asButtons[nButton].iString = NULL;
	nButton++;

	asButtons[nButton].iBitmap = 2;
	asButtons[nButton].idCommand = ID_VIEW_SPIN;
	asButtons[nButton].fsState = TBSTATE_ENABLED;
	asButtons[nButton].fsStyle = TBSTYLE_BUTTON | TBSTYLE_CHECK;
	asButtons[nButton].dwData = 2;
	asButtons[nButton].iString = NULL;
	nButton++;

	asButtons[nButton].iBitmap = 3;
	asButtons[nButton].idCommand = ID_VIEW_OVERLAY;
	asButtons[nButton].fsState = TBSTATE_ENABLED | TBSTATE_CHECKED;
	asButtons[nButton].fsStyle = TBSTYLE_BUTTON | TBSTYLE_CHECK;
	asButtons[nButton].dwData = 3;
	asButtons[nButton].iString = NULL;
	nButton++;

	/*
	 * asButtons[nButton].iBitmap = 4;
	 * asButtons[nButton].idCommand = ID_VIEW_ROTATE;
	 * asButtons[nButton].fsState = TBSTATE_ENABLED;
	 * asButtons[nButton].fsStyle = TBSTYLE_BUTTON | TBSTYLE_CHECK;
	 * asButtons[nButton].dwData = 4;
	 * asButtons[nButton].iString = NULL;
	 * nButton++;
	 * ;
	 * asButtons[nButton].iBitmap = 5;
	 * asButtons[nButton].idCommand = ID_VIEW_POINTTOSELECTED;
	 * asButtons[nButton].fsState = TBSTATE_ENABLED;
	 * asButtons[nButton].fsStyle = TBSTYLE_BUTTON;
	 * asButtons[nButton].dwData = 5;
	 * asButtons[nButton].iString = NULL;
	 * nButton++;
	 */
	asButtons[nButton].iBitmap = 6;
	asButtons[nButton].idCommand = ID_VIEW_CREATE_HOSPITAL;
	asButtons[nButton].fsState = TBSTATE_ENABLED;
	asButtons[nButton].fsStyle = TBSTYLE_BUTTON | TBSTYLE_CHECK;
	asButtons[nButton].dwData = 6;
	asButtons[nButton].iString = NULL;
	nButton++;

	asButtons[nButton].iBitmap = 7;
	asButtons[nButton].idCommand = ID_VIEW_CREATE_POLICE;
	asButtons[nButton].fsState = TBSTATE_ENABLED;
	asButtons[nButton].fsStyle = TBSTYLE_BUTTON | TBSTYLE_CHECK;
	asButtons[nButton].dwData = 7;
	asButtons[nButton].iString = NULL;
	nButton++;

	asButtons[nButton].iBitmap = 8;
	asButtons[nButton].idCommand = ID_VIEW_CREATE_BANK;
	asButtons[nButton].fsState = TBSTATE_ENABLED;
	asButtons[nButton].fsStyle = TBSTYLE_BUTTON | TBSTYLE_CHECK;
	asButtons[nButton].dwData = 8;
	asButtons[nButton].iString = NULL;
	nButton++;

	asButtons[nButton].iBitmap = 9;
	asButtons[nButton].idCommand = ID_VIEW_CREATE_EVENT;
	asButtons[nButton].fsState = TBSTATE_ENABLED;
	asButtons[nButton].fsStyle = TBSTYLE_BUTTON | TBSTYLE_CHECK;
	asButtons[nButton].dwData = 9;
	asButtons[nButton].iString = NULL;
	nButton++;

	asButtons[nButton].iBitmap = 10;
	asButtons[nButton].idCommand = ID_VIEW_CREATE_MOBILE;
	asButtons[nButton].fsState = TBSTATE_ENABLED;
	asButtons[nButton].fsStyle = TBSTYLE_BUTTON | TBSTYLE_CHECK;
	asButtons[nButton].dwData = 10;
	asButtons[nButton].iString = NULL;
	nButton++;

	asButtons[nButton].iBitmap = 11;
	asButtons[nButton].idCommand = ID_VIEW_CREATE_RESCUE;
	asButtons[nButton].fsState = TBSTATE_ENABLED;
	asButtons[nButton].fsStyle = TBSTYLE_BUTTON | TBSTYLE_CHECK;
	asButtons[nButton].dwData = 11;
	asButtons[nButton].iString = NULL;
	nButton++;

	asButtons[nButton].iBitmap = 12;
	asButtons[nButton].idCommand = ID_VIEW_CREATE_FIRE;
	asButtons[nButton].fsState = TBSTATE_ENABLED;
	asButtons[nButton].fsStyle = TBSTYLE_BUTTON | TBSTYLE_CHECK;
	asButtons[nButton].dwData = 12;
	asButtons[nButton].iString = NULL;
	nButton++;

	asButtons[nButton].iBitmap = 13;
	asButtons[nButton].idCommand = ID_VIEW_CREATE_AMBULANCE;
	asButtons[nButton].fsState = TBSTATE_ENABLED;
	asButtons[nButton].fsStyle = TBSTYLE_BUTTON | TBSTYLE_CHECK;
	asButtons[nButton].dwData = 13;
	asButtons[nButton].iString = NULL;
	nButton++;

	asButtons[nButton].iBitmap = 14;
	asButtons[nButton].idCommand = ID_VIEW_CREATE_TRANSPORT;
	asButtons[nButton].fsState = TBSTATE_ENABLED;
	asButtons[nButton].fsStyle = TBSTYLE_BUTTON | TBSTYLE_CHECK;
	asButtons[nButton].dwData = 14;
	asButtons[nButton].iString = NULL;
	nButton++;

	asButtons[nButton].iBitmap = 15;
	asButtons[nButton].idCommand = ID_VIEW_CREATE_EXTERNAL;
	asButtons[nButton].fsState = TBSTATE_ENABLED;
	asButtons[nButton].fsStyle = TBSTYLE_BUTTON | TBSTYLE_CHECK;
	asButtons[nButton].dwData = 15;
	asButtons[nButton].iString = NULL;
	nButton++;

	/*
	 * gwhToolbar = CreateToolbarEx (hwnd, TBSTYLE_WRAPABLE | WS_CHILD | WS_VISIBLE |
	 * WS_BORDER | WS_CLIPSIBLINGS, IDR_TOOLBAR1, 7, hInstance, IDR_TOOLBAR1,
	 * asButtons, nButton, 16, 16, 16, 16, sizeof (TBBUTTON));
	 * if (gwhToolbar == NULL) { return NULL;
	 * } ;
	 * Create right-click menu
	 */
	hMainMenuContext = LoadMenu (hInstance, MAKEINTRESOURCE (IDR_MENU3));

	/*
	 * GetClientRect (hwnd, & sRectWin);
	 * GetWindowRect (psToolbar, & sRectTB);
	 * MoveWindow (psToolbar, 0, 16, sRectWin.right - sRectWin.left, sRectTB.bottom
	 * sRectTB.top, TRUE);
	 * ;
	 * TCITEM tie;
	 * tie.mask = TCIF_TEXT;
	 * char szTemp[100];
	 * sprintf(szTemp, "Topology");
	 * tie.pszText = szTemp;
	 * ;
	 * TabCtrl_InsertItem (vmTabList, 0, &tie);
	 * ;
	 * ShowTopologyTab ();
	 */
	if (opt->getAllowAgents ()) {
		opt->listenHandle = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) listenProc, this, 0, NULL);
	}

	if (opt->getAllowControl ()) {
		opt->listenControlHandle = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) listenControlProc, this, 0, NULL);
	}

	// For simplicity we're going to run the server straight away
	opt->listenPDAServerHandle = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) runServer, this, 0, NULL);
	opt->listenPDAServerHandle = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) runServerDande, this, 0, NULL);

	SetViewTick ();

	SetupDandelion ();

	// Load the options file
	psFileHub->Load (OPTIONS_FILENAME);

	ResourceBeep (IDR_STARTUP);

	return true;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::ToggleFullScreen () {
	if (boFullScreen) {
		SetWindowLongPtr (hwnd, GWL_STYLE, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW);
		SetWindowPos (hwnd, HWND_NOTOPMOST, sNotFullRect.left, sNotFullRect.top, sNotFullRect.right - sNotFullRect.left, sNotFullRect.bottom - sNotFullRect.top, SWP_SHOWWINDOW | SWP_FRAMECHANGED);
		SetMenu (hwnd, hMainMenuTop);
		InvalidateRect (NULL, NULL, TRUE);
		SetPriorityClass (GetCurrentProcess (), NORMAL_PRIORITY_CLASS);
		boFullScreen = false;
	}
	else {
		GetWindowRect (hwnd, & sNotFullRect);
		SetWindowLongPtr (hwnd, GWL_STYLE, WS_POPUPWINDOW);
		SetWindowPos (hwnd, HWND_TOPMOST, 0, 0, GetSystemMetrics (SM_CXSCREEN), GetSystemMetrics (SM_CYSCREEN), SWP_SHOWWINDOW | SWP_FRAMECHANGED);
		SetMenu (hwnd, NULL);
		SetPriorityClass (GetCurrentProcess (), HIGH_PRIORITY_CLASS);
		boFullScreen = true;
	}
	SetMenuTick (hwnd, ID_FULLSCREEN, boFullScreen);

	//wglDeleteContext (hRC);
	//hDC = GetDC (hwnd);
	//hRC = wglCreateContext (hDC);
	//wglMakeCurrent (hDC, hRC);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::ShowMainWindow (int nCmdShow) {
	ShowWindow (hwnd, nCmdShow);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cVM* cWindow::getCurrentVM () {
	return pConsole->pCurrentVM;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::setCurrentVM (cVM* pVM) {
	pConsole->pCurrentVM = pVM;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
LRESULT CALLBACK cWindow::wndProc (HWND window, UINT msg, WPARAM wparam, LPARAM lparam) {
	cWindow*  pWin;
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
		pWin = (cWindow*) GetWindowLong (window, GWL_USERDATA);
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
 *  void normalize (float v[3]) ;
 *  { ;
 *  GLfloat d = sqrt (v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
 *  if (d == 0.0) ;
 *  { ;
 *  return;
 *  } ;
 *  v[0] /= d;
 *  v[1] /= d;
 *  v[2] /= d;
 *  } ;
 *  void normcrossprod (float v1[3], float v2[3], float out[3]) ;
 *  { ;
 *  out[0] = v1[1] * v2[2] - v1[2] * v2[1];
 *  out[1] = v1[2] * v2[0] - v1[0] * v2[2];
 *  out[2] = v1[0] * v2[1] - v1[1] * v2[0];
 *  normalize (out);
 *  }
 =======================================================================================================================
 */
void cWindow::run () {
	BOOL quit;
	float theta1;
	float theta2;
	DWORD uTime;
	float fTimeChange;
	BOOL boUpdate;
	cTopologyNode*	psIterator;

	quit = FALSE;
	theta1 = 0.0f;
	theta2 = 0.0f;
	boUpdate = FALSE;

	/*
	 * while (GetMessage(&message, NULL, 0, 0)) ;
	 * { ;
	 * TranslateMessage(&message);
	 * DispatchMessage(&message);
	 * }
	 */
	while (!quit) {

		/* check for messages */
		if (PeekMessage (&message, NULL, 0, 0, PM_REMOVE)) {

			/* handle or dispatch messages */
			if (message.message == WM_QUIT) {
				quit = TRUE;
			} else {
				TranslateMessage (&message);
				DispatchMessage (&message);
			}
		} else {

			/* Perform idle activities/animations */
			uTime = timeGetTime ();
			fTimeChange = (float) (uTime - guPrevTimePhi);

			if (fTimeChange > 10.0f) {

				if (nCurID >= 0) {
					EventNewNode*  psData;
					char szText[EVENT_NAME_LEN];

					psData = new EventNewNode ();
					psData->nNodeID = nCurID;

					psData->fXPos = ((float)(rand () % 1000) / 1000.0f) - 0.5f;//(float) (SCREEN_CENTRE_X - (SCREEN_WIDTH / 2) + (rand () % SCREEN_WIDTH));
					psData->fYPos = ((float)(rand () % 1000) / 1000.0f) - 0.5f;//(float) (SCREEN_CENTRE_Y - (SCREEN_HEIGHT / 2) + (rand () % SCREEN_HEIGHT));
					psData->fZPos = 0.0;
					psData->nType = 0;
					psData->psWindow = this;
					_snprintf (szText, EVENT_NAME_LEN, "Create node %d", psData->nNodeID);
					pTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NOW, 0, (Event*) psData);
					nCurID = -1;
				}

				/*
				 * part of communication with client. delete node in same thread. not sure if it's
				 * still being used
				 */
				if (nCurDeleteID >= 0) {
					cTopologyNode*	psNode;
					psNode = pTopology->DecodeNodeNumber (nCurDeleteID);
					pTopology->SetSelectedNode (psNode);
					DeleteSelecteNode ();
					nCurDeleteID = -1;
				}

				/*
				 * Part of comunication with client. szNameList[0] used as a flag. ;
				 * Put here to make sure the operation of pList is within the same thread with the
				 * main ;
				 * All nodes' name and ID are concatenated in szNamelist for sending back to
				 * client
				 */
				if (szNameList[0] == 'R') {
					char szTemp[5 * 1024];
					cTopologyNode*	psNode;
					strcat (szNameList, "Name list;");

					psIterator = pList->startIterator ();
					szTemp[0] = 0;
					while (pList->isIteratorValid (psIterator)) {
						psNode = pList->getCurrentNode (psIterator);
						strcat (szNameList, psNode->GetName ());

						/*
						 * sSend = sSend+sTemp;
						 */
						sprintf (szTemp, "(%d);", psNode->getID ());
						strcat (szNameList, szTemp);

						/*
						 * delete sTemp;
						 */
						psIterator = pList->moveForward (psIterator);
					}

					szNameList[0] = 'N';	/* flag that the sznamelist is ready to send */
				}

				if (fTimeChange > 20.0f) {
					fTimeChange = 20.0f;
				}

				if (gboSpin) {

					/*
					 * pTopology->ChangeView (0.02f * sin (0.0015f * (float)uTime), 0.0015f
					 * fTimeChange, 0.0f, 0.0f);
					 */
					pTopology->ChangeView ((fTimeChange * 0.00005f), 0.0f, 0.0f, 0.0f);
				}

				SpinMomentum ();
				TranslateMomentum ();
				if (boNextRenderType) {
					boNextRenderType = !pTopology->PrepareForNextRenderType ();
					if (!boNextRenderType) {
						SetViewTick ();
					}
				}

				/* Deal with any event updates */
				boUpdate = pTopology->GetTimer ()->Update ();
				if (boUpdate) {
					pTopology->RedrawTopology ();
				}

				pTopology->Update (pTopology->GetTimer ()->GetTime ());

				guPrevTimePhi = uTime;

				/* Analyse the topology if necessary */
				if (boTopologyChanged && pXMLParse && (pXMLParse->gboCompleted)) {
					pXMLParse->ParseStart (hwnd);
					boTopologyChanged = FALSE;
				}
			}
		}
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cWindow::messageHandler (UINT msg, WPARAM wparam, LPARAM lparam) {
	cConsole*  pCon;
	cOptions*  opt;
	UINT uID;
	UINT uMouseMsg;
	HMENU hPopUpMenu;
	POINT sPoint;
	cAnalyse*  psAnalyse;
	cTopologyNode*	psIterator;
	SHORT nAlt;
	SHORT nCtrl;
	EventViewProperties * psData;
	char szText[EVENT_NAME_LEN];
	bool boRunning;

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
		case ID_FILE_NEWVIRTUALMACHINE:
			newVM (true, -1);
			return true;
			break;
		case ID_FILE_SAVE_TOPOLOGY:
			saveToXML ();
			return true;
			break;
		case ID_FILE_LOAD:
			loadFromXML ();
			return true;
			break;
		case ID_SCENE_LOAD:
			loadScene ();
			return true;
			break;
		case ID_SCENE_TRON:
			pTopology->psRenderGeo->SetScene (SCENE_TRON);
			return true;
			break;
		case ID_SCENE_JMUMAP:
			pTopology->psRenderGeo->SetScene (SCENE_JMUMAP);
			return true;
			break;
		case ID_SCENE_LANDMARKS:
			pTopology->psRenderGeo->SetScene (SCENE_LIVERPOOL);
			return true;
			break;
		case ID_SCENE_STJOHN:
			pTopology->psRenderGeo->SetScene (SCENE_STJOHN);
			return true;
			break;
		case ID_FILE_EXIT:
			cleanup ();
			return true;
			break;
		case ID_CURRENTMACHINE_LOADPROGRAM:
			loadStateFile ();
			return true;
			break;
		case ID_CURRENTMACHINE_SAVEPROGRAM:
			saveStateFile ();
			return true;
			break;
		case ID_CURRENTMACHINE_CLOSEMACHINE:
			closeVM ();
			return true;
			break;
		case ID_CURRENTMACHINE_ENDPROGRAM:
			if (pConsole->pCurrentVM->getLoaded ()) {

				/* end program */
				int response;
				response = MessageBox (hwnd, "Are you sure you want to end the program?", "End Program?",
									   MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION);
				if (response == IDNO) {
					return true;
				}

				pConsole->pCurrentVM->setRestart (true);
				pConsole->pCurrentVM->setLoaded (false);
				pConsole->pCurrentVM->ConsolePrint ("\nProgram Ended\n");
			}

			return true;
			break;
		case ID_VIEW_OPENCONSOLE:
			pCon->OpenConsole ();
			return true;
			break;
		case ID_VIEW_OPTIONS:
			DialogBoxParam (hInstance, (LPCTSTR) IDD_OPTIONS, hwnd, (DLGPROC) optionProc, NULL);
			if (opt->getAllowAgents () && (opt->listenHandle == NULL)) {
				opt->listenHandle = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) listenProc, this, 0, NULL);
			} else {
				opt->listenHandle = NULL;
			}

			if (opt->getAllowControl () && (opt->listenControlHandle == NULL)) {
				opt->listenControlHandle = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) listenControlProc, this, 0,
														 NULL);
			} else {
				opt->listenControlHandle = NULL;
			}

			SetupDandelion ();
			return true;
			break;
		case ID_VIEW_SELECT:
			geViewState = VIEWSTATE_SELECT;
			SetViewTick ();
			return true;
			break;
		case ID_VIEW_ROTATE:
			geViewState = VIEWSTATE_ROTATE;
			SetViewTick ();
			return true;
			break;
		case ID_VIEW_RUNSERVER:
			boRunning = psServer->GetRunning ();
			if (boRunning) {
				MessageBox (hwnd, "Server is already running", "MATTS Server", MB_OK);
			}
			else {
				opt->listenPDAServerHandle = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) runServer, this, 0, NULL);
			}
			boRunning = psServerDande->GetRunning ();
			if (boRunning) {
				MessageBox (hwnd, "Server (Dandelion) is already running", "MATTS Server", MB_OK);
			}
			else {
				opt->listenPDAServerHandle = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) runServerDande, this, 0, NULL);
			}
			return true;
			break;
		case ID_VIEW_STOPSERVER:
			boRunning = psServer->GetRunning ();
			if (boRunning) {
				Disconnect ();
			}
			else {
				MessageBox (hwnd, "Server is not currently running", "MATTS Server", MB_OK);
			}
			boRunning = psServerDande->GetRunning ();
			if (boRunning) {
				DisconnectDande ();
			}
			else {
				MessageBox (hwnd, "Server (Dandelion) is not currently running", "MATTS Server", MB_OK);
			}
			return true;
			break;
		case ID_VIEW_CREATE_HOSPITAL:
			geViewState = VIEWSTATE_CREATENODE;
			nNodeCreateType = pTopology->nNodeButtonNodeType;
			SetViewTick ();
			return true;
			break;
		//case ID_VIEW_CREATE_POLICE:
		//	geViewState = VIEWSTATE_CREATE_POLICE;
		//	pTopology->eNodeButtonNodeType = TYPE_POLICE;
		//	SetViewTick ();
		//	return true;
		//	break;
		//case ID_VIEW_CREATE_BANK:
		//	geViewState = VIEWSTATE_CREATE_BANK;
		//	pTopology->eNodeButtonNodeType = TYPE_BANK;
		//	SetViewTick ();
		//	return true;
		//	break;
		//case ID_VIEW_CREATE_EVENT:
		//	geViewState = VIEWSTATE_CREATE_EVENT;
		//	pTopology->eNodeButtonNodeType = TYPE_EVENT;
		//	SetViewTick ();
		//	return true;
		//	break;
		//case ID_VIEW_CREATE_MOBILE:
		//	geViewState = VIEWSTATE_CREATE_MOBILE;
		//	pTopology->eNodeButtonNodeType = TYPE_MOBILE;
		//	SetViewTick ();
		//	return true;
		//	break;
		//case ID_VIEW_CREATE_RESCUE:
		//	geViewState = VIEWSTATE_CREATE_RESCUE;
		//	pTopology->eNodeButtonNodeType = TYPE_RESCUE;
		//	SetViewTick ();
		//	return true;
		//	break;
		//case ID_VIEW_CREATE_FIRE:
		//	geViewState = VIEWSTATE_CREATE_FIRE;
		//	pTopology->eNodeButtonNodeType = TYPE_FIRE;
		//	SetViewTick ();
		//	return true;
		//	break;
		//case ID_VIEW_CREATE_AMBULANCE:
		//	geViewState = VIEWSTATE_CREATE_AMBULANCE;
		//	pTopology->eNodeButtonNodeType = TYPE_AMBULANCE;
		//	SetViewTick ();
		//	return true;
		//	break;
		//case ID_VIEW_CREATE_TRANSPORT:
		//	geViewState = VIEWSTATE_CREATE_TRANSPORT;
		//	pTopology->eNodeButtonNodeType = TYPE_TRANSPORT;
		//	SetViewTick ();
		//	return true;
		//	break;
		//case ID_VIEW_CREATE_EXTERNAL:
		//	geViewState = VIEWSTATE_CREATE_EXTERNAL;
		//	pTopology->eNodeButtonNodeType = TYPE_EXTERNAL;
		//	SetViewTick ();
		//	return true;
		//	break;
		case ID_VIEW_CREATELINK:
			geViewState = VIEWSTATE_CREATELINK;
			SetViewTick ();
			return true;
			break;
		case ID_VIEW_SPIN:
			if (opt->getRecordViewEvents ()) {
				/* Add a view property event to the event list */
				psData = new EventViewProperties ();
				psData->boSpin = !gboSpin;
				psData->boNetworkView = (pTopology->eRenderType == RENDERTYPE_NET);
				psData->boOverlay = pTopology->GetOverlayStatus ();
				psData->psWindow = this;
				if (psData->boSpin) {
					_snprintf (szText, EVENT_NAME_LEN, "Start view spin");
				}
				else {
					_snprintf (szText, EVENT_NAME_LEN, "Stop view spin");
				}
				pTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NOW, 0, psData);
			}
			else {
				gboSpin = !gboSpin;
				SetViewTick ();
			}
			return true;
			break;
		case ID_VIEW_OVERLAY:
			if (opt->getRecordViewEvents ()) {
				/* Add a view property event to the event list */
				psData = new EventViewProperties ();
				psData->boSpin = gboSpin;
				psData->boNetworkView = (pTopology->eRenderType == RENDERTYPE_NET);
				psData->boOverlay = !pTopology->GetOverlayStatus ();
				psData->psWindow = this;
				if (psData->boOverlay) {
					_snprintf (szText, EVENT_NAME_LEN, "Show information overlay");
				}
				else {
					_snprintf (szText, EVENT_NAME_LEN, "Hide information overlay");
				}
				pTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NOW, 0, psData);
			}
			else {
				pTopology->ToggleOverlayStatus ();
				SetViewTick ();
			}
			return true;
			break;
		case ID_VIEW_RENDERTYPE:
			if (opt->getRecordViewEvents () && (!boNextRenderType)) {
				/* Add a view property event to the event list */
				psData = new EventViewProperties ();
				psData->boSpin = gboSpin;
				psData->boNetworkView = !(pTopology->eRenderType == RENDERTYPE_NET);
				psData->boOverlay = pTopology->GetOverlayStatus ();
				psData->psWindow = this;
				if (psData->boNetworkView) {
					_snprintf (szText, EVENT_NAME_LEN, "Change to network render");
				}
				else {
					_snprintf (szText, EVENT_NAME_LEN, "Change to geographical render");
				}
				pTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NOW, 0, psData);
			}
			else {
				boNextRenderType = TRUE;
				SetViewTick ();
			}
			return true;
			break;
		case ID_VIEW_POINTTOSELECTED:
			pTopology->PointToSelected ();
			return true;
			break;
		case ID_HELP_ABOUT:
			DialogBoxParam (hInstance, (LPCTSTR) ID_ABOUT, hwnd, (DLGPROC) aboutProc, (LPARAM) this);
			return true;
			break;
		case ID_HELP_POLICY:
			CreateDialogParam (hInstance, (LPCTSTR) IDD_POLICY, hwnd, (DLGPROC) policyProc, (LPARAM) pList);
			return true;
			break;
		case ID_HELP_CONTENTS:

			/*
			 * WinHelp (hwnd, "MATTS.hlp", HELP_FINDER, 0);
			 */
			return true;
			break;
		case ID_CHANNELS:
			if (pTopology->GetSelectedLink () != NULL) {
				DialogBoxParam (hInstance, (LPCTSTR) IDD_CHANNELS, hwnd, (DLGPROC) channelsProc, (LPARAM) this);
			}

			return true;
			break;
		case ID_NODEPROPS:
			if (pTopology->GetSelectedNode () != NULL) {
				DialogBoxParam (hInstance, (LPCTSTR) IDD_NODEPROPS, hwnd, (DLGPROC) componentProc, (LPARAM) this);
			}

			return true;
			break;
		case ID_PROPERTIES:
			if (pTopology->GetSelectedNode () != NULL) {
				DialogBoxParam (hInstance, (LPCTSTR) IDD_NODEPROPS, hwnd, (DLGPROC) componentProc, (LPARAM) this);
			}
			else {
				if (pTopology->GetSelectedLink () != NULL) {
					DialogBoxParam (hInstance, (LPCTSTR) IDD_CHANNELS, hwnd, (DLGPROC) channelsProc, (LPARAM) this);
				}
			}
			return true;
			break;
		case ID_TOPOLOGY_ASSIGNDATA:
			if (pTopology->GetSelectedNode () != NULL) {
				DialogBoxParam (hInstance, (LPCTSTR) IDD_ASSIGNDATA, hwnd, (DLGPROC) assignDataProc, (LPARAM) this);
			}

			return true;
			break;
		case ID_TOPOLOGY_SENDDATA:
			if (pTopology->GetSelectedNode () != NULL) {
				DialogBoxParam (hInstance, (LPCTSTR) IDD_SENDDATA, hwnd, (DLGPROC) sendDataProc, (LPARAM) this);
			}

			return true;
			break;
		case ID_TOPOLOGY_MANAGEDATA:
			DialogBoxParam (hInstance, (LPCTSTR) IDD_MANAGEDATA, hwnd, (DLGPROC) manageDataProc, (LPARAM) pTopology);
			return true;
			break;
		case ID_TOPOLOGY_DELETELINK:
			DeleteSelectedLink ();
			return true;
			break;

		/* added a case for deleting selected node AA */
		case ID_TOPOLOGY_DELETENODE:
			DeleteSelecteNode ();

			/*
			 * cCons->DeleteVMConsole();
			 */
			return true;
			break;
		case ID_TOPOLOGY_PETRIFY:

			/* Freeze all agents */
			psIterator = pList->startIterator ();

			while (pList->isIteratorValid (psIterator)) {
				pList->getCurrentItem (psIterator)->threadPause (true);
				psIterator = pList->moveForward (psIterator);
			}

			SetMenuTick (hwnd, ID_TOPOLOGY_PETRIFY, true);
			return true;
			break;
		case ID_TOPOLOGY_REANIMATE:

			/* Unfreeze all agents */
			psIterator = pList->startIterator ();
			while (pList->isIteratorValid (psIterator)) {
				pList->getCurrentItem (psIterator)->threadPause (false);
				psIterator = pList->moveForward (psIterator);
			}

			SetMenuTick (hwnd, ID_TOPOLOGY_PETRIFY, false);
			return true;
			break;
		case ID_TOPOLOGY_ANALYSE:

			/*
			 * sAnalyse.AnalyseCode (pList->getCurrentItem ());
			 */
			psAnalyse = new cAnalyse (pList->getCurrentItem (psTabIterator));
			return true;
			break;
		case ID_TOPOLOGY_LOADXML:
			LoadAnalysisXMLFile ();
			return true;
			break;
		case ID_TOPOLOGY_ANALYSETOPOLOGY:
			pTopology->ResetAnalyseView ();
			if (pXMLParse->XMLFileLoaded ()) {
				pXMLParse->ParseStart (hwnd);
			} else {
				LoadAnalysisXMLFile ();
			}

			return true;
			break;
		case ID_TOPOLOGY_SHOWRESULTS:
			if (pXMLParse->gboCompleted) {
				if (opt->getDemoCycle ()) {
					geDemoCycle = DEMOCYCLE_CHECKED;
				} else {

					/*
					 * DialogBoxParam (hInstance, (LPCTSTR) IDD_CHECKRESULT, hwnd,
					 * (DLGPROC)checkresultsProc, (LPARAM)pXMLParse);
					 */
					CreateDialogParam (hInstance, (LPCTSTR) IDD_CHECKRESULT, hwnd, (DLGPROC) checkresultsProc,
									   (LPARAM) pXMLParse);
				}
			}

			return true;
			break;
		case ID_TOPOLOGY_RESET:
			pTopology->ResetAnalyseView ();
			return true;
			break;
		case ID_CURRENTMACHINE_PAUSEPROGRAM:

			/* Pause the current program */
			pConsole->pCurrentVM->threadPause (!(pConsole->pCurrentVM->threadPaused ()));
			if (!(pConsole->pCurrentVM->threadPaused ())) {
				SetMenuTick (hwnd, ID_TOPOLOGY_PETRIFY, false);
			}

			SetMenuTick (hwnd, ID_CURRENTMACHINE_PAUSEPROGRAM, (pConsole->pCurrentVM->threadPaused ()));
			return true;
			break;
		case ID_EVENTS_RESETTIME:

			/* Reset the event timeline */
			ResetTime ();
			return true;
			break;
		case ID_EVENTS_SHOWEVENTLIST:
			CreateDialogParam (hInstance, (LPCTSTR) IDD_EVENTLIST, hwnd, (DLGPROC) eventlistProc, (LPARAM) pTopology);
			return true;
			break;
		case ID_EVENTS_RESETCLEAR:
			if (MessageBox (hwnd, "Are you sure you want to clear all events and topology", "Warning", MB_YESNO) == IDYES) {
				ResetTime ();
				pTopology->ResetNextID ();
				pTopology->ClearTimerEvents ();
			}
			return true;
			break;
		case ID_MENU_SHOW:
			ShowWindow (hwnd, SW_RESTORE);
			SetForegroundWindow (hwnd);
			break;
			return true;
		case ID_MENU_ABOUT:
			DialogBoxParam (hInstance, (LPCTSTR) ID_ABOUT, hwnd, (DLGPROC) aboutProc, (LPARAM) pList);
			break;
			return true;
		case ID_MENU_EXIT:
			cleanup ();
			break;
			return true;
		case ID_PROPERTYSET:
			if (pTopology-> GetSelectedNode () != NULL) {
				pTopology->GetSelectedNode ()->PropertyRemoveAll ();
				LoadPropertySetXMLFile (pTopology->GetSelectedNode ());
				DialogBoxParam (hInstance, (LPCTSTR) IDD_NODEPROPS, hwnd, (DLGPROC) componentProc, (LPARAM) this);
			}
			else {
				if (pTopology->GetSelectedLink () != NULL) {
					pTopology->GetSelectedLink ()->PropertyRemoveAll ();
					LoadPropertySetXMLFile (pTopology->GetSelectedLink ());
					DialogBoxParam (hInstance, (LPCTSTR) IDD_CHANNELS, hwnd, (DLGPROC) channelsProc, (LPARAM) this);
				}
			}
			return true;
			break;
		}
		case ID_OPENDEFAULTPROPERTYSET:
			pcDefaultProperties->PropertyRemoveAll ();
			LoadDefaultPropertySetXMLFile ();
			return true;
			break;
		case ID_FULLSCREEN:
			ToggleFullScreen ();
			return true;
			break;
		return true;
		break;
	case WM_PAINT:
		redraw ();
		return true;
		break;
	case WM_ERASEBKGND:
		return true;
		break;
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		nAlt = GetKeyState (VK_MENU);
		nCtrl = GetAsyncKeyState (VK_CONTROL);
		if (pCon->isInput ()) {
			switch (wparam) {
			case VK_DELETE:
				pCon->setChar (8);
				return true;
				break;
			default:
				return false;
				break;
			}
		} else {
			switch (wparam) {
			case VK_LEFT:
				if (nCtrl) {
					pTopology->MoveCentre (0.0f, 0.1f, 0.0f);
				}
				else {
					pTopology->ChangeView (-DELTA_ROT, 0.0f, 0.0f, 0.0f);
				}
				return true;
				break;
			case VK_RIGHT:
				if (nCtrl) {
					pTopology->MoveCentre (0.0f, -0.1f, 0.0f);
				}
				else {
					pTopology->ChangeView (DELTA_ROT, 0.0f, 0.0f, 0.0f);
				}
				return true;
				break;
			case VK_UP:
				if (nCtrl) {
					pTopology->MoveCentre (-0.1f, 0.0f, 0.0f);
				}
				else {
					pTopology->ChangeView (0.0f, -DELTA_ROT, 0.0f, 0.0f);
				}
				return true;
				break;
			case VK_DOWN:
				if (nCtrl) {
					pTopology->MoveCentre (0.1f, 0.0f, 0.0f);
				}
				else {
					pTopology->ChangeView (0.0f, DELTA_ROT, 0.0f, 0.0f);
				}
				return true;
				break;
			case VK_OEM_COMMA:
				pTopology->ChangeView (0.0f, 0.0f, DELTA_ROT, 0.0f);
				return true;
				break;
			case VK_OEM_PERIOD:
				pTopology->ChangeView (0.0f, 0.0f, -DELTA_ROT, 0.0f);
				return true;
				break;
			case VK_OEM_MINUS:
				pTopology->ChangeView (0.0f, 0.0f, 0.0f, DELTA_ROT);
				return true;
				break;
			case VK_OEM_PLUS:
				pTopology->ChangeView (0.0f, 0.0f, 0.0f, -DELTA_ROT);
				return true;
				break;
			case VK_RETURN:
				if (nAlt & VKEY_DOWN) {
					ToggleFullScreen ();
				}
				return true;
				break;
			case 'P':
				if (pTopology->GetSelectedNode () != NULL) {
					DialogBoxParam (hInstance, (LPCTSTR) IDD_NODEPROPS, hwnd, (DLGPROC) componentProc, (LPARAM) this);
				}
				else {
					if (pTopology->GetSelectedLink () != NULL) {
						DialogBoxParam (hInstance, (LPCTSTR) IDD_CHANNELS, hwnd, (DLGPROC) channelsProc, (LPARAM) this);
					}
				}
				return true;
				break;
			case VK_DELETE:
				DeleteSelectedLink ();
				return true;
				break;
			case VK_SPACE:
				// Toggle the timer pause state
				TogglePause ();
				break;
			case '1':
				geViewState = VIEWSTATE_SELECT;
				SetViewTick ();
				return true;
				break;
			case '2':
				geViewState = VIEWSTATE_ROTATE;
				SetViewTick ();
				return true;
				break;
			case '3':
				geViewState = VIEWSTATE_CREATENODE;
				SetViewTick ();
				return true;
				break;
			case '4':
				geViewState = VIEWSTATE_CREATELINK;
				SetViewTick ();
				return true;
				break;
			case '6':
				pTopology->PointToSelected ();
				return true;
				break;
			case 'A':
				pTopology->ResetAnalyseView ();
				if (pXMLParse->XMLFileLoaded ()) {
					pXMLParse->ParseStart (hwnd);
				} else {
					LoadAnalysisXMLFile ();
				}

				return true;
				break;
			case 'R':
				pTopology->ResetAnalyseView ();
				return true;
				break;
			case 'O':
				DialogBoxParam (hInstance, (LPCTSTR) IDD_OPTIONS, hwnd, (DLGPROC) optionProc, NULL);
				if (opt->getAllowAgents () && (opt->listenHandle == NULL)) {
					opt->listenHandle = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) listenProc, this, 0, NULL);
				} else {
					opt->listenHandle = NULL;
				}

				if (opt->getAllowControl () && (opt->listenControlHandle == NULL)) {
					opt->listenControlHandle = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) listenControlProc, this,
															 0, NULL);
				} else {
					opt->listenControlHandle = NULL;
				}

				SetupDandelion ();
				return true;
				break;
			case 'D':

				/* Demo cycle */
				opt->toggleDemoCycle ();
				if (opt->getDemoCycle ()) {
					geDemoCycle = DEMOCYCLE_START;
				} else {
					geDemoCycle = DEMOCYCLE_STOPPED;
					gboSpin = FALSE;
				}

				return false;
				break;
			case 'V':

				/* Change render type view */
				boNextRenderType = TRUE;
				break;
			default:
				return false;
				break;
			}
		}

		break;
	case WM_CHAR:
		if (pCon->isInput ()) {
			pCon->setChar ((char) wparam);
		} else {
			switch ((char) wparam) {
			case '[':
				pTopology->SelectPreviousNode ();
				return true;
				break;
			case ']':
				pTopology->SelectNextNode ();
				return true;
				break;
			case '{':
				pTopology->SelectPreviousLink ();
				return true;
				break;
			case '}':
				pTopology->SelectNextLink ();
				return true;
				break;
			}
		}

		return true;
		break;
	case WM_APP_NOTIFYICON:
		uID = (UINT) wparam;
		uMouseMsg = (UINT) lparam;

		switch (uMouseMsg) {
		case WM_LBUTTONDOWN:
			if (uID == 1) {
				if (!hwnd) {
					ShowWindow (hwnd, SW_RESTORE);
					SetForegroundWindow (hwnd);
				} else {
					if (IsIconic (hwnd)) {
						ShowWindow (hwnd, SW_RESTORE);
						SetForegroundWindow (hwnd);
					} else {
						ShowWindow (hwnd, SW_RESTORE);
						SetForegroundWindow (hwnd);
					}
				}
			}

			break;
			return true;
		case WM_RBUTTONDOWN:
			SetForegroundWindow (hwnd);
			GetCursorPos (&sPoint);
			hPopUpMenu = GetSubMenu (notifyIconMenu, 0);
			SetMenuDefaultItem (hPopUpMenu, ID_MENU_SHOW, FALSE);
			TrackPopupMenuEx (hPopUpMenu, (TPM_RIGHTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON | TPM_HORIZONTAL),
							  sPoint.x, sPoint.y, hwnd, NULL);
			break;
			return true;
		}

		break;
	case WM_LBUTTONDOWN:
		boDrag = false;
		switch (geViewState) {
		case VIEWSTATE_SELECT:
			if ((wparam & MK_CONTROL) != 0) {
				LeftButtonDownCreateLink (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
			} else {
				LeftButtonDownSelectRotate (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
			}

			return true;
			break;
		case VIEWSTATE_ROTATE:
			LeftButtonDownRotate (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
			return true;
			break;
		case VIEWSTATE_CREATENODE:
			LeftButtonDownCreateNode (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam), nNodeCreateType);
			return true;
			break;
		//case VIEWSTATE_CREATE_POLICE:
		//	LeftButtonDownCreateNode (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam), TYPE_POLICE);
		//	return true;
		//	break;
		//case VIEWSTATE_CREATE_BANK:
		//	LeftButtonDownCreateNode (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam), TYPE_BANK);
		//	return true;
		//	break;
		//case VIEWSTATE_CREATE_EVENT:
		//	LeftButtonDownCreateNode (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam), TYPE_EVENT);
		//	return true;
		//	break;
		//case VIEWSTATE_CREATE_MOBILE:
		//	LeftButtonDownCreateNode (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam), TYPE_MOBILE);
		//	return true;
		//	break;
		//case VIEWSTATE_CREATE_RESCUE:
		//	LeftButtonDownCreateNode (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam), TYPE_RESCUE);
		//	return true;
		//	break;
		//case VIEWSTATE_CREATE_FIRE:
		//	LeftButtonDownCreateNode (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam), TYPE_FIRE);
		//	return true;
		//	break;
		//case VIEWSTATE_CREATE_AMBULANCE:
		//	LeftButtonDownCreateNode (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam), TYPE_AMBULANCE);
		//	return true;
		//	break;
		//case VIEWSTATE_CREATE_TRANSPORT:
		//	LeftButtonDownCreateNode (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam), TYPE_TRANSPORT);
		//	return true;
		//	break;
		//case VIEWSTATE_CREATE_EXTERNAL:
		//	LeftButtonDownCreateNode (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam), TYPE_EXTERNAL);
		//	return true;
		//	break;
		case VIEWSTATE_CREATELINK:
			if ((wparam & MK_CONTROL) != 0) {
				LeftButtonDownSelectRotate (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
			} else {
				LeftButtonDownCreateLink (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
			}

			return true;
			break;
		}

		break;
	case WM_LBUTTONUP:
		switch (geViewState) {
		case VIEWSTATE_SELECT:
			LeftButtonUp (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
			return true;
			break;
		case VIEWSTATE_ROTATE:
			LeftButtonUp (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
			return true;
			break;
		case VIEWSTATE_CREATENODE:
			LeftButtonUp (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
			return true;
			break;
		//case VIEWSTATE_CREATE_POLICE:
		//	LeftButtonUp (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
		//	return true;
		//	break;
		//case VIEWSTATE_CREATE_BANK:
		//	LeftButtonUp (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
		//	return true;
		//	break;
		//case VIEWSTATE_CREATE_EVENT:
		//	LeftButtonUp (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
		//	return true;
		//	break;
		//case VIEWSTATE_CREATE_MOBILE:
		//	LeftButtonUp (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
		//	return true;
		//	break;
		//case VIEWSTATE_CREATE_RESCUE:
		//	LeftButtonUp (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
		//	return true;
		//	break;
		//case VIEWSTATE_CREATE_FIRE:
		//	LeftButtonUp (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
		//	return true;
		//	break;
		//case VIEWSTATE_CREATE_AMBULANCE:
		//	LeftButtonUp (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
		//	return true;
		//	break;
		//case VIEWSTATE_CREATE_TRANSPORT:
		//	LeftButtonUp (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
		//	return true;
		//	break;
		//case VIEWSTATE_CREATE_EXTERNAL:
		//	LeftButtonUp (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
		//	return true;
		//	break;
		case VIEWSTATE_CREATELINK:
			LeftButtonUp (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
			return true;
			break;
		}

		break;
	case WM_MOUSEMOVE:
		switch (geViewState) {
		case VIEWSTATE_SELECT:
			MouseMove (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
			return true;
			break;
		case VIEWSTATE_ROTATE:
			MouseMove (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
			return true;
			break;
		case VIEWSTATE_CREATENODE:
			MouseMove (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
			return true;
			break;
		//case VIEWSTATE_CREATE_POLICE:
		//	MouseMove (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
		//	return true;
		//	break;
		//case VIEWSTATE_CREATE_BANK:
		//	MouseMove (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
		//	return true;
		//	break;
		//case VIEWSTATE_CREATE_EVENT:
		//	MouseMove (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
		//	return true;
		//	break;
		//case VIEWSTATE_CREATE_MOBILE:
		//	MouseMove (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
		//	return true;
		//	break;
		//case VIEWSTATE_CREATE_RESCUE:
		//	MouseMove (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
		//	return true;
		//	break;
		//case VIEWSTATE_CREATE_FIRE:
		//	MouseMove (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
		//	return true;
		//	break;
		//case VIEWSTATE_CREATE_AMBULANCE:
		//	MouseMove (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
		//	return true;
		//	break;
		//case VIEWSTATE_CREATE_TRANSPORT:
		//	MouseMove (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
		//	return true;
		//	break;
		//case VIEWSTATE_CREATE_EXTERNAL:
		//	MouseMove (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
		//	return true;
		//	break;
		case VIEWSTATE_CREATELINK:
			MouseMove (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
			return true;
			break;
		}

		break;
	case WM_MOUSEWHEEL:
		ScrollWheel (GET_WHEEL_DELTA_WPARAM (wparam));
		return true;
		break;
	case WM_RBUTTONDOWN:
		boDrag = false;
		RightButtonDownTranslate (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));

		///*
		// * SetForegroundWindow (hwnd);
		// */
		//LeftButtonDownSelect (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
		//CancelDrag ();
		//hPopUpMenu = GetSubMenu (hMainMenuContext, 0);
		//SetMenuDefaultItem (hPopUpMenu, ID_MENU_SHOW, FALSE);
		//GetWindowRect (hwnd, &winRect);
		//TrackPopupMenuEx (hPopUpMenu, (TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON | TPM_HORIZONTAL),
		//				  GET_X_LPARAM (lparam) + winRect.left + MAINRCLICKMENU_XOFFSET,
		//				  GET_Y_LPARAM (lparam) + winRect.top + MAINRCLICKMENU_YOFFSET, hwnd, NULL);
		break;
	case WM_RBUTTONUP:
		RightButtonUp (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));
		//CancelDrag ();
		break;
	case WM_LBUTTONDBLCLK:
		LeftButtonDownSelect (GET_X_LPARAM (lparam), GET_Y_LPARAM (lparam));

		/*
		 * if (pTopology->GetSelectedNode () != NULL) ;
		 * { ;
		 * DialogBoxParam (hInstance, (LPCTSTR) IDD_NODEPROPS, hwnd,
		 * (DLGPROC)componentProc, (LPARAM)this);
		 * } ;
		 * if (pTopology->GetSelectedLink () != NULL) ;
		 * { ;
		 * DialogBoxParam (hInstance, (LPCTSTR) IDD_CHANNELS, hwnd, (DLGPROC)channelsProc, (LPARAM)this);
		 * }
		 */
		CancelDrag ();
		break;
	default:
		return false;
		break;
	}

	return false;
}

/*
 =======================================================================================================================
 *  Toggle the pause state
 =======================================================================================================================
 */
void cWindow::TogglePause () {
	pTopology->GetTimer ()->TogglePause ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::SetupDandelion (void) {
	cOptions*  opt;
	string sAddress;
	int nPort;
	int nPortPos;

	opt = cOptions::create ();

	if (psDlClient) {

		/* Delete the previous client */
		delete psDlClient;
		psDlClient = NULL;
	}

	if (opt->getUseDandelion ()) {

		/* Create a new client */
		nPort = 4972;
		sAddress.assign (opt->getDandeServer ());
		nPortPos = (int) sAddress.rfind (':');
		if (nPortPos >= 0) {
			nPort = atoi (sAddress.substr (nPortPos + 1).c_str ());
			sAddress.resize (nPortPos);
		}

		psDlClient = new DlClientWinMt (sAddress.c_str (), nPort);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::SetOverlayStatus (bool boOverlay) {
	pTopology->SetOverlayStatus (boOverlay);
	SetViewTick ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::SetSpinStatus (bool boSpin) {
	gboSpin = boSpin;
	SetViewTick ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::SetNetworkViewStatus (bool boNetView) {
	if ((pTopology->eRenderType != RENDERTYPE_NET) == boNetView) {
		boNextRenderType = TRUE;
	}
	SetViewTick ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::DeleteSelectedLink (void) {
	EventDeleteLink*  psData;
	char szText[EVENT_NAME_LEN];

	if (pTopology->GetSelectedLink () != NULL) {
		psData = new EventDeleteLink ();
		psData->nNodeFromID = pTopology->GetSelectedLink ()->psLinkFrom->getID ();
		psData->nLinkID = pTopology->GetSelectedLink ()->nLinkID;
		psData->psWindow = this;
		_snprintf (szText, EVENT_NAME_LEN, "Delete link from node %d to %d", psData->nNodeFromID,
				   pTopology->GetSelectedLink ()->psLinkTo->getID ());
		pTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NOW, 0, psData);
	}
}

/*
 =======================================================================================================================
 *  Added delete links to a given selected node before deleting the node AA
 =======================================================================================================================
 */
void cWindow::DeleteLink (int nNodeFromID, int nLinkID) {
	cTopologyLink*	psLink;
	cTopologyNode*	psNodeFrom;
	cTopologyNode*	psNodeTo;
	char szFrom[TOPOLOGYNODE_NAMLEN];
	char szTo[TOPOLOGYNODE_NAMLEN];
	cTopologyNode*	psIterator;

	if (pList) {
		psIterator = pList->startIterator ();
		while ((pList->isIteratorValid (psIterator)) && (nNodeFromID != pList->getCurrentID (psIterator))) {
			psIterator = pList->moveForward (psIterator);
		}

		if (pList->isIteratorValid (psIterator)) {
			psLink = pList->getLinksOut (psIterator);
			while (psLink && (psLink->nLinkID != nLinkID)) {
				psLink = psLink->pFromNext;
			}

			if (psLink) {
				psNodeFrom = psLink->psLinkFrom;
				psNodeTo = psLink->psLinkTo;

				pTopology->DeleteLink (psLink);

				/* Notify Dandelion */
				_snprintf (szFrom, sizeof (szFrom), "%d", psNodeFrom->getID ());
				_snprintf (szTo, sizeof (szTo), "%d", psNodeTo->getID ());
				if (psDlClient) {
					psDlClient->SubLink (szFrom, szTo);
				}
			}
		}
	}

	/* Set to analyse */
	TopologyChanged ();
}

/*
 =======================================================================================================================
 *  added a function to delete selected node within the topology AA
 =======================================================================================================================
 */
void cWindow::DeleteSelecteNode (void) {
	EventDeleteNode*  psData;
	char szText[EVENT_NAME_LEN];

	if (pTopology->GetSelectedNode () != NULL) {
		psData = new EventDeleteNode ();
		psData->nNodeID = pTopology->GetSelectedNode ()->getID ();
		psData->psWindow = this;
		_snprintf (szText, EVENT_NAME_LEN, "Delete node %d", psData->nNodeID);
		pTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NOW, 0, psData);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::DeleteNode (int nNodeID) {
	char szName[TOPOLOGYNODE_NAMLEN];
	cTopologyNode*	psIterator;

	if (pList) {
		psIterator = pList->startIterator ();
		while (pList->isIteratorValid (psIterator)) {

			/* Check if nNodeID = pList->getCurrentID so as to detect the intended node */
			if (nNodeID == pList->getCurrentID (psIterator)) {
				psIterator = pList->removeCurrentItem (psIterator);
				pTopology->SetSelectedNode (NULL);
				pTopology->SetSelectedLink (NULL);

				/*
				 * cCons->DeleteVMConsole();
				 * Notify Dandelion
				 */
				_snprintf (szName, sizeof (szName), "%d", nNodeID);
				if (psDlClient) {
					psDlClient->SubNode (szName);
				}
			}

			psIterator = pList->moveForward (psIterator);
		}

		pTopology->SetSelectedNode (NULL);
		pTopology->RedrawTopology ();
	}

	/* Set to analyse */
	TopologyChanged ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::resize (WPARAM wparam, LPARAM lparam) {
	cOptions*  opt;

	/*
	 * resize tabs ;
	 * window created. get window rect for control size
	 */
	GetClientRect (hwnd, &winRect);
	if (pTopology) {
		pTopology->Resize (&winRect);
	}
	GetWindowRect (hwnd, &winRect);

	SetWindowPos (gwhToolbar, NULL, NULL, NULL, winRect.right - 1, 30, SWP_NOMOVE);

	opt = cOptions::create ();

	if ((wparam == SIZE_MINIMIZED) && (opt->getAllowMinimise ())) {
		NOTIFYICONDATA sIconData;
		HICON hTrayIcon;
		char temp[100];

		/* minimize to tray */
		notifyIconMenu = NULL;
		trayActive = true;
		if (opt->getAllowAgents ()) {
			sprintf (temp, "MATTS v%.2f (Accepting Incoming Agents)", VERSION);
			hTrayIcon = LoadIcon (hInstance, MAKEINTRESOURCE (IDI_SMACTIVE));
		} else {
			sprintf (temp, "MATTS v%.2f (Standalone)", VERSION);
			hTrayIcon = LoadIcon (hInstance, MAKEINTRESOURCE (IDI_SMINACTIVE));
		}

		sIconData.cbSize = sizeof (NOTIFYICONDATA);
		sIconData.hWnd = hwnd;
		sIconData.uID = 1;
		sIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		sIconData.uCallbackMessage = WM_APP_NOTIFYICON;
		sIconData.hIcon = hTrayIcon;
		strncpy (sIconData.szTip, temp, 64);
		Shell_NotifyIcon (NIM_ADD, &sIconData);
		notifyIconMenu = LoadMenu (hInstance, MAKEINTRESOURCE (IDR_MENU2));

		if (hTrayIcon) {
			DestroyIcon (hTrayIcon);
		}

		/* remove from taskbar */
		ShowWindow (hwnd, SW_HIDE);

		/*
		 * SetWindowLongPtr(hwnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
		 */
	}

	if ((wparam == SIZE_RESTORED) && (trayActive)) {
		NOTIFYICONDATA sIconData;

		/* remove tray icon */
		trayActive = false;
		sIconData.hWnd = hwnd;
		sIconData.uID = 1;
		sIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		sIconData.uCallbackMessage = WM_APP_NOTIFYICON;
		sIconData.hIcon = NULL;
		Shell_NotifyIcon (NIM_DELETE, &sIconData);

		/*
		 * reshow taskbar entry ;
		 * SetWindowLongPtr(hwnd, GWL_EXSTYLE, NULL);
		 */
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::newVM (bool boSetAsActive, int nNodeID) {
	char szText[256];
	char szName[1024];
	struct hostent*	 psHost;
	int nReturn;
	int nHostCount;

	/* Create VM */
	cOptions*  temp;
	temp = cOptions::create ();

	if (pList->getCount () != temp->getMachineLimit ()) {
		cOptions*  opt;
		if ((boSetAsActive) && (pConsole->pCurrentVM != NULL)) {
			pConsole->pCurrentVM->setActive (false);
		}

		psTabIterator = pList->newNode (nNodeID);
		pConsole->pCurrentVM = pList->getCurrentItem (psTabIterator);

		if (!pConsole->NewVMConsole (boSetAsActive)) {
			return ;
		}

		sprintf (szText, "    MATTS Version %.2f\n", VERSION);
		pConsole->pCurrentVM->ConsolePrint ("-------------------------\n");
		pConsole->pCurrentVM->ConsolePrint (szText);
		pConsole->pCurrentVM->ConsolePrint (" Host IP: ");

		nReturn = 0;
		if (gethostname (szName, sizeof (szName)) == 0) {
			psHost = gethostbyname (szName);
			if (psHost) {
				nHostCount = 0;
				while (psHost->h_addr_list[nHostCount] != 0) {
					pConsole->pCurrentVM->ConsolePrint (inet_ntoa (*((struct in_addr*) psHost->h_addr_list[nHostCount])));
					pConsole->pCurrentVM->ConsolePrint ("\n");
					nHostCount++;
				}
			}
		}

		sprintf (szText, " Node %d\n", pList->getCurrentID (psTabIterator));
		pConsole->pCurrentVM->ConsolePrint (szText);

		pConsole->pCurrentVM->ConsolePrint ("-------------------------\n\n");
		opt = cOptions::create ();
		if (opt->getAllowAgents ()) {
			pConsole->pCurrentVM->ConsolePrint ("Ready to receive agent\n\n");
		}
	} else {
		MessageBox (hwnd,
					"You Already Have the Maximum Limit of Virtual Machines. To Add More, Change the Limit Options", "Maximum VM Limit Reached",
					MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::closeVM () {
	cTopologyNode*	psIterator;

	/* check if vm running a prog */
	bool bLoaded;
	HANDLE temp;
	bLoaded = pConsole->pCurrentVM->getLoaded ();

	if (bLoaded == true) {

		/* warn */
		int answer;
		answer = MessageBox (hwnd, "The Machine is currently running a program. Are you sure you want to close it?",
							 "Close Machine?", MB_YESNO | MB_ICONQUESTION | MB_APPLMODAL);

		if (answer == IDNO) {
			return ;
		}
	}

	/* close thread */
	temp = pConsole->pCurrentVM->getHandle ();

	pConsole->pCurrentVM->setRestart (true);
	pConsole->pCurrentVM->setLoaded (false);
	pConsole->pCurrentVM->threadQuit = true;

	WaitForSingleObject (temp, INFINITE);
	CloseHandle (temp);

	/* delete VM */
	psTabIterator = pList->removeCurrentItem (psTabIterator);
	pConsole->pCurrentVM = pList->getCurrentItem (psTabIterator);

	/* delete Tab for VM */
	pConsole->DeleteVMConsole ();

	psIterator = pList->startIterator ();
	while ((pList->getCurrentID (psIterator) != pConsole->activeID) && (pList->isIteratorValid (psIterator))) {
		psIterator = pList->moveForward (psIterator);
	}

	pConsole->pCurrentVM = pList->getCurrentItem (psIterator);

	if (pConsole->pCurrentVM == NULL) {
		EnableMenuItem (GetMenu (hwnd), ID_CURRENTMACHINE_LOADPROGRAM, (MF_BYCOMMAND | MF_GRAYED));
		EnableMenuItem (GetMenu (hwnd), ID_CURRENTMACHINE_SAVEPROGRAM, (MF_BYCOMMAND | MF_GRAYED));
		EnableMenuItem (GetMenu (hwnd), ID_CURRENTMACHINE_CLEARCONSOLE, (MF_BYCOMMAND | MF_GRAYED));
		EnableMenuItem (GetMenu (hwnd), ID_CURRENTMACHINE_CLOSEMACHINE, (MF_BYCOMMAND | MF_GRAYED));
		EnableMenuItem (GetMenu (hwnd), ID_CURRENTMACHINE_ENDPROGRAM, (MF_BYCOMMAND | MF_GRAYED));
		pConsole->setVM (NULL);
		pConsole->clear ();
		pConsole->ConsolePrint ("No Active Machines");
	} else {
		pConsole->setVM (pConsole->pCurrentVM);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::redraw () {
	PAINTSTRUCT ps;
	HDC hdc;

	hdc = BeginPaint (hwnd, &ps);

	if (!boFreezeRedraw) {
		if (pTopology != NULL) {
			pTopology->PlotMain (hdc);
		}
	}

	EndPaint (hwnd, &ps);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::cleanup () {
	cTopologyNode*	psIterator;

	int i;
	bool running;

	cOptions*  opt;

	/* check if any prog's running */
	psIterator = pList->startIterator ();
	i = pList->getCount ();
	running = false;

	for (int x = 0; x < i; x++) {
		cVM*  tempvm;
		tempvm = pList->getCurrentItem (psIterator);
		if (tempvm->getLoaded ()) {
			running = true;
			break;
		}
	}

	if (running) {

		/* at least 1 vm is running a prog */
		int response;
		response = MessageBox (hwnd, "One or more programs are running. Are you sure you want to exit?", "Exit?",
							   MB_ICONQUESTION | MB_YESNO | MB_APPLMODAL);
		if (response == IDNO) {

			/* dont quit */
			return ;
		}
	}

	/* Write options */
	opt = cOptions::create ();
	//opt->writeOptions ();
	psFileHub->Save (OPTIONS_FILENAME);
	delete opt;

	/* clean up taskbar */
	if (trayActive) {
		NOTIFYICONDATA sIconData;
		sIconData.cbSize = sizeof (NOTIFYICONDATA);
		sIconData.hWnd = hwnd;
		sIconData.uID = 1;
		sIconData.uFlags = 0;
		Shell_NotifyIcon (NIM_DELETE, &sIconData);
		DestroyMenu (notifyIconMenu);
	}

	/* Clean up some other stuff */
	DestroyMenu (hMainMenuContext);

	WSACleanup ();
	PostQuitMessage (0);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::loadStateFile () {
	char szSelected[_MAX_PATH];
	char szFilter[255];
	OPENFILENAME sOfn;
	char*  szConvert;
	char szExt[32];
	BOOL boOkay;
	char szTitle[260];

	/* check if running */
	if (pConsole->pCurrentVM->getLoaded ()) {
		int response;
		response = MessageBox (hwnd,
							   "The Current VM is already running a program. Are you sure you want to load a new one?",
							   "Load Program?", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION);
		if (response == IDNO) {
			return ;
		}
	}

	/* Open the Save As dialogue box */
	ZeroMemory (&sOfn, sizeof (OPENFILENAME));

	sOfn.lStructSize = sizeof (OPENFILENAME);
	sOfn.hwndOwner = hwnd;
	sOfn.hInstance = 0;

	strcpy (szFilter, "dat files (*.dat)\n*.dat\nAll files (*.*)\n*.*\n\n");
	szConvert = strchr (szFilter, '\n');
	while (szConvert) {
		*szConvert = '\0';
		szConvert = strchr (szConvert + 1, '\n');
	}

	sOfn.lpstrFilter = szFilter;

	sOfn.lpstrCustomFilter = NULL;
	sOfn.nMaxCustFilter = 0;
	sOfn.nFilterIndex = 0;
	strcpy (szSelected, "");
	sOfn.lpstrFile = szSelected;
	sOfn.nMaxFile = sizeof (szSelected);
	sOfn.lpstrFileTitle = NULL;
	sOfn.nMaxFileTitle = 0;
	sOfn.lpstrInitialDir = NULL;
	sOfn.lpstrTitle = NULL;

	sOfn.Flags = (OFN_ENABLESIZING | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR);

	strcpy (szExt, "dat");
	sOfn.lpstrDefExt = szExt;

	boOkay = GetOpenFileName (&sOfn);

	if (boOkay) {
		HANDLE tempHandle;
		int x;
		int y;

		/* Need to remove the path so we can just display filename */
		szTitle[0] = 0;
		x = sizeof (szSelected);
		y = 0;

		/* Work back */
		while (szSelected[x] <= 0)
			x--;

		/* Now found end of text */
		while (szSelected[x] != '\\') {
			x--;
		}

		x++;

		/* x now == the position of the final slash */
		while ((szSelected[x] != 0) && (szSelected[x] != '.')) {
			szTitle[y] = szSelected[x];
			y++;
			x++;
		}

		szTitle[y] = 0;

		pConsole->SetTabName (szTitle);
		pConsole->pCurrentVM->loadExecute (szSelected);

		tempHandle = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) threadProc, pConsole->pCurrentVM, 0, NULL);

		pConsole->pCurrentVM->setThreadHandle (tempHandle);

		pConsole->pCurrentVM->threadPause (true);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::saveStateFile () {
	char szSelected[_MAX_PATH];
	char szFilter[255];
	OPENFILENAME sOfn;
	char*  szConvert;
	char szExt[32];
	BOOL boOkay;

	/* Open the Save As dialogue box */
	ZeroMemory (&sOfn, sizeof (OPENFILENAME));

	sOfn.lStructSize = sizeof (OPENFILENAME);
	sOfn.hwndOwner = hwnd;
	sOfn.hInstance = 0;

	strcpy (szFilter, "dat files (*.dat)\n*.dat\nAll files (*.*)\n*.*\n\n");
	szConvert = strchr (szFilter, '\n');
	while (szConvert) {
		*szConvert = '\0';
		szConvert = strchr (szConvert + 1, '\n');
	}

	sOfn.lpstrFilter = szFilter;
	sOfn.lpstrCustomFilter = NULL;
	sOfn.nMaxCustFilter = 0;
	sOfn.nFilterIndex = 0;
	strcpy (szSelected, "");
	sOfn.lpstrFile = szSelected;
	sOfn.nMaxFile = sizeof (szSelected);
	sOfn.lpstrFileTitle = NULL;
	sOfn.nMaxFileTitle = 0;
	sOfn.lpstrInitialDir = NULL;
	sOfn.lpstrTitle = NULL;

	sOfn.Flags = (OFN_ENABLESIZING | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR);

	strcpy (szExt, "dat");
	sOfn.lpstrDefExt = szExt;

	boOkay = GetSaveFileName (&sOfn);

	if (boOkay) {
		pConsole->pCurrentVM->saveExecutingProgram (szSelected);
	}
}


/*
 =======================================================================================================================
 *  Load the default Property Set XML file
 =======================================================================================================================
 */
void cWindow::LoadDefaultPropertySetXMLFile () {
	LoadPropertySetXMLFile (pcDefaultProperties);
}

/*
 =======================================================================================================================
 *  Load a Property Set XML file
 =======================================================================================================================
 */
void cWindow::LoadPropertySetXMLFile (cProperties * pcDefaultNodeLink) {
	char szSelected[_MAX_PATH];
	char szFilter[255];
	OPENFILENAME sOfn;
	char*  szConvert;
	char szExt[32];
	BOOL boOkay;
	cPropertySet*  pcProperties;

	/* Open the Load dialogue box */
	ZeroMemory (&sOfn, sizeof (OPENFILENAME));

	sOfn.lStructSize = sizeof (OPENFILENAME);
	sOfn.hwndOwner = hwnd;
	sOfn.hInstance = 0;

	strcpy (szFilter, "xml files (*.xml)\n*.xml\nAll files (*.*)\n*.*\n\n");
	szConvert = strchr (szFilter, '\n');
	while (szConvert) {
		*szConvert = '\0';
		szConvert = strchr (szConvert + 1, '\n');
	}

	sOfn.lpstrFilter = szFilter;

	sOfn.lpstrCustomFilter = NULL;
	sOfn.nMaxCustFilter = 0;
	sOfn.nFilterIndex = 0;
	strcpy (szSelected, "");
	sOfn.lpstrFile = szSelected;
	sOfn.nMaxFile = sizeof (szSelected);
	sOfn.lpstrFileTitle = NULL;
	sOfn.nMaxFileTitle = 0;
	sOfn.lpstrInitialDir = NULL;
	sOfn.lpstrTitle = NULL;

	sOfn.Flags = (OFN_ENABLESIZING | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR);

	strcpy (szExt, "xml");
	sOfn.lpstrDefExt = szExt;

	boOkay = GetOpenFileName (&sOfn);

	if (boOkay) {
		// Create a new Property Set loader object
		pcProperties = new cPropertySet (this);

		// Set the default node or link (in case the information's not in the XML file)
		pcProperties->ChangeDefaults (pcDefaultNodeLink);

		// Load the Property Set file
		pcProperties->LoadXML (szSelected);

		// Delete the Property Set loader object
		delete pcProperties;

		/* Set to analyse */
		TopologyChanged ();
	}
}

/*
 =======================================================================================================================
 *  Load an analysis XML file
 =======================================================================================================================
 */
void cWindow::LoadAnalysisXMLFile () {
	char szSelected[_MAX_PATH];
	char szFilter[255];
	OPENFILENAME sOfn;
	char*  szConvert;
	char szExt[32];
	BOOL boOkay;

	/* Open the Load dialogue box */
	ZeroMemory (&sOfn, sizeof (OPENFILENAME));

	sOfn.lStructSize = sizeof (OPENFILENAME);
	sOfn.hwndOwner = hwnd;
	sOfn.hInstance = 0;

	strcpy (szFilter, "xml files (*.xml)\n*.xml\nAll files (*.*)\n*.*\n\n");
	szConvert = strchr (szFilter, '\n');
	while (szConvert) {
		*szConvert = '\0';
		szConvert = strchr (szConvert + 1, '\n');
	}

	sOfn.lpstrFilter = szFilter;

	sOfn.lpstrCustomFilter = NULL;
	sOfn.nMaxCustFilter = 0;
	sOfn.nFilterIndex = 0;
	strcpy (szSelected, "");
	sOfn.lpstrFile = szSelected;
	sOfn.nMaxFile = sizeof (szSelected);
	sOfn.lpstrFileTitle = NULL;
	sOfn.nMaxFileTitle = 0;
	sOfn.lpstrInitialDir = NULL;
	sOfn.lpstrTitle = NULL;

	sOfn.Flags = (OFN_ENABLESIZING | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR);

	strcpy (szExt, "xml");
	sOfn.lpstrDefExt = szExt;

	boOkay = GetOpenFileName (&sOfn);

	if (boOkay) {
		pXMLParse->LoadXMLFile (szSelected);

		/* Set to analyse */
		TopologyChanged ();
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
LRESULT CALLBACK cWindow::optionProc (HWND window, UINT msg, WPARAM wparam, LPARAM lparam) {
	cOptions*  options;
	char temp[OPTIONS_STRING_MAX];

	options = NULL;

	switch (msg) {
	case WM_INITDIALOG:
		options = cOptions::create ();
		sprintf (temp, "%d", options->getMachineLimit ());
		SetDlgItemText (window, IDC_MACNUM, temp);
		sprintf (temp, "%ld", options->getStackLimit () / 1024 + 1);
		SetDlgItemText (window, IDC_STACK, temp);

		if (options->getAllowMinimise ()) {
			CheckDlgButton (window, IDC_MIN, BST_CHECKED);
		}

		if (options->getAllowAgents ()) {
			CheckDlgButton (window, IDC_ALLOW, BST_CHECKED);
		}

		if (options->getAutoCreate ()) {
			CheckDlgButton (window, IDC_CREATE, BST_CHECKED);
		}

		if (options->getAllowControl ()) {
			CheckDlgButton (window, IDC_CONTROL, BST_CHECKED);
		}

		sprintf (temp, "%d", options->getSelectSpeed ());
		SetDlgItemText (window, IDC_SELSPEED, temp);
		sprintf (temp, "%d", options->getColourSpeed ());
		SetDlgItemText (window, IDC_COLSPEED, temp);
		sprintf (temp, "%g", options->getRotateHalfLife ());
		SetDlgItemText (window, IDC_HALFLIFE, temp);

		if (options->getRotateSelected ()) {
			CheckDlgButton (window, IDC_ROTATESEL, BST_CHECKED);
		}

		if (options->getSoundVolume () > 0.0f) {
			CheckDlgButton (window, IDC_SOUND, BST_CHECKED);
		}

		if (options->getScreenShotMode ()) {
			CheckDlgButton (window, IDC_SCREENSHOT, BST_CHECKED);
		}

		if (options->getUseDandelion ()) {
			CheckDlgButton (window, IDC_DANDELION, BST_CHECKED);
			EnableWindow (GetDlgItem (window, IDC_DANDESERVER), TRUE);
		}

		if (options->getRecordViewEvents ()) {
			CheckDlgButton (window, IDC_VIEWEVENTS, BST_CHECKED);
		}

		SetDlgItemText (window, IDC_DANDESERVER, options->getDandeServer ());
		return true;
		break;
	case WM_COMMAND:
		if (LOWORD (wparam) == IDOK) {
			float tempFloat;
			long int tempNum2;
			int tempNum;

			options = cOptions::create ();
			GetDlgItemText (window, IDC_MACNUM, temp, sizeof (temp));
			tempNum = atoi (temp);
			if ((tempNum > 100) || (tempNum < 1)) {
				sprintf (temp, "%d", 10);
				SetDlgItemText (window, IDC_MACNUM, temp);
				return true;
				break;
			}

			/* check limit */
			GetDlgItemText (window, IDC_STACK, temp, sizeof (temp));
			tempNum2 = atoi (temp);

			if ((tempNum2 > 100) || (tempNum2 < 1)) {
				sprintf (temp, "%d", 32);
				SetDlgItemText (window, IDC_STACK, temp);
				return true;
				break;
			}

			/*
			 * check stack ;
			 * ok, set options
			 */
			options->setMachineLimit (tempNum);
			options->setStackLimit (tempNum2 * 1024 + 1);

			if (IsDlgButtonChecked (window, IDC_MIN) == BST_CHECKED) {
				options->setAllowMinimise (true);
			} else {
				options->setAllowMinimise (false);
			}

			if (IsDlgButtonChecked (window, IDC_ALLOW) == BST_CHECKED) {
				options->setAllowAgents (true);
			} else {
				options->setAllowAgents (false);
			}

			if (IsDlgButtonChecked (window, IDC_CREATE) == BST_CHECKED) {
				options->setAutoCreate (true);
			} else {
				options->setAutoCreate (false);
			}

			if (IsDlgButtonChecked (window, IDC_CONTROL) == BST_CHECKED) {
				options->setAllowControl (true);
			} else {
				options->setAllowControl (false);
			}

			GetDlgItemText (window, IDC_SELSPEED, temp, sizeof (temp));
			tempNum = atoi (temp);
			options->setSelectSpeed (tempNum);

			GetDlgItemText (window, IDC_COLSPEED, temp, sizeof (temp));
			tempNum = atoi (temp);
			options->setColourSpeed (tempNum);

			GetDlgItemText (window, IDC_HALFLIFE, temp, sizeof (temp));
			tempFloat = (float) atof (temp);

			options->setRotateHalfLife (tempFloat);

			if (IsDlgButtonChecked (window, IDC_ROTATESEL) == BST_CHECKED) {
				options->setRotateSelected (true);
			} else {
				options->setRotateSelected (false);
			}

			if (IsDlgButtonChecked (window, IDC_SOUND) == BST_CHECKED) {
				options->setSoundVolume (1.0f);
			} else {
				options->setSoundVolume (0.0f);
			}

			if (IsDlgButtonChecked (window, IDC_SCREENSHOT) == BST_CHECKED) {
				options->setScreenShotMode (true);
			} else {
				options->setScreenShotMode (false);
			}

			if (IsDlgButtonChecked (window, IDC_DANDELION) == BST_CHECKED) {
				options->setUseDandelion (true);
			} else {
				options->setUseDandelion (false);
			}

			GetDlgItemText (window, IDC_DANDESERVER, temp, sizeof (temp));
			options->setDandeServer (temp);

			if (IsDlgButtonChecked (window, IDC_VIEWEVENTS) == BST_CHECKED) {
				options->setRecordViewEvents (true);
			} else {
				options->setRecordViewEvents (false);
			}

			EndDialog (window, LOWORD (wparam));
			return true;
			break;
		}

		if (LOWORD (wparam) == IDCANCEL) {
			EndDialog (window, LOWORD (wparam));
		}

		if (LOWORD (wparam) == IDC_DANDELION) {
			if (IsDlgButtonChecked (window, IDC_DANDELION) == BST_CHECKED) {
				EnableWindow (GetDlgItem (window, IDC_DANDESERVER), TRUE);
			} else {
				EnableWindow (GetDlgItem (window, IDC_DANDESERVER), FALSE);
			}
		}

		return true;
		break;
	case WM_CLOSE:
		EndDialog (window, 0);
		return true;
		break;
	}

	return false;	/* DefWindowProc(window, msg, wparam,
					 * lparam);
					 * */
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
LRESULT CALLBACK cWindow::aboutProc (HWND window, UINT msg, WPARAM wparam, LPARAM lparam) {
	cList*	psList;
	cVM*  vm;
	long int numVM;
	cTopologyNode*	psIterator;
	cWindow*  psWindow;

	psList = NULL;
	vm = NULL;

	psWindow = (cWindow*) lparam;

	switch (msg) {
	case WM_INITDIALOG:
		MEMORYSTATUS memStat;

		DWORD vmMem;
		psList = psWindow->pList;
		vm = psList->getCurrentItem (psWindow->psTabIterator);

		char temp[60];
		GlobalMemoryStatus (&memStat);
		SetWindowText (window, "About");
		SetDlgItemText (window, IDC_ABOUT, "MATTS - Mobile Agent Topology Test System");
		sprintf (temp, "Version %.2f", VERSION);
		SetDlgItemText (window, IDC_VERSION, temp);
		SetDlgItemText (window, IDC_DESC, "Copyright Liverpool John Moore's University 2004-2006");
		SetDlgItemText (window, IDC_SYSTEM, "System Resources");
		sprintf (temp, "Total Physical Memory: \t\t%d KB", (memStat.dwTotalPhys / 1024));
		SetDlgItemText (window, IDC_TOTALMEM, temp);
		sprintf (temp, "Avaliable Physical Memory: \t%d KB", (memStat.dwAvailPhys / 1024));
		SetDlgItemText (window, IDC_MEMFREE, temp);
		sprintf (temp, "Number of Running Virtual Machines: \t%d", psList->getCount ());
		SetDlgItemText (window, IDC_NUMMACHINES, temp);

		numVM = psList->getCount ();
		psIterator = psList->startIterator ();
		vmMem = 0;

		for (int x = 0; x < numVM; x++) {
			cVM*  vm;
			vm = psList->getCurrentItem (psIterator);
			vmMem += sizeof (cVM);
			vmMem += vm->getMemSize ();
			psIterator = psList->moveForward (psIterator);
		}

		sprintf (temp, "Allocated VM Memory: \t%d KB", (vmMem / 1024));
		SetDlgItemText (window, IDC_MACHINEMEM, temp);
		SetDlgItemText (window, IDC_MACHINES, "Virtual Machine Information");
		return true;
		break;
	case WM_COMMAND:
		if (LOWORD (wparam) == IDOK || LOWORD (wparam) == IDCANCEL) {
			EndDialog (window, LOWORD (wparam));
		}

		return true;
		break;
	case WM_CLOSE:
		EndDialog (window, 0);
		return true;
		break;
	}

	return false;	/* DefWindowProc(window, msg, wparam,
					 * lparam);
					 * */
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
LRESULT CALLBACK cWindow::policyProc (HWND window, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
	case WM_INITDIALOG:
		SetWindowText (window, "Policy");
		SetDlgItemText (window, IDC_POLICY2,
						"Only node satisfies following criteria allowed to have connection with externals:\n(SL=Sensitivity Level ES=Encryption Strength SS=Staff Skills)\n1. Have either Firewall or IDS running\n2. SL=0; ES stronger than TDES-168; SS at least High or\n		SL=1; ES stronger than TDES-168; SS at least High or\n		SL=2; ES stronger than TDES-168; SS at least Mid or\n		SL=3; ES stronger than RC2-128; SS at least Mid or\n		SL=4; ES stronger than RC2-128; SS at least Mid or\n		SL=5; ES stronger than DES-56; SS at least Mid or\n		SL=6; ES stronger than DES-56; SS at least Mid or\n		SL=7; ES stronger than DES-56; SS at least Low or\n		SL=8; ES stronger than WEP-114; SS at least Low or\n		SL=9; ES stronger than WEP-114; SS at least Low");
		return true;
		break;
	case WM_COMMAND:
		if (LOWORD (wparam) == IDOK || LOWORD (wparam) == IDCANCEL) {
			EndDialog (window, LOWORD (wparam));
		}

		return true;
		break;
	case WM_CLOSE:
		EndDialog (window, 0);
		return true;
		break;
	}

	return false;	/* DefWindowProc(window, msg, wparam,
					 * lparam);
					 * */
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
LRESULT CALLBACK cWindow::assignDataProc (HWND window, UINT msg, WPARAM wparam, LPARAM lparam) {
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	/*
	 * Prepare string for use with FindFile functions. First, ;
	 * copy the string to a buffer, then append '\*' to the ;
	 * directory name.
	 */
	LPSTR szDir;
	char szTemp[65535];
	static cWindow * psWindow = NULL;
	static cTopology * pTopology = NULL;
	char*  szFileName;
	char szTemp2[256];
	EventAssignData*  psData;
	EventRemoveData*  psRemoveData;
	char szText[EVENT_NAME_LEN];

	hFind = INVALID_HANDLE_VALUE;
	szDir = (LPSTR) malloc (1024);
	strncpy (szDir, "Data", 1024);
	strncat (szDir, "\\*", 1024);
	switch (msg) {
	case WM_INITDIALOG:
		psWindow = (cWindow *)lparam;
		pTopology = psWindow->pTopology;

		/* copy assigned data to list box */
		strcpy (szTemp, pTopology->GetSelectedNode ()->szData);

		szFileName = strtok (szTemp, ";");

		while (szFileName != NULL) {
			ListBox_AddString (GetDlgItem (window, IDC_ASSIGNEDDATA), szFileName);
			szFileName = strtok (NULL, ";");
		}

		/* Find the first file in the directory. */
		strcpy (szTemp, pTopology->GetSelectedNode ()->szData);
		hFind = FindFirstFile (szDir, &FindFileData);
		if (hFind == INVALID_HANDLE_VALUE) {
			printf ("Invalid file handle. Error is %u.\n", GetLastError ());
			return (-1);
		} else {

			/* exclude the default "." and ".." directory */
			if ((strstr (szTemp, FindFileData.cFileName) == NULL) && (strcmp (FindFileData.cFileName, ".") != 0)
			&& (strcmp (FindFileData.cFileName, "..") != 0)) {
				ListBox_AddString (GetDlgItem (window, IDC_AVAILABLEDATA), FindFileData.cFileName);
			}

			/* List all the other files in the directory. */
			while (FindNextFile (hFind, &FindFileData) != 0) {
				if ((strstr (szTemp, FindFileData.cFileName) == NULL) && (strcmp (FindFileData.cFileName, ".") != 0)
				&& (strcmp (FindFileData.cFileName, "..") != 0)) {
					ListBox_AddString (GetDlgItem (window, IDC_AVAILABLEDATA), FindFileData.cFileName);
				}
			}
		}

		return true;
		break;
	case WM_COMMAND:
		switch (LOWORD (wparam)) {
		case IDOK:
			EndDialog (window, LOWORD (wparam));
			return true;
			break;
		case IDCANCEL:
			EndDialog (window, LOWORD (wparam));
			return true;
			break;
		case IDC_ADDDATA:	/* when add button clicked */
			if (ListBox_GetCurSel (GetDlgItem (window, IDC_AVAILABLEDATA)) != LB_ERR) {
				psData = new EventAssignData ();
				psData->nNodeID = pTopology->GetSelectedNode ()->getID ();

				/* move the file name from available to assigned */
				ListBox_GetText (GetDlgItem (window, IDC_AVAILABLEDATA),
								 ListBox_GetCurSel (GetDlgItem (window, IDC_AVAILABLEDATA)), szTemp2);
				ListBox_AddString (GetDlgItem (window, IDC_ASSIGNEDDATA), szTemp2);
				ListBox_DeleteString (GetDlgItem (window, IDC_AVAILABLEDATA),
									  ListBox_GetCurSel (GetDlgItem (window, IDC_AVAILABLEDATA)));

				/* attach the file to the node */
				psData->szDataName = (char*) malloc (strlen (szTemp2) + 1);
				if (psData->szDataName) {
					strcpy (psData->szDataName, szTemp2);
				}

				psData->psWindow = psWindow;
				_snprintf (szText, EVENT_NAME_LEN, "Assign data %s to node %d", psData->szDataName, psData->nNodeID);
				pTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NOW, 0, psData);
			}

			return true;
			break;
		case IDC_REMOVEDATA:	/* when remove button clicked */
			if (ListBox_GetCurSel (GetDlgItem (window, IDC_ASSIGNEDDATA)) != LB_ERR) {

				/* remove the file name from assigned to available */
				psRemoveData = new EventRemoveData ();
				psRemoveData->nNodeID = pTopology->GetSelectedNode ()->getID ();
				ListBox_GetText (GetDlgItem (window, IDC_ASSIGNEDDATA),
								 ListBox_GetCurSel (GetDlgItem (window, IDC_ASSIGNEDDATA)), szTemp2);
				ListBox_AddString (GetDlgItem (window, IDC_AVAILABLEDATA), szTemp2);
				ListBox_DeleteString (GetDlgItem (window, IDC_ASSIGNEDDATA),
									  ListBox_GetCurSel (GetDlgItem (window, IDC_ASSIGNEDDATA)));

				/* detach the file from the node */
				psRemoveData->szDataName = (char*) malloc (strlen (szTemp2) + 1);
				if (psRemoveData->szDataName) {
					strcpy (psRemoveData->szDataName, szTemp2);
				}

				psRemoveData->psWindow = psWindow;
				_snprintf (szText, EVENT_NAME_LEN, "Remove data %s from node %d ", psRemoveData->szDataName,
						   psRemoveData->nNodeID);
				pTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NOW, 0, psRemoveData);
			}

			return true;
			break;
		}

		return true;
		break;
	case WM_CLOSE:
		EndDialog (window, 0);
		return true;
		break;
	}

	return false;	/* DefWindowProc(window, msg, wparam,
					 * lparam);
					 * */
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
LRESULT CALLBACK cWindow::sendDataProc (HWND window, UINT msg, WPARAM wparam, LPARAM lparam) {
	char szTemp[65535];
	static cTopology * pTopology = NULL;
	static cWindow * psWindow = NULL;
	char*  szFileName;
	static int nCurrentNodeID = 0;
	char*  szNodeName;
	char*  szFileName1;
	char*  cStart;
	char*  cEnd;
	static dataXML dataRecord[MAX_DATANUM];
	static loadData*  ld;
	static int dataIndex = -1;
	EventSendData*	psSendData;
	EventNewLink*  psLinkData;
	EventPropertiesLink*  psLinkProData;
	char szText[EVENT_NAME_LEN];
	cTopologyNode*	psIterator;

	szNodeName = (char*) malloc (TOPOLOGYNODE_NAMLEN);
	szNodeName[0] = 0;
	szFileName1 = (char*) malloc (TOPOLOGYNODE_NAMLEN);

	szFileName1[0] = 0;
	switch (msg) {
	case WM_INITDIALOG:
		psWindow = (cWindow *) lparam;
		pTopology = psWindow->pTopology;

		/* parse the "Data.xml" to ge the database of all files' name and sensitivity level */
		dataIndex = -1;
		for (int i = 0; i < MAX_DATANUM; i++) {
			dataRecord[i].szName = (char*) malloc (FILE_NAMELEN);
			dataRecord[i].szName[0] = 0;
		}

		ld = new loadData (dataRecord, &dataIndex);
		ld->Sax2ReadXML ("Data.xml");

		/* get the node's assigned file list */
		strcpy (szTemp, pTopology->GetSelectedNode ()->szData);

		szFileName = strtok (szTemp, ";");
		if (szFileName != NULL)
			SetDlgItemText (window, IDC_FILELIST, szFileName);
		while (szFileName != NULL) {
			ComboBox_AddString (GetDlgItem (window, IDC_FILELIST), szFileName);
			szFileName = strtok (NULL, ";");
		}

		/* get existing nodes list */
		nCurrentNodeID = pTopology->GetSelectedNode ()->getID ();
		psIterator = pTopology->GetpList ()->startIterator ();

		while (pTopology->GetpList ()->isIteratorValid (psIterator)) {
			if (pTopology->GetpList ()->getCurrentID (psIterator) != nCurrentNodeID) {
				strcpy (szNodeName, pTopology->GetpList ()->getCurrentNode (psIterator)->GetName ());
				sprintf (szTemp, "(%d)", pTopology->GetpList ()->getCurrentNode (psIterator)->getID ());
				strcat (szNodeName, szTemp);
				ComboBox_AddString (GetDlgItem (window, IDC_NODELIST), szNodeName);
			}

			psIterator = pTopology->GetpList ()->moveForward (psIterator);
		}

		ComboBox_GetLBText (GetDlgItem (window, IDC_NODELIST), 0, szNodeName);
		SetDlgItemText (window, IDC_NODELIST, szNodeName);

		return true;
		break;
	case WM_COMMAND:
		switch (LOWORD (wparam)) {
		case IDOK:

			/* source node id */
			psSendData = new EventSendData ();
			psSendData->nFrom = pTopology->GetSelectedNode ()->getID ();

			/* get destination node id by reading the ID number in () */
			GetDlgItemText (window, IDC_NODELIST, szNodeName, TOPOLOGYNODE_NAMLEN);
			if (szNodeName[0] != 0) {
				cStart = strrchr (szNodeName, '(');
				cStart[0] = '0';
				cEnd = strrchr (szNodeName, ')');
				cStart[cEnd - cStart] = 0;

				psSendData->nTo = atoi (cStart);

				/* get the to be sent file name */
				GetDlgItemText (window, IDC_FILELIST, szFileName1, TOPOLOGYNODE_NAMLEN);
				if (szFileName1[0] != 0) {
					psSendData->szDataName = (char*) malloc (strlen (szFileName1) + 1);
					if (psSendData->szDataName) {
						strcpy (psSendData->szDataName, szFileName1);
					}
				}

				/* generate sending event */
				psSendData->psWindow = psWindow;
				_snprintf (szText, EVENT_NAME_LEN, "Send data %s from node %d to node %d", psSendData->szDataName,
						   psSendData->nFrom, psSendData->nTo);
				pTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NOW, 0, psSendData);

				/* create link between source node and destination node */
				psLinkData = new EventNewLink ();
				psLinkData->nFrom = psSendData->nFrom;
				psLinkData->nTo = psSendData->nTo;
				psLinkData->nLinkID = pTopology->GetNextLinkID ();
				psLinkData->psWindow = psWindow;
				_snprintf (szText, EVENT_NAME_LEN, "Create link from node %d to %d", psLinkData->nFrom, psLinkData->nTo);
				pTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NOW, 0, psLinkData);

				/*
				 * set up the link's properties based on the file's sensitivity level (from
				 * Data.xml) and etc
				 */
				if (pTopology->GetSelectedLink () != NULL) {
					psLinkProData = new EventPropertiesLink ();
					psLinkProData->nNodeFromID = pTopology->GetSelectedLink ()->psLinkFrom->getID ();
					psLinkProData->nLinkID = pTopology->GetSelectedLink ()->nLinkID;

					psLinkProData->szLinkOutput = (char*) malloc (1);
					psLinkProData->szLinkOutput[0] = 0;
					psLinkProData->szLinkInput = (char*) malloc (1);
					psLinkProData->szLinkInput[0] = 0;
					psLinkProData->nSecurity = (int) (pTopology->ConvertSecurityToInt ('U'));

					for (int i = 0; i <= dataIndex; i++) {
						if (strcmp (dataRecord[i].szName, szFileName1) == 0) {
							psLinkProData->nSensitivityLevel = dataRecord[i].nSensitivityLevel;
							psLinkProData->nChannelIn = i + 1;
							psLinkProData->nChannelOut = i + 1;

							break;
						} else if (i == dataIndex) {

							/*
							 * if no mached file name found in Data.xml file, set the link's sensitivity level
							 * to highest by default ;
							 * it equals to send a file with sensitivity level of 0, the security thus is at
							 * the tightest level
							 */
							MessageBox (NULL, "No matched file name found in Data.xml file!", "Warning!", MB_ICONWARNING);
							psLinkProData->nSensitivityLevel = 0;
							psLinkProData->nChannelIn = 0;
							psLinkProData->nChannelOut = 0;
						}
					}

					psLinkProData->psWindow = psWindow;
					_snprintf (szText, EVENT_NAME_LEN, "Set link properties from node %d to %d",
							   psLinkProData->nNodeFromID, pTopology->GetSelectedLink ()->psLinkTo->getID ());
					pTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NOW, 0, psLinkProData);
				}

				pTopology->RedrawTopology ();
			}

			EndDialog (window, LOWORD (wparam));
			return true;
			break;
		case IDCANCEL:
			EndDialog (window, LOWORD (wparam));
			return true;
			break;
		}

		return true;
		break;
	case WM_CLOSE:
		EndDialog (window, 0);
		return true;
		break;
	}

	return false;	/* DefWindowProc(window, msg, wparam,
					 * lparam);
					 * */
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
LRESULT CALLBACK cWindow::manageDataProc (HWND window, UINT msg, WPARAM wparam, LPARAM lparam) {
	int dataIndex;
	int curIndex;
	int nSLValue;
	char*  szTemp;
	char*  szTempSL;
	char*  cStart;
	bool boTemp;

	dataIndex = -1;
	curIndex = -1;
	nSLValue = -1;
	szTemp = (char*) malloc (1024);

	szTemp[0] = 0;
	szTempSL = (char*) malloc (256);

	szTempSL[0] = 0;
	boTemp = false;

	switch (msg) {
	case WM_INITDIALOG:

		/* parse the Data.xml file */
		dataXML dataRecord[MAX_DATANUM];

		loadData*  ld;
		for (int i = 0; i < MAX_DATANUM; i++) {
			dataRecord[i].szName = (char*) malloc (FILE_NAMELEN);
			dataRecord[i].szName[0] = 0;
		}

		ld = new loadData (dataRecord, &dataIndex);
		ld->Sax2ReadXML ("Data.xml");

		/* put all file names and sensitivity levels in drop down list */
		for (int i = 0; i <= dataIndex; i++) {
			sprintf (szTemp, "(%d)", dataRecord[i].nSensitivityLevel);
			ListBox_AddString (GetDlgItem (window, IDC_DATAINXML), strcat (dataRecord[i].szName, szTemp));
		}

		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_ADDSL), "9");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_ADDSL), "8");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_ADDSL), "7");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_ADDSL), "6");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_ADDSL), "5");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_ADDSL), "4");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_ADDSL), "3");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_ADDSL), "2");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_ADDSL), "1");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_ADDSL), "0");
		sprintf (szTemp, "%d", 9);
		SetDlgItemText (window, IDC_COMBO_ADDSL, szTemp);

		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_UPDATESL), "9");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_UPDATESL), "8");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_UPDATESL), "7");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_UPDATESL), "6");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_UPDATESL), "5");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_UPDATESL), "4");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_UPDATESL), "3");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_UPDATESL), "2");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_UPDATESL), "1");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_UPDATESL), "0");
		sprintf (szTemp, "%d", 9);
		SetDlgItemText (window, IDC_COMBO_UPDATESL, szTemp);

		return true;
		break;
	case WM_COMMAND:
		switch (LOWORD (wparam)) {
		case IDOK:	/* button texted as "close" */
			/* save the file name and sensitivity level in Dada.xml file */
			saveData * sd;
			sd = new saveData ("Data.xml");
			dataIndex = ListBox_GetCount (GetDlgItem (window, IDC_DATAINXML));
			for (curIndex = 0; curIndex < dataIndex; curIndex++) {
				ListBox_GetText (GetDlgItem (window, IDC_DATAINXML), curIndex, szTemp);
				cStart = strrchr (szTemp, '(');
				nSLValue = int (cStart[1] - '0');
				cStart[0] = 0;
				sd->saveFile (szTemp, nSLValue);
			}

			sd->createXMLFile ();

			EndDialog (window, LOWORD (wparam));
			return true;
			break;
		case IDCANCEL:
			EndDialog (window, LOWORD (wparam));
			return true;
			break;
		case IDC_MODIFY_DATA:	/* update button clicked, change the sensitivity level */
			GetDlgItemText (window, IDC_COMBO_UPDATESL, szTempSL, 256);
			nSLValue = int (szTempSL[0] - '0');
			curIndex = ListBox_GetCurSel (GetDlgItem (window, IDC_DATAINXML));

			if ((curIndex != LB_ERR) && (nSLValue >= 0) && (nSLValue <= 9)) {
				ListBox_GetText (GetDlgItem (window, IDC_DATAINXML), curIndex, szTemp);
				cStart = strrchr (szTemp, '(');
				cStart[1] = szTempSL[0];
				ListBox_DeleteString (GetDlgItem (window, IDC_DATAINXML), curIndex);
				ListBox_InsertString (GetDlgItem (window, IDC_DATAINXML), curIndex, szTemp);
			}

			return true;
			break;
		case IDC_DELETE_DATA:	/* delete a file from the list */
			curIndex = ListBox_GetCurSel (GetDlgItem (window, IDC_DATAINXML));
			if (curIndex != LB_ERR) {
				ListBox_DeleteString (GetDlgItem (window, IDC_DATAINXML), curIndex);
			}

			return true;
			break;
		case IDC_ADD_DATA:		/* add a new file into the list */
			for (int i = 0; i <= 9; i++) {
				GetDlgItemText (window, IDC_EDIT_FILENAME, szTemp, 1024);;
				strcat (szTemp, "(");
				sprintf (szTempSL, "%d", i);
				strcat (szTemp, szTempSL);
				strcat (szTemp, ")");
				if (ListBox_FindStringExact (GetDlgItem (window, IDC_DATAINXML), -1, szTemp) != LB_ERR) {
					boTemp = true;
					break;
				}
			}

			if (boTemp) {

				/* file name must be different */
				MessageBox (NULL, "File already included!", "Error!", MB_ICONERROR);
			} else {
				GetDlgItemText (window, IDC_EDIT_FILENAME, szTemp, 1024);
				GetDlgItemText (window, IDC_COMBO_ADDSL, szTempSL, 256);
				nSLValue = int (szTempSL[0] - '0');
				if ((nSLValue >= 0) && (nSLValue <= 9) && (strlen (szTemp) > 0)) {
					strcat (szTemp, "(");
					strcat (szTemp, szTempSL);
					strcat (szTemp, ")");
					ListBox_AddString (GetDlgItem (window, IDC_DATAINXML), szTemp);
				}
			}

			return true;
			break;
		}

		return true;
		break;
	case WM_CLOSE:	/* same as click close button, save the list into Data.xml file */
		saveData * sd;
		sd = new saveData ("Data.xml");
		dataIndex = ListBox_GetCount (GetDlgItem (window, IDC_DATAINXML));
		for (curIndex = 0; curIndex < dataIndex; curIndex++) {
			ListBox_GetText (GetDlgItem (window, IDC_DATAINXML), curIndex, szTemp);
			cStart = strrchr (szTemp, '(');
			nSLValue = int (cStart[1] - '0');
			cStart[0] = 0;
			sd->saveFile (szTemp, nSLValue);
		}

		sd->createXMLFile ();

		EndDialog (window, 0);
		return true;
		break;
	}

	return false;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
LRESULT CALLBACK cWindow::channelsProc (HWND window, UINT msg, WPARAM wparam, LPARAM lparam) {
	static cWindow*	 pWindow = NULL;
	static cTopology*  pTopology = NULL;
	char szTemp[256];
	int nValue;
	EventPropertiesLink*  psData;
	char szText[EVENT_NAME_LEN];

	switch (msg) {
	case WM_INITDIALOG:
		pWindow = (cWindow*) lparam;
		pTopology = pWindow->pTopology;
		if (pTopology->GetSelectedLink () != NULL) {
			sprintf (szTemp, "%d", pTopology->GetSelectedLink ()->nChannelOut);
			SetDlgItemText (window, IDC_OUTCHANNEL, szTemp);

			sprintf (szTemp, "%d", pTopology->GetSelectedLink ()->nChannelIn);
			SetDlgItemText (window, IDC_INCHANNEL, szTemp);

			strncpy (szTemp, pTopology->GetSelectedLink ()->szLinkOutput, sizeof (szTemp));
			SetDlgItemText (window, IDC_OUTPROPERTY, szTemp);

			strncpy (szTemp, pTopology->GetSelectedLink ()->szLinkInput, sizeof (szTemp));
			SetDlgItemText (window, IDC_INPROPERTY, szTemp);

			sprintf (szTemp, "%c", pTopology->ConvertIntToSecurity ((SEC) (pTopology->GetSelectedLink ()->nSecurity)));
			SetDlgItemText (window, IDC_INOUTSEC, szTemp);

			sprintf (szTemp, "%d", pTopology->GetSelectedLink ()->nSensitivityLevel);
			SetDlgItemText (window, IDC_SLCHANNEL, szTemp);
		}

		return true;
		break;
	case WM_COMMAND:
		if (LOWORD (wparam) == IDOK) {
			if (pTopology->GetSelectedLink () != NULL) {
				psData = new EventPropertiesLink ();
				psData->nNodeFromID = pTopology->GetSelectedLink ()->psLinkFrom->getID ();
				psData->nLinkID = pTopology->GetSelectedLink ()->nLinkID;

				/* Channel out */
				GetDlgItemText (window, IDC_OUTCHANNEL, szTemp, sizeof (szTemp));
				nValue = atoi (szTemp);
				psData->nChannelOut = nValue;

				/* Channel in */
				GetDlgItemText (window, IDC_INCHANNEL, szTemp, sizeof (szTemp));
				nValue = atoi (szTemp);
				psData->nChannelIn = nValue;

				/* Arbitrary link output properties */
				GetDlgItemText (window, IDC_OUTPROPERTY, szTemp, sizeof (szTemp));
				psData->szLinkOutput = (char*) malloc (strlen (szTemp) + 1);
				if (psData->szLinkOutput) {
					strcpy (psData->szLinkOutput, szTemp);
				}

				/* Arbitrary link input properties */
				GetDlgItemText (window, IDC_INPROPERTY, szTemp, sizeof (szTemp));
				psData->szLinkInput = (char*) malloc (strlen (szTemp) + 1);
				if (psData->szLinkInput) {
					strcpy (psData->szLinkInput, szTemp);
				}

				/* Security level */
				GetDlgItemText (window, IDC_INOUTSEC, szTemp, sizeof (szTemp));
				nValue = (int) (pTopology->ConvertSecurityToInt (szTemp[0]));
				psData->nSecurity = nValue;

				/* Sensitivity level */
				GetDlgItemText (window, IDC_SLCHANNEL, szTemp, sizeof (szTemp));
				nValue = atoi (szTemp);
				psData->nSensitivityLevel = nValue;

				psData->psWindow = pWindow;
				_snprintf (szText, EVENT_NAME_LEN, "Set link properties from node %d to %d", psData->nNodeFromID,
						   pTopology->GetSelectedLink ()->psLinkTo->getID ());
				pTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NOW, 0, psData);
			}

			pTopology->boLinkCancelled = false;
			EndDialog (window, LOWORD (wparam));
			return true;
			break;
		}

		if (LOWORD (wparam) == IDCANCEL) {
			pTopology->boLinkCancelled = true;
			EndDialog (window, LOWORD (wparam));
		}

		return true;
		break;
	case WM_CLOSE:
		EndDialog (window, 0);
		return true;
		break;
	}

	return false;	/* DefWindowProc(window, msg, wparam,
					 * lparam);
					 * */
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
LRESULT CALLBACK cWindow::componentProc (HWND window, UINT msg, WPARAM wparam, LPARAM lparam) {
	static cWindow*	 pWindow = NULL;
	static cTopology*  pTopology = NULL;
	static cPropertySet*  pcProperties = NULL;
	static cTopologyNode * psNode;
	char szTemp[256];
	int nValue;
	float fValue;
	bool boValue;
	EventPropertiesNode*  psData;
	EventPropertySetNode*  psPropertiesEvent;
	char szText[EVENT_NAME_LEN];
	ControlList sControls;
	ControlList::const_iterator sControlIter;
	PROPTYPE ePropType;
	char szValue[1024];
	std::list<string> sOptionsList;
	std::list<string>::iterator iOptions;

	switch (msg) {
	case WM_INITDIALOG:
		pWindow = (cWindow*) lparam;
		pTopology = pWindow->pTopology;
		psNode = pTopology->GetSelectedNode ();
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_SL), "9");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_SL), "8");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_SL), "7");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_SL), "6");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_SL), "5");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_SL), "4");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_SL), "3");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_SL), "2");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_SL), "1");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_SL), "0");

		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_EA), "Unspecified");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_EA), "WEP-40");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_EA), "RC2-40");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_EA), "RC4-40");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_EA), "WEP-114");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_EA), "DES-56");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_EA), "RC5-56");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_EA), "RC2-128");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_EA), "RC4-128");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_EA), "RC5-128");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_EA), "TDES-168");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_EA), "IDEA-128");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_EA), "Skipjack-80");

		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_SS), "Low");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_SS), "Mid");
		ComboBox_AddString (GetDlgItem (window, IDC_COMBO_SS), "High");

		if (psNode) {

			/* node ID */
			sprintf (szTemp, "%d", psNode->getID ());
			SetDlgItemText (window, IDC_NODEID, szTemp);

			/* Arbitary node property */
			sprintf (szTemp, "%s", psNode->szCertProps);
			SetDlgItemText (window, IDC_NODECERTPROPS, szTemp);

			/* Security max */
			sprintf (szTemp, "%c", pTopology->ConvertIntToSecurity ((SEC) (psNode->nSecurityMax)));
			SetDlgItemText (window, IDC_NODESECMAX, szTemp);

			/* security min */
			sprintf (szTemp, "%c", pTopology->ConvertIntToSecurity ((SEC) (psNode->nSecurityMin)));
			SetDlgItemText (window, IDC_NODESECMIN, szTemp);

			/* Node name */
			sprintf (szTemp, "%s", psNode->GetName ());
			SetDlgItemText (window, IDC_NODENAME, szTemp);

			/* sensitivity level */
			sprintf (szTemp, "%d", psNode->nSensitivityLevel);
			SetDlgItemText (window, IDC_COMBO_SL, szTemp);

			/* encryption algorithm */
			sprintf (szTemp, "%s", psNode->szEncryptAlgorithm);
			SetDlgItemText (window, IDC_COMBO_EA, szTemp);

			/* staff skills */
			sprintf (szTemp, "%s", psNode->szStaffSkills);
			SetDlgItemText (window, IDC_COMBO_SS, szTemp);

			/* Add hoc Rx and Tx distances */
			SetDlgItemInt (window, IDC_ADHOCTX, (unsigned int) (psNode->fRangeTx * ADHOC_SCALE), false);
			SetDlgItemInt (window, IDC_ADHOCRX, (unsigned int) (psNode->fRangeRx * ADHOC_SCALE), false);

			if (psNode->boFirewall) {
				CheckDlgButton (window, IDC_CHECK_FIREWALL, BST_CHECKED);
			} else {
				CheckDlgButton (window, IDC_CHECK_FIREWALL, BST_UNCHECKED);
			}

			if (psNode->boIDS) {
				CheckDlgButton (window, IDC_CHECK_IDS, BST_CHECKED);
			} else {
				CheckDlgButton (window, IDC_CHECK_IDS, BST_UNCHECKED);
			}

			if (psNode->boExternal) {
				CheckDlgButton (window, IDC_CHECK_EXTERNAL, BST_CHECKED);
			} else {
				CheckDlgButton (window, IDC_CHECK_EXTERNAL, BST_UNCHECKED);
			}
		}

		if (psNode && (psNode->boAdHoc)) {
			CheckDlgButton (window, IDC_CHECK_ADHOC, BST_CHECKED);
			EnableWindow (GetDlgItem (window, IDC_ADHOCRX), TRUE);
			EnableWindow (GetDlgItem (window, IDC_ADHOCTX), TRUE);
			EnableWindow (GetDlgItem (window, IDC_SLIDER_ADHOCRX), TRUE);
			EnableWindow (GetDlgItem (window, IDC_SLIDER_ADHOCTX), TRUE);
		} else {
			CheckDlgButton (window, IDC_CHECK_ADHOC, BST_UNCHECKED);
			EnableWindow (GetDlgItem (window, IDC_ADHOCRX), FALSE);
			EnableWindow (GetDlgItem (window, IDC_ADHOCTX), FALSE);
			EnableWindow (GetDlgItem (window, IDC_SLIDER_ADHOCRX), FALSE);
			EnableWindow (GetDlgItem (window, IDC_SLIDER_ADHOCTX), FALSE);
		}

		if (psNode) {
			pcProperties = new cPropertySet (pWindow);
			pcProperties->AddPropertySetToWindow (window, 140, psNode);
		}

		return true;
		break;
	case WM_COMMAND:
		if (LOWORD (wparam) == IDOK) {
			if (pTopology->GetSelectedNode () != NULL) {
				psData = new EventPropertiesNode ();
				psData->nNodeID = pTopology->GetSelectedNode ()->getID ();

				/* Arbitrary node properties */
				GetDlgItemText (window, IDC_NODECERTPROPS, szTemp, sizeof (szTemp));
				psData->szCertProps = (char*) malloc (strlen (szTemp) + 1);
				if (psData->szCertProps) {
					strcpy (psData->szCertProps, szTemp);
				}

				/* Security max */
				GetDlgItemText (window, IDC_NODESECMAX, szTemp, sizeof (szTemp));
				nValue = (int) (pTopology->ConvertSecurityToInt (szTemp[0]));
				psData->nSecurityMax = nValue;

				/* Security min */
				GetDlgItemText (window, IDC_NODESECMIN, szTemp, sizeof (szTemp));
				nValue = (int) (pTopology->ConvertSecurityToInt (szTemp[0]));
				psData->nSecurityMin = nValue;

				/* Node name */
				GetDlgItemText (window, IDC_NODENAME, szTemp, sizeof (szTemp));
				psData->szName = (char*) malloc (strlen (szTemp) + 1);
				if (psData->szName) {
					strcpy (psData->szName, szTemp);
				}

				/* Sensitivity level */
				GetDlgItemText (window, IDC_COMBO_SL, szTemp, sizeof (szTemp));
				nValue = int (szTemp[0] - '0');
				if ((nValue >= 0) && (nValue <= 9)) {
					psData->nSensitivityLevel = nValue;
				} else {
					psData->nSensitivityLevel = 9;
				}

				/* Encryption algorithm */
				GetDlgItemText (window, IDC_COMBO_EA, szTemp, sizeof (szTemp));
				psData->szEncryptAlgorithm = (char*) malloc (strlen (szTemp) + 1);
				if (psData->szEncryptAlgorithm) {
					strcpy (psData->szEncryptAlgorithm, szTemp);
				}

				/* Staff skills */
				GetDlgItemText (window, IDC_COMBO_SS, szTemp, sizeof (szTemp));
				psData->szStaffSkills = (char*) malloc (strlen (szTemp) + 1);
				if (psData->szStaffSkills) {
					strcpy (psData->szStaffSkills, szTemp);
				}

				/* Firewall */
				if (IsDlgButtonChecked (window, IDC_CHECK_FIREWALL) == BST_CHECKED) {
					psData->boFirewall = true;
				} else {
					psData->boFirewall = false;
				}

				/* IDS */
				if (IsDlgButtonChecked (window, IDC_CHECK_IDS) == BST_CHECKED) {
					psData->boIDS = true;
				} else {
					psData->boIDS = false;
				}

				/* External */
				if (IsDlgButtonChecked (window, IDC_CHECK_EXTERNAL) == BST_CHECKED) {
					psData->boExternal = true;
				} else {
					psData->boExternal = false;
				}

				/* Add hoc Rx and Tx distances */
				if (IsDlgButtonChecked (window, IDC_CHECK_ADHOC) == BST_CHECKED) {
					psData->boAdHoc = true;
				} else {
					psData->boAdHoc = false;
				}

				psData->fRangeTx = ((float) GetDlgItemInt (window, IDC_ADHOCTX, NULL, false)) / ADHOC_SCALE;
				psData->fRangeRx = ((float) GetDlgItemInt (window, IDC_ADHOCRX, NULL, false)) / ADHOC_SCALE;

				psData->psWindow = pWindow;
				_snprintf (szText, EVENT_NAME_LEN, "Set node %d properties", psData->nNodeID);
				pTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NOW, 0, psData);

				// Collect the details from the dynamic properties
				psPropertiesEvent = new EventPropertySetNode ();
				sControls = pcProperties->GetControlList ();
				for (sControlIter = sControls.begin (); sControlIter != sControls.end (); ++sControlIter) {
					ePropType = psNode->PropertyGetType (sControlIter->szProperty);
					switch (ePropType) {
					case PROPTYPE_INT:
						SendMessage (sControlIter->hControl, WM_GETTEXT, sizeof (szValue), reinterpret_cast<LPARAM> ((LPCTSTR) (szValue)));
						sscanf (szValue, "%d", & nValue);
						psPropertiesEvent->PropertySetInt (sControlIter->szProperty, nValue);
						break;
					case PROPTYPE_FLOAT:
						SendMessage (sControlIter->hControl, WM_GETTEXT, sizeof (szValue), reinterpret_cast<LPARAM> ((LPCTSTR) (szValue)));
						sscanf (szValue, "%f", & fValue);
						psPropertiesEvent->PropertySetFloat (sControlIter->szProperty, fValue);
						break;
					case PROPTYPE_BOOL:
						boValue = (SendMessage (sControlIter->hControl, BM_GETCHECK, 0, 0) == BST_CHECKED);
						psPropertiesEvent->PropertySetBool (sControlIter->szProperty, boValue);
						break;
					case PROPTYPE_STRING:
						SendMessage (sControlIter->hControl, WM_GETTEXT, sizeof (szValue), reinterpret_cast<LPARAM> ((LPCTSTR) (szValue)));
						psPropertiesEvent->PropertySetString (sControlIter->szProperty, szValue);
						break;
					case PROPTYPE_OPTION:
						SendMessage (sControlIter->hControl, WM_GETTEXT, sizeof (szValue), reinterpret_cast<LPARAM> ((LPCTSTR) (szValue)));
						if (strlen (szValue) > 0) {
							sOptionsList = psNode->PropertyGetOptionList (sControlIter->szProperty);
							for (iOptions = sOptionsList.begin (); iOptions != sOptionsList.end (); iOptions++) {
								psPropertiesEvent->PropertySetOptionAdd (sControlIter->szProperty, iOptions->c_str ());
							}
							psPropertiesEvent->PropertySetOption (sControlIter->szProperty, szValue);
						}
						break;
					default:
						// Do nothing
						break;
					}
				}

				psPropertiesEvent->nNodeID = psNode->getID ();
				psPropertiesEvent->psWindow = pWindow;
				_snprintf (szText, EVENT_NAME_LEN, "Set node %d property set", psData->nNodeID);
				pTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NOW, 0, psPropertiesEvent);
			}

			if (pcProperties) {
				pcProperties->ClearControls ();
				delete pcProperties;
				pcProperties = NULL;
			}
			EndDialog (window, LOWORD (wparam));
			return true;
			break;
		}

		if (LOWORD (wparam) == IDCANCEL) {
			pTopology->boLinkCancelled = true;
			if (pcProperties) {
				pcProperties->ClearControls ();
				delete pcProperties;
				pcProperties = NULL;
			}
			EndDialog (window, LOWORD (wparam));
		}

		if (LOWORD (wparam) == IDC_CHECK_ADHOC) {
			if (IsDlgButtonChecked (window, IDC_CHECK_ADHOC) == BST_CHECKED) {
				EnableWindow (GetDlgItem (window, IDC_ADHOCRX), TRUE);
				EnableWindow (GetDlgItem (window, IDC_ADHOCTX), TRUE);
				EnableWindow (GetDlgItem (window, IDC_SLIDER_ADHOCRX), TRUE);
				EnableWindow (GetDlgItem (window, IDC_SLIDER_ADHOCTX), TRUE);
			} else {
				EnableWindow (GetDlgItem (window, IDC_ADHOCRX), FALSE);
				EnableWindow (GetDlgItem (window, IDC_ADHOCTX), FALSE);
				EnableWindow (GetDlgItem (window, IDC_SLIDER_ADHOCRX), FALSE);
				EnableWindow (GetDlgItem (window, IDC_SLIDER_ADHOCTX), FALSE);
			}
		}

		if ((LOWORD (wparam) == IDC_ADHOCTX) && (HIWORD (wparam) == EN_UPDATE)) {
			nValue = GetDlgItemInt (window, IDC_ADHOCTX, NULL, false) / ADHOC_SLIDER_FACTOR;
			SendMessage (GetDlgItem (window, IDC_SLIDER_ADHOCTX), TBM_SETPOS, (WPARAM) TRUE, (LPARAM) nValue);
		}

		if ((LOWORD (wparam) == IDC_ADHOCRX) && (HIWORD (wparam) == EN_UPDATE)) {
			nValue = GetDlgItemInt (window, IDC_ADHOCRX, NULL, false) / ADHOC_SLIDER_FACTOR;
			SendMessage (GetDlgItem (window, IDC_SLIDER_ADHOCRX), TBM_SETPOS, (WPARAM) TRUE, (LPARAM) nValue);
		}

		return true;
		break;
	case WM_HSCROLL:
		if ((HWND) lparam == GetDlgItem (window, IDC_SLIDER_ADHOCTX)) {
			nValue = (int) SendMessage ((HWND) lparam, TBM_GETPOS, 0, 0) * ADHOC_SLIDER_FACTOR;
			SetDlgItemInt (window, IDC_ADHOCTX, (unsigned int) nValue, false);
		}

		if ((HWND) lparam == GetDlgItem (window, IDC_SLIDER_ADHOCRX)) {
			nValue = (int) SendMessage ((HWND) lparam, TBM_GETPOS, 0, 0) * ADHOC_SLIDER_FACTOR;
			SetDlgItemInt (window, IDC_ADHOCRX, (unsigned int) nValue, false);
		}

		break;
	case WM_CLOSE:
		if (pcProperties) {
			pcProperties->ClearControls ();
			delete pcProperties;
			pcProperties = NULL;
		}
		EndDialog (window, 0);
		return true;
		break;
	}

	return false;	/* DefWindowProc(window, msg, wparam,
					 * lparam);
					 * */
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
LRESULT CALLBACK cWindow::checkresultsProc (HWND window, UINT msg, WPARAM wparam, LPARAM lparam) {
	cXMLParse*	pXMLParse;
	char szTemp[256];

	int nResultListNum;
	char*  szResultListItem;
	char*  szResultListDesc;
	int nItem;
	RECT sRect;
	HWND hList;

	pXMLParse = NULL;

	switch (msg) {
	case WM_INITDIALOG:
		pXMLParse = (cXMLParse*) lparam;
		GetWindowRect (window, &sRect);

		hList = GetDlgItem (window, IDC_LIST1);

		nResultListNum = pXMLParse->ResultListNum ();
		for (nItem = 0; nItem < nResultListNum; nItem++) {
			strcpy (szTemp, "");
			szResultListItem = pXMLParse->ResultListItem (nItem);
			szResultListDesc = pXMLParse->ResultListDesc (nItem);
			if (pXMLParse->ResultListItemResult (nItem)) {
				_snprintf (szTemp, sizeof (szTemp), "TRUE	: %s (%s)", szResultListDesc, szResultListItem);
			} else {
				_snprintf (szTemp, sizeof (szTemp), "FALSE : %s (%s)", szResultListDesc, szResultListItem);
			}

			SendMessage (hList, (UINT) LB_ADDSTRING, 0, (LPARAM) szTemp);
		}

#if defined PERFORMANCE_TIME
		_snprintf (szTemp, sizeof (szTemp), "Timing : %f seconds", pXMLParse->PerformanceTiming ());
		SendMessage (hList, (UINT) LB_ADDSTRING, 0, (LPARAM) szTemp);
#endif

		/* Add the performance timing result in too */
		return true;
		break;
	case WM_COMMAND:
		if (LOWORD (wparam) == IDOK) {
			EndDialog (window, LOWORD (wparam));
			return true;
			break;
		}

		if (LOWORD (wparam) == IDCANCEL) {
			EndDialog (window, LOWORD (wparam));
		}

		return true;
		break;
	case WM_CLOSE:
		EndDialog (window, 0);
		return true;
		break;
	}

	return false;	/* DefWindowProc(window, msg, wparam,
					 * lparam);
					 * */
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void PopulateEventListCallback (unsigned int uStart, unsigned int uEnd, char const* szEvent, void* pData) {
	EventListData*	psData;
	LVITEM sItem;
	char szTemp[256];

	psData = (EventListData*) pData;

	sItem.mask = (LVIF_DI_SETITEM | LVIF_TEXT);
	_snprintf (szTemp, sizeof (szTemp), "%u", uStart);
	sItem.iItem = psData->nItem;
	sItem.iSubItem = 0;
	sItem.pszText = szTemp;
	SendMessage (psData->hList, (UINT) LVM_INSERTITEM, 0, (LPARAM) (&sItem));

	sItem.iSubItem = 1;
	_snprintf (szTemp, sizeof (szTemp), "%s", szEvent);
	sItem.pszText = szTemp;
	SendMessage (psData->hList, (UINT) LVM_SETITEM, 0, (LPARAM) (&sItem));

	psData->nItem++;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
LRESULT CALLBACK cWindow::eventlistProc (HWND window, UINT msg, WPARAM wparam, LPARAM lparam) {
	cTopology*	psTopology;
	char szTemp[256];

	RECT sRect;
	HWND hList;
	LVCOLUMN sColumn;
	static EventListData sEventList;

	psTopology = NULL;

	switch (msg) {
	case WM_INITDIALOG:
		psTopology = (cTopology*) lparam;
		GetWindowRect (window, &sRect);

		hList = GetDlgItem (window, IDC_EVENTLIST);

		_snprintf (szTemp, sizeof (szTemp), "Time");
		sColumn.mask = (LVCF_FMT | LVCF_TEXT | LVCF_WIDTH);
		sColumn.fmt = LVCFMT_LEFT;
		sColumn.cx = EVENT_COLUMN_TIME_WIDTH;
		sColumn.pszText = szTemp;
		SendMessage (hList, (UINT) LVM_INSERTCOLUMN, 0, (LPARAM) (&sColumn));
		_snprintf (szTemp, sizeof (szTemp), "Event");
		sColumn.cx = EVENT_COLUMN_EVENT_WIDTH;
		sColumn.pszText = szTemp;
		SendMessage (hList, (UINT) LVM_INSERTCOLUMN, 1, (LPARAM) (&sColumn));

		sEventList.hList = hList;
		sEventList.nItem = 0;
		psTopology->GetTimer ()->PopulateEventList (PopulateEventListCallback, (void*) (&sEventList));
		return true;
		break;
	case WM_COMMAND:
		if (LOWORD (wparam) == IDOK) {
			EndDialog (window, LOWORD (wparam));
			return true;
			break;
		}

		return true;
		break;
	case WM_CLOSE:
		EndDialog (window, 0);
		return true;
		break;
	}

	return false;	/* DefWindowProc(window, msg, wparam,
					 * lparam);
					 * */
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void threadProc (cVM* pCurrentVM) {
	pCurrentVM->threadQuit = false;
	pCurrentVM->execute ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void listenProc (cWindow* pWin) {
	SOCKET nSockfd;
	struct sockaddr_in sHostAddr;
	struct sockaddr_in sClientAddr;
	int nReturn;
	int nSize;
	SOCKET nAcceptfd;
	char*  pcMainMemory;
	int nStorePos;
	fd_set sListenSet;
	struct timeval sTime;
	cOptions*  temp;
	long int memSize;
	cList*	pList;
	cVM*  pCurrent;
	bool bListen;
	bool boTopologyTab;
	cTopologyNode*	psIterator;

	nAcceptfd = NULL;
	temp = cOptions::create ();
	memSize = temp->getStackLimit ();
	pList = pWin->getList ();
	pCurrent = pWin->getCurrentVM ();
	bListen = temp->getAllowAgents ();
	boTopologyTab = false;

	::FILETIME sFileTime;
	GetSystemTimeAsFileTime (&sFileTime);
	srand (sFileTime.dwLowDateTime);

	if (bListen == true) {
		nSockfd = socket (AF_INET, SOCK_STREAM, 0);

		if (nSockfd != -1) {
			memset (&sHostAddr, 0, sizeof (struct sockaddr));
			sHostAddr.sin_family = AF_INET;
			sHostAddr.sin_port = htons (6432);
			sHostAddr.sin_addr.s_addr = htonl (INADDR_ANY);
			memset (&(sHostAddr.sin_zero), '\0', 8);

			nReturn = bind (nSockfd, (struct sockaddr*) &sHostAddr, sizeof (struct sockaddr));

			if (nReturn == 0) {
				nReturn = 0;
				if (nReturn == 0) {
					nReturn = listen (nSockfd, 10);
					if (nReturn == 0) {
						while (bListen) {
							pCurrent = pWin->getCurrentVM ();
							bListen = temp->getAllowAgents ();

							/*
							 * if (!bListen) ;
							 * { ;
							 * return;
							 * }
							 */
							FD_ZERO (&sListenSet);
							FD_SET (nSockfd, &sListenSet);
							sTime.tv_sec = 0;
							sTime.tv_usec = 100000;
							nReturn = select ((int) nSockfd + 1, &sListenSet, NULL, NULL, &sTime);
							if (nReturn != -1) {
								if (FD_ISSET (nSockfd, &sListenSet)) {

									/* need to find a VM */
									if ((pCurrent == NULL) || (pCurrent->getLoaded ())) {

										/* current one is not good */
										int numNodes;
										numNodes = pList->getCount ();
										psIterator = pList->startIterator ();

										for (int x = 0; x < numNodes; x++) {
											pCurrent = pList->getCurrentItem (psIterator);

											if (!pCurrent->getLoaded ()) {

												/* vm is good */
												break;
											} else {

												/* next */
												psIterator = pList->moveForward (psIterator);
											}
										}

										pCurrent = pList->getCurrentItem (psIterator);

										if (pCurrent == NULL) {
											if (temp->getAutoCreate ()) {
												if (numNodes < temp->getMachineLimit ()) {

													/* no good VM's, but can create one */
													pWin->newVM (false, -1);
													boTopologyTab = true;
													pCurrent = pWin->getCurrentVM ();
												}
											} else {

												/* no good vm's and can't create one so leave it */
												break;
											}
										}
									}

									/*
									 * current vm good ;
									 * Now we need to accept the data
									 */
									pCurrent->ConsolePrint ("Receiving agent...\n");

									nSize = sizeof (struct sockaddr_in);
									nAcceptfd = accept (nSockfd, (struct sockaddr*) &sClientAddr, &nSize);

									pCurrent->setRestart (true);
									pCurrent->setLoaded (false);

									pcMainMemory = pCurrent->getMainMemory ();
									nStorePos = 0;

									do
									{
										nReturn = recv (nAcceptfd, (pcMainMemory + nStorePos), (memSize - nStorePos), 0);
										if (nReturn < 0) {
											pCurrent->ConsolePrint ("Error receiving agent.\n");
											nReturn = WSAGetLastError ();
											printf ("Error: %d\n", nReturn);
										}

										/*
										 * Connection failing because socket set to nonblocking ;
										 * when set to blocking
										 */
										if (nReturn > 0) {
											nStorePos += nReturn;
										}
									} while ((nReturn > 0) && (nStorePos < memSize));

									/*
									 * shutdown (nAcceptfd, SD_BOTH);
									 * closesocket (nAcceptfd);
									 */
									if (nStorePos < memSize) {
										HANDLE tempHandle;

										/* Code loaded, now we can run it */
										pCurrent->ConsolePrint ("...Agent received.\n");
										pCurrent->setUpExecution ();
										tempHandle = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) threadProc,
																   pCurrent, 0, NULL);
										pCurrent->setThreadHandle (tempHandle);
									} else {

										/* Not enough memory to load the code */
										pCurrent->ConsolePrint ("Agent too large to receive fully.\n");
										pCurrent->setLoaded (false);
										pCurrent->setRestart (true);
									}

									/*
									 * if (boTopologyTab) ;
									 * { ;
									 * pWin->ShowTopologyTab ();
									 * boTopologyTab = false;
									 * }
									 */
								}

								/*
								 * shutdown (nAcceptfd, SD_BOTH);
								 * closesocket (nAcceptfd);
								 */
							} else {
								nReturn = WSAGetLastError ();
								if (pCurrent)
									pCurrent->ConsolePrint ("Error listening on socket\n");
								temp->setAllowAgents (false);
							}
						}
					} else {
						if (pCurrent)
							pCurrent->ConsolePrint ("Error listening to socket.\n");
						temp->setAllowAgents (false);
					}
				} else {
					if (pCurrent)
						pCurrent->ConsolePrint ("Error setting socket to non blocking\n");
					temp->setAllowAgents (false);
				}
			} else {
				if (pCurrent)
					pCurrent->ConsolePrint ("Error binding to socket.\n");
				temp->setAllowAgents (false);
			}
		} else {
			if (pCurrent)
				pCurrent->ConsolePrint ("Error creating socket.\n");
			temp->setAllowAgents (false);
		}

		bListen = temp->getAllowAgents ();
	}

	shutdown (nSockfd, SD_BOTH);
	closesocket (nSockfd);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void listenControlProc (cWindow* pWin) {
	SOCKET nSockfd;
	struct sockaddr_in sHostAddr;
	struct sockaddr_in sClientAddr;
	int nReturn;
	int nSize;
	SOCKET nAcceptfd;
	char pcControlBuffer[1024];
	int nStorePos;
	fd_set sListenSet;
	struct timeval sTime;
	cOptions*  temp;
	long int memSize;
	cList*	pList;
	cVM*  pCurrent;
	bool bListen;
	bool boTopologyTab;
	int nControlID;
	int nControlID1;
	int nControlID2;
	cTopologyNode*	psNode;
	cTopologyNode*	psNode1;
	cTopologyNode*	psNode2;
	cTopologyLink*	psLink;
	int nLinkID;
	int nNodeAddedID;
	bool boNodeAdded;
	char szText[EVENT_NAME_LEN];
	cTopologyNode*	psIterator;

	nAcceptfd = NULL;
	temp = cOptions::create ();
	memSize = sizeof (pcControlBuffer);
	pList = pWin->getList ();
	pCurrent = pWin->getCurrentVM ();
	bListen = temp->getAllowControl ();
	boTopologyTab = false;
	boNodeAdded = false;

	::FILETIME sFileTime;
	GetSystemTimeAsFileTime (&sFileTime);
	srand (sFileTime.dwLowDateTime);

	if (bListen == true) {
		nSockfd = socket (AF_INET, SOCK_STREAM, 0);

		if (nSockfd != -1) {
			memset (&sHostAddr, 0, sizeof (struct sockaddr));
			sHostAddr.sin_family = AF_INET;
			sHostAddr.sin_port = htons (6431);
			sHostAddr.sin_addr.s_addr = htonl (INADDR_ANY);
			memset (&(sHostAddr.sin_zero), '\0', 8);

			nReturn = bind (nSockfd, (struct sockaddr*) &sHostAddr, sizeof (struct sockaddr));

			if (nReturn == 0) {
				nReturn = 0;
				if (nReturn == 0) {
					nReturn = listen (nSockfd, 10);
					if (nReturn == 0) {
						while (bListen) {
							pCurrent = pWin->getCurrentVM ();
							bListen = temp->getAllowControl ();

							/*
							 * if (!bListen) ;
							 * { ;
							 * return;
							 * }
							 */
							FD_ZERO (&sListenSet);
							FD_SET (nSockfd, &sListenSet);
							sTime.tv_sec = 0;
							sTime.tv_usec = 100000;
							nReturn = select ((int) nSockfd + 1, &sListenSet, NULL, NULL, &sTime);
							if (nReturn != -1) {
								if (FD_ISSET (nSockfd, &sListenSet)) {

									/*/
									 *  need to find a VM ;
									 *  int numNodes = pList->getCount();
									 *  if (numNodes < temp->getMachineLimit()) ;
									 *  { ;
									 *  // Create new VM one ;
									 *  pWin->newVM(true);
									 *  boTopologyTab = true;
									 *  pCurrent = pWin->getCurrentVM();
									 *  } current vm good Now we need to accept the data ;
									 *  pCurrent->ConsolePrint("VM created through remote control.\n");
									 */
									nSize = sizeof (struct sockaddr_in);
									nAcceptfd = accept (nSockfd, (struct sockaddr*) &sClientAddr, &nSize);

									/*
									 * pCurrent->setRestart (true);
									 * pCurrent->setLoaded (false);
									 */
									nStorePos = 0;

									do
									{
										nReturn = recv (nAcceptfd, (pcControlBuffer + nStorePos), (memSize - nStorePos),
														0);
										if (nReturn < 0) {
											if (pCurrent) {
												pCurrent->ConsolePrint ("Error receiving agent.\n");
											}

											nReturn = WSAGetLastError ();
											printf ("Error: %d\n", nReturn);
										}

										/*
										 * Connection failing because socket set to nonblocking ;
										 * when set to blocking
										 */
										if (nReturn > 0) {
											nStorePos += nReturn;
										}

										/*
										 * Note: this should be the following (socked should be non-blocking for this ;
										 * } while ((nReturn > 0) && (nStorePos < memSize));
										 */
									} while ((nReturn < 0) && (nStorePos < memSize));

									/*
									 * shutdown (nAcceptfd, SD_BOTH);
									 * closesocket (nAcceptfd);
									 */
									if (nStorePos < memSize) {
										if (nStorePos > 2) {

											/* Code loaded, now we can run it */
											pcControlBuffer[nStorePos] = 0;

											nControlID = 0;
											switch (pcControlBuffer[1]) {
											case 'N':		/* Node control */
												switch (pcControlBuffer[0]) {
												case '+':	/* Add node */
													if (!boNodeAdded) {
														EventNewNode*  psData;
														psData = new EventNewNode ();
														nNodeAddedID = pList->getNextID ();
														psData->nNodeID = nNodeAddedID;
														psData->fXPos = 0.5;
														psData->fYPos = 1.0;
														psData->fZPos = 0.0;
														psData->nType = 0;
														psData->psWindow = pWin;
														_snprintf (szText, EVENT_NAME_LEN, "Create node %d", psData->nNodeID);
														pWin->pTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NEXT, 0, psData);

														Sleep (100);
														boNodeAdded = true;
													}

													/* Set other properties */
													EventPropertiesNode*  psNodePropData;
													psNodePropData = new EventPropertiesNode ();
													psNodePropData->boAdHoc = false;
													psNodePropData->fRangeRx = 3.0f;
													psNodePropData->fRangeTx = 3.0f;
													psNodePropData->nNodeID = nNodeAddedID;
													psNodePropData->boExternal = false;
													psNodePropData->boFirewall = false;
													psNodePropData->boIDS = false;
													psNodePropData->nSecurityMax = SEC_U;
													psNodePropData->nSecurityMin = SEC_U;
													psNodePropData->nSensitivityLevel = 5;
													psNodePropData->szCertProps = "";
													psNodePropData->szEncryptAlgorithm = (char*) malloc (ENCRYPT_ALGORITHM);
													_snprintf (psNodePropData->szEncryptAlgorithm, ENCRYPT_ALGORITHM, "AES");
													psNodePropData->szName = (char*) malloc (sizeof ("Westwood admin") + 1);
													_snprintf (psNodePropData->szName, sizeof ("Westwood admin") + 1, "Westwood admin");
													psNodePropData->szStaffSkills = (char*) malloc (STAFF_SKILLS);
													_snprintf (psNodePropData->szStaffSkills, STAFF_SKILLS, "High");

													psNodePropData->psWindow = pWin;

													_snprintf (szText, EVENT_NAME_LEN, "Set node %d properties", psNodePropData->nNodeID);
													pWin->pTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NEXT, 0, psNodePropData);

													Sleep (100);

													EventPropertySetNode * psNodePropSet;
													psNodePropSet = new EventPropertySetNode ();
													psNodePropSet->nNodeID = nNodeAddedID;
													psNodePropSet->PropertySetInt ("Authorisation", 5);
													psNodePropSet->psWindow = pWin;

													_snprintf (szText, EVENT_NAME_LEN, "Set node set %d properties", psNodePropSet->nNodeID);
													pWin->pTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NEXT, 0, psNodePropSet);



													/*
													 * // Create new VM one pWin->boFreezeRedraw = true;
													 * pWin->newVM (true, -1);
													 * pCurrent = pWin->getCurrentVM();
													 * boTopologyTab = true;
													 * pcControlBuffer[nStorePos] = 0;
													 * //pCurrent->ConsolePrint(pcControlBuffer);
													 * sscanf (pcControlBuffer, "+N%d", & nControlID);
													 * //sprintf (szCommand, "\nAdd node with ID %d.\n", nControlID);
													 * //pCurrent->ConsolePrint(szCommand);
													 * if (pCurrent) { pCurrent->psTopologyNode->nControlID = nControlID;
													 * }
													 */
													break;
												case '-':	/* Remove node */
													if (boNodeAdded) {

														/* Set other properties */
														EventPropertiesNode*  psNodePropData;
														psNodePropData = new EventPropertiesNode ();
														psNodePropData->boAdHoc = false;
														psNodePropData->fRangeRx = 3.0f;
														psNodePropData->fRangeTx = 3.0f;
														psNodePropData->nNodeID = nNodeAddedID;
														psNodePropData->boExternal = true;
														psNodePropData->boFirewall = false;
														psNodePropData->boIDS = false;
														psNodePropData->nSecurityMax = SEC_U;
														psNodePropData->nSecurityMin = SEC_U;
														psNodePropData->nSensitivityLevel = 5;
														psNodePropData->szCertProps = "";
														psNodePropData->szEncryptAlgorithm = (char*) malloc (ENCRYPT_ALGORITHM);
														_snprintf (psNodePropData->szEncryptAlgorithm, ENCRYPT_ALGORITHM, "AES");
														psNodePropData->szName = (char*) malloc (sizeof ("Unattended") + 1);
														_snprintf (psNodePropData->szName, sizeof ("Unattended") + 1, "Unattended");
														psNodePropData->szStaffSkills = (char*) malloc (STAFF_SKILLS);
														_snprintf (psNodePropData->szStaffSkills, STAFF_SKILLS, "High");
														psNodePropData->psWindow = pWin;

														_snprintf (szText, EVENT_NAME_LEN, "Set node %d properties", psNodePropData->nNodeID);
														pWin->pTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NEXT, 0, psNodePropData);

														Sleep (100);

														EventPropertySetNode * psNodePropSet;
														psNodePropSet = new EventPropertySetNode ();
														psNodePropSet->nNodeID = nNodeAddedID;
														psNodePropSet->PropertySetInt ("Authorisation", 0);
														psNodePropSet->psWindow = pWin;

														_snprintf (szText, EVENT_NAME_LEN, "Set node set %d properties", psNodePropSet->nNodeID);
														pWin->pTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NEXT, 0, psNodePropSet);

														Sleep (100);

														EventDeleteNode*  psDeleteData;
														psDeleteData = new EventDeleteNode ();
														psDeleteData->nNodeID = nNodeAddedID;
														psDeleteData->psWindow = pWin;
														_snprintf (szText, EVENT_NAME_LEN, "Delete node %d",
																   psDeleteData->nNodeID);
														pWin->pTopology->GetTimer ()->AddEvent (szText, pWin->pTopology->GetTimer ()->GetTime () + 5000, 0, psDeleteData);

														Sleep (5000);
														boNodeAdded = FALSE;
													}

													/*
													 * sscanf (pcControlBuffer, "-N%d", & nControlID);
													 * //sprintf (szCommand, "\nRemove node with ID %d.\n", nControlID);
													 * //pCurrent->ConsolePrint(szCommand);
													 * // Find the relevant VM, if it exists pList->startIterator();
													 * while ((pList->isIteratorValid()) && (pList->getCurrentItem
													 * ()->psTopologyNode->nControlID != nControlID)) { pList->moveForward();
													 * } pCurrent = pList->getCurrentItem ();
													 * if (pCurrent) { pWin->setCurrentVM (pCurrent);
													 * pWin->closeVM ();
													 * }
													 */
													break;
												default:	/* No action */
													break;
												}

												break;
											case 'L':		/* Link control */
												switch (pcControlBuffer[0]) {
												case '+':	/* Add link */
													sscanf (pcControlBuffer, "+L%d,%d", &nControlID1, &nControlID2);

													psNode1 = NULL;
													psNode2 = NULL;
													psIterator = pList->startIterator ();
													while
													(
														(pList->isIteratorValid (psIterator))
													&&	((psNode1 == NULL) || (psNode2 == NULL))
													) {
														psNode = pList->getCurrentItem (psIterator)->psTopologyNode;
														if (psNode->nControlID == nControlID1) {
															psNode1 = psNode;
														}

														if (psNode->nControlID == nControlID2) {
															psNode2 = psNode;
														}

														psIterator = pList->moveForward (psIterator);
													}

													if ((psNode1 != NULL) && (psNode2 != NULL)) {
														nLinkID = pWin->pTopology->GetNextLinkID ();
														new cTopologyLink (psNode1, psNode2, nLinkID);
														pWin->pTopology->RedrawTopology ();
													}

													break;
												case '-':	/* Remove link */
													sscanf (pcControlBuffer, "-L%d,%d", &nControlID1, &nControlID2);

													psLink = pWin->pTopology->FindCommandLink (nControlID1, nControlID2);
													if (psLink != NULL) {
														if (psLink == pWin->pTopology->GetSelectedLink ()) {
															pWin->pTopology->SetSelectedLink (NULL);
														}

														delete psLink;
														pWin->pTopology->RedrawTopology ();
													}

													break;
												default:	/* No action */
													break;
												}

												break;
											default:
												break;
											}
										}
									} else {

										/* Not enough memory to load the code */
										if (pCurrent) {
											pCurrent->ConsolePrint ("Control data not received fully.\n");
											pCurrent->setLoaded (false);
											pCurrent->setRestart (true);
										}
									}

									/*
									 * if (boTopologyTab) ;
									 * { ;
									 * pWin->ShowTopologyTab ();
									 * boTopologyTab = false;
									 * pWin->boFreezeRedraw = false;
									 * }
									 */
								}

								shutdown (nAcceptfd, SD_BOTH);
								closesocket (nAcceptfd);
							} else {
								nReturn = WSAGetLastError ();
								if (pCurrent)
									pCurrent->ConsolePrint ("Error listening on socket\n");
								temp->setAllowAgents (false);
							}
						}
					} else {
						if (pCurrent)
							pCurrent->ConsolePrint ("Error listening to socket.\n");
						temp->setAllowAgents (false);
					}
				} else {
					if (pCurrent)
						pCurrent->ConsolePrint ("Error setting socket to non blocking\n");
					temp->setAllowAgents (false);
				}
			} else {
				if (pCurrent)
					pCurrent->ConsolePrint ("Error binding to socket.\n");
				temp->setAllowAgents (false);
			}
		} else {
			if (pCurrent)
				pCurrent->ConsolePrint ("Error creating socket.\n");
			temp->setAllowAgents (false);
		}

		bListen = temp->getAllowAgents ();
	}

	shutdown (nSockfd, SD_BOTH);
	closesocket (nSockfd);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::CancelDrag (void) {
	eDragType = DRAGTYPE_INVALID;
	pTopology->psRenderGeo->boShowGhostLink = FALSE;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::CreateLink (int nFrom, int nTo, int nLinkID) {
	char szFrom[TOPOLOGYNODE_NAMLEN];
	char szTo[TOPOLOGYNODE_NAMLEN];

	pTopology->CreateLink (nFrom, nTo, nLinkID);
	ResourceBeep (IDR_SOUNDCREATELINK);

	/* Notify Dandelion */
	_snprintf (szFrom, sizeof (szFrom), "%d", nFrom);
	_snprintf (szTo, sizeof (szTo), "%d", nTo);
	if (psDlClient) {
		psDlClient->AddLink (szFrom, szTo);
	}

	/* Set to analyse */
	TopologyChanged ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::LeftButtonUp (int nXMousePos, int nYMousePos) {
	cTopologyNode*	psNodeFound;
	GLdouble fX;
	GLdouble fY;
	GLdouble fZ;
	EventNewLink*  psLinkData;
	EventMoveNode*	psNodeMoveData;
	SelectObj sObjectsSelected;
	unsigned int uTime;
	char szText[EVENT_NAME_LEN];

	sObjectsSelected = pTopology->SelectMap (nXMousePos, nYMousePos);
	pTopology->ConvertCoords (nXMousePos, nYMousePos, sObjectsSelected.fZPos, &fX, &fY, &fZ);

	switch (eDragType) {
	case DRAGTYPE_BOXMOVE:
		if (pTopology->psSelectedNode) {
			pTopology->MoveNode (pTopology->psSelectedNode, (float) (fXDragOffset + fX), (float) (fYDragOffset + fY),
								 0.0);
			UpdateAdHocLinks (pTopology->psSelectedNode);
			pTopology->RedrawTopology ();
			ResourceBeep (IDR_SOUNDMEDIUMBEEP);

			uTime = pTopology->GetTimer ()->GetTime ();
			psNodeMoveData = new EventMoveNode ();
			psNodeMoveData->nNodeID = pTopology->psSelectedNode->getID ();
			psNodeMoveData->fXPosTo = (float) (fXDragOffset + fX);
			psNodeMoveData->fYPosTo = (float) (fYDragOffset + fY);
			psNodeMoveData->fZPosTo = 0.0;
			psNodeMoveData->psWindow = this;
			_snprintf (szText, EVENT_NAME_LEN, "Move node %d", psNodeMoveData->nNodeID);
			pTopology->GetTimer ()->AddEvent (szText, uDragStartTime, (uTime - uDragStartTime), psNodeMoveData);
		}

		break;
	case DRAGTYPE_LINKSET:
		if (pTopology->psSelectedNode) {

			/*
			 * Find out if we arrived somewhere ;
			 * Find the box clicked in, if there was one
			 */
			sObjectsSelected = pTopology->SelectMain (nXMousePos, nYMousePos);
			psNodeFound = pTopology->DecodeNodeNumber (sObjectsSelected.nNode);

			if (psNodeFound && (psNodeFound != pTopology->psSelectedNode)) {

				/* Connect the nodes together */
				psLinkData = new EventNewLink ();
				psLinkData->nFrom = pTopology->psSelectedNode->getID ();;
				psLinkData->nTo = sObjectsSelected.nNode;
				psLinkData->nLinkID = pTopology->GetNextLinkID ();
				psLinkData->psWindow = this;

				_snprintf (szText, EVENT_NAME_LEN, "Create link from node %d to %d", psLinkData->nFrom, psLinkData->nTo);
				pTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NOW, 0, psLinkData);
			}
		}

		break;
	case DRAGTYPE_TRANSLATE:
		pTopology->MoveCentre (-(float) (nYMousePos - fYDragOffset) / MOUSE_TRANSLATE_SCALE,
							   (float) (nXMousePos - fXDragOffset) / MOUSE_TRANSLATE_SCALE, 0.0f);

		fTranslateMomentum = 1.0f;

		fXDragOffset = (float) nXMousePos;
		fYDragOffset = (float) nYMousePos;
		break;
	case DRAGTYPE_ROTATE:
		pTopology->ChangeView ((float) (nXMousePos - fXDragOffset) / MOUSE_ROTATE_SCALE,
							   (float) (nYMousePos - fYDragOffset) / MOUSE_ROTATE_SCALE, 0.0f, 0.0f);

		fSpinMomentum = 1.0f;

		fXDragOffset = (float) nXMousePos;
		fYDragOffset = (float) nYMousePos;
		break;
	default:
		break;
	}

	eDragType = DRAGTYPE_INVALID;
	pTopology->psRenderGeo->boShowGhostLink = FALSE;
}

#define DRAG_OPENMENU (1)

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::RightButtonUp (int nXMousePos, int nYMousePos) {
	cTopologyNode*	psNodeFound;
	GLdouble fX;
	GLdouble fY;
	GLdouble fZ;
	EventNewLink*  psLinkData;
	EventMoveNode*	psNodeMoveData;
	SelectObj sObjectsSelected;
	unsigned int uTime;
	char szText[EVENT_NAME_LEN];
	HMENU hPopUpMenu;

	sObjectsSelected = pTopology->SelectMap (nXMousePos, nYMousePos);
	pTopology->ConvertCoords (nXMousePos, nYMousePos, sObjectsSelected.fZPos, &fX, &fY, &fZ);

	switch (eDragType) {
	case DRAGTYPE_BOXMOVE:
		if (pTopology->psSelectedNode) {
			pTopology->MoveNode (pTopology->psSelectedNode, (float) (fXDragOffset + fX), (float) (fYDragOffset + fY),
								 0.0);
			UpdateAdHocLinks (pTopology->psSelectedNode);
			pTopology->RedrawTopology ();
			ResourceBeep (IDR_SOUNDMEDIUMBEEP);

			uTime = pTopology->GetTimer ()->GetTime ();
			psNodeMoveData = new EventMoveNode ();
			psNodeMoveData->nNodeID = pTopology->psSelectedNode->getID ();
			psNodeMoveData->fXPosTo = (float) (fXDragOffset + fX);
			psNodeMoveData->fYPosTo = (float) (fYDragOffset + fY);
			psNodeMoveData->fZPosTo = 0.0;
			psNodeMoveData->psWindow = this;
			_snprintf (szText, EVENT_NAME_LEN, "Move node %d", psNodeMoveData->nNodeID);
			pTopology->GetTimer ()->AddEvent (szText, uDragStartTime, (uTime - uDragStartTime), psNodeMoveData);
		}

		break;
	case DRAGTYPE_LINKSET:
		if (pTopology->psSelectedNode) {

			/*
			 * Find out if we arrived somewhere ;
			 * Find the box clicked in, if there was one
			 */
			sObjectsSelected = pTopology->SelectMain (nXMousePos, nYMousePos);
			psNodeFound = pTopology->DecodeNodeNumber (sObjectsSelected.nNode);

			if (psNodeFound && (psNodeFound != pTopology->psSelectedNode)) {

				/* Connect the nodes together */
				psLinkData = new EventNewLink ();
				psLinkData->nFrom = pTopology->psSelectedNode->getID ();;
				psLinkData->nTo = sObjectsSelected.nNode;
				psLinkData->nLinkID = pTopology->GetNextLinkID ();
				psLinkData->psWindow = this;

				_snprintf (szText, EVENT_NAME_LEN, "Create link from node %d to %d", psLinkData->nFrom, psLinkData->nTo);
				pTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NOW, 0, psLinkData);
			}
		}

		break;
	case DRAGTYPE_TRANSLATE:
		// If the mouse hasn't moved, open the menu
		if (boDrag == false) {
			LeftButtonDownSelect (nXMousePos, nYMousePos);
			CancelDrag ();
			hPopUpMenu = GetSubMenu (hMainMenuContext, 0);
			SetMenuDefaultItem (hPopUpMenu, ID_MENU_SHOW, FALSE);
			GetWindowRect (hwnd, &winRect);
			TrackPopupMenuEx (hPopUpMenu, (TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON | TPM_HORIZONTAL),
								nXMousePos + winRect.left + MAINRCLICKMENU_XOFFSET,
								nYMousePos + winRect.top + MAINRCLICKMENU_YOFFSET, hwnd, NULL);
		}
		else {
			pTopology->MoveCentre (-(float) (nYMousePos - fYDragOffset) / MOUSE_TRANSLATE_SCALE,
									 (float) (nXMousePos - fXDragOffset) / MOUSE_TRANSLATE_SCALE, 0.0f);
			fTranslateMomentum = 1.0f;
		}

		fXDragOffset = (float) nXMousePos;
		fYDragOffset = (float) nYMousePos;
		break;
	case DRAGTYPE_ROTATE:
		pTopology->ChangeView ((float) (nXMousePos - fXDragOffset) / MOUSE_ROTATE_SCALE,
							   (float) (nYMousePos - fYDragOffset) / MOUSE_ROTATE_SCALE, 0.0f, 0.0f);

		fSpinMomentum = 1.0f;

		fXDragOffset = (float) nXMousePos;
		fYDragOffset = (float) nYMousePos;
		break;
	default:
		break;
	}

	eDragType = DRAGTYPE_INVALID;
	pTopology->psRenderGeo->boShowGhostLink = FALSE;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::SpinMomentum (void) {
	EventChangeView*  psData;
	char szText[EVENT_NAME_LEN];
	unsigned int uTime;

	uTime = pTopology->GetTimer ()->GetTime ();

	if (eDragType == DRAGTYPE_ROTATE) {
		fSpinMomentumXComponent = 0.0f;
		fSpinMomentumYComponent = 0.0f;
		fSpinMomentumZComponent = 0.0f;
	}

	if ((fSpinMomentum > 0.0f) && (abs (fSpinMomentumXComponent * fSpinMomentum) < SPINMOMENTUM_STOPX)
		&& (abs (fSpinMomentumYComponent * fSpinMomentum) < SPINMOMENTUM_STOPY)
		&& (abs (fSpinMomentumZComponent * fSpinMomentum) < SPINMOMENTUM_STOPZ)) {
		cOptions*  opt;

		/* Stop the spinning */
		fSpinMomentum = 0.0f;

		opt = cOptions::create ();
		if (opt->getRecordViewEvents ()) {

			/* Add a move camera event to the event list */
			psData = new EventChangeView ();
			pTopology->GetView (&psData->fTheta, &psData->fPhi, &psData->fPsi, &psData->fRadius);
			psData->psWindow = this;
			_snprintf (szText, EVENT_NAME_LEN, "Change view rotation");
			pTopology->GetTimer ()->AddEvent (szText, uDragStartTime, (uTime - uDragStartTime), psData);
		}
	} else {
		pTopology->ChangeView ((fSpinMomentumXComponent * fSpinMomentum), (fSpinMomentumYComponent * fSpinMomentum),
							   (fSpinMomentumZComponent * fSpinMomentum), 0.0f);
		fSpinMomentum *= SPINMOMENTUM_RESISTANCE;
	}

	if ((uScrollWheelTimeStart > 0) && (uTime > (uScrollWheelTimeLast + SCROLLWHEEL_EVENTDELAY))) {
		cOptions*  opt;
		opt = cOptions::create ();

		if (opt->getRecordViewEvents ()) {

			/* Add a move camera event to the event list */
			psData = new EventChangeView ();
			pTopology->GetView (&psData->fTheta, &psData->fPhi, &psData->fPsi, &psData->fRadius);
			psData->psWindow = this;
			_snprintf (szText, EVENT_NAME_LEN, "Change view zoom");
			pTopology->GetTimer ()->AddEvent (szText, uScrollWheelTimeStart, (uTime - uScrollWheelTimeStart), psData);
		}

		uScrollWheelTimeStart = 0;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::TranslateMomentum (void) {
	EventTranslateView*  psData;
	char szText[EVENT_NAME_LEN];
	unsigned int uTime;

	uTime = pTopology->GetTimer ()->GetTime ();

	if (eDragType == DRAGTYPE_TRANSLATE) {
		fTranslateMomentumXComponent = 0.0f;
		fTranslateMomentumYComponent = 0.0f;
		fTranslateMomentumZComponent = 0.0f;
	}

	if ((fTranslateMomentum > 0.0f) && (abs (fTranslateMomentumXComponent * fTranslateMomentum) < TRANSLATEMOMENTUM_STOPX)
		&& (abs (fTranslateMomentumYComponent * fTranslateMomentum) < TRANSLATEMOMENTUM_STOPY)
		&& (abs (fTranslateMomentumZComponent * fTranslateMomentum) < TRANSLATEMOMENTUM_STOPZ)) {
		cOptions*  opt;

		/* Stop the spinning */
		fTranslateMomentum = 0.0f;

		opt = cOptions::create ();
		if (opt->getRecordViewEvents ()) {

			/* Add a translate camera event to the event list */
			psData = new EventTranslateView ();
			pTopology->GetCentre (&psData->fX, &psData->fY, &psData->fZ);
			psData->psWindow = this;
			_snprintf (szText, EVENT_NAME_LEN, "Change view translation");
			pTopology->GetTimer ()->AddEvent (szText, uDragStartTime, (uTime - uDragStartTime), psData);
		}
	} else {
		pTopology->MoveCentre (-(fTranslateMomentumYComponent * fTranslateMomentum), (fTranslateMomentumXComponent * fTranslateMomentum),
							   (fTranslateMomentumZComponent * fTranslateMomentum));
		fTranslateMomentum *= TRANSLATEMOMENTUM_RESISTANCE;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::ScrollWheel (int nZDelta) {
	pTopology->ChangeView (0.0f, 0.0f, 0.0f, (DELTA_ROT_SCROLL * (float) nZDelta / (float) WHEEL_DELTA));
	uScrollWheelTimeLast = pTopology->GetTimer ()->GetTime ();

	if (uScrollWheelTimeStart == 0) {
		uScrollWheelTimeStart = uScrollWheelTimeLast;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::MouseMove (int nXMousePos, int nYMousePos) {
	SelectObj sMapSelected;
	SelectObj sObjectSelected;
	cTopologyNode*	psDecodedNode;
	GLdouble fX;
	GLdouble fY;
	GLdouble fZ;

	boDrag = true;
	psDecodedNode = NULL;

	sMapSelected = pTopology->SelectMap (nXMousePos, nYMousePos);
	pTopology->ConvertCoords (nXMousePos, nYMousePos, sMapSelected.fZPos, &fX, &fY, &fZ);

	switch (eDragType) {
	case DRAGTYPE_BOXMOVE:
		if (pTopology->psSelectedNode) {
			pTopology->MoveNode (pTopology->psSelectedNode, (float) (fXDragOffset + fX), (float) (fYDragOffset + fY),
								 0.0f);
			UpdateAdHocLinks (pTopology->psSelectedNode);
			pTopology->RedrawTopology ();
		}

		break;
	case DRAGTYPE_LINKSET:
		if (pTopology->psSelectedNode) {
			sObjectSelected = pTopology->SelectMain (nXMousePos, nYMousePos);
			psDecodedNode = pTopology->DecodeNodeNumber (sObjectSelected.nNode);
			if (psDecodedNode && (psDecodedNode != pTopology->psSelectedNode)) {
				pTopology->psRenderGeo->SetGhostLinkEnd (psDecodedNode->fXPos, psDecodedNode->fYPos,
														 psDecodedNode->fZPos, TRUE);
			} else {
				pTopology->psRenderGeo->SetGhostLinkEnd (fX, fY, fZ, FALSE);
			}

			pTopology->RedrawTopology ();
		}

		break;
	case DRAGTYPE_TRANSLATE:
		pTopology->MoveCentre (-(float) (nYMousePos - fYDragOffset) / MOUSE_TRANSLATE_SCALE,
							   (float) (nXMousePos - fXDragOffset) / MOUSE_TRANSLATE_SCALE, 0.0f);

		fTranslateMomentumXComponent = (float) (nXMousePos - fXDragOffset) / MOUSE_TRANSLATE_SCALE;
		fTranslateMomentumYComponent = (float) (nYMousePos - fYDragOffset) / MOUSE_TRANSLATE_SCALE;
		fTranslateMomentumZComponent = 0.0f;

		fXDragOffset = (float) nXMousePos;
		fYDragOffset = (float) nYMousePos;
		break;
	case DRAGTYPE_ROTATE:
		pTopology->ChangeView ((float) (nXMousePos - fXDragOffset) / MOUSE_ROTATE_SCALE,
							   (float) (nYMousePos - fYDragOffset) / MOUSE_ROTATE_SCALE, 0.0f, 0.0f);

		fSpinMomentumXComponent = (float) (nXMousePos - fXDragOffset) / MOUSE_ROTATE_SCALE;
		fSpinMomentumYComponent = (float) (nYMousePos - fYDragOffset) / MOUSE_ROTATE_SCALE;
		fSpinMomentumZComponent = 0.0f;

		fXDragOffset = (float) nXMousePos;
		fYDragOffset = (float) nYMousePos;
	default:
		break;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cWindow::Select (int nXMousePos, int nYMousePos) {
	SelectObj sObjectSelected;
	GLdouble fX;
	GLdouble fY;
	GLdouble fZ;
	bool boFound;
	cTopologyNode*	psDecodedNode;
	cTopologyLink*	psDecodeLink;
	cOptions*  psOptions;

	boFound = false;
	psDecodedNode = NULL;
	psDecodeLink = NULL;
	psOptions = NULL;

	psOptions = cOptions::create ();

	sObjectSelected = pTopology->SelectMain (nXMousePos, nYMousePos);
	pTopology->ConvertCoords (nXMousePos, nYMousePos, sObjectSelected.fZPos, &fX, &fY, &fZ);

	/* Find the box clicked in, if there was one */
	pTopology->psSelectedNode = NULL;
	psDecodedNode = pTopology->DecodeNodeNumber (sObjectSelected.nNode);
	if (psDecodedNode) {
		pTopology->psSelectedLink = NULL;
		if ((psOptions->getScreenShotMode ()) && (pTopology->psSelectedNode == psDecodedNode)) {
			pTopology->psSelectedNode = NULL;
		} else {
			pTopology->psSelectedNode = psDecodedNode;

			fXDragOffset = pTopology->psSelectedNode->fXPos - fX;
			fYDragOffset = pTopology->psSelectedNode->fYPos - fY;
			fZDragOffset = pTopology->psSelectedNode->fZPos - fZ;
			eDragType = DRAGTYPE_BOXMOVE;
			uDragStartTime = pTopology->GetTimer ()->GetTime ();
			boFound = true;
		}
	} else {
		psDecodeLink = pTopology->DecodeLinkNumber (sObjectSelected.nLink);
		if (psDecodeLink) {
			pTopology->psSelectedNode = NULL;
			pTopology->psSelectedLink = psDecodeLink;
			pTopology->psSelectedLink->bringToFront ();
			boFound = true;
		}
	}

	pTopology->RedrawTopology ();

	return boFound;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::LeftButtonDownSelect (int nXMousePos, int nYMousePos) {
	if (!pTopology->ClickButton (nXMousePos, nYMousePos)) {
		Select (nXMousePos, nYMousePos);
	} else {
		ResourceBeep (IDR_SOUNDSHORTBEEP);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::LeftButtonDownSelectRotate (int nXMousePos, int nYMousePos) {
	bool boFound;
	SHORT nCtrl;

	if (!pTopology->ClickButton (nXMousePos, nYMousePos)) {
		boFound = Select (nXMousePos, nYMousePos);

		if (boFound) {
			ResourceBeep (IDR_SOUNDMEDIUMBEEP);
		} else {
			fXDragOffset = (float) nXMousePos;
			fYDragOffset = (float) nYMousePos;
			nCtrl = GetAsyncKeyState (VK_CONTROL);
			if (nCtrl) {
				eDragType = DRAGTYPE_TRANSLATE;
				fTranslateMomentum = 0.0f;
			}
			else {
				eDragType = DRAGTYPE_ROTATE;
				fSpinMomentum = 0.0f;
			}
			uDragStartTime = pTopology->GetTimer ()->GetTime ();
		}
	} else {
		ResourceBeep (IDR_SOUNDSHORTBEEP);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::RightButtonDownTranslate (int nXMousePos, int nYMousePos) {
	fXDragOffset = (float) nXMousePos;
	fYDragOffset = (float) nYMousePos;
	eDragType = DRAGTYPE_TRANSLATE;
	fTranslateMomentum = 0.0f;
	uDragStartTime = pTopology->GetTimer ()->GetTime ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::LeftButtonDownCreateLink (int nXMousePos, int nYMousePos) {
	SelectObj sObjectSelected;
	GLdouble fX;
	GLdouble fY;
	GLdouble fZ;

	if (!pTopology->ClickButton (nXMousePos, nYMousePos)) {
		sObjectSelected = pTopology->SelectMain (nXMousePos, nYMousePos);
		pTopology->ConvertCoords (nXMousePos, nYMousePos, sObjectSelected.fZPos, &fX, &fY, &fZ);

		/* Find the box clicked in, if there was one */
		pTopology->psSelectedNode = pTopology->DecodeNodeNumber (sObjectSelected.nNode);
		if (pTopology->psSelectedNode) {
			pTopology->psRenderGeo->SetGhostLinkEnd (fX, fY, 0.0f, FALSE);
			pTopology->psRenderGeo->boShowGhostLink = TRUE;
			eDragType = DRAGTYPE_LINKSET;
			uDragStartTime = pTopology->GetTimer ()->GetTime ();
			pTopology->RedrawTopology ();
		} else {
			LeftButtonDownRotate (nXMousePos, nYMousePos);
		}
	} else {
		ResourceBeep (IDR_SOUNDSHORTBEEP);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::LeftButtonDownRotate (int nXMousePos, int nYMousePos) {
	SHORT nCtrl;

	if (!pTopology->ClickButton (nXMousePos, nYMousePos)) {
		fXDragOffset = (float) nXMousePos;
		fYDragOffset = (float) nYMousePos;

		nCtrl = GetAsyncKeyState (VK_CONTROL);
		if (nCtrl) {
			eDragType = DRAGTYPE_TRANSLATE;
			fTranslateMomentum = 0.0f;
		}
		else {
			eDragType = DRAGTYPE_ROTATE;
			fSpinMomentum = 0.0f;
		}

		uDragStartTime = pTopology->GetTimer ()->GetTime ();
	} else {
		ResourceBeep (IDR_SOUNDSHORTBEEP);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::CreateNode (int nNodeID, float fXPos, float fYPos, float fZPos, int nType) {
	char szName[TOPOLOGYNODE_NAMLEN];
	cTopologyNode * pcNewNode;
	char const * szType;

	newVM (true, nNodeID);

	pcNewNode = pList->getCurrentNode (psTabIterator);

	/*
	 * ShowTopologyTab ();
	 * pList->endIterator ();
	 */
	pcNewNode->fXPos = fXPos;
	pcNewNode->fYPos = fYPos;
	pcNewNode->fZPos = fZPos;
	pcNewNode->nType = nType;

	// Assign the default properties to the node
	pcNewNode->PropertyRemoveAll ();
	pcNewNode->PropertyCopy (pcDefaultProperties);
	pcNewNode->PropertySetOptionEnum ("Type", nType);

	szType = pcNewNode->PropertyGetOption ("Type");
	_snprintf (szName, sizeof (szName), "%s Node", szType);
	pcNewNode->SetName (szName);

	ResourceBeep (IDR_SOUNDCREATENODE);

	pTopology->psSelectedNode = pcNewNode;
	pTopology->psSelectedLink = NULL;

	pTopology->RedrawTopology ();

	/* Notify Dandelion */
	_snprintf (szName, sizeof (szName), "%d", pcNewNode->getID ());
	if (psDlClient) {
		psDlClient->AddNode (szName);
	}

	pcNewNode->boAdHoc = true;
	pcNewNode->fRangeRx = 0.0f / ADHOC_SCALE;
	pcNewNode->fRangeTx = 0.0f / ADHOC_SCALE;

	/* Sort out any ad hoc links */
	UpdateAdHocLinks (pcNewNode);

	/* Set to analyse */
	TopologyChanged ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::TopologyChanged (void) {
	boTopologyChanged = true;;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::LeftButtonDownCreateNode (int nXMousePos, int nYMousePos, int nType) {
	EventNewNode*  psData;
	SelectObj sObjectsSelected;
	GLdouble fX;
	GLdouble fY;
	GLdouble fZ;
	char szText[EVENT_NAME_LEN];

	if (!pTopology->ClickButton (nXMousePos, nYMousePos)) {
		sObjectsSelected = pTopology->SelectMap (nXMousePos, nYMousePos);
		pTopology->ConvertCoords (nXMousePos, nYMousePos, sObjectsSelected.fZPos, &fX, &fY, &fZ);

		psData = new EventNewNode ();
		psData->nNodeID = pList->getNextID ();
		psData->fXPos = (float) fX;
		psData->fYPos = (float) fY;
		psData->fZPos = 0.0f;
		psData->nType = nType;
		psData->psWindow = this;
		_snprintf (szText, EVENT_NAME_LEN, "Create node %d", psData->nNodeID);
		pTopology->GetTimer ()->AddEvent (szText, TIMER_TIME_NOW, 0, psData);
	} else {
		ResourceBeep (IDR_SOUNDSHORTBEEP);
	}
}

/*
 =======================================================================================================================
 *  void cWindow::ShowTopologyTab (void) ;
 *  { ;
 *  pConsole->setVM (NULL);
 *  TabCtrl_SetCurSel (vmTabList, 0);
 *  pConsole->activeID = TabCtrl_GetCurSel (vmTabList);
 *  changeTab ();
 *  } ;
 =======================================================================================================================
 */
void cWindow::SetMenuTick (HWND hWnd, UINT uMenuID, bool boTick) {
	MENUITEMINFO sMenuItem;

	sMenuItem.cbSize = sizeof (MENUITEMINFO);
	sMenuItem.fMask = MIIM_STATE;

	/*
	 * GetMenuItemInfo (hMainMenuTop, uMenuID, FALSE, & sMenuItem);
	 */
	if (boTick) {
		sMenuItem.fState = MFS_CHECKED;
	} else {
		sMenuItem.fState = MFS_UNCHECKED;
	}

	SetMenuItemInfo (hMainMenuTop, uMenuID, FALSE, &sMenuItem);
	SetMenuItemInfo (hMainMenuContext, uMenuID, FALSE, &sMenuItem);
}

/*
 =======================================================================================================================
 *  Enable OpenGL
 =======================================================================================================================
 */
void cWindow::EnableOpenGL (HWND hWnd, HDC* hDC, HGLRC* hRC) {
	PIXELFORMATDESCRIPTOR pfd;
	int format;

	PIXELFORMATDESCRIPTOR pfd2;
	int npf;
	char const*	 szVendor;

	/* get the device context (DC) */
	*hDC = GetDC (hWnd);

	/* set the pixel format for the DC */
	ZeroMemory (&pfd, sizeof (pfd));
	pfd.nSize = sizeof (pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_GENERIC_ACCELERATED | PFD_TYPE_RGBA;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	format = ChoosePixelFormat (*hDC, &pfd);
	SetPixelFormat (*hDC, format, &pfd);

	npf = GetPixelFormat (*hDC);

	DescribePixelFormat (*hDC, npf, sizeof (pfd2), &pfd2);

	/* create and enable the render context (RC) */
	*hRC = wglCreateContext (*hDC);
	wglMakeCurrent (*hDC, *hRC);

	szVendor = (char const*) glGetString (GL_VENDOR);
}

/*
 =======================================================================================================================
 *  Disable OpenGL
 =======================================================================================================================
 */
void cWindow::DisableOpenGL (HWND hWnd, HDC hDC, HGLRC hRC) {
	wglMakeCurrent (NULL, NULL);
	wglDeleteContext (hRC);
	ReleaseDC (hWnd, hDC);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::SetViewTick (void) {
	LRESULT nResult;

	/* Set the menu ticks */
	SetMenuTick (hwnd, ID_VIEW_SELECT, (geViewState == VIEWSTATE_SELECT));
	SetMenuTick (hwnd, ID_VIEW_ROTATE, (geViewState == VIEWSTATE_ROTATE));
	SetMenuTick (hwnd, ID_VIEW_CREATE_HOSPITAL, (geViewState == VIEWSTATE_CREATENODE));
	//SetMenuTick (hwnd, ID_VIEW_CREATE_POLICE, (geViewState == VIEWSTATE_CREATE_POLICE));
	//SetMenuTick (hwnd, ID_VIEW_CREATE_BANK, (geViewState == VIEWSTATE_CREATE_BANK));
	//SetMenuTick (hwnd, ID_VIEW_CREATE_EVENT, (geViewState == VIEWSTATE_CREATE_EVENT));
	//SetMenuTick (hwnd, ID_VIEW_CREATE_MOBILE, (geViewState == VIEWSTATE_CREATE_MOBILE));
	//SetMenuTick (hwnd, ID_VIEW_CREATE_RESCUE, (geViewState == VIEWSTATE_CREATE_RESCUE));
	//SetMenuTick (hwnd, ID_VIEW_CREATE_FIRE, (geViewState == VIEWSTATE_CREATE_FIRE));
	//SetMenuTick (hwnd, ID_VIEW_CREATE_AMBULANCE, (geViewState == VIEWSTATE_CREATE_AMBULANCE));
	//SetMenuTick (hwnd, ID_VIEW_CREATE_TRANSPORT, (geViewState == VIEWSTATE_CREATE_TRANSPORT));
	//SetMenuTick (hwnd, ID_VIEW_CREATE_EXTERNAL, (geViewState == VIEWSTATE_CREATE_EXTERNAL));
	SetMenuTick (hwnd, ID_VIEW_CREATELINK, (geViewState == VIEWSTATE_CREATELINK));
	SetMenuTick (hwnd, ID_VIEW_SPIN, gboSpin);
	SetMenuTick (hwnd, ID_VIEW_OVERLAY, (pTopology->GetOverlayStatus ()));
	SetMenuTick (hwnd, ID_VIEW_RENDERTYPE, ((pTopology->eRenderType == RENDERTYPE_NET) ^ boNextRenderType));
	SetMenuTick (hwnd, ID_FULLSCREEN, boFullScreen);

	/* Set the toolbar button depressions */
	nResult = SendMessage (gwhToolbar, TB_CHECKBUTTON, ID_VIEW_SELECT, MAKELONG ((geViewState == VIEWSTATE_SELECT), 0));
	nResult = SendMessage (gwhToolbar, TB_CHECKBUTTON, ID_VIEW_ROTATE, MAKELONG ((geViewState == VIEWSTATE_ROTATE), 0));
	nResult = SendMessage (gwhToolbar, TB_CHECKBUTTON, ID_VIEW_CREATE_HOSPITAL,
						   MAKELONG ((geViewState == VIEWSTATE_CREATENODE), 0));
	//nResult = SendMessage (gwhToolbar, TB_CHECKBUTTON, ID_VIEW_CREATE_POLICE,
	//					   MAKELONG ((geViewState == VIEWSTATE_CREATE_POLICE), 0));
	//nResult = SendMessage (gwhToolbar, TB_CHECKBUTTON, ID_VIEW_CREATE_BANK,
	//					   MAKELONG ((geViewState == VIEWSTATE_CREATE_BANK), 0));
	//nResult = SendMessage (gwhToolbar, TB_CHECKBUTTON, ID_VIEW_CREATE_EVENT,
	//					   MAKELONG ((geViewState == VIEWSTATE_CREATE_EVENT), 0));
	//nResult = SendMessage (gwhToolbar, TB_CHECKBUTTON, ID_VIEW_CREATE_MOBILE,
	//					   MAKELONG ((geViewState == VIEWSTATE_CREATE_MOBILE), 0));
	//nResult = SendMessage (gwhToolbar, TB_CHECKBUTTON, ID_VIEW_CREATE_RESCUE,
	//					   MAKELONG ((geViewState == VIEWSTATE_CREATE_RESCUE), 0));
	//nResult = SendMessage (gwhToolbar, TB_CHECKBUTTON, ID_VIEW_CREATE_FIRE,
	//					   MAKELONG ((geViewState == VIEWSTATE_CREATE_FIRE), 0));
	//nResult = SendMessage (gwhToolbar, TB_CHECKBUTTON, ID_VIEW_CREATE_AMBULANCE,
	//					   MAKELONG ((geViewState == VIEWSTATE_CREATE_AMBULANCE), 0));
	//nResult = SendMessage (gwhToolbar, TB_CHECKBUTTON, ID_VIEW_CREATE_TRANSPORT,
	//					   MAKELONG ((geViewState == VIEWSTATE_CREATE_TRANSPORT), 0));
	//nResult = SendMessage (gwhToolbar, TB_CHECKBUTTON, ID_VIEW_CREATE_EXTERNAL,
	//					   MAKELONG ((geViewState == VIEWSTATE_CREATE_EXTERNAL), 0));

	nResult = SendMessage (gwhToolbar, TB_CHECKBUTTON, ID_VIEW_CREATELINK,
						   MAKELONG ((geViewState == VIEWSTATE_CREATELINK), 0));
	nResult = SendMessage (gwhToolbar, TB_CHECKBUTTON, ID_VIEW_SPIN, MAKELONG (gboSpin, 0));
	nResult = SendMessage (gwhToolbar, TB_CHECKBUTTON, ID_VIEW_OVERLAY, MAKELONG ((pTopology->GetOverlayStatus ()), 0));

	/* Set the main window HUD button depressions */
	pTopology->aboButtonPressed[BUTTON_SELECT] = (geViewState == VIEWSTATE_SELECT);
	pTopology->aboButtonPressed[BUTTON_LINKS] = (geViewState == VIEWSTATE_CREATELINK);
	pTopology->aboButtonPressed[BUTTON_SPIN] = gboSpin;
	pTopology->aboButtonPressed[BUTTON_NET] = ((pTopology->eRenderType == RENDERTYPE_NET) ^ boNextRenderType);
	pTopology->aboButtonPressed[BUTTON_DETAILS] = (pTopology->GetOverlayStatus ());
	pTopology->aboButtonPressed[BUTTON_LEFT] = false;
	pTopology->aboButtonPressed[BUTTON_NODES] = (((int) geViewState >= 2) && ((int) geViewState <= 11));
	pTopology->aboButtonPressed[BUTTON_RIGHT] = false;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::ResourceBeep (unsigned int nResourceID) {
	float fSoundVolume;
	cOptions*  options;

	options = NULL;
	options = cOptions::create ();
	fSoundVolume = options->getSoundVolume ();

	if (fSoundVolume > 0.0f) {
		PlaySound (MAKEINTRESOURCE (nResourceID), NULL, SND_RESOURCE | SND_ASYNC);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::SetVolume (float fVolume) {
	cOptions*  options;

	options = NULL;

	options = cOptions::create ();
	options->setSoundVolume (fVolume);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::saveToXML () {
	char szSelected[_MAX_PATH];
	char szFilter[255];
	OPENFILENAME sOfn;
	char*  szConvert;
	char szExt[32];
	BOOL boOkay;

	/* Open the Save As dialogue box */
	ZeroMemory (&sOfn, sizeof (OPENFILENAME));

	sOfn.lStructSize = sizeof (OPENFILENAME);
	sOfn.hwndOwner = hwnd;
	sOfn.hInstance = 0;

	strcpy (szFilter, "xml files (*.xml)\n*.xml\nAll files (*.*)\n*.*\n\n");

	szConvert = strchr (szFilter, '\n');
	while (szConvert) {
		*szConvert = '\0';
		szConvert = strchr (szConvert + 1, '\n');
	}

	sOfn.lpstrFilter = szFilter;
	sOfn.lpstrCustomFilter = NULL;
	sOfn.nMaxCustFilter = 0;
	sOfn.nFilterIndex = 0;
	strcpy (szSelected, "");
	sOfn.lpstrFile = szSelected;
	sOfn.nMaxFile = sizeof (szSelected);
	sOfn.lpstrFileTitle = NULL;
	sOfn.nMaxFileTitle = 0;
	sOfn.lpstrInitialDir = NULL;
	sOfn.lpstrTitle = NULL;

	sOfn.Flags = (OFN_ENABLESIZING | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR);

	strcpy (szExt, "xml");
	sOfn.lpstrDefExt = szExt;

	boOkay = GetSaveFileName (&sOfn);

	if (boOkay) {
		/*
		 * Saving out using the cFileHub routines
		 * is currently disabled while it's being worked on
		 */
		psFileHub->Save (szSelected);
	}	/* End of boOkay */
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::loadFromXML () {
	char szSelected[_MAX_PATH];
	char szFilter[255];
	OPENFILENAME sOfn;
	char*  szConvert;
	char szExt[32];
	BOOL boOkay;

	/* Open the Save As dialogue box */
	ZeroMemory (&sOfn, sizeof (OPENFILENAME));

	sOfn.lStructSize = sizeof (OPENFILENAME);
	sOfn.hwndOwner = hwnd;
	sOfn.hInstance = 0;

	strcpy (szFilter, "xml files (*.xml)\n*.xml\nAll files (*.*)\n*.*\n\n");
	szConvert = strchr (szFilter, '\n');
	while (szConvert) {
		*szConvert = '\0';
		szConvert = strchr (szConvert + 1, '\n');
	}

	sOfn.lpstrFilter = szFilter;

	sOfn.lpstrCustomFilter = NULL;
	sOfn.nMaxCustFilter = 0;
	sOfn.nFilterIndex = 0;
	strcpy (szSelected, "");
	sOfn.lpstrFile = szSelected;
	sOfn.nMaxFile = sizeof (szSelected);
	sOfn.lpstrFileTitle = NULL;
	sOfn.nMaxFileTitle = 0;
	sOfn.lpstrInitialDir = NULL;
	sOfn.lpstrTitle = "Open Node XML File";

	sOfn.Flags = (OFN_ENABLESIZING | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR);

	strcpy (szExt, "xml");
	sOfn.lpstrDefExt = szExt;

	boOkay = GetOpenFileName (&sOfn);

	if (boOkay) {
		DeleteTopology ();
		pTopology->ResetNextID ();
		pTopology->ClearTimerEvents ();
		psFileHub->Load (szSelected);

		///* Notify Dandelion */
		//_snprintf (szName, sizeof (szName), "%d", (pList->getCurrentNode (psTabIterator))->getID ());
		//if (psDlClient) {
		//	psDlClient->AddNode (szName);
		//}

		///* Notify Dandelion */
		//_snprintf (szFrom, sizeof (szFrom), "%d", linkRecord[i].nOutId);
		//_snprintf (szTo, sizeof (szTo), "%d", linkRecord[i].nInId);
		//if (psDlClient) {
		//	psDlClient->AddLink (szFrom, szTo);
		//}

		/* Set to analyse */
		TopologyChanged ();

		redraw ();
	}	/* end of if boOkay */
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::loadScene() {
	char szSelected[_MAX_PATH];
	char szFilter[255];
	OPENFILENAME sOfn;
	char*  szConvert;
	char szExt[32];
	BOOL boOkay;

	/* Open the Save As dialogue box */
	ZeroMemory (&sOfn, sizeof (OPENFILENAME));

	sOfn.lStructSize = sizeof (OPENFILENAME);
	sOfn.hwndOwner = hwnd;
	sOfn.hInstance = 0;

	strcpy (szFilter, "xml files (*.xml)\n*.xml\nAll files (*.*)\n*.*\n\n");
	szConvert = strchr (szFilter, '\n');
	while (szConvert) {
		*szConvert = '\0';
		szConvert = strchr (szConvert + 1, '\n');
	}

	sOfn.lpstrFilter = szFilter;

	sOfn.lpstrCustomFilter = NULL;
	sOfn.nMaxCustFilter = 0;
	sOfn.nFilterIndex = 0;
	strcpy (szSelected, "");
	sOfn.lpstrFile = szSelected;
	sOfn.nMaxFile = sizeof (szSelected);
	sOfn.lpstrFileTitle = NULL;
	sOfn.nMaxFileTitle = 0;
	sOfn.lpstrInitialDir = NULL;
	sOfn.lpstrTitle = "Open Scenery XML file";

	sOfn.Flags = (OFN_ENABLESIZING | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR);

	strcpy (szExt, "xml");
	sOfn.lpstrDefExt = szExt;

	boOkay = GetOpenFileName (&sOfn);

	if (boOkay) {
		pTopology->psRenderGeo->LoadFile (szSelected);

		redraw ();
	}	/* end of if boOkay */
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::DeleteTopology () {
	cTopologyNode*	psIterator;

	pTopology->psSelectedLink = NULL;
	pTopology->psSelectedNode = NULL;
	psIterator = pList->startIterator ();
	while (pList->isIteratorValid (psIterator)) {
		psIterator = pList->removeCurrentItem (psIterator);
		psIterator = pList->startIterator ();
	}

	if (psDlClient) {
		psDlClient->Clear ();
	}

	/* Set to analyse */
	TopologyChanged ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::ResetTime () {
	float fTheta = (3.0f * 3.14159265 / 2.0f);
	float fPhi = (3.14159265 / 2.0f);
	float fPsi = 0.0f;
	float fRadius = 6.2f;

	/* Clear the existing topolog */
	DeleteTopology ();
	CancelDrag ();
	pTopology->psSelectedNode = NULL;
	pTopology->psSelectedLink = NULL;

	pTopology->ViewOffset (&fTheta, &fPhi, &fPsi, &fRadius);
	pTopology->ChangeView (fTheta, fPhi, fPsi, fRadius);
	pTopology->SetCentre (0.0f, 0.0f, 0.0f);

	/* Reset the event timeline */
	pTopology->GetTimer ()->ResetTime ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::UpdateAdHocLinks (cTopologyNode* psNode) {
	float fX1;
	float fY1;
	float fZ1;
	float fX2;
	float fY2;
	float fZ2;
	float fDistSquared;
	cTopologyLink*	psLink;
	cTopologyLink*	psLinkFound;
	cTopologyNode*	psNodeLinked;
	int nLinkID;
	cTopologyNode*	psIterator;

	if (pList) {
		fX1 = psNode->fXPos;
		fY1 = psNode->fYPos;
		fZ1 = psNode->fZPos;

		if (psNode->boAdHoc) {
			psIterator = pList->startIterator ();

			while (pList->isIteratorValid (psIterator)) {
				psNodeLinked = pList->getCurrentNode (psIterator);
				if (psNodeLinked->boAdHoc) {
					fX2 = pList->getXPos (psIterator);
					fY2 = pList->getYPos (psIterator);
					fZ2 = pList->getZPos (psIterator);

					fDistSquared = ((fX1 - fX2) * (fX1 - fX2)) + ((fY1 - fY2) * (fY1 - fY2)) + ((fZ1 - fZ2) * (fZ1 - fZ2));

					if (psNodeLinked != psNode) {
						if (((fDistSquared) < (psNode->fRangeTx * psNode->fRangeTx))
							|| ((fDistSquared) < (psNodeLinked->fRangeRx * psNodeLinked->fRangeRx))) {

							/* Add new link if there isn't one already */
							psLink = psNode->pLinksOut;
							psLinkFound = NULL;
							while ((psLink) && (psLinkFound == NULL)) {
								if ((psLink->boAdHoc) && (psLink->psLinkTo == psNodeLinked)) {
									psLinkFound = psLink;
								}

								psLink = psLink->pFromNext;
							}

							if (psLinkFound == NULL) {

								/* Create a new link */
								nLinkID = pTopology->GetNextLinkID ();

								/* Note that we don't add this in to the timer */
								CreateLink (psNode->getID (), psNodeLinked->getID (), nLinkID);
								if (pTopology->GetSelectedLink ()) {
									pTopology->GetSelectedLink ()->boAdHoc = true;
								}
							}
						} else {

							/*
							 * Remove any existing ad hoc links ;
							 * Add new link if there isn't one already
							 */
							psLink = psNode->pLinksOut;
							while (psLink) {
								if ((psLink->boAdHoc) && (psLink->psLinkTo == psNodeLinked)) {

									/* Remove the link */
									psLinkFound = psLink;
									psLink = psLink->pFromNext;
									DeleteLink (psNode->getID (), psLinkFound->nLinkID);
								} else {
									psLink = psLink->pFromNext;
								}
							}
						}

						if (((fDistSquared) < (psNodeLinked->fRangeTx * psNodeLinked->fRangeTx))
							|| ((fDistSquared) < (psNode->fRangeRx * psNode->fRangeRx))) {

							/* Add new link if there isn't one already */
							psLink = psNodeLinked->pLinksOut;
							psLinkFound = NULL;
							while ((psLink) && (psLinkFound == NULL)) {
								if ((psLink->boAdHoc) && (psLink->psLinkTo == psNode)) {
									psLinkFound = psLink;
								}

								psLink = psLink->pFromNext;
							}

							if (psLinkFound == NULL) {

								/* Create a new link */
								nLinkID = pTopology->GetNextLinkID ();

								/* Note that we don't add this in to the timer */
								CreateLink (psNodeLinked->getID (), psNode->getID (), nLinkID);
								if (pTopology->GetSelectedLink ()) {
									pTopology->GetSelectedLink ()->boAdHoc = true;
								}
							}
						} else {

							/*
							 * Remove any existing ad hoc links ;
							 * Add new link if there isn't one already
							 */
							psLink = psNodeLinked->pLinksOut;
							while (psLink) {
								if ((psLink->boAdHoc) && (psLink->psLinkTo == psNode)) {

									/* Remove the link */
									psLinkFound = psLink;
									psLink = psLink->pFromNext;
									DeleteLink (psNodeLinked->getID (), psLinkFound->nLinkID);
								} else {
									psLink = psLink->pFromNext;
								}
							}
						}
					}
				}

				psIterator = pList->moveForward (psIterator);
			}
		} else {

			/* Remove any ad-hoc links */
			psLink = psNode->pLinksOut;
			while (psLink) {
				if (psLink->boAdHoc) {

					/* Remove the link */
					psLinkFound = psLink;
					psLink = psLink->pFromNext;
					DeleteLink (psNode->getID (), psLinkFound->nLinkID);
				} else {
					psLink = psLink->pFromNext;
				}
			}

			psLink = psNode->pLinksIn;
			while (psLink) {
				if (psLink->boAdHoc) {

					/* Remove the link */
					psLinkFound = psLink;
					psLink = psLink->pFromNext;
					DeleteLink (psNode->getID (), psLinkFound->nLinkID);
				} else {
					psLink = psLink->pFromNext;
				}
			}
		}
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void runServer (cWindow* pWin) {
	pWin->psServer->RunServer (pWin);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::Disconnect () {
	psServer->StopServer ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void runServerDande (cWindow* pWin) {
	pWin->psServerDande->RunServer (pWin);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cWindow::DisconnectDande () {
	psServerDande->StopServer ();
}

/*
 =======================================================================================================================
 *  Populate a node list to send to the client
 =======================================================================================================================
 */
int cWindow::PopulateNodeList (char * szNodeList, int nLength) {
	cTopologyNode*	psNode;
	cTopologyNode*	psIterator;
	char szNodeID[32];

	if (szNodeList) {
		psIterator = pList->startIterator ();
		szNodeList[0] = 0;
		while (pList->isIteratorValid (psIterator)) {
			psNode = pList->getCurrentNode (psIterator);
			strcat (szNodeList, psNode->GetName ());

			/*
			 * sSend = sSend + sTemp;
			 */
			sprintf (szNodeID, "(%d);", psNode->getID ());
			strcat (szNodeList, szNodeID);

			/*
			 * delete sTemp;
			 */
			psIterator = pList->moveForward (psIterator);
		}
	}
	return (int)strlen (szNodeList);
}

void cWindow::LoadNodeTextures () {
	nNodeTypes = 0;
	nNodeCreateType = 0;
	nNodeTextureStart = pTopology->psTexture->GetFloatingNum () + (int)TEXNAME_NUM;

	LoadNodeTexture ("Resources\\Unknown.png");
	LoadNodeTexture ("Resources\\Company.png");
	LoadNodeTexture ("Resources\\Government.png");

	LoadNodeTexture ("Resources\\Gold.png");
	LoadNodeTexture ("Resources\\Silver.png");
	LoadNodeTexture ("Resources\\Bronze.png");

	LoadNodeTexture ("Resources\\Ambulance.png");
	LoadNodeTexture ("Resources\\FireEngine.png");
	LoadNodeTexture ("Resources\\Police.png");

	LoadNodeTexture ("Resources\\Fire.png");
	LoadNodeTexture ("Resources\\Water.png");
	LoadNodeTexture ("Resources\\Evacuation.png");
	LoadNodeTexture ("Resources\\Blank.png");
}

void cWindow::LoadNodeTexture (char * const szFilename) {
	pTopology->psTexture->LoadFloatingTexture (szFilename);
	nNodeTypes++;
}
