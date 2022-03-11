/*----------------------------------------------------------------------------/
/  FatFs - Generic FAT Filesystem Module  R0.14b                              /
/-----------------------------------------------------------------------------/
/
/ Copyright (C) 2021, ChaN, all right reserved.
/
/ FatFs module is an open source software. Redistribution and use of FatFs in
/ source and binary forms, with or without modification, are permitted provided
/ that the following condition is met:
/
/ 1. Redistributions of source code must retain the above copyright notice,
/    this condition and the following disclaimer.
/
/ This software is provided by the copyright holder and contributors "AS IS"
/ and any warranties related to this software are DISCLAIMED.
/ The copyright owner or contributors be NOT LIABLE for any damages caused
/ by use of this software.
/
/----------------------------------------------------------------------------*/


// C++ port of the original source code is subject to MIT License

// Copyright (c) 2022 Chris Sutton

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef __FF_COMMON_HPP__
#define __FF_COMMON_HPP__

#include <ff_types.hpp>

namespace fatfs 
{

    #if FF_FS_RPATH != 0
static BYTE CurrVol;				/* Current drive */
#endif

/*--------------------------------*/
/* LFN/Directory working buffer   */
/*--------------------------------*/

#if FF_USE_LFN == 0		/* Non-LFN configuration */
#if FF_FS_EXFAT
#error LFN must be enabled when enable exFAT
#endif
#define DEF_NAMBUF
#define INIT_NAMBUF(fs)
#define FREE_NAMBUF()
#define LEAVE_MKFS(res)	return res

#else					/* LFN configurations */
#if FF_MAX_LFN < 12 || FF_MAX_LFN > 255
#error Wrong setting of FF_MAX_LFN
#endif
#if FF_LFN_BUF < FF_SFN_BUF || FF_SFN_BUF < 12
#error Wrong setting of FF_LFN_BUF or FF_SFN_BUF
#endif
#if FF_LFN_UNICODE < 0 || FF_LFN_UNICODE > 3
#error Wrong setting of FF_LFN_UNICODE
#endif
static const BYTE LfnOfs[] = {1,3,5,7,9,14,16,18,20,22,24,28,30};	/* FAT: Offset of LFN characters in the directory entry */
#define MAXDIRB(nc)	((nc + 44U) / 15 * SZDIRE)	/* exFAT: Size of directory entry block scratchpad buffer needed for the name length */

#if FF_USE_LFN == 1		/* LFN enabled with static working buffer */
#if FF_FS_EXFAT
[[maybe_unused]] static BYTE	DirBuf[MAXDIRB(FF_MAX_LFN)] ;	/* Directory entry block scratchpad buffer */
#endif
[[maybe_unused]] static WCHAR LfnBuf[FF_MAX_LFN + 1];		/* LFN working buffer */
#define DEF_NAMBUF
#define INIT_NAMBUF(fs)
#define FREE_NAMBUF()
#define LEAVE_MKFS(res)	return res

#elif FF_USE_LFN == 2 	/* LFN enabled with dynamic working buffer on the stack */
#if FF_FS_EXFAT
#define DEF_NAMBUF		WCHAR lbuf[FF_MAX_LFN+1]; BYTE dbuf[MAXDIRB(FF_MAX_LFN)];	/* LFN working buffer and directory entry block scratchpad buffer */
#define INIT_NAMBUF(fs)	{ (fs)->lfnbuf = lbuf; (fs)->dirbuf = dbuf; }
#define FREE_NAMBUF()
#else
#define DEF_NAMBUF		WCHAR lbuf[FF_MAX_LFN+1];	/* LFN working buffer */
#define INIT_NAMBUF(fs)	{ (fs)->lfnbuf = lbuf; }
#define FREE_NAMBUF()
#endif
#define LEAVE_MKFS(res)	return res

#elif FF_USE_LFN == 3 	/* LFN enabled with dynamic working buffer on the heap */
#if FF_FS_EXFAT
#define DEF_NAMBUF		WCHAR *lfn;	/* Pointer to LFN working buffer and directory entry block scratchpad buffer */
#define INIT_NAMBUF(fs)	{ lfn = ff_memalloc((FF_MAX_LFN+1)*2 + MAXDIRB(FF_MAX_LFN)); if (!lfn) LEAVE_FF(fs, FR_NOT_ENOUGH_CORE); (fs)->lfnbuf = lfn; (fs)->dirbuf = (BYTE*)(lfn+FF_MAX_LFN+1); }
#define FREE_NAMBUF()	ff_memfree(lfn)
#else
#define DEF_NAMBUF		WCHAR *lfn;	/* Pointer to LFN working buffer */
#define INIT_NAMBUF(fs)	{ lfn = ff_memalloc((FF_MAX_LFN+1)*2); if (!lfn) LEAVE_FF(fs, FR_NOT_ENOUGH_CORE); (fs)->lfnbuf = lfn; }
#define FREE_NAMBUF()	ff_memfree(lfn)
#endif
#define LEAVE_MKFS(res)	{ if (!work) ff_memfree(buf); return res; }
#define MAX_MALLOC	0x8000	/* Must be >=FF_MAX_SS */

#else
#error Wrong setting of FF_USE_LFN

#endif	/* FF_USE_LFN == 1 */
#endif	/* FF_USE_LFN == 0 */


class DriverCommon
{
public:
    DriverCommon() = default;

