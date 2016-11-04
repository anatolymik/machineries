#include "..\..\basic\basic.h"

#include "..\pe.h"
#include "..\pestrucs.h"

/***************************************************************************************\
|	Internal variables																	|
\***************************************************************************************/

static uint8_t	g_PeUnwindCodeLengths[] = {
	0x01,			// UWOP_PUSH_NONVOL
	0x02,			// UWOP_ALLOC_LARGE (or 3, checking in code)
	0x01,			// UWOP_ALLOC_SMALL
	0x01,			// UWOP_SET_FPREG
	0x02,			// UWOP_SAVE_NONVOL
	0x03,			// UWOP_SAVE_NONVOL_FAR
	0x02,			// UWOP_EPILOG
	0x03,			// UWOP_SPARE_CODE
	0x02,			// UWOP_SAVE_XMM128
	0x03,			// UWOP_SAVE_XMM128_FAR
	0x01			// UWOP_PUSH_MACHFRAME
};

/***************************************************************************************\
|	Internal functions																	|
\***************************************************************************************/

static uint32_t PeOpCodeLength( uint32_t *Length, SUnwindCode Code ) {

	// is code valid?
	if ( Code.OpCode > PE_IMAGE_UWOP_MAXIMUM ) {
		return ERR_PE_CORRUPED_PDATA;
	}

	// do we support this code?
	if ( g_PeUnwindCodeLengths[Code.OpCode] == 0xff ) {
		return ERR_PE_UNSUPPORTED_PDATA;
	}

	// alloc large is 2 or 3 slots length, do check and return if needed
	if ( Code.OpCode == PE_IMAGE_UWOP_ALLOC_LARGE ) {
		if ( Code.OpInfo == 1 ) {
			*Length = g_PeUnwindCodeLengths[Code.OpCode] + 1;
			return ERR_SUCCESS;
		}
	}

	// return opcode length
	*Length = g_PeUnwindCodeLengths[Code.OpCode];
	return ERR_SUCCESS;

}

static SRuntimeFunction* PeLookupFunctionEntry( void *Base, SOPTHeader32Plus *OptHeader, uint32_t RipRva ) {

	SRuntimeFunction	*FunctionTable;
	uint32_t			FunctionTableLength;

	int32_t				Start;
	int32_t				End;
	uint32_t			Current;

	// check if .pdata section exists
	if ( OptHeader->Windows.NumberOfRvaAndSizes < 4 ) {
		return NULL;
	}

	// get function table address and its length
	FunctionTable = (SRuntimeFunction*)((uintptr_t)Base + OptHeader->DataDirs.ExceptionTable.VirtualAddress);
	FunctionTableLength = OptHeader->DataDirs.ExceptionTable.Size / sizeof(SRuntimeFunction);

	// find corresponding function entry, we can use binary search algorithm
	// because function table is sorted
	Start = 0;
	End = FunctionTableLength - 1;
	while ( End >= Start ) {

		Current = (Start + End) / 2;

		if ( RipRva < FunctionTable[Current].BeginAddress ) {
			End = Current - 1;
		} else if ( RipRva >= FunctionTable[Current].EndAddress ) {
			Start = Current + 1;
		} else {
			return &FunctionTable[Current];
		}

	}

	// there wasn't function entry with such control rva, possibly it's leaf function

	return NULL;

}

