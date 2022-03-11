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

#ifndef __FF_DRIVER_HPP__
#define __FF_DRIVER_HPP__

#include <diskio_hardware_usb.hpp>
#include <diskio_hardware_mmc.hpp>
#include <ff_driver_common.hpp>


namespace fatfs {

#ifndef FF_DEFINED
#define FF_DEFINED	86631	/* Revision ID */

/// @brief Main class for FatFS public API
/// @tparam DISKIO_HW Must be a derived type of DiskioHardwareBase
template<typename DISKIO_HW>
class Driver : public DriverCommon 
{

public:

	Driver(DISKIO_HW &diskio); 

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

	/* Dynamic memory allocation */
	#if FF_USE_LFN == 3									
		void* f_memalloc (UINT msize);					/* Allocate memory block */
		void f_memfree (void* mblock);					/* Free memory block */
	#endif

	/* Sync functions */
	#if FF_FS_REENTRANT
		int f_cre_syncobj (BYTE vol, FF_SYNC_t* sobj);	/* Create a sync object */
		int f_req_grant (FF_SYNC_t sobj);				/* Lock sync object */
		void f_rel_grant (FF_SYNC_t sobj);				/* Unlock sync object */
		int f_del_syncobj (FF_SYNC_t sobj);				/* Delete a sync object */
	#endif

private:
	/// @brief The low-layer Disk IO implementation object. 
	/// Assign using c'tor with a derived type that matches this template class specialization.
	/// e.g. DiskioHardwareMMC should be used with Driver<DiskioHardwareMMC>
	std::unique_ptr<DiskioHardwareBase> m_diskio;

	#if !FF_FS_READONLY
		/// @brief Flush disk access window in the filesystem object  
		/// @param fs Filesystem object
		/// @return FR_OK or FR_DISK_ERR
		FRESULT sync_window (FATFS* fs);
	#endif

	/// @brief Move disk access window in the filesystem object  
	/// @param fs Filesystem object
	/// @param sect Sector LBA to make appearance in the fs->win[]
	/// @return FR_OK or FR_DISK_ERR
	FRESULT move_window (FATFS* fs,	LBA_t sect);

	#if !FF_FS_READONLY		

		/// @brief Synchronize filesystem and data on the storage 
		/// @param fs Filesystem object
		/// @return FR_OK or FR_DISK_ERR
		FRESULT sync_fs (FATFS* fs);

	#endif // !FF_FS_READONLY

	/// @brief FAT access - Read value of an FAT entry  
	/// @param obj Corresponding object
	/// @param clst Cluster number to get the value
	/// @return DWORD 0xFFFFFFFF:Disk error, 1:Internal error, 2..0x7FFFFFFF:Cluster status
	DWORD get_fat (FFOBJID* obj, DWORD clst);	

	#if !FF_FS_READONLY
		
		/// @brief FAT access - Change value of an FAT entry     
		/// @param fs Corresponding filesystem object
		/// @param clst FAT index number (cluster number) to be changed 
		/// @param val New value to be set to the entry
		/// @return FRESULT FR_OK(0):succeeded, !=0:error
		FRESULT put_fat (FATFS* fs, DWORD clst,	DWORD val);
		
	#endif /* !FF_FS_READONLY */

	#if FF_FS_EXFAT && !FF_FS_READONLY

		/// @brief Find a contiguous free cluster block
		/// @param fs Filesystem object
		/// @param clst Cluster number to scan from
		/// @param ncl Number of contiguous clusters to find (1..)
		/// @return DWORD 0:Not found, 2..:Cluster block found, 0xFFFFFFFF:Disk error
		DWORD find_bitmap (FATFS* fs, DWORD clst, DWORD ncl);

		/// @brief Set/Clear a block of allocation bitmap
		/// @param fs Filesystem object
		/// @param clst Cluster number to change from
		/// @param ncl Number of clusters to be changed
		/// @param bv bit value to be set (0 or 1) 
		/// @return FRESULT 
		FRESULT change_bitmap (FATFS* fs, DWORD clst, DWORD ncl, int bv);

		/// @brief Fill the first fragment of the FAT chain
		/// @param obj Pointer to the corresponding object 
		/// @return FRESULT 
		FRESULT fill_first_frag (FFOBJID* obj);