	/*-----------------------------------------------------------------------*/
	/* Load/Store multi-byte word in the FAT structure                       */
	/*-----------------------------------------------------------------------*/

	// Load a 2-byte little-endian word 
	WORD ld_word (const BYTE* ptr);

	// Load a 4-byte little-endian word
	DWORD ld_dword (const BYTE* ptr);

	// Load an 8-byte little-endian word
	QWORD ld_qword (const BYTE* ptr);	
#if !FF_FS_READONLY
	// Store a 2-byte word in little-endian
	void st_word (BYTE* ptr, WORD val);

	// Store a 4-byte word in little-endian
	void st_dword (BYTE* ptr, DWORD val);		
#if FF_FS_EXFAT
	// Store an 8-byte word in little-endian 
	void st_qword (BYTE* ptr, QWORD val);
#endif
#endif	/* !FF_FS_READONLY */	

	/*-----------------------------------------------------------------------*/
	/* String functions                                                      */
	/*-----------------------------------------------------------------------*/

	// Test if the byte is DBC 1st byte 
	int dbc_1st (BYTE c);	

	// Test if the byte is DBC 2nd byte
	int dbc_2nd (BYTE c);

#if FF_USE_LFN
	// Get a Unicode code point from the TCHAR string in defined API encodeing  
	DWORD tchar2uni (	/* Returns a character in UTF-16 encoding (>=0x10000 on surrogate pair, 0xFFFFFFFF on decode error) */
		const TCHAR** str		/* Pointer to pointer to TCHAR string in configured encoding */
	);

	// Store a Unicode char in defined API encoding
	UINT put_utf (	/* Returns number of encoding units written (0:buffer overflow or wrong encoding) */
		DWORD chr,	/* UTF-16 encoded character (Surrogate pair if >=0x10000) */
		TCHAR* buf,	/* Output buffer */
		UINT szb	/* Size of the buffer */
	);
#endif	/* FF_USE_LFN */

#if FF_FS_REENTRANT
	/*-----------------------------------------------------------------------*/
	/* Request/Release grant to access the volume                            */
	/*-----------------------------------------------------------------------*/
	int lock_fs (		/* 1:Ok, 0:timeout */
		FATFS* fs		/* Filesystem object */
	);

	void unlock_fs (
		FATFS* fs,		/* Filesystem object */
		FRESULT res		/* Result code to be returned */
	);

#endif // FF_FS_REENTRANT

#if FF_FS_LOCK != 0
	/*-----------------------------------------------------------------------*/
	/* File lock control functions                                           */
	/*-----------------------------------------------------------------------*/

	FRESULT chk_lock (	/* Check if the file can be accessed */
		DIR* dp,		/* Directory object pointing the file to be checked */
		int acc			/* Desired access type (0:Read mode open, 1:Write mode open, 2:Delete or rename) */
	);

	int enq_lock (void);

	UINT inc_lock (	/* Increment object open counter and returns its index (0:Internal error) */
		DIR* dp,	/* Directory object pointing the file to register or increment */
		int acc		/* Desired access (0:Read, 1:Write, 2:Delete/Rename) */
	);

	FRESULT dec_lock (	/* Decrement object open counter */
		UINT i			/* Semaphore index (1..) */
	);

	void clear_lock (	/* Clear lock entries of the volume */
		FATFS *fs
	);
#endif	/* FF_FS_LOCK != 0 */

	/*-----------------------------------------------------------------------*/
	/* Get physical sector number from cluster number                        */
	/*-----------------------------------------------------------------------*/