static uint32_t PeVirtualUnwindEpilogueFromUnwindInfo(
	void				*Base,
	SRuntimeFunction	*RuntimeEntry,
	uint32_t			OffsetInEpilog,
	SContext			*Context
) {

	SUnwindInfo	*UnwindInfo;

	uint32_t	CodeLength;
	uint32_t	ChainDepth = 0;

	uint32_t	CurOffsetInEpilog = 0;
	uint32_t	Index = 0;

	uint32_t	Status;

	UnwindInfo = (SUnwindInfo*)((uintptr_t)Base + RuntimeEntry->UnwindInfo);

	// go through the slots and find first epilogue unwind code
	for ( ;; ) {

		// scan current chain
		for ( ; Index < UnwindInfo->CountOfCodes; ) {

			if ( (UnwindInfo->UnwindCode[Index].OpCode == PE_IMAGE_UWOP_PUSH_NONVOL) ||
				 (UnwindInfo->UnwindCode[Index].OpCode == PE_IMAGE_UWOP_PUSH_MACHFRAME) ) {
				break;
			}

			Status = PeOpCodeLength( &CodeLength, UnwindInfo->UnwindCode[Index] );
			if ( Status != ERR_SUCCESS ) {
				return Status;
			}

			Index += CodeLength;

		}

		// did we find begin?
		if ( Index < UnwindInfo->CountOfCodes ) {
			break;
		}

		// did we reach end of unwind info?
		if ( !(UnwindInfo->Flags & PE_IMAGE_UNW_FLAG_CHAININFO) ) {
			break;
		}

		// update chain depth and check if we have reached depth limit
		ChainDepth++;
		if ( ChainDepth > 32 ) {
			return ERR_PE_CORRUPED_PDATA;
		}

		// get pointers to next chain
		RuntimeEntry = (SRuntimeFunction*)&UnwindInfo->UnwindCode[(UnwindInfo->CountOfCodes + 1) & ~1];
		UnwindInfo = (SUnwindInfo*)((uintptr_t)Base + RuntimeEntry->UnwindInfo);

		// we can't unwind stack frame
		if ( UnwindInfo->Version > PE_IMAGE_UNW_VER_MAX ) {
			return ERR_PE_UNSUPPORTED_PDATA;
		}

	}

	// there is nothing to unwind
	if ( Index >= UnwindInfo->CountOfCodes ) {
		Context->Rip = *((uint64_t*)Context->Rsp);
		Context->Rsp += 8;
		return ERR_SUCCESS;
	}

	// unwind stack
	for ( ;; ) {

		// did we unwind all pop instruction?
		if ( UnwindInfo->UnwindCode[Index].OpCode != PE_IMAGE_UWOP_PUSH_NONVOL ) {
			break;
		}

		// rip is at or before this epilogue point
		if ( CurOffsetInEpilog >= OffsetInEpilog ) {
			Context->Gpr[UnwindInfo->UnwindCode[Index].OpInfo] = *((uint64_t*)Context->Rsp);
			Context->Rsp += 8;
		}

		// update current offset in epilogue, if register number is greater than 8 then
		// pop instruction uses rex prefix that increases pop instruction length
		CurOffsetInEpilog++;
		if ( UnwindInfo->UnwindCode[Index].OpInfo > 8 ) {
			CurOffsetInEpilog++;
		}

		// update current unwind slot index and check if we have reached the end
		Index++;
		if ( Index >= UnwindInfo->CountOfCodes ) {
			Context->Rip = *((uint64_t*)Context->Rsp);
			Context->Rsp += 8;
			return ERR_SUCCESS;
		}

	}

	// is this "add rsp, 8" instruction?
	if ( (UnwindInfo->UnwindCode[Index].OpCode == PE_IMAGE_UWOP_ALLOC_SMALL) &&
		 (UnwindInfo->UnwindCode[Index].OpInfo == 0) ) {

		// rip is at or before this epilogue point
		if ( CurOffsetInEpilog >= OffsetInEpilog ) {
			Context->Rsp += 8;
		}

		Index++;

	}

	// did we reach unwind info end?
	if ( (Index >= UnwindInfo->CountOfCodes) ||
		 (UnwindInfo->UnwindCode[Index].OpCode != PE_IMAGE_UWOP_PUSH_MACHFRAME) ) {

		Context->Rip = *((uint64_t*)Context->Rsp);
		Context->Rsp += 8;
		return ERR_SUCCESS;

	}

	// the end wasn't reached because there is machine frame, unwind it

	if ( UnwindInfo->UnwindCode[Index].OpInfo > 1 ) {
		return ERR_PE_CORRUPED_PDATA;
	}

	// skip error code in stack
	if ( UnwindInfo->UnwindCode[Index].OpInfo == 1 ) {
		Context->Rsp += 8;
	}

	Context->Rip = *((uint64_t*)Context->Rsp);
	Context->Rsp += 8;

	Context->SegCs = *((uint16_t*)Context->Rsp);
	Context->Rsp += 8;

	Context->EFlags = *((uint32_t*)Context->Rsp);
	Context->Rsp += 8;

	Context->SegSs = *((uint16_t*)((uintptr_t)(Context->Rsp + 8)));
	Context->Rsp = *((uint64_t*)Context->Rsp);

	return ERR_SUCCESS;

}

