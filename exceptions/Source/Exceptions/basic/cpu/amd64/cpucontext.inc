;
;
; Declarations
;
;

CONTEXT_AMD64					equ		00100000h

CONTEXT_CONTROL					equ		(CONTEXT_AMD64 or 00000001h)
CONTEXT_INTEGER					equ		(CONTEXT_AMD64 or 00000002h)
CONTEXT_SEGMENTS				equ		(CONTEXT_AMD64 or 00000004h)
CONTEXT_FLOATING_POINT			equ		(CONTEXT_AMD64 or 00000008h)
CONTEXT_DEBUG_REGISTERS			equ		(CONTEXT_AMD64 or 00000010h)

CONTEXT_FULL					equ		(CONTEXT_CONTROL or CONTEXT_INTEGER or CONTEXT_FLOATING_POINT)

CONTEXT_ALL						equ		(CONTEXT_CONTROL or CONTEXT_INTEGER or CONTEXT_SEGMENTS or CONTEXT_FLOATING_POINT or CONTEXT_DEBUG_REGISTERS)

CONTEXT_XSTATE					equ		(CONTEXT_AMD64 or 00000040h)

CONTEXT_EXCEPTION_ACTIVE		equ		08000000h
CONTEXT_SERVICE_ACTIVE			equ		10000000h
CONTEXT_EXCEPTION_REQUEST		equ		40000000h
CONTEXT_EXCEPTION_REPORTING		equ		80000000h

;
;
; Structures
;
;

SXmm struct
	Low_		QWORD	?
	High_		QWORD	?
SXmm ends

SFxSaveArea struct
	ControlWord			WORD	?
	StatusWord			WORD	?

	TagWord				BYTE	?
	Reserved1			BYTE	?

	ErrorOpcode			WORD	?
	ErrorOffset			DWORD	?
	ErrorSelector		WORD	?
	Reserved2			WORD	?

	DataOffset			DWORD	?
	DataSelector		WORD	?
	Reserved3			WORD	?

	MxCsr				DWORD	?
	MxCsr_Mask			DWORD	?

	FloatRegisters		SXmm	8 dup(<>)
	XmmRegisters		SXmm	16 dup(<>)

	Reserved4			BYTE	96 dup(?)
SFxSaveArea ends

SXmmRegs struc
	Header_					SXmm	2 dup (<>)
	Legacy_					SXmm	8 dup (<>)
	Xmm0_					SXmm	<>
	Xmm1_					SXmm	<>
	Xmm2_					SXmm	<>
	Xmm3_					SXmm	<>
	Xmm4_					SXmm	<>
	Xmm5_					SXmm	<>
	Xmm6_					SXmm	<>
	Xmm7_					SXmm	<>
	Xmm8_					SXmm	<>
	Xmm9_					SXmm	<>
	Xmm10_					SXmm	<>
	Xmm11_					SXmm	<>
	Xmm12_					SXmm	<>
	Xmm13_					SXmm	<>
	Xmm14_					SXmm	<>
	Xmm15_					SXmm	<>
SXmmRegs ends

SXmmContext union
	FltSave_				SFxSaveArea			<>
	XmmRegs_				SXmmRegs			<>
SXmmContext ends

SContext struct
	ContextFlags			DWORD	?

	MxCsr					DWORD	?

	SegCs					WORD	?
	SegDs					WORD	?
	SegEs					WORD	?
	SegFs					WORD	?
	SegGs					WORD	?
	SegSs					WORD	?
	EFlags					DWORD	?

	Dr0_					QWORD	?
	Dr1_					QWORD	?
	Dr2_					QWORD	?
	Dr3_					QWORD	?
	Dr6_					QWORD	?
	Dr7_					QWORD	?

	Rax_					QWORD	?
	Rcx_					QWORD	?
	Rdx_					QWORD	?
	Rbx_					QWORD	?
	Rsp_					QWORD	?
	Rbp_					QWORD	?
	Rsi_					QWORD	?
	Rdi_					QWORD	?
	R8_						QWORD	?
	R9_						QWORD	?
	R10_					QWORD	?
	R11_					QWORD	?
	R12_					QWORD	?
	R13_					QWORD	?
	R14_					QWORD	?
	R15_					QWORD	?
	Rip_					QWORD	?

	XmmContext_				SXmmContext	<>
SContext ends