	LBA_t clst2sect (	/* !=0:Sector number, 0:Failed (invalid cluster#) */
		FATFS* fs,		/* Filesystem object */
		DWORD clst		/* Cluster# to be converted */
	);

    #if FF_USE_FASTSEEK
	/*-----------------------------------------------------------------------*/
	/* FAT handling - Convert offset into cluster with link map table        */
	/*-----------------------------------------------------------------------*/

	DWORD clmt_clust (	/* <2:Error, >=2:Cluster number */
		FIL* fp,		/* Pointer to the file object */
		FSIZE_t ofs		/* File offset to be converted to cluster# */
	);
#endif	/* FF_USE_FASTSEEK */


	/*-----------------------------------------------------------------------*/
	/* FAT: Directory handling - Load/Store start cluster number             */
	/*-----------------------------------------------------------------------*/

	DWORD ld_clust (	/* Returns the top cluster value of the SFN entry */
		FATFS* fs,			/* Pointer to the fs object */
		const BYTE* dir		/* Pointer to the key entry */
	);

#if !FF_FS_READONLY
	void st_clust (
		FATFS* fs,	/* Pointer to the fs object */
		BYTE* dir,	/* Pointer to the key entry */
		DWORD cl	/* Value to be set */
	);
#endif

#if FF_USE_LFN
	/*--------------------------------------------------------*/
	/* FAT-LFN: Compare a part of file name with an LFN entry */
	/*--------------------------------------------------------*/

	int cmp_lfn (		/* 1:matched, 0:not matched */
		const WCHAR* lfnbuf,	/* Pointer to the LFN working buffer to be compared */
		BYTE* dir				/* Pointer to the directory entry containing the part of LFN */
	);

#if FF_FS_MINIMIZE <= 1 || FF_FS_RPATH >= 2 || FF_USE_LABEL || FF_FS_EXFAT
	/*-----------------------------------------------------*/
	/* FAT-LFN: Pick a part of file name from an LFN entry */
	/*-----------------------------------------------------*/

	int pick_lfn (	/* 1:succeeded, 0:buffer overflow or invalid LFN entry */
		WCHAR* lfnbuf,		/* Pointer to the LFN working buffer */
		BYTE* dir			/* Pointer to the LFN entry */
	);
#endif

#if !FF_FS_READONLY
	/*-----------------------------------------*/
	/* FAT-LFN: Create an entry of LFN entries */
	/*-----------------------------------------*/

	void put_lfn (
		const WCHAR* lfn,	/* Pointer to the LFN */
		BYTE* dir,			/* Pointer to the LFN entry to be created */
		BYTE ord,			/* LFN order (1-20) */
		BYTE sum			/* Checksum of the corresponding SFN */
	);
#endif	/* !FF_FS_READONLY */
#endif	/* FF_USE_LFN */

#if FF_USE_LFN && !FF_FS_READONLY
	/*-----------------------------------------------------------------------*/
	/* FAT-LFN: Create a Numbered SFN                                        */
	/*-----------------------------------------------------------------------*/

	void gen_numname (
		BYTE* dst,			/* Pointer to the buffer to store numbered SFN */
		const BYTE* src,	/* Pointer to SFN in directory form */
		const WCHAR* lfn,	/* Pointer to LFN */
		UINT seq			/* Sequence number */
	);
#endif	/* FF_USE_LFN && !FF_FS_READONLY */

#if FF_USE_LFN
	/*-----------------------------------------------------------------------*/
	/* FAT-LFN: Calculate checksum of an SFN entry                           */
	/*-----------------------------------------------------------------------*/

	BYTE sum_sfn (
		const BYTE* dir		/* Pointer to the SFN entry */
	);
#endif	/* FF_USE_LFN */

#if FF_FS_EXFAT
	/*-----------------------------------------------------------------------*/
	/* exFAT: Checksum                                                       */
	/*-----------------------------------------------------------------------*/

	WORD xdir_sum (	/* Get checksum of the directoly entry block */
		const BYTE* dir		/* Directory entry block to be calculated */
	);

	WORD xname_sum (	/* Get check sum (to be used as hash) of the file name */
		const WCHAR* name	/* File name to be calculated */
	);

#if !FF_FS_READONLY && FF_USE_MKFS
	DWORD xsum32 (	/* Returns 32-bit checksum */
		BYTE  dat,			/* Byte to be calculated (byte-by-byte processing) */
		DWORD sum			/* Previous sum value */
	);

    
#endif

