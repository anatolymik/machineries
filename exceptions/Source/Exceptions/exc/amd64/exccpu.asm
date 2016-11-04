.code

include basic.inc
include excdsptch.inc

;
;
; External functions
;
;

extern  CpuDispatchException:proc

;
;
; Variables
;
;

g_CpuSimdMxCsr	DWORD		(CPU_SIMD_INVALID_OPERATION_MASK or		\
							 CPU_SIMD_DENORMAL_OPERATION_MASK or	\
							 CPU_SIMD_DIVIDE_BY_ZERO_MASK or		\
							 CPU_SIMD_OVERFLOW_MASK or				\
							 CPU_SIMD_UNDERFLOW_MASK or				\
							 CPU_SIMD_PRECISION_MASK)

;
;
; Structures
;
;

SMachineFrame struct

	Rip_		QWORD	?
	Cs_			QWORD	?
	RFlags_		QWORD	?
	Rsp_		QWORD	?
	Ss_			QWORD	?

SMachineFrame ends

SMachineFrameWithError struct

	ErrorCode_	QWORD	?
	Rip_		QWORD	?
	Cs_			QWORD	?
	RFlags_		QWORD	?
	Rsp_		QWORD	?
	Ss_			QWORD	?

SMachineFrameWithError ends

SGprFrame struct

	R11_		QWORD	?
	R10_		QWORD	?
	R9_			QWORD	?
	R8_			QWORD	?
	Rdx_		QWORD	?
	Rcx_		QWORD	?
	Rax_		QWORD	?
	Rbp_		QWORD	?

SGprFrame ends

SXmmFrame struct

	Xmm0_		SXmm	<>
	Xmm1_		SXmm	<>
	Xmm2_		SXmm	<>
	Xmm3_		SXmm	<>
	Xmm4_		SXmm	<>
	Xmm5_		SXmm	<>

	MxCsr		DWORD	?
	Align0		DWORD	?
	Align1		QWORD	?

SXmmFrame ends

;
;
; Macro
;
;

CpuGenerateHandlerEntry macro Name, ErrorCode

Name proc frame

	ifb <ErrorCode>
		@catstr( <Name>, <FrameSize> )			equ		(size SExceptionRecord + 8 + size SXmmFrame + 8)
		@catstr( <Name>, <ExceptionRecord> )	equ		(rbp)
		@catstr( <Name>, <XmmFrame> )			equ		(@catstr( <Name>, <ExceptionRecord> ) + size SExceptionRecord + 8)
		@catstr( <Name>, <XmmFrameOff> )		equ		(size SExceptionRecord + 8)
		@catstr( <Name>, <GprFrame> )			equ		(@catstr( <Name>, <XmmFrame> ) + size SXmmFrame + 8)
		@catstr( <Name>, <MachineFrame> )		equ		(@catstr( <Name>, <GprFrame> ) + (size SGprFrame))

		.pushframe
	else
		@catstr( <Name>, <FrameSize> )			equ		(size SExceptionRecord + 8 + size SXmmFrame)
		@catstr( <Name>, <ExceptionRecord> )	equ		(rbp)
		@catstr( <Name>, <XmmFrame> )			equ		(@catstr( <Name>, <ExceptionRecord> ) + size SExceptionRecord + 8)
		@catstr( <Name>, <XmmFrameOff> )		equ		(size SExceptionRecord + 8)
		@catstr( <Name>, <GprFrame> )			equ		(@catstr( <Name>, <XmmFrame> ) + size SXmmFrame)
		@catstr( <Name>, <ErrorCode> )			equ		(@catstr( <Name>, <GprFrame> ) + (size SGprFrame))
		@catstr( <Name>, <MachineFrame> )		equ		(@catstr( <Name>, <ErrorCode> ) + 8)

		.pushframe	code
	endif

	push	rbp
	.pushreg	rbp

	push	rax
	.pushreg	rax

	push	rcx
	.pushreg	rcx

	push	rdx
	.pushreg	rdx

	push	r8
	.pushreg	r8

	push	r9
	.pushreg	r9

	push	r10
	.pushreg	r10

	push	r11
	.pushreg	r11

	sub		rsp, @catstr( <Name>, <FrameSize> )
	.allocstack @catstr( <Name>, <FrameSize> )

	mov		rbp, rsp
	.setframe	rbp, 0

	movdqa	[@catstr( <Name>, <XmmFrame> ).SXmmFrame.Xmm0_], xmm0
	.savexmm128	xmm0, @catstr( <Name>, <XmmFrameOff> ) + SXmmFrame.Xmm0_

	movdqa	[@catstr( <Name>, <XmmFrame> ).SXmmFrame.Xmm1_], xmm1
	.savexmm128	xmm1, @catstr( <Name>, <XmmFrameOff> ) + SXmmFrame.Xmm1_

	movdqa	[@catstr( <Name>, <XmmFrame> ).SXmmFrame.Xmm2_], xmm2
	.savexmm128	xmm2, @catstr( <Name>, <XmmFrameOff> ) + SXmmFrame.Xmm2_

	movdqa	[@catstr( <Name>, <XmmFrame> ).SXmmFrame.Xmm3_], xmm3
	.savexmm128	xmm3, @catstr( <Name>, <XmmFrameOff> ) + SXmmFrame.Xmm3_

	movdqa	[@catstr( <Name>, <XmmFrame> ).SXmmFrame.Xmm4_], xmm4
	.savexmm128	xmm4, @catstr( <Name>, <XmmFrameOff> ) + SXmmFrame.Xmm4_

	movdqa	[@catstr( <Name>, <XmmFrame> ).SXmmFrame.Xmm5_], xmm5
	.savexmm128	xmm5, @catstr( <Name>, <XmmFrameOff> ) + SXmmFrame.Xmm5_

	.endprolog

	; clear direction flag
	cld

	; save simd status and control and load default
	stmxcsr	[@catstr( <Name>, <XmmFrame> ).SXmmFrame.MxCsr]
	ldmxcsr	[g_CpuSimdMxCsr]

	; initialize exception record
	mov		rax, [@catstr( <Name>, <MachineFrame> ).SMachineFrame.Rip_]
	mov		[@catstr( <Name>, <ExceptionRecord> ).SExceptionRecord.ExceptionAddress], rax

