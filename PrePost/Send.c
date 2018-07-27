/*$T MATTS/PrePost/Send.c GC 1.140 07/01/09 21:12:10 */
/*/
 *  Pre and Post conditions  */
#include "Local.h"
#include "Inter.h"

/*/
 =======================================================================================================================
 *  BUFFER OVERRUN TEST 01 (Sending) Set up the post condition (applied after code has terminated)
 =======================================================================================================================
 */
Operation* CreatePostCondition (void) {
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
Operation* ApplyPreCondition (Operation* psOp) {
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
