/*$T MATTS/cVM.cpp GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cVM.cpp ;
 * Last Modified: 10/11/04 ;
 * ;
 * Purpose: Implementation file for cross platform UbiComp virtual machine ;
 */
#include "cVM.h"
#include "cConsole.h"

/*
 =======================================================================================================================
 *  Constructor
 =======================================================================================================================
 */
cVM::cVM (long int size, cTopologyNode* psSetTopolgyNode) {
	bActive = false;
	bLoaded = false;
	bRestart = false;
	bInput = false;
	bListen = false;
	iProgramSize = 0;
	pMemory = NULL;
	memSize = size;
	mainMemory = new char[memSize];
	mainMemory[0] = '\0';
	iCurrentInst = 0;
	error = NULL;
	pProgram = NULL;
	iMaxMemory = 0;
	inputPos = 0;

	for (int x = 0; x < 32; x++) {
		for (int y = 0; y < 81; y++) {
			buffer[x][y] = 0;
		}
	}

	for (int x = 0; x < 256; x++) {
		inBuffer[x] = 0;
	}

	inBuffer[255] = '\0';

	pLibrary = cLocalLibrary::create ();
	threadHandle = NULL;
	BufferXPos = 0;
	BufferRow = 0;
	psTopologyNode = psSetTopolgyNode;
}

/*
 =======================================================================================================================
 *  Destructor
 =======================================================================================================================
 */
