/**
 * @file	compiler.h
 * @brief	include file for standard system include files,
 *			or project specific include files that are used frequently,
 *			but are changed infrequently
 */

#pragma once

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

#include <boolean.h>

#define	BYTESEX_LITTLE
#define	OSLANG_UTF8
#define	OSLINEBREAK_CRLF
#define  RESOURCE_US

typedef	signed int		SINT;
typedef	unsigned int	UINT;

typedef	int8_t		SINT8;
typedef	uint8_t		UINT8;
typedef	int16_t		SINT16;
typedef	uint16_t		UINT16;
typedef	int32_t		SINT32;
typedef	uint32_t		UINT32;

typedef  int32_t*    INTPTR;

typedef bool BOOL;
#ifndef	TRUE
#define	TRUE	true
#endif
#ifndef	FALSE
#define	FALSE	false
#endif

//#define PTR_TO_UINT32(p)	((UINT32)GPOINTER_TO_UINT(p))
//#define UINT32_TO_PTR(v)	GUINT_TO_POINTER((UINT32)(v))

#define	BRESULT				UINT
#define	OEMCHAR				char
#define	OEMTEXT(string)	string
#define	OEMSPRINTF			sprintf
#define	OEMSTRLEN			strlen

#define	SPRINTF		sprintf
#define	STRLEN		strlen
#define	GETRAND()	random()

#define SIZE_VGA

#ifndef	MAX_PATH
#define	MAX_PATH	256
#endif

#ifndef	max
#define	max(a,b)	(((a) > (b)) ? (a) : (b))
#endif
#ifndef	min
#define	min(a,b)	(((a) < (b)) ? (a) : (b))
#endif

#ifndef	ZeroMemory
#define	ZeroMemory(d,n)		memset((d), 0, (n))
#endif
#ifndef	CopyMemory
#define	CopyMemory(d,s,n)    memcpy((d), (s), (n))
#endif
#ifndef	FillMemory
#define	FillMemory(a, b, c)	memset((a), (c), (b))
#endif

#ifndef	roundup
#define	roundup(x, y)	((((x)+((y)-1))/(y))*(y))
#endif

#ifndef	NELEMENTS
#define	NELEMENTS(a)	((int)(sizeof(a) / sizeof(a[0])))
#endif

#define	msgbox(title, msg)	toolkit_messagebox(title, msg);

#include "common.h"
#include "milstr.h"
#include "_memory.h"
#include "rect.h"
#include "lstarray.h"
#include "trace.h"


#define	GETTICK()			SDL_GetTicks()
#define	__ASSERT(s)
#define	SPRINTF				sprintf
#define	STRLEN				strlen


#define	MEMOPTIMIZE		2

#define	VERMOUTH_LIB
#define  SOUND_CRITICAL
#define	SOUNDRESERVE	100
#define	SUPPORT_SWSEEKSND

#define	SCREEN_BPP		16


//extras
#define	SUPPORT_EUC
#define	SUPPORT_UTF8

#define	SUPPORT_CRT31KHZ
#define	SUPPORT_SWSEEKSND
#define  SUPPORT_PC9821

#if defined(SUPPORT_PC9821)
#define	CPUCORE_IA32
#define	IA32_PAGING_EACHSIZE
#define	SUPPORT_PC9801_119
#endif

#define	SUPPORT_PC9861K
#define	SUPPORT_SOFTKBD		0
#define  SUPPORT_S98
//#define  SUPPORT_WAVEREC
//#define  SUPPORT_RECVIDEO
#define	SUPPORT_KEYDISP
//#define	SUPPORT_MEMDBG32
#define	SUPPORT_HOSTDRV
#define	SUPPORT_IDEIO
#define	SUPPORT_SASI
#define	SUPPORT_SCSI

#define	SUPPORT_RESUME
#define	SUPPORT_STATSAVE	10
#define	SUPPORT_ROMEO

#define  SOUND_CRITICAL
#define	SOUNDRESERVE	100
#define	SUPPORT_EXTERNALCHIP

#define  SUPPORT_ARC
#define  SUPPORT_ZLIB

#define	SUPPORT_16BPP
#define	SUPPORT_NORMALDISP

//outdated things to ignore
#define	FASTCALL
#define	CPUCALL
#define	MEMCALL
#define	DMACCALL
#define	IOOUTCALL
#define	IOINPCALL
#define	SOUNDCALL
#define	VRAMCALL
#define	SCRNCALL
#define	VERMOUTHCL

