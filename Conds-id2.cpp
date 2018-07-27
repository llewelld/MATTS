/*$T MATTS/Conds-id2.cpp GC 1.140 07/01/09 21:12:09 */
/*/
 *  Pre and Post conditions  */
#include "cAnalyse.h"

#include <stdio.h>

/*/
 *  BUFFER OVERRUN TEST 01 (Sending) Defines
 */
#define BUFFERRECEIVELEN	(4)
#define BUFFERRECEIVENUM	(64)
#define BUFFERRECEIVEEND	(0)
#define BUFFERHEAPSTART		(1000)

/*/
 *  Global variables  */
static int gnLink = 0;

/*/
 =======================================================================================================================
 *  Set up the post condition (applied after code has terminated)
 =======================================================================================================================
 */
Operation* CreatePostCondition_id2 (void) {
	Operation*	psPost;

	psPost = NULL;
	psPost = CreateBinary (OPBINARY_LT, CreateVariable ("chanmax"), CreateVariable ("chantest"));

	return psPost;
}

/*/
 =======================================================================================================================
 *  Apply the pre condition (applied before code is executed)
 =======================================================================================================================
 */
Operation* ApplyPreCondition_id2 (Operation* psOp) {
	Operation*	psFind;
	Operation*	psSub;

	psFind = NULL;
	psSub = NULL;

	/* MOV sc[nM3] 0 */
	psFind = CreateVariable ("channel");

	/* Should really be dependant on nM3 */
	psSub = CreateInteger (0);
	psOp = SubstituteOperation (psOp, psFind, psSub);
	FreeRecursive (psFind);
	psFind = NULL;
	FreeRecursive (psSub);
	psSub = NULL;

	/* MOV chanmax 0 */
	psFind = CreateVariable ("chanmax");
	psSub = CreateInteger (0);
	psOp = SubstituteOperation (psOp, psFind, psSub);
	FreeRecursive (psFind);
	psFind = NULL;
	FreeRecursive (psSub);
	psSub = NULL;

	/* MOV counter 0 */
	psFind = CreateVariable ("counter");
	psSub = CreateInteger (0);
	psOp = SubstituteOperation (psOp, psFind, psSub);
	FreeRecursive (psFind);
	psFind = NULL;
	FreeRecursive (psSub);
	psSub = NULL;

	/*
	 * // MOV chantest 3 ;
	 * psFind = CreateVariable ("chantest");
	 * psSub = CreateInteger (5);
	 * psOp = SubstituteOperation (psOp, psFind, psSub);
	 * FreeRecursive (psFind);
	 * psFind = NULL;
	 * FreeRecursive (psSub);
	 * psSub = NULL;
	 */
	return psOp;
}

/*/
 =======================================================================================================================
 *  Apply the mid condition for LIB SendInteger
 =======================================================================================================================
 */
