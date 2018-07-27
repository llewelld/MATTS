/*$T MATTS/cTimerEvent.cpp GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cTimerEvent.cpp ;
 * Last Modified: 17/06/08 ;
 * ;
 * Purpose: The event class used by cTimer ;
 * This class is opaque for use by the cTimer class;
 * it should not be accessed directly. ;
 */
#include "cTimerEvent.h"
#include "Event.h"

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */

cTimerEvent::cTimerEvent (cTimerEvent **	 ppsEventList, char const*	szName, unsigned long uStart, unsigned long uEnd, Event*  psEvent) {
	cTimerEvent*  psSearchList;
	cTimerEvent*  psPrevious;

	psPrevious = NULL;
	this->uStart = uStart;
	this->uEnd = uEnd;

	/* Add it into the temporally ordered list */
	psSearchList = *ppsEventList;
	while ((psSearchList) && (psSearchList->uStart <= uStart)) {
		psPrevious = psSearchList;
		psSearchList = psSearchList->psNextTemporal;
	}

	if (psSearchList) {

		/* Add this into the list just after the item we found */
		psNextTemporal = psSearchList->psNextTemporal;
		psPreviousTemporal = psSearchList;

		psPreviousTemporal->psNextTemporal = this;
		if (psNextTemporal) {
			psNextTemporal->psPreviousTemporal = this;
		}
	}
	else {
		if (psPrevious) {

			/* We reached the end of the list */
			psNextTemporal = NULL;
			psPreviousTemporal = psPrevious;

			psPreviousTemporal->psNextTemporal = this;
		} else {

			/* The list was empty anyway */
			psNextTemporal = NULL;
			psPreviousTemporal = NULL;
		}
	}

	if (psPrevious == NULL) {

		/* We added the event at the start of the list */
		*ppsEventList = this;
	}

	/* Add the event's name */
	this->szName = (char*) malloc (strlen (szName) + 1);
	strcpy (this->szName, szName);

	/* When created the event isn't yet active */
	psNextActive = NULL;
	psPreviousActive = NULL;

	/* Set up the rest of data */
	this->psEvent = psEvent;
	uStart = 0;
	uEnd = 0;
	boActive = FALSE;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cTimerEvent::~cTimerEvent () {

	/* Call the cleanup callback */
	if (psEvent) {
		delete psEvent;
		psEvent = NULL;
	}

	/* Remove from the event lists */
	if (psNextTemporal) {
		psNextTemporal->psPreviousTemporal = psPreviousTemporal;
		psNextTemporal = NULL;
	}

	if (psPreviousTemporal) {
		psPreviousTemporal->psNextTemporal = psNextTemporal;
		psPreviousTemporal = NULL;
	}

	if (psNextActive) {
		psNextActive->psPreviousActive = psPreviousActive;
		psNextActive = NULL;
	}

	if (psPreviousActive) {
		psPreviousActive->psNextActive = psNextActive;
		psPreviousActive = NULL;
	}

	/* Free up the event name */
	free ((void*) szName);
	szName = NULL;

	/* Reset the rest of the data just in case */
	uStart = 0;
	uEnd = 0;
	boActive = FALSE;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cTimerEvent* cTimerEvent::Activate (cTimerEvent* psActiveEventQueue) {
	cTimerEvent*  psNewActiveEventQueue;

	psNewActiveEventQueue = psActiveEventQueue;

	if (!boActive) {

		/* Add the event to the start of the active event queue */
		this->psNextActive = psActiveEventQueue;
		this->psPreviousActive = NULL;
		if (psActiveEventQueue) {
			psActiveEventQueue->psPreviousActive = this;
		}

		psNewActiveEventQueue = this;

		/* Set as active */
		boActive = TRUE;

		/* Call the inital callback */
		if (psEvent) {
			psEvent->Callback (uEventHandle, 0.0f);

			/* TODO: Deactiveate event if boContinue returns false */
		}
	}

	return psNewActiveEventQueue;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cTimerEvent* cTimerEvent::Deactivate (cTimerEvent* psActiveEventQueue) {
	cTimerEvent*  psNewActiveEventQueue;

	psNewActiveEventQueue = psActiveEventQueue;

	if (boActive) {

		/* Remove the event from the event queue */
		if (psNextActive) {
			psNextActive->psPreviousActive = psPreviousActive;
		}

		if (psPreviousActive) {
			psPreviousActive->psNextActive = psNextActive;
		}

		if (psActiveEventQueue == this) {
			psNewActiveEventQueue = psNextActive;
		}

		psNextActive = NULL;
		psPreviousActive = NULL;

		/* Call the final callback */
		if (psEvent) {
			psEvent->Callback (uEventHandle, 1.0f);
		}

		/* Set as inactive */
		boActive = FALSE;
	}

	return psNewActiveEventQueue;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTimerEvent::FireCallback (unsigned long uTime) {

	/* Call the callback */
	float fCompleted;
	bool boContinue;

	fCompleted = 0.0f;

	if (uEnd != uStart) {
		fCompleted = (float) ((double) (uTime - uStart) / (double) (uEnd - uStart));
	}

	if (psEvent && (fCompleted > 0.0f) && (fCompleted < 1.0f)) {
		boContinue = psEvent->Callback (uEventHandle, fCompleted);

		/* TODO: Deactiveate event if boContinue returns false */
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
char const* cTimerEvent::GetEventName () {
	return szName;
}
