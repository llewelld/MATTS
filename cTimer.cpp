/*$T MATTS/cTimer.cpp GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cTimer.cpp ;
 * Last Modified: 17/06/08 ;
 * ;
 * Purpose: Allow events to happen over time ;
 */
#include "cTimer.h"
#include "Event.h"
#include <string>
#include "cFileHub.h"

using namespace std;

#define BLOCKQUANTUM	(8)

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */

cTimer::cTimer (cWindow * psWindow) {
	this->psWindow = psWindow;
	uTime = TIMER_TIME_MIN;
	uNextEventHandle = 1;
	psEventList = NULL;
	psActiveEventQueue = NULL;
	psNextInactive = psEventList;
	pfPopulateListCallback = NULL;
	pPopulateListData = NULL;
	apfCreateObject = NULL;
	aszIdentifier = NULL;
	nCreateRoutines = 0;
	nCreateRoutineAlloc = 0;
	boFrozen = false;
	hTimerMutex = CreateMutex (NULL, false, NULL);
	boPaused = false;

	uStartTime = timeGetTime () - TIMER_TIME_MIN;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cTimer::~cTimer () {
	cTimerEvent*  psEventCycle;
	cTimerEvent*  psEventNext;
	int nIdentifier;

	// Free up the file routine pointers (but not the routines!)
	if (apfCreateObject) {
		free (apfCreateObject);
		apfCreateObject = NULL;
	}

	// Free up the identifiers
	if (aszIdentifier) {
		for (nIdentifier = 0; nIdentifier < nCreateRoutines; nIdentifier++) {
			if (aszIdentifier[nIdentifier]) {
				free (aszIdentifier[nIdentifier]);
				aszIdentifier[nIdentifier] = NULL;
			}
		}
		free (aszIdentifier);
		aszIdentifier = NULL;
	}

	/* Delete the events in the event list. */
	psEventCycle = psEventList;
	while (psEventList) {
		psEventNext = psEventList->psNextTemporal;
		delete psEventList;
		psEventList = psEventNext;
	}

	CloseHandle (hTimerMutex);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
unsigned long cTimer::GetTime () {
	return uTime;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTimer::ResetTime () {
	WaitForSingleObject (hTimerMutex, INFINITE);

	uStartTime = timeGetTime () - TIMER_TIME_MIN;
	uTime = TIMER_TIME_MIN;
	psNextInactive = psEventList;

	ReleaseMutex (hTimerMutex);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTimer::SetTime (unsigned long uTime) {
	WaitForSingleObject (hTimerMutex, INFINITE);

	uStartTime = timeGetTime () - uTime;
	this->uTime = uTime;

	// Find the next inactive event
	psNextInactive = psEventList;
	while (psNextInactive && psNextInactive->uStart <= uTime) {
		psNextInactive = psNextInactive->psNextTemporal;
	}

	ReleaseMutex (hTimerMutex);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
unsigned int cTimer::AddEvent (char const*	szName, unsigned long uStart, unsigned long uLength, Event*  psEvent) {
	cTimerEvent*  psNewEvent;
	unsigned long uHandleReturn;
	unsigned long uStartSet;
	unsigned long uEnd;

	uHandleReturn = 0;
	WaitForSingleObject (hTimerMutex, INFINITE);

	switch (uStart) {
	case TIMER_TIME_NOW:
		uStartSet = uTime;
		break;
	case TIMER_TIME_THEN:
		uStartSet = uTime - 1;
		break;
	case TIMER_TIME_NEXT:
		uStartSet = uTime + 1;
		break;
	default:
		uStartSet = uStart;
		break;
	}

	uEnd = uStartSet + uLength;

	psNewEvent = new cTimerEvent (&psEventList, szName, uStartSet, uEnd, psEvent);
	if (psNewEvent) {
		uHandleReturn = uNextEventHandle;
		psNewEvent->uEventHandle = uHandleReturn;
		uNextEventHandle++;

		if (psEventList == NULL) {

			/* Set up the event list */
			psEventList = psNewEvent;
			psNextInactive = psEventList;
		}
		else {
			/* Check if the event should be active */
			if ((uTime >= psNewEvent->uStart) && (uStart != TIMER_TIME_THEN) && !boFrozen) {

				/* Activate the event */
				psActiveEventQueue = psNewEvent->Activate (psActiveEventQueue);
			}

			if ((uTime < psNewEvent->uStart)
				&& ((psNextInactive == NULL) || (psNewEvent->uStart < psNextInactive->uStart) || (uStart == TIMER_TIME_NOW))) {

				/* Set this as being the next event to activate */
				psNextInactive = psNewEvent;
			}
		}
	}

	ReleaseMutex (hTimerMutex);

	if (psNewEvent && pfPopulateListCallback) {
		pfPopulateListCallback (psNewEvent->uStart, psNewEvent->uEnd, psNewEvent->GetEventName (), pPopulateListData);
	}

	return uHandleReturn;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cTimer::RemoveEvent (unsigned int uHandle) {
	cTimerEvent*  psSearchList;
	bool boFound;

	boFound = FALSE;

	WaitForSingleObject (hTimerMutex, INFINITE);

	/* First we need to find the event */
	psSearchList = psEventList;
	while ((psSearchList) && (psSearchList->uEventHandle != uHandle)) {
		psSearchList = psSearchList->psNextTemporal;
	}

	if (psSearchList) {

		/* Is the event active? */
		if (psSearchList->boActive) {

			/* The event is active, so we need to deactivate it */
			psActiveEventQueue = psSearchList->Deactivate (psActiveEventQueue);
		}

		if (psSearchList == psEventList) {
			psEventList = psSearchList->psNextTemporal;
		}

		/* We've found it, so now we need to delete it */
		delete psSearchList;

		boFound = TRUE;
	}

	ReleaseMutex (hTimerMutex);

	return boFound;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cTimer::Update () {

	/*
	 * cTimerEvent * psSearchList;
	 */
	cTimerEvent*  psSearchActive;
	bool boUpdate;
	unsigned int uCurrentTime;

	boUpdate = TRUE;

	WaitForSingleObject (hTimerMutex, INFINITE);

	/* Update cycle */
	uCurrentTime = timeGetTime ();
	if ((uCurrentTime - uTime - uStartTime) > TIMER_TICKSSKIP_MAX) {
		uStartTime = uCurrentTime - uTime - TIMER_TICKSSKIP_MAX;
	}

	if (!boPaused) {
		uTime = uCurrentTime - uStartTime;
	}

	if (psActiveEventQueue) {
		boUpdate = TRUE;
	}

	/* Remove any old events from the active list */
	psSearchActive = psActiveEventQueue;
	while (psSearchActive) {
		if (uTime > psSearchActive->uEnd) {

			/* Deactivate the event */
			psActiveEventQueue = psSearchActive->Deactivate (psActiveEventQueue);
		}

		psSearchActive = psSearchActive->psNextActive;
	}

	/* Fire any event callbacks from the active list */
	psSearchActive = psActiveEventQueue;
	while (psSearchActive) {

		/* Fire the callback */
		psSearchActive->FireCallback (uTime);
		psSearchActive = psSearchActive->psNextActive;
	}

	/* Add any new events to the active list */
	while ((psNextInactive) && (uTime >= psNextInactive->uStart)) {

		/* Activate the event */
		psActiveEventQueue = psNextInactive->Activate (psActiveEventQueue);
		psNextInactive = psNextInactive->psNextTemporal;
		boUpdate = TRUE;
	}

	ReleaseMutex (hTimerMutex);

	return boUpdate;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTimer::TogglePause () {
	boPaused = !boPaused;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTimer::PopulateEventList (EventListCallback pfCallback, void* pData) {
	cTimerEvent*  psEvent;

	WaitForSingleObject (hTimerMutex, INFINITE);

	if (pfCallback) {
		psEvent = psEventList;
		while (psEvent) {
			pfCallback (psEvent->uStart, psEvent->uEnd, psEvent->GetEventName (), pData);
			psEvent = psEvent->psNextTemporal;
		}
	}

	pfPopulateListCallback = pfCallback;
	pPopulateListData = pData;

	ReleaseMutex (hTimerMutex);
}

/*
 =======================================================================================================================
 *  Save out the Timer data and all events to XML file
 =======================================================================================================================
 */
void cTimer::Save (FILE * hFile, int nIndent) {
	cTimerEvent*  psEvent;

	fprintf (hFile, "%*s<Timer>\n", nIndent * 2, " ");
	nIndent++;
	fprintf (hFile, "%*s<Time type=\"ulong\">%u</Time>\n", nIndent * 2, " ", uTime);

	psEvent = psEventList;
	while (psEvent) {
		fprintf (hFile, "%*s<TimerEvent>\n", nIndent * 2, " ");
		nIndent++;
		//fprintf (hFile, "%*s<type int=\"%d\"/>\n", nIndent * 2, " ", psEvent->psEvent->nEvent);
		fprintf (hFile, "%*s<start type=\"ulong\">%u</start>\n", nIndent * 2, " ", psEvent->uStart);
		fprintf (hFile, "%*s<end type=\"ulong\">%u</end>\n", nIndent * 2, " ", psEvent->uEnd);
		fprintf (hFile, "%*s<name type=\"string\">%s</name>\n", nIndent * 2, " ", psEvent->GetEventName ());
		psEvent->psEvent->Save (hFile, nIndent);
		nIndent--;
		fprintf (hFile, "%*s</TimerEvent>\n", nIndent * 2, " ");
		psEvent = psEvent->psNextTemporal;
	}
	nIndent--;
	fprintf (hFile, "%*s</Timer>\n", nIndent * 2, " ");
}

/*
 =======================================================================================================================
 *  Start the process to load in the Timer data and all events from XML file
 =======================================================================================================================
 */
void cTimer::LoadStart (char const * szChunkName) {
	sLoadText = "";
	psLoadEvent = NULL;
	sLoadName = "";
	uLoadStart = 0;
	uLoadEnd = 0;
	sLoadDataType = "";
	sLoadDataID = "";
	eTag = TIMERTAG_INVALID;
	boFrozen = true;
}

/*
 =======================================================================================================================
 *  Endt the process to load in the Timer data and all events from XML file
 =======================================================================================================================
 */
void cTimer::LoadEnd (char const * szChunkName) {
	boFrozen = false;
}

/*
 =======================================================================================================================
 *  Register a way to create an object with a given identity
 =======================================================================================================================
 */
void cTimer::RegisterLoadEvent (char const * szIdentifier, Kahlo pfCreateObject) {
	if (nCreateRoutines >= nCreateRoutineAlloc) {
		// Need some more space
		nCreateRoutineAlloc = ((int)(nCreateRoutines / BLOCKQUANTUM) + 1) * BLOCKQUANTUM;
		apfCreateObject = (Kahlo *)realloc (apfCreateObject, sizeof (Kahlo) * nCreateRoutineAlloc);
		aszIdentifier = (char * *)realloc (aszIdentifier, sizeof (char *) * nCreateRoutineAlloc);
	}

	// Make a copy of the pointer to the routine and the identifier
	apfCreateObject[nCreateRoutines] = pfCreateObject;
	aszIdentifier[nCreateRoutines] = (char *)malloc (strlen (szIdentifier) + 1);
	strcpy (aszIdentifier[nCreateRoutines], szIdentifier);

	nCreateRoutines++;
}

/*
 =======================================================================================================================
 *  Find the load event object creation routine
 =======================================================================================================================
 */
Kahlo cTimer::FindLoadEvent (char const * szIdentifier) {
	int nFileRoutine;
	Kahlo psFound;

	// Find the event load routine
	psFound = NULL;
	for (nFileRoutine = 0; (!psFound) && (nFileRoutine < nCreateRoutines); nFileRoutine++) {
		if (_stricmp (szIdentifier, aszIdentifier[nFileRoutine]) == 0) {
			// Found the routine
			psFound = apfCreateObject[nFileRoutine];
		}
	}

	return psFound;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTimer::startElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname, const Attributes&  attrs) {
	char* szElementName;
  unsigned int uAttribute;
	char * szAttributeName;
	char * szAttributeValue;
	Kahlo pfLoadCreateObject;

	szElementName = XMLString::transcode (localname);
	sLoadText = "";

	// Check the attributes
	for (uAttribute = 0; uAttribute < attrs.getLength(); uAttribute++) {
		szAttributeName = XMLString::transcode (attrs.getLocalName (uAttribute));
		szAttributeValue = XMLString::transcode (attrs.getValue (uAttribute));

		if (_stricmp (szAttributeName, "type") == 0) {
			sLoadDataType = szAttributeValue;
		}
		else if (_stricmp (szAttributeName, "id") == 0) {
			sLoadDataID = szAttributeValue;
		}

		XMLString::release (& szAttributeName);
		XMLString::release (& szAttributeValue);
	}

	if (eTag != TIMERTAG_EVENT) {
		if (eTag == TIMERTAG_TIMEREVENT) {
			// Find the common event details
			if (_stricmp (szElementName, "start") == 0) {
				eTag = TIMERTAG_START;
			}
			else if (_stricmp (szElementName, "end") == 0) {
				eTag = TIMERTAG_END;
			}
			else if (_stricmp (szElementName, "name") == 0) {
				eTag = TIMERTAG_NAME;
			}
			else if (_stricmp (szElementName, "event") == 0) {
				// Create an event object of type sLoadDataType
				pfLoadCreateObject = FindLoadEvent (sLoadDataType.c_str ());
				// Create the object
				if (pfLoadCreateObject) {
					psLoadEvent = pfLoadCreateObject ();
					psLoadEvent->psWindow = psWindow;
				}
				eTag = TIMERTAG_EVENT;
			}
		}
		else {
			if (_stricmp (szElementName, "timerevent") == 0) {
				eTag = TIMERTAG_TIMEREVENT;
			}
			else if (_stricmp (szElementName, "time") == 0) {
				eTag = TIMERTAG_TIME;
			}
		}
	}
	else {
		// Everything it to be passed on to the current event
	}

	XMLString::release (& szElementName);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTimer::characters (const XMLCh *const chars, const unsigned int length) {
	char*  szMessage;

	szMessage = XMLString::transcode (chars);
	sLoadText.append (szMessage);
	XMLString::release (& szMessage);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTimer::endElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname) {
	char * szElementName;
	unsigned long uNewTime;

	szElementName = XMLString::transcode (localname);

	switch (eTag) {
		case TIMERTAG_NAME:
			if (_stricmp (szElementName, "name") == 0) {
				sLoadName = sLoadText;
				eTag = TIMERTAG_TIMEREVENT;
			}
			break;
		case TIMERTAG_START:
			if (_stricmp (szElementName, "start") == 0) {
				uLoadStart = strtoul (sLoadText.c_str () , NULL, 10);
				eTag = TIMERTAG_TIMEREVENT;
			}
			break;
		case TIMERTAG_END:
			if (_stricmp (szElementName, "end") == 0) {
				uLoadEnd = strtoul (sLoadText.c_str () , NULL, 10);
				eTag = TIMERTAG_TIMEREVENT;
			}
			break;
		case TIMERTAG_EVENT:
			if (_stricmp (szElementName, "event") == 0) {
				eTag = TIMERTAG_TIMEREVENT;
			}
			else {
				if (psLoadEvent) {
					// Pass on the result of type sLoadDataType
					if (_stricmp (szElementName, "Property") == 0) {
						psLoadEvent->Load (sLoadDataID.c_str(), sLoadDataType.c_str (), sLoadText.c_str ());
					}
					else {
						psLoadEvent->Load (szElementName, sLoadDataType.c_str (), sLoadText.c_str ());
					}
				}
			}
			break;
		case TIMERTAG_TIMEREVENT:
			if (_stricmp (szElementName, "timerevent") == 0) {
				if (psLoadEvent) {
					// Add the event to the event list
					AddEvent (sLoadName.c_str (), uLoadStart, (uLoadEnd - uLoadStart), psLoadEvent);
					// Detach the event data
					psLoadEvent = NULL;
				}
				eTag = TIMERTAG_INVALID;
			}
			break;
		case TIMERTAG_TIME:
			if (_stricmp (szElementName, "time") == 0) {
				uNewTime = strtoul (sLoadText.c_str () , NULL, 10);
				SetTime (uNewTime);
				eTag = TIMERTAG_INVALID;
			}
			break;
	}

	XMLString::release (& szElementName);
}

