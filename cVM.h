/*$T MATTS/cVM.h GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cVM.h ;
 * Last Modified: 11/11/04 ;
 * ;
 * Purpose: Header file for cross platform MATS virtual machine ;
 * ;
 * Members ;
 * error - Pointer to an error structure for recording errors ;
 * bActive - True if the VM is the active one within the app ;
 * bLoaded - True if a program is loaded, false otherwise ;
 * bRestart - True if the machine needs to restart the program ;
 * iProgramSize - Size (in bytes) of current program ;
 * pMemory - Pointer to start location within memory ;
 * iMaxMemory - Max size of extra memory (for checking) ;
 * mainMemory - Pointer to the machine's local stack ;
 * memSize - Size of the machine's local stack ;
 * iCurrentInst - Number of the program's Current instruction ;
 * pProgram - Pointer to the program's instructions ;
 * buffer[32][81] - Machine's local buffer ;
 * inBuffer[256] - VM's local input buffer ;
 * BufferXPos - Current x Position within the local buffer ;
 * BufferRow - Row within the local buffer that the VM is on ;
 * pLibrary - Pointer to VM's library ;
 * threadHandle - Handle to VM's execution thread ;
 * threadQuit - Bool value to tell a VM to quit ;
 * Methods ;
 * execute() - Executes the machines current program ;
 * getProgram() - Returns a pointer to the machines instructions ;
 * getMemory() - Returns a pointer to machines numeric memory ;
 * getTotalSize() - Returns total size of the machine ;
 * getProgramSize()- Returns Program Size ;
 * getMaxMemory() - Returns Max Memory ;
 * getCurrentInst()- Returns current instruction number ;
 * getMainMemory() - Returns pointer to main memory ;
 * loadExecute () - Loads and runs a program ;
 * saveExecutingProgram () - Saves program (calls local imp) ;
 * setUpExecution () - Sets up recieved agent ready for execution ;
 * setIsListening()- Sets listening ;
 * setLoaded() - Sets loaded ;
 * setRestart() - Sets Restart ;
 * setActive() - Sets active ;
 * setInput() - Sets input ;
 * getActive() - Returns active ;
 * getLoaded() - Returns loaded ;
 * getRestart() - Returns restart ;
 * getBuffer() - Returns local buffer and current x and row positions ;
 * inputChar() - Inputs a single char into the VM's input buffer ;
 * clearInput() - Clear's the input buffer ;
 * packageExecutingProgram () - Packages program for network transfer ;
 * isListening() - Returns if machine listening ;
 * isInput() - Returns if machine in input ;
 * setThreadHandle() - Sets machines thread handle ;
 * getHandle() - Returns handle to execution thread ;
 * clearBuffer() - Clears local buffer ;
 * ConsolePrint () - Prints to buffer or console (if active) ;
 * getMemSize() - Returns stack size ;
 * checkMaxMemory () - Checks current memory ;
 * storeString () - Stores a string in local stack ;
 * dereference () - De-references the passed memory location ;
 * loadFileAt () - Loads a program into memory (doesn't run it) ;
 * bufferOutput() - Writes the output to the local buffer ;
 * void executeMOV () - Executes a MOV (Move / Copy) instruction ;
 * void executeADD () - Executes a ADD (Addition) instuction ;
 * void executeSUB () - Executes a SUB (Subtraction) instruction ;
 * void executeMUL () - Executes a MUL (Multiplication) instruction ;
 * void executeDIV () - Executes a DIV (Division) instruction ;
 * void executeMOD () - Executes a MOD (Modulas) instruction ;
 * void executeORR () - Executes a ORR (Or) instruction ;
 * void executeAND () - Executes a AND (And) instruction ;
 * void executeBIC () - Executes a BIC (BIC) instruction ;
 * void executeEOR () - Executes a EOR (Exclusive Or) instruction ;
 * void executeROR () - Executes a ROR (Bit Shift Right) instruction ;
 * void executeROL () - Executes a ROL (Bit Shift Left) instruction ;
 * void executeBEQ () - Executes a BEQ (Break Equal) instruction ;
 * void executeBNE () - Executes a BNE (Break Not Equal) instruction ;
 * void executeBLT () - Executes a BLT (Break Less Than) instruction ;
 * void executeBLE () - Executes a BLE (Break Less or Equal) instruction ;
 * void executeBGT () - Executes a BGT (Break Greater Than) instruction ;
 * void executeBGE () - Executes a BGE (Break Greater or Equal) instruction ;
 * void executeLIB () - Executes a LIB (Library) instruction ;
 */
