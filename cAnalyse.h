/*$T MATTS/cAnalyse.h GC 1.140 07/01/09 21:12:08 */
/*
 * ;
 * Name: cAnalyse.h ;
 * Last Modified: 20/01/05 ;
 * Purpose: Header file for cAnalyse ;
 * Members ;
 * hWindow - Handle to main window ;
 * pList - Pointer to the list of Virtual Machines ;
 * Methods ;
 * plotMain() - Draws the console text and input ;
 * setVMList() - Sets up a list of Virtual Machines to deal with ;
 * TopologyInitialise() - Initialise ;
 */
#ifndef CANALYSE_H
#define CANALYSE_H

#include "cVM.h"
#include "Prop.h"

#define TIME_ANALYSE	(0)
#define UNRAVELMAX		(5)
#define PRINT_ALL		(FALSE)
#define COPY			CopyRecursive

/*
 * Buffer test defines ;
 * #define BUFFERRECEIVELEN (gnMaxReceive)
 */
#define BUFFERRECEIVENUM	(64)
#define BUFFERRECEIVEEND	(0)
#define REF_INVALID			(-1)
#define REF_VALID(r, m)		(((r) > -1) && (((r) < 1) || ((m >= 0) && (m < MEMORYSIZE))))
#define REF_MEMORY(r, m)	(((r) > 0) && ((m >= 0) && (m < MEMORYSIZE)))
#define REF_CONSTANT(r)		((r) == 0)
#define REF_BRANCH(r, m)	((m >= 0) && ((((r) == 0) && (m <= gnProgramSize)) || (((r) > 0) && (m < MEMORYSIZE))))
#define REF_OPTION(r, m)	(((r) < 0) || ((m >= 0) && (m < MEMORYSIZE)))

class cAnalyse
{
private:
	bool gboPrintAll;
	int gnTimeAnalyse;
	Operation*	ConvertToLogic (instruction* psProgram, int nStart, int nEnd, int* pnMemory, Operation* psPost);
	HANDLE ghThreadHandle;
	char gszOutput[1024 * 32];
	int gnLink;
	int gnMaxReceive;
	int gnHeapAllocs;

	void (*gpApplyMidCondition_LibSendInteger)
		(
			Operation **apsOp,
			int nPC,
			Operation * psM1,
			Operation * psM2,
			Operation * psM3,
			Operation * psMem,
			Operation * psM1set,
			Operation * psM2set
		);
	void (*gpApplyMidCondition_LibReceiveInteger)
		(
			Operation **apsOp,
			int nPC,
			Operation * psM1,
			Operation * psM2,
			Operation * psM3,
			Operation * psMem,
			Operation * psM1set,
			Operation * psM2set
		);
	void (*gpApplyMidCondition_LibHeapAlloc)
		(
			Operation **apsOp,
			int nPC,
			Operation * psM1,
			Operation * psM2,
			Operation * psM3,
			Operation * psMem,
			Operation * psM1set,
			Operation * psM2set,
			int *pnHeapAllocs
		);
public:
	cAnalyse (cVM* psAgent);
	~ cAnalyse ();

	/*
	 * void AnalyseCode (Instruction *psProgram, int nProgramSize, int *pnMemory);
	 */
	void AnalyseCode_id1 (cVM* psAgent, int nLink, int nMaxReceive);	/* Receiving */
	void AnalyseCode_id2 (cVM* psAgent, int nLink); /* Sending */
	void AnalyseCode_id3 (cVM* psAgent, int nLink); /* Access 01 */
	void AnalyseCode_id4 (cVM* psAgent, int nLink, int nInput); /* Access 02 */

	cVM*  gpsAgent;
	int gnResult;
	bool gboCompleted;
};
#endif
