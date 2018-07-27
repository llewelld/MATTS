/*$T MATTS/PrePost/Ex01.c GC 1.140 07/01/09 21:12:10 */
/*/
 *  Pre and Post conditions  */
#include "cAnalyse.h"

/*/
 =======================================================================================================================
 *  Ex01 Test Set up the post condition (applied after code has terminated)
 =======================================================================================================================
 */
Operation* CreatePostCondition (void) {
	Operation*	psPost;

	psPost = NULL;
	psPost = CreateBinary (OPBINARY_IND, CreateMemory (), CreateInteger (0));
	psPost = CreateBinary (OPBINARY_EQ, psPost, CreateInteger (1));

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
	Operation*	psMem;
	Operation*	psMemSub;

	psFind = NULL;
	psSub = NULL;
	psMem = NULL;
	psMemSub = NULL;

	psMem = CreateMemory ();
	psFind = CreateBinary (OPBINARY_IND, CreateMemory (), CreateInteger (0));
	psSub = CreateInteger (254);

	psMemSub = CreateTernary (OPTERNARY_SET, CreateMemory (), CreateInteger (0), CreateInteger (254));
	psOp = SubstituteOperationPair (psOp, psFind, psSub, psMem, psMemSub);

	FreeRecursive (psFind);
	psFind = NULL;
	FreeRecursive (psSub);
	psSub = NULL;
	FreeRecursive (psMemSub);
	psMemSub = NULL;
	FreeRecursive (psMem);
	psMem = NULL;

	return psOp;
}
