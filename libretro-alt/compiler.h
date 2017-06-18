/**
 * @file	compiler.h
 * @brief	include file for standard system include files,
 *			or project specific include files that are used frequently,
 *			but are changed infrequently
 */

#pragma once

#ifdef __OBJC__
#import <UIKit/UIKit.h>
#endif

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <stdarg.h>
#include <string.h>

#define SDL_arraysize(array)   (sizeof(array)/sizeof(array[0]))
#define SDL_Delay(a) usleep((a)*1000)

#define	BYTESEX_LITTLE
#define	OSLANG_UTF8
#define	OSLINEBREAK_CRLF
#define RESOURCE_US

#ifdef __WIN32__
#define	sigjmp_buf				jmp_buf
#define	sigsetjmp(env, mask)	setjmp(env)
#define	siglongjmp(env, val)	longjmp(env, val)
#endif

typedef	signed int			SINT;
typedef	unsigned int		UINT;
typedef	signed char			SINT8;
typedef	unsigned char		UINT8;
typedef	signed short		SINT16;
typedef	unsigned short		UINT16;
typedef	signed int			SINT32;
typedef	unsigned int		UINT32;
typedef	int64_t		SINT64;
typedef	uint64_t		UINT64;

typedef  int32_t*    INTPTR;

#define	msgbox(title, msg)

#define	BRESULT				UINT
#define	OEMCHAR				char
#define	OEMTEXT(string)		string
#define	OEMSPRINTF			sprintf
#define	OEMSTRLEN			strlen

#define SIZE_VGA
#if !defined(SIZE_VGA)
#define	RGB16		UINT32
#define	SIZE_QVGA
#endif

typedef	signed char		CHAR;
typedef	unsigned char	BYTE;

#if !defined(OBJC_BOOL_DEFINED)
typedef signed char BOOL;
#endif

#ifndef	TRUE
#define	TRUE	1
#endif

#ifndef	FALSE
#define	FALSE	0
#endif

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
#define	CopyMemory(d,s,n)	memcpy((d), (s), (n))
#endif
#ifndef	FillMemory
#define	FillMemory(a, b, c)	memset((a), (c), (b))
#endif

#include "common.h"
#include "milstr.h"
#include "_memory.h"
#include "rect.h"
#include "lstarray.h"
#include "trace.h"

#define  TRACE

#define	GETTICK()			GetTicks()
#define	__ASSERT(s)
#define	SPRINTF				sprintf
#define	STRLEN				strlen

#define	SUPPORT_UTF8

#define	SUPPORT_16BPP
#define	MEMOPTIMIZE		2

//#define SOUND_CRITICAL
#define	SOUNDRESERVE	100

#define	SUPPORT_OPM

#define	SUPPORT_CRT15KHZ
#define	SUPPORT_HOSTDRV
#define	SUPPORT_SWSEEKSND
#define	SUPPORT_SASI
#define	SUPPORT_SCSI

#define	SUPPORT_CRT31KHZ
#define	SUPPORT_SWSEEKSND
#define  SUPPORT_PC9821

#if defined(SUPPORT_PC9821)
#define	CPUCORE_IA32
#define  USE_FPU
#define	IA32_PAGING_EACHSIZE
#define	SUPPORT_PC9801_119
#endif
#define	SUPPORT_PC9861K
#define	SUPPORT_SOFTKBD		0
#define  SUPPORT_S98

//#define SUPPORT_EXTERNALCHIP
//tosee
#define	SUPPORT_RESUME
#define	SUPPORT_STATSAVE	10
#define	SUPPORT_ROMEO

#define	SUPPORT_NORMALDISP

#define SUPPORT_ARC

#define	SCREEN_BPP		16

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