void ApplyMidCondition_LibSendInteger_id2 (Operation **	 apsOp, int nPC, Operation*	 psM1, Operation*  psM2,
										   Operation*  psM3, Operation*	 psMem, Operation*	psM1set, Operation*	 psM2set) {
	Operation*	psTemp;
	Operation*	psFind;
	Operation*	psSub;

	psTemp = NULL;
	psFind = NULL;
	psSub = NULL;

	/*
	 * For this one, we have lots to do! ;
	 * channel = sc[nM3] ;
	 * send = nM2 ;
	 * BNE noreset nM3 gnLink ;
	 * ADD sc[nM3] sc[nM3] 1 ;
	 * BNE noreset nM2 0 ;
	 * BLE nomax sc[nM3] chanmax ;
	 * MOV chanmax sc[nM3] ;
	 * .nomax ;
	 * MOV sc[nM3] 0 ;
	 * .noreset ;
	 * MOV sc[nM3] 0
	 */
	psFind = CreateVariable ("channel");

	/* Should really be dependant on nM3 */
	psSub = CreateInteger (0);
	apsOp[nPC] = SubstituteOperation (COPY (apsOp[nPC + 1]), psFind, psSub);
	FreeRecursive (psFind);
	psFind = NULL;
	FreeRecursive (psSub);
	psSub = NULL;

	psTemp = COPY (apsOp[nPC]);

	/* MOV chanmax sc[nM3] */
	psFind = CreateVariable ("chanmax");
	psSub = CreateVariable ("channel");
	apsOp[nPC] = SubstituteOperation (COPY (apsOp[nPC]), psFind, psSub);
	FreeRecursive (psFind);
	psFind = NULL;
	FreeRecursive (psSub);
	psSub = NULL;

	/*
	 * BLE nomax sc[nM3] chanmax ;
	 * Branch forwards
	 */
	psSub = CreateBinary (OPBINARY_GT, CreateVariable ("channel"), CreateVariable ("chanmax"));
	apsOp[nPC] = CreateBinary (OPBINARY_LIMP, psSub, apsOp[nPC]);
	psSub = CreateBinary (OPBINARY_LE, CreateVariable ("channel"), CreateVariable ("chanmax"));
	psSub = CreateBinary (OPBINARY_LIMP, psSub, psTemp);
	apsOp[nPC] = CreateBinary (OPBINARY_LAND, apsOp[nPC], psSub);
	psSub = NULL;
	psTemp = NULL;

	/*
	 * BNE noreset nM2 0 ;
	 * Branch forwards
	 */
	psSub = CreateBinary (OPBINARY_EQ, COPY (psM2), CreateInteger (0));
	apsOp[nPC] = CreateBinary (OPBINARY_LIMP, psSub, apsOp[nPC]);
	psSub = CreateBinary (OPBINARY_NE, COPY (psM2), CreateInteger (0));
	psSub = CreateBinary (OPBINARY_LIMP, psSub, COPY (apsOp[nPC + 1]));
	apsOp[nPC] = CreateBinary (OPBINARY_LAND, apsOp[nPC], psSub);
	psSub = NULL;

	/* ADD sc[nM3] sc[nM3] 1 */
	psFind = CreateVariable ("channel");

	/* Should really be dependant on nM3 */
	psSub = CreateBinary (OPBINARY_ADD, CreateVariable ("channel"), CreateInteger (1));
	apsOp[nPC] = SubstituteOperation (apsOp[nPC], psFind, psSub);
	FreeRecursive (psFind);
	psFind = NULL;

	/*
	 * BNE noreset nM3 gnLink ;
	 * Branch forwards
	 */
	psSub = CreateBinary (OPBINARY_EQ, COPY (psM3), CreateInteger (gnLink));
	apsOp[nPC] = CreateBinary (OPBINARY_LIMP, psSub, apsOp[nPC]);
	psSub = CreateBinary (OPBINARY_NE, COPY (psM3), CreateInteger (gnLink));
	psSub = CreateBinary (OPBINARY_LIMP, psSub, COPY (apsOp[nPC + 1]));
	apsOp[nPC] = CreateBinary (OPBINARY_LAND, apsOp[nPC], psSub);
	psSub = NULL;
}

/*/
 =======================================================================================================================
 *  Apply the mid condition for LIB ReceiveInteger
 =======================================================================================================================
 */
void ApplyMidCondition_LibReceiveInteger_id2 (Operation **	apsOp, int nPC, Operation*	psM1, Operation*  psM2,
											  Operation*  psM3, Operation*	psMem, Operation*  psM1set,
											  Operation*  psM2set) {
	Operation*	psTemp;
	Operation*	psFind;
	Operation*	psMemSub;
	Operation*	psSub;

	psTemp = NULL;
	psFind = NULL;
	psMemSub = NULL;
	psSub = NULL;

	/*
	 * For this one, we have lots to do as well! ;
	 * channel = sc[nM3] ;
	 * receive = nM2 ;
	 * BNE noreceive nM3 gnLink ;
	 * ADD counter counter 1 ;
	 * (((counter MOD BUFFERRECEIVELEN) = 0) ;
	 * -> MOV *address BUFFERRECEIVEEND) ;
	 * ^ (((counter MOD BUFFERRECEIVELEN) != 0) ;
	 * -> MOV *address BUFFERRECEIVENUM) ;
	 * .noreceive ;
	 * (((counter MOD BUFFERRECEIVELEN) = 0) ;
	 * -> MOV *address BUFFERRECEIVEEND) ;
	 * ^ (((counter MOD BUFFERRECEIVELEN) != 0) ;
	 * -> MOV *address BUFFERRECEIVENUM)
	 */
	psSub = CreateInteger (BUFFERRECEIVEEND);
	psMemSub = CreateTernary (OPTERNARY_SET, CreateMemory (), COPY (psM2set), CreateInteger (BUFFERRECEIVEEND));
	psTemp = SubstituteOperationPair (COPY (apsOp[nPC + 1]), psM2, psSub, psMem, psMemSub);
	FreeRecursive (psSub);
	psSub = NULL;
	FreeRecursive (psMemSub);
	psMemSub = NULL;

	psSub = CreateInteger (BUFFERRECEIVENUM);
	psMemSub = CreateTernary (OPTERNARY_SET, CreateMemory (), COPY (psM2set), CreateInteger (BUFFERRECEIVENUM));
	psFind = SubstituteOperationPair (COPY (apsOp[nPC + 1]), psM2, psSub, psMem, psMemSub);
	FreeRecursive (psSub);
	psSub = NULL;

	apsOp[nPC] = CreateBinary (OPBINARY_LAND, CreateBinary (OPBINARY_LIMP, CreateBinary (OPBINARY_EQ,
							   CreateBinary (OPBINARY_MOD, CreateVariable ("counter"), CreateInteger (BUFFERRECEIVELEN)),
							   CreateInteger (0)), psTemp), CreateBinary (OPBINARY_LIMP, CreateBinary (OPBINARY_NE,
							   CreateBinary (OPBINARY_MOD, CreateVariable ("counter"), CreateInteger (BUFFERRECEIVELEN)),
							   CreateInteger (0)), psFind));

	psTemp = NULL;
	psFind = NULL;

	/* ADD counter counter 1 */
	psFind = CreateVariable ("counter");
	psSub = CreateBinary (OPBINARY_ADD, CreateVariable ("counter"), CreateInteger (1));
	apsOp[nPC] = SubstituteOperation (apsOp[nPC], psFind, psSub);
	FreeRecursive (psFind);
	psFind = NULL;

	/*
	 * BNE noreceive nM3 gnLink ;
	 * Branch forwards
	 */
	psSub = CreateBinary (OPBINARY_EQ, COPY (psM3), CreateInteger (gnLink));
	apsOp[nPC] = CreateBinary (OPBINARY_LIMP, psSub, apsOp[nPC]);
	psSub = CreateBinary (OPBINARY_NE, COPY (psM3), CreateInteger (gnLink));
	psSub = CreateBinary (OPBINARY_LIMP, psSub, COPY (apsOp[nPC + 1]));
	apsOp[nPC] = CreateBinary (OPBINARY_LAND, apsOp[nPC], psSub);
	psSub = NULL;
}

