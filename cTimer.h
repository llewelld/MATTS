/*$T MATTS/cTimer.h GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cTimer.h ;
 * Last Modified: 17/06/08 ;
 * ;
 * Purpose: Header file for cTimer ;
 */
#ifndef CTIMER_H
#define CTIMER_H

#include "Winsock2.h"
#include <windows.h>
#include <stdio.h>
#include "cTimerEvent.h"
#include "cFileHub.h"

/* This callback used to populate the event list */
typedef void (*EventListCallback) (unsigned int uStart, unsigned int uEnd, char const*szEvent, void*pData);

/* This callback used to create an object when loading events */
typedef Event * (*Kahlo) (void);

/* The number of timer ticks per second */
#define TIMER_SECOND_TICKS	(1000)

/*
 * The maximum number of ticks that can occur between updates (otherwise the clock
 * is paused)
 */
#define TIMER_TICKSSKIP_MAX (100)

/* This is when the MATTS universe was created */
#define TIMER_TIME_MIN	(3)

/* Used to start an event straight away */
#define TIMER_TIME_NOW	(1)

/* Used to add an event in the past that is not then immediately activated */
#define TIMER_TIME_THEN (0)

/*
 * Used to add an event that's not immediately active, but will be the next event
 * to be activated
 */
#define TIMER_TIME_NEXT (2)

/*
 * The current tag used for loading the XML events
 */
typedef enum {
  TIMERTAG_INVALID = -1,

	TIMERTAG_TIME,
	TIMERTAG_TIMEREVENT,
	TIMERTAG_NAME,
	TIMERTAG_START,
	TIMERTAG_END,
	TIMERTAG_EVENT,

  TIMERTAG_NUM
} TIMERTAG;

class cWindow;

/* The timer class that actually does the work */
class cTimer : public cLoadSave
{
private:
	cWindow * psWindow;
	unsigned long uTime;
	unsigned long uNextEventHandle;
	cTimerEvent*  psEventList;
	cTimerEvent*  psActiveEventQueue;
	cTimerEvent*  psNextInactive;
	DWORD uStartTime;
	EventListCallback pfPopulateListCallback;
	void * pPopulateListData;
	HANDLE hTimerMutex;
  string sLoadText;
	Event * psLoadEvent;
	string sLoadName;
	unsigned long uLoadStart;
	unsigned long uLoadEnd;
	TIMERTAG eTag;
	string sLoadDataType;
	string sLoadDataID;
	Kahlo * apfCreateObject;
	char * * aszIdentifier;
	int nCreateRoutines;
	int nCreateRoutineAlloc;
	bool boFrozen;
	bool boPaused;
public:
	cTimer (cWindow * psWindow);
	~ cTimer ();

	unsigned long GetTime ();
	void ResetTime ();
	void SetTime (unsigned long uTime);
	unsigned int AddEvent (char const*	szName, unsigned long uStart, unsigned long uLength, 
						   Event*  psEvent);
	bool RemoveEvent (unsigned int uHandle);

	bool Update ();
	void TogglePause ();
	void PopulateEventList (EventListCallback pfCallback, void * pData);
	void RegisterLoadEvent (char const * szIdentifier, Kahlo pfCreateObject);
	Kahlo FindLoadEvent (char const * szIdentifier);
	void Save (FILE * hFile, int nIndent);
	void LoadStart (char const * szChunkName);
	void LoadEnd (char const * szChunkName);
	void startElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname, const Attributes&  attrs);
	void characters (const XMLCh *const chars, const unsigned int length);
	void endElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname);
};
#endif
