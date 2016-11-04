#include "basic\basic.h"

#include "efi.h"

#include "pe\pe.h"
#include "exc\exc.h"

#pragma warning( push )
#pragma warning( disable : 4100 )

/***************************************************************************************\
|	Intrinsic functions																	|
\***************************************************************************************/

extern "C" void	__sidt( void *Destination );
#pragma intrinsic(__sidt)

/***************************************************************************************\
|	Internal variables																	|
\***************************************************************************************/

static void								*g_ImageBase;
static void								*g_ImageEnd;

static SIMPLE_TEXT_OUTPUT_INTERFACE		*g_Output;

/***************************************************************************************\
|	Internal functions																	|
\***************************************************************************************/

static void Print( wchar_t *String ) {

	g_Output->OutputString( g_Output, (CHAR16*)String );

}

static void PrintInteger( uint32_t i ) {

	CHAR16	String[16];
	CHAR16	*CurPos;

	CurPos = &String[15];
	*CurPos = 0;
	do {
		CurPos--;
		*CurPos = (i % 10) + L'0';
		i /= 10;
	} while ( i );

	g_Output->OutputString( g_Output, CurPos );

}

static void PrintHex( uint32_t i ) {

	CHAR16	String[16];
	CHAR16	*CurPos;

	CurPos = &String[15];
	*CurPos = 0;
	do {

		CurPos--;
		*CurPos = (i & 0xf) + L'0';
		if ( *CurPos > L'9' ) {
			*CurPos += L'A' - L'9' + 1;
		}

		CurPos--;
		*CurPos = ((i >> 4) & 0xf) + L'0';
		if ( *CurPos > L'9' ) {
			*CurPos += L'A' - L'9' + 1;
		}

		i = i >> 8;

	} while ( i );

	CurPos--;
	*CurPos = L'x';

	CurPos--;
	*CurPos = L'0';

	g_Output->OutputString( g_Output, CurPos );

}

static void* GetImageBase( void *Rip ) {

	// adequate image base searching should be placed here

	if ( Rip < g_ImageBase ) {
		return NULL;
	}

	if ( Rip >= g_ImageEnd ) {
		return NULL;
	}

	return g_ImageBase;

}

static void GetStackLimits( uintptr_t *LowLimit, uintptr_t *HighLimit ) {

	// adequate stack limits should be returned here

	*LowLimit = 0;
	*HighLimit = 0;
	(*HighLimit)--;

}

int ExceptionTest( uint32_t *abnormals ) {

	int				a;
	volatile int	c;

	// no exceptions
	a = 0;
	c = 1;

	// without exception
	__try {
		__try {

			__try {

				a = 10 / c;

			} __finally {

				if ( AbnormalTermination() ) {
					Print( L"abnormal finally 1\n\r" );
					(*abnormals)++;
				} else {
					Print( L"normal finally 1\n\r" );
				}

			}

		} __finally {

			if ( AbnormalTermination() ) {
				Print( L"abnormal finally 2\n\r" );
				(*abnormals)++;
			} else {
				Print( L"normal finally 2\n\r" );
			}

		}
	} __except ( EXCEPTION_EXECUTE_HANDLER ) {
		Print( L"catched(1) " );
		PrintHex( GetExceptionCode() );
		Print( L", abnormals: " );
		PrintInteger( *abnormals );
		Print( L"\n\r" );
	}

	// to force compiler skip optimization for previous
	// division
	if ( a > 10 ) {
		return a;
	}

	// further only exceptions
	a = 0;
	c = 0;

	// with exception
	__try {
		__try {

			__try {

				a = 10 / c;

			} __finally {

				if ( AbnormalTermination() ) {
					Print( L"abnormal finally 3\n\r" );
					(*abnormals)++;
				} else {
					Print( L"normal finally 3\n\r" );
				}

			}

		} __finally {

			if ( AbnormalTermination() ) {
				Print( L"abnormal finally 4\n\r" );
				(*abnormals)++;
			} else {
				Print( L"normal finally 4\n\r" );
			}

		}
	} __except ( EXCEPTION_EXECUTE_HANDLER ) {
		Print( L"catched(2) " );
		PrintHex( GetExceptionCode() );
		Print( L", abnormals: " );
		PrintInteger( *abnormals );
		Print( L"\n\r" );
	}

	// to force compiler skip optimization for previous
	// division
	if ( a > 0 ) {
		return a;
	}

	// exceptions without catching in this function
	__try {

		__try {

			__try {

				a = 10 / c;

			} __finally {

				if ( AbnormalTermination() ) {
					Print( L"abnormal finally 5\n\r" );
					(*abnormals)++;
				} else {
					Print( L"normal finally 5\n\r" );
				}

			}

		} __finally {

			if ( AbnormalTermination() ) {
				Print( L"abnormal finally 6\n\r" );
				(*abnormals)++;
			} else {
				Print( L"normal finally 6\n\r" );
			}

		}

	} __finally {

		if ( AbnormalTermination() ) {
			Print( L"abnormal finally 7\n\r" );
			(*abnormals)++;
		} else {
			Print( L"normal finally 7\n\r" );
		}

	}

	Print( L"func return\n\r" );

	// to force compiler skip optimization for previous
	// division
	return a;

}

