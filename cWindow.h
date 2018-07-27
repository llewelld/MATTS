/*$T MATTS/cWindow.h GC 1.140 07/01/09 21:12:10 */
/*
 * ;
 * Name: cList.h ;
 * Last Modified: 16/10/04 ;
 * ;
 * Purpose: Header file for Linked List ;
 * NOTE: none ;
 * Members ;
 * wndClass - Window class for creating main window ;
 * hwnd - handle to main window ;
 * vmTabList - handle to tab control child window ;
 * message - store for window's current message ;
 * winRect - window rectangle for resizing opperations ;
 * pList - pointer to the virtual machine list ;
 * hInstance - handle to instance of window ;
 * trayActive - True if the program is active in the system tray, false otherwise ;
 * notifyIconMenu - Menu for the tray icon ;
 * pTopology - pointer to the network topology info for the first tab ;
 * szNameList - char* used to store list of nodes' name and ID, requested by
 * network client ;
 * Methods ;
 * create() - Creates the main window ;
 * wndProc() - WndProc for main application ;
 * aboutProc() - WndProc for about dialog box ;
 * optionProc() - WndProc for options dialog box ;
 * policyProc() - WndProc for policy dialog box ;
 * run() - Runs the window ;
 * messageHandler()- Local message handler ;
 * ListenThread() - Thread for listening for agents ;
 * getConsole() - Returns a pointer to the console ;
 * getCurrentVM() - Returns a pointer to current VM ;
 * getList() - Returns a pointer to machine list ;
 * resize() - Resizes current window (including sys tray) ;
 * newVM() - Creates a new VM ;
 * closeVM() - Kills off the current VM ;
 * changeTab() - Changes active VM and tab ;
 * redraw() - Draws the window and all child windows ;
 * loadStateFile() - Loads a file for execution ;
 * saveStateFile() - Saves a program while in execution ;
 * cleanup() - Cleans up app for exit ;
 * assignDataProc - WndProc for assign data dialog box ;
 * sendDataProc - WndProc for send data dialog box ;
 * manageDataProc - WndProc for manage data dialog box ;
 * saveToXML() - Save the existing topology/scenarion into XML file ;
 * loadFromXML() - Load a saved XML file to recover the topology/scenario ;
 * runServer() - Open a port to listen and process the requests from client ;
 */
#ifndef CWINDOW_H
#define CWINDOW_H

#include "Winsock2.h"

#include "DlClientWinMt.h"
#include <commctrl.h>
#include "cList.h"
#include "cConsole.h"
#include "cTopology.h"
#include "Event.h"
#include "cXMLParse.h"
#include "cSaveXML.h"
#include "cLoadXML.h"
#include "loadData.h"
#include "saveData.h"
#include "cFileHub.h"

/* include "cServer.h" */

#define CONTROLS_YOFFSET		(TAB_YOFFSET + TAB_HEIGHT)
#define MAINRCLICKMENU_XOFFSET	(boFullScreen ? 2:4)
#define MAINRCLICKMENU_YOFFSET	(boFullScreen ? 2:46)
#define DEMOCYCLETIME_SPIN		(15000u)
#define DEMOCYCLETIME_SPUN		(1000u)
#define DEMOCYCLETIME_CHECKWAIT (5000u)
#define MAX_DATANUM				(50)
#define FILE_NAMELEN			(1024)

typedef enum
{
	VIEWSTATE_INVALID			= -1,
	VIEWSTATE_SELECT,
	VIEWSTATE_ROTATE,
	//VIEWSTATE_CREATE_HOSPITAL,
	//VIEWSTATE_CREATE_POLICE,
	//VIEWSTATE_CREATE_AMBULANCE,
	//VIEWSTATE_CREATE_BANK,
	//VIEWSTATE_CREATE_EVENT,
	//VIEWSTATE_CREATE_MOBILE,
	//VIEWSTATE_CREATE_RESCUE,
	//VIEWSTATE_CREATE_FIRE,
	//VIEWSTATE_CREATE_TRANSPORT,
	//VIEWSTATE_CREATE_EXTERNAL,
	VIEWSTATE_CREATELINK,
	VIEWSTATE_CREATENODE,			/* create node type deff by AA */

	VIEWSTATE_NUM
} VIEWSTATE;

typedef enum
{
	DEMOCYCLE_INVALID			= -1,
	DEMOCYCLE_START,
	DEMOCYCLE_SPIN,
	DEMOCYCLE_SPUN,
	DEMOCYCLE_CHECK,
	DEMOCYCLE_CHECKING,
	DEMOCYCLE_CHECKED,
	DEMOCYCLE_CHECKWAIT,
	DEMOCYCLE_STOPPED,
	DEMOCYCLE_NUM
} DEMOCYCLE;

class cConsole;
class cServer;
class cServerDande;
class cXMLPropertySet;

class cWindow
{
private:
	WNDCLASSEX wndClass;
	HWND hwnd;
	HDC hDC;
	HGLRC hRC;

