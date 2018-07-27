/*$T MATTS/Prop.h GC 1.140 07/01/09 21:12:11 */
/*/
 *  Proposition ;
 *  Allow the construction of nested propositions ;
 *  The Flying Pig! ;
 *  Started 5/8/2003
 */
#if !defined _H_PROP
#define _H_PROP

/*/
 *  Includes  */
/* include "local.h" */
#if defined _RISC_OS
#include "oslib/types.h"
#include "oslib/os.h"
#endif
#include "cVMTypes.h"

#include <stddef.h>

/*/
 *  Defines  */
/*
 * define _DEBUG ;
 * #define _MEM_PROFILE
 */
#ifndef TRUE
#define TRUE	(1)
#endif
#ifndef FALSE
#define FALSE	(0)
#endif

/*/
 -----------------------------------------------------------------------------------------------------------------------
 *  Structures
 -----------------------------------------------------------------------------------------------------------------------
 */
typedef struct _Operation Operation;

typedef enum _OPUNARY
{
	OPUNARY_INVALID = -1,
	OPUNARY_NOT,
	OPUNARY_VC,
	OPUNARY_NUM
} OPUNARY;

typedef enum _OPBINARY
{
	OPBINARY_INVALID= -1,
	OPBINARY_IND,
	OPBINARY_LAND,
	OPBINARY_LOR,
	OPBINARY_LIMP,
	OPBINARY_ADD,
	OPBINARY_SUB,
	OPBINARY_MUL,
	OPBINARY_DIV,
	OPBINARY_MOD,
	OPBINARY_OR,
	OPBINARY_AND,
	OPBINARY_BIC,
	OPBINARY_EOR,
	OPBINARY_ROR,
	OPBINARY_ROL,
	OPBINARY_EQ,
	OPBINARY_NE,
	OPBINARY_LT,
	OPBINARY_LE,
	OPBINARY_GT,
	OPBINARY_GE,
	OPBINARY_NUM
} OPBINARY;

typedef enum _OPTERNARY
{
	OPTERNARY_INVALID	= -1,
	OPTERNARY_SET,
	OPTERNARY_NUM
} OPTERNARY;

/*/
 *  Function prototypes  */
int proplocalmain (int argc, char ** argv);
void ShowWarning (os_error* sError);
#if !defined _DEBUG
inline void noprintf (char* szNull, ...);
#endif
inline void err (os_error* sError);

void PropMemReset (void);
void PropMemOutput (void);
inline void*  PropMemMalloc (size_t size);
inline void*  PropMemCalloc (size_t n, size_t size);
inline void PropMemFree (void* ptr);

Operation*	CreateMemory (void);
Operation*	CreateInteger (int nInteger);
Operation*	CreateTruthValue (bool boTruth);
Operation*	CreateVariable (char* szVar);
Operation*	CreateUnary (OPUNARY eOpType, Operation* psVar1);
Operation*	CreateBinary (OPBINARY eOpType, Operation* psVar1, Operation* psVar2);
Operation*	CreateTernary (OPTERNARY eOpType, Operation* psVar1, Operation* psVar2, Operation* psVar3);
char*  OperationString (Operation* psOp, char* szString, int nStrLen);
void FreeRecursive (Operation* psOp);
Operation*	CopyRecursive (Operation* psOp);
Operation*	FindOperation (Operation* psMain, Operation* psFind);
Operation*	SubstituteOperation (Operation* psMain, Operation* psFind, Operation* psSub);
Operation*	SubstituteOperationPair (Operation*	 psMain, Operation*	 psFind1, Operation*  psSub1, Operation*  psFind2,
									 Operation*	 psSub2);
Operation*	SimplifyOperation (Operation* psOp);
bool CompareOperations (Operation* psOp1, Operation* psOp2);
#endif /* if !defined _H_PROP */
