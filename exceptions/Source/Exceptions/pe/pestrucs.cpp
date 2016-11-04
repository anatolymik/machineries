#include "..\basic\basic.h"

#include "pe.h"
#include "pestrucs.h"

/***************************************************************************************\
|	Functions																			|
\***************************************************************************************/

void PeExtractPointers( void *Base, void **PeHeader, SCOFFHeader **CoffHeader, SOPTHeader32Plus **OptHeader ) {

	// get pointer to PE signature
	*PeHeader = (void*)((uintptr_t)Base + *(uint32_t*)((uintptr_t)Base + PE_SIGNATURE_OFFSET));

	// get pointer to COFF header
	*CoffHeader = (SCOFFHeader*)((uintptr_t)*PeHeader + sizeof(uint32_t));

	// get pointer to OPTIONAL header standard fields
	*OptHeader = (SOPTHeader32Plus*)((uintptr_t)*CoffHeader + sizeof(SCOFFHeader));

}
