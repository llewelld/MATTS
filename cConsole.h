/*$T MATTS/cConsole.h GC 1.140 07/01/09 21:12:08 */
/*
 * ;
 * Name: cConsole.h ;
 * Last Modified: 10/11/04 ;
 * ;
 * Purpose: Header file for console ;
 * NOTE: Singleton Class ;
 * ;
 * Members ;
 * instance - Pointer to single instance of the console ;
 * flag - Flag for if console has been created ;
 * hWindow;
 * - Handle to main window ;
 * pVM - Pointer to active VM ;
 * console[32][81] - Character array containing the console output text ;
 * currentRow - Current row the console is on ;
 * scrXPos - X position of the console within the array ;
 * string[256] - For inputing text ;
 * bInput - True if console is inputing text, false otherwise ;
 * cInput - Current character being input ;
 * Methods ;
 * create() - Creates the instance of the console ;
 * ConsoleInitialise() - Initialises console ;
 * setVM() - Sets pVM to a new machine ;
 * clear() - Clears the console ;
 * ConsolePrint() - Prints text to the console ;
 * ConsoleInputString() - Inputs a string to the console ;
 * ConsoleInputNumber() - Inputs a number to the console ;
 * plotMain() - Draws the console text and input ;
 * isInput() - Returns true if in input mode ;
 * setChar() - Sets input character ;
 * scroll() - Scrolls console 1 line down ;
 */
#ifndef CCONSOLE_H
#define CCONSOLE_H

#include "cVM.h"

#define TAB_YOFFSET (0)
#define TAB_HEIGHT	(30)

class cWindow;

class cConsole
{
private:
	static cConsole*  instance;
	static bool flag;
	HWND hWindow;
	cVM*  pVM;
	char console[32][81];
	int currentRow;
	int scrXPos;
	char string[256];
	bool bInput;
	char cInput;
	int nPos;
	WNDCLASSEX wndClass;
	HINSTANCE hInstance;
	RECT winRect;
	HWND vmTabList;
	cWindow*  psWindow;

	bool createWindow (HINSTANCE hinstance, LPSTR title, int iWidth, int iHeight, unsigned int iStyle);
	static LRESULT CALLBACK wndProc (HWND window, UINT msg, WPARAM wparam, LPARAM lparam);
	void cleanup ();
	void resize (WPARAM wparam, LPARAM lparam);
	bool messageHandler (UINT msg, WPARAM wparam, LPARAM lparam);
	void redraw ();
	void changeTab ();
	void WindowRedraw ();
public:
	cConsole ();
	~ cConsole ();
	static cConsole*  create ();
	void ConsoleInitialise (HINSTANCE hinstance, HWND hwnd, cWindow* psWin);
	void setVM (cVM* vm);
	void clear ();
	void ConsolePrint (char* szString);
	char*  ConsoleInputString (void);
	int ConsoleInputNumber (void);
	void plotMain (PAINTSTRUCT* psPaint, HDC sHDC);
	bool isInput ()			{ return bInput; }

	void setChar (char ch)	{ cInput = ch; }

	void scroll (void);
	void ConsoleRedraw (void);
	void OpenConsole ();
	bool NewVMConsole (bool boSetAsActive);
	void SetActiveTab (int nTabID);
	void DeleteVMConsole ();
	void SetTabName (char* szTitle);

	cVM*  pCurrentVM;
	int activeID;
};
#endif
