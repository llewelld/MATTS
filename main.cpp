/*$T MATTS/main.cpp GC 1.140 07/01/09 21:12:10 */
/*
 * ;
 * Name: main.cpp ;
 * Last Modified: 10/11/04 ;
 * ;
 * Purpose: Winmain file (starting point for App) ;
 */
#include "cWindow.h"

/*
 =======================================================================================================================
 *  int mainWindow, subWindow1,subWindow2,subWindow3;
 =======================================================================================================================
 */
int WINAPI WinMain (HINSTANCE hinstance, HINSTANCE hprevinst, LPSTR lpcmdline, int nCmdShow) {
	cWindow*  pWin;

	pWin = new cWindow;

	pWin->create (hinstance, "Mobile Agent Topology Test System", 1024, 768, WS_OVERLAPPEDWINDOW);
	pWin->ShowMainWindow (nCmdShow);
	pWin->run ();

	/*
	 * mainWindow = glutCreateWindow("SnowMen from 3D-Tech");
	 * subWindow1 = glutCreateSubWindow(mainWindow, 10,10,100,100);
	 */
	return 0;
}
