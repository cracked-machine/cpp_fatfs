/*----------------------------------------------------------------------------/
/  FatFs - Generic FAT Filesystem module  R0.14b                              /
/-----------------------------------------------------------------------------/
/
/ Copyright (C) 2021, ChaN, all right reserved.
/
/ FatFs module is an open source software. Redistribution and use of FatFs in
/ source and binary forms, with or without modification, are permitted provided
/ that the following condition is met:

/ 1. Redistributions of source code must retain the above copyright notice,
/    this condition and the following disclaimer.
/
/ This software is provided by the copyright holder and contributors "AS IS"
/ and any warranties related to this software are DISCLAIMED.
/ The copyright owner or contributors be NOT LIABLE for any damages caused
/ by use of this software.
/
/----------------------------------------------------------------------------*/

// C++ Port of the original source code are subject to MIT License

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

#ifndef FF_DEFINED
#define FF_DEFINED	86631	/* Revision ID */


#include <diskio.hpp>
#include <stdarg.h>

namespace fatfs {

class ff {

public:

	ff() = default; 
	static constexpr uint8_t SZDIRE			  =	32;		/* Size of a directory entry */

	FRESULT f_open (FIL* fp, const TCHAR* path, BYTE mode);				/* Open or create a file */
	FRESULT f_close (FIL* fp);											/* Close an open file object */
	FRESULT f_read (FIL* fp, void* buff, UINT btr, UINT* br);			/* Read data from the file */
	FRESULT f_write (FIL* fp, const void* buff, UINT btw, UINT* bw);	/* Write data to the file */
	FRESULT f_lseek (FIL* fp, FSIZE_t ofs);								/* Move file pointer of the file object */
	FRESULT f_rewind(FIL* fp) { return f_lseek((fp), 0); };
	FRESULT f_truncate (FIL* fp);										/* Truncate the file */
	FRESULT f_sync (FIL* fp);											/* Flush cached data of the writing file */
	FRESULT f_opendir (DIR* dp, const TCHAR* path);						/* Open a directory */
	FRESULT f_closedir (DIR* dp);										/* Close an open directory */
	FRESULT f_readdir (DIR* dp, FILINFO* fno);							/* Read a directory item */
	FRESULT f_rewinddir(DIR* dp) { return f_readdir((dp), 0); };
	FRESULT f_findfirst (DIR* dp, FILINFO* fno, const TCHAR* path, const TCHAR* pattern);	/* Find first file */
	FRESULT f_findnext (DIR* dp, FILINFO* fno);							/* Find next file */
	FRESULT f_mkdir (const TCHAR* path);								/* Create a sub directory */
	FRESULT f_unlink (const TCHAR* path);								/* Delete an existing file or directory */
	FRESULT f_rmdir(const TCHAR* path) { return f_unlink(path); };
	FRESULT f_rename (const TCHAR* path_old, const TCHAR* path_new);	/* Rename/Move a file or directory */
	FRESULT f_stat (const TCHAR* path, FILINFO* fno);					/* Get file status */
	FRESULT f_chmod (const TCHAR* path, BYTE attr, BYTE mask);			/* Change attribute of a file/dir */
	FRESULT f_utime (const TCHAR* path, const FILINFO* fno);			/* Change timestamp of a file/dir */
	FRESULT f_chdir (const TCHAR* path);								/* Change current directory */
	FRESULT f_chdrive (const TCHAR* path);								/* Change current drive */
	FRESULT f_getcwd (TCHAR* buff, UINT len);							/* Get current directory */
	FRESULT f_getfree (const TCHAR* path, DWORD* nclst, FATFS** fatfs);	/* Get number of free clusters on the drive */
	FRESULT f_getlabel (const TCHAR* path, TCHAR* label, DWORD* vsn);	/* Get volume label */
	FRESULT f_setlabel (const TCHAR* label);							/* Set volume label */
	FRESULT f_forward (FIL* fp, UINT(*func)(const BYTE*,UINT), UINT btf, UINT* bf);	/* Forward data to the stream */
	FRESULT f_expand (FIL* fp, FSIZE_t fsz, BYTE opt);					/* Allocate a contiguous block to the file */
	FRESULT f_mount (FATFS* fs, const TCHAR* path, BYTE opt);			/* Mount/Unmount a logical drive */
	FRESULT f_unmount(const TCHAR* path) { return f_mount(0, path, 0); };
	FRESULT f_mkfs (const TCHAR* path, const MKFS_PARM* opt, void* work, UINT len);	/* Create a FAT volume */
	FRESULT f_fdisk (BYTE pdrv, const LBA_t ptbl[], void* work);		/* Divide a physical drive into some partitions */
	FRESULT f_setcp (WORD cp);											/* Set current code page */
	int f_putc (TCHAR c, FIL* fp);										/* Put a character to the file */
	int f_puts (const TCHAR* str, FIL* cp);								/* Put a string to the file */
	int f_printf (FIL* fp, const TCHAR* str, ...);						/* Put a formatted string to the file */
	TCHAR* f_gets (TCHAR* buff, int len, FIL* fp);						/* Get a string from the file */
	FSIZE_t f_size(FIL* fp) 	{ return ((fp)->obj.objsize); };
	FSIZE_t f_tell(FIL* fp) 	{ return ((fp)->fptr); };
	BYTE f_error(FIL* fp)		{ return ((fp)->err); };
	FSIZE_t f_eof(FIL* fp) 	{ return ((int)((fp)->fptr == (fp)->obj.objsize)); };

