// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#include <stdio.h>
#include <tchar.h>
#include <wtypes.h>
#include <map>
#include <vector>

#include "RzChromaSDKDefines.h"
#include "RzChromaSDKTypes.h"
#include "RzErrors.h"

#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )

#ifdef _MSC_VER
#if _MSC_VER >= 1600
#include <cstdint>
#else
typedef __int8              int8_t;
typedef __int16             int16_t;
typedef __int32             int32_t;
typedef __int64             int64_t;
typedef unsigned __int8     uint8_t;
typedef unsigned __int16    uint16_t;
typedef unsigned __int32    uint32_t;
typedef unsigned __int64    uint64_t;
#endif
#elif __GNUC__ >= 3
#include <cstdint>
#endif

typedef int8_t      s8;
typedef int16_t     s16;
typedef int32_t     s32;
typedef int64_t     s64;
typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64; 

PACK(
typedef union {
	struct {
		// Root Layer
		uint16_t preamble_size;
		uint16_t postamble_size;
		uint8_t  acn_id[12];
		uint16_t root_flength;
		uint32_t root_vector;
		uint8_t  cid[16];

		// Frame Layer
		uint16_t frame_flength;
		uint32_t frame_vector;
		uint8_t  source_name[64];
		uint8_t  priority;
		uint16_t reserved;
		uint8_t  sequence_number;
		uint8_t  options;
		uint16_t universe;

		// DMP Layer
		uint16_t dmp_flength;
		uint8_t  dmp_vector;
		uint8_t  type;
		uint16_t first_address;
		uint16_t address_increment;
		uint16_t property_value_count;
		uint8_t  property_values[513];
	};
	uint8_t raw[638];
} e131_packet_t);