		/// @brief Fill the last fragment of the FAT chain
		/// @param obj Pointer to the corresponding object
		/// @param lcl Last cluster of the fragment
		/// @param term Value to set the last FAT entry 
		/// @return FRESULT 
		FRESULT fill_last_frag (FFOBJID* obj, DWORD lcl, DWORD term);

	#endif	/* FF_FS_EXFAT && !FF_FS_READONLY */				

	#if !FF_FS_READONLY

		/// @brief FAT handling - Remove a cluster chain  
		/// @param obj Corresponding object
		/// @param clst Cluster to remove a chain from
		/// @param pclst Previous cluster of clst (0 if entire chain)
		/// @return FRESULT FR_OK(0):succeeded, !=0:error
		FRESULT remove_chain (FFOBJID* obj,	DWORD clst,	DWORD pclst);

		/// @brief FAT handling - Stretch a chain or Create a new chain
		/// @param obj Corresponding object
		/// @param clst Cluster# to stretch, 0:Create a new chain
		/// @return DWORD 0:No free cluster, 1:Internal error, 0xFFFFFFFF:Disk error, >=2:New cluster#
		DWORD create_chain (FFOBJID* obj, DWORD clst);

	#endif /* !FF_FS_READONLY */

	#if !FF_FS_READONLY
		/// @brief Directory handling - Fill a cluster with zeros
		/// @param fs Filesystem object
		/// @param clst Directory table to clear
		/// @return FRESULT Returns FR_OK or FR_DISK_ERR
		FRESULT dir_clear (FATFS *fs, DWORD clst);

	#endif	/* !FF_FS_READONLY */	

	/// @brief Directory handling - Set directory index
	/// @param dp Pointer to directory object
	/// @param ofs Offset of directory table
	/// @return FRESULT FR_OK(0):succeeded, !=0:error
	FRESULT dir_sdi (DIR* dp, DWORD ofs);

	/// @brief Directory handling - Move directory table index next
	/// @param dp Pointer to the directory object
	/// @param stretch 0: Do not stretch table, 1: Stretch table if needed
	/// @return FRESULT FR_OK(0):succeeded, FR_NO_FILE:End of table, FR_DENIED:Could not stretch
	FRESULT dir_next (DIR* dp, int stretch);

	#if !FF_FS_READONLY
		
		/// @brief Directory handling - Reserve a block of directory entries
		/// @param dp Pointer to the directory object
		/// @param n_ent Number of contiguous entries to allocate
		/// @return FRESULT FR_OK(0):succeeded, !=0:error
		FRESULT dir_alloc (DIR* dp,	UINT n_ent);		

	#endif	/* !FF_FS_READONLY */

	#if FF_FS_EXFAT
		
		/// @brief exFAT: Get a directry entry block
		/// @param dp Reading direcotry object pointing top of the entry block to load
		/// @return FRESULT FR_INT_ERR: invalid entry block
		FRESULT load_xdir (DIR* dp);


		#if !FF_FS_READONLY || FF_FS_RPATH != 0
			
			/// @brief exFAT: Load the object's directory entry block
			/// @param dp Blank directory object to be used to access containing direcotry
			/// @param obj Object with its containing directory information
			/// @return FRESULT 
			FRESULT load_obj_xdir (DIR* dp,	const FFOBJID* obj);

		#endif

		#if !FF_FS_READONLY
			
			/// @brief exFAT: Store the directory entry block
			/// @param dp Pointer to the direcotry object 
			/// @return FRESULT 
			FRESULT store_xdir (DIR* dp);		

		#endif	/* !FF_FS_READONLY */
		
	#endif	/* FF_FS_EXFAT */

	#if FF_FS_MINIMIZE <= 1 || FF_FS_RPATH >= 2 || FF_USE_LABEL || FF_FS_EXFAT

		enum class Directory {
			READFILE = 0,
			READLABEL = 1
		};

		/// @brief Read an object from the directory
		/// @param dp Pointer to the directory object
		/// @param vol Filtered by 0:file/directory or 1:volume label
		/// @return FRESULT 
		FRESULT dir_read (DIR* dp, int vol);