	/*--------------------------------------------------------------*/
	/* Additional user defined functions                            */

	/* RTC function */
#if !FF_FS_READONLY && !FF_FS_NORTC
	DWORD get_fattime (void);
#endif

	/* LFN support functions */
#if FF_USE_LFN >= 1						/* Code conversion (defined in unicode.c) */
	WCHAR f_oem2uni (WCHAR oem, WORD cp);	/* OEM code to Unicode conversion */
	WCHAR f_uni2oem (DWORD uni, WORD cp);	/* Unicode to OEM code conversion */
	DWORD f_wtoupper (DWORD uni);			/* Unicode upper-case conversion */
#endif
#if FF_USE_LFN == 3						/* Dynamic memory allocation */
	void* f_memalloc (UINT msize);			/* Allocate memory block */
	void f_memfree (void* mblock);			/* Free memory block */
#endif

	/* Sync functions */
#if FF_FS_REENTRANT
	int f_cre_syncobj (BYTE vol, FF_SYNC_t* sobj);	/* Create a sync object */
	int f_req_grant (FF_SYNC_t sobj);		/* Lock sync object */
	void f_rel_grant (FF_SYNC_t sobj);		/* Unlock sync object */
	int f_del_syncobj (FF_SYNC_t sobj);	/* Delete a sync object */
#endif

private:

	Diskio diskio;

	/* Limits and boundaries */
	static constexpr uint32_t MAX_DIR 		= 0x200000;			/* Max size of FAT directory */
	static constexpr uint32_t MAX_DIR_EX	= 0x10000000;		/* Max size of exFAT directory */
	static constexpr uint32_t MAX_FAT12		= 0xFF5;			/* Max FAT12 clusters (differs from specs, but right for real DOS/Windows behavior) */
	static constexpr uint32_t MAX_FAT16		= 0xFFF5;			/* Max FAT16 clusters (differs from specs, but right for real DOS/Windows behavior) */
	static constexpr uint32_t MAX_FAT32		= 0x0FFFFFF5;		/* Max FAT32 clusters (not specified, practical limit) */
	static constexpr uint32_t MAX_EXFAT		= 0x7FFFFFFD;		/* Max exFAT clusters (differs from specs, implementation limit) */
	
	/* Additional file access control and file status flags for internal use */
	static constexpr uint8_t FA_SEEKEND		= 0x20;	/* Seek to end of the file on file open */
	static constexpr uint8_t FA_MODIFIED	= 0x40;	/* File has been modified */
	static constexpr uint8_t FA_DIRTY		= 0x80;	/* FIL.buf[] needs to be written-back */	

	/* Additional file attribute bits for internal use */
	static constexpr uint8_t AM_VOL			= 0x08;	/* Volume label */
	static constexpr uint8_t AM_LFN			= 0x0F;	/* LFN entry */
	static constexpr uint8_t AM_MASK		= 0x3F;	/* Mask of defined bits in FAT */
	static constexpr uint8_t AM_MASKX	    = 0x37;	/* Mask of defined bits in exFAT */

	/* Name status flags in fn[11] */
	static constexpr uint8_t NSFLAG			= 11;	/* Index of the name status byte */
	static constexpr uint8_t NS_LOSS		= 0x01;	/* Out of 8.3 format */
	static constexpr uint8_t NS_LFN			= 0x02;	/* Force to create LFN entry */
	static constexpr uint8_t NS_LAST		= 0x04;	/* Last segment */
	static constexpr uint8_t NS_BODY		= 0x08;	/* Lower case flag (body) */
	static constexpr uint8_t NS_EXT			= 0x10;	/* Lower case flag (ext) */
	static constexpr uint8_t NS_DOT			= 0x20;	/* Dot entry */
	static constexpr uint8_t NS_NOLFN		= 0x40;	/* Do not find LFN */
	static constexpr uint8_t NS_NONAME		= 0x80;	/* Not followed */	

	/* exFAT directory entry types */
	static constexpr uint8_t ET_BITMAP		= 0x81;	/* Allocation bitmap */
	static constexpr uint8_t ET_UPCASE		= 0x82;	/* Up-case table */
	static constexpr uint8_t ET_VLABEL		= 0x83;	/* Volume label */
	static constexpr uint8_t ET_FILEDIR		= 0x85;	/* File and directory */
	static constexpr uint8_t ET_STREAM		= 0xC0;	/* Stream extension */
	static constexpr uint8_t ET_FILENAME	= 0xC1;	/* Name extension */	

	/* FatFs refers the FAT structure as simple byte array instead of structure member
	/ because the C structure is not binary compatible between different platforms */

