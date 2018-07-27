/*$T MATTS/Conds-id1.h GC 1.140 07/01/09 21:12:09 */
/*/
 *  Pre and Post conditions  */
#if !defined _H_CONDS_ID1
#define _H_CONDS_ID1

#include "Prop.h"

/*/
 *  Set up the post condition (applied after code has terminated)
 */
Operation*	CreatePostCondition_id1 (void);

/*/
 *  Apply the pre condition (applied before code is executed)
 */
Operation*	ApplyPreCondition_id1 (Operation* psOp);

/*/
 *  Apply the mid condition for LIB SendInteger  */
void ApplyMidCondition_LibSendInteger_id1 (Operation **	 apsOp, int nPC, Operation*	 psM1, Operation*  psM2,
										   Operation*  psM3, Operation*	 psMem, Operation*	psM1set, Operation*	 psM2set);

/*/
 *  Apply the mid condition for LIB ReceiveInteger  */
void ApplyMidCondition_LibReceiveInteger_id1 (Operation **	apsOp, int nPC, Operation*	psM1, Operation*  psM2,
											  Operation*  psM3, Operation*	psMem, Operation*  psM1set,
											  Operation*  psM2set);

/*/
 *  Apply the mid condition for LIB HeapAlloc  */
void ApplyMidCondition_LibHeapAlloc_id1 (Operation **  apsOp, int nPC, Operation*  psM1, Operation*	 psM2,
										 Operation*	 psM3, Operation*  psMem, Operation*  psM1set, Operation*  psM2set,
										 int*  pnHeapAllocs);
#endif /* if !defined _H_CONDS_ID1 */