cVM::~cVM () {

	/* need to clean up */
	if (error) {
		delete error;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cVM::checkMaxMemory (long int* pM) {
	long int iCurrent;

	iCurrent = (long int) (pM - pMemory);
	if (iCurrent > iMaxMemory) {
		iMaxMemory = iCurrent;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cVM::getBuffer (char buf[32][81], int* xPos, int* row, char string[256], int* inputBufPos) {
	for (int x = 0; x < 32; x++) {
		for (int y = 0; y < 81; y++) {
			buf[x][y] = buffer[x][y];
		}
	}

	for (int x = 0; x < 256; x++)
		string[x] = inBuffer[x];

	*xPos = BufferXPos;
	*row = BufferRow;
	*inputBufPos = inputPos;
}

/*
 =======================================================================================================================
 *  execute the program
 =======================================================================================================================
 */
void cVM::execute () {
	long int*  pnM1;
	long int*  pnM2;
	long int*  pnM3;
	long int nM1;
	long int nM2;
	long int nM3;
	long int nR1;
	long int nR2;
	long int nR3;
	bool bException;

	bException = false;
	bRestart = false;

	while ((iCurrentInst < iProgramSize) && (!bException) && (!bRestart) && (!threadQuit)) {

		/* Dereference values */
		nM1 = pProgram[iCurrentInst].nM1;
		nR1 = pProgram[iCurrentInst].nR1;

		bException = dereference (&pnM1, &nM1, &nR1);

		nM2 = pProgram[iCurrentInst].nM2;
		nR2 = pProgram[iCurrentInst].nR2;

		bException = dereference (&pnM2, &nM2, &nR2);

		nM3 = pProgram[iCurrentInst].nM3;
		nR3 = pProgram[iCurrentInst].nR3;

		bException = dereference (&pnM3, &nM3, &nR3);

		if (!bException) {

			/* Execute the instruction */
			switch (pProgram[iCurrentInst].inst) {
			case mnemonic_mov:
				executeMOV (pnM1, nM3);
				checkMaxMemory (pnM1);
				break;
			case mnemonic_add:
				executeADD (pnM1, nM2, nM3);
				checkMaxMemory (pnM1);
				break;
			case mnemonic_sub:
				executeSUB (pnM1, nM2, nM3);
				checkMaxMemory (pnM1);
				break;
			case mnemonic_mul:
				executeMUL (pnM1, nM2, nM3);
				checkMaxMemory (pnM1);
				break;
			case mnemonic_div:
				executeDIV (pnM1, nM2, nM3);
				checkMaxMemory (pnM1);
				break;
			case mnemonic_mod:
				executeMOD (pnM1, nM2, nM3);
				checkMaxMemory (pnM1);
				break;
			case mnemonic_orr:
				executeORR (pnM1, nM2, nM3);
				checkMaxMemory (pnM1);
				break;
			case mnemonic_and:
				executeAND (pnM1, nM2, nM3);
				checkMaxMemory (pnM1);
				break;
			case mnemonic_bic:
				executeBIC (pnM1, nM2, nM3);
				checkMaxMemory (pnM1);
				break;
			case mnemonic_eor:
				executeEOR (pnM1, nM2, nM3);
				checkMaxMemory (pnM1);
				break;
			case mnemonic_ror:
				executeROR (pnM1, nM2, nM3);
				checkMaxMemory (pnM1);
				break;
			case mnemonic_rol:
				executeROL (pnM1, nM2, nM3);
				checkMaxMemory (pnM1);
				break;
			case mnemonic_beq:
				executeBEQ (nM1, nM2, nM3);
				break;
			case mnemonic_bne:
				executeBNE (nM1, nM2, nM3);
				break;
			case mnemonic_blt:
				executeBLT (nM1, nM2, nM3);
				break;
			case mnemonic_ble:
				executeBLE (nM1, nM2, nM3);
				break;
			case mnemonic_bgt:
				executeBGT (nM1, nM2, nM3);
				break;
			case mnemonic_bge:
				executeBGE (nM1, nM2, nM3);
				break;
			case mnemonic_lib:
				executeLIB (nM1, pnM2, nM2, pnM3, nM3);
				break;
			default:
				ConsolePrint ("Inst: Invalid\n");
				bRestart = true;
				bLoaded = false;
				break;
			}
		}

		if (iCurrentInst < 0) {
			bException = true;
		}
	}

	if (bException) {
		ConsolePrint ("**Exception**\n");
	}

	if (iCurrentInst >= iProgramSize) {
		bRestart = true;
		bLoaded = false;
	}

	return ;
}

/*
 =======================================================================================================================
 *  MOV instruction
 =======================================================================================================================
 */
void cVM::executeMOV (long int* pnM1, long int nM3) {
	if (pnM1) {
		*pnM1 = nM3;
	}

	iCurrentInst++;
}

/*
 =======================================================================================================================
 *  ADD instruction
 =======================================================================================================================
 */
void cVM::executeADD (long int* pnM1, long int nM2, long int nM3) {
	if (pnM1) {
		*pnM1 = nM2 + nM3;
	}

	iCurrentInst++;
}

/*
 =======================================================================================================================
 *  SUB instruction
 =======================================================================================================================
 */
void cVM::executeSUB (long int* pnM1, long int nM2, long int nM3) {
	if (pnM1) {
		*pnM1 = nM2 - nM3;
	}

	iCurrentInst++;
}

/*
 =======================================================================================================================
 *  MUL instruction
 =======================================================================================================================
 */
void cVM::executeMUL (long int* pnM1, long int nM2, long int nM3) {
	if (pnM1) {
		*pnM1 = nM2 * nM3;
	}

	iCurrentInst++;
}

/*
 =======================================================================================================================
 *  DIV instruction
 =======================================================================================================================
 */
void cVM::executeDIV (long int* pnM1, long int nM2, long int nM3) {
	if (pnM1) {
		*pnM1 = nM2 / nM3;
	}

	iCurrentInst++;
}

/*
 =======================================================================================================================
 *  MOD instruction
 =======================================================================================================================
 */
void cVM::executeMOD (long int* pnM1, long int nM2, long int nM3) {
	if (pnM1) {
		*pnM1 = nM2 % nM3;
	}

	iCurrentInst++;
}

/*
 =======================================================================================================================
 *  OR instruction
 =======================================================================================================================
 */
void cVM::executeORR (long int* pnM1, long int nM2, long int nM3) {
	if (pnM1) {
		*pnM1 = nM2 | nM3;
	}

	iCurrentInst++;
}

/*
 =======================================================================================================================
 *  AND instruction
 =======================================================================================================================
 */
void cVM::executeAND (long int* pnM1, long int nM2, long int nM3) {
	if (pnM1) {
		*pnM1 = nM2 & nM3;
	}

	iCurrentInst++;
}

/*
 =======================================================================================================================
 *  BIC instruction
 =======================================================================================================================
 */
void cVM::executeBIC (long int* pnM1, long int nM2, long int nM3) {
	if (pnM1) {
		*pnM1 = nM2 & (~nM3);
	}

	iCurrentInst++;
}

/*
 =======================================================================================================================
 *  EOR instruction
 =======================================================================================================================
 */
void cVM::executeEOR (long int* pnM1, long int nM2, long int nM3) {
	if (pnM1) {
		*pnM1 = nM2 ^ nM3;
	}

	iCurrentInst++;
}

/*
 =======================================================================================================================
 *  ROR instruction
 =======================================================================================================================
 */
void cVM::executeROR (long int* pnM1, long int nM2, long int nM3) {
	if (pnM1) {
		*pnM1 = nM2 >> nM3;
	}

	iCurrentInst++;
}

/*
 =======================================================================================================================
 *  ROL instruction
 =======================================================================================================================
 */
void cVM::executeROL (long int* pnM1, long int nM2, long int nM3) {
	if (pnM1) {
		*pnM1 = nM2 << nM3;
	}

	iCurrentInst++;
}

/*
 =======================================================================================================================
 *  BEQ instruction
 =======================================================================================================================
 */
void cVM::executeBEQ (long int nM1, long int nM2, long int nM3) {
	if (nM2 == nM3) {
		iCurrentInst = nM1;
	} else {
		iCurrentInst++;
	}
}

/*
 =======================================================================================================================
 *  BNE instruction
 =======================================================================================================================
 */
void cVM::executeBNE (long int nM1, long int nM2, long int nM3) {
	if (nM2 != nM3) {
		iCurrentInst = nM1;
	} else {
		iCurrentInst++;
	}
}

/*
 =======================================================================================================================
 *  BLT instruction
 =======================================================================================================================
 */
void cVM::executeBLT (long int nM1, long int nM2, long int nM3) {
	if (nM2 < nM3) {
		iCurrentInst = nM1;
	} else {
		iCurrentInst++;
	}
}

/*
 =======================================================================================================================
 *  BLE instruction
 =======================================================================================================================
 */
void cVM::executeBLE (long int nM1, long int nM2, long int nM3) {
	if (nM2 <= nM3) {
		iCurrentInst = nM1;
	} else {
		iCurrentInst++;
	}
}

/*
 =======================================================================================================================
 *  BGT instruction
 =======================================================================================================================
 */
void cVM::executeBGT (long int nM1, long int nM2, long int nM3) {
	if (nM2 > nM3) {
		iCurrentInst = nM1;
	} else {
		iCurrentInst++;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cVM::executeBGE (long int nM1, long int nM2, long int nM3) {
	if (nM2 >= nM3) {
		iCurrentInst = nM1;
	} else {
		iCurrentInst++;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cVM::executeLIB (long int nM1, long int* pnM2, long int nM2, long int* pnM3, long int nM3) {
	pnM3 = pnM3;
	switch (nM1) {
	case lib_printString:
		pLibrary->executeLIB_PrintString (pnM2, this);
		iCurrentInst++;
		break;
	case lib_printInteger:
		pLibrary->executeLIB_PrintInteger (nM2, this);
		iCurrentInst++;
		break;
	case lib_printNumber:
		pLibrary->executeLIB_PrintNumber (nM2, this);
		iCurrentInst++;
		break;
	case lib_inputString:
		pLibrary->executeLIB_InputString (pnM2, nM3, this);
		iCurrentInst++;
		break;
	case lib_inputNumber:
		pLibrary->executeLIB_InputNumber (pnM2, this);
		iCurrentInst++;
		break;
	case lib_sendState:
		pLibrary->executeLIB_SendState (pnM2, this);
		iCurrentInst++;
		break;
	case lib_checkHost:
		pLibrary->executeLIB_CheckHost (pnM2, pnM3, this);
		iCurrentInst++;
		break;
	case lib_playSound:
		pLibrary->executeLIB_PlaySound (nM2, nM3);
		iCurrentInst++;
		break;
	case lib_sendInteger:
		if (pLibrary->executeLIB_SendInteger (nM2, nM3, this)) {
			iCurrentInst++;
		}

		break;
	case lib_receiveInteger:
		if (pLibrary->executeLIB_ReceiveInteger (pnM2, nM3, this)) {
			iCurrentInst++;
		}

		break;
	case lib_heapAlloc:
		pLibrary->executeLIB_HeapAlloc (pnM2, nM3, this);
		iCurrentInst++;
		break;
	case lib_setImage:
		pLibrary->executeLIB_SetImage (pnM2, this);
		iCurrentInst++;
		break;
	case lib_readImage:
		pLibrary->executeLIB_ReadImage (pnM2, this);
		iCurrentInst++;
		break;
	case lib_wait:
		pLibrary->executeLIB_Wait (nM2, this);
		iCurrentInst++;
		break;
	default:
		ConsolePrint ("LIB invalid\n");
		break;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cVM::dereference (long int ** ppnM1, long int* pnM1, long int* pnR1) {
	bool bException;

	bException = false;
	*ppnM1 = NULL;
	while (*pnR1 > 0) {
		if ((*pnM1 < 0) || (*pnM1 >= memorySize)) {
			bException = true;
		} else {
			*ppnM1 = &pMemory[*pnM1];
			*pnM1 = pMemory[*pnM1];
		}

		(*pnR1)--;
	}

	return bException;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
long int cVM::storeString (char* szString, long int* pnMemory, long int nLength) {
	unsigned long int uStore;
	long int nShift;
	long int nLetter;
	long int nPos;
	char cChar;

	nLetter = 0;
	uStore = 0;
	nShift = 0;
	nPos = 0;

	while (nLetter < nLength) {
		switch (szString[nLetter]) {
		case '\n':
			cChar = 0;
			break;
		case '\\':
			nLetter++;
			switch (szString[nLetter]) {
			case 'n':
				cChar = '\n';
				break;
			default:
				cChar = '\\';
				break;
			}

			break;
		default:
			cChar = szString[nLetter];
			break;
		}

		nLetter++;
		if (cChar == 0) {
			nLetter = nLength;
		}

		uStore |= cChar << (nShift * 8);
		nShift++;
		if ((nShift > 3) || (nLetter >= nLength)) {

			/* Store the latest four letters */
			if (nLetter == nLength) {

				/* Store a terminating null */
				uStore &= ~(0xff << 24);
			}

			pnMemory[nPos] = (long int) (uStore);
			uStore = 0;
			nShift = 0;
			nPos++;
		}
	}

	return nPos;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cVM::loadExecute (char* szExecuteFile) {
	long int nExecFileSize;
	long int nInputMemSize;

	nExecFileSize = loadFileAt (szExecuteFile);

	if (nExecFileSize == 0) {
		ConsolePrint ("File Failed to Load");
		return 0;
	} else {
		iCurrentInst = ((outputHeader*) mainMemory)->PC;

		iProgramSize = ((((outputHeader*) mainMemory)->progSize) / sizeof (instruction)) - 1;
		nInputMemSize = ((((outputHeader*) mainMemory)->inputSize) / sizeof (long int));

		pProgram = (instruction *) (mainMemory + ((outputHeader*) mainMemory)->progStartPos);
		pMemory = (long int*) (mainMemory + ((outputHeader*) mainMemory)->inputStartPos);

		iMaxMemory = nInputMemSize;

		bRestart = true;
		bLoaded = true;
	}

	return nExecFileSize;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
long int cVM::loadFileAt (char* szFilename) {
	pLibrary->localGetFileSize (szFilename, &iProgramSize, this);

	if (iProgramSize > 0) {
		if (mainMemory) {
			pLibrary->localLoadFile (szFilename, mainMemory, this);
		}
	} else {
		iProgramSize = 0;
	}

	return iProgramSize;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cVM::saveExecutingProgram (char* szOutputFile) {
	long int nOutputSize;
	char*  pOutputFile;
	long int nOutFilePos;
	long int nOutFileSectionSize;

	nOutputSize = sizeof (outputHeader) +
		((iProgramSize + 1) * sizeof (instruction)) +
		(iMaxMemory * sizeof (long int));
	pOutputFile = mainMemory;
	((outputHeader*) pOutputFile)->PC = iCurrentInst;

	nOutFilePos = 0;
	nOutFileSectionSize = sizeof (outputHeader);
	((outputHeader*) pOutputFile)->headerSize = nOutFileSectionSize;

	nOutFilePos += nOutFileSectionSize;
	nOutFileSectionSize = (iProgramSize + 1) * sizeof (instruction);
	((outputHeader*) pOutputFile)->progStartPos = nOutFilePos;
	((outputHeader*) pOutputFile)->progSize = nOutFileSectionSize;

	nOutFilePos += nOutFileSectionSize;
	nOutFileSectionSize = (iMaxMemory * sizeof (long int));
	((outputHeader*) pOutputFile)->inputStartPos = nOutFilePos;
	((outputHeader*) pOutputFile)->inputSize = nOutFileSectionSize;

	pLibrary->localSaveFile (szOutputFile, pOutputFile, nOutputSize, this);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cVM::packageExecutingProgram (char ** ppcOutputData, long int* pnOutputSize) {
	long int nOutputSize;
	char*  pOutputFile;
	long int nOutFilePos;
	long int nOutFileSectionSize;

	nOutputSize = sizeof (outputHeader) +
		((iProgramSize + 1) * sizeof (instruction)) +
		(iMaxMemory * sizeof (long int));
	pOutputFile = mainMemory;	/* (char*)calloc (nOutputSize, sizeof
								 * (char));
								 * */
	((outputHeader*) pOutputFile)->PC = iCurrentInst;

	nOutFilePos = 0;
	nOutFileSectionSize = sizeof (outputHeader);
	((outputHeader*) pOutputFile)->headerSize = nOutFileSectionSize;

	nOutFilePos += nOutFileSectionSize;
	nOutFileSectionSize = (iProgramSize + 1) * sizeof (instruction);
	((outputHeader*) pOutputFile)->progStartPos = nOutFilePos;
	((outputHeader*) pOutputFile)->progSize = nOutFileSectionSize;

	nOutFilePos += nOutFileSectionSize;
	nOutFileSectionSize = (iMaxMemory * sizeof (long int));
	((outputHeader*) pOutputFile)->inputStartPos = nOutFilePos;
	((outputHeader*) pOutputFile)->inputSize = nOutFileSectionSize;

	if (ppcOutputData) {
		*ppcOutputData = pOutputFile;
	}

	if (pnOutputSize) {
		*pnOutputSize = nOutputSize;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cVM::setUpExecution (void) {
	ConsolePrint ("Starting Execution\n");
	iCurrentInst = ((outputHeader*) mainMemory)->PC + 1;

	iProgramSize = ((((outputHeader*) mainMemory)->progSize) / sizeof (instruction)) - 1;

	pProgram = (instruction *) (mainMemory + ((outputHeader*) mainMemory)->progStartPos);
	pMemory = (long int*) (mainMemory + ((outputHeader*) mainMemory)->inputStartPos);

	iMaxMemory = ((((outputHeader*) mainMemory)->inputSize) / sizeof (long int));;

	bRestart = true;
	bLoaded = true;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cVM::ConsolePrint (char* szString) {
	if (bActive == true) {
		cConsole*  pCon;

		/*
		 * machine is currently active in main window;
		 * buffer then print to console
		 */
		bufferOutput (szString);
		pCon = cConsole::create ();
		pCon->ConsolePrint (szString);
	} else {

		/*
		 * machine not active in main window;
		 * buffer output
		 */
		bufferOutput (szString);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
char* cVM::getInBuffer () {
	return inBuffer;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cVM::inputChar (char c) {
	inBuffer[inputPos] = c;
	if (c == 0 || c == 8) {

		/* delete */
		inputPos--;
	} else {
		inputPos++;
	}

	inBuffer[inputPos] = 0;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cVM::clearInput () {
	for (int x = 0; x < 256; x++)
		inBuffer[x] = 0;
	inputPos = 0;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cVM::bufferOutput (char* szString) {
	int nStrPos;
	int nMaxLen;
	int nCopyPos;
	char cChar;

	nStrPos = 0;
	nMaxLen = 0;
	nCopyPos = 0;
	cChar = 0;

	while (szString[nStrPos] > 0) {
		if (BufferXPos >= 80) {

			/* Scroll the window */
			BufferRow++;
			if (BufferRow >= 32) {
				BufferRow = 0;
			}

			BufferXPos = 0;
		}

		nMaxLen = 80 - BufferXPos;

		/*
		 * Copy the string over ;
		 * Until nMaxLen is reached, or a newline
		 */
		nCopyPos = 0;
		while ((nCopyPos < nMaxLen) && (szString[nStrPos] >= 0x20)) {
			cChar = szString[nStrPos++];
			buffer[BufferRow][BufferXPos + nCopyPos++] = cChar;
		}

		buffer[BufferRow][BufferXPos + nCopyPos] = 0;

		/* Print the line */
		BufferXPos += nCopyPos;

		if ((szString[nStrPos] == 0x0d) || (szString[nStrPos] == 0x0a)) {

			/* Newline */
			BufferXPos = 80;
			nStrPos++;
		}
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cVM::clearBuffer () {
	for (int x = 0; x < 32; x++) {
		for (int y = 0; y < 81; y++) {
			buffer[x][y] = 0;
		}
	}

	BufferXPos = 0;
	BufferRow = 0;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cVM::threadPause (bool boPause) {
	if (boPause) {

		/* Pause the thread */
		if (SuspendThread (threadHandle) != -1) {
			boPaused = true;
		}
	} else {

		/* Restart the thread */
		if (ResumeThread (threadHandle) != -1) {
			boPaused = false;
		}
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cVM::threadPaused () {
	return boPaused;
}
