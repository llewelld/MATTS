/*$T MATTS/cVMTypes.h GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cVMTypes.h ;
 * Last Modified: 10/10/04 ;
 * ;
 * Purpose: Header file for data types used by virtual machine ;
 */
#ifndef CVMTYPES_H
#define CVMTYPES_H

const int memorySize = 5000;
#ifndef NULL
const int NULL = 0;
#endif
enum mnemonic
{
	mnemonic_invalid= -1,
	mnemonic_mov,
	mnemonic_add,
	mnemonic_sub,
	mnemonic_mul,
	mnemonic_div,
	mnemonic_mod,
	mnemonic_orr,
	mnemonic_and,
	mnemonic_bic,
	mnemonic_eor,
	mnemonic_ror,
	mnemonic_rol,
	mnemonic_beq,
	mnemonic_bne,
	mnemonic_blt,
	mnemonic_ble,
	mnemonic_bgt,
	mnemonic_bge,
	mnemonic_lib,
	mnemonic_num
};

struct instruction
{
	mnemonic inst;
	long int nM1;
	long int nM2;
	long int nM3;
	long int nR1;
	long int nR2;
	long int nR3;
};

enum lib
{
	lib_invalid			= -1,
	lib_printString,
	lib_printInteger,
	lib_printNumber,
	lib_inputString,
	lib_inputNumber,
	lib_sendState,
	lib_checkHost,
	lib_playSound,
	lib_sendInteger,
	lib_receiveInteger,
	lib_heapAlloc,
	lib_setImage,
	lib_readImage,
	lib_wait,
	lib_num
};

struct outputHeader
{
	long int headerSize;
	long int progStartPos;
	long int progSize;
	long int inputStartPos;
	long int inputSize;
	long int PC;
};

struct os_error
{
	int errnum;
	char errmess[212];
};
#endif
