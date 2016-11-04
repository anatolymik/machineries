#include "..\..\basic\basic.h"

#include "..\..\pe\pe.h"

#include "excdsptch.h"

/***************************************************************************************\
|	External functions																	|
\***************************************************************************************/

extern "C" void						CaptureFullWithSegsContext( SContext *Context );
extern "C" void						RestoreFullWithSegsContext( SContext *Context );

extern "C" EExceptionDisposition	ExecuteHandlerForUnwind(
										SExceptionRecord	*ExceptionRecord,
										void				*EstablisherFrame,
										SContext			*ContextRecord,
										SDispatcherContext	*DispatcherContext
									);

extern "C" EExceptionDisposition	ExecuteHandlerForException(
										SExceptionRecord	*ExceptionRecord,
										void				*EstablisherFrame,
										SContext			*ContextRecord,
										SDispatcherContext	*DispatcherContext
									);

/***************************************************************************************\
|	Global variables																	|
\***************************************************************************************/

PGetStackLimits		g_GetStackLimitsFunc = NULL;
PGetImageBase		g_GetImageBaseFunc = NULL;

/***************************************************************************************\
|	Internal functions																	|
\***************************************************************************************/

static void CopyContext( SContext *Destination, SContext *Source ) {

	Destination->Rbx = Source->Rbx;
	Destination->Rsp = Source->Rsp;
	Destination->Rbp = Source->Rbp;
	Destination->Rsi = Source->Rsi;
	Destination->Rdi = Source->Rdi;
	Destination->R12 = Source->R12;
	Destination->R13 = Source->R13;
	Destination->R14 = Source->R14;
	Destination->R15 = Source->R15;
	Destination->Rip = Source->Rip;
	Destination->EFlags = Source->EFlags;
	Destination->SegCs = Source->SegCs;
	Destination->SegDs = Source->SegDs;
	Destination->SegEs = Source->SegEs;
	Destination->SegFs = Source->SegFs;
	Destination->SegGs = Source->SegGs;
	Destination->SegSs = Source->SegSs;

	Destination->Xmm6 = Source->Xmm6;
	Destination->Xmm7 = Source->Xmm7;
	Destination->Xmm8 = Source->Xmm8;
	Destination->Xmm9 = Source->Xmm9;
	Destination->Xmm10 = Source->Xmm10;
	Destination->Xmm11 = Source->Xmm11;
	Destination->Xmm12 = Source->Xmm12;
	Destination->Xmm13 = Source->Xmm13;
	Destination->Xmm14 = Source->Xmm14;
	Destination->Xmm15 = Source->Xmm15;
	Destination->MxCsr = Source->MxCsr;

}

/***************************************************************************************\
|	Functions																			|
\***************************************************************************************/

