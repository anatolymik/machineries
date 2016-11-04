#pragma once

/***************************************************************************************\
|	Declarations																		|
\***************************************************************************************/

#define EXCEPTION_NONCONTINUABLE			0x01
#define EXCEPTION_UNWINDING					0x02
#define EXCEPTION_EXIT_UNWIND				0x04
#define EXCEPTION_STACK_INVALID				0x08
#define EXCEPTION_NESTED_CALL				0x10
#define EXCEPTION_TARGET_UNWIND				0x20
#define EXCEPTION_COLLIDED_UNWIND			0x40

#define EXCEPTION_UNWIND					(EXCEPTION_UNWINDING | EXCEPTION_EXIT_UNWIND | EXCEPTION_TARGET_UNWIND | EXCEPTION_COLLIDED_UNWIND)

#define EXCEPTION_MAXIMUM_PARAMETERS		15	// maximum exception parameters

#define EXCEPTION_EXECUTE_HANDLER			1
#define EXCEPTION_CONTINUE_SEARCH			0
#define EXCEPTION_CONTINUE_EXECUTION		-1

#define EXCEPTION_FLAGS_READ_FAULT			0
#define EXCEPTION_FLAGS_WRITE_FAULT			1
#define EXCEPTION_FLAGS_EXECUTE_FAULT		8

/***************************************************************************************\
|	Structure declarations																|
\***************************************************************************************/

struct SExceptionRecord;
struct SDispatcherContext;

/***************************************************************************************\
|	Types																				|
\***************************************************************************************/

enum EExceptionDisposition {
	ExceptionContinueExecution,
	ExceptionContinueSearch,
	ExceptionNestedException,
	ExceptionCollidedUnwind
};

/***************************************************************************************\
|	Prototypes																			|
\***************************************************************************************/

typedef	void					(*PGetStackLimits)(
									uintptr_t	*LowLimit,
									uintptr_t	*HighLimit
								);

typedef	void*					(*PGetImageBase)(
									void	*Rip
								);

typedef EExceptionDisposition	(*PExceptionRoutine)(
									SExceptionRecord	*ExceptionRecord,
									void				*EstablisherFrame,
									SContext			*ContextRecord,
									SDispatcherContext	*DispatcherContext
								);

/***************************************************************************************\
|	Structures																			|
\***************************************************************************************/

struct SExceptionRecord {

	uint32_t			ExceptionCode;
	uint32_t			ExceptionFlags;

	SExceptionRecord	*ExceptionRecord;
	void*				ExceptionAddress;

	uint32_t			NumberParameters;
	uintptr_t			ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];

};

struct SDispatcherContext {

	uint64_t				ControlPc;
	uint64_t				ImageBase;
	SRuntimeFunction		*FunctionEntry;
	uint64_t				EstablisherFrame;
	uint64_t				TargetIp;
	SContext				*ContextRecord;
	PExceptionRoutine		LanguageHandler;
	void*					HandlerData;
	void*					HistoryTable;
	uint32_t				ScopeIndex;
	uint32_t				Fill0;

};

struct SExceptionPointers {

	SExceptionRecord	*ExceptionRecord;
	SContext			*ContextRecord;

};

/***************************************************************************************\
|	Global variables																	|
\***************************************************************************************/

extern PGetStackLimits	g_GetStackLimitsFunc;
extern PGetImageBase	g_GetImageBaseFunc;

/***************************************************************************************\
|	Intrinsic functions																	|
\***************************************************************************************/

extern "C" unsigned long	_exception_code( void );
#pragma intrinsic( _exception_code )

extern "C" void*			_exception_info( void );
#pragma intrinsic( _exception_info )

extern "C" int				_abnormal_termination( void );
#pragma intrinsic( _abnormal_termination )

/***************************************************************************************\
|	Macro functions																		|
\***************************************************************************************/

#define GetExceptionCode			_exception_code
#define GetExceptionInformation()	((SExceptionPointers*)_exception_info())
#define AbnormalTermination			_abnormal_termination

/***************************************************************************************\
|	Functions																			|
\***************************************************************************************/

void		UnwindStack(
				void				*TargetFrame,
				void				*TargetIp,
				SExceptionRecord	*ExceptionRecord,
				void				*ReturnValue
			);
bool		DispatchException( SExceptionRecord *ExceptionRecord, SContext *Context );
