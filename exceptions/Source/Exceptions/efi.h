#pragma once

/***************************************************************************************\
|	Macro																				|
\***************************************************************************************/

#define EFIAPI				__cdecl

#define EFI_SUCCESS			0

/***************************************************************************************\
|	Types																				|
\***************************************************************************************/

typedef uint64_t	UINT64;
typedef int64_t		INT64;
typedef uint32_t	UINT32;
typedef int32_t		INT32;
typedef uint16_t	UINT16;
typedef int16_t		INT16;
typedef uint8_t		UINT8;
typedef int8_t		INT8;

typedef intptr_t	INTN;
typedef uintptr_t	UINTN;

typedef UINT16		CHAR16;
typedef UINT8		CHAR8;
typedef UINT8		BOOLEAN;

typedef UINTN		EFI_STATUS;
typedef	void		*EFI_HANDLE;

/***************************************************************************************\
|	Structure declarations																|
\***************************************************************************************/

struct	EFI_RUNTIME_SERVICES;
struct	EFI_BOOT_SERVICES;
struct	EFI_CONFIGURATION_TABLE;

struct	SIMPLE_INPUT_INTERFACE;
struct	SIMPLE_TEXT_OUTPUT_INTERFACE;

/***************************************************************************************\
|	Prototypes																			|
\***************************************************************************************/

typedef EFI_STATUS (EFIAPI *EFI_TEXT_RESET)( SIMPLE_TEXT_OUTPUT_INTERFACE *This, BOOLEAN ExtendedVerification );
typedef EFI_STATUS (EFIAPI *EFI_TEXT_OUTPUT_STRING)( SIMPLE_TEXT_OUTPUT_INTERFACE *This, CHAR16 *String );
typedef EFI_STATUS (EFIAPI *EFI_TEXT_TEST_STRING)( SIMPLE_TEXT_OUTPUT_INTERFACE *This, CHAR16 *String );
typedef EFI_STATUS (EFIAPI *EFI_TEXT_QUERY_MODE)( SIMPLE_TEXT_OUTPUT_INTERFACE *This, UINTN ModeNumber, UINTN *Columns, UINTN *Rows );
typedef EFI_STATUS (EFIAPI *EFI_TEXT_SET_MODE)( SIMPLE_TEXT_OUTPUT_INTERFACE *This, UINTN ModeNumber );
typedef EFI_STATUS (EFIAPI *EFI_TEXT_SET_ATTRIBUTE)( SIMPLE_TEXT_OUTPUT_INTERFACE *This, UINTN Attribute );
typedef EFI_STATUS (EFIAPI *EFI_TEXT_CLEAR_SCREEN)( SIMPLE_TEXT_OUTPUT_INTERFACE *This );
typedef EFI_STATUS (EFIAPI *EFI_TEXT_SET_CURSOR_POSITION)( SIMPLE_TEXT_OUTPUT_INTERFACE *This, UINTN Column, UINTN Row );
typedef EFI_STATUS (EFIAPI *EFI_TEXT_ENABLE_CURSOR)( SIMPLE_TEXT_OUTPUT_INTERFACE *This, BOOLEAN Enable );

/***************************************************************************************\
|	Structures																			|
\***************************************************************************************/

struct SIMPLE_TEXT_OUTPUT_MODE {

	INT32							MaxMode;
	INT32							Mode;
	INT32							Attribute;
	INT32							CursorColumn;
	INT32							CursorRow;
	BOOLEAN							CursorVisible;

};

struct SIMPLE_TEXT_OUTPUT_INTERFACE {

	EFI_TEXT_RESET					Reset;

	EFI_TEXT_OUTPUT_STRING			OutputString;
	EFI_TEXT_TEST_STRING			TestString;

	EFI_TEXT_QUERY_MODE				QueryMode;
	EFI_TEXT_SET_MODE				SetMode;
	EFI_TEXT_SET_ATTRIBUTE			SetAttribute;

	EFI_TEXT_CLEAR_SCREEN			ClearScreen;
	EFI_TEXT_SET_CURSOR_POSITION	SetCursorPosition;
	EFI_TEXT_ENABLE_CURSOR			EnableCursor;

	SIMPLE_TEXT_OUTPUT_MODE			*Mode;

};

struct EFI_TABLE_HEADER {

	UINT64							Signature;
	UINT32							Revision;
	UINT32							HeaderSize;
	UINT32							CRC32;
	UINT32							Reserved;

};

struct EFI_SYSTEM_TABLE {

	EFI_TABLE_HEADER				Hdr;

	CHAR16							*FirmwareVendor;
	UINT32							FirmwareRevision;

	EFI_HANDLE						ConsoleInHandle;
	SIMPLE_INPUT_INTERFACE			*ConIn;

	EFI_HANDLE						ConsoleOutHandle;
	SIMPLE_TEXT_OUTPUT_INTERFACE	*ConOut;

	EFI_HANDLE						StandardErrorHandle;
	SIMPLE_TEXT_OUTPUT_INTERFACE	*StdErr;

	EFI_RUNTIME_SERVICES			*RuntimeServices;
	EFI_BOOT_SERVICES				*BootServices;

	UINTN							NumberOfTableEntries;
	EFI_CONFIGURATION_TABLE			*ConfigurationTable;

};