void UnwindStack(
	void				*TargetFrame,
	void				*TargetIp,
	SExceptionRecord	*ExceptionRecord,
	void				*ReturnValue
) {

	SDispatcherContext		DispatcherContext;
	SContext				PreviousContext;
	SContext				CurrentContext;

	EExceptionDisposition	Disposition;

	SExceptionRecord	UnwindRecord;

	PExceptionRoutine	Handler;
	void				*HandlerData;

	uint64_t			FunctionFrame = 0;
	SRuntimeFunction	*FunctionEntry;

	void				*Base;

	uint32_t		ExceptionFlags;
	uint32_t		ScopeIndex;

	uintptr_t		StackLowLimit;
	uintptr_t		StackHighLimit;

	uint64_t		Rip;

	uint32_t		Status;

	// get stack limits
	g_GetStackLimitsFunc( &StackLowLimit, &StackHighLimit );

	// capture current context from which we'll start unwinding
	CaptureFullWithSegsContext( &CurrentContext );
	memcpy( &PreviousContext, &CurrentContext, sizeof(SContext) );

	// if no exception record was supplied then it's simple unwinding
	if ( !ExceptionRecord ) {
		UnwindRecord.ExceptionCode = ERR_UNWIND;
		UnwindRecord.ExceptionRecord = NULL;
		UnwindRecord.ExceptionAddress = (void*)CurrentContext.Rip;
		UnwindRecord.NumberParameters = 0;
		ExceptionRecord = &UnwindRecord;
	}

	// set starting unwind flags
	ExceptionFlags = EXCEPTION_UNWINDING;

	do {

		// save rip
		Rip = CurrentContext.Rip;

		// get module base
		Base = g_GetImageBaseFunc( (void*)Rip );
		if ( !Base ) {

			// no module containing this address
			// possibly this is a calling of function is at
			// invalid address, or this is just leaf function

			PreviousContext.Rip = *((uint64_t*)PreviousContext.Rsp);
			PreviousContext.Rsp += 8;

		} else {

			// unwind current stack frame
			Status = PeVirtualUnwind(
				Base,
				PE_IMAGE_UNW_FLAG_UHANDLER,
				(void**)&Handler,
				&HandlerData,
				&FunctionFrame,
				&FunctionEntry,
				&PreviousContext
			);
			if ( Status != ERR_SUCCESS ) {
				ASSERT( false );
			}

			// check stack alignment
			if ( FunctionFrame & 0x7 ) {
				ASSERT( FALSE );
			}

			// check stack limits
			if ( (FunctionFrame < StackLowLimit) || (FunctionFrame >= StackHighLimit) ) {
				ASSERT( FALSE );
			}

			// check if we skipped target frame because unwinding fails
			if ( FunctionFrame > (uint64_t)TargetFrame ) {
				ASSERT( FALSE );
			}

			// if there is unwind handler, call it
			if ( Handler ) {

				// prepare context
				DispatcherContext.TargetIp = (uint64_t)TargetIp;
				ScopeIndex = 0;
				do {

					// set exception flags properly
					if ( (uint64_t)TargetFrame == FunctionFrame ) {
						ExceptionFlags |= EXCEPTION_TARGET_UNWIND;
					}

					ExceptionRecord->ExceptionFlags = ExceptionFlags;

					// set return value before exception handler calling for the case
					// if the handler decides directly continue execution
					CurrentContext.Rax = (uint64_t)ReturnValue;

					// set context
					DispatcherContext.ControlPc = Rip;
					DispatcherContext.ImageBase = (uint64_t)Base;
					DispatcherContext.FunctionEntry = FunctionEntry;
					DispatcherContext.EstablisherFrame = FunctionFrame;
					DispatcherContext.ContextRecord = &CurrentContext;
					DispatcherContext.LanguageHandler = Handler;
					DispatcherContext.HandlerData = HandlerData;
					DispatcherContext.HistoryTable = NULL;
					DispatcherContext.ScopeIndex = ScopeIndex;

					// and execute handler
					Disposition = ExecuteHandlerForUnwind(
						ExceptionRecord,
						(void*)FunctionFrame,
						&CurrentContext,
						&DispatcherContext
					);

					// clear unnecessary flags
					ExceptionFlags &= ~(EXCEPTION_COLLIDED_UNWIND | EXCEPTION_TARGET_UNWIND);

					if ( Disposition == ExceptionCollidedUnwind ) {

						CopyContext( &CurrentContext, DispatcherContext.ContextRecord );
						CopyContext( &PreviousContext, DispatcherContext.ContextRecord );

						Status = PeVirtualUnwind(
							Base,
							0,
							NULL,
							NULL,
							&FunctionFrame,
							&FunctionEntry,
							&PreviousContext
						);
						if ( Status != ERR_SUCCESS ) {
							ASSERT( false );
						}

						Rip = DispatcherContext.ControlPc;
						FunctionEntry = DispatcherContext.FunctionEntry;
						FunctionFrame = DispatcherContext.EstablisherFrame;
						Handler = DispatcherContext.LanguageHandler;
						HandlerData = DispatcherContext.HandlerData;
						ScopeIndex = DispatcherContext.ScopeIndex;

						ExceptionFlags |= EXCEPTION_COLLIDED_UNWIND;

					} else if ( Disposition != ExceptionContinueSearch ) {

						ASSERT( false );

					}

				} while ( ExceptionFlags & EXCEPTION_COLLIDED_UNWIND );

			}

		}

		// if we reached target frame then prepare new current context
		if ( FunctionFrame != (uint64_t)TargetFrame ) {
			memcpy( &CurrentContext, &PreviousContext, sizeof(SContext) );
		}

		// check stack alignment and check its limits
		if ( (CurrentContext.Rsp & 0x7) || (CurrentContext.Rsp < StackLowLimit) || (CurrentContext.Rsp >= StackHighLimit) ) {
			break;
		}

	} while ( (uint64_t)TargetFrame != FunctionFrame );

	// did we reached target frame?
	if ( (uint64_t)TargetFrame != FunctionFrame ) {

		// some unwind error

		ASSERT( false );

	} else {

		// set return value and rip
		CurrentContext.Rax = (uint64_t)ReturnValue;
		CurrentContext.Rip = (uint64_t)TargetIp;

		// switch to unwound context
		RestoreFullWithSegsContext( &CurrentContext );

	}

}