	static constexpr uint8_t BS_JmpBoot		= 0;		/* x86 jump instruction (3-byte) */
	static constexpr uint8_t BS_OEMName		= 3;		/* OEM name (8-byte) */
	static constexpr uint8_t BPB_BytsPerSec	= 11;		/* Sector size [byte] (WORD) */
	static constexpr uint8_t BPB_SecPerClus	= 13;		/* Cluster size [sector] (BYTE) */
	static constexpr uint8_t BPB_RsvdSecCnt	= 14;		/* Size of reserved area [sector] (WORD) */
	static constexpr uint8_t BPB_NumFATs	= 16;		/* Number of FATs (BYTE) */
	static constexpr uint8_t BPB_RootEntCnt	= 17;		/* Size of root directory area for FAT [entry] (WORD) */
	static constexpr uint8_t BPB_TotSec16	= 19;		/* Volume size (16-bit) [sector] (WORD) */
	static constexpr uint8_t BPB_Media		= 21;		/* Media descriptor byte (BYTE) */
	static constexpr uint8_t BPB_FATSz16	= 22;		/* FAT size (16-bit) [sector] (WORD) */
	static constexpr uint8_t BPB_SecPerTrk	= 24;		/* Number of sectors per track for int13h [sector] (WORD) */
	static constexpr uint8_t BPB_NumHeads	= 26;		/* Number of heads for int13h (WORD) */
	static constexpr uint8_t BPB_HiddSec	= 28;		/* Volume offset from top of the drive (DWORD) */
	static constexpr uint8_t BPB_TotSec32	= 32;		/* Volume size (32-bit) [sector] (DWORD) */
	static constexpr uint8_t BS_DrvNum		= 36;		/* Physical drive number for int13h (BYTE) */
	static constexpr uint8_t BS_NTres		= 37;		/* WindowsNT error flag (BYTE) */
	static constexpr uint8_t BS_BootSig		= 38;		/* Extended boot signature (BYTE) */
	static constexpr uint8_t BS_VolID		= 39;		/* Volume serial number (DWORD) */
	static constexpr uint8_t BS_VolLab		= 43;		/* Volume label string (8-byte) */
	static constexpr uint8_t BS_FilSysType	= 54;		/* Filesystem type string (8-byte) */
	static constexpr uint8_t BS_BootCode	= 62;		/* Boot code (448-byte) */
	static constexpr uint16_t BS_55AA		= 510;		/* Signature word (WORD) */	


	static constexpr uint8_t BPB_FATSz32	= 36;		/* FAT32: FAT size [sector] (DWORD) */
	static constexpr uint8_t BPB_ExtFlags32	= 40;		/* FAT32: Extended flags (WORD) */
	static constexpr uint8_t BPB_FSVer32	= 42;		/* FAT32: Filesystem version (WORD) */
	static constexpr uint8_t BPB_RootClus32	= 44;		/* FAT32: Root directory cluster (DWORD) */
	static constexpr uint8_t BPB_FSInfo32	= 48;		/* FAT32: Offset of FSINFO sector (WORD) */
	static constexpr uint8_t BPB_BkBootSec32 = 50;		/* FAT32: Offset of backup boot sector (WORD) */
	static constexpr uint8_t BS_DrvNum32	= 64;		/* FAT32: Physical drive number for int13h (BYTE) */
	static constexpr uint8_t BS_NTres32		= 65;		/* FAT32: Error flag (BYTE) */
	static constexpr uint8_t BS_BootSig32	= 66;		/* FAT32: Extended boot signature (BYTE) */
	static constexpr uint8_t BS_VolID32		= 67;		/* FAT32: Volume serial number (DWORD) */
	static constexpr uint8_t BS_VolLab32	= 71;		/* FAT32: Volume label string (8-byte) */
	static constexpr uint8_t BS_FilSysType32 = 82;		/* FAT32: Filesystem type string (8-byte) */
	static constexpr uint8_t BS_BootCode32	= 90;		/* FAT32: Boot code (420-byte) */

	static constexpr uint8_t BPB_ZeroedEx	= 11;		/* exFAT: MBZ field (53-byte) */
	static constexpr uint8_t BPB_VolOfsEx	= 64;		/* exFAT: Volume offset from top of the drive [sector] (QWORD) */
	static constexpr uint8_t BPB_TotSecEx	= 72;		/* exFAT: Volume size [sector] (QWORD) */
	static constexpr uint8_t BPB_FatOfsEx	= 80;		/* exFAT: FAT offset from top of the volume [sector] (DWORD) */
	static constexpr uint8_t BPB_FatSzEx	= 84;		/* exFAT: FAT size [sector] (DWORD) */
	static constexpr uint8_t BPB_DataOfsEx	= 88;		/* exFAT: Data offset from top of the volume [sector] (DWORD) */
	static constexpr uint8_t BPB_NumClusEx	= 92;		/* exFAT: Number of clusters (DWORD) */
	static constexpr uint8_t BPB_RootClusEx	= 96;		/* exFAT: Root directory start cluster (DWORD) */
	static constexpr uint8_t BPB_VolIDEx	= 100;		/* exFAT: Volume serial number (DWORD) */
	static constexpr uint8_t BPB_FSVerEx	= 104;		/* exFAT: Filesystem version (WORD) */
	static constexpr uint8_t BPB_VolFlagEx	= 106;		/* exFAT: Volume flags (WORD) */
	static constexpr uint8_t BPB_BytsPerSecEx = 108;		/* exFAT: Log2 of sector size in unit of byte (BYTE) */
	static constexpr uint8_t BPB_SecPerClusEx = 109;		/* exFAT: Log2 of cluster size in unit of sector (BYTE) */
	static constexpr uint8_t BPB_NumFATsEx	  = 110;		/* exFAT: Number of FATs (BYTE) */
	static constexpr uint8_t BPB_DrvNumEx	  = 111;		/* exFAT: Physical drive number for int13h (BYTE) */
	static constexpr uint8_t BPB_PercInUseEx  = 112;		/* exFAT: Percent in use (BYTE) */
	static constexpr uint8_t BPB_RsvdEx		  = 113;		/* exFAT: Reserved (7-byte) */
	static constexpr uint8_t BS_BootCodeEx	  = 120;		/* exFAT: Boot code (390-byte) */