	#endif	/* FF_FS_MINIMIZE <= 1 || FF_USE_LABEL || FF_FS_RPATH >= 2 */

	/// @brief Directory handling - Find an object in the directory
	/// @param dp Pointer to the directory object with the file name
	/// @return FRESULT FR_OK(0):succeeded, !=0:error 
	FRESULT dir_find (DIR* dp);

	#if !FF_FS_READONLY

		/// @brief Register an object to the directory     
		/// @param dp Target directory with object name to be created
		/// @return FRESULT FR_OK:succeeded, FR_DENIED:no free entry or too many SFN collision, FR_DISK_ERR:disk error
		FRESULT dir_register (DIR* dp);

	#endif /* !FF_FS_READONLY */

	#if !FF_FS_READONLY && FF_FS_MINIMIZE == 0

		/// @brief Remove an object from the directory   
		/// @param dp Directory object pointing the entry to be removed
		/// @return FRESULT FR_OK:Succeeded, FR_DISK_ERR:A disk error
		FRESULT dir_remove (DIR* dp);

	#endif /* !FF_FS_READONLY && FF_FS_MINIMIZE == 0 */

	/// @brief Follow a file path 
	/// @param dp Directory object to return last directory and found object
	/// @param path Full-path string to find a file or directory
	/// @return FRESULT FR_OK(0): successful, !=0: error code
	FRESULT follow_path (DIR* dp, const TCHAR* path);

	/// @brief Load a sector and check if it is an FAT VBR 
	/// @param fs Filesystem object
	/// @param sect Sector to load and check if it is an FAT-VBR or not
	/// @return UINT 0:FAT/FAT32 VBR, 1:exFAT VBR, 2:Not FAT and valid BS, 3:Not FAT and invalid BS, 4:Disk error
	UINT check_fs (FATFS* fs, LBA_t sect);

	/// @brief Find an FAT volume (It supports only generic partitioning rules, MBR, GPT and SFD)
	/// @param fs Filesystem object
	/// @param part Partition to fined = 0:auto, 1..:forced
	/// @return UINT Returns BS status found in the hosting drive
	UINT find_volume (FATFS* fs, UINT part);	

	/// @brief Determine logical drive number and mount the volume if needed
	/// @param path Pointer to pointer to the path name (drive number) 
	/// @param rfs Pointer to pointer to the found filesystem object
	/// @param mode !=0: Check write protection for write access 
	/// @return FRESULT FR_OK(0): successful, !=0: an error occurred
	FRESULT mount_volume (const TCHAR** path, FATFS** rfs, BYTE mode);

	/// @brief Check if the file/directory object is valid or not
	/// @param obj Pointer to the FFOBJID, the 1st member in the FIL/DIR object, to check validity
	/// @param rfs Pointer to pointer to the owner filesystem object to return
	/// @return FRESULT Returns FR_OK or FR_INVALID_OBJECT
	FRESULT validate (FFOBJID* obj,	FATFS** rfs);

	#if !FF_FS_READONLY && FF_USE_MKFS
		/*-----------------------------------------------------------------------*/
		/* Create FAT/exFAT volume (with sub-functions)                          */
		/*-----------------------------------------------------------------------*/

		#define N_SEC_TRACK 63			/* Sectors per track for determination of drive CHS */
		#define	GPT_ALIGN	0x100000	/* Alignment of partitions in GPT [byte] (>=128KB) */
		#define GPT_ITEMS	128			/* Number of GPT table size (>=128, sector aligned) */

		/// @brief Create partitions on the physical drive in format of MBR or GPT
		/// @param drv Physical drive number
		/// @param plst Partition list
		/// @param sys System ID (for only MBR, temp setting)
		/// @param buf Working buffer for a sector
		/// @return FRESULT 
		FRESULT create_partition (BYTE drv,	const LBA_t plst[], BYTE sys, BYTE* buf);	

	#endif
		
	#if !FF_FS_READONLY
		
		#define SZ_PUTC_BUF	64
		#define SZ_NUM_BUF	32
		