EFI_STATUS EfiMain( EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable ) {

	SIdtr		Idtr;
	SIntDesc	IdtTable[CPU_EXCEPTION_VECTORS];
	uint32_t	IdtLength;

	uint32_t	abnormals = 0;

	uint32_t	Status;

	g_Output = SystemTable->ConOut;

	Print( L"Exceptions using example\r\n" );

	// search image base
	g_ImageBase = (void*)((uintptr_t)EfiMain & ~(4096 - 1));
	for ( ; ; g_ImageBase = (void*)((uintptr_t)g_ImageBase - 4096) ) {
		Status = PeRecognize( g_ImageBase );
		if ( Status == ERR_SUCCESS ) {
			break;
		}
	}
	g_ImageEnd = (void*)((uintptr_t)g_ImageBase + PeGetImageSize( g_ImageBase ));

	// initialize exception dispatcher function callbacks
	g_GetStackLimitsFunc = GetStackLimits;
	g_GetImageBaseFunc = GetImageBase;

	// get current CPU idt
	__sidt( &Idtr );

	// copy its exception vectors
	IdtLength = (Idtr.Limit + 1) / sizeof(SIntDesc);
	if ( IdtLength > CPU_EXCEPTION_VECTORS ) {
		IdtLength = CPU_EXCEPTION_VECTORS;
	}
	IdtLength *= sizeof(SIntDesc);
	memcpy( IdtTable, (void*)Idtr.Base, IdtLength );

	// and reinitialize them for our own
	CpuInitializeExceptionHandlers( &Idtr );

	// test exceptions
	__try {
		__try {
			__try {
				ExceptionTest( &abnormals );
			} __finally {
				if ( AbnormalTermination() ) {
					Print( L"outer abnormal finally 1\n\r" );
					abnormals++;
				} else {
					Print( L"outer normal finally 1\n\r" );
				}
			}
		} __finally {
			if ( AbnormalTermination() ) {
				Print( L"outer abnormal finally 2\n\r" );
				abnormals++;
			} else {
				Print( L"outer normal finally 2\n\r" );
			}
		}
	} __except ( EXCEPTION_EXECUTE_HANDLER ) {
		Print( L"outer catched " );
		PrintHex( GetExceptionCode() );
		Print( L", abnormals: " );
		PrintInteger( abnormals );
		Print( L"\n\r" );
	}

	// restore original exception vectors
	memcpy( (void*)Idtr.Base, IdtTable, IdtLength );

	Print( L"That's all\r\n" );

	return EFI_SUCCESS;

}
