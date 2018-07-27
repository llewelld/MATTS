/*$T MATTS/cTimerEvent.h GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cTimerEvent.h ;
 * Last Modified: 17/06/08 ;
 * ;
 * Purpose: Header file for cTimerEvent ;
 * This class is opaque for use by the cTimer class;
 * it should not be accessed directly. ;
 */
#ifndef CTIMEREVENT_H
#define CTIMEREVENT_H

class Event;

/*
 * The event callback function is called every frame when the event is active ;
 * This is also used for the cleanup function, called when the event is deleted to
 * allow pData to be freed ;
 * uHandle - the event handle ;
 * fCompleted - how far through the event things are (from 0 to 1) ;
 * pData - the date blob passed when the event was created ;
 * Return - true if everything is okay, false to terminate the event prematurely
 */
typedef bool (*EventCallback) (unsigned int uHandle, float fCompleted, void*pData);

/*
 * The timer event class that actually does the work ;
 * This class is opaque for use by the cTimer class;
 * it should not be accessed directly.
 */
class cTimerEvent
{
private:
	char*  szName;
public:
	cTimerEvent*  psNextTemporal;
	cTimerEvent*  psPreviousTemporal;
	cTimerEvent*  psNextActive;
	cTimerEvent*  psPreviousActive;

	unsigned long uStart;
	unsigned long uEnd;
	unsigned long uEventHandle;
	Event* psEvent;
	bool boActive;

	cTimerEvent (cTimerEvent **	 ppsEventList, char const*	szName, unsigned long uStart, unsigned long uEnd,
				 Event*  psEvent);
	~ cTimerEvent ();

	cTimerEvent*  Activate (cTimerEvent* psActiveEventQueue);
	cTimerEvent*  Deactivate (cTimerEvent* psActiveEventQueue);
	void FireCallback (unsigned long uTime);
	char const*	 GetEventName ();
};
#endif