static bool PeVirtualUnwindEpilogueFromCode(
	void				*Base,
	SOPTHeader32Plus	*OptHeader,
	SRuntimeFunction	*RuntimeEntry,
	SUnwindInfo			*UnwindInfo,
	SContext			*Context
) {

	SUnwindInfo *SecUnwindInfo;

	uint8_t		FrameReg = 0;
	uint8_t		Reg;

	uint64_t	NextRip;
	uint8_t		*Rip;

	bool		InEpilogue;

	Rip = (uint8_t*)Context->Rip;

	// check for add rsp instructions

	// add rsp, imm8
	if ( (Rip[0] == 0x48) &&
		 (Rip[1] == 0x83) &&
		 (Rip[2] == 0xc4) ) {

		Rip += 4;

	// add rsp, imm32
	} else if ( (Rip[0] == 0x48) &&
				(Rip[1] == 0x81) &&
				(Rip[2] == 0xc4) ) {

		Rip += 7;

	// lea rsp, ???
	} else if ( ((Rip[0] & 0xfe) == 0x48) &&
				(Rip[1] == 0x8d) ) {

		FrameReg = ((Rip[0] & 0x1) << 3) | (Rip[2] & 0x7);
		if ( (FrameReg != 0) && (FrameReg == UnwindInfo->FrameRegister) ) {

			if ( (Rip[2] & 0xf8) == 0x60 ) {

				// lea rsp, framereg + disp8
				Rip += 4;

			} else if ( (Rip[2] & 0xf8) == 0xa0 ) {

				// lea rsp, framereg + disp32
				Rip += 7;

			}

		}

	}

	// check for pop instructions

	for ( ;; ) {

		// pop
		if ( (Rip[0] & 0xf8) == 0x58 ) {

			Rip += 1;

		// rex pop
		} else if ( ((Rip[0] & 0xf0) == 0x40) &&
					((Rip[1] & 0xf8) == 0x58) ) {

			Rip += 2;

		} else {

			break;

		}

	}

	// check if current instruction is return's instruction from the function

	InEpilogue = false;
	if ( (Rip[0] == 0xc3) ||
		 (Rip[0] == 0xc2) ||
		 ((Rip[0] == 0xf3) && (Rip[1] == 0xc3)) ) {

		// ret instruction
		InEpilogue = true;

	} else if ( (Rip[0] == 0xeb) ||
				(Rip[0] == 0xe9) ) {

		// unconditional jump

		// compute target address
		NextRip = (uint64_t)Rip - (uint64_t)Base;
		if ( Rip[0] == 0xeb ) {
			NextRip += 2 + (int8_t)Rip[1];
		} else {
			NextRip += 5 + *((int32_t*)&Rip[1]);
		}

		// check if jump is to out of function
		if ( (RuntimeEntry->BeginAddress > NextRip) ||
			 (RuntimeEntry->EndAddress <= NextRip) ) {

			// find primary entry for the function
			while ( UnwindInfo->Flags & PE_IMAGE_UNW_FLAG_CHAININFO ) {
				RuntimeEntry = (SRuntimeFunction*)&UnwindInfo->UnwindCode[(UnwindInfo->CountOfCodes + 1) & ~1];
				UnwindInfo = (SUnwindInfo*)((uintptr_t)Base + RuntimeEntry->UnwindInfo);
			}

			// get function entry for next rip
			RuntimeEntry = PeLookupFunctionEntry(
				Base,
				OptHeader,
				(uint32_t)NextRip
			);
			if ( RuntimeEntry ) {

				// get pointer to unwind info and find primary entry for the function
				SecUnwindInfo = (SUnwindInfo*)((uintptr_t)Base + RuntimeEntry->UnwindInfo);
				while ( SecUnwindInfo->Flags & PE_IMAGE_UNW_FLAG_CHAININFO ) {
					RuntimeEntry = (SRuntimeFunction*)&SecUnwindInfo->UnwindCode[(SecUnwindInfo->CountOfCodes + 1) & ~1];
					SecUnwindInfo = (SUnwindInfo*)((uintptr_t)Base + RuntimeEntry->UnwindInfo);
				}

			} else {

				SecUnwindInfo = NULL;

			}

			// if unwind info addresses are the same, it is the same function, in this case
			// check if jump is to function begin, that means the function calls itself
			if ( (UnwindInfo != SecUnwindInfo) || (RuntimeEntry->BeginAddress == NextRip) ) {
				InEpilogue = true;
			}

		// check if the function calls itself
		} else if ( (RuntimeEntry->BeginAddress == NextRip) && !(UnwindInfo->Flags & PE_IMAGE_UNW_FLAG_CHAININFO) ) {

			InEpilogue = true;

		}

	// indirect jump
	} else if ( (Rip[0] == 0xff) &&
				(Rip[1] == 0x25) ) {

		// indirect jump leads outside the function unconditionally
		InEpilogue = true;

	// indirect jump with rex prefix
	} else if ( ((Rip[0] & 0xf8) == 0x48) &&
				(Rip[1] == 0xff) &&
				((Rip[2] & 0x38) == 0x20) ) {

		// indirect jump leads outside the function unconditionally
		InEpilogue = true;

	}

	// that was not in epilogue
	if ( !InEpilogue ) {
		return false;
	}

	// it was in epilogue, unwind stack in this case

	Rip = (uint8_t*)Context->Rip;

	// emulate add rsp instructions

	if ( (Rip[0] & 0xf8) == 0x48 ) {

		// add rsp, imm8
		if ( Rip[1] == 0x83 ) {

			Context->Rsp += (int8_t)Rip[3];
			Rip += 4;

		// add rsp, imm32
		} else if ( Rip[1] == 0x81 ) {

			Context->Rsp += *((int32_t*)&Rip[3]);
			Rip += 7;

		// lea rsp, ???
		} else if ( Rip[1] == 0x8d ) {

			// lea rsp, framereg + disp8
			if ( (Rip[2] & 0xf8) == 0x60 ) {

				Context->Rsp = Context->Gpr[FrameReg];
				Context->Rsp += (int8_t)Rip[3];
				Rip += 4;

			// lea rsp, framereg + disp32
			} else if ( (Rip[2] & 0xf8) == 0xa0 ) {

				Context->Rsp = Context->Gpr[FrameReg];
				Context->Rsp += *((int32_t*)&Rip[3]);
				Rip += 7;

			}

		}

	}

	// emulate pop instructions

	for ( ;; ) {

		// pop
		if ( (Rip[0] & 0xf8) == 0x58 ) {

			Reg = Rip[0] & 0x07;
			Context->Gpr[Reg] = *((uint64_t*)Context->Rsp);
			Context->Rsp += 8;
			Rip += 1;

		// rex pop
		} else if ( ((Rip[0] & 0xf0) == 0x40) &&
					((Rip[1] & 0xf8) == 0x58) ) {

			Reg = ((Rip[0] & 1) << 3) | (Rip[1] & 0x07);
			Context->Gpr[Reg] = *((uint64_t*)Context->Rsp);
			Context->Rsp += 8;
			Rip += 2;

		} else {

			break;

		}

	}

	// emulate return instruction

	Context->Rip = *((uint64_t*)Context->Rsp);
	Context->Rsp += 8;

	return true;

}