endm

CpuGenerateHandlerExit macro Name, ErrorCode

	; restore MxCsr
	ldmxcsr	[@catstr( <Name>, <XmmFrame> ).SXmmFrame.MxCsr]

	movdqa	xmm5, [@catstr( <Name>, <XmmFrame> ).SXmmFrame.Xmm5_]
	movdqa	xmm4, [@catstr( <Name>, <XmmFrame> ).SXmmFrame.Xmm4_]
	movdqa	xmm3, [@catstr( <Name>, <XmmFrame> ).SXmmFrame.Xmm3_]
	movdqa	xmm2, [@catstr( <Name>, <XmmFrame> ).SXmmFrame.Xmm2_]
	movdqa	xmm1, [@catstr( <Name>, <XmmFrame> ).SXmmFrame.Xmm1_]
	movdqa	xmm0, [@catstr( <Name>, <XmmFrame> ).SXmmFrame.Xmm0_]
	lea		rsp, [rbp+@catstr( <Name>, <FrameSize> )]
	pop		r11
	pop		r10
	pop		r9
	pop		r8
	pop		rdx
	pop		rcx
	pop		rax
	pop		rbp
	ifnb <ErrorCode>
		add	rsp, 8
	endif
	iretq

Name endp

endm

;
; CpuDivideError
;
; Entry point to divide error handler
;
CpuGenerateHandlerEntry	<CpuDivideError>, <>

	; initialize exception record
	mov		[CpuDivideErrorExceptionRecord.SExceptionRecord.ExceptionCode], ERR_DIVISION_BY_ZERO
	mov		[CpuDivideErrorExceptionRecord.SExceptionRecord.ExceptionFlags], 0
	mov		[CpuDivideErrorExceptionRecord.SExceptionRecord.ExceptionRecord], 0
	mov		[CpuDivideErrorExceptionRecord.SExceptionRecord.NumberParameters], 0

	; process exception
	lea		rcx, [CpuDivideErrorGprFrame]
	lea		rdx, [CpuDivideErrorXmmFrame]
	lea		rax, [CpuDivideErrorMachineFrame]
	lea		r8, [CpuDivideErrorExceptionRecord]
	call	CpuProcessException

CpuGenerateHandlerExit	<CpuDivideError>, <>

;
; CpuDebug
;
; Entry point to debug exception handler
;
CpuGenerateHandlerEntry	<CpuDebug>, <>

	; special debugger case, should be changed for something adequate
	jmp		$

CpuGenerateHandlerExit	<CpuDebug>, <>

;
; CpuNmi
;
; Entry point to NMI handler
;
CpuGenerateHandlerEntry	<CpuNmi>, <>

	; NMI, should be changed for something adequate
	jmp		$

CpuGenerateHandlerExit	<CpuNmi>, <>

;
; CpuBreakpoint
;
; Entry point to breakpoint exception handler
;
CpuGenerateHandlerEntry	<CpuBreakpoint>, <>

	; special debugger case, should be changed for something adequate
	jmp		$

CpuGenerateHandlerExit	<CpuBreakpoint>, <>

;
; CpuIntegerOverflow
;
; Entry point to integer overflow exception handler
;
CpuGenerateHandlerEntry	<CpuIntegerOverflow>, <>

	; initialize exception record
	mov		[CpuIntegerOverflowExceptionRecord.SExceptionRecord.ExceptionCode], ERR_INTEGER_OVERFLOW
	mov		[CpuIntegerOverflowExceptionRecord.SExceptionRecord.ExceptionFlags], 0
	mov		[CpuIntegerOverflowExceptionRecord.SExceptionRecord.ExceptionRecord], 0
	mov		[CpuIntegerOverflowExceptionRecord.SExceptionRecord.NumberParameters], 0

	; process exception
	lea		rcx, [CpuIntegerOverflowGprFrame]
	lea		rdx, [CpuIntegerOverflowXmmFrame]
	lea		rax, [CpuIntegerOverflowMachineFrame]
	lea		r8, [CpuIntegerOverflowExceptionRecord]
	call	CpuProcessException