	static constexpr uint8_t DIR_Name		  = 0;		/* Short file name (11-byte) */
	static constexpr uint8_t DIR_Attr		  = 11;		/* Attribute (BYTE) */
	static constexpr uint8_t DIR_NTres		  = 12;		/* Lower case flag (BYTE) */
	static constexpr uint8_t DIR_CrtTime10	  = 13;		/* Created time sub-second (BYTE) */
	static constexpr uint8_t DIR_CrtTime	  = 14;		/* Created time (DWORD) */
	static constexpr uint8_t DIR_LstAccDate	  = 18;		/* Last accessed date (WORD) */
	static constexpr uint8_t DIR_FstClusHI	  = 20;		/* Higher 16-bit of first cluster (WORD) */
	static constexpr uint8_t DIR_ModTime	  = 22;		/* Modified time (DWORD) */
	static constexpr uint8_t DIR_FstClusLO	  = 26;		/* Lower 16-bit of first cluster (WORD) */
	static constexpr uint8_t DIR_FileSize	  = 28;		/* File size (DWORD) */
	static constexpr uint8_t LDIR_Ord		  = 0;		/* LFN: LFN order and LLE flag (BYTE) */
	static constexpr uint8_t LDIR_Attr		  = 11;		/* LFN: LFN attribute (BYTE) */
	static constexpr uint8_t LDIR_Type		  = 12;		/* LFN: Entry type (BYTE) */
	static constexpr uint8_t LDIR_Chksum	  =	13;		/* LFN: Checksum of the SFN (BYTE) */
	static constexpr uint8_t LDIR_FstClusLO	  =	26;		/* LFN: MBZ field (WORD) */
	static constexpr uint8_t XDIR_Type		  = 0;		/* exFAT: Type of exFAT directory entry (BYTE) */
	static constexpr uint8_t XDIR_NumLabel	  =	1;		/* exFAT: Number of volume label characters (BYTE) */
	static constexpr uint8_t XDIR_Label		  =	2;		/* exFAT: Volume label (11-WORD) */
	static constexpr uint8_t XDIR_CaseSum	  =	4;		/* exFAT: Sum of case conversion table (DWORD) */
	static constexpr uint8_t XDIR_NumSec	  = 1;		/* exFAT: Number of secondary entries (BYTE) */
	static constexpr uint8_t XDIR_SetSum	  = 2;		/* exFAT: Sum of the set of directory entries (WORD) */
	static constexpr uint8_t XDIR_Attr		  =	4;		/* exFAT: File attribute (WORD) */
	static constexpr uint8_t XDIR_CrtTime	  =	8;		/* exFAT: Created time (DWORD) */
	static constexpr uint8_t XDIR_ModTime	  =	12;		/* exFAT: Modified time (DWORD) */
	static constexpr uint8_t XDIR_AccTime	  =	16;		/* exFAT: Last accessed time (DWORD) */
	static constexpr uint8_t XDIR_CrtTime10	  =	20;		/* exFAT: Created time subsecond (BYTE) */
	static constexpr uint8_t XDIR_ModTime10	  =	21;		/* exFAT: Modified time subsecond (BYTE) */
	static constexpr uint8_t XDIR_CrtTZ		  =	22;		/* exFAT: Created timezone (BYTE) */
	static constexpr uint8_t XDIR_ModTZ		  =	23;		/* exFAT: Modified timezone (BYTE) */
	static constexpr uint8_t XDIR_AccTZ		  =	24;		/* exFAT: Last accessed timezone (BYTE) */
	static constexpr uint8_t XDIR_GenFlags	  =	33;		/* exFAT: General secondary flags (BYTE) */
	static constexpr uint8_t XDIR_NumName	  =	35;		/* exFAT: Number of file name characters (BYTE) */
	static constexpr uint8_t XDIR_NameHash	  =	36;		/* exFAT: Hash of file name (WORD) */
	static constexpr uint8_t XDIR_ValidFileSize = 40;		/* exFAT: Valid file size (QWORD) */
	static constexpr uint8_t XDIR_FstClus	  =	52;		/* exFAT: First cluster of the file data (DWORD) */
	static constexpr uint8_t XDIR_FileSize	  =	56;		/* exFAT: File/Directory size (QWORD) */


	static constexpr uint8_t DDEM			  =	0xE5;	/* Deleted directory entry mark set to DIR_Name[0] */
	static constexpr uint8_t RDDEM			  =	0x05;	/* Replacement of the character collides with DDEM */
	static constexpr uint8_t LLEF			  =	0x40;	/* Last long entry flag in LDIR_Ord */

