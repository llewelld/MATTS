/*$T MATTS/Prop.cpp GC 1.140 07/01/09 21:12:10 */
/*/
 *  Proposition ;
 *  Allow the construction of nested propositions ;
 *  The Flying Pig! ;
 *  Started 5/8/2003 Includes
 */
#include "Prop.h"

#if defined _RISC_OS
#include "oslib/os.h"
#include "oslib/osmodule.h"
#include "oslib/messagetrans.h"
#include "oslib/wimp.h"
#include "oslib/dragasprite.h"
#include "oslib/macros.h"
#include "oslib/osfile.h"
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#if defined _RISC_OS
#if defined _DEBUG
#include <kernel.h>
#endif
#endif

/*/
 =======================================================================================================================
 *  Defines
 =======================================================================================================================
 */
#define WORDALIGN(b)	(((b) + 3) &~3)
#define MAXVARSIZE		(64)
#if defined _DEBUG
#define DPRINTF printf
#else
#define DPRINTF noprintf
#endif
#if defined _DEBUG
#define REPORT		Report
#define REPORTVAR	ReportVar
void Report (char* szMessage);
void ReportVar (char* szFormat, int nVariable);
#else
#define REPORT
#define REPORTVAR
#endif
#if defined _RISC_OS
#define _snprintf	snprintf
#endif
#if defined _MEM_PROFILE
#define PropMalloc	PropMemMalloc
#define PropCalloc	PropMemCalloc
#define PropFree	PropMemFree
#else
#define PropMalloc	malloc
#define PropCalloc	calloc
#define PropFree	free
#endif /* if defined _MEM_PROFILE */

/* Structures */
typedef enum _OPTYPE
{
	OPTYPE_INVALID		= -1,
	OPTYPE_MEMORY,
	OPTYPE_INTEGER,
	OPTYPE_TRUTHVALUE,
	OPTYPE_VARIABLE,
	OPTYPE_UNARY,
	OPTYPE_BINARY,
	OPTYPE_TERNARY,
	OPTYPE_NUM
} OPTYPE;

typedef struct _OpUnary
{
	OPUNARY eOpType;
	Operation*	psVar1;
} OpUnary;

typedef struct _OpBinary
{
	OPBINARY eOpType;
	Operation*	psVar1;
	Operation*	psVar2;
} OpBinary;

typedef struct _OpTernary
{
	OPTERNARY eOpType;
	Operation*	psVar1;
	Operation*	psVar2;
	Operation*	psVar3;
} OpTernary;

typedef struct _OpVariable
{
	char szVar[MAXVARSIZE];
} OpVariable;

struct _Operation
{
	OPTYPE eOpType;
	union
	{
		bool boTruth;
		int nInteger;
		OpVariable*	 psVar;
		OpUnary*  psUnary;
		OpBinary*  psBinary;
		OpTernary*	psTernary;
	} Vars;
};

/*/
 *  Global variables Main application  */
char*  RecurseString (Operation* psOp, int nStrLen);
bool SubstituteRecursive (Operation* psMain, Operation* psFind, Operation* psSub);
int SubstituteRecursivePair (Operation*	 psMain, Operation*	 psFind1, Operation*  psSub1, Operation*  psFind2,
							 Operation*	 psSub2);
int CompareOperationsPair (Operation* psMain, Operation* psCompare1, Operation* psCompare2);
void LocalBell (void);

#if !defined _DEBUG

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
inline void noprintf (char* szNull, ...) {
	szNull = szNull;
}

#endif

/*/
 =======================================================================================================================
 *  Respond to swi errors
 =======================================================================================================================
 */
inline void err (os_error* sError) {
	if (sError) {
		ShowWarning (sError);
	}
}

/*/
 =======================================================================================================================
 *  Display an error
 =======================================================================================================================
 */
void ShowWarning (os_error* sError) {
	LocalBell ();
	printf (sError->errmess);
}

/*/
 =======================================================================================================================
 *  Make a beeping sound
 =======================================================================================================================
 */
void LocalBell (void)
{
#if defined _RISC_OS
	xos_bell ();
#elif defined _WINDOWS

	/* Nothing here yet */
#else
#error Undefined platform
#endif
}

#if defined _MEM_PROFILE
static int gnPropMemAllocated = 0;
#endif

/*/
 =======================================================================================================================
 *  Reset memory allocation details
 =======================================================================================================================
 */
void PropMemReset (void)
{
#if defined _MEM_PROFILE
	printf ("Memory profiling reset\n");
	gnPropMemAllocated = 0;
#else
	printf ("No memory profiling\n");
#endif
}

/*/
 =======================================================================================================================
 *  Output current memory allocation details
 =======================================================================================================================
 */
void PropMemOutput (void)
{
#if defined _MEM_PROFILE
	printf ("Total memory blocks allocated = %d\n", gnPropMemAllocated);
#endif
}

/*/
 =======================================================================================================================
 *  void * malloc (size_t size)
 =======================================================================================================================
 */
inline void* PropMemMalloc (size_t size)
{
#if defined _MEM_PROFILE
	gnPropMemAllocated += 1;
#endif
	return malloc (size);
}

/*/
 =======================================================================================================================
 *  void * calloc (size_t size)
 =======================================================================================================================
 */
inline void* PropMemCalloc (size_t n, size_t size)
{
#if defined _MEM_PROFILE
	gnPropMemAllocated += 1;
#endif
	return calloc (n, size);
}

/*/
 =======================================================================================================================
 *  void free (void * ptr)
 =======================================================================================================================
 */
inline void PropMemFree (void* ptr)
{
#if defined _MEM_PROFILE
	gnPropMemAllocated -= 1;
#endif
	return free (ptr);
}

/*/
 =======================================================================================================================
 *  Main program
 =======================================================================================================================
 */
int proplocalmain (int argc, char ** argv) {
	Operation*	psOp;
	Operation*	psOp2;
	char szString[512];
	Operation*	psSub;
	Operation*	psFind;
	Operation*	psFind2;

	argc = argc;
	argv = argv;

	printf ("Example structure\n");

	PropMemReset ();

	psOp = CreateBinary (OPBINARY_LOR, CreateBinary (OPBINARY_LAND, CreateVariable ("m1"), CreateVariable ("m2")),
						 CreateBinary (OPBINARY_IND, CreateMemory (), CreateInteger (57)));
	psOp2 = CopyRecursive (psOp);
	psFind = CreateBinary (OPBINARY_LAND, CreateVariable ("m1"), CreateVariable ("m2"));
	psFind2 = CreateVariable ("m2");
	psSub = CreateBinary (OPBINARY_SUB, CreateVariable ("m2"), CreateVariable ("m2"));
	psSub = SubstituteOperation (psSub, psFind2, psFind);

	OperationString (psOp2, szString, sizeof (szString));
	printf ("Copied is: %s\n", szString);

	OperationString (psOp, szString, sizeof (szString));
	printf ("Output is: %s\n", szString);

	psOp = SubstituteOperation (psOp, psFind, psSub);
	psOp = SubstituteOperation (psOp, psFind, psSub);

	OperationString (psFind, szString, sizeof (szString));
	printf ("To find is: %s\n", szString);

	OperationString (psSub, szString, sizeof (szString));
	printf ("To substitute is: %s\n", szString);

	OperationString (psOp, szString, sizeof (szString));
	printf ("Substituted twice is: %s\n", szString);

	FreeRecursive (psOp);
	FreeRecursive (psOp2);
	FreeRecursive (psFind);
	FreeRecursive (psFind2);
	FreeRecursive (psSub);

	PropMemOutput ();

	return 0;
}