CpuGenerateHandlerExit	<CpuIntegerOverflow>, <>

;
; CpuBoundRangeExceeded
;
; Entry point to bound range exception handler
;
CpuGenerateHandlerEntry	<CpuBoundRangeExceeded>, <>

	; initialize exception record
	mov		[CpuBoundRangeExceededExceptionRecord.SExceptionRecord.ExceptionCode], ERR_ARRAY_BOUNDS_EXCEEDED
	mov		[CpuBoundRangeExceededExceptionRecord.SExceptionRecord.ExceptionFlags], 0
	mov		[CpuBoundRangeExceededExceptionRecord.SExceptionRecord.ExceptionRecord], 0
	mov		[CpuBoundRangeExceededExceptionRecord.SExceptionRecord.NumberParameters], 0

	; process exception
	lea		rcx, [CpuBoundRangeExceededGprFrame]
	lea		rdx, [CpuBoundRangeExceededXmmFrame]
	lea		rax, [CpuBoundRangeExceededMachineFrame]
	lea		r8, [CpuBoundRangeExceededExceptionRecord]
	call	CpuProcessException

CpuGenerateHandlerExit	<CpuBoundRangeExceeded>, <>

;
; CpuInvalidOpcode
;
; Entry point to invalid opcode exception handler
;
CpuGenerateHandlerEntry	<CpuInvalidOpcode>, <>

	; initialize exception record
	mov		[CpuInvalidOpcodeExceptionRecord.SExceptionRecord.ExceptionCode], ERR_ILLEGAL_INSTRUCTION
	mov		[CpuInvalidOpcodeExceptionRecord.SExceptionRecord.ExceptionFlags], 0
	mov		[CpuInvalidOpcodeExceptionRecord.SExceptionRecord.ExceptionRecord], 0
	mov		[CpuInvalidOpcodeExceptionRecord.SExceptionRecord.NumberParameters], 0

	; process exception
	lea		rcx, [CpuInvalidOpcodeGprFrame]
	lea		rdx, [CpuInvalidOpcodeXmmFrame]
	lea		rax, [CpuInvalidOpcodeMachineFrame]
	lea		r8, [CpuInvalidOpcodeExceptionRecord]
	call	CpuProcessException

CpuGenerateHandlerExit	<CpuInvalidOpcode>, <>

;
; CpuDeviceNotAvailable
;
; Entry point to device not available exception handler
;
CpuGenerateHandlerEntry	<CpuDeviceNotAvailable>, <>

	; allocate space for home region
	sub		rsp, 5*8

	; show error, this exception should never happen
	mov		rax, [CpuDeviceNotAvailableMachineFrame.SMachineFrame.Rip_]
	mov		[rsp+4*8], rax
	mov		r9, cr4
	mov		r8, cr0
	mov		rdx, CPU_DEVICE_NOT_AVAILABLE_VECTOR
	mov		rcx, ERR_UNEXCPECTED_EXCEPTION
	call	CpuFatalError

CpuGenerateHandlerExit	<CpuDeviceNotAvailable>, <>

;
; CpuDoubleFault
;
; Entry point to double fault exception handler
;
CpuGenerateHandlerEntry	<CpuDoubleFault>, <ErrorCode>

	; allocate space for home region
	sub		rsp, 5*8

	; show error, this exception should never happen
	mov		rax, [CpuDoubleFaultMachineFrame.SMachineFrame.Rip_]
	mov		[rsp+4*8], rax
	mov		r9, cr4
	mov		r8, cr0
	mov		rdx, CPU_DOUBLE_FAULT_VECTOR
	mov		rcx, ERR_UNEXCPECTED_EXCEPTION
	call	CpuFatalError

CpuGenerateHandlerExit	<CpuDoubleFault>, <ErrorCode>

;
; CpuCoprocessorSegmentOverrun
;
; Entry point to coprocessor segment overrun exception handler
;
CpuGenerateHandlerEntry	<CpuCoprocessorSegmentOverrun>, <>

	; allocate space for home region
	sub		rsp, 5*8

	; show error, this exception should never happen
	mov		rax, [CpuCoprocessorSegmentOverrunMachineFrame.SMachineFrame.Rip_]
	mov		[rsp+4*8], rax
	mov		r9, cr4
	mov		r8, cr0
	mov		rdx, CPU_COPROCESSOR_SEGMENT_OVERRUN_VECTOR
	mov		rcx, ERR_UNEXCPECTED_EXCEPTION
	call	CpuFatalError

CpuGenerateHandlerExit	<CpuCoprocessorSegmentOverrun>, <>

;
; CpuInvalidTss
;
; Entry point to invalid TSS exception handler
;
CpuGenerateHandlerEntry	<CpuInvalidTss>, <ErrorCode>

	; allocate space for home region
	sub		rsp, 5*8

	; show error, this exception should never happen
	mov		rax, [CpuInvalidTssMachineFrame.SMachineFrame.Rip_]
	mov		[rsp+4*8], rax
	mov		r9d, [CpuInvalidTssErrorCode]
	mov		r8, cr0
	mov		rdx, CPU_INVALID_TSS_VECTOR
	mov		rcx, ERR_UNEXCPECTED_EXCEPTION
	call	CpuFatalError

