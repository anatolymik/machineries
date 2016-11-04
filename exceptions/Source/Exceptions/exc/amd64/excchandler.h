#pragma once

/***************************************************************************************\
|	Prototypes																			|
\***************************************************************************************/

typedef int32_t					(*PCLanguageExceptionHandler)(
									SExceptionPointers	*ExceptionPointers,
									uint64_t			EstablisherFrame
								);

/***************************************************************************************\
|	Structures																			|
\***************************************************************************************/

struct SScopeTable {

	uint32_t	Count;
	struct {
		uint32_t	BeginAddress;
		uint32_t	EndAddress;
		uint32_t	HandlerAddress;
		uint32_t	JumpTarget;
	} ScopeRecord[1];

};