/***************************************************************************************\
|	Functions																			|
\***************************************************************************************/

uint32_t PeVirtualUnwind(
	void				*Base,
	uint32_t			HandlerType,
	void				**Handler,
	void				**HandlerData,
	uint64_t			*FunctionFrame,
	SRuntimeFunction	**FunctionEntry,
	SContext			*Context
) {

	void				*PeHeader;
	SCOFFHeader			*CoffHeader;
	SOPTHeader32Plus	*OptHeader;

	SRuntimeFunction	*RuntimeEntry;
	SUnwindInfo			*UnwindInfo;

	uint32_t			OffsetInProlog;

	uint64_t			FrameBase;
	uint64_t			FrameOffset;

	uintptr_t			BeginAddr;
	uintptr_t			EndAddr;

	uint32_t			EpilogSize;
	uint32_t			EpilogOffset;

	uint32_t			ChainDepth = 0;
	uint32_t			CodeLength;

	uint32_t			Index;

	uint64_t			Rip;

	bool				SkipReturnUnwind = false;

	uint32_t			Status;

	// zero results
	*FunctionEntry = NULL;
	if ( HandlerType != 0 ) {
		*Handler = NULL;
	}

	// extract PE pointers
	PeExtractPointers( Base, &PeHeader, &CoffHeader, &OptHeader );

	// get function entry for corresponding rip
	RuntimeEntry = PeLookupFunctionEntry(
		Base,
		OptHeader,
		(uint32_t)((uintptr_t)Context->Rip - (uintptr_t)Base)
	);
	if ( !RuntimeEntry ) {

		// this is leaf function

		// function has no frame
		*FunctionFrame = 0;

		Context->Rip = *((uint64_t*)Context->Rsp);
		Context->Rsp += 8;

		return ERR_SUCCESS;

	}

	// get begin and end addresses
	BeginAddr = (uintptr_t)Base + RuntimeEntry->BeginAddress;
	EndAddr = (uintptr_t)Base + RuntimeEntry->EndAddress;

	// get pointer to unwind info and check version
	UnwindInfo = (SUnwindInfo*)((uintptr_t)Base + RuntimeEntry->UnwindInfo);
	if ( UnwindInfo->Version > PE_IMAGE_UNW_VER_MAX ) {
		return ERR_PE_UNSUPPORTED_PDATA;
	}

	// save current rip;
	Rip = Context->Rip;

	// get current frame base
	OffsetInProlog = (uint32_t)(Context->Rip - BeginAddr);
	if ( UnwindInfo->FrameRegister == 0 ) {

		// frame base is current rsp

		FrameBase = Context->Rsp;

	} else if ( (OffsetInProlog >= UnwindInfo->SizeOfProlog) || (UnwindInfo->Flags & PE_IMAGE_UNW_FLAG_CHAININFO) ) {

		// compute frame base

		FrameBase = Context->Gpr[UnwindInfo->FrameRegister];
		FrameBase -= UnwindInfo->FrameOffset * 16;

	} else {

		// the instruction that raised exception is in prologue, we should
		// check if frame register was established before exception

		Index = 0;
		while ( Index < UnwindInfo->CountOfCodes ) {

			// we found frame establishing slot?
			if ( UnwindInfo->UnwindCode[Index].OpCode == PE_IMAGE_UWOP_SET_FPREG ) {
				break;
			}

			// get code length
			Status = PeOpCodeLength( &CodeLength, UnwindInfo->UnwindCode[Index] );
			if ( Status != ERR_SUCCESS ) {
				return Status;
			}

			// and go to next unwind slot
			Index += CodeLength;

		}

		// was frame pointer established?
		if ( OffsetInProlog >= UnwindInfo->UnwindCode[Index].CodeOffset ) {
			FrameBase = Context->Gpr[UnwindInfo->FrameRegister];
			FrameBase -= UnwindInfo->FrameOffset * 16;
		} else {
			FrameBase = Context->Rsp;
		}

	}

	// return frame base
	*FunctionFrame = FrameBase;

	// current version is less than 2
	if ( UnwindInfo->Version < PE_IMAGE_UNW_VER_2 ) {

		// try to unwind epilogue from code flow
		if ( PeVirtualUnwindEpilogueFromCode( Base, OptHeader, RuntimeEntry, UnwindInfo, Context ) ) {
			return ERR_SUCCESS;
		}

	} else if ( (UnwindInfo->CountOfCodes > 0) &&
				(UnwindInfo->UnwindCode[0].OpCode == PE_IMAGE_UWOP_EPILOG) ) {

		// if it's version 2 or higher then try to unwind epilogue from unwind information

		EpilogSize = UnwindInfo->UnwindCode[0].Epilogue.OffsetLowOrSize;

		// if this epilogue entry is valid then unwind it if rip is within
		if ( UnwindInfo->UnwindCode[0].Epilogue.OffsetHighOrFlags & PE_IMAGE_UWP_EPILOG_AT_THE_END ) {

			// is rip within epilogue?
			if ( (Rip >= EndAddr - EpilogSize) && (Rip < EndAddr) ) {

				Status = PeVirtualUnwindEpilogueFromUnwindInfo(
					Base,
					RuntimeEntry,
					(uint32_t)(Rip - (EndAddr - EpilogSize)),
					Context
				);
				return Status;

			}

		}

		// go through the list of epilogues and try to find the one
		for ( uint32_t i = 1; i < UnwindInfo->CountOfCodes; i++ ) {

			// did we reach epilogue codes end?
			if ( UnwindInfo->UnwindCode[i].OpCode != PE_IMAGE_UWOP_EPILOG ) {
				break;
			}

			EpilogOffset = UnwindInfo->UnwindCode[i].Epilogue.OffsetLowOrSize + (UnwindInfo->UnwindCode[i].Epilogue.OffsetHighOrFlags << 8);
			if ( EpilogOffset == 0 ) {
				// it's last code entry
				break;
			}

			// is rip within epilogue?
			if ( (Rip >= EndAddr - EpilogOffset) && (Rip < EndAddr - EpilogOffset + EpilogSize) ) {

				Status = PeVirtualUnwindEpilogueFromUnwindInfo(
					Base,
					RuntimeEntry,
					(uint32_t)(Rip - (EndAddr - EpilogOffset)),
					Context
				);
				return Status;

			}

		}

	}

	for ( ;; ) {

		// go through unwind code and unwind current stack frame
		for ( uint32_t i = 0; i < UnwindInfo->CountOfCodes; ) {

			// if the instruction is before current unwind slot, then we
			// should skip it
			if ( OffsetInProlog < UnwindInfo->UnwindCode[i].CodeOffset ) {

				// get code length
				Status = PeOpCodeLength( &CodeLength, UnwindInfo->UnwindCode[i] );
				if ( Status != ERR_SUCCESS ) {
					return Status;
				}

				// and go to next unwind slot
				i += CodeLength;

				continue;

			}

			// otherwise unwind current slot properly
			if ( UnwindInfo->UnwindCode[i].OpCode == PE_IMAGE_UWOP_PUSH_NONVOL ) {

				Context->Gpr[UnwindInfo->UnwindCode[i].OpInfo] = *((uint64_t*)Context->Rsp);
				Context->Rsp += 8;
				i += 1;

			} else if ( UnwindInfo->UnwindCode[i].OpCode == PE_IMAGE_UWOP_ALLOC_LARGE ) {

				if ( UnwindInfo->UnwindCode[i].OpInfo == 0 ) {
					FrameOffset = UnwindInfo->UnwindCode[i + 1].FrameOffset * 8;
					Context->Rsp += FrameOffset;
					i += 2;
				} else if ( UnwindInfo->UnwindCode[i].OpInfo == 1 ) {
					FrameOffset = UnwindInfo->UnwindCode[i + 1].FrameOffset + (UnwindInfo->UnwindCode[i + 2].FrameOffset << 16);
					Context->Rsp += FrameOffset;
					i += 3;
				} else {
					return ERR_PE_CORRUPED_PDATA;
				}

			} else if ( UnwindInfo->UnwindCode[i].OpCode == PE_IMAGE_UWOP_ALLOC_SMALL ) {

				FrameOffset = (UnwindInfo->UnwindCode[i].OpInfo * 8) + 8;
				Context->Rsp += FrameOffset;
				i += 1;

			} else if ( UnwindInfo->UnwindCode[i].OpCode == PE_IMAGE_UWOP_SET_FPREG ) {

				Context->Rsp = Context->Gpr[UnwindInfo->FrameRegister];
				Context->Rsp -= UnwindInfo->FrameOffset * 16;
				i += 1;

			} else if ( UnwindInfo->UnwindCode[i].OpCode == PE_IMAGE_UWOP_SAVE_NONVOL ) {

				FrameOffset = UnwindInfo->UnwindCode[i + 1].FrameOffset * 8;
				Context->Gpr[UnwindInfo->UnwindCode[i].OpInfo] = *(uint64_t*)(FrameBase + FrameOffset);
				i += 2;

			} else if ( UnwindInfo->UnwindCode[i].OpCode == PE_IMAGE_UWOP_SAVE_NONVOL_FAR ) {

				FrameOffset = UnwindInfo->UnwindCode[i + 1].FrameOffset + (UnwindInfo->UnwindCode[i + 2].FrameOffset << 16);
				Context->Gpr[UnwindInfo->UnwindCode[i].OpInfo] = *(uint64_t*)(FrameBase + FrameOffset);
				i += 3;

			} else if ( UnwindInfo->UnwindCode[i].OpCode == PE_IMAGE_UWOP_EPILOG ) {

				i += 2;

			} else if ( UnwindInfo->UnwindCode[i].OpCode == PE_IMAGE_UWOP_SPARE_CODE ) {

				i += 3;

			} else if ( UnwindInfo->UnwindCode[i].OpCode == PE_IMAGE_UWOP_SAVE_XMM128 ) {

				FrameOffset = UnwindInfo->UnwindCode[i + 1].FrameOffset * 16;

				memcpy(
					(void*)((uintptr_t)&Context->Xmm0 + (UnwindInfo->UnwindCode[i].OpInfo * 16)),
					(void*)(FrameBase + FrameOffset),
					16
				);

				i += 2;

			} else if ( UnwindInfo->UnwindCode[i].OpCode == PE_IMAGE_UWOP_SAVE_XMM128_FAR ) {

				FrameOffset = UnwindInfo->UnwindCode[i + 1].FrameOffset + (UnwindInfo->UnwindCode[i + 2].FrameOffset << 16);

				memcpy(
					(void*)((uintptr_t)&Context->Xmm0 + (UnwindInfo->UnwindCode[i].OpInfo * 16)),
					(void*)(FrameBase + FrameOffset),
					16
				);

				i += 3;

			} else if ( UnwindInfo->UnwindCode[i].OpCode == PE_IMAGE_UWOP_PUSH_MACHFRAME ) {

				if ( UnwindInfo->UnwindCode[i].OpInfo > 1 ) {
					return ERR_PE_CORRUPED_PDATA;
				}

				// skip error code in stack
				if ( UnwindInfo->UnwindCode[i].OpInfo == 1 ) {
					Context->Rsp += 8;
				}

				Context->Rip = *((uint64_t*)Context->Rsp);
				Context->Rsp += 8;

				Context->SegCs = *((uint16_t*)Context->Rsp);
				Context->Rsp += 8;

				Context->EFlags = *((uint32_t*)Context->Rsp);
				Context->Rsp += 8;

				Context->SegSs = *((uint16_t*)((uintptr_t)(Context->Rsp + 8)));
				Context->Rsp = *((uint64_t*)Context->Rsp);

				SkipReturnUnwind = true;

				i += 1;

			} else {

				return ERR_PE_CORRUPED_PDATA;

			}

		}

		if ( UnwindInfo->Flags & PE_IMAGE_UNW_FLAG_CHAININFO ) {

			// get pointers to next chain
			RuntimeEntry = (SRuntimeFunction*)&UnwindInfo->UnwindCode[(UnwindInfo->CountOfCodes + 1) & ~1];
			UnwindInfo = (SUnwindInfo*)((uintptr_t)Base + RuntimeEntry->UnwindInfo);

			// we can't unwind stack frame
			if ( UnwindInfo->Version > PE_IMAGE_UNW_VER_MAX ) {
				return ERR_PE_UNSUPPORTED_PDATA;
			}

			// update addresses
			BeginAddr = (uintptr_t)Base + RuntimeEntry->BeginAddress;
			EndAddr = (uintptr_t)Base + RuntimeEntry->EndAddress;

			// update offset in prologue
			OffsetInProlog = (uint32_t)(Rip - BeginAddr);

			// update chain depth and check if we have reached depth limit
			ChainDepth++;
			if ( ChainDepth > 32 ) {
				return ERR_PE_CORRUPED_PDATA;
			}

			continue;

		}

		break;

	}

	// if there wasn't machine frame then unwind return
	if ( !SkipReturnUnwind ) {
		Context->Rip = *((uint64_t*)Context->Rsp);
		Context->Rsp += 8;
	}

	// return handler for stack frame
	if ( (OffsetInProlog >= UnwindInfo->SizeOfProlog) &&
		 (UnwindInfo->Flags & HandlerType) ) {

		*Handler = (void*)(*((uint32_t*)&UnwindInfo->UnwindCode[(UnwindInfo->CountOfCodes + 1) & ~1]) + (uintptr_t)Base);
		*HandlerData = &UnwindInfo->UnwindCode[((UnwindInfo->CountOfCodes + 1) & ~1) + 2];

	}

	// return entry for stack frame
	*FunctionEntry = RuntimeEntry;

	return ERR_SUCCESS;

}