CpuGenerateHandlerExit	<CpuInvalidTss>, <ErrorCode>

;
; CpuSegmentNotPresent
;
; Entry point to segment not present exception handler
;
CpuGenerateHandlerEntry	<CpuSegmentNotPresent>, <ErrorCode>

	; allocate space for home region
	sub		rsp, 5*8

	; show error, this exception should never happen
	mov		rax, [CpuSegmentNotPresentMachineFrame.SMachineFrame.Rip_]
	mov		[rsp+4*8], rax
	mov		r9d, [CpuSegmentNotPresentErrorCode]
	mov		r8, cr0
	mov		rdx, CPU_SEGMENT_NOT_PRESENT_VECTOR
	mov		rcx, ERR_UNEXCPECTED_EXCEPTION
	call	CpuFatalError

CpuGenerateHandlerExit	<CpuSegmentNotPresent>, <ErrorCode>

;
; CpuStackFault
;
; Entry point to segment not present exception handler
;
CpuGenerateHandlerEntry	<CpuStackFault>, <ErrorCode>

	; allocate space for home region
	sub		rsp, 5*8

	; show error, this exception should never happen
	mov		rax, [CpuStackFaultMachineFrame.SMachineFrame.Rip_]
	mov		[rsp+4*8], rax
	mov		r9d, [CpuStackFaultErrorCode]
	mov		r8, cr0
	mov		rdx, CPU_STACK_FAULT_VECTOR
	mov		rcx, ERR_UNEXCPECTED_EXCEPTION
	call	CpuFatalError

CpuGenerateHandlerExit	<CpuStackFault>, <ErrorCode>

;
; CpuGeneralProtection
;
; Entry point to general protection exception handler
;
CpuGenerateHandlerEntry	<CpuGeneralProtection>, <ErrorCode>

	; initialize exception record, set error as if it was read access on unknown address
	mov		[CpuGeneralProtectionExceptionRecord.SExceptionRecord.ExceptionCode], ERR_ACCESS_VIOLATION
	mov		[CpuGeneralProtectionExceptionRecord.SExceptionRecord.ExceptionFlags], 0
	mov		[CpuGeneralProtectionExceptionRecord.SExceptionRecord.ExceptionRecord], 0
	mov		[CpuGeneralProtectionExceptionRecord.SExceptionRecord.NumberParameters], 2
	mov		[CpuGeneralProtectionExceptionRecord.SExceptionRecord.ExceptionInformation+0], 0
	xor		rax, rax
	dec		rax
	mov		[CpuGeneralProtectionExceptionRecord.SExceptionRecord.ExceptionInformation+8], rax

	; process exception
	lea		rcx, [CpuGeneralProtectionGprFrame]
	lea		rdx, [CpuGeneralProtectionXmmFrame]
	lea		rax, [CpuGeneralProtectionMachineFrame]
	lea		r8, [CpuGeneralProtectionExceptionRecord]
	call	CpuProcessException

CpuGenerateHandlerExit	<CpuGeneralProtection>, <ErrorCode>

;
; CpuPageFault
;
; Entry point to page fault handler
;
CpuGenerateHandlerEntry	<CpuPageFault>, <ErrorCode>

	; read error code and detect access type executed
	mov		eax, [CpuPageFaultErrorCode]

	; reserved bits set is unexcpected
	test	eax, CPU_PAGE_FAULT_ERROR_RSVD
	jz		CpuPageFaultNoReservedBitsSet

	; allocate space for home region
	sub		rsp, 5*8

	mov		r9d, eax
	mov		rax, [CpuPageFaultMachineFrame.SMachineFrame.Rip_]
	mov		[rsp+4*8], rax
	mov		r8, cr2
	mov		rdx, CPU_PAGE_FAULT_VECTOR
	mov		rcx, ERR_UNEXCPECTED_EXCEPTION
	call	CpuFatalError

CpuPageFaultNoReservedBitsSet:

	; initialize exception record
	mov		[CpuPageFaultExceptionRecord.SExceptionRecord.ExceptionCode], ERR_ACCESS_VIOLATION
	mov		[CpuPageFaultExceptionRecord.SExceptionRecord.ExceptionFlags], 0
	mov		[CpuPageFaultExceptionRecord.SExceptionRecord.ExceptionRecord], 0
	mov		[CpuPageFaultExceptionRecord.SExceptionRecord.NumberParameters], 2

	mov		ecx, EXCEPTION_FLAGS_EXECUTE_FAULT
	test	eax, CPU_PAGE_FAULT_ERROR_ID
	jnz		CpuPageFaultInitializeParameters

	mov		ecx, EXCEPTION_FLAGS_READ_FAULT
	test	eax, CPU_PAGE_FAULT_ERROR_WR
	jz		CpuPageFaultInitializeParameters

	mov		ecx, EXCEPTION_FLAGS_WRITE_FAULT

