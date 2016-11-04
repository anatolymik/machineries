#pragma once

/***************************************************************************************\
|	Declarations																		|
\***************************************************************************************/

#define PE_SIGNATURE_OFFSET							0x3c
#define PE_SIGNATURE								"PE\0\0"
#define PE_SIGNATURE_LENGTH							(sizeof(PE_SIGNATURE)-sizeof(char))

#define PE_MAGIC_PE32								0x10b
#define PE_MAGIC_PE32_PLUS							0x20b

#define PE_IMAGE_SUBSYSTEM_UNKNOWN					0
#define PE_IMAGE_SUBSYSTEM_NATIVE					1
#define PE_IMAGE_SUBSYSTEM_WINDOWS_GUI				2
#define PE_IMAGE_SUBSYSTEM_WINDOWS_CUI				3
#define PE_IMAGE_SUBSYSTEM_POSIX_CUI				7
#define PE_IMAGE_SUBSYSTEM_WINDOWS_CE_GUI			9
#define PE_IMAGE_SUBSYSTEM_EFI_APPLICATION			10
#define PE_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER	11
#define PE_IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER		12
#define PE_IMAGE_SUBSYSTEM_EFI_ROM					13
#define PE_IMAGE_SUBSYSTEM_XBOX						14

#define PE_IMAGE_FILE_MACHINE_UNKNOWN				0
#define PE_IMAGE_FILE_MACHINE_AM33					0x1d3
#define PE_IMAGE_FILE_MACHINE_AMD64					0x8664
#define PE_IMAGE_FILE_MACHINE_ARM					0x1c0
#define PE_IMAGE_FILE_MACHINE_ARMNT					0x1c4
#define PE_IMAGE_FILE_MACHINE_ARM64					0xaa64
#define PE_IMAGE_FILE_MACHINE_EBC					0xebc
#define PE_IMAGE_FILE_MACHINE_I386					0x14c
#define PE_IMAGE_FILE_MACHINE_IA64					0x200
#define PE_IMAGE_FILE_MACHINE_M32R					0x9041
#define PE_IMAGE_FILE_MACHINE_MIPS16				0x266
#define PE_IMAGE_FILE_MACHINE_MIPSFPU				0x366
#define PE_IMAGE_FILE_MACHINE_MIPSFPU16				0x466
#define PE_IMAGE_FILE_MACHINE_POWERPC				0x1f0
#define PE_IMAGE_FILE_MACHINE_POWERPCFP				0x1f1
#define PE_IMAGE_FILE_MACHINE_R4000					0x166
#define PE_IMAGE_FILE_MACHINE_SH3					0x1a2
#define PE_IMAGE_FILE_MACHINE_SH3DSP				0x1a3
#define PE_IMAGE_FILE_MACHINE_SH4					0x1a6
#define PE_IMAGE_FILE_MACHINE_SH5					0x1a8
#define PE_IMAGE_FILE_MACHINE_THUMB					0x1c2
#define PE_IMAGE_FILE_MACHINE_WCEMIPSV2				0x169

#define PE_IMAGE_FILE_RELOCS_STRIPPED				0x0001
#define PE_IMAGE_FILE_EXECUTABLE_IMAGE				0x0002
#define PE_IMAGE_FILE_LINE_NUMS_STRIPPED			0x0004
#define PE_IMAGE_FILE_LOCAL_SYMS_STRIPPED			0x0008
#define PE_IMAGE_FILE_AGGRESSIVE_WS_TRIM			0x0010
#define PE_IMAGE_FILE_LARGE_ADDRESS_AWARE			0x0020
#define PE_IMAGE_FILE_BYTES_REVERSED_LO				0x0080
#define PE_IMAGE_FILE_32BIT_MACHINE					0x0100
#define PE_IMAGE_FILE_DEBUG_STRIPPED				0x0200
#define PE_IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP		0x0400
#define PE_IMAGE_FILE_NET_RUN_FROM_SWAP				0x0800
#define PE_IMAGE_FILE_SYSTEM						0x1000
#define PE_IMAGE_FILE_DLL							0x2000
#define PE_IMAGE_FILE_UP_SYSTEM_ONLY				0x4000
#define PE_IMAGE_FILE_BYTES_REVERSED_HI				0x8000

