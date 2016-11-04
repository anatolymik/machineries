.code

include basic.inc
include excdsptch.inc

;
; KeCaptureFullWithSegsContext
;
; arguments:
; rcx - (SContext*)Context
;
CaptureFullWithSegsContext proc frame

	db		48h	; save rflags
	pushfq
	.allocstack 8

	.endprolog

	mov		[rcx.SContext.SegCs], cs
	mov		[rcx.SContext.SegDs], ds
	mov		[rcx.SContext.SegEs], es
	mov		[rcx.SContext.SegSs], ss
	mov		[rcx.SContext.SegFs], fs
	mov		[rcx.SContext.SegGs], gs

	mov		[rcx.SContext.Rax_], rax
	mov		[rcx.SContext.Rcx_], rcx
	mov		[rcx.SContext.Rdx_], rdx
	mov		[rcx.SContext.Rbx_], rbx
	lea		rax, [rsp+8+8]
	mov		[rcx.SContext.Rsp_], rax
	mov		[rcx.SContext.Rbp_], rbp
	mov		[rcx.SContext.Rsi_], rsi
	mov		[rcx.SContext.Rdi_], rdi
	mov		[rcx.SContext.R8_], r8
	mov		[rcx.SContext.R9_], r9
	mov		[rcx.SContext.R10_], r10
	mov		[rcx.SContext.R11_], r11
	mov		[rcx.SContext.R12_], r12
	mov		[rcx.SContext.R13_], r13
	mov		[rcx.SContext.R14_], r14
	mov		[rcx.SContext.R15_], r15

	movdqa	[rcx.SContext.XmmContext_.XmmRegs_.Xmm0_], xmm0
	movdqa	[rcx.SContext.XmmContext_.XmmRegs_.Xmm1_], xmm1
	movdqa	[rcx.SContext.XmmContext_.XmmRegs_.Xmm2_], xmm2
	movdqa	[rcx.SContext.XmmContext_.XmmRegs_.Xmm3_], xmm3
	movdqa	[rcx.SContext.XmmContext_.XmmRegs_.Xmm4_], xmm4
	movdqa	[rcx.SContext.XmmContext_.XmmRegs_.Xmm5_], xmm5
	movdqa	[rcx.SContext.XmmContext_.XmmRegs_.Xmm6_], xmm6
	movdqa	[rcx.SContext.XmmContext_.XmmRegs_.Xmm7_], xmm7
	movdqa	[rcx.SContext.XmmContext_.XmmRegs_.Xmm8_], xmm8
	movdqa	[rcx.SContext.XmmContext_.XmmRegs_.Xmm9_], xmm9
	movdqa	[rcx.SContext.XmmContext_.XmmRegs_.Xmm10_], xmm10
	movdqa	[rcx.SContext.XmmContext_.XmmRegs_.Xmm11_], xmm11
	movdqa	[rcx.SContext.XmmContext_.XmmRegs_.Xmm12_], xmm12
	movdqa	[rcx.SContext.XmmContext_.XmmRegs_.Xmm13_], xmm13
	movdqa	[rcx.SContext.XmmContext_.XmmRegs_.Xmm14_], xmm14
	movdqa	[rcx.SContext.XmmContext_.XmmRegs_.Xmm15_], xmm15

	stmxcsr	[rcx.SContext.MxCsr]

	mov		rax, [rsp+8]
	mov		[rcx.SContext.Rip_], rax

	mov		rax, [rsp]
	mov		[rcx.SContext.EFlags], eax

	mov		[rcx.SContext.ContextFlags], CONTEXT_FULL or CONTEXT_SEGMENTS

	add		rsp, 8

	ret

CaptureFullWithSegsContext endp

