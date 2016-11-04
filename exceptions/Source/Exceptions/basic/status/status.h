#pragma once

/***************************************************************************************\
|	Status codes																		|
\***************************************************************************************/

#define ERR_SUCCESS						0

#define ERR_UNWIND						0x80000001
#define ERR_NO_MEMORY					0x80000002
#define ERR_DIVISION_BY_ZERO			0x80000003
#define ERR_INTEGER_OVERFLOW			0x80000004
#define ERR_ARRAY_BOUNDS_EXCEEDED		0x80000005
#define ERR_ILLEGAL_INSTRUCTION			0x80000006
#define ERR_ACCESS_VIOLATION			0x80000007
#define ERR_FLOAT_INVALID_OPERATION		0x80000008
#define ERR_FLOAT_DIVIDE_BY_ZERO		0x80000009
#define ERR_FLOAT_OVERFLOW				0x8000000a
#define ERR_FLOAT_UNDERFLOW				0x8000000b
#define ERR_FLOAT_INEXACT_RESULT		0x8000000c
#define ERR_UNEXCPECTED_EXCEPTION		0x8000000d
#define ERR_MACHINE_FAILURE				0x8000000e

#define ERR_PE_UNRECOGNIZED				0x8000000f
#define ERR_PE_UNSUPPORTED				0x80000010
#define ERR_PE_UNSUPPORTED_PDATA		0x80000011
#define ERR_PE_CORRUPED_PDATA			0x80000012
