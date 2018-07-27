/*$T MATTS/PrePost/Simple.c GC 1.140 07/01/09 21:12:10 */
/*/
 *  Pre and Post conditions  */
#include "cAnalyse.h"

/*/
 =======================================================================================================================
 *  Simple test: "ind(M, 1) < 5" Set up the post condition (applied after code has terminated)
 =======================================================================================================================
 */
Operation* CreatePostCondition (void) {
	Operation*	psPost;

	psPost = NULL;
	psPost = CreateBinary (OPBINARY_IND, CreateMemory (), CreateInteger (1));
	psPost = CreateBinary (OPBINARY_LT, psPost, CreateInteger (5));

	return psPost;
}

/*/
 =======================================================================================================================
 *  Apply the pre condition (applied before code is executed)
 =======================================================================================================================
 */
Operation* ApplyPreCondition (Operation* psOp) {
	return psOp;
}