	/*------------------------------------------------------------------*/
	/* exFAT: Initialize object allocation info with loaded entry block */
	/*------------------------------------------------------------------*/

	void init_alloc_info (
		FATFS* fs,		/* Filesystem object */
		FFOBJID* obj	/* Object allocation information to be initialized */
	);

#endif // #if FF_FS_EXFAT


	/* LFN support functions */
#if FF_USE_LFN >= 1						/* Code conversion (defined in unicode.c) */
	WCHAR f_oem2uni (WCHAR oem, WORD cp);	/* OEM code to Unicode conversion */
	WCHAR f_uni2oem (DWORD uni, WORD cp);	/* Unicode to OEM code conversion */
	DWORD f_wtoupper (DWORD uni);			/* Unicode upper-case conversion */
#endif


#if	!FF_FS_READONLY 
#if	FF_FS_EXFAT 
/*-------------------------------------------*/
/* exFAT: Create a new directory enrty block */
/*-------------------------------------------*/

void create_xdir (
    BYTE* dirb,			/* Pointer to the direcotry entry block buffer */
    const WCHAR* lfn	/* Pointer to the object name */
);	

#endif	/* !FF_FS_READONLY */
#endif	/* FF_FS_EXFAT */

#if FF_FS_MINIMIZE <= 1 || FF_FS_RPATH >= 2
	/*-----------------------------------------------------------------------*/
	/* Get file information from directory entry                             */
	/*-----------------------------------------------------------------------*/

	void get_fileinfo (
		DIR* dp,			/* Pointer to the directory object */
		FILINFO* fno		/* Pointer to the file information to be filled */
	);
#endif /* FF_FS_MINIMIZE <= 1 || FF_FS_RPATH >= 2 */

#if FF_USE_FIND && FF_FS_MINIMIZE <= 1
	/*-----------------------------------------------------------------------*/
	/* Pattern matching                                                      */
	/*-----------------------------------------------------------------------*/

	#define FIND_RECURS	4	/* Maximum number of wildcard terms in the pattern to limit recursion */


	DWORD get_achar (	/* Get a character and advance ptr */
		const TCHAR** ptr		/* Pointer to pointer to the ANSI/OEM or Unicode string */
	);

	int pattern_match (	/* 0:mismatched, 1:matched */
		const TCHAR* pat,	/* Matching pattern */
		const TCHAR* nam,	/* String to be tested */
		UINT skip,			/* Number of pre-skip chars (number of ?s, b8:infinite (* specified)) */
		UINT recur			/* Recursion count */
	);	
#endif /* FF_USE_FIND && FF_FS_MINIMIZE <= 1 */

	/*-----------------------------------------------------------------------*/
	/* Pick a top segment and create the object name in directory form       */
	/*-----------------------------------------------------------------------*/

	FRESULT create_name (	/* FR_OK: successful, FR_INVALID_NAME: could not create */
		DIR* dp,					/* Pointer to the directory object */
		const TCHAR** path			/* Pointer to pointer to the segment in the path string */
	);

	/*-----------------------------------------------------------------------*/
	/* Get logical drive number from path name                               */
	/*-----------------------------------------------------------------------*/

	int get_ldnumber (	/* Returns logical drive number (-1:invalid drive number or null pointer) */
		const TCHAR** path		/* Pointer to pointer to the path name */
	);



	/*-----------------------------------------------------------------------*/
	/* GPT support functions                                                 */
	/*-----------------------------------------------------------------------*/

#if FF_LBA64

	/* Calculate CRC32 in byte-by-byte */

	DWORD crc32 (	/* Returns next CRC value */
		DWORD crc,			/* Current CRC value */
		BYTE d				/* A byte to be processed */
	);

	/* Check validity of GPT header */

	int test_gpt_header (	/* 0:Invalid, 1:Valid */
		const BYTE* gpth			/* Pointer to the GPT header */
	);	

#if !FF_FS_READONLY && FF_USE_MKFS

	/* Generate random value */
	DWORD make_rand (
		DWORD seed,		/* Seed value */
		BYTE* buff,		/* Output buffer */
		UINT n			/* Data length */
	);

#endif // FF_LBA64
#endif // !FF_FS_READONLY && FF_USE_MKFS


}; // class DriverCommon 


} // namespace fatfs 


#endif // __FF_COMMON_HPP__