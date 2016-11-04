#pragma once

/***************************************************************************************\
|	Declarations																		|
\***************************************************************************************/

#define PE_IMAGE_RNF_INDIRECT				1

#define PE_IMAGE_UNW_VER_1					1
#define PE_IMAGE_UNW_VER_2					2
#define PE_IMAGE_UNW_VER_MAX				PE_IMAGE_UNW_VER_2

#define PE_IMAGE_UNW_FLAG_EHANDLER			0x1
#define PE_IMAGE_UNW_FLAG_UHANDLER			0x2
#define PE_IMAGE_UNW_FLAG_CHAININFO			0x4

#define PE_IMAGE_UWOP_PUSH_NONVOL			0
#define PE_IMAGE_UWOP_ALLOC_LARGE			1
#define PE_IMAGE_UWOP_ALLOC_SMALL			2
#define PE_IMAGE_UWOP_SET_FPREG				3
#define PE_IMAGE_UWOP_SAVE_NONVOL			4
#define PE_IMAGE_UWOP_SAVE_NONVOL_FAR		5
#define PE_IMAGE_UWOP_EPILOG				6
#define PE_IMAGE_UWOP_SPARE_CODE			7
#define PE_IMAGE_UWOP_SAVE_XMM128			8
#define PE_IMAGE_UWOP_SAVE_XMM128_FAR		9
#define PE_IMAGE_UWOP_PUSH_MACHFRAME		10
#define PE_IMAGE_UWOP_MAXIMUM				PE_IMAGE_UWOP_PUSH_MACHFRAME

#define PE_IMAGE_UWP_EPILOG_AT_THE_END		1

#define PE_IMAGE_UNW_CODE_INF_RAX			0
#define PE_IMAGE_UNW_CODE_INF_RCX			1
#define PE_IMAGE_UNW_CODE_INF_RDX			2
#define PE_IMAGE_UNW_CODE_INF_RBX			3
#define PE_IMAGE_UNW_CODE_INF_RSP			4
#define PE_IMAGE_UNW_CODE_INF_RBP			5
#define PE_IMAGE_UNW_CODE_INF_RSI			6
#define PE_IMAGE_UNW_CODE_INF_RDI			7
#define PE_IMAGE_UNW_CODE_INF_R8			8
#define PE_IMAGE_UNW_CODE_INF_R9			9
#define PE_IMAGE_UNW_CODE_INF_R10			10
#define PE_IMAGE_UNW_CODE_INF_R11			11
#define PE_IMAGE_UNW_CODE_INF_R12			12
#define PE_IMAGE_UNW_CODE_INF_R13			13
#define PE_IMAGE_UNW_CODE_INF_R14			14
#define PE_IMAGE_UNW_CODE_INF_R15			15

/***************************************************************************************\
|	Structures																			|
\***************************************************************************************/

#pragma warning( push )
#pragma warning( disable : 4201 )

union SUnwindCode {

	struct {
		uint8_t		CodeOffset;
		uint8_t		OpCode				: 4;
		uint8_t		OpInfo				: 4;
	};

	struct {
		uint8_t		OffsetLowOrSize;
		uint8_t		OpCode				: 4;
		uint8_t		OffsetHighOrFlags	: 4;
	} Epilogue;

	uint16_t	FrameOffset;

};

struct SUnwindInfo {

	uint8_t		Version			: 3;
	uint8_t		Flags			: 5;

	uint8_t		SizeOfProlog;
	uint8_t		CountOfCodes;

	uint8_t		FrameRegister	: 4;
	uint8_t		FrameOffset		: 4;

	SUnwindCode	UnwindCode[1];

};

struct SRuntimeFunction {

	uint32_t		BeginAddress;
	uint32_t		EndAddress;
	uint32_t		UnwindInfo;

};

#pragma warning( pop )

/***************************************************************************************\
|	Functions																			|
\***************************************************************************************/

uint32_t	PeVirtualUnwind(
				void				*Base,
				uint32_t			HandlerType,
				void				**Handler,
				void				**HandlerData,
				uint64_t			*FunctionFrame,
				SRuntimeFunction	**FunctionEntry,
				SContext			*Context
			);
