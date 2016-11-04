#include "..\..\basic\basic.h"

#include "..\..\pe\pe.h"

#include "excdsptch.h"
#include "exccpu.h"

/***************************************************************************************\
|	External functions																	|
\***************************************************************************************/

extern "C" void	CpuDivideError();
extern "C" void	CpuDebug();
extern "C" void	CpuNmi();
extern "C" void	CpuBreakpoint();
extern "C" void	CpuIntegerOverflow();
extern "C" void	CpuBoundRangeExceeded();
extern "C" void	CpuInvalidOpcode();
extern "C" void	CpuDeviceNotAvailable();
extern "C" void	CpuDoubleFault();
extern "C" void	CpuCoprocessorSegmentOverrun();
extern "C" void	CpuInvalidTss();
extern "C" void	CpuSegmentNotPresent();
extern "C" void	CpuStackFault();
extern "C" void	CpuGeneralProtection();
extern "C" void	CpuPageFault();
extern "C" void	CpuFpuError();
extern "C" void	CpuAlignmentCheck();
extern "C" void	CpuMachineCheck();
extern "C" void	CpuSIMD();
extern "C" void	CpuUnexpectedException();

/***************************************************************************************\
|	Functions																			|
\***************************************************************************************/

void CpuInitializeExceptionHandlers( SIdtr *Idtr ) {

	SIntDesc *Desc;

	// initialize default interrupt gates
	Desc = (SIntDesc*)Idtr->Base;
	for ( uint32_t i = 0; i < CPU_EXCEPTION_VECTORS; i++, Desc++ ) {

		Desc->Base1 = (uint64_t)CpuUnexpectedException & 0xffff;
		if ( i == CPU_NMI_VECTOR ) {
			Desc->Type = CPU_INTERRUPT_TYPE_INTERRUPT;
		} else if ( i < CPU_EXCEPTION_VECTORS ) {
			Desc->Type = CPU_INTERRUPT_TYPE_TRAP;
		} else {
			Desc->Type = CPU_INTERRUPT_TYPE_INTERRUPT;
		}
		Desc->P = true;
		Desc->Base2 = ((uint64_t)CpuUnexpectedException >> 16) & 0xffff;
		Desc->Base3 = (uint64_t)CpuUnexpectedException >> 32;

	}

	// initialize specific exception handler entry points
	Desc = (SIntDesc*)Idtr->Base;

	Desc[CPU_DIVIDE_ERROR_VECTOR].Base1 = (uint64_t)CpuDivideError & 0xffff;
	Desc[CPU_DIVIDE_ERROR_VECTOR].Base2 = ((uint64_t)CpuDivideError >> 16) & 0xffff;
	Desc[CPU_DIVIDE_ERROR_VECTOR].Base3 = (uint64_t)CpuDivideError >> 32;

	Desc[CPU_DEBUG_VECTOR].Base1 = (uint64_t)CpuDebug & 0xffff;
	Desc[CPU_DEBUG_VECTOR].Base2 = ((uint64_t)CpuDebug >> 16) & 0xffff;
	Desc[CPU_DEBUG_VECTOR].Base3 = (uint64_t)CpuDebug >> 32;

	Desc[CPU_NMI_VECTOR].Base1 = (uint64_t)CpuNmi & 0xffff;
	Desc[CPU_NMI_VECTOR].Base2 = ((uint64_t)CpuNmi >> 16) & 0xffff;
	Desc[CPU_NMI_VECTOR].Base3 = (uint64_t)CpuNmi >> 32;

	Desc[CPU_BREAKPOINT_VECTOR].Base1 = (uint64_t)CpuBreakpoint & 0xffff;
	Desc[CPU_BREAKPOINT_VECTOR].Base2 = ((uint64_t)CpuBreakpoint >> 16) & 0xffff;
	Desc[CPU_BREAKPOINT_VECTOR].Base3 = (uint64_t)CpuBreakpoint >> 32;

	Desc[CPU_INTEGER_OVERFLOW_VECTOR].Base1 = (uint64_t)CpuIntegerOverflow & 0xffff;
	Desc[CPU_INTEGER_OVERFLOW_VECTOR].Base2 = ((uint64_t)CpuIntegerOverflow >> 16) & 0xffff;
	Desc[CPU_INTEGER_OVERFLOW_VECTOR].Base3 = (uint64_t)CpuIntegerOverflow >> 32;

	Desc[CPU_BOUND_RANGE_EXCEEDED_VECTOR].Base1 = (uint64_t)CpuBoundRangeExceeded & 0xffff;
	Desc[CPU_BOUND_RANGE_EXCEEDED_VECTOR].Base2 = ((uint64_t)CpuBoundRangeExceeded >> 16) & 0xffff;
	Desc[CPU_BOUND_RANGE_EXCEEDED_VECTOR].Base3 = (uint64_t)CpuBoundRangeExceeded >> 32;

	Desc[CPU_INVALID_OPCODE_VECTOR].Base1 = (uint64_t)CpuInvalidOpcode & 0xffff;
	Desc[CPU_INVALID_OPCODE_VECTOR].Base2 = ((uint64_t)CpuInvalidOpcode >> 16) & 0xffff;
	Desc[CPU_INVALID_OPCODE_VECTOR].Base3 = (uint64_t)CpuInvalidOpcode >> 32;

	Desc[CPU_DEVICE_NOT_AVAILABLE_VECTOR].Base1 = (uint64_t)CpuDeviceNotAvailable & 0xffff;
	Desc[CPU_DEVICE_NOT_AVAILABLE_VECTOR].Base2 = ((uint64_t)CpuDeviceNotAvailable >> 16) & 0xffff;
	Desc[CPU_DEVICE_NOT_AVAILABLE_VECTOR].Base3 = (uint64_t)CpuDeviceNotAvailable >> 32;

	Desc[CPU_DOUBLE_FAULT_VECTOR].Base1 = (uint64_t)CpuDoubleFault & 0xffff;
	Desc[CPU_DOUBLE_FAULT_VECTOR].Base2 = ((uint64_t)CpuDoubleFault >> 16) & 0xffff;
	Desc[CPU_DOUBLE_FAULT_VECTOR].Base3 = (uint64_t)CpuDoubleFault >> 32;

	Desc[CPU_COPROCESSOR_SEGMENT_OVERRUN_VECTOR].Base1 = (uint64_t)CpuCoprocessorSegmentOverrun & 0xffff;
	Desc[CPU_COPROCESSOR_SEGMENT_OVERRUN_VECTOR].Base2 = ((uint64_t)CpuCoprocessorSegmentOverrun >> 16) & 0xffff;
	Desc[CPU_COPROCESSOR_SEGMENT_OVERRUN_VECTOR].Base3 = (uint64_t)CpuCoprocessorSegmentOverrun >> 32;

	Desc[CPU_INVALID_TSS_VECTOR].Base1 = (uint64_t)CpuInvalidTss & 0xffff;
	Desc[CPU_INVALID_TSS_VECTOR].Base2 = ((uint64_t)CpuInvalidTss >> 16) & 0xffff;
	Desc[CPU_INVALID_TSS_VECTOR].Base3 = (uint64_t)CpuInvalidTss >> 32;

	Desc[CPU_SEGMENT_NOT_PRESENT_VECTOR].Base1 = (uint64_t)CpuSegmentNotPresent & 0xffff;
	Desc[CPU_SEGMENT_NOT_PRESENT_VECTOR].Base2 = ((uint64_t)CpuSegmentNotPresent >> 16) & 0xffff;
	Desc[CPU_SEGMENT_NOT_PRESENT_VECTOR].Base3 = (uint64_t)CpuSegmentNotPresent >> 32;

	Desc[CPU_STACK_FAULT_VECTOR].Base1 = (uint64_t)CpuStackFault & 0xffff;
	Desc[CPU_STACK_FAULT_VECTOR].Base2 = ((uint64_t)CpuStackFault >> 16) & 0xffff;
	Desc[CPU_STACK_FAULT_VECTOR].Base3 = (uint64_t)CpuStackFault >> 32;

	Desc[CPU_GENERAL_PROTECTION_VECTOR].Base1 = (uint64_t)CpuGeneralProtection & 0xffff;
	Desc[CPU_GENERAL_PROTECTION_VECTOR].Base2 = ((uint64_t)CpuGeneralProtection >> 16) & 0xffff;
	Desc[CPU_GENERAL_PROTECTION_VECTOR].Base3 = (uint64_t)CpuGeneralProtection >> 32;

	Desc[CPU_PAGE_FAULT_VECTOR].Base1 = (uint64_t)CpuPageFault & 0xffff;
	Desc[CPU_PAGE_FAULT_VECTOR].Base2 = ((uint64_t)CpuPageFault >> 16) & 0xffff;
	Desc[CPU_PAGE_FAULT_VECTOR].Base3 = (uint64_t)CpuPageFault >> 32;

	Desc[CPU_FPU_ERROR_VECTOR].Base1 = (uint64_t)CpuFpuError & 0xffff;
	Desc[CPU_FPU_ERROR_VECTOR].Base2 = ((uint64_t)CpuFpuError >> 16) & 0xffff;
	Desc[CPU_FPU_ERROR_VECTOR].Base3 = (uint64_t)CpuFpuError >> 32;

	Desc[CPU_ALIGNMENT_CHECK_VECTOR].Base1 = (uint64_t)CpuAlignmentCheck & 0xffff;
	Desc[CPU_ALIGNMENT_CHECK_VECTOR].Base2 = ((uint64_t)CpuAlignmentCheck >> 16) & 0xffff;
	Desc[CPU_ALIGNMENT_CHECK_VECTOR].Base3 = (uint64_t)CpuAlignmentCheck >> 32;

	Desc[CPU_MACHINE_CHECK_VECTOR].Base1 = (uint64_t)CpuMachineCheck & 0xffff;
	Desc[CPU_MACHINE_CHECK_VECTOR].Base2 = ((uint64_t)CpuMachineCheck >> 16) & 0xffff;
	Desc[CPU_MACHINE_CHECK_VECTOR].Base3 = (uint64_t)CpuMachineCheck >> 32;

	Desc[CPU_SIMD_VECTOR].Base1 = (uint64_t)CpuSIMD & 0xffff;
	Desc[CPU_SIMD_VECTOR].Base2 = ((uint64_t)CpuSIMD >> 16) & 0xffff;
	Desc[CPU_SIMD_VECTOR].Base3 = (uint64_t)CpuSIMD >> 32;

}

extern "C" bool CpuDispatchException( SExceptionRecord *ExceptionRecord, SContext *Context ) {

	if ( ExceptionRecord->ExceptionCode == ERR_DIVISION_BY_ZERO ) {

		// this is special case, we should test here for overflow instead of
		// division by zero

	}

	if ( ExceptionRecord->ExceptionCode == ERR_ACCESS_VIOLATION ) {

		// this is special case, it can be stack overflow instead of
		// not mapped memory

	}

	return DispatchException( ExceptionRecord, Context );

}