CpuPageFaultInitializeParameters:

	mov		[CpuPageFaultExceptionRecord.SExceptionRecord.ExceptionInformation+0], rcx
	mov		rcx, cr2
	mov		[CpuPageFaultExceptionRecord.SExceptionRecord.ExceptionInformation+8], rcx

	; process exception
	lea		rcx, [CpuPageFaultGprFrame]
	lea		rdx, [CpuPageFaultXmmFrame]
	lea		rax, [CpuPageFaultMachineFrame]
	lea		r8, [CpuPageFaultExceptionRecord]
	call	CpuProcessException

CpuGenerateHandlerExit	<CpuPageFault>, <ErrorCode>

;
; CpuFpuError
;
; Entry point to fpu error exception handler
;
CpuGenerateHandlerEntry	<CpuFpuError>, <>

	; allocate space for home region
	sub		rsp, 5*8

	; show error, this exception should never happen
	mov		rax, [CpuFpuErrorMachineFrame.SMachineFrame.Rip_]
	mov		[rsp+4*8], rax
	mov		r9, cr4
	mov		r8, cr0
	mov		rdx, CPU_FPU_ERROR_VECTOR
	mov		rcx, ERR_UNEXCPECTED_EXCEPTION
	call	CpuFatalError

CpuGenerateHandlerExit	<CpuFpuError>, <>

;
; CpuAlignmentCheck
;
; Entry point to alignment check exception handler
;
CpuGenerateHandlerEntry	<CpuAlignmentCheck>, <ErrorCode>

	; allocate space for home region
	sub		rsp, 5*8

	; show error, this exception should never happen
	mov		rax, [CpuAlignmentCheckMachineFrame.SMachineFrame.Rip_]
	mov		[rsp+4*8], rax
	mov		r9, cr4
	mov		r8, cr0
	mov		rdx, CPU_ALIGNMENT_CHECK_VECTOR
	mov		rcx, ERR_UNEXCPECTED_EXCEPTION
	call	CpuFatalError

CpuGenerateHandlerExit	<CpuAlignmentCheck>, <ErrorCode>

;
; CpuMachineCheck
;
; Entry point to machine check exception handler
;
CpuGenerateHandlerEntry	<CpuMachineCheck>, <>

	; allocate space for home region
	sub		rsp, 5*8

	; show error, this exception normally would never happen
	xor		rax, rax
	mov		[rsp+4*8], rax
	mov		r9, rax
	mov		r8, rax
	mov		rdx, rax
	mov		rcx, ERR_MACHINE_FAILURE
	call	CpuFatalError

CpuGenerateHandlerExit	<CpuMachineCheck>, <>

;
; CpuSIMD
;
; Entry point to SIMD exception handler
;
CpuGenerateHandlerEntry	<CpuSIMD>, <>

	; get status and mask bits
	mov		ax, word ptr[CpuSIMDXmmFrame.SXmmFrame.MxCsr]
	mov		cx, ax
	and		ax, 03fh
	shr		cx, 7
	and		cx, 03fh

	; get unmasked exceptions and detect
	; those that was generated
	not		cx
	and		cx, 03fh
	and		cx, ax

	; check if an exception was valid
	test	cx, cx
	jnz		CpuSIMDValidException

	; allocate space for home region
	sub		rsp, 5*8

	; show error, this exception normally would never happen
	mov		rax, [CpuSIMDMachineFrame.SMachineFrame.Rip_]
	mov		[rsp+4*8], rax
	mov		r9, cr4
	mov		r8, cr0
	mov		rdx, CPU_SIMD_VECTOR
	mov		rcx, ERR_UNEXCPECTED_EXCEPTION
	call	CpuFatalError

CpuSIMDValidException:

	mov		eax, ERR_FLOAT_INVALID_OPERATION
	test	cx, CPU_SIMD_INVALID_OPERATION
	jnz		CpuSIMDValidInitializeRecord

	mov		eax, ERR_FLOAT_DIVIDE_BY_ZERO
	test	cx, CPU_SIMD_ZERO_DIVIDE
	jnz		CpuSIMDValidInitializeRecord

	mov		eax, ERR_FLOAT_INVALID_OPERATION
	test	cx, CPU_SIMD_DENORMAL
	jnz		CpuSIMDValidInitializeRecord

	mov		eax, ERR_FLOAT_OVERFLOW
	test	cx, CPU_SIMD_OVERFLOW
	jnz		CpuSIMDValidInitializeRecord

	mov		eax, ERR_FLOAT_UNDERFLOW
	test	cx, CPU_SIMD_UNDERFLOW
	jnz		CpuSIMDValidInitializeRecord

	mov		eax, ERR_FLOAT_INEXACT_RESULT

CpuSIMDValidInitializeRecord:

	; initialize exception record
	mov		[CpuSIMDExceptionRecord.SExceptionRecord.ExceptionCode], eax
	mov		[CpuSIMDExceptionRecord.SExceptionRecord.ExceptionFlags], 0
	mov		[CpuSIMDExceptionRecord.SExceptionRecord.ExceptionRecord], 0
	mov		[CpuSIMDExceptionRecord.SExceptionRecord.NumberParameters], 2
	xor		rcx, rcx
	mov		[CpuSIMDExceptionRecord.SExceptionRecord.ExceptionInformation+0], rcx
	mov		cx, word ptr[CpuSIMDXmmFrame.SXmmFrame.MxCsr]
	mov		[CpuSIMDExceptionRecord.SExceptionRecord.ExceptionInformation+8], rcx

	; process exception
	lea		rcx, [CpuSIMDGprFrame]
	lea		rdx, [CpuSIMDXmmFrame]
	lea		rax, [CpuSIMDMachineFrame]
	lea		r8, [CpuSIMDExceptionRecord]
	call	CpuProcessException

