#include "..\..\types\types.h"

/***************************************************************************************\
|	Function implementations															|
\***************************************************************************************/

void* memset( void *ptr, int value, size_t num ) {

	uint64_t	locval;
	void		*locptr;

	locval = (uint8_t)value;
	locval |= locval << 8;
	locval |= locval << 16;
	locval |= locval << 32;
	locptr = ptr;

	while ( num >= sizeof(uint64_t) ) {
		*((uint64_t*)ptr) = locval;
		ptr = (void*)((uint64_t*)ptr + 1);
		num -= sizeof(uint64_t);
	}

	if ( num >= sizeof(uint32_t) ) {
		*((uint32_t*)ptr) = (uint32_t)locval;
		ptr = (void*)((uint32_t*)ptr + 1);
		num -= sizeof(uint32_t);
	}

	if ( num >= sizeof(uint16_t) ) {
		*((uint16_t*)ptr) = (uint16_t)locval;
		ptr = (void*)((uint16_t*)ptr + 1);
		num -= sizeof(uint16_t);
	}

	if ( num >= sizeof(uint8_t) ) {
		*((uint8_t*)ptr) = (uint8_t)locval;
	}

	return locptr;

}

void* memcpy( void *dest, const void* src, size_t num ) {

	void *locdest;

	locdest = dest;

	while ( num >= sizeof(uint64_t) ) {
		*((uint64_t*)dest) = *((uint64_t*)src);
		dest = (void*)((uint64_t*)dest + 1);
		src = (void*)((uint64_t*)src + 1);
		num -= sizeof(uint64_t);
	}

	if ( num >= sizeof(uint32_t) ) {
		*((uint32_t*)dest) = *((uint32_t*)src);
		dest = (void*)((uint32_t*)dest + 1);
		src = (void*)((uint32_t*)src + 1);
		num -= sizeof(uint32_t);
	}

	if ( num >= sizeof(uint16_t) ) {
		*((uint16_t*)dest) = *((uint16_t*)src);
		dest = (void*)((uint16_t*)dest + 1);
		src = (void*)((uint16_t*)src + 1);
		num -= sizeof(uint16_t);
	}

	if ( num >= sizeof(uint8_t) ) {
		*((uint8_t*)dest) = *((uint8_t*)src);
	}

	return locdest;

}

int memcmp( const void *ptr1, const void *ptr2, size_t num ) {

	while ( num > 0 ) {

		if ( *((uint8_t*)ptr1) < *((uint8_t*)ptr2) ) {
			return -1;
		} else if ( *((uint8_t*)ptr1) > *((uint8_t*)ptr2) ) {
			return 1;
		}

		ptr1 = (void*)((uintptr_t)ptr1 + 1);
		ptr2 = (void*)((uintptr_t)ptr2 + 1);

		num--;

	}

	return 0;

}