		/// @brief Output buffer and work area
		typedef struct {
			FIL *fp;				// Ptr to the writing file 
			int idx, nchr;			// Write index of buf[] (-1:error), number of encoding units written 
			#if FF_USE_LFN && FF_LFN_UNICODE == 1
				WCHAR hs;
			#elif FF_USE_LFN && FF_LFN_UNICODE == 2
				BYTE bs[4];
				UINT wi, ct;
			#endif
			BYTE buf[SZ_PUTC_BUF];	// Write buffer
		} putbuff;


		/// @brief Buffered file write with code conversion
		/// @param pb output buffer
		/// @param c 
		void putc_bfd (putbuff* pb, TCHAR c);

		/// @brief Flush remaining characters in the buffer
		/// @param pb output buffer
		/// @return int 
		int putc_flush (putbuff* pb);

		/// @brief Initialize write buffer
		/// @param pb output buffer
		/// @param fp 
		void putc_init (putbuff* pb, FIL* fp);	



		/*-----------------------------------------------------------------------*/
		/* Put a Formatted String to the File (with sub-functions)               */
		/*-----------------------------------------------------------------------*/
		#if FF_PRINT_FLOAT && FF_INTDEF == 2
			/// @brief Calculate log10(n) in integer output
			/// @param n 
			/// @return int 
			int ilog10 (double n);	

			/// @brief Calculate 10^n in integer input
			/// @param n 
			/// @return double 
			double i10x (int n);	

			/// @brief Compare two floats
			/// @param x 
			/// @param y 
			/// @return true 
			/// @return false 
			bool isEqual(double x, double y);

			/// @brief Convert float to ascii
			/// @param buf Buffer to output the floating point string
			/// @param val Value to output
			/// @param prec Number of fractional digits
			/// @param fmt Notation
			void ftoa (char* buf, double val, int prec, TCHAR fmt);


		#endif /* !FF_FS_READONLY */
	#endif /* FF_USE_STRFUNC */


	#if FF_CODE_PAGE == 0

		/// @brief Set Active Codepage for the Path Name
		/// @param cp Value to be set as active code page 
		/// @return FRESULT 
		FRESULT f_setcp (WORD cp);

	#endif	/* FF_CODE_PAGE == 0 */

};

/// @brief Template specialization for MMC types using SPI
using DriverSPI = Driver<DiskIO_MMC_SPI>;

/// @brief Constructor for Driver
/// @tparam DISKIO_HW Must be a derived type of DiskioHardwareBase
/// @param diskio The low-level Disk IO implementation
template<typename DISKIO_HW>
Driver<DISKIO_HW>::Driver(DISKIO_HW &diskio)
{
	m_diskio = std::unique_ptr<DISKIO_HW>(&diskio);
}

/*--------------------------------*/
/* File/Volume controls           */
/*--------------------------------*/

#if FF_STR_VOLUME_ID
#ifdef FF_VOLUME_STRS
static const char* const VolumeStr[FF_VOLUMES] = {FF_VOLUME_STRS};	/* Pre-defined volume ID */
#endif
#endif

#if FF_LBA64
#if FF_MIN_GPT > 0x100000000
#error Wrong FF_MIN_GPT setting
#endif
static const BYTE GUID_MS_Basic[16] = {0xA2,0xA0,0xD0,0xEB,0xE5,0xB9,0x33,0x44,0x87,0xC0,0x68,0xB6,0xB7,0x26,0x99,0xC7};
#endif

static FATFS* FatFs[FF_VOLUMES];	/* Pointer to the filesystem objects (logical drives) */
static WORD Fsid;	

/* Character code support macros */
#define IsSeparator(c)	((c) == '/' || (c) == '\\')
#define IsTerminator(c)	((UINT)(c) < (FF_USE_LFN ? ' ' : '!'))
#define IsSurrogate(c)	((c) >= 0xD800 && (c) <= 0xDFFF)
#define IsSurrogateH(c)	((c) >= 0xD800 && (c) <= 0xDBFF)
#define IsSurrogateL(c)	((c) >= 0xDC00 && (c) <= 0xDFFF)

/* Post process on fatal error in the file operations */
#define ABORT(fs, res)		{ fp->err = (BYTE)(res); LEAVE_FF(fs, res); }

#endif /* FF_DEFINED */

} // namespace fatfs

/// function implementations
#include <ff_driver_public_impl.hpp>
#include <ff_driver_hidden_impl.hpp>

#endif // __FF_HPP__