/*/
 =======================================================================================================================
 *  Create a memory operation
 =======================================================================================================================
 */
Operation* CreateMemory (void) {
	Operation*	psOp;

	psOp = (Operation*) PropMalloc (sizeof (Operation));
	psOp->eOpType = OPTYPE_MEMORY;

	return psOp;
}

/*/
 =======================================================================================================================
 *  Create an integer operation
 =======================================================================================================================
 */
Operation* CreateInteger (int nInteger) {
	Operation*	psOp;

	psOp = (Operation*) PropMalloc (sizeof (Operation));
	psOp->eOpType = OPTYPE_INTEGER;
	psOp->Vars.nInteger = nInteger;

	return psOp;
}

/*/
 =======================================================================================================================
 *  Create true/false truth value operation
 =======================================================================================================================
 */
Operation* CreateTruthValue (bool boTruth) {
	Operation*	psOp;

	psOp = (Operation*) PropMalloc (sizeof (Operation));
	psOp->eOpType = OPTYPE_TRUTHVALUE;
	psOp->Vars.boTruth = boTruth;

	return psOp;
}

/*/
 =======================================================================================================================
 *  Create a variable
 =======================================================================================================================
 */
Operation* CreateVariable (char* szVar) {
	Operation*	psOp;

	psOp = (Operation*) PropMalloc (sizeof (Operation));
	psOp->eOpType = OPTYPE_VARIABLE;
	psOp->Vars.psVar = (OpVariable*) PropMalloc (sizeof (OpVariable));
	strncpy (psOp->Vars.psVar->szVar, szVar, MAXVARSIZE);

	return psOp;
}

/*/
 =======================================================================================================================
 *  Create a unary operation
 =======================================================================================================================
 */
Operation* CreateUnary (OPUNARY eOpType, Operation* psVar1) {
	Operation*	psOp;

	psOp = (Operation*) PropMalloc (sizeof (Operation));
	psOp->eOpType = OPTYPE_UNARY;
	psOp->Vars.psUnary = (OpUnary*) PropMalloc (sizeof (OpUnary));
	psOp->Vars.psUnary->eOpType = eOpType;
	psOp->Vars.psUnary->psVar1 = psVar1;

	return psOp;
}

/*/
 =======================================================================================================================
 *  Create a binary operation
 =======================================================================================================================
 */
Operation* CreateBinary (OPBINARY eOpType, Operation* psVar1, Operation* psVar2) {
	Operation*	psOp;

	psOp = (Operation*) PropMalloc (sizeof (Operation));
	psOp->eOpType = OPTYPE_BINARY;
	psOp->Vars.psBinary = (OpBinary*) PropMalloc (sizeof (OpBinary));
	psOp->Vars.psBinary->eOpType = eOpType;
	psOp->Vars.psBinary->psVar1 = psVar1;
	psOp->Vars.psBinary->psVar2 = psVar2;

	return psOp;
}

/*/
 =======================================================================================================================
 *  Create a ternary operation
 =======================================================================================================================
 */
Operation* CreateTernary (OPTERNARY eOpType, Operation* psVar1, Operation* psVar2, Operation* psVar3) {
	Operation*	psOp;

	psOp = (Operation*) PropMalloc (sizeof (Operation));
	psOp->eOpType = OPTYPE_TERNARY;
	psOp->Vars.psTernary = (OpTernary*) PropMalloc (sizeof (OpTernary));
	psOp->Vars.psTernary->eOpType = eOpType;
	psOp->Vars.psTernary->psVar1 = psVar1;
	psOp->Vars.psTernary->psVar2 = psVar2;
	psOp->Vars.psTernary->psVar3 = psVar3;

	return psOp;
}

/*/
 =======================================================================================================================
 *  Recursively free up all of the memory used by a formula and its sub formulas
 =======================================================================================================================
 */
void FreeRecursive (Operation* psOp) {
	if (psOp) {
		switch (psOp->eOpType) {
		case OPTYPE_MEMORY:
		case OPTYPE_INTEGER:
		case OPTYPE_TRUTHVALUE:

			/* Nothing else to free - backtrack */
			break;
		case OPTYPE_VARIABLE:

			/* Just free up the variable string, then backtrack */
			PropFree (psOp->Vars.psVar);
			break;
		case OPTYPE_UNARY:

			/* Free up any operations further down the tree */
			if (psOp->Vars.psUnary) {
				FreeRecursive (psOp->Vars.psUnary->psVar1);
				PropFree (psOp->Vars.psUnary);
			}

			/* Then backtrack */
			break;
		case OPTYPE_BINARY:

			/* Free up any operations further down the tree */
			if (psOp->Vars.psBinary) {
				FreeRecursive (psOp->Vars.psBinary->psVar1);
				FreeRecursive (psOp->Vars.psBinary->psVar2);
				PropFree (psOp->Vars.psBinary);
			}

			/* Then backtrack */
			break;
		case OPTYPE_TERNARY:

			/* Free up any operations further down the tree */
			if (psOp->Vars.psTernary) {
				FreeRecursive (psOp->Vars.psTernary->psVar1);
				FreeRecursive (psOp->Vars.psTernary->psVar2);
				FreeRecursive (psOp->Vars.psTernary->psVar3);
				PropFree (psOp->Vars.psTernary);
			}

			/* Then backtrack */
			break;
		default:
			printf ("Invalid operation type\n");
			break;
		}

		PropFree (psOp);
	}
}

/*/
 =======================================================================================================================
 *  Turn a formula into a string
 =======================================================================================================================
 */
char* OperationString (Operation* psOp, char* szString, int nStrLen) {
	char*  szRecurse;

	szRecurse = RecurseString (psOp, nStrLen);
	strncpy (szString, szRecurse, nStrLen);
	szString[nStrLen - 1] = 0;
	PropFree (szRecurse);

	return szString;
}

/*/
 =======================================================================================================================
 *  Recursively turn a formula into a string
 =======================================================================================================================
 */