#define PE_IMAGE_REL_BASED_ABSOLUTE					0
#define PE_IMAGE_REL_BASED_HIGH						1
#define PE_IMAGE_REL_BASED_LOW						2
#define PE_IMAGE_REL_BASED_HIGHLOW					3
#define PE_IMAGE_REL_BASED_HIGHADJ					4
#define PE_IMAGE_REL_BASED_DIR64					10

/***************************************************************************************\
|	Structures																			|
\***************************************************************************************/

#pragma pack( push, 1 )

struct SCOFFHeader {

	uint16_t	Machine;
	uint16_t	NumberOfSections;
	uint32_t	TimeDateStamp;
	uint32_t	PointerToSymbolTable;
	uint32_t	NumberOfSymbols;
	uint16_t	SizeOfOptionalHeader;
	uint16_t	Characteristics;

};

struct SOPTHeaderStandard32Plus {

	uint16_t	Magic;
	uint8_t		MajorLinkerVersion;
	uint8_t		MinorLinkerVersion;
	uint32_t	SizeOfCode;
	uint32_t	SizeOfInitializedData;
	uint32_t	SizeOfUninitializedData;
	uint32_t	AddressOfEntryPoint;
	uint32_t	BaseOfCode;

};

struct SOPTHeaderWindows32Plus {

	uint64_t	ImageBase;
	uint32_t	SectionAlignment;
	uint32_t	FileAlignment;
	uint16_t	MajorOperatingSystemVersion;
	uint16_t	MinorOperatingSystemVersion;
	uint16_t	MajorImageVersion;
	uint16_t	MinorImageVersion;
	uint16_t	MajorSubsystemVersion;
	uint16_t	MinorSubsystemVersion;
	uint32_t	Win32VersionValue;
	uint32_t	SizeOfImage;
	uint32_t	SizeOfHeaders;
	uint32_t	CheckSum;
	uint16_t	Subsystem;
	uint16_t	DllCharacteristics;
	uint64_t	SizeOfStackReserve;
	uint64_t	SizeOfStackCommit;
	uint64_t	SizeOfHeapReserve;
	uint64_t	SizeOfHeapCommit;
	uint32_t	LoaderFlags;
	uint32_t	NumberOfRvaAndSizes;

};

struct SImageDataDirectory {

	uint32_t	VirtualAddress;
	uint32_t	Size;

};

struct SOPTHeaderDataDirs32Plus {

	SImageDataDirectory		ExportTable;
	SImageDataDirectory		ImportTable;
	SImageDataDirectory		ResourceTable;
	SImageDataDirectory		ExceptionTable;
	SImageDataDirectory		CertificateTable;
	SImageDataDirectory		BaseRelocationTable;
	SImageDataDirectory		Debug;
	SImageDataDirectory		Architecture;
	SImageDataDirectory		GlobalPtr;
	SImageDataDirectory		TLSTable;
	SImageDataDirectory		LoadConfigTable;
	SImageDataDirectory		BoundImport;
	SImageDataDirectory		IAT;
	SImageDataDirectory		DelayImportDescriptor;
	SImageDataDirectory		CLRRuntimeHeader;
	SImageDataDirectory		Reserved;

};

struct SOPTHeader32Plus {

	SOPTHeaderStandard32Plus	Standard;
	SOPTHeaderWindows32Plus		Windows;
	SOPTHeaderDataDirs32Plus	DataDirs;

};

struct SBaseRelocBlockEntry {

	uint16_t	Offset	: 12;
	uint16_t	Type	: 4;

};

struct SBaseRelocBlock {

	uint32_t				PageRVA;
	uint32_t				BlockSize;
	SBaseRelocBlockEntry	Entries[1];

};

#pragma pack( pop )

/***************************************************************************************\
|	Functions																			|
\***************************************************************************************/

void	PeExtractPointers( void *Base, void **PeHeader, SCOFFHeader **CoffHeader, SOPTHeader32Plus **OptHeader );