CpuGenerateHandlerExit	<CpuSIMD>, <>

;
; CpuUnexpectedException
;
; Entry point to default exception handler
;
CpuGenerateHandlerEntry	<CpuUnexpectedException>, <>

	; allocate space for home region
	sub		rsp, 5*8

	; show error, this exception should never happen
	xor		rax, rax
	mov		[rsp+4*8], rax
	mov		r9, rax
	mov		r8, rax
	mov		rdx, rax
	dec		rdx
	mov		rcx, ERR_UNEXCPECTED_EXCEPTION
	call	CpuFatalError

CpuGenerateHandlerExit	<CpuUnexpectedException>, <>

;
; CpuProcessException
;
; arguments:
; rax - (SMachineFrame*)MachineFrame
; rcx - (SGprFrame*)GprFrame
; rdx - (SXmmFrame*)XmmFrame
; r8 - (SExceptionRecord*)ExceptionRecord
;
CpuProcessException proc frame

	CpuProcessExceptionFrameSize			equ		((6*8) + (4*8) + size SContext + 8)
	CpuProcessExceptionHome					equ		(rsp)
	CpuProcessExceptionArguments			equ		(CpuProcessExceptionHome + (6*8))
	CpuProcessExceptionContext				equ		(CpuProcessExceptionArguments + (4*8))

	sub		rsp, CpuProcessExceptionFrameSize
	.allocstack CpuProcessExceptionFrameSize

	.endprolog

	; store arguments
	mov		[CpuProcessExceptionArguments+(0*8)], rax
	mov		[CpuProcessExceptionArguments+(1*8)], rcx
	mov		[CpuProcessExceptionArguments+(2*8)], rdx
	mov		[CpuProcessExceptionArguments+(3*8)], r8

	; initialize exception context
	mov		r11, [rcx.SGprFrame.Rax_]
	mov		[CpuProcessExceptionContext.SContext.Rax_], r11
	mov		r11, [rcx.SGprFrame.Rcx_]
	mov		[CpuProcessExceptionContext.SContext.Rcx_], r11
	mov		r11, [rcx.SGprFrame.Rdx_]
	mov		[CpuProcessExceptionContext.SContext.Rdx_], r11
	mov		[CpuProcessExceptionContext.SContext.Rbx_], rbx

	mov		r11, [rax.SMachineFrame.Rsp_]
	mov		[CpuProcessExceptionContext.SContext.Rsp_], r11
	mov		r11, [rcx.SGprFrame.Rbp_]
	mov		[CpuProcessExceptionContext.SContext.Rbp_], r11

	mov		[CpuProcessExceptionContext.SContext.Rsi_], rsi
	mov		[CpuProcessExceptionContext.SContext.Rdi_], rdi

	mov		r11, [rcx.SGprFrame.R8_]
	mov		[CpuProcessExceptionContext.SContext.R8_], r11
	mov		r11, [rcx.SGprFrame.R9_]
	mov		[CpuProcessExceptionContext.SContext.R9_], r11
	mov		r11, [rcx.SGprFrame.R10_]
	mov		[CpuProcessExceptionContext.SContext.R10_], r11
	mov		r11, [rcx.SGprFrame.R11_]
	mov		[CpuProcessExceptionContext.SContext.R11_], r11

	mov		[CpuProcessExceptionContext.SContext.R12_], r12
	mov		[CpuProcessExceptionContext.SContext.R13_], r13
	mov		[CpuProcessExceptionContext.SContext.R14_], r14
	mov		[CpuProcessExceptionContext.SContext.R15_], r15

	mov		r11, [rax.SMachineFrame.Rip_]
	mov		[CpuProcessExceptionContext.SContext.Rip_], r11

	mov		r11, [rax.SMachineFrame.RFlags_]
	mov		[CpuProcessExceptionContext.SContext.EFlags], r11d

	; initialize exception segment registers
	mov		r11, [rax.SMachineFrame.Cs_]
	mov		[CpuProcessExceptionContext.SContext.SegCs], r11w

	mov		[CpuProcessExceptionContext.SContext.SegDs], ds
	mov		[CpuProcessExceptionContext.SContext.SegEs], es
	mov		[CpuProcessExceptionContext.SContext.SegFs], fs
	mov		[CpuProcessExceptionContext.SContext.SegGs], gs

	mov		r11, [rax.SMachineFrame.Ss_]
	mov		[CpuProcessExceptionContext.SContext.SegSs], r11w

	; initialize exception xmm registers
	movdqa	[CpuProcessExceptionContext.SContext.XmmContext_.XmmRegs_.Xmm0_], xmm0
	movdqa	[CpuProcessExceptionContext.SContext.XmmContext_.XmmRegs_.Xmm1_], xmm1
	movdqa	[CpuProcessExceptionContext.SContext.XmmContext_.XmmRegs_.Xmm2_], xmm2
	movdqa	[CpuProcessExceptionContext.SContext.XmmContext_.XmmRegs_.Xmm3_], xmm3
	movdqa	[CpuProcessExceptionContext.SContext.XmmContext_.XmmRegs_.Xmm4_], xmm4
	movdqa	[CpuProcessExceptionContext.SContext.XmmContext_.XmmRegs_.Xmm5_], xmm5
	movdqa	[CpuProcessExceptionContext.SContext.XmmContext_.XmmRegs_.Xmm6_], xmm6
	movdqa	[CpuProcessExceptionContext.SContext.XmmContext_.XmmRegs_.Xmm7_], xmm7
	movdqa	[CpuProcessExceptionContext.SContext.XmmContext_.XmmRegs_.Xmm8_], xmm8
	movdqa	[CpuProcessExceptionContext.SContext.XmmContext_.XmmRegs_.Xmm9_], xmm9
	movdqa	[CpuProcessExceptionContext.SContext.XmmContext_.XmmRegs_.Xmm10_], xmm10
	movdqa	[CpuProcessExceptionContext.SContext.XmmContext_.XmmRegs_.Xmm11_], xmm11
	movdqa	[CpuProcessExceptionContext.SContext.XmmContext_.XmmRegs_.Xmm12_], xmm12
	movdqa	[CpuProcessExceptionContext.SContext.XmmContext_.XmmRegs_.Xmm13_], xmm13
	movdqa	[CpuProcessExceptionContext.SContext.XmmContext_.XmmRegs_.Xmm14_], xmm14
	movdqa	[CpuProcessExceptionContext.SContext.XmmContext_.XmmRegs_.Xmm15_], xmm15

	; initialize xmm state
	mov		r11d, [rdx.SXmmFrame.MxCsr]
	mov		[CpuProcessExceptionContext.SContext.MxCsr], r11d

	; initialize context flags
	mov		[CpuProcessExceptionContext.SContext.ContextFlags], CONTEXT_FULL or CONTEXT_SEGMENTS

	; call exception dispatcher
	lea		rdx, [CpuProcessExceptionContext]
	mov		rcx, r8
	call	CpuDispatchException

	; test if we have to resume execution
	test	al, al
	jnz		CpuProcessExceptionResume

	; exception wasn't handled, stop system here
	mov		qword ptr[CpuProcessExceptionHome+(4*8)], 0
	xor		r9, r9
	xor		r8, r8
	xor		rdx, rdx
	xor		rcx, rcx
	call	CpuFatalError