	static constexpr uint8_t FSI_LeadSig	  =	0;		/* FAT32 FSI: Leading signature (DWORD) */
	static constexpr uint16_t FSI_StrucSig	  =	484;		/* FAT32 FSI: Structure signature (DWORD) */
	static constexpr uint16_t FSI_Free_Count  = 488;		/* FAT32 FSI: Number of free clusters (DWORD) */
	static constexpr uint16_t FSI_Nxt_Free	  =	492;		/* FAT32 FSI: Last allocated cluster (DWORD) */

	static constexpr uint16_t MBR_Table		  =	446;		/* MBR: Offset of partition table in the MBR */
	static constexpr uint8_t SZ_PTE			  =	16;		/* MBR: Size of a partition table entry */
	static constexpr uint8_t PTE_Boot		  =	0;		/* MBR PTE: Boot indicator */
	static constexpr uint8_t PTE_StHead		  =	1;		/* MBR PTE: Start head */
	static constexpr uint8_t PTE_StSec		  =	2;		/* MBR PTE: Start sector */
	static constexpr uint8_t PTE_StCyl		  =	3;		/* MBR PTE: Start cylinder */
	static constexpr uint8_t PTE_System		  =	4;		/* MBR PTE: System ID */
	static constexpr uint8_t PTE_EdHead		  =	5;		/* MBR PTE: End head */
	static constexpr uint8_t PTE_EdSec		  =	6;		/* MBR PTE: End sector */
	static constexpr uint8_t PTE_EdCyl		  =	7;		/* MBR PTE: End cylinder */
	static constexpr uint8_t PTE_StLba		  =	8;		/* MBR PTE: Start in LBA */
	static constexpr uint8_t PTE_SizLba		  =	12;		/* MBR PTE: Size in LBA */

	static constexpr uint8_t GPTH_Sign		  =	0;		/* GPT: Header signature (8-byte) */
	static constexpr uint8_t GPTH_Rev		  =	8;		/* GPT: Revision (DWORD) */
	static constexpr uint8_t GPTH_Size		  =	12;		/* GPT: Header size (DWORD) */
	static constexpr uint8_t GPTH_Bcc		  =	16;		/* GPT: Header BCC (DWORD) */
	static constexpr uint8_t GPTH_CurLba	  =	24;		/* GPT: Main header LBA (QWORD) */
	static constexpr uint8_t GPTH_BakLba	  = 32;		/* GPT: Backup header LBA (QWORD) */
	static constexpr uint8_t GPTH_FstLba	  = 40;		/* GPT: First LBA for partitions (QWORD) */
	static constexpr uint8_t GPTH_LstLba	  =	48;		/* GPT: Last LBA for partitions (QWORD) */
	static constexpr uint8_t GPTH_DskGuid	  =	56;		/* GPT: Disk GUID (16-byte) */
	static constexpr uint8_t GPTH_PtOfs		  =	72;		/* GPT: Partation table LBA (QWORD) */
	static constexpr uint8_t GPTH_PtNum		  =	80;		/* GPT: Number of table entries (DWORD) */
	static constexpr uint8_t GPTH_PteSize	  =	84;		/* GPT: Size of table entry (DWORD) */
	static constexpr uint8_t GPTH_PtBcc		  =	88;		/* GPT: Partation table BCC (DWORD) */
	static constexpr uint8_t SZ_GPTE		  = 128;		/* GPT: Size of partition table entry */
	static constexpr uint8_t GPTE_PtGuid	  = 0;		/* GPT PTE: Partition type GUID (16-byte) */
	static constexpr uint8_t GPTE_UpGuid	  =	16;		/* GPT PTE: Partition unique GUID (16-byte) */
	static constexpr uint8_t GPTE_FstLba	  =	32;		/* GPT PTE: First LBA (QWORD) */
	static constexpr uint8_t GPTE_LstLba	  =	40;		/* GPT PTE: Last LBA inclusive (QWORD) */
	static constexpr uint8_t GPTE_Flags		  =	48;		/* GPT PTE: Flags (QWORD) */
	static constexpr uint8_t GPTE_Name		  =	56;		/* GPT PTE: Name */	

	/*--------------------------------------------------------------*/
	/* Flags and offset address                                     */

	/* File access mode and open method flags (3rd argument of f_open) */
	static constexpr uint8_t FA_READ		  =	0x01;
	static constexpr uint8_t FA_WRITE		  =	0x02;
	static constexpr uint8_t FA_OPEN_EXISTING =	0x00;
	static constexpr uint8_t FA_CREATE_NEW	  =	0x04;
	static constexpr uint8_t FA_CREATE_ALWAYS =	0x08;
	static constexpr uint8_t FA_OPEN_ALWAYS	  =	0x10;
	static constexpr uint8_t FA_OPEN_APPEND	  =	0x30;

	/* Fast seek controls (2nd argument of f_lseek) */
	static constexpr QWORD CREATE_LINKMAP	  = ((FSIZE_t)0 - 1);

	/* Format options (2nd argument of f_mkfs) */
	static constexpr uint8_t FM_FAT			  = 0x01;
	static constexpr uint8_t FM_FAT32		  = 0x02;
	static constexpr uint8_t FM_EXFAT		  =	0x04;
	static constexpr uint8_t FM_ANY		      = 0x07;
	static constexpr uint8_t FM_SFD			  = 0x08;

