/*$T MATTS/Conds-id3.cpp GC 1.140 07/01/09 21:12:09 */
/*/
 *  Pre and Post conditions  */
#include "cAnalyse.h"

#include <stdio.h>

/*/
 *  ACCESS CONTROL TEST 01 Defines Global variables  */
/*
 * static int gnLink = 0;
 */
static int gnCountReceived;

/*/
 =======================================================================================================================
 *  Set up the post condition (applied after code has terminated)
 =======================================================================================================================
 */
Operation* CreatePostCondition_id3 (void) {
	Operation*	psPost;

	psPost = NULL;
	psPost = CreateBinary (OPBINARY_EQ, CreateVariable ("sent"), CreateVariable ("test"));

	gnCountReceived = 0;

	return psPost;
}

/*/
 =======================================================================================================================
 *  Apply the pre condition (applied before code is executed)
 =======================================================================================================================
 */
Operation* ApplyPreCondition_id3 (Operation* psOp) {
	int nCount;
	Operation*	psFind;
	Operation*	psTemp;
	char szTemp[1024];
	Operation*	psPost;

	psPost = NULL;

	printf ("Testing for %d variables...\n", gnCountReceived);

	psTemp = NULL;
	for (nCount = 0; (nCount < gnCountReceived) && (psTemp == NULL); nCount++) {
		sprintf (szTemp, "r[%d]", nCount);
		psFind = CreateVariable (szTemp);
		psTemp = NULL;
		psTemp = FindOperation (psOp, psFind);
		FreeRecursive (psFind);
		psFind = NULL;
	}

	if (psTemp) {
		psPost = CreateTruthValue (TRUE);
	} else {
		psPost = CreateTruthValue (FALSE);
	}

	return psPost;
}

/*/
 =======================================================================================================================
 *  Apply the mid condition for LIB SendInteger
 =======================================================================================================================
 */
void ApplyMidCondition_LibSendInteger_id3 (Operation **	 apsOp, int nPC, Operation*	 psM1, Operation*  psM2,
										   Operation*  psM3, Operation*	 psMem, Operation*	psM1set, Operation*	 psM2set) {
	Operation*	psFind;
	Operation*	psSub;

	psFind = NULL;
	psSub = NULL;

	/*
	 * channel = sc[nM3] ;
	 * send = nM2 ;
	 * ADD sent sent nM2 ;
	 * ADD sent sent nM2
	 */
	psFind = CreateVariable ("sent");

	/* Should really be dependant on nM3 */
	psSub = CreateBinary (OPBINARY_ADD, CreateVariable ("sent"), COPY (psM2));
	apsOp[nPC] = SubstituteOperation (COPY (apsOp[nPC + 1]), psFind, psSub);

	FreeRecursive (psFind);
	psFind = NULL;
	FreeRecursive (psSub);
	psSub = NULL;
}

/*/
 =======================================================================================================================
 *  Apply the mid condition for LIB ReceiveInteger
 =======================================================================================================================
 */
void ApplyMidCondition_LibReceiveInteger_id3 (Operation **	apsOp, int nPC, Operation*	psM1, Operation*  psM2,
											  Operation*  psM3, Operation*	psMem, Operation*  psM1set,
											  Operation*  psM2set) {
	Operation*	psMemSub;
	Operation*	psSub;
	char szTemp[1024];

	psMemSub = NULL;
	psSub = NULL;

	/*
	 * channel = sc[nM3] ;
	 * receive = nM2 ;
	 * Establish a variable for the received data
	 */
	sprintf (szTemp, "r[%d]", gnCountReceived);
	gnCountReceived++;

	/* MOV *nM2 gnCountReceived */
	psSub = CreateVariable (szTemp);
	psMemSub = CreateTernary (OPTERNARY_SET, CreateMemory (), COPY (psM2set), CreateVariable (szTemp));
	apsOp[nPC] = SubstituteOperationPair (COPY (apsOp[nPC + 1]), psM2, psSub, psMem, psMemSub);
	FreeRecursive (psSub);
	psSub = NULL;
	FreeRecursive (psMemSub);
	psMemSub = NULL;
}

/*/
 =======================================================================================================================
 *  Apply the mid condition for LIB HeapAlloc
 =======================================================================================================================
 */
void ApplyMidCondition_LibHeapAlloc_id3 (Operation **  apsOp, int nPC, Operation*  psM1, Operation*	 psM2,
										 Operation*	 psM3, Operation*  psMem, Operation*  psM1set, Operation*  psM2set,
										 int*  pnHeapAllocs) {
}