;
; KeRestoreFullWithSegsContext
;
; arguments:
; rcx - (SContext*)Context
;
RestoreFullWithSegsContext proc frame

	sub		rsp, (8*5)
	.allocstack (8*5)

	.endprolog

	movdqa	xmm0, [rcx.SContext.XmmContext_.XmmRegs_.Xmm0_]
	movdqa	xmm1, [rcx.SContext.XmmContext_.XmmRegs_.Xmm1_]
	movdqa	xmm2, [rcx.SContext.XmmContext_.XmmRegs_.Xmm2_]
	movdqa	xmm3, [rcx.SContext.XmmContext_.XmmRegs_.Xmm3_]
	movdqa	xmm4, [rcx.SContext.XmmContext_.XmmRegs_.Xmm4_]
	movdqa	xmm5, [rcx.SContext.XmmContext_.XmmRegs_.Xmm5_]
	movdqa	xmm6, [rcx.SContext.XmmContext_.XmmRegs_.Xmm6_]
	movdqa	xmm7, [rcx.SContext.XmmContext_.XmmRegs_.Xmm7_]
	movdqa	xmm8, [rcx.SContext.XmmContext_.XmmRegs_.Xmm8_]
	movdqa	xmm9, [rcx.SContext.XmmContext_.XmmRegs_.Xmm9_]
	movdqa	xmm10, [rcx.SContext.XmmContext_.XmmRegs_.Xmm10_]
	movdqa	xmm11, [rcx.SContext.XmmContext_.XmmRegs_.Xmm11_]
	movdqa	xmm12, [rcx.SContext.XmmContext_.XmmRegs_.Xmm12_]
	movdqa	xmm13, [rcx.SContext.XmmContext_.XmmRegs_.Xmm13_]
	movdqa	xmm14, [rcx.SContext.XmmContext_.XmmRegs_.Xmm14_]
	movdqa	xmm15, [rcx.SContext.XmmContext_.XmmRegs_.Xmm15_]

	ldmxcsr	[rcx.SContext.MxCsr]

	mov		rax, [rcx.SContext.Rip_]
	mov		[rsp],  rax
	mov		ax, [rcx.SContext.SegCs]
	mov		[rsp+8], ax
	mov		eax, [rcx.SContext.EFlags]
	mov		[rsp+16], eax
	mov		rax, [rcx.SContext.Rsp_]
	mov		[rsp+24], rax
	mov		ax, [rcx.SContext.SegSs]
	mov		[rsp+32], ax

	mov		rax, [rcx.SContext.Rax_]
	mov		rdx, [rcx.SContext.Rdx_]
	mov		rbx, [rcx.SContext.Rbx_]
	mov		rbp, [rcx.SContext.Rbp_]
	mov		rsi, [rcx.SContext.Rsi_]
	mov		rdi, [rcx.SContext.Rdi_]
	mov		r8, [rcx.SContext.R8_]
	mov		r9, [rcx.SContext.R9_]
	mov		r10, [rcx.SContext.R10_]
	mov		r11, [rcx.SContext.R11_]
	mov		r12, [rcx.SContext.R12_]
	mov		r13, [rcx.SContext.R13_]
	mov		r14, [rcx.SContext.R14_]
	mov		r15, [rcx.SContext.R15_]
	mov		rcx, [rcx.SContext.Rcx_]

	iretq

RestoreFullWithSegsContext endp

;
; ExceptionHandler
;
; arguments:
; rcx - (SExceptionRecord*)ExceptionRecord
; rdx - (void*)EstablisherFrame
; r8 - (SContext)ContextRecord
; r9 - (SDispatcherContext*)DispatcherContext
;
; return:
; EXCEPTION_DISPOSITION
;
ExceptionHandler proc

	; if it's unwind then return continue search disposition
	mov		eax, ExceptionContinueSearch
	test	[rcx.SExceptionRecord.ExceptionFlags], EXCEPTION_UNWIND
	jnz		Exit

	; establish context for nested exception

	mov		rax, [rdx+(8*4)] ; get dispatcher context passed to ExecuteHandlerForException function
	mov		rax, [rax.SDispatcherContext.EstablisherFrame] ; save establisher frame in current dispatcher context
	mov		[r9.SDispatcherContext.EstablisherFrame], rax

	mov		eax, ExceptionNestedException