	/* Filesystem type (FATFS.fs_type) */
	static constexpr uint8_t FS_FAT12		  = 1;
	static constexpr uint8_t FS_FAT16		  = 2;
	static constexpr uint8_t FS_FAT32		  = 3;
	static constexpr uint8_t FS_EXFAT		  = 4;

	/* File attribute bits for directory entry (FILINFO.fattrib) */
	static constexpr uint8_t AM_RDO			  = 0x01;	/* Read only */
	static constexpr uint8_t AM_HID			  = 0x02;	/* Hidden */
	static constexpr uint8_t AM_SYS			  = 0x04;	/* System */
	static constexpr uint8_t AM_DIR			  = 0x10;	/* Directory */
	static constexpr uint8_t AM_ARC	          = 0x20;	/* Archive */


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
	/* Move/Flush disk access window in the filesystem object                */
	/*-----------------------------------------------------------------------*/
#if !FF_FS_READONLY
	FRESULT sync_window (	/* Returns FR_OK or FR_DISK_ERR */
		FATFS* fs			/* Filesystem object */
	);
#endif

	FRESULT move_window (	/* Returns FR_OK or FR_DISK_ERR */
		FATFS* fs,		/* Filesystem object */
		LBA_t sect		/* Sector LBA to make appearance in the fs->win[] */
	);

#if !FF_FS_READONLY
	/*-----------------------------------------------------------------------*/
	/* Synchronize filesystem and data on the storage                        */
	/*-----------------------------------------------------------------------*/

	FRESULT sync_fs (	/* Returns FR_OK or FR_DISK_ERR */
		FATFS* fs		/* Filesystem object */
	);

#endif // !FF_FS_READONLY

	/*-----------------------------------------------------------------------*/
	/* Get physical sector number from cluster number                        */
	/*-----------------------------------------------------------------------*/

	LBA_t clst2sect (	/* !=0:Sector number, 0:Failed (invalid cluster#) */
		FATFS* fs,		/* Filesystem object */
		DWORD clst		/* Cluster# to be converted */
	);

	/*-----------------------------------------------------------------------*/
	/* FAT access - Read value of an FAT entry                               */
	/*-----------------------------------------------------------------------*/

	DWORD get_fat (		/* 0xFFFFFFFF:Disk error, 1:Internal error, 2..0x7FFFFFFF:Cluster status */
		FFOBJID* obj,	/* Corresponding object */
		DWORD clst		/* Cluster number to get the value */
	);

#if !FF_FS_READONLY
	/*-----------------------------------------------------------------------*/
	/* FAT access - Change value of an FAT entry                             */
	/*-----------------------------------------------------------------------*/

	FRESULT put_fat (	/* FR_OK(0):succeeded, !=0:error */
		FATFS* fs,		/* Corresponding filesystem object */
		DWORD clst,		/* FAT index number (cluster number) to be changed */
		DWORD val		/* New value to be set to the entry */
	);
#endif /* !FF_FS_READONLY */

#if FF_FS_EXFAT && !FF_FS_READONLY
	/*-----------------------------------------------------------------------*/
	/* exFAT: Accessing FAT and Allocation Bitmap                            */
	/*-----------------------------------------------------------------------*/

	/*--------------------------------------*/
	/* Find a contiguous free cluster block */
	/*--------------------------------------*/

	DWORD find_bitmap (	/* 0:Not found, 2..:Cluster block found, 0xFFFFFFFF:Disk error */
		FATFS* fs,	/* Filesystem object */
		DWORD clst,	/* Cluster number to scan from */
		DWORD ncl	/* Number of contiguous clusters to find (1..) */
	);

	/*----------------------------------------*/
	/* Set/Clear a block of allocation bitmap */
	/*----------------------------------------*/

	FRESULT change_bitmap (
		FATFS* fs,	/* Filesystem object */
		DWORD clst,	/* Cluster number to change from */
		DWORD ncl,	/* Number of clusters to be changed */
		int bv		/* bit value to be set (0 or 1) */
	);

	/*---------------------------------------------*/
	/* Fill the first fragment of the FAT chain    */
	/*---------------------------------------------*/

	FRESULT fill_first_frag (
		FFOBJID* obj	/* Pointer to the corresponding object */
	);

	/*---------------------------------------------*/
	/* Fill the last fragment of the FAT chain     */
	/*---------------------------------------------*/

	FRESULT fill_last_frag (
		FFOBJID* obj,	/* Pointer to the corresponding object */
		DWORD lcl,		/* Last cluster of the fragment */
		DWORD term		/* Value to set the last FAT entry */
	);
#endif	/* FF_FS_EXFAT && !FF_FS_READONLY */				

#if !FF_FS_READONLY
	/*-----------------------------------------------------------------------*/
	/* FAT handling - Remove a cluster chain                                 */
	/*-----------------------------------------------------------------------*/

	FRESULT remove_chain (	/* FR_OK(0):succeeded, !=0:error */
		FFOBJID* obj,		/* Corresponding object */
		DWORD clst,			/* Cluster to remove a chain from */
		DWORD pclst			/* Previous cluster of clst (0 if entire chain) */
	);


	/*-----------------------------------------------------------------------*/
	/* FAT handling - Stretch a chain or Create a new chain                  */
	/*-----------------------------------------------------------------------*/

