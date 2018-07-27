/*$T MATTS/cOptions.h GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cOptions.h ;
 * Last Modified: 10/11/04 ;
 * ;
 * Purpose: Clas to hold user option choices ;
 * NOTE: Singleton Class ;
 * Members ;
 * listenHandle - Handle to the agent listening thread ;
 * instance - Pointer to the single instance of the options class ;
 * flag - True if options class has been created, false otherwise ;
 * stack - Size of each (new) virtual machine's program stack ;
 * machineLimit - Maximum number of machines allowed ;
 * allowMinimise - True if program can be minimised to sys tray ;
 * allowAgents - True if allowing agents off other computers ;
 * autoCreate - True if allowing auto creation of VM's for new recieved agents ;
 * Methods ;
 * readOptions() - Reads options off disk (or sets defaults if it doesn't exist) ;
 * writeOptions() - Writes the options to disk ;
 * create() - Creates the single instance of the options class ;
 * getStackLimit() - Returns the stack limit ;
 * getMachineLimit() - Returns the machine limit ;
 * getAllowMinimise() - Returns allow minimise ;
 * getAllowAgents()- Returns allowing agents ;
 * getAutoCreate() - Returns auto create ;
 * setMachineLimit() - Sets machine limit ;
 * setAllowAgents()- Sets allow agents ;
 * setAutoCreate() - sets auto create ;
 * setStackLimit() - sets the stack limit ;
 * setAllowMinimise() - sets allow minimise ;
 */
#ifndef COPTIONS_H
#define COPTIONS_H

#define WIN32_LEAN_AND_MEAN 1

#include <windows.h>
#include <string>
#include "cFileHub.h"

using namespace std;

#define OPTIONS_FILENAME "Resources\\State.xml"
#define OPTIONS_STRING_MAX	(512)

/*
 * The current tag used for loading the XML options
 */
typedef enum {
  OPTIONSTAG_INVALID = -1,

	OPTIONSTAG_OPTIONS,
	OPTIONSTAG_OPTION,

  OPTIONSTAG_NUM
} OPTIONSTAG;

class cOptions : public cLoadSave
{
public:
	HANDLE listenHandle;
	HANDLE listenControlHandle;
	HANDLE listenPDAServerHandle;

	cOptions ();
	virtual ~cOptions ();
	static cOptions*  create ();
	long int getStackLimit ()			{ return stack; }
	int getMachineLimit ()				{ return machineLimit; }
	bool getAllowMinimise ()			{ return allowMinimise; }
	bool getAllowAgents ()				{ return allowAgents; }
	bool getAutoCreate ()				{ return autoCreate; }
	bool getAllowControl ()				{ return allowControl; }
	void setMachineLimit (int limit)	{ machineLimit = limit; }
	void setAllowAgents (bool allow)	{ allowAgents = allow; }
	void setAutoCreate (bool bauto)		{ autoCreate = bauto; }
	void setStackLimit (long int limit) { stack = limit; }
	void setAllowMinimise (bool min)	{ allowMinimise = min; }
	void setAllowControl (bool allow)				{ allowControl = allow; }
	float getSoundVolume ()							{ return fSoundVolume; }
	void setSoundVolume (float fVolume)				{ fSoundVolume = fVolume; }
	int getSelectSpeed ()							{ return selectSpeed; }
	int getColourSpeed ()							{ return colourSpeed; }
	float getRotateHalfLife ()						{ return rotHalfLife; }
	bool getRotateSelected ()						{ return rotSelected; }
	bool getDemoCycle ()							{ return demoCycle; }
	bool getScreenShotMode (void)					{ return screenShotMode; }
	bool getUseDandelion (void)						{ return dandelion; }
	char const*	 getDandeServer (void)				{ return dandeServer.c_str (); }
	bool getRecordViewEvents (void)					{ return viewEvents; }
	void setSelectSpeed (int speed)					{ selectSpeed = speed; }
	void setColourSpeed (int speed)					{ colourSpeed = speed; }
	void setRotateHalfLife (float halfLife)			{ rotHalfLife = halfLife; }
	void setRotateSelected (bool rotate)			{ rotSelected = rotate; }
	void setDemoCycle (bool demo)					{ demoCycle = demo; }
	void toggleDemoCycle (void)						{ demoCycle = !demoCycle; }
	void setScreenShotMode (bool screenShot)		{ screenShotMode = screenShot; }
	void setUseDandelion (bool useDandelion)		{ dandelion = useDandelion; }
	void setDandeServer (char const* dandeAddress)	{ dandeServer.assign (dandeAddress); }
	void setRecordViewEvents (bool recordViewEvents){ viewEvents = recordViewEvents; }

	void Save (FILE * hFile, int nIndent);
	void LoadStart (char const * szChunkName);
	void LoadEnd (char const * szChunkName);
	void startElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname, const Attributes&  attrs);
	void characters (const XMLCh *const chars, const unsigned int length);
	void endElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname);

private:
	OPTIONSTAG eTag;
	string sOption;
	string sLoadText;
	static cOptions*  instance;
	static bool flag;
	long int stack;
	int machineLimit;
	bool allowMinimise;
	bool allowAgents;
	bool autoCreate;
	bool allowControl;
	int selectSpeed;
	int colourSpeed;
	float rotHalfLife;
	bool rotSelected;
	bool demoCycle;
	float fSoundVolume;
	bool screenShotMode;
	bool dandelion;
	string dandeServer;
	bool viewEvents;
};
#endif
