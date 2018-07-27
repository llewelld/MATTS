/*$T DynamicMatts/cPropertySet.hpp GC 1.140 12/27/09 14:10:04 */

#if !defined(cPropertySet_HPP)
#define cPropertySet_HPP

#include "cControls.h"

/*
 =======================================================================================================================
 *  The control handle list containers
 =======================================================================================================================
 */
class DynamicHandles {
public:
	DynamicHandles (char const * szProperty, HWND hControl);
	~DynamicHandles ();
	DynamicHandles (DynamicHandles const & sCopyFrom);
	HWND hControl;
	char * szProperty;
private:
};

typedef std::list<DynamicHandles> ControlList;

/*
 =======================================================================================================================
 *    Class for creating the window from the PropertySet XML file
 =======================================================================================================================
 */
class cPropertySet
{
public:
	cPropertySet (cWindow * psWindow);
	~cPropertySet ();

	cControls*	LoadXML (char* szFileNam);
	HWND GenerateWindow (cControls* acControls);
	int AddPropertySetToWindow (HWND hMyDialog, int nYStart, cProperties * pcProperties);
	static BOOL WINAPI ProcessMessage (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	void AddControl (char const * szProperty, HWND hControl);
	void ClearControls ();
	HWND ControlHandleFromName (char const * szName);
	char const * ControlNameFromHandle (HWND hControl);
	ControlList const GetControlList ();
	void ChangeDefaults (cProperties * pcDefaultNodeLink);
private:
	cWindow * psWindow;
	ControlList sControls;
	cProperties * pcDefaultNodeLink;
};

#endif /* cPropertySet_HPP */