	DWORD create_chain (	/* 0:No free cluster, 1:Internal error, 0xFFFFFFFF:Disk error, >=2:New cluster# */
		FFOBJID* obj,		/* Corresponding object */
		DWORD clst			/* Cluster# to stretch, 0:Create a new chain */
	);
#endif /* !FF_FS_READONLY */

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
/* Directory handling - Fill a cluster with zeros                        */
/*-----------------------------------------------------------------------*/

#if !FF_FS_READONLY
	FRESULT dir_clear (	/* Returns FR_OK or FR_DISK_ERR */
		FATFS *fs,		/* Filesystem object */
		DWORD clst		/* Directory table to clear */
	);
#endif	/* !FF_FS_READONLY */	

	/*-----------------------------------------------------------------------*/
	/* Directory handling - Set directory index                              */
	/*-----------------------------------------------------------------------*/

	FRESULT dir_sdi (	/* FR_OK(0):succeeded, !=0:error */
		DIR* dp,		/* Pointer to directory object */
		DWORD ofs		/* Offset of directory table */
	);

	/*-----------------------------------------------------------------------*/
	/* Directory handling - Move directory table index next                  */
	/*-----------------------------------------------------------------------*/

	FRESULT dir_next (	/* FR_OK(0):succeeded, FR_NO_FILE:End of table, FR_DENIED:Could not stretch */
		DIR* dp,				/* Pointer to the directory object */
		int stretch				/* 0: Do not stretch table, 1: Stretch table if needed */
	);

#if !FF_FS_READONLY
	/*-----------------------------------------------------------------------*/
	/* Directory handling - Reserve a block of directory entries             */
	/*-----------------------------------------------------------------------*/

	FRESULT dir_alloc (	/* FR_OK(0):succeeded, !=0:error */
		DIR* dp,				/* Pointer to the directory object */
		UINT n_ent				/* Number of contiguous entries to allocate */
	);		
#endif	/* !FF_FS_READONLY */

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

	/*-----------------------------------*/
	/* exFAT: Get a directry entry block */
	/*-----------------------------------*/

	FRESULT load_xdir (	/* FR_INT_ERR: invalid entry block */
		DIR* dp					/* Reading direcotry object pointing top of the entry block to load */
	);

	/*------------------------------------------------------------------*/
	/* exFAT: Initialize object allocation info with loaded entry block */
	/*------------------------------------------------------------------*/

	void init_alloc_info (
		FATFS* fs,		/* Filesystem object */
		FFOBJID* obj	/* Object allocation information to be initialized */
	);

#if !FF_FS_READONLY || FF_FS_RPATH != 0
	/*------------------------------------------------*/
	/* exFAT: Load the object's directory entry block */
	/*------------------------------------------------*/

	FRESULT load_obj_xdir (
		DIR* dp,			/* Blank directory object to be used to access containing direcotry */
		const FFOBJID* obj	/* Object with its containing directory information */
	);
#endif

#if !FF_FS_READONLY
	/*----------------------------------------*/
	/* exFAT: Store the directory entry block */
	/*----------------------------------------*/

	FRESULT store_xdir (
		DIR* dp				/* Pointer to the direcotry object */
	);		

	/*-------------------------------------------*/
	/* exFAT: Create a new directory enrty block */
	/*-------------------------------------------*/

	void create_xdir (
		BYTE* dirb,			/* Pointer to the direcotry entry block buffer */
		const WCHAR* lfn	/* Pointer to the object name */
	);	
#endif	/* !FF_FS_READONLY */
#endif	/* FF_FS_EXFAT */

#if FF_FS_MINIMIZE <= 1 || FF_FS_RPATH >= 2 || FF_USE_LABEL || FF_FS_EXFAT
	/*-----------------------------------------------------------------------*/
	/* Read an object from the directory                                     */
	/*-----------------------------------------------------------------------*/

	#define DIR_READ_FILE(dp) dir_read(dp, 0)
	#define DIR_READ_LABEL(dp) dir_read(dp, 1)

	FRESULT dir_read (
		DIR* dp,		/* Pointer to the directory object */
		int vol			/* Filtered by 0:file/directory or 1:volume label */
	);
#endif	/* FF_FS_MINIMIZE <= 1 || FF_USE_LABEL || FF_FS_RPATH >= 2 */

	/*-----------------------------------------------------------------------*/
	/* Directory handling - Find an object in the directory                  */
	/*-----------------------------------------------------------------------*/

	FRESULT dir_find (	/* FR_OK(0):succeeded, !=0:error */
		DIR* dp					/* Pointer to the directory object with the file name */
	);

#if !FF_FS_READONLY
	/*-----------------------------------------------------------------------*/
	/* Register an object to the directory                                   */
	/*-----------------------------------------------------------------------*/

	FRESULT dir_register (	/* FR_OK:succeeded, FR_DENIED:no free entry or too many SFN collision, FR_DISK_ERR:disk error */
		DIR* dp						/* Target directory with object name to be created */
	);
#endif /* !FF_FS_READONLY */

#if !FF_FS_READONLY && FF_FS_MINIMIZE == 0
	/*-----------------------------------------------------------------------*/
	/* Remove an object from the directory                                   */
	/*-----------------------------------------------------------------------*/

