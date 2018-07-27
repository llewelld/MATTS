/*$T MATTS/Event.h GC 1.140 07/01/09 21:12:10 */
#ifndef EVENT_H
#define EVENT_H

#include "cWindow.h"

#define EVENT_NAME_LEN	(64)

/*
 -----------------------------------------------------------------------------------------------------------------------
 *  Populate event list structure
 -----------------------------------------------------------------------------------------------------------------------
 */
typedef struct _EventListData
{
	HWND hList;
	int nItem;
} EventListData;

/*
 -----------------------------------------------------------------------------------------------------------------------
 *  Event data structures
 -----------------------------------------------------------------------------------------------------------------------
 */

class Event
{
public:
	cWindow*  psWindow;

	virtual ~Event ();
	virtual bool Callback (unsigned int uHandle, float fCompleted);
	virtual void Save (FILE * hFile, int nIndent);
	virtual void Load (char const * szProperty, char const * szType, char const * szValue);
	static Event * Kahlo () {return new Event ();}
protected:
	static void SaveElement (FILE * hFile, int nIndent, char const * szName, int nValue);
	static void SaveElement (FILE * hFile, int nIndent, char const * szName, char const * szValue);
	static void SaveElement (FILE * hFile, int nIndent, char const * szName, float fValue);
	static void SaveElement (FILE * hFile, int nIndent, char const * szName, bool boValue);
	static void SaveElement (FILE * hFile, int nIndent, char const * szName, char const * szType, char const * szValue);
};

class EventNewNode : public Event
{
public:
	int nNodeID;
	float fXPos;
	float fYPos;
	float fZPos;
	int nType;

	bool Callback (unsigned int uHandle, float fCompleted);
	void Save (FILE * hFile, int nIndent);
	void Load (char const * szProperty, char const * szType, char const * szValue);
	static Event * Kahlo () {return new EventNewNode ();}
};

class EventNewLink : public Event
{
public:
	int nFrom;
	int nTo;
	int nLinkID;

	bool Callback (unsigned int uHandle, float fCompleted);
	void Save (FILE * hFile, int nIndent);
	void Load (char const * szProperty, char const * szType, char const * szValue);
	static Event * Kahlo () {return new EventNewLink ();}
};

class EventMoveNode : public Event
{
public:
	int nNodeID;
	float fXPosTo;
	float fYPosTo;
	float fZPosTo;

	bool Callback (unsigned int uHandle, float fCompleted);
	void Save (FILE * hFile, int nIndent);
	void Load (char const * szProperty, char const * szType, char const * szValue);
	static Event * Kahlo () {return new EventMoveNode ();}
};

class EventDeleteNode : public Event
{
public:
	int nNodeID;

	bool Callback (unsigned int uHandle, float fCompleted);
	void Save (FILE * hFile, int nIndent);
	void Load (char const * szProperty, char const * szType, char const * szValue);
	static Event * Kahlo () {return new EventDeleteNode ();}
};

class EventDeleteLink : public Event
{
public:
	int nNodeFromID;
	int nLinkID;

	bool Callback (unsigned int uHandle, float fCompleted);
	void Save (FILE * hFile, int nIndent);
	void Load (char const * szProperty, char const * szType, char const * szValue);
	static Event * Kahlo () {return new EventDeleteLink ();}
};

class EventPropertiesNode : public Event
{
public:
	int nNodeID;
	char*  szCertProps;
	int nSecurityMax;
	int nSecurityMin;
	char*  szName;
	int nSensitivityLevel;
	char*  szEncryptAlgorithm;
	char*  szStaffSkills;
	bool boFirewall;
	bool boIDS;
	bool boExternal;
	bool boAdHoc;
	float fRangeTx;
	float fRangeRx;

	EventPropertiesNode ();
	~EventPropertiesNode ();
	bool Callback (unsigned int uHandle, float fCompleted);
	void Save (FILE * hFile, int nIndent);
	void Load (char const * szProperty, char const * szType, char const * szValue);
	static Event * Kahlo () {return new EventPropertiesNode ();}
};

class EventPropertiesLink : public Event
{
public:
	int nNodeFromID;
	int nLinkID;
	int nChannelOut;
	int nChannelIn;
	char*  szLinkOutput;
	char*  szLinkInput;
	int nSecurity;
	int nSensitivityLevel;

