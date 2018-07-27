/*$T MATTS/PrePost/Template.c GC 1.140 07/01/09 21:12:10 */
/*/
 *  Pre and Post conditions  */
#include "cAnalyse.h"

/*/
 =======================================================================================================================
 *  Title Set up the post condition (applied after code has terminated)
 =======================================================================================================================
 */
Operation* CreatePostCondition (void) {
	Operation*	psPost;

	psPost = NULL;
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

	return psOp;
}