char* RecurseString (Operation* psOp, int nStrLen) {
	char*  szReturn;
	char*  szVar1;
	char*  szVar2;
	char*  szVar3;

	szReturn = (char*) PropCalloc (nStrLen, 1);

	if (psOp) {
		switch (psOp->eOpType) {
		case OPTYPE_MEMORY:
			_snprintf (szReturn, nStrLen, "M");
			szReturn[nStrLen - 1] = 0;
			break;
		case OPTYPE_INTEGER:
			_snprintf (szReturn, nStrLen, "%d", psOp->Vars.nInteger);
			szReturn[nStrLen - 1] = 0;
			break;
		case OPTYPE_TRUTHVALUE:
			if (psOp->Vars.boTruth) {
				_snprintf (szReturn, nStrLen, "TRUE");
				szReturn[nStrLen - 1] = 0;
			} else {
				_snprintf (szReturn, nStrLen, "FALSE");
				szReturn[nStrLen - 1] = 0;
			}

			break;
		case OPTYPE_VARIABLE:
			_snprintf (szReturn, nStrLen, "%s", psOp->Vars.psVar->szVar);
			szReturn[nStrLen - 1] = 0;
			break;
		case OPTYPE_UNARY:
			szVar1 = RecurseString (psOp->Vars.psUnary->psVar1, nStrLen);
			switch (psOp->Vars.psUnary->eOpType) {
			case OPUNARY_NOT:
				_snprintf (szReturn, nStrLen, "~%s", szVar1);
				szReturn[nStrLen - 1] = 0;
				break;
			case OPUNARY_VC:
				_snprintf (szReturn, nStrLen, "VC(%s)", szVar1);
				szReturn[nStrLen - 1] = 0;
				break;
			default:
				printf ("Invalid unary operator\n");
				break;
			}

			PropFree (szVar1);
			break;
		case OPTYPE_BINARY:
			szVar1 = RecurseString (psOp->Vars.psBinary->psVar1, nStrLen);
			szVar2 = RecurseString (psOp->Vars.psBinary->psVar2, nStrLen);
			switch (psOp->Vars.psBinary->eOpType) {
			case OPBINARY_IND:
				_snprintf (szReturn, nStrLen, "ind(%s, %s)", szVar1, szVar2);
				szReturn[nStrLen - 1] = 0;
				break;
			case OPBINARY_LAND:
				_snprintf (szReturn, nStrLen, "(%s ^ %s)", szVar1, szVar2);
				szReturn[nStrLen - 1] = 0;
				break;
			case OPBINARY_LOR:
				_snprintf (szReturn, nStrLen, "(%s v %s)", szVar1, szVar2);
				szReturn[nStrLen - 1] = 0;
				break;
			case OPBINARY_LIMP:
				_snprintf (szReturn, nStrLen, "(%s -> %s)", szVar1, szVar2);
				szReturn[nStrLen - 1] = 0;
				break;
			case OPBINARY_ADD:
				_snprintf (szReturn, nStrLen, "(%s + %s)", szVar1, szVar2);
				szReturn[nStrLen - 1] = 0;
				break;
			case OPBINARY_SUB:
				_snprintf (szReturn, nStrLen, "(%s - %s)", szVar1, szVar2);
				szReturn[nStrLen - 1] = 0;
				break;
			case OPBINARY_MUL:
				_snprintf (szReturn, nStrLen, "(%s * %s)", szVar1, szVar2);
				szReturn[nStrLen - 1] = 0;
				break;
			case OPBINARY_DIV:
				_snprintf (szReturn, nStrLen, "(%s / %s)", szVar1, szVar2);
				szReturn[nStrLen - 1] = 0;
				break;
			case OPBINARY_MOD:
				_snprintf (szReturn, nStrLen, "(%s MOD %s)", szVar1, szVar2);
				szReturn[nStrLen - 1] = 0;
				break;
			case OPBINARY_OR:
				_snprintf (szReturn, nStrLen, "(%s OR %s)", szVar1, szVar2);
				szReturn[nStrLen - 1] = 0;
				break;
			case OPBINARY_AND:
				_snprintf (szReturn, nStrLen, "(%s AND %s)", szVar1, szVar2);
				szReturn[nStrLen - 1] = 0;
				break;
			case OPBINARY_BIC:
				_snprintf (szReturn, nStrLen, "(%s BIC %s)", szVar1, szVar2);
				szReturn[nStrLen - 1] = 0;
				break;
			case OPBINARY_EOR:
				_snprintf (szReturn, nStrLen, "(%s EOR %s)", szVar1, szVar2);
				szReturn[nStrLen - 1] = 0;
				break;
			case OPBINARY_ROR:
				_snprintf (szReturn, nStrLen, "(%s >> %s)", szVar1, szVar2);
				szReturn[nStrLen - 1] = 0;
				break;
			case OPBINARY_ROL:
				_snprintf (szReturn, nStrLen, "(%s << %s)", szVar1, szVar2);
				szReturn[nStrLen - 1] = 0;
				break;
			case OPBINARY_EQ:
				_snprintf (szReturn, nStrLen, "(%s = %s)", szVar1, szVar2);
				szReturn[nStrLen - 1] = 0;
				break;
			case OPBINARY_NE:
				_snprintf (szReturn, nStrLen, "(%s != %s)", szVar1, szVar2);
				szReturn[nStrLen - 1] = 0;
				break;
			case OPBINARY_LT:
				_snprintf (szReturn, nStrLen, "(%s < %s)", szVar1, szVar2);
				szReturn[nStrLen - 1] = 0;
				break;
			case OPBINARY_LE:
				_snprintf (szReturn, nStrLen, "(%s <= %s)", szVar1, szVar2);
				szReturn[nStrLen - 1] = 0;
				break;
			case OPBINARY_GT:
				_snprintf (szReturn, nStrLen, "(%s > %s)", szVar1, szVar2);
				szReturn[nStrLen - 1] = 0;
				break;
			case OPBINARY_GE:
				_snprintf (szReturn, nStrLen, "(%s >= %s)", szVar1, szVar2);
				szReturn[nStrLen - 1] = 0;
				break;
			default:
				printf ("Invalid binary operator\n");
				break;
			}

			PropFree (szVar1);
			PropFree (szVar2);
			break;
		case OPTYPE_TERNARY:
			szVar1 = RecurseString (psOp->Vars.psTernary->psVar1, nStrLen);
			szVar2 = RecurseString (psOp->Vars.psTernary->psVar2, nStrLen);
			szVar3 = RecurseString (psOp->Vars.psTernary->psVar3, nStrLen);
			switch (psOp->Vars.psTernary->eOpType) {
			case OPTERNARY_SET:
				_snprintf (szReturn, nStrLen, "set(%s, %s, %s)", szVar1, szVar2, szVar3);
				szReturn[nStrLen - 1] = 0;
				break;
			default:
				printf ("Invalid binary operator\n");
				break;
			}

			PropFree (szVar1);
			PropFree (szVar2);
			PropFree (szVar3);
			break;
		default:
			printf ("Invalid operation type\n");
			break;
		}
	} else {
		strncpy (szReturn, "", nStrLen);
	}

	return szReturn;
}

/*/
 =======================================================================================================================
 *  Recursively copy a formula and all its subformulas
 =======================================================================================================================
 */
Operation* CopyRecursive (Operation* psOp) {
	Operation*	psReturn;
	psReturn = NULL;
	if (psOp) {
		switch (psOp->eOpType) {
		case OPTYPE_MEMORY:
			psReturn = CreateMemory ();
			break;
		case OPTYPE_INTEGER:
			psReturn = CreateInteger (psOp->Vars.nInteger);
			break;
		case OPTYPE_TRUTHVALUE:
			psReturn = CreateTruthValue (psOp->Vars.boTruth);
			break;
		case OPTYPE_VARIABLE:
			psReturn = CreateVariable (psOp->Vars.psVar->szVar);
			break;
		case OPTYPE_UNARY:
			psReturn = CreateUnary (psOp->Vars.psUnary->eOpType, CopyRecursive (psOp->Vars.psUnary->psVar1));
			break;
		case OPTYPE_BINARY:
			psReturn = CreateBinary (psOp->Vars.psBinary->eOpType, CopyRecursive (psOp->Vars.psBinary->psVar1),
									 CopyRecursive (psOp->Vars.psBinary->psVar2));
			break;
		case OPTYPE_TERNARY:
			psReturn = CreateTernary (psOp->Vars.psTernary->eOpType, CopyRecursive (psOp->Vars.psTernary->psVar1),
									  CopyRecursive (psOp->Vars.psTernary->psVar2),
									  CopyRecursive (psOp->Vars.psTernary->psVar3));
			break;
		default:
			printf ("Invalid operation type\n");
			break;
		}
	}

	return psReturn;
}

