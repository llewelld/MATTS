/*$T MATTS/cAnalyse.cpp GC 1.140 07/01/09 21:12:08 */
/*
 * ;
 * Name: cTopology.cpp ;
 * Last Modified: 20/12/05 ;
 * Purpose: Deal with network topology between agents ;
 */
#include "cAnalyse.h"

#include "Conds-id1.h"
#include "Conds-id2.h"
#include "Conds-id3.h"
#include "Conds-id4.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

void threadProc (cAnalyse* pcAnalyse);

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */

cAnalyse::cAnalyse (cVM* psAgent) {
	gboPrintAll = PRINT_ALL;
	gnTimeAnalyse = TIME_ANALYSE;
	gpsAgent = psAgent;
	gnResult = 0;
	gboCompleted = FALSE;
	gnLink = 0;
	gnMaxReceive = 1;
	gnHeapAllocs = 0;

	gpApplyMidCondition_LibSendInteger = NULL;
	gpApplyMidCondition_LibReceiveInteger = NULL;
	gpApplyMidCondition_LibHeapAlloc = NULL;

	/*
	 * Analyse the code RIGHT NOW! ;
	 * AnalyseCode (gpsAgent);
	 * ;
	 * ghThreadHandle = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) threadProc,
	 * this, 0, NULL);
	 */
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cAnalyse::~cAnalyse () {
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void threadProc (cAnalyse* pcAnalyse) {

	/*
	 * pcAnalyse->gboCompleted = FALSE;
	 * pcAnalyse->AnalyseCode (pcAnalyse->gpsAgent);
	 */
}

/*/
 =======================================================================================================================
 *  Perform Direct Code Analysis on the code ;
 *  void cAnalyse::AnalyseCode (Instruction *psProgram, int nProgramSize, int *pnMemory) ;
 *  id1 (receiving)
 =======================================================================================================================
 */
void cAnalyse::AnalyseCode_id1 (cVM* psAgent, int nLink, int nMaxReceive) {
	Operation*	psOp;
	Operation*	psPost;

	/*
	 * char szOutput[1024*32];
	 */
	Operation*	psTest;

	instruction*  psProgram;
	int nProgramSize;
	int*  pnMemory;

	psOp = NULL;
	psPost = NULL;
	psTest = NULL;

	gpApplyMidCondition_LibSendInteger = &ApplyMidCondition_LibSendInteger_id1;
	gpApplyMidCondition_LibReceiveInteger = &ApplyMidCondition_LibReceiveInteger_id1;
	gpApplyMidCondition_LibHeapAlloc = &ApplyMidCondition_LibHeapAlloc_id1;

	gnLink = nLink;
	gnMaxReceive = nMaxReceive;

	if (gnMaxReceive == 0) {
		sprintf (gszOutput, "No input on channel %d, no checking required\n", gnLink);
		psAgent->ConsolePrint (gszOutput);
		gnMaxReceive = 1;
		gnResult = TRUE;
	} else {
		psProgram = psAgent->getProgram ();
		nProgramSize = psAgent->getProgramSize ();
		pnMemory = (int*) psAgent->getMemory ();

		/* Post condition */
		psPost = CreatePostCondition_id1 ();
		OperationString (psPost, gszOutput, sizeof (gszOutput));
		psAgent->ConsolePrint ("Postcondition is:-\n");
		psAgent->ConsolePrint (gszOutput);
		psAgent->ConsolePrint ("\n");

		psAgent->ConsolePrint ("\nAnalysing instruction... \n");
		psOp = ConvertToLogic (psProgram, 0, nProgramSize, pnMemory, psPost);
		psAgent->ConsolePrint ("  done.\n\n");
		OperationString (psOp, gszOutput, sizeof (gszOutput));
		psAgent->ConsolePrint ("Output is:-\n");
		psAgent->ConsolePrint (gszOutput);
		psAgent->ConsolePrint ("\n");

		/* Pre condition */
		psAgent->ConsolePrint ("\nApplying Precondition...\n");
		psOp = ApplyPreCondition_id1 (psOp);

		/* Display some info */
		printf ("Code allocated %d heaps\n", gnHeapAllocs);

		/* Simplify result */
		psAgent->ConsolePrint ("Simplifying result...\n");
		psOp = SimplifyOperation (psOp);
		OperationString (psOp, gszOutput, sizeof (gszOutput));
		psAgent->ConsolePrint ("\nFinal output is:-\n");
		psAgent->ConsolePrint (gszOutput);
		psAgent->ConsolePrint ("\n");

		/* Check the result */
		psTest = CreateTruthValue (TRUE);
		if (CompareOperations (psOp, psTest)) {
			gnResult = TRUE;
		}

		/* Free up the operation memory */
		FreeRecursive (psTest);
		psTest = NULL;
		FreeRecursive (psOp);
		psOp = NULL;
		FreeRecursive (psPost);
		psPost = NULL;
	}

	gboCompleted = TRUE;
}

/*/
 =======================================================================================================================
 *  Perform Direct Code Analysis on the code ;
 *  void cAnalyse::AnalyseCode (Instruction *psProgram, int nProgramSize, int *pnMemory) ;
 *  id2 (sending)
 =======================================================================================================================
 */
void cAnalyse::AnalyseCode_id2 (cVM* psAgent, int nLink) {
	Operation*	psOp;
	Operation*	psPost;

	/*
	 * char szOutput[1024*32];
	 */
	Operation*	psTest;

	instruction*  psProgram;
	int nProgramSize;
	int*  pnMemory;
	int nChanMax;

	psOp = NULL;
	psPost = NULL;
	psTest = NULL;

	gpApplyMidCondition_LibSendInteger = &ApplyMidCondition_LibSendInteger_id2;
	gpApplyMidCondition_LibReceiveInteger = &ApplyMidCondition_LibReceiveInteger_id2;
	gpApplyMidCondition_LibHeapAlloc = &ApplyMidCondition_LibHeapAlloc_id2;

	gnLink = nLink;

	psProgram = psAgent->getProgram ();
	nProgramSize = psAgent->getProgramSize ();
	pnMemory = (int*) psAgent->getMemory ();

	/* Post condition */
	psPost = CreatePostCondition_id2 ();
	OperationString (psPost, gszOutput, sizeof (gszOutput));
	psAgent->ConsolePrint ("Postcondition is:-\n");
	psAgent->ConsolePrint (gszOutput);
	psAgent->ConsolePrint ("\n");

	psAgent->ConsolePrint ("\nAnalysing instruction... \n");
	psOp = ConvertToLogic (psProgram, 0, nProgramSize, pnMemory, psPost);
	psAgent->ConsolePrint ("  done.\n\n");
	OperationString (psOp, gszOutput, sizeof (gszOutput));
	psAgent->ConsolePrint ("Output is:-\n");
	psAgent->ConsolePrint (gszOutput);
	psAgent->ConsolePrint ("\n");

	/* Pre condition */
	psAgent->ConsolePrint ("\nApplying Precondition...\n");
	psOp = ApplyPreCondition_id2 (psOp);

	/* Display some info */
	printf ("Code allocated %d heaps\n", gnHeapAllocs);

	/* Simplify result */
	psAgent->ConsolePrint ("Simplifying result...\n");
	psOp = SimplifyOperation (psOp);
	OperationString (psOp, gszOutput, sizeof (gszOutput));
	psAgent->ConsolePrint ("\nFinal output is:-\n");
	psAgent->ConsolePrint (gszOutput);
	psAgent->ConsolePrint ("\n");

	/* Check the result */
	gnResult = sscanf (gszOutput, "(%d < chantest)", &nChanMax);

	if (gnResult != EOF) {
		gnResult = nChanMax;
	} else {
		gnResult = -1;
	}

	/* Free up the operation memory */
	FreeRecursive (psTest);
	psTest = NULL;
	FreeRecursive (psOp);
	psOp = NULL;
	FreeRecursive (psPost);
	psPost = NULL;

	gboCompleted = TRUE;
}

/*/
 =======================================================================================================================
 *  Perform Direct Code Analysis on the code ;
 *  void cAnalyse::AnalyseCode (Instruction *psProgram, int nProgramSize, int *pnMemory) ;
 *  id3 (access control 01)
 =======================================================================================================================
 */
void cAnalyse::AnalyseCode_id3 (cVM* psAgent, int nLink) {
	Operation*	psOp;
	Operation*	psPost;

	/*
	 * char szOutput[1024*32];
	 */
	Operation*	psTest;

	instruction*  psProgram;
	int nProgramSize;
	int*  pnMemory;

	psOp = NULL;
	psPost = NULL;
	psTest = NULL;

	gpApplyMidCondition_LibSendInteger = &ApplyMidCondition_LibSendInteger_id3;
	gpApplyMidCondition_LibReceiveInteger = &ApplyMidCondition_LibReceiveInteger_id3;
	gpApplyMidCondition_LibHeapAlloc = &ApplyMidCondition_LibHeapAlloc_id3;

	gnLink = nLink;

	psProgram = psAgent->getProgram ();
	nProgramSize = psAgent->getProgramSize ();
	pnMemory = (int*) psAgent->getMemory ();

	/* Post condition */
	psPost = CreatePostCondition_id3 ();
	OperationString (psPost, gszOutput, sizeof (gszOutput));
	psAgent->ConsolePrint ("Postcondition is:-\n");
	psAgent->ConsolePrint (gszOutput);
	psAgent->ConsolePrint ("\n");

	psAgent->ConsolePrint ("\nAnalysing instruction... \n");
	psOp = ConvertToLogic (psProgram, 0, nProgramSize, pnMemory, psPost);
	psAgent->ConsolePrint ("  done.\n\n");
	OperationString (psOp, gszOutput, sizeof (gszOutput));
	psAgent->ConsolePrint ("Output is:-\n");
	psAgent->ConsolePrint (gszOutput);
	psAgent->ConsolePrint ("\n");

	/* Pre condition */
	psAgent->ConsolePrint ("\nApplying Precondition...\n");
	psOp = ApplyPreCondition_id3 (psOp);

	/* Display some info */
	printf ("Code allocated %d heaps\n", gnHeapAllocs);

	/* Simplify result */
	psAgent->ConsolePrint ("Simplifying result...\n");
	psOp = SimplifyOperation (psOp);
	OperationString (psOp, gszOutput, sizeof (gszOutput));
	psAgent->ConsolePrint ("\nFinal output is:-\n");
	psAgent->ConsolePrint (gszOutput);
	psAgent->ConsolePrint ("\n");

	/* Check the result */
	psTest = CreateTruthValue (TRUE);
	if (CompareOperations (psOp, psTest)) {
		gnResult = TRUE;
	}

	/* Free up the operation memory */
	FreeRecursive (psTest);
	psTest = NULL;
	FreeRecursive (psOp);
	psOp = NULL;
	FreeRecursive (psPost);
	psPost = NULL;

	gboCompleted = TRUE;
}

/*/
 =======================================================================================================================
 *  Perform Direct Code Analysis on the code ;
 *  void cAnalyse::AnalyseCode (Instruction *psProgram, int nProgramSize, int *pnMemory) ;
 *  id4 (access control 02)
 =======================================================================================================================
 */
void cAnalyse::AnalyseCode_id4 (cVM* psAgent, int nLinkIn, int nLinkOut) {
	Operation*	psOp;
	Operation*	psPost;

	/*
	 * char szOutput[1024*32];
	 */
	Operation*	psTest;

	instruction*  psProgram;
	int nProgramSize;
	int*  pnMemory;

	psOp = NULL;
	psPost = NULL;
	psTest = NULL;

	gpApplyMidCondition_LibSendInteger = &ApplyMidCondition_LibSendInteger_id4;
	gpApplyMidCondition_LibReceiveInteger = &ApplyMidCondition_LibReceiveInteger_id4;
	gpApplyMidCondition_LibHeapAlloc = &ApplyMidCondition_LibHeapAlloc_id4;

	gnLink = nLinkIn;

	psProgram = psAgent->getProgram ();
	nProgramSize = psAgent->getProgramSize ();
	pnMemory = (int*) psAgent->getMemory ();

	/* Post condition */
	psPost = CreatePostCondition_id4 (nLinkIn, nLinkOut);
	OperationString (psPost, gszOutput, sizeof (gszOutput));
	psAgent->ConsolePrint ("Postcondition is:-\n");
	psAgent->ConsolePrint (gszOutput);
	psAgent->ConsolePrint ("\n");

	psAgent->ConsolePrint ("\nAnalysing instruction... \n");
	psOp = ConvertToLogic (psProgram, 0, nProgramSize, pnMemory, psPost);
	psAgent->ConsolePrint ("  done.\n\n");
	OperationString (psOp, gszOutput, sizeof (gszOutput));
	psAgent->ConsolePrint ("Output is:-\n");
	psAgent->ConsolePrint (gszOutput);
	psAgent->ConsolePrint ("\n");

	/* Pre condition */
	psAgent->ConsolePrint ("\nApplying Precondition...\n");
	psOp = ApplyPreCondition_id4 (psOp);

	/* Display some info */
	printf ("Code allocated %d heaps\n", gnHeapAllocs);

	/* Simplify result */
	psAgent->ConsolePrint ("Simplifying result...\n");
	psOp = SimplifyOperation (psOp);
	OperationString (psOp, gszOutput, sizeof (gszOutput));
	psAgent->ConsolePrint ("\nFinal output is:-\n");
	psAgent->ConsolePrint (gszOutput);
	psAgent->ConsolePrint ("\n");

	/* Check the result */
	psTest = CreateTruthValue (TRUE);
	if (CompareOperations (psOp, psTest)) {
		gnResult = TRUE;
	}

	/* Free up the operation memory */
	FreeRecursive (psTest);
	psTest = NULL;
	FreeRecursive (psOp);
	psOp = NULL;
	FreeRecursive (psPost);
	psPost = NULL;

	gboCompleted = TRUE;
}

/*/
 =======================================================================================================================
 *  Convert program to propositional logic
 =======================================================================================================================
 */
Operation* cAnalyse::ConvertToLogic (instruction* psProgram, int nStart, int nEnd, int* pnMemory, Operation* psPost) {
	Operation **  apsOp;
	int nPC;
	int nPCBranch;
	int nRef;
	Operation*	psM1;
	Operation*	psM2;
	Operation*	psM3;
	Operation*	psTemp;
	Operation*	psFind;
	Operation*	psMem;
	Operation*	psMemSub;
	Operation*	psSub;
	Operation*	psM1set;
	Operation*	psM2set;
	int nBranchLoop;
	OPBINARY eBranch;
	OPBINARY eNoBranch;

	apsOp = NULL;
	psM1 = NULL;
	psM2 = NULL;
	psM3 = NULL;
	psTemp = NULL;
	psFind = NULL;
	psMem = NULL;
	psMemSub = NULL;
	psSub = NULL;
	psM1set = NULL;
	psM2set = NULL;
	eBranch = OPBINARY_INVALID;
	eNoBranch = OPBINARY_INVALID;

	pnMemory = pnMemory;

	/*
	 * Create space to store a record of all of the formulae at each stage ;
	 * of the process
	 */
	apsOp = (Operation**) calloc (nEnd + 1, sizeof (Operation *));

	/* Start at the end of the program and work backwards */
	nPC = nEnd; /* 1;
				 * */

	/* Postcondition is `true' if none has been specified */
	if (psPost) {

		/*
		 * We must create a copy as it will be destroyed at the end of the ;
		 * function
		 */
		apsOp[nPC] = COPY (psPost);
	} else {
		apsOp[nPC] = CreateTruthValue (TRUE);
	}

	nPC--;

	/* Reset the heap block allocation count */
	gnHeapAllocs = 0;

	/*
	 * A lot of the operations need a "memory" variable, so we create one ;
	 * ready to use
	 */
	psMem = CreateMemory ();

	while (nPC >= nStart) {

		/* Clear the current expression (not strictly necessary, but safest) */
		apsOp[nPC] = NULL;

		/* Create the expression formulae */
		if (psProgram[nPC].nR1 > REF_INVALID) {
			nRef = psProgram[nPC].nR1;
			psM1 = CreateInteger (psProgram[nPC].nM1);
			while (nRef > 0) {
				psM1 = CreateBinary (OPBINARY_IND, CreateMemory (), psM1);
				nRef--;
			}
		}

		if (psProgram[nPC].nR1 > 0) {
			nRef = psProgram[nPC].nR1;
			psM1set = CreateInteger (psProgram[nPC].nM1);
			while (nRef > 1) {
				psM1set = CreateBinary (OPBINARY_IND, CreateMemory (), psM1set);
				nRef--;
			}
		} else {
			psM1set = NULL;
		}

		if (psProgram[nPC].nR2 > REF_INVALID) {
			nRef = psProgram[nPC].nR2;
			psM2 = CreateInteger (psProgram[nPC].nM2);
			while (nRef > 0) {
				psM2 = CreateBinary (OPBINARY_IND, CreateMemory (), psM2);
				nRef--;
			}
		}

		/*
		 * psM2set is only used in the LIB commands, where M1 is used for the ;
		 * library value
		 */
		if (psProgram[nPC].nR2 > 0) {
			nRef = psProgram[nPC].nR2;
			psM2set = CreateInteger (psProgram[nPC].nM2);
			while (nRef > 1) {
				psM2set = CreateBinary (OPBINARY_IND, CreateMemory (), psM2set);
				nRef--;
			}
		} else {
			psM2set = NULL;
		}

		if (psProgram[nPC].nR3 > REF_INVALID) {
			nRef = psProgram[nPC].nR3;
			psM3 = CreateInteger (psProgram[nPC].nM3);
			while (nRef > 0) {
				psM3 = CreateBinary (OPBINARY_IND, CreateMemory (), psM3);
				nRef--;
			}
		}

		switch (psProgram[nPC].inst) {
		case mnemonic_mov:
			psMemSub = CreateTernary (OPTERNARY_SET, CreateMemory (), COPY (psM1set), COPY (psM3));
			apsOp[nPC] = SubstituteOperationPair (COPY (apsOp[nPC + 1]), psM1, psM3, psMem, psMemSub);
			break;
		case mnemonic_add:
			psSub = CreateBinary (OPBINARY_ADD, COPY (psM2), COPY (psM3));
			psMemSub = CreateTernary (OPTERNARY_SET, CreateMemory (), COPY (psM1set), COPY (psSub));
			apsOp[nPC] = SubstituteOperationPair (COPY (apsOp[nPC + 1]), psM1, psSub, psMem, psMemSub);
			break;
		case mnemonic_sub:
			psSub = CreateBinary (OPBINARY_SUB, COPY (psM2), COPY (psM3));
			psMemSub = CreateTernary (OPTERNARY_SET, CreateMemory (), COPY (psM1set), COPY (psSub));
			apsOp[nPC] = SubstituteOperationPair (COPY (apsOp[nPC + 1]), psM1, psSub, psMem, psMemSub);
			break;
		case mnemonic_mul:
			psSub = CreateBinary (OPBINARY_MUL, COPY (psM2), COPY (psM3));
			psMemSub = CreateTernary (OPTERNARY_SET, CreateMemory (), COPY (psM1set), COPY (psSub));
			apsOp[nPC] = SubstituteOperationPair (COPY (apsOp[nPC + 1]), psM1, psSub, psMem, psMemSub);
			break;
		case mnemonic_div:
			psSub = CreateBinary (OPBINARY_DIV, COPY (psM2), COPY (psM3));
			psMemSub = CreateTernary (OPTERNARY_SET, CreateMemory (), COPY (psM1set), COPY (psSub));
			apsOp[nPC] = SubstituteOperationPair (COPY (apsOp[nPC + 1]), psM1, psSub, psMem, psMemSub);
			break;
		case mnemonic_mod:
			psSub = CreateBinary (OPBINARY_MOD, COPY (psM2), COPY (psM3));
			psMemSub = CreateTernary (OPTERNARY_SET, CreateMemory (), COPY (psM1set), COPY (psSub));
			apsOp[nPC] = SubstituteOperationPair (COPY (apsOp[nPC + 1]), psM1, psSub, psMem, psMemSub);
			break;
		case mnemonic_orr:
			psSub = CreateBinary (OPBINARY_OR, COPY (psM2), COPY (psM3));
			psMemSub = CreateTernary (OPTERNARY_SET, CreateMemory (), COPY (psM1set), COPY (psSub));
			apsOp[nPC] = SubstituteOperationPair (COPY (apsOp[nPC + 1]), psM1, psSub, psMem, psMemSub);
			break;
		case mnemonic_and:
			psSub = CreateBinary (OPBINARY_AND, COPY (psM2), COPY (psM3));
			psMemSub = CreateTernary (OPTERNARY_SET, CreateMemory (), COPY (psM1set), COPY (psSub));
			apsOp[nPC] = SubstituteOperationPair (COPY (apsOp[nPC + 1]), psM1, psSub, psMem, psMemSub);
			break;
		case mnemonic_bic:
			psSub = CreateBinary (OPBINARY_BIC, COPY (psM2), COPY (psM3));
			psMemSub = CreateTernary (OPTERNARY_SET, CreateMemory (), COPY (psM1set), COPY (psSub));
			apsOp[nPC] = SubstituteOperationPair (COPY (apsOp[nPC + 1]), psM1, psSub, psMem, psMemSub);
			break;
		case mnemonic_eor:
			psSub = CreateBinary (OPBINARY_EOR, COPY (psM2), COPY (psM3));
			psMemSub = CreateTernary (OPTERNARY_SET, CreateMemory (), COPY (psM1set), COPY (psSub));
			apsOp[nPC] = SubstituteOperationPair (COPY (apsOp[nPC + 1]), psM1, psSub, psMem, psMemSub);
			break;
		case mnemonic_ror:
			psSub = CreateBinary (OPBINARY_ROR, COPY (psM2), COPY (psM3));
			psMemSub = CreateTernary (OPTERNARY_SET, CreateMemory (), COPY (psM1set), COPY (psSub));
			apsOp[nPC] = SubstituteOperationPair (COPY (apsOp[nPC + 1]), psM1, psSub, psMem, psMemSub);
			break;
		case mnemonic_rol:
			psSub = CreateBinary (OPBINARY_ROL, COPY (psM2), COPY (psM3));
			psMemSub = CreateTernary (OPTERNARY_SET, CreateMemory (), COPY (psM1set), COPY (psSub));
			apsOp[nPC] = SubstituteOperationPair (COPY (apsOp[nPC + 1]), psM1, psSub, psMem, psMemSub);
			break;
		case mnemonic_beq:
		case mnemonic_bne:
		case mnemonic_blt:
		case mnemonic_ble:
		case mnemonic_bgt:
		case mnemonic_bge:
			if (psProgram[nPC].inst == mnemonic_beq) {
				eBranch = OPBINARY_EQ;
				eNoBranch = OPBINARY_NE;
			}

			if (psProgram[nPC].inst == mnemonic_bne) {
				eBranch = OPBINARY_NE;
				eNoBranch = OPBINARY_EQ;
			}

			if (psProgram[nPC].inst == mnemonic_blt) {
				eBranch = OPBINARY_LT;
				eNoBranch = OPBINARY_GE;
			}

			if (psProgram[nPC].inst == mnemonic_ble) {
				eBranch = OPBINARY_LE;
				eNoBranch = OPBINARY_GT;
			}

			if (psProgram[nPC].inst == mnemonic_bgt) {
				eBranch = OPBINARY_GT;
				eNoBranch = OPBINARY_LE;
			}

			if (psProgram[nPC].inst == mnemonic_bge) {
				eBranch = OPBINARY_GE;
				eNoBranch = OPBINARY_LT;
			}

			if (psProgram[nPC].nM1 > nEnd) {
				nPCBranch = nEnd;
			} else {
				nPCBranch = psProgram[nPC].nM1;
			}

			if (apsOp[nPCBranch]) {

				/* Branch forwards */
				apsOp[nPC] = CreateBinary (eBranch, COPY (psM2), COPY (psM3));
				apsOp[nPC] = CreateBinary (OPBINARY_LIMP, apsOp[nPC], COPY (apsOp[nPCBranch]));
				psTemp = CreateBinary (eNoBranch, COPY (psM2), COPY (psM3));
				psTemp = CreateBinary (OPBINARY_LIMP, psTemp, COPY (apsOp[nPC + 1]));
				apsOp[nPC] = CreateBinary (OPBINARY_LAND, apsOp[nPC], psTemp);
				psTemp = NULL;
			} else {

				/*
				 * Branch backwards - we're going to unravel the loop a ;
				 * certain number of times
				 */
				psFind = CreateUnary (OPUNARY_VC, CreateInteger (nPC));
				apsOp[nPC] = CreateBinary (eBranch, COPY (psM2), COPY (psM3));

				/*
				 * Add a marker, since we haven't reached the branch point yet ;
				 * This shouldn't be affected by anything else
				 */
				apsOp[nPC] = CreateBinary (OPBINARY_LIMP, apsOp[nPC], COPY (psFind));
				psTemp = CreateBinary (eNoBranch, COPY (psM2), COPY (psM3));
				psTemp = CreateBinary (OPBINARY_LIMP, psTemp, COPY (apsOp[nPC + 1]));
				apsOp[nPC] = CreateBinary (OPBINARY_LAND, apsOp[nPC], psTemp);
				psTemp = NULL;
				psTemp = COPY (apsOp[nPC]);

				for (nBranchLoop = 0; nBranchLoop < (UNRAVELMAX - 1); nBranchLoop++) {

					/* Run through the commands in the loop */
					psSub = ConvertToLogic (psProgram, nPCBranch, nPC /* + 1 */, pnMemory, apsOp[nPC]);
					FreeRecursive (apsOp[nPC]);

					/*
					 * Replace the previous VC reference with the new operation ;
					 * structure
					 */
					apsOp[nPC] = SubstituteOperation (COPY (psTemp), psFind, psSub);
					FreeRecursive (psSub);
					psSub = NULL;
				}

				FreeRecursive (psFind);
				psFind = NULL;
				FreeRecursive (psTemp);
				psTemp = NULL;
			}

			break;
		case mnemonic_lib:

			/*
			 * Depends on the LIB in question ;
			 * Note we really ought to check if psProgram[nPC].nR1 != 0 here
			 */
			switch ((lib) psProgram[nPC].nM1) {
			default:
			case lib_printString:
			case lib_printInteger:
			case lib_printNumber:
			case lib_inputString:
			case lib_inputNumber:
			case lib_sendState:
			case lib_checkHost:
			case lib_playSound:

				/* No effect at the moment */
				apsOp[nPC] = COPY (apsOp[nPC + 1]);
				break;
			case lib_sendInteger:
				if (gpApplyMidCondition_LibSendInteger) {
					gpApplyMidCondition_LibSendInteger (apsOp, nPC, psM1, psM2, psM3, psMem, psM1set, psM2set);
				} else {
					apsOp[nPC] = COPY (apsOp[nPC + 1]);
				}

				break;
			case lib_receiveInteger:
				if (gpApplyMidCondition_LibReceiveInteger) {
					gpApplyMidCondition_LibReceiveInteger (apsOp, nPC, psM1, psM2, psM3, psMem, psM1set, psM2set);
				} else {
					apsOp[nPC] = COPY (apsOp[nPC + 1]);
				}

				break;
			case lib_heapAlloc:
				if (gpApplyMidCondition_LibHeapAlloc) {
					gpApplyMidCondition_LibHeapAlloc (apsOp, nPC, psM1, psM2, psM3, psMem, psM1set, psM2set,
													  &gnHeapAllocs);
				} else {
					apsOp[nPC] = COPY (apsOp[nPC + 1]);
				}

				break;
			}

			break;
		default:

			/*
			 * psAgent->ConsolePrint ("Unknown operation\n");
			 */
			apsOp[nPC] = COPY (apsOp[nPC + 1]);
			break;
		}

		apsOp[nPC] = SimplifyOperation (apsOp[nPC]);

		if (psM1) {
			FreeRecursive (psM1);
			psM1 = NULL;
		}

		if (psM1set) {
			FreeRecursive (psM1set);
			psM1set = NULL;
		}

		if (psM2) {
			FreeRecursive (psM2);
			psM2 = NULL;
		}

		if (psM2set) {
			FreeRecursive (psM2set);
			psM2set = NULL;
		}

		if (psM3) {
			FreeRecursive (psM3);
			psM3 = NULL;
		}

		if (psMemSub) {
			FreeRecursive (psMemSub);
			psMemSub = NULL;
		}

		if (psSub) {
			FreeRecursive (psSub);
			psSub = NULL;
		}

		if (gnTimeAnalyse == 0) {
			if (gboPrintAll) {
				char szOutput[1024 * 128];
				sprintf (szOutput, "Output %3d :-\n", nPC);

				/*
				 * psAgent->ConsolePrint (szOutput);
				 */
				OperationString (apsOp[nPC], szOutput, sizeof (szOutput));

				/*
				 * psAgent->ConsolePrint (szOutput);
				 */
			} else {
				char szOutput[12];
				sprintf (szOutput, "%3d, ", nPC);
				gpsAgent->ConsolePrint (szOutput);
			}
		}

		nPC--;
	}

	psTemp = apsOp[nStart];

	/* Free up the operation memory */
	nPC = nEnd; /* 1;
				 * */
	while (nPC > nStart) {
		FreeRecursive (apsOp[nPC]);
		apsOp[nPC] = NULL;
		nPC--;
	}

	free (apsOp);
	apsOp = NULL;

	/* Free up our "memory" variable */
	if (psMem) {
		FreeRecursive (psMem);
	}

	return psTemp;
}
