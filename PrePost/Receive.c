/*$T MATTS/PrePost/Receive.c GC 1.140 07/01/09 21:12:10 */
/*/
 *  Pre and Post conditions  */
#include "Local.h"
#include "Inter.h"

/*/
 *  BUFFER OVERRUN TEST GENERAL (Receiving)
 */
#define BUFFERHEAPSTART (1000)

/*/
 =======================================================================================================================
 *  Set up the post condition (applied after code has terminated)
 =======================================================================================================================
 */
Operation* CreatePostCondition (void) {
	Operation*	psPost;

	psPost = NULL;
	psPost = CreateBinary (OPBINARY_IND, CreateMemory (), CreateVariable ("heap[1]"));

	/*
	 * CreateInteger (BUFFERHEAPSTART + 3));
	 */
	psPost = CreateBinary (OPBINARY_EQ, psPost, CreateVariable ("marker"));

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

	/* MOV heap BUFFERHEAPSTART */
	psFind = CreateVariable ("heap");
	psSub = CreateInteger (BUFFERHEAPSTART);
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

	return psOp;
}