bool DispatchException( SExceptionRecord *ExceptionRecord, SContext *Context ) {

	SDispatcherContext		DispatcherContext;
	SContext				LocContext;

	EExceptionDisposition	Disposition;

	PExceptionRoutine	Handler;
	void				*HandlerData;

	uint64_t			FunctionFrame;
	SRuntimeFunction	*FunctionEntry;

	void				*Base;

	uint64_t			NestedFrame;

	uint32_t			ExceptionFlags;
	uint32_t			ScopeIndex;

	uintptr_t			StackLowLimit;
	uintptr_t			StackHighLimit;

	uint64_t			Rip;

	uint32_t			Status;

	// get stack limits
	g_GetStackLimitsFunc( &StackLowLimit, &StackHighLimit );

	// copy and initialize starting context
	memset( &DispatcherContext, 0, sizeof(DispatcherContext) );
	memcpy( &LocContext, Context, sizeof(SContext) );
	ExceptionFlags = ExceptionRecord->ExceptionFlags & EXCEPTION_NONCONTINUABLE;
	NestedFrame = 0;

	for ( ;; ) {

		// update current rip
		Rip = LocContext.Rip;

		// get module base
		Base = g_GetImageBaseFunc( (void*)Rip );
		if ( !Base ) {

			// no module containing this address
			// possibly this is a calling of function is at
			// invalid address, or this is just leaf function

			LocContext.Rip = *((uint64_t*)LocContext.Rsp);
			LocContext.Rsp += 8;

		} else {

			// unwind current stack frame
			Status = PeVirtualUnwind(
				Base,
				PE_IMAGE_UNW_FLAG_EHANDLER,
				(void**)&Handler,
				&HandlerData,
				&FunctionFrame,
				&FunctionEntry,
				&LocContext
			);
			if ( Status != ERR_SUCCESS ) {
				ASSERT( false );
			}

			// check frame alignment and check it is within stack limits
			if ( (FunctionFrame != 0) &&
				 ((FunctionFrame & 0x7) || (FunctionFrame < StackLowLimit) || (FunctionFrame >= StackHighLimit)) ) {

				ExceptionFlags |= EXCEPTION_STACK_INVALID;
				break;

			}

			// if there is a handler
			if ( Handler ) {

				// prepare context
				ScopeIndex = 0;
				for ( ;; ) {

					ExceptionRecord->ExceptionFlags = ExceptionFlags;

					DispatcherContext.ControlPc = Rip;
					DispatcherContext.ImageBase = (uint64_t)Base;
					DispatcherContext.FunctionEntry = FunctionEntry;
					DispatcherContext.EstablisherFrame = FunctionFrame;
					DispatcherContext.ContextRecord = &LocContext;
					DispatcherContext.LanguageHandler = Handler;
					DispatcherContext.HandlerData = HandlerData;
					DispatcherContext.HistoryTable = NULL;
					DispatcherContext.ScopeIndex = ScopeIndex;

					// and execute handler
					Disposition = ExecuteHandlerForException(
						ExceptionRecord,
						(void*)FunctionFrame,
						Context,
						&DispatcherContext
					);

					// propogate noncontinuable exception flag
					ExceptionFlags |= (ExceptionRecord->ExceptionFlags & EXCEPTION_NONCONTINUABLE);

					// reset nested flag if we have reached nested context end
					if ( NestedFrame == FunctionFrame ) {
						ExceptionFlags &= ~EXCEPTION_NESTED_CALL;
						NestedFrame = 0;
					}

					if ( Disposition == ExceptionContinueExecution ) {

						// disposition is to continue execution

						if ( ExceptionFlags & EXCEPTION_NONCONTINUABLE ) {
							ASSERT( false );
						}
						return true;

					} else if ( Disposition == ExceptionNestedException ) {

						// disposition is nested exception, set nested flag

						ExceptionFlags |= EXCEPTION_NESTED_CALL;
						if ( DispatcherContext.EstablisherFrame > NestedFrame ) {
							NestedFrame = DispatcherContext.EstablisherFrame;
						}

					} else if ( Disposition == ExceptionCollidedUnwind ) {

						// disposition is collided unwind, copy the context and continue
						// exception handler lookup from frame where first exception occurred

						CopyContext( &LocContext, DispatcherContext.ContextRecord );

						Rip = DispatcherContext.ControlPc;
						FunctionEntry = DispatcherContext.FunctionEntry;
						FunctionFrame = DispatcherContext.EstablisherFrame;
						Handler = DispatcherContext.LanguageHandler;
						HandlerData = DispatcherContext.HandlerData;
						ScopeIndex = DispatcherContext.ScopeIndex;

						LocContext.Rip = Rip;

						continue;

					} else if ( Disposition != ExceptionContinueSearch ) {

						// disposition is invalid

						ASSERT( false );

					}

					break;

				}

			}

		}

		// check stack alignment and check its limits
		if ( (LocContext.Rsp & 0x7) || (LocContext.Rsp < StackLowLimit) || (LocContext.Rsp >= StackHighLimit) ) {
			break;
		}

	}

	// exception is not handled, set final exception flags
	ExceptionRecord->ExceptionFlags = ExceptionFlags;

	return false;

}
