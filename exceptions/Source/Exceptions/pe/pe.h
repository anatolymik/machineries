#pragma once

/***************************************************************************************\
|	Architecture includes																|
\***************************************************************************************/

#include "amd64\peunwind.h"

/***************************************************************************************\
|	Functions																			|
\***************************************************************************************/

uint32_t	PeRecognize( void *Base );
uint32_t	PeGetImageSize( void *Base );