	EventPropertiesLink ();
	~EventPropertiesLink ();
	bool Callback (unsigned int uHandle, float fCompleted);
	void Save (FILE * hFile, int nIndent);
	void Load (char const * szProperty, char const * szType, char const * szValue);
	static Event * Kahlo () {return new EventPropertiesLink ();}
};

class EventPropertySetNode : public Event, public cProperties
{
public:
	int nNodeID;

	EventPropertySetNode ();
	~EventPropertySetNode ();
	bool Callback (unsigned int uHandle, float fCompleted);
	void Save (FILE * hFile, int nIndent);
	void Load (char const * szProperty, char const * szType, char const * szValue);
	static Event * Kahlo () {return new EventPropertySetNode ();}
};

class EventPropertySetClearNode : public Event
{
public:
	int nNodeID;

	EventPropertySetClearNode ();
	~EventPropertySetClearNode ();
	bool Callback (unsigned int uHandle, float fCompleted);
	void Save (FILE * hFile, int nIndent);
	void Load (char const * szProperty, char const * szType, char const * szValue);
	static Event * Kahlo () {return new EventPropertySetClearNode ();}
};

class EventPropertySetLink : public Event, public cProperties
{
public:
	int nLinkID;

	EventPropertySetLink ();
	~EventPropertySetLink ();
	bool Callback (unsigned int uHandle, float fCompleted);
	void Save (FILE * hFile, int nIndent);
	void Load (char const * szProperty, char const * szType, char const * szValue);
	static Event * Kahlo () {return new EventPropertySetLink ();}
};

class EventPropertySetClearLink : public Event
{
public:
	int nLinkID;

	EventPropertySetClearLink ();
	~EventPropertySetClearLink ();
	bool Callback (unsigned int uHandle, float fCompleted);
	void Save (FILE * hFile, int nIndent);
	void Load (char const * szProperty, char const * szType, char const * szValue);
	static Event * Kahlo () {return new EventPropertySetClearLink ();}
};

class EventAssignData : public Event
{
public:
	int nNodeID;
	char*  szDataName;

	EventAssignData ();
	~EventAssignData ();
	bool Callback (unsigned int uHandle, float fCompleted);
	void Save (FILE * hFile, int nIndent);
	void Load (char const * szProperty, char const * szType, char const * szValue);
	static Event * Kahlo () {return new EventAssignData ();}
};

class EventRemoveData : public Event
{
public:
	int nNodeID;
	char*  szDataName;

	EventRemoveData ();
	~EventRemoveData ();
	bool Callback (unsigned int uHandle, float fCompleted);
	void Save (FILE * hFile, int nIndent);
	void Load (char const * szProperty, char const * szType, char const * szValue);
	static Event * Kahlo () {return new EventRemoveData ();}
};

class EventSendData : public Event
{
public:
	int nFrom;
	int nTo;
	char*  szDataName;

	EventSendData ();
	~EventSendData ();
	bool Callback (unsigned int uHandle, float fCompleted);
	void Save (FILE * hFile, int nIndent);
	void Load (char const * szProperty, char const * szType, char const * szValue);
	static Event * Kahlo () {return new EventSendData ();}
};

class EventChangeView : public Event
{
public:
	float fTheta;
	float fPhi;
	float fPsi;
	float fRadius;

	bool Callback (unsigned int uHandle, float fCompleted);
	void Save (FILE * hFile, int nIndent);
	void Load (char const * szProperty, char const * szType, char const * szValue);
	static Event * Kahlo () {return new EventChangeView ();}
};

class EventTranslateView : public Event
{
public:
	float fX;
	float fY;
	float fZ;

	bool Callback (unsigned int uHandle, float fCompleted);
	void Save (FILE * hFile, int nIndent);
	void Load (char const * szProperty, char const * szType, char const * szValue);
	static Event * Kahlo () {return new EventTranslateView ();}
};

class EventViewProperties : public Event
{
public:
	bool boSpin;
	bool boNetworkView;
	bool boOverlay;

	bool Callback (unsigned int uHandle, float fCompleted);
	void Save (FILE * hFile, int nIndent);
	void Load (char const * szProperty, char const * szType, char const * szValue);
	static Event * Kahlo () {return new EventViewProperties ();}
};

#endif