/*/
 =======================================================================================================================
 *  Compare two formulae recursively
 =======================================================================================================================
 */
bool CompareOperations (Operation* psOp1, Operation* psOp2) {
	bool boReturn;

	boReturn = TRUE;

	if ((psOp1) && (psOp2)) {
		if (psOp1->eOpType == psOp2->eOpType) {
			switch (psOp1->eOpType) {
			case OPTYPE_MEMORY:

				/* Return TRUE */
				break;
			case OPTYPE_INTEGER:
				if (psOp1->Vars.nInteger != psOp2->Vars.nInteger) {
					boReturn = FALSE;
				}

				break;
			case OPTYPE_TRUTHVALUE:
				if (psOp1->Vars.boTruth != psOp2->Vars.boTruth) {
					boReturn = FALSE;
				}

				break;
			case OPTYPE_VARIABLE:
				if (strcmp (psOp1->Vars.psVar->szVar, psOp2->Vars.psVar->szVar) != 0) {
					boReturn = FALSE;
				}

				break;
			case OPTYPE_UNARY:
				if (psOp1->Vars.psUnary->eOpType != psOp2->Vars.psUnary->eOpType) {
					boReturn = FALSE;
				} else {
					boReturn = CompareOperations (psOp1->Vars.psUnary->psVar1, psOp2->Vars.psUnary->psVar1);
				}

				break;
			case OPTYPE_BINARY:
				if (psOp1->Vars.psBinary->eOpType != psOp2->Vars.psBinary->eOpType) {
					boReturn = FALSE;
				} else {
					boReturn =
						(
							CompareOperations (psOp1->Vars.psBinary->psVar1, psOp2->Vars.psBinary->psVar1)
						&&	CompareOperations (psOp1->Vars.psBinary->psVar2, psOp2->Vars.psBinary->psVar2)
						);
				}

				break;
			case OPTYPE_TERNARY:
				if (psOp1->Vars.psTernary->eOpType != psOp2->Vars.psTernary->eOpType) {
					boReturn = FALSE;
				} else {
					boReturn =
						(
							CompareOperations (psOp1->Vars.psTernary->psVar1, psOp2->Vars.psTernary->psVar1)
						&&	CompareOperations (psOp1->Vars.psTernary->psVar2, psOp2->Vars.psTernary->psVar2)
						&&	CompareOperations (psOp1->Vars.psTernary->psVar3, psOp2->Vars.psTernary->psVar3)
						);
				}

				break;
			default:
				printf ("Invalid operation type\n");
				break;
			}
		} else {
			boReturn = FALSE;
		}
	} else {
		boReturn = FALSE;
	}

	return boReturn;
}

/*/
 =======================================================================================================================
 *  Search a formula for a given subformula
 =======================================================================================================================
 */
Operation* FindOperation (Operation* psMain, Operation* psFind) {
	Operation*	psReturn;
	bool boSame;

	psReturn = NULL;

	if ((psMain) && (psFind)) {
		switch (psMain->eOpType) {
		case OPTYPE_MEMORY:
		case OPTYPE_VARIABLE:
		case OPTYPE_TRUTHVALUE:
		case OPTYPE_INTEGER:
			boSame = CompareOperations (psMain, psFind);
			if (boSame) {
				psReturn = psMain;
			}

			break;
		case OPTYPE_UNARY:
			psReturn = FindOperation (psMain->Vars.psUnary->psVar1, psFind);
			if (!psReturn) {
				boSame = CompareOperations (psMain, psFind);
				if (boSame) {
					psReturn = psMain;
				}
			}

			break;
		case OPTYPE_BINARY:
			psReturn = FindOperation (psMain->Vars.psBinary->psVar1, psFind);
			if (!psReturn) {
				psReturn = FindOperation (psMain->Vars.psBinary->psVar2, psFind);
				if (!psReturn) {
					boSame = CompareOperations (psMain, psFind);
					if (boSame) {
						psReturn = psMain;
					}
				}
			}

			break;
		case OPTYPE_TERNARY:
			psReturn = FindOperation (psMain->Vars.psTernary->psVar1, psFind);
			if (!psReturn) {
				psReturn = FindOperation (psMain->Vars.psTernary->psVar2, psFind);
				if (!psReturn) {
					psReturn = FindOperation (psMain->Vars.psTernary->psVar3, psFind);
					if (!psReturn) {
						boSame = CompareOperations (psMain, psFind);
						if (boSame) {
							psReturn = psMain;
						}
					}
				}
			}

			break;
		default:
			printf ("Invalid operation type\n");
			break;
		}
	}

	return psReturn;
}

/*/
 =======================================================================================================================
 *  Substitute all instances of a given subformula for a formula
 =======================================================================================================================
 */
Operation* SubstituteOperation (Operation* psMain, Operation* psFind, Operation* psSub) {
	bool boFind;
	Operation*	psReturn;

	boFind = SubstituteRecursive (psMain, psFind, psSub);
	if (boFind) {
		FreeRecursive (psMain);
		psReturn = CopyRecursive (psSub);
	} else {
		psReturn = psMain;
	}

	return psReturn;
}

/*/
 =======================================================================================================================
 *  Substitute recursively all instances of a given subformula for a formula
 =======================================================================================================================
 */
bool SubstituteRecursive (Operation* psMain, Operation* psFind, Operation* psSub) {
	bool boSubstitute;
	bool boFind;

	boSubstitute = FALSE;

	if ((psMain) && (psSub)) {
		switch (psMain->eOpType) {
		case OPTYPE_MEMORY:
		case OPTYPE_VARIABLE:
		case OPTYPE_TRUTHVALUE:
		case OPTYPE_INTEGER:
			boSubstitute = CompareOperations (psMain, psFind);
			break;
		case OPTYPE_UNARY:
			boSubstitute = CompareOperations (psMain, psFind);
			if (!boSubstitute) {
				boFind = SubstituteRecursive (psMain->Vars.psUnary->psVar1, psFind, psSub);
				if (boFind) {
					FreeRecursive (psMain->Vars.psUnary->psVar1);
					psMain->Vars.psUnary->psVar1 = CopyRecursive (psSub);
				}
			}

			break;
		case OPTYPE_BINARY:
			boSubstitute = CompareOperations (psMain, psFind);
			if (!boSubstitute) {
				boFind = SubstituteRecursive (psMain->Vars.psBinary->psVar1, psFind, psSub);
				if (boFind) {
					FreeRecursive (psMain->Vars.psBinary->psVar1);
					psMain->Vars.psBinary->psVar1 = CopyRecursive (psSub);
				}

				boFind = SubstituteRecursive (psMain->Vars.psBinary->psVar2, psFind, psSub);
				if (boFind) {
					FreeRecursive (psMain->Vars.psBinary->psVar2);
					psMain->Vars.psBinary->psVar2 = CopyRecursive (psSub);
				}
			}

			break;
		case OPTYPE_TERNARY:
			boSubstitute = CompareOperations (psMain, psFind);
			if (!boSubstitute) {
				boFind = SubstituteRecursive (psMain->Vars.psTernary->psVar1, psFind, psSub);
				if (boFind) {
					FreeRecursive (psMain->Vars.psTernary->psVar1);
					psMain->Vars.psTernary->psVar1 = CopyRecursive (psSub);
				}

				boFind = SubstituteRecursive (psMain->Vars.psTernary->psVar2, psFind, psSub);
				if (boFind) {
					FreeRecursive (psMain->Vars.psTernary->psVar2);
					psMain->Vars.psTernary->psVar2 = CopyRecursive (psSub);
				}

				boFind = SubstituteRecursive (psMain->Vars.psTernary->psVar2, psFind, psSub);
				if (boFind) {
					FreeRecursive (psMain->Vars.psTernary->psVar2);
					psMain->Vars.psTernary->psVar2 = CopyRecursive (psSub);
				}
			}

			break;
		default:
			printf ("Invalid operation type\n");
			break;
		}
	}

	return boSubstitute;
}

