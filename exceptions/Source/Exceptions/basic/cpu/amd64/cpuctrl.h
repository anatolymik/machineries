#pragma once

/***************************************************************************************\
|	Declarations																		|
\***************************************************************************************/

#define CPU_INTERRUPT_TYPE_INTERRUPT				0xe
#define CPU_INTERRUPT_TYPE_TRAP						0xf

#define CPU_DIVIDE_ERROR_VECTOR						0
#define CPU_DEBUG_VECTOR							1
#define CPU_NMI_VECTOR								2
#define CPU_BREAKPOINT_VECTOR						3
#define CPU_INTEGER_OVERFLOW_VECTOR					4
#define CPU_BOUND_RANGE_EXCEEDED_VECTOR				5
#define CPU_INVALID_OPCODE_VECTOR					6
#define CPU_DEVICE_NOT_AVAILABLE_VECTOR				7
#define CPU_DOUBLE_FAULT_VECTOR						8
#define CPU_COPROCESSOR_SEGMENT_OVERRUN_VECTOR		9
#define CPU_INVALID_TSS_VECTOR						10
#define CPU_SEGMENT_NOT_PRESENT_VECTOR				11
#define CPU_STACK_FAULT_VECTOR						12
#define CPU_GENERAL_PROTECTION_VECTOR				13
#define CPU_PAGE_FAULT_VECTOR						14
#define CPU_FPU_ERROR_VECTOR						16
#define CPU_ALIGNMENT_CHECK_VECTOR					17
#define CPU_MACHINE_CHECK_VECTOR					18
#define CPU_SIMD_VECTOR								19
#define CPU_EXCEPTION_VECTORS						32
#define CPU_VECTORS									256

#define CPU_PAGE_FAULT_ERROR_PRESENT				0x01
#define CPU_PAGE_FAULT_ERROR_WR						0x02
#define CPU_PAGE_FAULT_ERROR_US						0x04
#define CPU_PAGE_FAULT_ERROR_RSVD					0x08
#define CPU_PAGE_FAULT_ERROR_ID						0x10

#define CPU_SIMD_INVALID_OPERATION					0x0001
#define CPU_SIMD_DENORMAL							0x0002
#define CPU_SIMD_ZERO_DIVIDE						0x0004
#define CPU_SIMD_OVERFLOW							0x0008
#define CPU_SIMD_UNDERFLOW							0x0010
#define CPU_SIMD_PRECISION							0x0020
#define CPU_SIMD_DENORMAL_ARE_ZEROS					0x0040
#define CPU_SIMD_INVALID_OPERATION_MASK				0x0080
#define CPU_SIMD_DENORMAL_OPERATION_MASK			0x0100
#define CPU_SIMD_DIVIDE_BY_ZERO_MASK				0x0200
#define CPU_SIMD_OVERFLOW_MASK						0x0400
#define CPU_SIMD_UNDERFLOW_MASK						0x0800
#define CPU_SIMD_PRECISION_MASK						0x1000
#define CPU_SIMD_ROUND_NEAREST						0x0000
#define CPU_SIMD_ROUND_DOWN							0x2000
#define CPU_SIMD_ROUND_UP							0x4000
#define CPU_SIMD_ROUND_TOWARD_ZERO					0x6000
#define CPU_SIMD_FLUSH_TO_ZERO						0x8000

/***************************************************************************************\
|	Structures																			|
\***************************************************************************************/

#pragma pack( push, 1 )

struct SIdtr {

	uint16_t	Limit;
	uint64_t	Base;

};

struct SIntDesc {

	uint16_t	Base1;

	uint16_t	Selector;

	uint8_t		IST			: 3;
	uint8_t		Zeroes1		: 5;

	uint8_t		Type		: 4;
	uint8_t		Zeroes2		: 1;
	uint8_t		DPL			: 2;
	uint8_t		P			: 1;

	uint16_t	Base2;
	uint32_t	Base3;

	uint32_t	Reserved;

};

#pragma pack( pop )