Exit:

	ret

ExceptionHandler endp

;
; ExecuteHandlerForException
;
; arguments:
; rcx - (SExceptionRecord*)ExceptionRecord
; rdx - (void*)EstablisherFrame
; r8 - (SContext*)ContextRecord
; r9 - (SDispatcherContext*)DispatcherContext
;
; return:
; EXCEPTION_DISPOSITION
;
ExecuteHandlerForException proc frame:ExceptionHandler

	sub		rsp, 8+(8*4)
	.allocstack 8+(8*4)

	.endprolog

	mov		[rsp+(8*4)], r9	; save dispatcher context in stack

	call	[r9.SDispatcherContext.LanguageHandler] ; call handler
	nop												; fill for unwinding, it's needed for unwind function to
													; see function body correctly instead of epilog

	add		rsp, 8+(8*4)

	ret

ExecuteHandlerForException endp

;
; UnwindHandler
;
; arguments:
; rcx - (SExceptionRecord*)ExceptionRecord
; rdx - (void*)EstablisherFrame
; r8 - (SContext*)ContextRecord
; r9 - (SDispatcherContext*)DispatcherContext
;
; return:
; EXCEPTION_DISPOSITION
;
UnwindHandler proc

	mov		rax, [rdx+(8*4)] ; get dispatcher context passed to ExecuteHandlerForUnwind function

	mov		rcx, [rax.SDispatcherContext.ControlPc]	; copy control pc
	mov		[r9.SDispatcherContext.ControlPc], rcx

	mov		rcx, [rax.SDispatcherContext.ImageBase]	; copy image base
	mov		[r9.SDispatcherContext.ImageBase], rcx

	mov		rcx, [rax.SDispatcherContext.FunctionEntry]	; copy function entry
	mov		[r9.SDispatcherContext.FunctionEntry], rcx

	mov		rcx, [rax.SDispatcherContext.EstablisherFrame]	; copy establisher frame
	mov		[r9.SDispatcherContext.EstablisherFrame], rcx

	mov		rcx, [rax.SDispatcherContext.ContextRecord]	; copy context record
	mov		[r9.SDispatcherContext.ContextRecord], rcx

	mov		rcx, [rax.SDispatcherContext.LanguageHandler]	; copy language handler
	mov		[r9.SDispatcherContext.LanguageHandler], rcx

	mov		rcx, [rax.SDispatcherContext.HandlerData]	; copy handler data
	mov		[r9.SDispatcherContext.HandlerData], rcx

	mov		rcx, [rax.SDispatcherContext.HistoryTable]	; copy history table
	mov		[r9.SDispatcherContext.HistoryTable], rcx

	mov		ecx, [rax.SDispatcherContext.ScopeIndex]	; copy scope index
	mov		[r9.SDispatcherContext.ScopeIndex], ecx

	mov		eax, ExceptionCollidedUnwind				; it's collided unwind

	ret

UnwindHandler endp

;
; ExecuteHandlerForUnwind
;
; arguments:
; rcx - (SExceptionRecord*)ExceptionRecord
; rdx - (void*)EstablisherFrame
; r8 - (SContext*)ContextRecord
; r9 - (SDispatcherContext*)DispatcherContext
;
; return:
; EXCEPTION_DISPOSITION
;
ExecuteHandlerForUnwind proc frame:UnwindHandler

	sub		rsp, 8+(8*4)
	.allocstack 8+(8*4)

	.endprolog

	mov		[rsp+(8*4)], r9	; save dispatcher context in stack

	call	[r9.SDispatcherContext.LanguageHandler] ; call handler
	nop												; fill for unwinding, it's needed for unwind function to
													; see function body correctly instead of epilog

	add		rsp, 8+(8*4)

	ret

ExecuteHandlerForUnwind endp

end