	FRESULT dir_remove (	/* FR_OK:Succeeded, FR_DISK_ERR:A disk error */
		DIR* dp					/* Directory object pointing the entry to be removed */
	);
#endif /* !FF_FS_READONLY && FF_FS_MINIMIZE == 0 */

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
	/* Follow a file path                                                    */
	/*-----------------------------------------------------------------------*/

	FRESULT follow_path (	/* FR_OK(0): successful, !=0: error code */
		DIR* dp,					/* Directory object to return last directory and found object */
		const TCHAR* path			/* Full-path string to find a file or directory */
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

	/*-----------------------------------------------------------------------*/
	/* Load a sector and check if it is an FAT VBR                           */
	/*-----------------------------------------------------------------------*/

	/* Check what the sector is */

	UINT check_fs (	/* 0:FAT/FAT32 VBR, 1:exFAT VBR, 2:Not FAT and valid BS, 3:Not FAT and invalid BS, 4:Disk error */
		FATFS* fs,			/* Filesystem object */
		LBA_t sect			/* Sector to load and check if it is an FAT-VBR or not */
	);

	/* Find an FAT volume */
	/* (It supports only generic partitioning rules, MBR, GPT and SFD) */

	UINT find_volume (	/* Returns BS status found in the hosting drive */
		FATFS* fs,		/* Filesystem object */
		UINT part		/* Partition to fined = 0:auto, 1..:forced */
	);	

	/*-----------------------------------------------------------------------*/
	/* Determine logical drive number and mount the volume if needed         */
	/*-----------------------------------------------------------------------*/

	FRESULT mount_volume (	/* FR_OK(0): successful, !=0: an error occurred */
		const TCHAR** path,			/* Pointer to pointer to the path name (drive number) */
		FATFS** rfs,				/* Pointer to pointer to the found filesystem object */
		BYTE mode					/* !=0: Check write protection for write access */
	);

	/*-----------------------------------------------------------------------*/
	/* Check if the file/directory object is valid or not                    */
	/*-----------------------------------------------------------------------*/

	FRESULT validate (	/* Returns FR_OK or FR_INVALID_OBJECT */
		FFOBJID* obj,			/* Pointer to the FFOBJID, the 1st member in the FIL/DIR object, to check validity */
		FATFS** rfs				/* Pointer to pointer to the owner filesystem object to return */
	);

#if !FF_FS_READONLY && FF_USE_MKFS
	/*-----------------------------------------------------------------------*/
	/* Create FAT/exFAT volume (with sub-functions)                          */
	/*-----------------------------------------------------------------------*/

	#define N_SEC_TRACK 63			/* Sectors per track for determination of drive CHS */
	#define	GPT_ALIGN	0x100000	/* Alignment of partitions in GPT [byte] (>=128KB) */
	#define GPT_ITEMS	128			/* Number of GPT table size (>=128, sector aligned) */


	/* Create partitions on the physical drive in format of MBR or GPT */

	FRESULT create_partition (
		BYTE drv,			/* Physical drive number */
		const LBA_t plst[],	/* Partition list */
		BYTE sys,			/* System ID (for only MBR, temp setting) */
		BYTE* buf			/* Working buffer for a sector */
	);	
#endif
#if !FF_FS_READONLY
	
	#define SZ_PUTC_BUF	64
	#define SZ_NUM_BUF	32
	/*-----------------------------------------------------------------------*/
	/* Put a Character to the File (with sub-functions)                      */
	/*-----------------------------------------------------------------------*/

	/* Output buffer and work area */

	typedef struct {
		FIL *fp;		/* Ptr to the writing file */
		int idx, nchr;	/* Write index of buf[] (-1:error), number of encoding units written */
	#if FF_USE_LFN && FF_LFN_UNICODE == 1
		WCHAR hs;
	#elif FF_USE_LFN && FF_LFN_UNICODE == 2
		BYTE bs[4];
		UINT wi, ct;
	#endif
		BYTE buf[SZ_PUTC_BUF];	/* Write buffer */
	} putbuff;


	/* Buffered file write with code conversion */
	void putc_bfd (putbuff* pb, TCHAR c);

	/* Flush remaining characters in the buffer */
	int putc_flush (putbuff* pb);

	/* Initialize write buffer */
	void putc_init (putbuff* pb, FIL* fp);	



	/*-----------------------------------------------------------------------*/
	/* Put a Formatted String to the File (with sub-functions)               */
	/*-----------------------------------------------------------------------*/
#if FF_PRINT_FLOAT && FF_INTDEF == 2

	int ilog10 (double n);	/* Calculate log10(n) in integer output */	
	double i10x (int n);	/* Calculate 10^n in integer input */
	bool isEqual(double x, double y);
	void ftoa (
		char* buf,	/* Buffer to output the floating point string */
		double val,	/* Value to output */
		int prec,	/* Number of fractional digits */
		TCHAR fmt	/* Notation */
	);


#endif /* !FF_FS_READONLY */
#endif /* FF_USE_STRFUNC */


#if FF_CODE_PAGE == 0
	/*-----------------------------------------------------------------------*/
	/* Set Active Codepage for the Path Name                                 */
	/*-----------------------------------------------------------------------*/

	FRESULT f_setcp (
		WORD cp		/* Value to be set as active code page */
	);
#endif	/* FF_CODE_PAGE == 0 */

};

} // namespace fatfs


#endif /* FF_DEFINED */

