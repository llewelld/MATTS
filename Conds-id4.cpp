/*$T MATTS/Conds-id4.cpp GC 1.140 07/01/09 21:12:09 */
/*/
 *  Pre and Post conditions  */
#include "cAnalyse.h"

#include <stdio.h>

/*/
 *  ACCESS CONTROL TEST 02 Defines  */
#define CHANNEL_IN	(0)
#define CHANNEL_OUT (0)

/*/
 *  Global variables  */
/*
 * static int gnLink = 0;
 */
static int gnCountReceived;
static int gnCountNotReceived;

static int gnCheckChannelIn = CHANNEL_IN;
static int gnCheckChannelOut = CHANNEL_OUT;

/*/
 =======================================================================================================================
 *  Set up the post condition (applied after code has terminated)
 =======================================================================================================================
 */
Operation* CreatePostCondition_id4 (int nCheckChannelIn, int nCheckChannelOut) {
	Operation*	psPost;

	psPost = NULL;
	gnCheckChannelIn = nCheckChannelIn;
	gnCheckChannelOut = nCheckChannelOut;

	gnCountReceived = 0;
	gnCountNotReceived = 0;

	psPost = CreateBinary (OPBINARY_EQ, CreateVariable ("sent"), CreateVariable ("test"));

	return psPost;
}

/*/
 =======================================================================================================================
 *  Apply the pre condition (applied before code is executed)
 =======================================================================================================================
 */
Operation* ApplyPreCondition_id4 (Operation* psOp) {
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

	FreeRecursive (psOp);
	psOp = NULL;

	return psPost;
}

/*/
 =======================================================================================================================
 *  Apply the mid condition for LIB SendInteger
 =======================================================================================================================
 */
void ApplyMidCondition_LibSendInteger_id4 (Operation **	 apsOp, int nPC, Operation*	 psM1, Operation*  psM2,
										   Operation*  psM3, Operation*	 psMem, Operation*	psM1set, Operation*	 psM2set) {
	Operation*	psFind;
	Operation*	psSub;
	Operation*	psTemp;

	psFind = NULL;
	psSub = NULL;
	psTemp = NULL;

	/*
	 * channel = sc[nM3] ;
	 * send = nM2 ;
	 * BNE skip nM2 gnCheckChannelOut ;
	 * ADD sent sent nM2 ;
	 * .skip ;
	 * ADD sent sent nM2
	 */
	psFind = CreateVariable ("sent");

	/* Should really be dependant on nM3 */
	psSub = CreateBinary (OPBINARY_ADD, CreateVariable ("sent"), COPY (psM2));
	psTemp = SubstituteOperation (COPY (apsOp[nPC + 1]), psFind, psSub);

	FreeRecursive (psFind);
	psFind = NULL;
	FreeRecursive (psSub);
	psSub = NULL;

	/* BNE skip nM3 gnCheckChannelOut */
	apsOp[nPC] = CreateBinary (OPBINARY_EQ, COPY (psM3), CreateInteger (gnCheckChannelOut));
	apsOp[nPC] = CreateBinary (OPBINARY_LIMP, apsOp[nPC], psTemp);
	psTemp = NULL;

	psTemp = CreateBinary (OPBINARY_NE, COPY (psM3), CreateInteger (gnCheckChannelOut));
	psTemp = CreateBinary (OPBINARY_LIMP, psTemp, COPY (apsOp[nPC + 1]));

	apsOp[nPC] = CreateBinary (OPBINARY_LAND, apsOp[nPC], psTemp);
	psTemp = NULL;
}

/*/
 =======================================================================================================================
 *  Apply the mid condition for LIB ReceiveInteger
 =======================================================================================================================
 */
void ApplyMidCondition_LibReceiveInteger_id4 (Operation **	apsOp, int nPC, Operation*	psM1, Operation*  psM2,
											  Operation*  psM3, Operation*	psMem, Operation*  psM1set,
											  Operation*  psM2set) {
	Operation*	psMemSub;
	Operation*	psSub;
	char szTemp1[1024];
	Operation*	psTemp1;
	char szTemp2[1024];
	Operation*	psTemp2;
	Operation*	psTemp;

	psMemSub = NULL;
	psSub = NULL;
	psTemp1 = NULL;
	psTemp2 = NULL;
	psTemp = NULL;

	/*
	 * channel = sc[nM3] ;
	 * receive = nM2 ;
	 * Establish a variable for the received data
	 */
	sprintf (szTemp1, "r[%d]", gnCountReceived);
	gnCountReceived++;
	sprintf (szTemp2, "n[%d]", gnCountNotReceived);
	gnCountNotReceived++;

	/*
	 * BNE skip nM2 gnCheckChannelIn ;
	 * MOV *nM2 r[gnCountReceived] ;
	 * BEQ skip2 0 0 ;
	 * .skip ;
	 * MOV *nM2 s[gnCountNotReceived] ;
	 * .skip2 ;
	 * MOV *nM2 r[gnCountReceived]
	 */
	psSub = CreateVariable (szTemp1);
	psMemSub = CreateTernary (OPTERNARY_SET, CreateMemory (), COPY (psM2set), CreateVariable (szTemp1));
	psTemp1 = SubstituteOperationPair (COPY (apsOp[nPC + 1]), psM2, psSub, psMem, psMemSub);

	FreeRecursive (psSub);
	psSub = NULL;
	FreeRecursive (psMemSub);
	psMemSub = NULL;

	/* MOV *nM2 s[gnCountNotReceived] */
	psSub = CreateVariable (szTemp2);
	psMemSub = CreateTernary (OPTERNARY_SET, CreateMemory (), COPY (psM2set), CreateVariable (szTemp2));
	psTemp2 = SubstituteOperationPair (COPY (apsOp[nPC + 1]), psM2, psSub, psMem, psMemSub);

	FreeRecursive (psSub);
	psSub = NULL;
	FreeRecursive (psMemSub);
	psMemSub = NULL;

	/* BNE skip nM3 gnCheckChannelIn */
	apsOp[nPC] = CreateBinary (OPBINARY_EQ, COPY (psM3), CreateInteger (gnCheckChannelIn));
	apsOp[nPC] = CreateBinary (OPBINARY_LIMP, apsOp[nPC], psTemp1);
	psTemp1 = NULL;

	psTemp = CreateBinary (OPBINARY_NE, COPY (psM3), CreateInteger (gnCheckChannelIn));
	psTemp = CreateBinary (OPBINARY_LIMP, psTemp, psTemp2);
	psTemp2 = NULL;

	apsOp[nPC] = CreateBinary (OPBINARY_LAND, apsOp[nPC], psTemp);
	psTemp = NULL;
}

/*/
 =======================================================================================================================
 *  Apply the mid condition for LIB HeapAlloc
 =======================================================================================================================
 */
void ApplyMidCondition_LibHeapAlloc_id4 (Operation **  apsOp, int nPC, Operation*  psM1, Operation*	 psM2,
										 Operation*	 psM3, Operation*  psMem, Operation*  psM1set, Operation*  psM2set,
										 int*  pnHeapAllocs) {
	apsOp[nPC] = COPY (apsOp[nPC + 1]);
}