/*/
 =======================================================================================================================
 *  Recursively simplify an operation if possible
 =======================================================================================================================
 */
Operation* SimplifyOperation (Operation* psOp) {
	Operation*	psReturn;
	OpUnary*  psUna;
	OpBinary*  psBin;
	OpTernary*	psTer;
	Operation*	psTemp;

	psReturn = NULL;
	psUna = NULL;
	psBin = NULL;
	psTer = NULL;
	psTemp = NULL;

	if (psOp) {
		switch (psOp->eOpType) {
		case OPTYPE_MEMORY:
			psReturn = psOp;
			break;
		case OPTYPE_INTEGER:
			psReturn = psOp;
			break;
		case OPTYPE_TRUTHVALUE:
			psReturn = psOp;
			break;
		case OPTYPE_VARIABLE:
			psReturn = psOp;
			break;
		case OPTYPE_UNARY:
			psReturn = psOp;
			psUna = psReturn->Vars.psUnary;
			psUna->psVar1 = SimplifyOperation (psUna->psVar1);
			switch (psUna->eOpType) {
			case OPUNARY_NOT:
				switch (psUna->psVar1->eOpType) {
				case OPTYPE_UNARY:
					if (psUna->psVar1->Vars.psUnary->eOpType == OPUNARY_NOT) {

						/* Remove pairs of 'nots' */
						psReturn = CopyRecursive (psUna->psVar1->Vars.psUnary->psVar1);
						FreeRecursive (psOp);
					}

					break;
				case OPTYPE_TRUTHVALUE:

					/* Negate the truth value */
					psReturn = CreateTruthValue (!psUna->psVar1->Vars.boTruth);
					FreeRecursive (psOp);
					break;
				case OPTYPE_BINARY:
					psBin = psUna->psVar1->Vars.psBinary;
					switch (psBin->eOpType) {
					case OPBINARY_EQ:

						/* Change to NE */
						psReturn = CreateBinary (OPBINARY_NE, psBin->psVar1, psBin->psVar2);
						FreeRecursive (psOp);
						break;
					case OPBINARY_NE:

						/* Change to EQ */
						psReturn = CreateBinary (OPBINARY_EQ, psBin->psVar1, psBin->psVar2);
						FreeRecursive (psOp);
						break;
					case OPBINARY_LT:

						/* Change to GE */
						psReturn = CreateBinary (OPBINARY_GE, psBin->psVar1, psBin->psVar2);
						FreeRecursive (psOp);
						break;
					case OPBINARY_LE:

						/* Change to GT */
						psReturn = CreateBinary (OPBINARY_GT, psBin->psVar1, psBin->psVar2);
						FreeRecursive (psOp);
						break;
					case OPBINARY_GT:

						/* Change to LE */
						psReturn = CreateBinary (OPBINARY_LE, psBin->psVar1, psBin->psVar2);
						FreeRecursive (psOp);
						break;
					case OPBINARY_GE:

						/* Change to LT */
						psReturn = CreateBinary (OPBINARY_LT, psBin->psVar1, psBin->psVar2);
						FreeRecursive (psOp);
						break;
					default:

						/* Do nothing */
						break;
					}

					break;
				default:

					/* Do nothing */
					break;
				}

				break;
			default:

				/* Do nothing */
				break;
			}

			break;
		case OPTYPE_BINARY:
			psBin = psOp->Vars.psBinary;
			psBin->psVar1 = SimplifyOperation (psBin->psVar1);
			psBin->psVar2 = SimplifyOperation (psBin->psVar2);
			psReturn = psOp;
			switch (psBin->eOpType) {
			case OPBINARY_LAND:
				if (((psBin->psVar1->eOpType == OPTYPE_TRUTHVALUE) && (!psBin->psVar1->Vars.boTruth))
				|| ((psBin->psVar2->eOpType == OPTYPE_TRUTHVALUE) && (!psBin->psVar2->Vars.boTruth))) {
					psReturn = CreateTruthValue (FALSE);
					FreeRecursive (psOp);
				} else {
					if (((psBin->psVar1->eOpType == OPTYPE_TRUTHVALUE) && (psBin->psVar1->Vars.boTruth))
					|| (CompareOperations (psBin->psVar1, psBin->psVar2))) {
						psReturn = CopyRecursive (psBin->psVar2);
						FreeRecursive (psOp);
					} else {
						if ((psBin->psVar2->eOpType == OPTYPE_TRUTHVALUE) && (psBin->psVar2->Vars.boTruth)) {
							psReturn = CopyRecursive (psBin->psVar1);
							FreeRecursive (psOp);
						}
					}
				}

				break;
			case OPBINARY_LOR:
				if (((psBin->psVar1->eOpType == OPTYPE_TRUTHVALUE) && (psBin->psVar1->Vars.boTruth))
				|| ((psBin->psVar2->eOpType == OPTYPE_TRUTHVALUE) && (psBin->psVar2->Vars.boTruth))) {
					psReturn = CreateTruthValue (TRUE);
					FreeRecursive (psOp);
				} else {
					if ((psBin->psVar1->eOpType == OPTYPE_TRUTHVALUE) && (!psBin->psVar1->Vars.boTruth)
					&& (psBin->psVar2->eOpType == OPTYPE_TRUTHVALUE) && (!psBin->psVar2->Vars.boTruth)) {
						psReturn = CreateTruthValue (FALSE);
						FreeRecursive (psOp);
					}
				}

				break;
			case OPBINARY_LIMP:
				if ((((psBin->psVar1->eOpType == OPTYPE_TRUTHVALUE) && (!psBin->psVar1->Vars.boTruth))
				|| ((psBin->psVar2->eOpType == OPTYPE_TRUTHVALUE) && (psBin->psVar2->Vars.boTruth)))
				|| CompareOperations (psBin->psVar1, psBin->psVar2)) {
					psReturn = CreateTruthValue (TRUE);
					FreeRecursive (psOp);
				} else {
					if ((psBin->psVar1->eOpType == OPTYPE_TRUTHVALUE) && (psBin->psVar1->Vars.boTruth)) {
						psReturn = CopyRecursive (psBin->psVar2);
						FreeRecursive (psOp);
					}
				}

				break;
			case OPBINARY_ADD:
				if ((psBin->psVar1->eOpType == OPTYPE_INTEGER) && (psBin->psVar2->eOpType == OPTYPE_INTEGER)) {
					psReturn = CreateInteger (psBin->psVar1->Vars.nInteger + psBin->psVar2->Vars.nInteger);
					FreeRecursive (psOp);
				} else {
					if ((psBin->psVar1->eOpType == OPTYPE_BINARY) && (psBin->psVar2->eOpType == OPTYPE_INTEGER)
					&& (psBin->psVar1->Vars.psBinary->eOpType == OPBINARY_ADD)
					&& (psBin->psVar1->Vars.psBinary->psVar2->eOpType == OPTYPE_INTEGER)) {
						psReturn = CreateBinary (OPBINARY_ADD, CopyRecursive (psBin->psVar1->Vars.psBinary->psVar1),
												 CreateInteger (psBin->psVar1->Vars.psBinary->psVar2->Vars.nInteger +
												 psBin->psVar2->Vars.nInteger));
						FreeRecursive (psOp);
					} else {
						if ((psBin->psVar1->eOpType == OPTYPE_BINARY) && (psBin->psVar2->eOpType == OPTYPE_INTEGER)
						&& (psBin->psVar1->Vars.psBinary->eOpType == OPBINARY_SUB)
						&& (psBin->psVar1->Vars.psBinary->psVar2->eOpType == OPTYPE_INTEGER)) {
							if (psBin->psVar1->Vars.psBinary->psVar2->Vars.nInteger < psBin->psVar2->Vars.nInteger) {
								psReturn = CreateBinary (OPBINARY_ADD,
														 CopyRecursive (psBin->psVar1->Vars.psBinary->psVar1),
														 CreateInteger (psBin->psVar2->Vars.nInteger -
														 psBin->psVar1->Vars.psBinary->psVar2->Vars.nInteger));
							} else {
								psReturn = CreateBinary (OPBINARY_SUB,
														 CopyRecursive (psBin->psVar1->Vars.psBinary->psVar1),
														 CreateInteger (
															 psBin->psVar1->Vars.psBinary->psVar2->Vars.nInteger -
														 psBin->psVar2->Vars.nInteger));
							}

							FreeRecursive (psOp);
						}
					}
				}

				break;
			case OPBINARY_SUB:
				if ((psBin->psVar1->eOpType == OPTYPE_INTEGER) && (psBin->psVar2->eOpType == OPTYPE_INTEGER)) {
					psReturn = CreateInteger (psBin->psVar1->Vars.nInteger - psBin->psVar2->Vars.nInteger);
					FreeRecursive (psOp);
				}

				break;
			case OPBINARY_MUL:
				if ((psBin->psVar1->eOpType == OPTYPE_INTEGER) && (psBin->psVar2->eOpType == OPTYPE_INTEGER)) {
					psReturn = CreateInteger (psBin->psVar1->Vars.nInteger * psBin->psVar2->Vars.nInteger);
					FreeRecursive (psOp);
				}

				break;
			case OPBINARY_DIV:
				if ((psBin->psVar1->eOpType == OPTYPE_INTEGER) && (psBin->psVar2->eOpType == OPTYPE_INTEGER)) {
					psReturn = CreateInteger (psBin->psVar1->Vars.nInteger / psBin->psVar2->Vars.nInteger);
					FreeRecursive (psOp);
				}

				break;
			case OPBINARY_MOD:
				if ((psBin->psVar1->eOpType == OPTYPE_INTEGER) && (psBin->psVar2->eOpType == OPTYPE_INTEGER)) {
					psReturn = CreateInteger (psBin->psVar1->Vars.nInteger % psBin->psVar2->Vars.nInteger);
					FreeRecursive (psOp);
				}

				break;
			case OPBINARY_OR:
				if ((psBin->psVar1->eOpType == OPTYPE_INTEGER) && (psBin->psVar2->eOpType == OPTYPE_INTEGER)) {
					psReturn = CreateInteger (psBin->psVar1->Vars.nInteger | psBin->psVar2->Vars.nInteger);
					FreeRecursive (psOp);
				}

				break;
			case OPBINARY_AND:
				if ((psBin->psVar1->eOpType == OPTYPE_INTEGER) && (psBin->psVar2->eOpType == OPTYPE_INTEGER)) {
					psReturn = CreateInteger (psBin->psVar1->Vars.nInteger & psBin->psVar2->Vars.nInteger);
					FreeRecursive (psOp);
				}

				break;
			case OPBINARY_BIC:
				if ((psBin->psVar1->eOpType == OPTYPE_INTEGER) && (psBin->psVar2->eOpType == OPTYPE_INTEGER)) {
					psReturn = CreateInteger (psBin->psVar1->Vars.nInteger &!psBin->psVar2->Vars.nInteger);
					FreeRecursive (psOp);
				}

				break;
			case OPBINARY_EOR:
				if ((psBin->psVar1->eOpType == OPTYPE_INTEGER) && (psBin->psVar2->eOpType == OPTYPE_INTEGER)) {
					psReturn = CreateInteger (psBin->psVar1->Vars.nInteger ^ psBin->psVar2->Vars.nInteger);
					FreeRecursive (psOp);
				}

				break;
			case OPBINARY_ROR:
				if ((psBin->psVar1->eOpType == OPTYPE_INTEGER) && (psBin->psVar2->eOpType == OPTYPE_INTEGER)) {
					psReturn = CreateInteger (psBin->psVar1->Vars.nInteger >> psBin->psVar2->Vars.nInteger);
					FreeRecursive (psOp);
				}

				break;
			case OPBINARY_ROL:
				if ((psBin->psVar1->eOpType == OPTYPE_INTEGER) && (psBin->psVar2->eOpType == OPTYPE_INTEGER)) {
					psReturn = CreateInteger (psBin->psVar1->Vars.nInteger << psBin->psVar2->Vars.nInteger);
					FreeRecursive (psOp);
				}

				break;
			case OPBINARY_EQ:
				if ((psBin->psVar1->eOpType == OPTYPE_INTEGER) && (psBin->psVar2->eOpType == OPTYPE_INTEGER)) {
					if (psBin->psVar1->Vars.nInteger == psBin->psVar2->Vars.nInteger) {
						psReturn = CreateTruthValue (TRUE);
						FreeRecursive (psOp);
					} else {
						psReturn = CreateTruthValue (FALSE);
						FreeRecursive (psOp);
					}
				} else {
					if (CompareOperations (psBin->psVar1, psBin->psVar2)) {
						psReturn = CreateTruthValue (TRUE);
						FreeRecursive (psOp);
					}
				}

				break;
			case OPBINARY_NE:
				if ((psBin->psVar1->eOpType == OPTYPE_INTEGER) && (psBin->psVar2->eOpType == OPTYPE_INTEGER)) {
					if (psBin->psVar1->Vars.nInteger != psBin->psVar2->Vars.nInteger) {
						psReturn = CreateTruthValue (TRUE);
						FreeRecursive (psOp);
					} else {
						psReturn = CreateTruthValue (FALSE);
						FreeRecursive (psOp);
					}
				} else {
					if (CompareOperations (psBin->psVar1, psBin->psVar2)) {
						psReturn = CreateTruthValue (FALSE);
						FreeRecursive (psOp);
					}
				}

				break;
			case OPBINARY_LT:
				if ((psBin->psVar1->eOpType == OPTYPE_INTEGER) && (psBin->psVar2->eOpType == OPTYPE_INTEGER)) {
					if (psBin->psVar1->Vars.nInteger < psBin->psVar2->Vars.nInteger) {
						psReturn = CreateTruthValue (TRUE);
						FreeRecursive (psOp);
					} else {
						psReturn = CreateTruthValue (FALSE);
						FreeRecursive (psOp);
					}
				}

				break;
			case OPBINARY_LE:
				if ((psBin->psVar1->eOpType == OPTYPE_INTEGER) && (psBin->psVar2->eOpType == OPTYPE_INTEGER)) {
					if (psBin->psVar1->Vars.nInteger <= psBin->psVar2->Vars.nInteger) {
						psReturn = CreateTruthValue (TRUE);
						FreeRecursive (psOp);
					} else {
						psReturn = CreateTruthValue (FALSE);
						FreeRecursive (psOp);
					}
				}

				break;
			case OPBINARY_GT:
				if ((psBin->psVar1->eOpType == OPTYPE_INTEGER) && (psBin->psVar2->eOpType == OPTYPE_INTEGER)) {
					if (psBin->psVar1->Vars.nInteger > psBin->psVar2->Vars.nInteger) {
						psReturn = CreateTruthValue (TRUE);
						FreeRecursive (psOp);
					} else {
						psReturn = CreateTruthValue (FALSE);
						FreeRecursive (psOp);
					}
				}

				break;
			case OPBINARY_GE:
				if ((psBin->psVar1->eOpType == OPTYPE_INTEGER) && (psBin->psVar2->eOpType == OPTYPE_INTEGER)) {
					if (psBin->psVar1->Vars.nInteger >= psBin->psVar2->Vars.nInteger) {
						psReturn = CreateTruthValue (TRUE);
						FreeRecursive (psOp);
					} else {
						psReturn = CreateTruthValue (FALSE);
						FreeRecursive (psOp);
					}
				}

				break;
			case OPBINARY_IND:

				/* Search for a 'set' reference in the memory */
				psTemp = psBin->psVar1;
				while
				(
					(psTemp->eOpType == OPTYPE_TERNARY)
				&&	(psTemp->Vars.psTernary->eOpType == OPTERNARY_SET)
				&&	(!CompareOperations (psBin->psVar2, psTemp->Vars.psTernary->psVar2))
				&&	(
						(psTemp->Vars.psTernary->psVar2->eOpType != OPTYPE_BINARY)
					||	(
							(psTemp->Vars.psTernary->psVar2->eOpType == OPTYPE_BINARY)
						&&	(psTemp->Vars.psTernary->psVar2->Vars.psBinary->eOpType != OPBINARY_IND)
						)
					)
				) {
					psTemp = psTemp->Vars.psTernary->psVar1;
				}

				if ((psTemp->eOpType == OPTYPE_TERNARY) && (psTemp->Vars.psTernary->eOpType == OPTERNARY_SET)
				&& (CompareOperations (psBin->psVar2, psTemp->Vars.psTernary->psVar2)) && (
						(psTemp->Vars.psTernary->psVar2->eOpType != OPTYPE_BINARY) || ((psTemp->Vars.psTernary->psVar2->eOpType == OPTYPE_BINARY)
				&& (psTemp->Vars.psTernary->psVar2->Vars.psBinary->eOpType != OPBINARY_IND)))) {
					psReturn = CopyRecursive (psTemp->Vars.psTernary->psVar3);
					FreeRecursive (psOp);
				}

				/*
				 * psTemp = psBin->psVar1;
				 * while ((psTemp->eOpType == OPTYPE_TERNARY) ;
				 * && (psTemp->Vars.psTernary->eOpType == OPTERNARY_SET) ;
				 * && (psTemp->Vars.psTernary->psVar2->eOpType == OPTYPE_BINARY) ;
				 * && (psTemp->Vars.psTernary->psVar2->Vars.psBinary->eOpType ;
				 * == OPBINARY_IND) ;
				 * && (!CompareOperations (psBin->psVar2, ;
				 * psTemp->Vars.psTernary->psVar2->Vars.psBinary->psVar2)) ;
				 * && ((psTemp->Vars.psTernary->psVar2->Vars.psBinary->psVar2-> ;
				 * eOpType != OPTYPE_BINARY) ;
				 * || ((psTemp->Vars.psTernary->psVar2->Vars.psBinary->psVar2-> ;
				 * eOpType == OPTYPE_BINARY) ;
				 * && (psTemp->Vars.psTernary->psVar2->Vars.psBinary->psVar2-> ;
				 * Vars.psBinary->eOpType != OPBINARY_IND)))) ;
				 * { ;
				 * psTemp = psTemp->Vars.psTernary->psVar1;
				 * } ;
				 * if ((psTemp->eOpType == OPTYPE_TERNARY) ;
				 * && (psTemp->Vars.psTernary->eOpType == OPTERNARY_SET) ;
				 * && (psTemp->Vars.psTernary->psVar2->eOpType == OPTYPE_BINARY) ;
				 * && (psTemp->Vars.psTernary->psVar2->Vars.psBinary->eOpType ;
				 * == OPBINARY_IND) ;
				 * && (CompareOperations (psBin->psVar2, ;
				 * psTemp->Vars.psTernary->psVar2->Vars.psBinary->psVar2)) ;
				 * && ((psTemp->Vars.psTernary->psVar2->Vars.psBinary->psVar2-> ;
				 * eOpType != OPTYPE_BINARY) ;
				 * || ((psTemp->Vars.psTernary->psVar2->Vars.psBinary->psVar2-> ;
				 * eOpType == OPTYPE_BINARY) ;
				 * && (psTemp->Vars.psTernary->psVar2->Vars.psBinary->psVar2-> ;
				 * Vars.psBinary->eOpType != OPBINARY_IND)))) ;
				 * { ;
				 * psReturn = CopyRecursive (psTemp->Vars.psTernary->psVar3);
				 * FreeRecursive (psOp);
				 * }
				 */
				break;
			default:
				break;
			}

			break;
		case OPTYPE_TERNARY:
			psTer = psOp->Vars.psTernary;
			psTer->psVar1 = SimplifyOperation (psTer->psVar1);

			psTer->psVar2 = SimplifyOperation (psTer->psVar2);

			/*
			 * if ((psTer->eOpType == OPTERNARY_SET) ;
			 * && (psTer->psVar2->eOpType == OPTYPE_BINARY) ;
			 * && (psTer->psVar2->Vars.psBinary->eOpType == OPBINARY_IND)) ;
			 * { ;
			 * // Skip the ind operation, ;
			 * // since we can't have a set without an ind! ;
			 * psTer->psVar2->Vars.psBinary->psVar1 = ;
			 * SimplifyOperation (psTer->psVar2->Vars.psBinary->psVar1);
			 * psTer->psVar2->Vars.psBinary->psVar2 = ;
			 * SimplifyOperation (psTer->psVar2->Vars.psBinary->psVar2);
			 * } ;
			 * else ;
			 * { ;
			 * psTer->psVar2 = SimplifyOperation (psTer->psVar2);
			 * }
			 */
			psTer->psVar3 = SimplifyOperation (psTer->psVar3);
			psReturn = psOp;
			switch (psTer->eOpType) {
			case OPTERNARY_SET:
				if ((psTer->psVar1->eOpType == OPTYPE_TERNARY)
				&& (CompareOperations (psTer->psVar2, psTer->psVar1->Vars.psTernary->psVar2))) {
					psReturn = CreateTernary (OPTERNARY_SET, CopyRecursive (psTer->psVar1->Vars.psTernary->psVar1),
											  CopyRecursive (psTer->psVar2), CopyRecursive (psTer->psVar3));
					FreeRecursive (psOp);
				}

				break;
			default:
				break;
			}

			break;
		default:
			psReturn = psOp;
			printf ("Invalid operation type\n");
			break;
		}
	}

	return psReturn;
}

