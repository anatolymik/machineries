#include "..\basic\basic.h"

#include "pe.h"
#include "pestrucs.h"

/***************************************************************************************\
|	Function implementations															|
\***************************************************************************************/

uint32_t PeRecognize( void *Base ) {

	void				*PeHeader;
	SCOFFHeader			*CoffHeader;
	SOPTHeader32Plus	*OptHeader;

	// get pointer to PE signature and check it
	PeHeader = (void*)((uintptr_t)Base + *(uint32_t*)((uintptr_t)Base + PE_SIGNATURE_OFFSET));
	if ( memcmp( PeHeader, PE_SIGNATURE, PE_SIGNATURE_LENGTH ) != 0 ) {
		return ERR_PE_UNRECOGNIZED;
	}

	// get pointer to COFF header
	CoffHeader = (SCOFFHeader*)((uintptr_t)PeHeader + sizeof(uint32_t));

	// check if machine type supported
	if ( CoffHeader->Machine != PE_IMAGE_FILE_MACHINE_AMD64 ) {
		return ERR_PE_UNSUPPORTED;
	}

	// check if there is optional header
	if ( CoffHeader->SizeOfOptionalHeader == 0 ) {
		return ERR_PE_UNSUPPORTED;
	}

	// get pointer to OPTIONAL header standard fields
	OptHeader = (SOPTHeader32Plus*)((uintptr_t)CoffHeader + sizeof(SCOFFHeader));

	// check if PE format supported
	if ( OptHeader->Standard.Magic != PE_MAGIC_PE32_PLUS ) {
		return ERR_PE_UNSUPPORTED;
	}

	// check if we are efi application
	if ( OptHeader->Windows.Subsystem != PE_IMAGE_SUBSYSTEM_EFI_APPLICATION ) {
		return ERR_PE_UNSUPPORTED;
	}

	return ERR_SUCCESS;

}

uint32_t PeGetImageSize( void *Base ) {

	void				*PeHeader;
	SCOFFHeader			*CoffHeader;
	SOPTHeader32Plus	*OptHeader;

	// extract PE pointers
	PeExtractPointers( Base, &PeHeader, &CoffHeader, &OptHeader );

	return OptHeader->Windows.SizeOfImage;

}