CpuProcessExceptionResume:

	; restore arguments
	mov		rax, [CpuProcessExceptionArguments+(0*8)]
	mov		rcx, [CpuProcessExceptionArguments+(1*8)]
	mov		rdx, [CpuProcessExceptionArguments+(2*8)]
	mov		r8, [CpuProcessExceptionArguments+(3*8)]

	; restore control
	test	[CpuProcessExceptionContext.SContext.ContextFlags], CONTEXT_CONTROL
	jz		CpuProcessExceptionSkipRestoreControl

	; Ss, Rsp, Cs, Rip, EFlags

	xor		r11, r11

	mov		r11w, [CpuProcessExceptionContext.SContext.SegSs]
	mov		[rax.SMachineFrame.Ss_], r11

	mov		r11w, [CpuProcessExceptionContext.SContext.SegCs]
	mov		[rax.SMachineFrame.Cs_], r11

	mov		r11, [CpuProcessExceptionContext.SContext.Rsp_]
	mov		[rax.SMachineFrame.Rsp_], r11

	mov		r11, [CpuProcessExceptionContext.SContext.Rip_]
	mov		[rax.SMachineFrame.Rip_], r11

	mov		r11d, [CpuProcessExceptionContext.SContext.EFlags]
	shl		r11, 32
	shr		r11, 32
	mov		[rax.SMachineFrame.RFlags_], r11

CpuProcessExceptionSkipRestoreControl:

	; restore integer
	test	[CpuProcessExceptionContext.SContext.ContextFlags], CONTEXT_INTEGER
	jz		CpuProcessExceptionSkipRestoreInteger

	; Rax, Rcx, Rdx, Rbx, Rbp, Rsi, Rdi, R8-R15

	mov		r11, [CpuProcessExceptionContext.SContext.Rax_]
	mov		[rcx.SGprFrame.Rax_], r11
	mov		r11, [CpuProcessExceptionContext.SContext.Rcx_]
	mov		[rcx.SGprFrame.Rcx_], r11
	mov		r11, [CpuProcessExceptionContext.SContext.Rdx_]
	mov		[rcx.SGprFrame.Rdx_], r11
	mov		rbx, [CpuProcessExceptionContext.SContext.Rbx_]

	mov		r11, [CpuProcessExceptionContext.SContext.Rbp_]
	mov		[rcx.SGprFrame.Rbp_], r11

	mov		rsi, [CpuProcessExceptionContext.SContext.Rsi_]
	mov		rdi, [CpuProcessExceptionContext.SContext.Rdi_]

	mov		r11, [CpuProcessExceptionContext.SContext.R8_]
	mov		[rcx.SGprFrame.R8_], r11
	mov		r11, [CpuProcessExceptionContext.SContext.R9_]
	mov		[rcx.SGprFrame.R9_], r11
	mov		r11, [CpuProcessExceptionContext.SContext.R10_]
	mov		[rcx.SGprFrame.R10_], r11
	mov		r11, [CpuProcessExceptionContext.SContext.R11_]
	mov		[rcx.SGprFrame.R11_], r11

	mov		r12, [CpuProcessExceptionContext.SContext.R12_]
	mov		r13, [CpuProcessExceptionContext.SContext.R13_]
	mov		r14, [CpuProcessExceptionContext.SContext.R14_]
	mov		r15, [CpuProcessExceptionContext.SContext.R15_]

