#pragma once

/***************************************************************************************\
|	Structures																			|
\***************************************************************************************/

#pragma warning( push )
#pragma warning( disable : 4201 )

struct __declspec(align(16)) SXmm {

	uint64_t	Low;
	int64_t		High;

};

struct __declspec(align(16)) SFxSaveArea {

	uint16_t	ControlWord;
	uint16_t	StatusWord;
	uint8_t		TagWord;
	uint8_t		Reserved1;
	uint16_t	ErrorOpcode;
	uint32_t	ErrorOffset;
	uint16_t	ErrorSelector;
	uint16_t	Reserved2;
	uint32_t	DataOffset;
	uint16_t	DataSelector;
	uint16_t	Reserved3;
	uint32_t	MxCsr;
	uint32_t	MxCsr_Mask;
	SXmm		FloatRegisters[8];

	SXmm		XmmRegisters[16];
	uint8_t		Reserved4[96];

};

struct __declspec(align(16)) SContext {

	uint32_t	ContextFlags;
	uint32_t	MxCsr;

	uint16_t	SegCs;
	uint16_t	SegDs;
	uint16_t	SegEs;
	uint16_t	SegFs;
	uint16_t	SegGs;
	uint16_t	SegSs;
	uint32_t	EFlags;

	uint64_t	Dr0;
	uint64_t	Dr1;
	uint64_t	Dr2;
	uint64_t	Dr3;
	uint64_t	Dr6;
	uint64_t	Dr7;

	union {
		uint64_t	Gpr[16];
		struct {
			uint64_t	Rax;
			uint64_t	Rcx;
			uint64_t	Rdx;
			uint64_t	Rbx;
			uint64_t	Rsp;
			uint64_t	Rbp;
			uint64_t	Rsi;
			uint64_t	Rdi;
			uint64_t	R8;
			uint64_t	R9;
			uint64_t	R10;
			uint64_t	R11;
			uint64_t	R12;
			uint64_t	R13;
			uint64_t	R14;
			uint64_t	R15;
		};
	};

	uint64_t	Rip;

	union {
		SFxSaveArea	FltSave;
		struct {
			SXmm	Header[2];
			SXmm	Legacy[8];
			SXmm	Xmm0;
			SXmm	Xmm1;
			SXmm	Xmm2;
			SXmm	Xmm3;
			SXmm	Xmm4;
			SXmm	Xmm5;
			SXmm	Xmm6;
			SXmm	Xmm7;
			SXmm	Xmm8;
			SXmm	Xmm9;
			SXmm	Xmm10;
			SXmm	Xmm11;
			SXmm	Xmm12;
			SXmm	Xmm13;
			SXmm	Xmm14;
			SXmm	Xmm15;
		};
	};

};

#pragma warning( pop )