	/*
	 * HWND vmTabList;
	 */
	HWND gwhToolbar;
	MSG message;
	RECT winRect;
	cConsole*  pConsole;
	HINSTANCE hInstance;
	bool trayActive;
	HMENU notifyIconMenu;
	HMENU hMainMenuTop;
	HMENU hMainMenuContext;
	cXMLParse*	pXMLParse;
	bool boFullScreen;
	RECT sNotFullRect;
	VIEWSTATE geViewState;
	bool gboSpin;
	DWORD guPrevTimeTheta;
	DWORD guPrevTimePhi;
	DEMOCYCLE geDemoCycle;
	DWORD guDemoCycleStart;
	double fXDragOffset;
	double fYDragOffset;
	double fZDragOffset;
	DRAGTYPE eDragType;
	unsigned int uDragStartTime;
	bool boDrag;
	float fSpinMomentum;
	float fSpinMomentumXComponent;
	float fSpinMomentumYComponent;
	float fSpinMomentumZComponent;
	float fTranslateMomentum;
	float fTranslateMomentumXComponent;
	float fTranslateMomentumYComponent;
	float fTranslateMomentumZComponent;
	unsigned int uScrollWheelTimeStart;
	unsigned int uScrollWheelTimeLast;
	bool boNextRenderType;
	bool boTopologyChanged;
	cFileHub * psFileHub;
	cSaveXML * psSaveXML;
	cXMLPropertySet * psXMLPropertySet;
	int nNodeCreateType;
	int nNodeTypes;
	int nNodeTextureStart;

	void LeftButtonDownSelect (int nXMousePos, int nYMousePos);
	void LeftButtonDownSelectRotate (int nXMousePos, int nYMousePos);
	void RightButtonDownTranslate (int nXMousePos, int nYMousePos);
	void LeftButtonDownRotate (int nXMousePos, int nYMousePos);
	void LeftButtonUp (int nXMousePos, int nYMousePos);
	void RightButtonUp (int nXMousePos, int nYMousePos);
	void MouseMove (int nXMousePos, int nYMousePos);
	void CancelDrag (void);
	void LeftButtonDownCreateLink (int nXMousePos, int nYMousePos);
	void LeftButtonDownCreateNode (int nXMousePos, int nYMousePos, int nType);
	void EnableOpenGL (HWND hWnd, HDC* hDC, HGLRC* hRC);
	void DisableOpenGL (HWND hWnd, HDC hDC, HGLRC hRC);
	void SetViewTick (void);
	void DeleteSelectedLink (void);
	void ToggleFullScreen ();
	void TogglePause ();
	void LoadNodeTextures ();
	void LoadNodeTexture (char * const szFilename);

	/*
	 * void DeleteSelecteNode (void);
	 * // added a signature of deleting selected node from the topology AA
	 */
	void ResetTime ();
	void SpinMomentum (void);
	void TranslateMomentum (void);
	void ScrollWheel (int nZDelta);
	void Disconnect (void);
	void DisconnectDande (void);
	void SetupDandelion (void);
public:
	DlClientWinMt*	psDlClient;
	cServer * psServer;
	cServerDande * psServerDande;
	int nCurID;
	int nCurDeleteID;
	char*  szNameList;
	void DeleteSelecteNode (void);	/* Added a signature of deleting selected node from the topology AA. */

	/* Moved as public by bo */
	cWindow ();
	~ cWindow ();

	bool create (HINSTANCE hInstance, LPSTR title, int iWidth = CW_USEDEFAULT, int iHeight = CW_USEDEFAULT,
				 unsigned int iStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE);
	void ShowMainWindow (int nCmdShow);
	void ResourceBeep (unsigned int nResourceID);
	void SetVolume (float fVolume);
	void TopologyChanged (void);
	int PopulateNodeList (char * szNodeList, int nLength);

	static LRESULT CALLBACK wndProc (HWND window, UINT msg, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK aboutProc (HWND window, UINT msg, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK policyProc (HWND window, UINT msg, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK optionProc (HWND window, UINT msg, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK channelsProc (HWND window, UINT msg, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK checkresultsProc (HWND window, UINT msg, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK eventlistProc (HWND window, UINT msg, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK componentProc (HWND window, UINT msg, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK assignDataProc (HWND window, UINT msg, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK sendDataProc (HWND window, UINT msg, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK manageDataProc (HWND window, UINT msg, WPARAM wparam, LPARAM lparam);
	void run ();
	bool messageHandler (UINT msg, WPARAM wparam, LPARAM lparam);
	void ListenThread ();

	/*
	 * void ShowTopologyTab (void);
	 */
	void setCurrentVM (cVM* pVM);

	cConsole*  getConsole ()	{ return pConsole; }

	cVM*  getCurrentVM ();
	cTopologyList*	getList ()	{ return pList; }

	cTopology*	pTopology;
	bool boFreezeRedraw;

	/* message handler functions */
	void resize (WPARAM wparam, LPARAM lparam);
	void newVM (bool boSetAsActive, int nNodeID);
	void closeVM ();
	void changeTab ();
	void redraw ();
	void loadStateFile ();
	void saveStateFile ();
	void cleanup ();
	void LoadAnalysisXMLFile ();
	void LoadPropertySetXMLFile (cProperties * pcDefaultNodeLink);
	void LoadDefaultPropertySetXMLFile ();
	void loadScene();
	void SetMenuTick (HWND hWnd, UINT uMenuID, bool boTick);
	void saveToXML ();
	void loadFromXML ();

	void DeleteTopology (void);
	void CreateNode (int nNodeID, float fXPos, float fYPos, float fZPos, int nType);
	void DeleteNode (int nNodeID);
	void CreateLink (int nFrom, int nTo, int nLinkID);
	bool Select (int nXMousePos, int nYMousePos);
	void DeleteLink (int nNodeFromID, int nLinkID);
	void UpdateAdHocLinks (cTopologyNode* psNode);
	void SetOverlayStatus (bool boOverlay);
	void SetSpinStatus (bool boSpin);
	void SetNetworkViewStatus (bool boNetView);

	cTopologyList*	pList;
	cTopologyNode*	psTabIterator;
	cConsole*  cCons;	/* added by AA */
	char*  nodeRecord;
	cProperties * pcDefaultProperties;
};
#endif