/*/
 =======================================================================================================================
 *  Substitute all instances of a given pair of subformula for respective formulae
 =======================================================================================================================
 */
Operation* SubstituteOperationPair (Operation*	psMain, Operation*	psFind1, Operation*	 psSub1, Operation*	 psFind2,
									Operation*	psSub2) {
	int nFind;
	Operation*	psReturn;

	nFind = SubstituteRecursivePair (psMain, psFind1, psSub1, psFind2, psSub2);
	switch (nFind) {
	default:
	case 0:
		psReturn = psMain;
		break;
	case 1:
		FreeRecursive (psMain);
		psReturn = CopyRecursive (psSub1);
		break;
	case 2:
		FreeRecursive (psMain);
		psReturn = CopyRecursive (psSub2);
		break;
	}

	return psReturn;
}

/*/
 =======================================================================================================================
 *  Substitute recursively all instances of a given pair of subformula for respective formulae
 =======================================================================================================================
 */
int SubstituteRecursivePair (Operation*	 psMain, Operation*	 psFind1, Operation*  psSub1, Operation*  psFind2,
							 Operation*	 psSub2) {
	int nSubstitute;
	int nFind;

	nSubstitute = 0;

	if ((psMain) && (psSub1) && (psSub2)) {
		switch (psMain->eOpType) {
		case OPTYPE_MEMORY:
		case OPTYPE_VARIABLE:
		case OPTYPE_TRUTHVALUE:
		case OPTYPE_INTEGER:
			nSubstitute = CompareOperationsPair (psMain, psFind1, psFind2);
			break;
		case OPTYPE_UNARY:
			nSubstitute = CompareOperationsPair (psMain, psFind1, psFind2);
			if (nSubstitute == 0) {
				nFind = SubstituteRecursivePair (psMain->Vars.psUnary->psVar1, psFind1, psSub1, psFind2, psSub2);
				switch (nFind) {
				case 1:
					FreeRecursive (psMain->Vars.psUnary->psVar1);
					psMain->Vars.psUnary->psVar1 = CopyRecursive (psSub1);
					break;
				case 2:
					FreeRecursive (psMain->Vars.psUnary->psVar1);
					psMain->Vars.psUnary->psVar1 = CopyRecursive (psSub2);
					break;
				default:

					/* Do nothing */
					break;
				}
			}

			break;
		case OPTYPE_BINARY:
			nSubstitute = CompareOperationsPair (psMain, psFind1, psFind2);
			if (nSubstitute == 0) {
				nFind = SubstituteRecursivePair (psMain->Vars.psBinary->psVar1, psFind1, psSub1, psFind2, psSub2);
				switch (nFind) {
				case 1:
					FreeRecursive (psMain->Vars.psBinary->psVar1);
					psMain->Vars.psBinary->psVar1 = CopyRecursive (psSub1);
					break;
				case 2:
					FreeRecursive (psMain->Vars.psBinary->psVar1);
					psMain->Vars.psBinary->psVar1 = CopyRecursive (psSub2);
					break;
				default:

					/* Do nothing */
					break;
				}

				nFind = SubstituteRecursivePair (psMain->Vars.psBinary->psVar2, psFind1, psSub1, psFind2, psSub2);
				switch (nFind) {
				case 1:
					FreeRecursive (psMain->Vars.psBinary->psVar2);
					psMain->Vars.psBinary->psVar2 = CopyRecursive (psSub1);
					break;
				case 2:
					FreeRecursive (psMain->Vars.psBinary->psVar2);
					psMain->Vars.psBinary->psVar2 = CopyRecursive (psSub2);
					break;
				default:

					/* Do nothing */
					break;
				}
			}

			break;
		case OPTYPE_TERNARY:
			nSubstitute = CompareOperationsPair (psMain, psFind1, psFind2);
			if (nSubstitute == 0) {
				nFind = SubstituteRecursivePair (psMain->Vars.psTernary->psVar1, psFind1, psSub1, psFind2, psSub2);
				switch (nFind) {
				case 1:
					FreeRecursive (psMain->Vars.psTernary->psVar1);
					psMain->Vars.psTernary->psVar1 = CopyRecursive (psSub1);
					break;
				case 2:
					FreeRecursive (psMain->Vars.psTernary->psVar1);
					psMain->Vars.psTernary->psVar1 = CopyRecursive (psSub2);
					break;
				default:

					/* Do nothing */
					break;
				}

				nFind = SubstituteRecursivePair (psMain->Vars.psTernary->psVar2, psFind1, psSub1, psFind2, psSub2);
				switch (nFind) {
				case 1:
					FreeRecursive (psMain->Vars.psTernary->psVar2);
					psMain->Vars.psTernary->psVar2 = CopyRecursive (psSub1);
					break;
				case 2:
					FreeRecursive (psMain->Vars.psTernary->psVar2);
					psMain->Vars.psTernary->psVar2 = CopyRecursive (psSub2);
					break;
				default:

					/* Do nothing */
					break;
				}

				nFind = SubstituteRecursivePair (psMain->Vars.psTernary->psVar3, psFind1, psSub1, psFind2, psSub2);
				switch (nFind) {
				case 1:
					FreeRecursive (psMain->Vars.psTernary->psVar3);
					psMain->Vars.psTernary->psVar3 = CopyRecursive (psSub1);
					break;
				case 2:
					FreeRecursive (psMain->Vars.psTernary->psVar3);
					psMain->Vars.psTernary->psVar3 = CopyRecursive (psSub2);
					break;
				default:

					/* Do nothing */
					break;
				}
			}

			break;
		default:
			printf ("Invalid operation type\n");
			break;
		}
	}

	return nSubstitute;
}

/*/
 =======================================================================================================================
 *  Compare a pair of formulae against a formula recursively
 =======================================================================================================================
 */
int CompareOperationsPair (Operation* psMain, Operation* psCompare1, Operation* psCompare2) {
	int nReturn;

	nReturn = 0;

	if (CompareOperations (psMain, psCompare1)) {
		nReturn = 1;
	} else {
		if (CompareOperations (psMain, psCompare2)) {
			nReturn = 2;
		}
	}

	return nReturn;
}
