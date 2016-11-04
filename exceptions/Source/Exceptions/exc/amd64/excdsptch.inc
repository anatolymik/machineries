;
;
; Declarations
;
;

EXCEPTION_NONCONTINUABLE		equ		01h
EXCEPTION_UNWINDING				equ		02h
EXCEPTION_EXIT_UNWIND			equ		04h
EXCEPTION_STACK_INVALID			equ		08h
EXCEPTION_NESTED_CALL			equ		10h
EXCEPTION_TARGET_UNWIND			equ		20h
EXCEPTION_COLLIDED_UNWIND		equ		40h

EXCEPTION_UNWIND				equ		(EXCEPTION_UNWINDING or EXCEPTION_EXIT_UNWIND or EXCEPTION_TARGET_UNWIND or EXCEPTION_COLLIDED_UNWIND)

EXCEPTION_MAXIMUM_PARAMETERS	equ		15

EXCEPTION_FLAGS_READ_FAULT		equ		0
EXCEPTION_FLAGS_WRITE_FAULT		equ		1
EXCEPTION_FLAGS_EXECUTE_FAULT	equ		8

;
;
; Types
;
;

ExceptionContinueExecution		equ	00000h
ExceptionContinueSearch			equ	00001h
ExceptionNestedException		equ	00002h
ExceptionCollidedUnwind			equ	00003h

;
;
; Structures
;
;

SExceptionRecord struc

	ExceptionCode			DWORD	?
	ExceptionFlags			DWORD	?

	ExceptionRecord			QWORD	?
	ExceptionAddress		QWORD	?

	NumberParameters		DWORD	?
	Align0					DWORD	?
	ExceptionInformation	QWORD	EXCEPTION_MAXIMUM_PARAMETERS dup(?)

SExceptionRecord ends

SDispatcherContext struct

	ControlPc			QWORD		?
	ImageBase			QWORD		?
	FunctionEntry		QWORD		?
	EstablisherFrame	QWORD		?
	TargetIp			QWORD		?
	ContextRecord		QWORD		?
	LanguageHandler		QWORD		?
	HandlerData			QWORD		?
	HistoryTable		QWORD		?
	ScopeIndex			DWORD		?
	Fill0				DWORD		?

SDispatcherContext ends