CpuProcessExceptionSkipRestoreInteger:

	; restore segments
	test	[CpuProcessExceptionContext.SContext.ContextFlags], CONTEXT_SEGMENTS
	jz		CpuProcessExceptionSkipRestoreSegments

	; ds, es, fs, gs

	mov		ds, [CpuProcessExceptionContext.SContext.SegDs]
	mov		es, [CpuProcessExceptionContext.SContext.SegEs]
	mov		fs, [CpuProcessExceptionContext.SContext.SegFs]
	mov		gs, [CpuProcessExceptionContext.SContext.SegGs]

CpuProcessExceptionSkipRestoreSegments:

	; restore segments
	test	[CpuProcessExceptionContext.SContext.ContextFlags], CONTEXT_FLOATING_POINT
	jz		CpuProcessExceptionSkipFloatingPoint

	; xmm0 - xmm15

	movdqa	xmm0, [CpuProcessExceptionContext.SContext.XmmContext_.XmmRegs_.Xmm0_]
	movdqa	[rdx.SXmmFrame.Xmm0_], xmm0
	movdqa	xmm0, [CpuProcessExceptionContext.SContext.XmmContext_.XmmRegs_.Xmm1_]
	movdqa	[rdx.SXmmFrame.Xmm1_], xmm0
	movdqa	xmm0, [CpuProcessExceptionContext.SContext.XmmContext_.XmmRegs_.Xmm2_]
	movdqa	[rdx.SXmmFrame.Xmm2_], xmm0
	movdqa	xmm0, [CpuProcessExceptionContext.SContext.XmmContext_.XmmRegs_.Xmm3_]
	movdqa	[rdx.SXmmFrame.Xmm3_], xmm0
	movdqa	xmm0, [CpuProcessExceptionContext.SContext.XmmContext_.XmmRegs_.Xmm4_]
	movdqa	[rdx.SXmmFrame.Xmm4_], xmm0
	movdqa	xmm0, [CpuProcessExceptionContext.SContext.XmmContext_.XmmRegs_.Xmm5_]
	movdqa	[rdx.SXmmFrame.Xmm5_], xmm0

	movdqa	xmm6, [CpuProcessExceptionContext.SContext.XmmContext_.XmmRegs_.Xmm6_]
	movdqa	xmm7, [CpuProcessExceptionContext.SContext.XmmContext_.XmmRegs_.Xmm7_]
	movdqa	xmm8, [CpuProcessExceptionContext.SContext.XmmContext_.XmmRegs_.Xmm8_]
	movdqa	xmm9, [CpuProcessExceptionContext.SContext.XmmContext_.XmmRegs_.Xmm9_]
	movdqa	xmm10, [CpuProcessExceptionContext.SContext.XmmContext_.XmmRegs_.Xmm10_]
	movdqa	xmm11, [CpuProcessExceptionContext.SContext.XmmContext_.XmmRegs_.Xmm11_]
	movdqa	xmm12, [CpuProcessExceptionContext.SContext.XmmContext_.XmmRegs_.Xmm12_]
	movdqa	xmm13, [CpuProcessExceptionContext.SContext.XmmContext_.XmmRegs_.Xmm13_]
	movdqa	xmm14, [CpuProcessExceptionContext.SContext.XmmContext_.XmmRegs_.Xmm14_]
	movdqa	xmm15, [CpuProcessExceptionContext.SContext.XmmContext_.XmmRegs_.Xmm15_]

	; restore MxCsr
	mov		r11d, [CpuProcessExceptionContext.SContext.MxCsr]
	mov		[rdx.SXmmFrame.MxCsr], r11d

CpuProcessExceptionSkipFloatingPoint:

	add		rsp, CpuProcessExceptionFrameSize
	ret

CpuProcessException endp

;
; CpuFatalError
;
; Currently halts the system. Should be replace for something adequate.
;
; arguments:
; rcx - (uint32_t)Status
; rdx - (uintptr_t)Parameter1
; r8 - (uintptr_t)Parameter2
; r9 - (uintptr_t)Parameter3
; stack0 - (uintptr_t)Parameter4
;
CpuFatalError proc

	; should be changed for something adequate
	jmp		$

CpuFatalError endp

end
