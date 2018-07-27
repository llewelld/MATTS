/*$T DynamicMatts/cControls.hpp GC 1.140 12/27/09 14:10:03 */

#if !defined(CCONTROLS_HPP)
#define CCONTROLS_HPP

#include <string>
#include <list>
#include "cWindow.h"

using namespace std;

class cControls;

/*
 =======================================================================================================================
 *    Class for specifying a control
 =======================================================================================================================
 */
class cControls
{
public:
	string szCaption;
	string szLabel;
	list<string> cValue;
	UINT64 nControlID;
	int nMinValue;
	int nMaxValue;

	void AddControl (cControls * pcControl);
	cControls * GetNext ();
	cControls ();
	cControls (cControls * pcCurrent);
	~cControls();

private:
	cControls * pcNext;
};

#endif /* CCONTROLS_HPP */