/*/
 =======================================================================================================================
 *  Apply the mid condition for LIB HeapAlloc
 =======================================================================================================================
 */
void ApplyMidCondition_LibHeapAlloc_id2 (Operation **  apsOp, int nPC, Operation*  psM1, Operation*	 psM2,
										 Operation*	 psM3, Operation*  psMem, Operation*  psM1set, Operation*  psM2set,
										 int*  pnHeapAllocs) {
	Operation*	psFind;
	Operation*	psMemSub;
	Operation*	psSub;
	char szTemp[1024];

	psFind = NULL;
	psMemSub = NULL;
	psSub = NULL;

	/*
	 * Add lots more to do here! ;
	 * MOV nM2 heap ;
	 * ADD heap[i] heap nM3 ;
	 * MOV *heap[i] marker ;
	 * ADD heap heap[i] 1 ;
	 * Establish a variable for the marker position for this heap
	 */
	sprintf (szTemp, "heap[%d]", *pnHeapAllocs);
	(*pnHeapAllocs)++;

	/* ADD heap heap[i] 1 */
	psFind = CreateVariable ("heap");
	psSub = CreateBinary (OPBINARY_ADD, CreateVariable (szTemp /* "heap[i] */ ), CreateInteger (1));
	apsOp[nPC] = SubstituteOperation (COPY (apsOp[nPC + 1]), psFind, psSub);
	FreeRecursive (psFind);
	psFind = NULL;
	FreeRecursive (psSub);
	psSub = NULL;

	/* MOV *heap[i] marker */
	psMemSub = CreateTernary (OPTERNARY_SET, CreateMemory (), CreateVariable (szTemp /* "heap[i] */ ),
							  CreateVariable ("marker"));
	psFind = CreateBinary (OPBINARY_IND, CreateMemory (), CreateVariable (szTemp /* "heap[i] */ ));
	psSub = CreateVariable ("marker");
	apsOp[nPC] = SubstituteOperationPair (apsOp[nPC], psFind, psSub, psMem, psMemSub);
	FreeRecursive (psMemSub);
	psMemSub = NULL;
	FreeRecursive (psFind);
	psFind = NULL;
	FreeRecursive (psSub);
	psSub = NULL;

	/* ADD heap[i] heap nM3 */
	psFind = CreateVariable (szTemp /* "heap[i] */ );
	psSub = CreateBinary (OPBINARY_ADD, CreateVariable ("heap"), COPY (psM3));
	apsOp[nPC] = SubstituteOperation (apsOp[nPC], psFind, psSub);
	FreeRecursive (psFind);
	psFind = NULL;
	FreeRecursive (psSub);
	psSub = NULL;

	/* MOV nM2 heap */
	psMemSub = CreateTernary (OPTERNARY_SET, CreateMemory (), COPY (psM2set), CreateVariable ("heap"));
	psSub = CreateVariable ("heap");
	apsOp[nPC] = SubstituteOperationPair (apsOp[nPC], psM2, psSub, psMem, psMemSub);
}
