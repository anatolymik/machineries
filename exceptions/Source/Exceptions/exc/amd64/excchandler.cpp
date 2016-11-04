#include "..\..\basic\basic.h"

#include "..\..\pe\pe.h"

#include "excdsptch.h"
#include "excchandler.h"

/***************************************************************************************\
|	Functions																			|
\***************************************************************************************/

extern "C" EExceptionDisposition __C_specific_handler(
	SExceptionRecord	*ExceptionRecord,
	void				*EstablisherFrame,
	SContext			*ContextRecord,
	SDispatcherContext	*DispatcherContext
) {

	SScopeTable						*HandlerData;

	PCLanguageExceptionHandler		Handler;
	int32_t							HandlerDisposition;

	SExceptionPointers	ExceptionPointers;

	uint64_t		ImageBase;
	uint64_t		TargetRipRva;
	uint64_t		RipRva;

	uint32_t		Index;

	// get exception context
	ImageBase = DispatcherContext->ImageBase;
	RipRva = DispatcherContext->ControlPc - ImageBase;
	HandlerData = (SScopeTable*)DispatcherContext->HandlerData;

	// it's exception handler
	if ( !(ExceptionRecord->ExceptionFlags & EXCEPTION_UNWIND) ) {

		// initialize exception pointers
		ExceptionPointers.ExceptionRecord = ExceptionRecord;
		ExceptionPointers.ContextRecord = ContextRecord;

		// get index we have to start from
		Index = DispatcherContext->ScopeIndex;

		// go through the list of handlers
		for ( ; Index < HandlerData->Count; Index++ ) {

			// check if it's our record
			if ( HandlerData->ScopeRecord[Index].BeginAddress > RipRva ) {
				continue;
			}
			if ( HandlerData->ScopeRecord[Index].EndAddress <= RipRva ) {
				continue;
			}

			// is there a handler?
			if ( !HandlerData->ScopeRecord[Index].JumpTarget ) {
				continue;
			}

			// call exception filter if we have to
			if ( HandlerData->ScopeRecord[Index].HandlerAddress == EXCEPTION_EXECUTE_HANDLER ) {
				HandlerDisposition = EXCEPTION_EXECUTE_HANDLER;
			} else {
				Handler = (PCLanguageExceptionHandler)(HandlerData->ScopeRecord[Index].HandlerAddress + ImageBase);
				HandlerDisposition = Handler( &ExceptionPointers, (uint64_t)EstablisherFrame );
			}

			// is disposition EXCEPTION_CONTINUE_EXECUTION?
			if ( HandlerDisposition == EXCEPTION_CONTINUE_EXECUTION ) {
				return ExceptionContinueExecution;
			}

			// skip this handler
			if ( HandlerDisposition == EXCEPTION_CONTINUE_SEARCH ) {
				continue;
			}

			// unwind stack as required
			UnwindStack(
				EstablisherFrame,
				(void*)(HandlerData->ScopeRecord[Index].JumpTarget + ImageBase),
				ExceptionRecord,
				(void*)ExceptionRecord->ExceptionCode
			);

		}

	} else {

		// get TargetRip rva
		TargetRipRva = DispatcherContext->TargetIp - ImageBase;

		// get index we have to start from
		Index = DispatcherContext->ScopeIndex;

		// go through the list of handlers
		for ( ; Index < HandlerData->Count; Index++ ) {

			// check if it's our record
			if ( HandlerData->ScopeRecord[Index].BeginAddress > RipRva ) {
				continue;
			}
			if ( HandlerData->ScopeRecord[Index].EndAddress <= RipRva ) {
				continue;
			}

			if ( ExceptionRecord->ExceptionFlags & EXCEPTION_TARGET_UNWIND ) {

				// try to find uplevel try block into which goto could
				// take place

				for ( uint32_t i = 0; i < HandlerData->Count; i++ ) {

					// check if it's our record
					if ( HandlerData->ScopeRecord[i].BeginAddress > TargetRipRva ) {
						continue;
					}
					if ( HandlerData->ScopeRecord[i].EndAddress <= TargetRipRva ) {
						continue;
					}

					// is this uplevel try block?
					if ( HandlerData->ScopeRecord[Index].JumpTarget != HandlerData->ScopeRecord[i].JumpTarget ) {
						continue;
					}
					if ( HandlerData->ScopeRecord[Index].HandlerAddress != HandlerData->ScopeRecord[i].HandlerAddress ) {
						continue;
					}

					return ExceptionContinueSearch;

				}

			}

			// is it exception handler?
			if ( HandlerData->ScopeRecord[Index].JumpTarget ) {

				// did we reach end of unwind handlers
				if ( HandlerData->ScopeRecord[Index].JumpTarget != TargetRipRva ) {
					continue;
				}
				return ExceptionContinueSearch;

			} else {

				// it's unwind record, call handler
				DispatcherContext->ScopeIndex = Index + 1;
				Handler = (PCLanguageExceptionHandler)(HandlerData->ScopeRecord[Index].HandlerAddress + ImageBase);
				Handler( (SExceptionPointers*)1, (uint64_t)EstablisherFrame );

			}

		}

	}

	return ExceptionContinueSearch;

}

extern "C" void _local_unwind( void *TargetFrame, void *TargetIp ) {

	UnwindStack( TargetFrame, TargetIp, NULL, NULL );

}
