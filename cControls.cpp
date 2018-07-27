#include "cControls.h"

/*
 =======================================================================================================================
 *    Constructor
 =======================================================================================================================
 */
cControls::cControls () {
	pcNext = NULL;
}

/*
 =======================================================================================================================
 *    Constructor, including adding it to the end of an existing list
 =======================================================================================================================
 */
cControls::cControls (cControls * pcCurrent) {
	pcNext = NULL;
	if (pcCurrent) {
		pcCurrent->AddControl (this);
	}
}

/*
 =======================================================================================================================
 *    Destructor
 =======================================================================================================================
 */
cControls::~cControls () {
	if (pcNext) {
		delete pcNext;
		pcNext = NULL;
	}
}

/*
 =======================================================================================================================
 *    Add the control to an existing list
 =======================================================================================================================
 */
void cControls::AddControl (cControls * pcControl) {
	cControls * pcCurrent;

	pcCurrent = this;
	while (pcCurrent->pcNext) {
		pcCurrent = pcCurrent->pcNext;
	}
	pcCurrent->pcNext = pcControl;
}

/*
 =======================================================================================================================
 *    Get the next item in the list
 =======================================================================================================================
 */
cControls * cControls::GetNext () {
	return pcNext;
}