#ifndef CVM_H
#define CVM_H

#include "cVMTypes.h"
#include "cLocalLibrary.h"

class cTopologyNode;

class cVM
{
	friend class cLibrary;
	friend class cLocalLibrary;
public:
	cVM (long int size, cTopologyNode* psSetTopolgyNode);
	~ cVM ();
	void execute ();
	instruction*  getProgram ()		{ return pProgram; }

	long int*  getMemory ()			{ return pMemory; }

	long int getTotalSize () {
		return sizeof (outputHeader) + ((iProgramSize + 1) * sizeof (instruction)) + (iMaxMemory * sizeof (long int));
	}

	long int getProgramSize ()	{ return iProgramSize; }

	long int getMaxMemory ()	{ return iMaxMemory; }

	long int getCurrentInst ()	{ return iCurrentInst; }

	char*  getMainMemory ()		{ return mainMemory; }

	int loadExecute (char* szExecuteFile);
	void saveExecutingProgram (char* szOutputFile);
	void setUpExecution (void);
	void setIsListening (bool isListening)	{ bListen = isListening; }

	void setLoaded (bool load)				{ bLoaded = load; }

	void setRestart (bool restart)			{ bRestart = restart; }

	void setActive (bool active)			{ bActive = active; }

	void setInput (bool input)				{ bInput = input; }

	bool getActive ()						{ return bActive; }

	bool getLoaded ()						{ return bLoaded; }

	bool getRestart ()						{ return bRestart; }

	void getBuffer (char buf[32][81], int* xPos, int* row, char string[256], int* inputBufPos);
	char*  getInBuffer ();
	void inputChar (char c);
	void clearInput ();
	void packageExecutingProgram (char ** ppcOutputData, long int* pnOutputSize);
	bool isListening ()					{ return bListen; }

	bool isInput ()						{ return bInput; }

	void setThreadHandle (HANDLE th)	{ threadHandle = th; }

	HANDLE getHandle ()					{ return threadHandle; }

	void clearBuffer ();
	void ConsolePrint (char* szString);
	int getMemSize ()	{ return memSize; }

	void threadPause (bool boPause);
	bool cVM:: threadPaused ();

	os_error*  error;
	bool threadQuit;

	cTopologyNode*	psTopologyNode;
private:
	bool bActive;
	bool bLoaded;
	bool bRestart;
	long int iProgramSize;
	long int*  pMemory;
	long int iMaxMemory;
	char*  mainMemory;
	long int memSize;
	long int iCurrentInst;
	instruction*  pProgram;
	char buffer[32][81];
	char inBuffer[256];
	int BufferXPos;
	int BufferRow;
	bool bInput;
	bool bListen;
	int inputPos;
	cLocalLibrary*	pLibrary;
	HANDLE threadHandle;
	bool boPaused;

	void checkMaxMemory (long int* pM);
	long int storeString (char* sString, long int* pMemory, long int iLength);
	bool dereference (long int ** ppM1, long int* pM1, long int* pR1);
	long int loadFileAt (char* szFilename);
	void bufferOutput (char* szString);

	void executeMOV (long int* pnM1, long int nM3);
	void executeADD (long int* pnM1, long int nM2, long int nM3);
	void executeSUB (long int* pnM1, long int nM2, long int nM3);
	void executeMUL (long int* pnM1, long int nM2, long int nM3);
	void executeDIV (long int* pnM1, long int nM2, long int nM3);
	void executeMOD (long int* pnM1, long int nM2, long int nM3);
	void executeORR (long int* pnM1, long int nM2, long int nM3);
	void executeAND (long int* pnM1, long int nM2, long int nM3);
	void executeBIC (long int* pnM1, long int nM2, long int nM3);
	void executeEOR (long int* pnM1, long int nM2, long int nM3);
	void executeROR (long int* pnM1, long int nM2, long int nM3);
	void executeROL (long int* pnM1, long int nM2, long int nM3);
	void executeBEQ (long int nM1, long int nM2, long int nM3);
	void executeBNE (long int nM1, long int nM2, long int nM3);
	void executeBLT (long int nM1, long int nM2, long int nM3);
	void executeBLE (long int nM1, long int nM2, long int nM3);
	void executeBGT (long int nM1, long int nM2, long int nM3);
	void executeBGE (long int nM1, long int nM2, long int nM3);

	void executeLIB (long int nM1, long int* pnM2, long int nM2, long int* pnM3, long int nM3);
};
#endif
