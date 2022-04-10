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


/// @brief The implementations of the private templated functions from fatfs::Driver (ff_driver.cpp)

#ifndef __FF_DRIVER_HIDDEN_IMPL_HPP__
#define __FF_DRIVER_HIDDEN_IMPL_HPP__

namespace fatfs
{


#if !FF_FS_READONLY

template<typename DISKIO_HW>
FRESULT Driver<DISKIO_HW>::sync_window (FATFS* fs)
{
	FRESULT res = FR_OK;


	if (fs->wflag) {	/* Is the disk access window dirty? */
		if (m_diskio->write(fs->pdrv, fs->win, fs->winsect, 1) == DiskioHardwareBase::DRESULT::RES_OK) {	/* Write it back into the volume */
			fs->wflag = 0;	/* Clear window dirty flag */
			if (fs->winsect - fs->fatbase < fs->fsize) {	/* Is it in the 1st FAT? */
				if (fs->n_fats == 2) m_diskio->write(fs->pdrv, fs->win, fs->winsect + fs->fsize, 1);	/* Reflect it to 2nd FAT if needed */
			}
		} else {
			res = FR_DISK_ERR;
		}
	}
	return res;
}
#endif    

template<typename DISKIO_HW>
FRESULT Driver<DISKIO_HW>::validate (FFOBJID* obj,	FATFS** rfs)
{
	FRESULT res = FR_INVALID_OBJECT;


	if (obj && obj->fs && obj->fs->fs_type && obj->id == obj->fs->id) {	/* Test if the object is valid */
	#if FF_FS_REENTRANT
			if (lock_fs(obj->fs)) {	/* Obtain the filesystem object */
				if (!(m_diskio->status(obj->fs->pdrv) & STA_NOINIT)) { /* Test if the phsical drive is kept initialized */
					res = FR_OK;
				} else {
					unlock_fs(obj->fs, FR_OK);
				}
			} else {
				res = FR_TIMEOUT;
			}
	#else
			if (!(m_diskio->status(obj->fs->pdrv) & DiskioHardwareBase::STA_NOINIT)) { /* Test if the phsical drive is kept initialized */
				res = FR_OK;
			}
	#endif
	}
	*rfs = (res == FR_OK) ? obj->fs : 0;	/* Corresponding filesystem object */
	return res;
}

template<typename DISKIO_HW>
FRESULT Driver<DISKIO_HW>::mount_volume (const TCHAR** path, FATFS** rfs, BYTE mode)
{
	int vol;
	DiskioHardwareBase::DSTATUS stat;
	LBA_t bsect;
	DWORD tsect, sysect, fasize, nclst, szbfat;
	WORD nrsv;
	FATFS *fs;
	UINT fmt;


	/* Get logical drive number */
	*rfs = 0;
	vol = get_ldnumber(path);
	if (vol < 0) return FR_INVALID_DRIVE;

	/* Check if the filesystem object is valid or not */
	fs = FatFs[vol];					/* Get pointer to the filesystem object */
	if (!fs) return FR_NOT_ENABLED;		/* Is the filesystem object available? */
#if FF_FS_REENTRANT
	if (!lock_fs(fs)) return FR_TIMEOUT;	/* Lock the volume */
#endif
	*rfs = fs;							/* Return pointer to the filesystem object */

	mode &= (BYTE)~FA_READ;				/* Desired access mode, write access or not */
	if (fs->fs_type != 0) {				/* If the volume has been mounted */
		stat = m_diskio->status(fs->pdrv);
		if (!(stat & DiskioHardwareBase::STA_NOINIT)) {		/* and the physical drive is kept initialized */
			if (!FF_FS_READONLY && mode && (stat & DiskioHardwareBase::STA_PROTECT)) {	/* Check write protection if needed */
				return FR_WRITE_PROTECTED;
			}
			return FR_OK;				/* The filesystem object is already valid */
		}
	}

	/* The filesystem object is not valid. */
	/* Following code attempts to mount the volume. (find an FAT volume, analyze the BPB and initialize the filesystem object) */

	fs->fs_type = 0;					/* Clear the filesystem object */
	fs->pdrv = LD2PD(vol);				/* Volume hosting physical drive */
	stat = m_diskio->initialize(fs->pdrv);	/* Initialize the physical drive */
	if (stat & DiskioHardwareBase::STA_NOINIT) { 			/* Check if the initialization succeeded */
		return FR_NOT_READY;			/* Failed to initialize due to no medium or hard error */
	}
	if (!FF_FS_READONLY && mode && (stat & DiskioHardwareBase::STA_PROTECT)) { /* Check disk write protection if needed */
		return FR_WRITE_PROTECTED;
	}
#if FF_MAX_SS != FF_MIN_SS				/* Get sector size (multiple sector size cfg only) */
	if (m_diskio->ioctl(fs->pdrv, GET_SECTOR_SIZE, &SS(fs)) != DiskioHardwareBase::DRESULT::RES_OK) return FR_DISK_ERR;
	if (SS(fs) > FF_MAX_SS || SS(fs) < FF_MIN_SS || (SS(fs) & (SS(fs) - 1))) return FR_DISK_ERR;
#endif

	/* Find an FAT volume on the drive */
	fmt = find_volume(fs, LD2PT(vol));
	if (fmt == 4) return FR_DISK_ERR;		/* An error occured in the disk I/O layer */
	if (fmt >= 2) return FR_NO_FILESYSTEM;	/* No FAT volume is found */
	bsect = fs->winsect;					/* Volume offset */

	/* An FAT volume is found (bsect). Following code initializes the filesystem object */

#if FF_FS_EXFAT
	if (fmt == 1) {
		QWORD maxlba;
		DWORD so, cv, bcl, i;

		for (i = BPB_ZeroedEx; i < BPB_ZeroedEx + 53 && fs->win[i] == 0; i++) ;	/* Check zero filler */
		if (i < BPB_ZeroedEx + 53) return FR_NO_FILESYSTEM;

		if (ld_word(fs->win + BPB_FSVerEx) != 0x100) return FR_NO_FILESYSTEM;	/* Check exFAT version (must be version 1.0) */

		if (1 << fs->win[BPB_BytsPerSecEx] != SS(fs)) {	/* (BPB_BytsPerSecEx must be equal to the physical sector size) */
			return FR_NO_FILESYSTEM;
		}

		maxlba = ld_qword(fs->win + BPB_TotSecEx) + bsect;	/* Last LBA of the volume + 1 */
		if (!FF_LBA64 && maxlba >= 0x100000000) return FR_NO_FILESYSTEM;	/* (It cannot be accessed in 32-bit LBA) */

		fs->fsize = ld_dword(fs->win + BPB_FatSzEx);	/* Number of sectors per FAT */

		fs->n_fats = fs->win[BPB_NumFATsEx];			/* Number of FATs */
		if (fs->n_fats != 1) return FR_NO_FILESYSTEM;	/* (Supports only 1 FAT) */

		fs->csize = 1 << fs->win[BPB_SecPerClusEx];		/* Cluster size */
		if (fs->csize == 0)	return FR_NO_FILESYSTEM;	/* (Must be 1..32768 sectors) */

		nclst = ld_dword(fs->win + BPB_NumClusEx);		/* Number of clusters */
		if (nclst > MAX_EXFAT) return FR_NO_FILESYSTEM;	/* (Too many clusters) */
		fs->n_fatent = nclst + 2;

		/* Boundaries and Limits */
		fs->volbase = bsect;
		fs->database = bsect + ld_dword(fs->win + BPB_DataOfsEx);
		fs->fatbase = bsect + ld_dword(fs->win + BPB_FatOfsEx);
		if (maxlba < (QWORD)fs->database + nclst * fs->csize) return FR_NO_FILESYSTEM;	/* (Volume size must not be smaller than the size requiered) */
		fs->dirbase = ld_dword(fs->win + BPB_RootClusEx);

		/* Get bitmap location and check if it is contiguous (implementation assumption) */
		so = i = 0;
		for (;;) {	/* Find the bitmap entry in the root directory (in only first cluster) */
			if (i == 0) {
				if (so >= fs->csize) return FR_NO_FILESYSTEM;	/* Not found? */
				if (move_window(fs, clst2sect(fs, (DWORD)fs->dirbase) + so) != FR_OK) return FR_DISK_ERR;
				so++;
			}
			if (fs->win[i] == ET_BITMAP) break;			/* Is it a bitmap entry? */
			i = (i + SZDIRE) % SS(fs);	/* Next entry */
		}
		bcl = ld_dword(fs->win + i + 20);				/* Bitmap cluster */
		if (bcl < 2 || bcl >= fs->n_fatent) return FR_NO_FILESYSTEM;	/* (Wrong cluster#) */
		fs->bitbase = fs->database + fs->csize * (bcl - 2);	/* Bitmap sector */
		for (;;) {	/* Check if bitmap is contiguous */
			if (move_window(fs, fs->fatbase + bcl / (SS(fs) / 4)) != FR_OK) return FR_DISK_ERR;
			cv = ld_dword(fs->win + bcl % (SS(fs) / 4) * 4);
			if (cv == 0xFFFFFFFF) break;				/* Last link? */
			if (cv != ++bcl) return FR_NO_FILESYSTEM;	/* Fragmented? */
		}

#if !FF_FS_READONLY
		fs->last_clst = fs->free_clst = 0xFFFFFFFF;		/* Initialize cluster allocation information */
#endif
		fmt = FS_EXFAT;			/* FAT sub-type */
	} else
#endif	/* FF_FS_EXFAT */
	{
		if (ld_word(fs->win + BPB_BytsPerSec) != SS(fs)) return FR_NO_FILESYSTEM;	/* (BPB_BytsPerSec must be equal to the physical sector size) */

		fasize = ld_word(fs->win + BPB_FATSz16);		/* Number of sectors per FAT */
		if (fasize == 0) fasize = ld_dword(fs->win + BPB_FATSz32);
		fs->fsize = fasize;

		fs->n_fats = fs->win[BPB_NumFATs];				/* Number of FATs */
		if (fs->n_fats != 1 && fs->n_fats != 2) return FR_NO_FILESYSTEM;	/* (Must be 1 or 2) */
		fasize *= fs->n_fats;							/* Number of sectors for FAT area */

		fs->csize = fs->win[BPB_SecPerClus];			/* Cluster size */
		if (fs->csize == 0 || (fs->csize & (fs->csize - 1))) return FR_NO_FILESYSTEM;	/* (Must be power of 2) */

		fs->n_rootdir = ld_word(fs->win + BPB_RootEntCnt);	/* Number of root directory entries */
		if (fs->n_rootdir % (SS(fs) / SZDIRE)) return FR_NO_FILESYSTEM;	/* (Must be sector aligned) */

		tsect = ld_word(fs->win + BPB_TotSec16);		/* Number of sectors on the volume */
		if (tsect == 0) tsect = ld_dword(fs->win + BPB_TotSec32);

		nrsv = ld_word(fs->win + BPB_RsvdSecCnt);		/* Number of reserved sectors */
		if (nrsv == 0) return FR_NO_FILESYSTEM;			/* (Must not be 0) */

		/* Determine the FAT sub type */
		sysect = nrsv + fasize + fs->n_rootdir / (SS(fs) / SZDIRE);	/* RSV + FAT + DIR */
		if (tsect < sysect) return FR_NO_FILESYSTEM;	/* (Invalid volume size) */
		nclst = (tsect - sysect) / fs->csize;			/* Number of clusters */
		if (nclst == 0) return FR_NO_FILESYSTEM;		/* (Invalid volume size) */
		fmt = 0;
		if (nclst <= MAX_FAT32) fmt = FS_FAT32;
		if (nclst <= MAX_FAT16) fmt = FS_FAT16;
		if (nclst <= MAX_FAT12) fmt = FS_FAT12;
		if (fmt == 0) return FR_NO_FILESYSTEM;

		/* Boundaries and Limits */
		fs->n_fatent = nclst + 2;						/* Number of FAT entries */
		fs->volbase = bsect;							/* Volume start sector */
		fs->fatbase = bsect + nrsv; 					/* FAT start sector */
		fs->database = bsect + sysect;					/* Data start sector */
		if (fmt == FS_FAT32) {
			if (ld_word(fs->win + BPB_FSVer32) != 0) return FR_NO_FILESYSTEM;	/* (Must be FAT32 revision 0.0) */
			if (fs->n_rootdir != 0) return FR_NO_FILESYSTEM;	/* (BPB_RootEntCnt must be 0) */
			fs->dirbase = ld_dword(fs->win + BPB_RootClus32);	/* Root directory start cluster */
			szbfat = fs->n_fatent * 4;					/* (Needed FAT size) */
		} else {
			if (fs->n_rootdir == 0)	return FR_NO_FILESYSTEM;	/* (BPB_RootEntCnt must not be 0) */
			fs->dirbase = fs->fatbase + fasize;			/* Root directory start sector */
			szbfat = (fmt == FS_FAT16) ?				/* (Needed FAT size) */
				fs->n_fatent * 2 : fs->n_fatent * 3 / 2 + (fs->n_fatent & 1);
		}
		if (fs->fsize < (szbfat + (SS(fs) - 1)) / SS(fs)) return FR_NO_FILESYSTEM;	/* (BPB_FATSz must not be less than the size needed) */

#if !FF_FS_READONLY
		/* Get FSInfo if available */
		fs->last_clst = fs->free_clst = 0xFFFFFFFF;		/* Initialize cluster allocation information */
		fs->fsi_flag = 0x80;
#if (FF_FS_NOFSINFO & 3) != 3
		if (fmt == FS_FAT32				/* Allow to update FSInfo only if BPB_FSInfo32 == 1 */
			&& ld_word(fs->win + BPB_FSInfo32) == 1
			&& move_window(fs, bsect + 1) == FR_OK)
		{
			fs->fsi_flag = 0;
			if (ld_word(fs->win + BS_55AA) == 0xAA55	/* Load FSInfo data if available */
				&& ld_dword(fs->win + FSI_LeadSig) == 0x41615252
				&& ld_dword(fs->win + FSI_StrucSig) == 0x61417272)
			{
#if (FF_FS_NOFSINFO & 1) == 0
				fs->free_clst = ld_dword(fs->win + FSI_Free_Count);
#endif
#if (FF_FS_NOFSINFO & 2) == 0
				fs->last_clst = ld_dword(fs->win + FSI_Nxt_Free);
#endif
			}
		}
#endif	/* (FF_FS_NOFSINFO & 3) != 3 */
#endif	/* !FF_FS_READONLY */
	}

	fs->fs_type = (BYTE)fmt;/* FAT sub-type */
	fs->id = ++Fsid;		/* Volume mount ID */
#if FF_USE_LFN == 1
	fs->lfnbuf = LfnBuf;	/* Static LFN working buffer */
#if FF_FS_EXFAT
	fs->dirbuf = DirBuf;	/* Static directory block scratchpad buuffer */
#endif
#endif
#if FF_FS_RPATH != 0
	fs->cdir = 0;			/* Initialize current directory */
#endif
#if FF_FS_LOCK != 0			/* Clear file lock semaphores */
	clear_lock(fs);
#endif
	return FR_OK;
}

template<typename DISKIO_HW>
UINT Driver<DISKIO_HW>::check_fs (FATFS* fs, LBA_t sect)
{
	WORD w, sign;
	BYTE b;


	fs->wflag = 0; fs->winsect = (LBA_t)0 - 1;		/* Invaidate window */
	if (move_window(fs, sect) != FR_OK) return 4;	/* Load the boot sector */
	sign = ld_word(fs->win + BS_55AA);
#if FF_FS_EXFAT
	if (sign == 0xAA55 && !memcmp(fs->win + BS_JmpBoot, "\xEB\x76\x90" "EXFAT   ", 11)) return 1;	/* It is an exFAT VBR */
#endif
	b = fs->win[BS_JmpBoot];
	if (b == 0xEB || b == 0xE9 || b == 0xE8) {	/* Valid JumpBoot code? (short jump, near jump or near call) */
		if (sign == 0xAA55 && !memcmp(fs->win + BS_FilSysType32, "FAT32   ", 8)) {
			return 0;	/* It is an FAT32 VBR */
		}
		/* FAT volumes formatted with early MS-DOS lack BS_55AA and BS_FilSysType, so FAT VBR needs to be identified without them. */
		w = ld_word(fs->win + BPB_BytsPerSec);
		b = fs->win[BPB_SecPerClus];
		if ((w & (w - 1)) == 0 && w >= FF_MIN_SS && w <= FF_MAX_SS	/* Properness of sector size (512-4096 and 2^n) */
			&& b != 0 && (b & (b - 1)) == 0				/* Properness of cluster size (2^n) */
			&& ld_word(fs->win + BPB_RsvdSecCnt) != 0	/* Properness of reserved sectors (MNBZ) */
			&& (UINT)fs->win[BPB_NumFATs] - 1 <= 1		/* Properness of FATs (1 or 2) */
			&& ld_word(fs->win + BPB_RootEntCnt) != 0	/* Properness of root dir entries (MNBZ) */
			&& (ld_word(fs->win + BPB_TotSec16) >= 128 || ld_dword(fs->win + BPB_TotSec32) >= 0x10000)	/* Properness of volume sectors (>=128) */
			&& ld_word(fs->win + BPB_FATSz16) != 0) {	/* Properness of FAT size (MNBZ) */
				return 0;	/* It can be presumed an FAT VBR */
		}
	}
	return sign == 0xAA55 ? 2 : 3;	/* Not an FAT VBR (valid or invalid BS) */
}


template<typename DISKIO_HW>
UINT Driver<DISKIO_HW>::find_volume (FATFS* fs, UINT part)
{
	UINT fmt, i;
	DWORD mbr_pt[4];


	fmt = check_fs(fs, 0);				/* Load sector 0 and check if it is an FAT VBR as SFD format */
	if (fmt != 2 && (fmt >= 3 || part == 0)) return fmt;	/* Returns if it is an FAT VBR as auto scan, not a BS or disk error */

	/* Sector 0 is not an FAT VBR or forced partition number wants a partition */

#if FF_LBA64
	if (fs->win[MBR_Table + PTE_System] == 0xEE) {	/* GPT protective MBR? */
		DWORD n_ent, v_ent, ofs;
		QWORD pt_lba;

		if (move_window(fs, 1) != FR_OK) return 4;	/* Load GPT header sector (next to MBR) */
		if (!test_gpt_header(fs->win)) return 3;	/* Check if GPT header is valid */
		n_ent = ld_dword(fs->win + GPTH_PtNum);		/* Number of entries */
		pt_lba = ld_qword(fs->win + GPTH_PtOfs);	/* Table location */
		for (v_ent = i = 0; i < n_ent; i++) {		/* Find FAT partition */
			if (move_window(fs, pt_lba + i * SZ_GPTE / SS(fs)) != FR_OK) return 4;	/* PT sector */
			ofs = i * SZ_GPTE % SS(fs);												/* Offset in the sector */
			if (!memcmp(fs->win + ofs + GPTE_PtGuid, GUID_MS_Basic, 16)) {	/* MS basic data partition? */
				v_ent++;
				fmt = check_fs(fs, ld_qword(fs->win + ofs + GPTE_FstLba));	/* Load VBR and check status */
				if (part == 0 && fmt <= 1) return fmt;			/* Auto search (valid FAT volume found first) */
				if (part != 0 && v_ent == part) return fmt;		/* Forced partition order (regardless of it is valid or not) */
			}
		}
		return 3;	/* Not found */
	}
#endif
	if (FF_MULTI_PARTITION && part > 4) return 3;	/* MBR has 4 partitions max */
	for (i = 0; i < 4; i++) {		/* Load partition offset in the MBR */
		mbr_pt[i] = ld_dword(fs->win + MBR_Table + i * SZ_PTE + PTE_StLba);
	}
	i = part ? part - 1 : 0;		/* Table index to find first */
	do {							/* Find an FAT volume */
		fmt = mbr_pt[i] ? check_fs(fs, mbr_pt[i]) : 3;	/* Check if the partition is FAT */
	} while (part == 0 && fmt >= 2 && ++i < 4);
	return fmt;
}


template<typename DISKIO_HW>
FRESULT Driver<DISKIO_HW>::move_window (FATFS* fs, LBA_t sect)
{
	FRESULT res = FR_OK;


	if (sect != fs->winsect) {	/* Window offset changed? */
#if !FF_FS_READONLY
		res = sync_window(fs);		/* Flush the window */
#endif
		if (res == FR_OK) {			/* Fill sector window with new data */
			if (m_diskio->read(fs->pdrv, fs->win, sect, 1) != DiskioHardwareBase::DRESULT::RES_OK) {
				sect = (LBA_t)0 - 1;	/* Invalidate window if read data is not valid */
				res = FR_DISK_ERR;
			}
			fs->winsect = sect;
		}
	}
	return res;
}

#if !FF_FS_READONLY

template<typename DISKIO_HW>
FRESULT Driver<DISKIO_HW>::sync_fs (FATFS* fs)
{
	FRESULT res;


	res = sync_window(fs);
	if (res == FR_OK) {
		if (fs->fs_type == FS_FAT32 && fs->fsi_flag == 1) {	/* FAT32: Update FSInfo sector if needed */
			/* Create FSInfo structure */
			std::memset(fs->win, 0, sizeof fs->win);
			st_word(fs->win + BS_55AA, 0xAA55);					/* Boot signature */
			st_dword(fs->win + FSI_LeadSig, 0x41615252);		/* Leading signature */
			st_dword(fs->win + FSI_StrucSig, 0x61417272);		/* Structure signature */
			st_dword(fs->win + FSI_Free_Count, fs->free_clst);	/* Number of free clusters */
			st_dword(fs->win + FSI_Nxt_Free, fs->last_clst);	/* Last allocated culuster */
			fs->winsect = fs->volbase + 1;						/* Write it into the FSInfo sector (Next to VBR) */
			m_diskio->write(fs->pdrv, fs->win, fs->winsect, 1);
			fs->fsi_flag = 0;
		}
		/* Make sure that no pending write process in the lower layer */
		if (m_diskio->ioctl(fs->pdrv, DiskioHardwareBase::CTRL_SYNC, 0) != DiskioHardwareBase::DRESULT::RES_OK) res = FR_DISK_ERR;
	}

	return res;
}

#endif // !FF_FS_READONLY

template<typename DISKIO_HW>
DWORD Driver<DISKIO_HW>::get_fat (FFOBJID* obj, DWORD clst)
{
	UINT wc, bc;
	DWORD val;
	FATFS *fs = obj->fs;


	if (clst < 2 || clst >= fs->n_fatent) {	/* Check if in valid range */
		val = 1;	/* Internal error */

	} else {
		val = 0xFFFFFFFF;	/* Default value falls on disk error */

		switch (fs->fs_type) {
		case FS_FAT12 :
			bc = (UINT)clst; bc += bc / 2;
			if (move_window(fs, fs->fatbase + (bc / SS(fs))) != FR_OK) break;
			wc = fs->win[bc++ % SS(fs)];		/* Get 1st byte of the entry */
			if (move_window(fs, fs->fatbase + (bc / SS(fs))) != FR_OK) break;
			wc |= fs->win[bc % SS(fs)] << 8;	/* Merge 2nd byte of the entry */
			val = (clst & 1) ? (wc >> 4) : (wc & 0xFFF);	/* Adjust bit position */
			break;

		case FS_FAT16 :
			if (move_window(fs, fs->fatbase + (clst / (SS(fs) / 2))) != FR_OK) break;
			val = ld_word(fs->win + clst * 2 % SS(fs));		/* Simple WORD array */
			break;

		case FS_FAT32 :
			if (move_window(fs, fs->fatbase + (clst / (SS(fs) / 4))) != FR_OK) break;
			val = ld_dword(fs->win + clst * 4 % SS(fs)) & 0x0FFFFFFF;	/* Simple DWORD array but mask out upper 4 bits */
			break;
#if FF_FS_EXFAT
		case FS_EXFAT :
			if ((obj->objsize != 0 && obj->sclust != 0) || obj->stat == 0) {	/* Object except root dir must have valid data length */
				DWORD cofs = clst - obj->sclust;	/* Offset from start cluster */
				DWORD clen = (DWORD)((LBA_t)((obj->objsize - 1) / SS(fs)) / fs->csize);	/* Number of clusters - 1 */

				if (obj->stat == 2 && cofs <= clen) {	/* Is it a contiguous chain? */
					val = (cofs == clen) ? 0x7FFFFFFF : clst + 1;	/* No data on the FAT, generate the value */
					break;
				}
				if (obj->stat == 3 && cofs < obj->n_cont) {	/* Is it in the 1st fragment? */
					val = clst + 1; 	/* Generate the value */
					break;
				}
				if (obj->stat != 2) {	/* Get value from FAT if FAT chain is valid */
					if (obj->n_frag != 0) {	/* Is it on the growing edge? */
						val = 0x7FFFFFFF;	/* Generate EOC */
					} else {
						if (move_window(fs, fs->fatbase + (clst / (SS(fs) / 4))) != FR_OK) break;
						val = ld_dword(fs->win + clst * 4 % SS(fs)) & 0x7FFFFFFF;
					}
					break;
				}
			}
			val = 1;	/* Internal error */
			break;
#endif
		default:
			val = 1;	/* Internal error */
		}
	}

	return val;
}


#if !FF_FS_READONLY

template<typename DISKIO_HW>
FRESULT Driver<DISKIO_HW>::put_fat (FATFS* fs,	DWORD clst,	DWORD val)
{
	UINT bc;
	BYTE *p;
	FRESULT res = FR_INT_ERR;


	if (clst >= 2 && clst < fs->n_fatent) {	/* Check if in valid range */
		switch (fs->fs_type) {
		case FS_FAT12:
			bc = (UINT)clst; bc += bc / 2;	/* bc: byte offset of the entry */
			res = move_window(fs, fs->fatbase + (bc / SS(fs)));
			if (res != FR_OK) break;
			p = fs->win + bc++ % SS(fs);
			*p = (clst & 1) ? ((*p & 0x0F) | ((BYTE)val << 4)) : (BYTE)val;	/* Update 1st byte */
			fs->wflag = 1;
			res = move_window(fs, fs->fatbase + (bc / SS(fs)));
			if (res != FR_OK) break;
			p = fs->win + bc % SS(fs);
			*p = (clst & 1) ? (BYTE)(val >> 4) : ((*p & 0xF0) | ((BYTE)(val >> 8) & 0x0F));	/* Update 2nd byte */
			fs->wflag = 1;
			break;

		case FS_FAT16:
			res = move_window(fs, fs->fatbase + (clst / (SS(fs) / 2)));
			if (res != FR_OK) break;
			st_word(fs->win + clst * 2 % SS(fs), (WORD)val);	/* Simple WORD array */
			fs->wflag = 1;
			break;

		case FS_FAT32:
#if FF_FS_EXFAT
		case FS_EXFAT:
#endif
			res = move_window(fs, fs->fatbase + (clst / (SS(fs) / 4)));
			if (res != FR_OK) break;
			if (!FF_FS_EXFAT || fs->fs_type != FS_EXFAT) {
				val = (val & 0x0FFFFFFF) | (ld_dword(fs->win + clst * 4 % SS(fs)) & 0xF0000000);
			}
			st_dword(fs->win + clst * 4 % SS(fs), val);
			fs->wflag = 1;
			break;
		}
	}
	return res;
}

#endif /* !FF_FS_READONLY */




#if FF_FS_EXFAT && !FF_FS_READONLY

template<typename DISKIO_HW>
DWORD Driver<DISKIO_HW>::find_bitmap (FATFS* fs, DWORD clst, DWORD ncl)
{
	BYTE bm, bv;
	UINT i;
	DWORD val, scl, ctr;


	clst -= 2;	/* The first bit in the bitmap corresponds to cluster #2 */
	if (clst >= fs->n_fatent - 2) clst = 0;
	scl = val = clst; ctr = 0;
	for (;;) {
		if (move_window(fs, fs->bitbase + val / 8 / SS(fs)) != FR_OK) return 0xFFFFFFFF;
		i = val / 8 % SS(fs); bm = 1 << (val % 8);
		do {
			do {
				bv = fs->win[i] & bm; bm <<= 1;		/* Get bit value */
				if (++val >= fs->n_fatent - 2) {	/* Next cluster (with wrap-around) */
					val = 0; bm = 0; i = SS(fs);
				}
				if (bv == 0) {	/* Is it a free cluster? */
					if (++ctr == ncl) return scl + 2;	/* Check if run length is sufficient for required */
				} else {
					scl = val; ctr = 0;		/* Encountered a cluster in-use, restart to scan */
				}
				if (val == clst) return 0;	/* All cluster scanned? */
			} while (bm != 0);
			bm = 1;
		} while (++i < SS(fs));
	}
}

template<typename DISKIO_HW>
FRESULT Driver<DISKIO_HW>::change_bitmap (FATFS* fs, DWORD clst, DWORD ncl, int bv)
{
	BYTE bm;
	UINT i;
	LBA_t sect;


	clst -= 2;	/* The first bit corresponds to cluster #2 */
	sect = fs->bitbase + clst / 8 / SS(fs);	/* Sector address */
	i = clst / 8 % SS(fs);					/* Byte offset in the sector */
	bm = 1 << (clst % 8);					/* Bit mask in the byte */
	for (;;) {
		if (move_window(fs, sect++) != FR_OK) return FR_DISK_ERR;
		do {
			do {
				if (bv == (int)((fs->win[i] & bm) != 0)) return FR_INT_ERR;	/* Is the bit expected value? */
				fs->win[i] ^= bm;	/* Flip the bit */
				fs->wflag = 1;
				if (--ncl == 0) return FR_OK;	/* All bits processed? */
			} while (bm <<= 1);		/* Next bit */
			bm = 1;
		} while (++i < SS(fs));		/* Next byte */
		i = 0;
	}
}

template<typename DISKIO_HW>
FRESULT Driver<DISKIO_HW>::fill_first_frag (FFOBJID* obj)
{
	FRESULT res;
	DWORD cl, n;


	if (obj->stat == 3) {	/* Has the object been changed 'fragmented' in this session? */
		for (cl = obj->sclust, n = obj->n_cont; n; cl++, n--) {	/* Create cluster chain on the FAT */
			res = put_fat(obj->fs, cl, cl + 1);
			if (res != FR_OK) return res;
		}
		obj->stat = 0;	/* Change status 'FAT chain is valid' */
	}
	return FR_OK;
}

template<typename DISKIO_HW>
FRESULT Driver<DISKIO_HW>::fill_last_frag (FFOBJID* obj, DWORD lcl, DWORD term)
{
	FRESULT res;


	while (obj->n_frag > 0) {	/* Create the chain of last fragment */
		res = put_fat(obj->fs, lcl - obj->n_frag + 1, (obj->n_frag > 1) ? lcl - obj->n_frag + 2 : term);
		if (res != FR_OK) return res;
		obj->n_frag--;
	}
	return FR_OK;
}

#endif	/* FF_FS_EXFAT && !FF_FS_READONLY */



#if !FF_FS_READONLY

template<typename DISKIO_HW>
FRESULT Driver<DISKIO_HW>::remove_chain (FFOBJID* obj,	DWORD clst,	DWORD pclst)
{
	FRESULT res = FR_OK;
	DWORD nxt;
	FATFS *fs = obj->fs;
#if FF_FS_EXFAT || FF_USE_TRIM
	DWORD scl = clst, ecl = clst;
#endif
#if FF_USE_TRIM
	LBA_t rt[2];
#endif

	if (clst < 2 || clst >= fs->n_fatent) return FR_INT_ERR;	/* Check if in valid range */

	/* Mark the previous cluster 'EOC' on the FAT if it exists */
	if (pclst != 0 && (!FF_FS_EXFAT || fs->fs_type != FS_EXFAT || obj->stat != 2)) {
		res = put_fat(fs, pclst, 0xFFFFFFFF);
		if (res != FR_OK) return res;
	}

	/* Remove the chain */
	do {
		nxt = get_fat(obj, clst);			/* Get cluster status */
		if (nxt == 0) break;				/* Empty cluster? */
		if (nxt == 1) return FR_INT_ERR;	/* Internal error? */
		if (nxt == 0xFFFFFFFF) return FR_DISK_ERR;	/* Disk error? */
		if (!FF_FS_EXFAT || fs->fs_type != FS_EXFAT) {
			res = put_fat(fs, clst, 0);		/* Mark the cluster 'free' on the FAT */
			if (res != FR_OK) return res;
		}
		if (fs->free_clst < fs->n_fatent - 2) {	/* Update FSINFO */
			fs->free_clst++;
			fs->fsi_flag |= 1;
		}
#if FF_FS_EXFAT || FF_USE_TRIM
		if (ecl + 1 == nxt) {	/* Is next cluster contiguous? */
			ecl = nxt;
		} else {				/* End of contiguous cluster block */
#if FF_FS_EXFAT
			if (fs->fs_type == FS_EXFAT) {
				res = change_bitmap(fs, scl, ecl - scl + 1, 0);	/* Mark the cluster block 'free' on the bitmap */
				if (res != FR_OK) return res;
			}
#endif
#if FF_USE_TRIM
			rt[0] = clst2sect(fs, scl);					/* Start of data area to be freed */
			rt[1] = clst2sect(fs, ecl) + fs->csize - 1;	/* End of data area to be freed */
			m_diskio->ioctl(fs->pdrv, DiskioHardwareBase::CTRL_TRIM, rt);		/* Inform storage device that the data in the block may be erased */
#endif
			scl = ecl = nxt;
		}
#endif
		clst = nxt;					/* Next cluster */
	} while (clst < fs->n_fatent);	/* Repeat while not the last link */

#if FF_FS_EXFAT
	/* Some post processes for chain status */
	if (fs->fs_type == FS_EXFAT) {
		if (pclst == 0) {	/* Has the entire chain been removed? */
			obj->stat = 0;		/* Change the chain status 'initial' */
		} else {
			if (obj->stat == 0) {	/* Is it a fragmented chain from the beginning of this session? */
				clst = obj->sclust;		/* Follow the chain to check if it gets contiguous */
				while (clst != pclst) {
					nxt = get_fat(obj, clst);
					if (nxt < 2) return FR_INT_ERR;
					if (nxt == 0xFFFFFFFF) return FR_DISK_ERR;
					if (nxt != clst + 1) break;	/* Not contiguous? */
					clst++;
				}
				if (clst == pclst) {	/* Has the chain got contiguous again? */
					obj->stat = 2;		/* Change the chain status 'contiguous' */
				}
			} else {
				if (obj->stat == 3 && pclst >= obj->sclust && pclst <= obj->sclust + obj->n_cont) {	/* Was the chain fragmented in this session and got contiguous again? */
					obj->stat = 2;	/* Change the chain status 'contiguous' */
				}
			}
		}
	}
#endif
	return FR_OK;
}


template<typename DISKIO_HW>
DWORD Driver<DISKIO_HW>::create_chain (FFOBJID* obj, DWORD clst)
{
	DWORD cs, ncl, scl;
	FRESULT res;
	FATFS *fs = obj->fs;


	if (clst == 0) {						/* Create a new chain */
		scl = fs->last_clst;				/* Suggested cluster to start to find */
		if (scl == 0 || scl >= fs->n_fatent) scl = 1;
	}
	else {									/* Stretch a chain */
		cs = get_fat(obj, clst);			/* Check the cluster status */
		if (cs < 2) return 1;				/* Test for insanity */
		if (cs == 0xFFFFFFFF) return cs;	/* Test for disk error */
		if (cs < fs->n_fatent) return cs;	/* It is already followed by next cluster */
		scl = clst;							/* Cluster to start to find */
	}
	if (fs->free_clst == 0) return 0;		/* No free cluster */

#if FF_FS_EXFAT
	if (fs->fs_type == FS_EXFAT) {					/* On the exFAT volume */
		ncl = find_bitmap(fs, scl, 1);				/* Find a free cluster */
		if (ncl == 0 || ncl == 0xFFFFFFFF) return ncl;	/* No free cluster or hard error? */
		res = change_bitmap(fs, ncl, 1, 1);			/* Mark the cluster 'in use' */
		if (res == FR_INT_ERR) return 1;
		if (res == FR_DISK_ERR) return 0xFFFFFFFF;
		if (clst == 0) {							/* Is it a new chain? */
			obj->stat = 2;							/* Set status 'contiguous' */
		} else {									/* It is a stretched chain */
			if (obj->stat == 2 && ncl != scl + 1) {	/* Is the chain got fragmented? */
				obj->n_cont = scl - obj->sclust;	/* Set size of the contiguous part */
				obj->stat = 3;						/* Change status 'just fragmented' */
			}
		}
		if (obj->stat != 2) {						/* Is the file non-contiguous? */
			if (ncl == clst + 1) {					/* Is the cluster next to previous one? */
				obj->n_frag = obj->n_frag ? obj->n_frag + 1 : 2;	/* Increment size of last framgent */
			} else {				/* New fragment */
				if (obj->n_frag == 0) obj->n_frag = 1;
				res = fill_last_frag(obj, clst, ncl);	/* Fill last fragment on the FAT and link it to new one */
				if (res == FR_OK) obj->n_frag = 1;
			}
		}
	} else
#endif
	{	/* On the FAT/FAT32 volume */
		ncl = 0;
		if (scl == clst) {						/* Stretching an existing chain? */
			ncl = scl + 1;						/* Test if next cluster is free */
			if (ncl >= fs->n_fatent) ncl = 2;
			cs = get_fat(obj, ncl);				/* Get next cluster status */
			if (cs == 1 || cs == 0xFFFFFFFF) return cs;	/* Test for error */
			if (cs != 0) {						/* Not free? */
				cs = fs->last_clst;				/* Start at suggested cluster if it is valid */
				if (cs >= 2 && cs < fs->n_fatent) scl = cs;
				ncl = 0;
			}
		}
		if (ncl == 0) {	/* The new cluster cannot be contiguous and find another fragment */
			ncl = scl;	/* Start cluster */
			for (;;) {
				ncl++;							/* Next cluster */
				if (ncl >= fs->n_fatent) {		/* Check wrap-around */
					ncl = 2;
					if (ncl > scl) return 0;	/* No free cluster found? */
				}
				cs = get_fat(obj, ncl);			/* Get the cluster status */
				if (cs == 0) break;				/* Found a free cluster? */
				if (cs == 1 || cs == 0xFFFFFFFF) return cs;	/* Test for error */
				if (ncl == scl) return 0;		/* No free cluster found? */
			}
		}
		res = put_fat(fs, ncl, 0xFFFFFFFF);		/* Mark the new cluster 'EOC' */
		if (res == FR_OK && clst != 0) {
			res = put_fat(fs, clst, ncl);		/* Link it from the previous one if needed */
		}
	}

	if (res == FR_OK) {			/* Update FSINFO if function succeeded. */
		fs->last_clst = ncl;
		if (fs->free_clst <= fs->n_fatent - 2) fs->free_clst--;
		fs->fsi_flag |= 1;
	} else {
		ncl = (res == FR_DISK_ERR) ? 0xFFFFFFFF : 1;	/* Failed. Generate error status */
	}

	return ncl;		/* Return new cluster number or error status */
}

#endif /* !FF_FS_READONLY */

#if !FF_FS_READONLY
template<typename DISKIO_HW>
FRESULT Driver<DISKIO_HW>::dir_clear (FATFS *fs, DWORD clst)
{
	LBA_t sect;
	UINT n, szb;
	BYTE *ibuf;


	if (sync_window(fs) != FR_OK) return FR_DISK_ERR;	/* Flush disk access window */
	sect = clst2sect(fs, clst);		/* Top of the cluster */
	fs->winsect = sect;				/* Set window to top of the cluster */
	std::memset(fs->win, 0, sizeof fs->win);	/* Clear window buffer */
#if FF_USE_LFN == 3		/* Quick table clear by using multi-secter write */
	/* Allocate a temporary buffer */
	for (szb = ((DWORD)fs->csize * SS(fs) >= MAX_MALLOC) ? MAX_MALLOC : fs->csize * SS(fs), ibuf = 0; szb > SS(fs) && (ibuf = ff_memalloc(szb)) == 0; szb /= 2) ;
	if (szb > SS(fs)) {		/* Buffer allocated? */
		std::memset(ibuf, 0, szb);
		szb /= SS(fs);		/* Bytes -> Sectors */
		for (n = 0; n < fs->csize && m_diskio->write(fs->pdrv, ibuf, sect + n, szb) == DiskioHardwareBase::DRESULT::RES_OK; n += szb) ;	/* Fill the cluster with 0 */
		ff_memfree(ibuf);
	} else
#endif
	{
		ibuf = fs->win; szb = 1;	/* Use window buffer (many single-sector writes may take a time) */
		for (n = 0; n < fs->csize && m_diskio->write(fs->pdrv, ibuf, sect + n, szb) == DiskioHardwareBase::DRESULT::RES_OK; n += szb) ;	/* Fill the cluster with 0 */
	}
	return (n == fs->csize) ? FR_OK : FR_DISK_ERR;
}
#endif	/* !FF_FS_READONLY */


template<typename DISKIO_HW>
FRESULT Driver<DISKIO_HW>::dir_sdi (DIR* dp, DWORD ofs)
{
	DWORD csz, clst;
	FATFS *fs = dp->obj.fs;


	if (ofs >= (DWORD)((FF_FS_EXFAT && fs->fs_type == FS_EXFAT) ? MAX_DIR_EX : MAX_DIR) || ofs % SZDIRE) {	/* Check range of offset and alignment */
		return FR_INT_ERR;
	}
	dp->dptr = ofs;				/* Set current offset */
	clst = dp->obj.sclust;		/* Table start cluster (0:root) */
	if (clst == 0 && fs->fs_type >= FS_FAT32) {	/* Replace cluster# 0 with root cluster# */
		clst = (DWORD)fs->dirbase;
		if (FF_FS_EXFAT) dp->obj.stat = 0;	/* exFAT: Root dir has an FAT chain */
	}

	if (clst == 0) {	/* Static table (root-directory on the FAT volume) */
		if (ofs / SZDIRE >= fs->n_rootdir) return FR_INT_ERR;	/* Is index out of range? */
		dp->sect = fs->dirbase;

	} else {			/* Dynamic table (sub-directory or root-directory on the FAT32/exFAT volume) */
		csz = (DWORD)fs->csize * SS(fs);	/* Bytes per cluster */
		while (ofs >= csz) {				/* Follow cluster chain */
			clst = get_fat(&dp->obj, clst);				/* Get next cluster */
			if (clst == 0xFFFFFFFF) return FR_DISK_ERR;	/* Disk error */
			if (clst < 2 || clst >= fs->n_fatent) return FR_INT_ERR;	/* Reached to end of table or internal error */
			ofs -= csz;
		}
		dp->sect = clst2sect(fs, clst);
	}
	dp->clust = clst;					/* Current cluster# */
	if (dp->sect == 0) return FR_INT_ERR;
	dp->sect += ofs / SS(fs);			/* Sector# of the directory entry */
	dp->dir = fs->win + (ofs % SS(fs));	/* Pointer to the entry in the win[] */

	return FR_OK;
}

template<typename DISKIO_HW>
FRESULT Driver<DISKIO_HW>::dir_next (DIR* dp,	int stretch)
{
	DWORD ofs, clst;
	FATFS *fs = dp->obj.fs;


	ofs = dp->dptr + SZDIRE;	/* Next entry */
	if (ofs >= (DWORD)((FF_FS_EXFAT && fs->fs_type == FS_EXFAT) ? MAX_DIR_EX : MAX_DIR)) dp->sect = 0;	/* Disable it if the offset reached the max value */
	if (dp->sect == 0) return FR_NO_FILE;	/* Report EOT if it has been disabled */

	if (ofs % SS(fs) == 0) {	/* Sector changed? */
		dp->sect++;				/* Next sector */

		if (dp->clust == 0) {	/* Static table */
			if (ofs / SZDIRE >= fs->n_rootdir) {	/* Report EOT if it reached end of static table */
				dp->sect = 0; return FR_NO_FILE;
			}
		}
		else {					/* Dynamic table */
			if ((ofs / SS(fs) & (fs->csize - 1)) == 0) {	/* Cluster changed? */
				clst = get_fat(&dp->obj, dp->clust);		/* Get next cluster */
				if (clst <= 1) return FR_INT_ERR;			/* Internal error */
				if (clst == 0xFFFFFFFF) return FR_DISK_ERR;	/* Disk error */
				if (clst >= fs->n_fatent) {					/* It reached end of dynamic table */
#if !FF_FS_READONLY
					if (!stretch) {								/* If no stretch, report EOT */
						dp->sect = 0; return FR_NO_FILE;
					}
					clst = create_chain(&dp->obj, dp->clust);	/* Allocate a cluster */
					if (clst == 0) return FR_DENIED;			/* No free cluster */
					if (clst == 1) return FR_INT_ERR;			/* Internal error */
					if (clst == 0xFFFFFFFF) return FR_DISK_ERR;	/* Disk error */
					if (dir_clear(fs, clst) != FR_OK) return FR_DISK_ERR;	/* Clean up the stretched table */
					if (FF_FS_EXFAT) dp->obj.stat |= 4;			/* exFAT: The directory has been stretched */
#else
					if (!stretch) dp->sect = 0;					/* (this line is to suppress compiler warning) */
					dp->sect = 0; return FR_NO_FILE;			/* Report EOT */
#endif
				}
				dp->clust = clst;		/* Initialize data for new cluster */
				dp->sect = clst2sect(fs, clst);
			}
		}
	}
	dp->dptr = ofs;						/* Current entry */
	dp->dir = fs->win + ofs % SS(fs);	/* Pointer to the entry in the win[] */

	return FR_OK;
}




#if !FF_FS_READONLY

template<typename DISKIO_HW>
FRESULT Driver<DISKIO_HW>::dir_alloc (DIR* dp,	UINT n_ent)
{
	FRESULT res;
	UINT n;
	FATFS *fs = dp->obj.fs;


	res = dir_sdi(dp, 0);
	if (res == FR_OK) {
		n = 0;
		do {
			res = move_window(fs, dp->sect);
			if (res != FR_OK) break;
#if FF_FS_EXFAT
			if ((fs->fs_type == FS_EXFAT) ? (int)((dp->dir[XDIR_Type] & 0x80) == 0) : (int)(dp->dir[DIR_Name] == DDEM || dp->dir[DIR_Name] == 0)) {	/* Is the entry free? */
#else
			if (dp->dir[DIR_Name] == DDEM || dp->dir[DIR_Name] == 0) {	/* Is the entry free? */
#endif
				if (++n == n_ent) break;	/* Is a block of contiguous free entries found? */
			} else {
				n = 0;				/* Not a free entry, restart to search */
			}
			res = dir_next(dp, 1);	/* Next entry with table stretch enabled */
		} while (res == FR_OK);
	}

	if (res == FR_NO_FILE) res = FR_DENIED;	/* No directory entry to allocate */
	return res;
}

#endif	/* !FF_FS_READONLY */



#if FF_FS_EXFAT

template<typename DISKIO_HW>
FRESULT Driver<DISKIO_HW>::load_xdir (DIR* dp)
{
	FRESULT res;
	UINT i, sz_ent;
	BYTE *dirb = dp->obj.fs->dirbuf;	/* Pointer to the on-memory direcotry entry block 85+C0+C1s */


	/* Load file directory entry */
	res = move_window(dp->obj.fs, dp->sect);
	if (res != FR_OK) return res;
	if (dp->dir[XDIR_Type] != ET_FILEDIR) return FR_INT_ERR;	/* Invalid order */
	memcpy(dirb + 0 * SZDIRE, dp->dir, SZDIRE);
	sz_ent = (dirb[XDIR_NumSec] + 1) * SZDIRE;
	if (sz_ent < 3 * SZDIRE || sz_ent > 19 * SZDIRE) return FR_INT_ERR;

	/* Load stream extension entry */
	res = dir_next(dp, 0);
	if (res == FR_NO_FILE) res = FR_INT_ERR;	/* It cannot be */
	if (res != FR_OK) return res;
	res = move_window(dp->obj.fs, dp->sect);
	if (res != FR_OK) return res;
	if (dp->dir[XDIR_Type] != ET_STREAM) return FR_INT_ERR;	/* Invalid order */
	memcpy(dirb + 1 * SZDIRE, dp->dir, SZDIRE);
	if (MAXDIRB(dirb[XDIR_NumName]) > sz_ent) return FR_INT_ERR;

	/* Load file name entries */
	i = 2 * SZDIRE;	/* Name offset to load */
	do {
		res = dir_next(dp, 0);
		if (res == FR_NO_FILE) res = FR_INT_ERR;	/* It cannot be */
		if (res != FR_OK) return res;
		res = move_window(dp->obj.fs, dp->sect);
		if (res != FR_OK) return res;
		if (dp->dir[XDIR_Type] != ET_FILENAME) return FR_INT_ERR;	/* Invalid order */
		if (i < MAXDIRB(FF_MAX_LFN)) memcpy(dirb + i, dp->dir, SZDIRE);
	} while ((i += SZDIRE) < sz_ent);

	/* Sanity check (do it for only accessible object) */
	if (i <= MAXDIRB(FF_MAX_LFN)) {
		if (xdir_sum(dirb) != ld_word(dirb + XDIR_SetSum)) return FR_INT_ERR;
	}
	return FR_OK;
}






#if !FF_FS_READONLY || FF_FS_RPATH != 0

template<typename DISKIO_HW>
FRESULT Driver<DISKIO_HW>::load_obj_xdir (DIR* dp,	const FFOBJID* obj)
{
	FRESULT res;

	/* Open object containing directory */
	dp->obj.fs = obj->fs;
	dp->obj.sclust = obj->c_scl;
	dp->obj.stat = (BYTE)obj->c_size;
	dp->obj.objsize = obj->c_size & 0xFFFFFF00;
	dp->obj.n_frag = 0;
	dp->blk_ofs = obj->c_ofs;

	res = dir_sdi(dp, dp->blk_ofs);	/* Goto object's entry block */
	if (res == FR_OK) {
		res = load_xdir(dp);		/* Load the object's entry block */
	}
	return res;
}
#endif


#if !FF_FS_READONLY

template<typename DISKIO_HW>
FRESULT Driver<DISKIO_HW>::store_xdir (DIR* dp)
{
	FRESULT res;
	UINT nent;
	BYTE *dirb = dp->obj.fs->dirbuf;	/* Pointer to the direcotry entry block 85+C0+C1s */

	/* Create set sum */
	st_word(dirb + XDIR_SetSum, xdir_sum(dirb));
	nent = dirb[XDIR_NumSec] + 1;

	/* Store the direcotry entry block to the directory */
	res = dir_sdi(dp, dp->blk_ofs);
	while (res == FR_OK) {
		res = move_window(dp->obj.fs, dp->sect);
		if (res != FR_OK) break;
		memcpy(dp->dir, dirb, SZDIRE);
		dp->obj.fs->wflag = 1;
		if (--nent == 0) break;
		dirb += SZDIRE;
		res = dir_next(dp, 0);
	}
	return (res == FR_OK || res == FR_DISK_ERR) ? res : FR_INT_ERR;
}




#endif	/* !FF_FS_READONLY */
#endif	/* FF_FS_EXFAT */



#if FF_FS_MINIMIZE <= 1 || FF_FS_RPATH >= 2 || FF_USE_LABEL || FF_FS_EXFAT

template<typename DISKIO_HW>
FRESULT Driver<DISKIO_HW>::dir_read (DIR* dp, int vol)
{
	FRESULT res = FR_NO_FILE;
	FATFS *fs = dp->obj.fs;
	BYTE attr, b;
#if FF_USE_LFN
	BYTE ord = 0xFF, sum = 0xFF;
#endif

	while (dp->sect) {
		res = move_window(fs, dp->sect);
		if (res != FR_OK) break;
		b = dp->dir[DIR_Name];	/* Test for the entry type */
		if (b == 0) {
			res = FR_NO_FILE; break; /* Reached to end of the directory */
		}
#if FF_FS_EXFAT
		if (fs->fs_type == FS_EXFAT) {	/* On the exFAT volume */
			if (FF_USE_LABEL && vol) {
				if (b == ET_VLABEL) break;	/* Volume label entry? */
			} else {
				if (b == ET_FILEDIR) {		/* Start of the file entry block? */
					dp->blk_ofs = dp->dptr;	/* Get location of the block */
					res = load_xdir(dp);	/* Load the entry block */
					if (res == FR_OK) {
						dp->obj.attr = fs->dirbuf[XDIR_Attr] & AM_MASK;	/* Get attribute */
					}
					break;
				}
			}
		} else
#endif
		{	/* On the FAT/FAT32 volume */
			dp->obj.attr = attr = dp->dir[DIR_Attr] & AM_MASK;	/* Get attribute */
#if FF_USE_LFN		/* LFN configuration */
			if (b == DDEM || b == '.' || (int)((attr & ~AM_ARC) == AM_VOL) != vol) {	/* An entry without valid data */
				ord = 0xFF;
			} else {
				if (attr == AM_LFN) {	/* An LFN entry is found */
					if (b & LLEF) {		/* Is it start of an LFN sequence? */
						sum = dp->dir[LDIR_Chksum];
						b &= (BYTE)~LLEF; ord = b;
						dp->blk_ofs = dp->dptr;
					}
					/* Check LFN validity and capture it */
					ord = (b == ord && sum == dp->dir[LDIR_Chksum] && pick_lfn(fs->lfnbuf, dp->dir)) ? ord - 1 : 0xFF;
				} else {				/* An SFN entry is found */
					if (ord != 0 || sum != sum_sfn(dp->dir)) {	/* Is there a valid LFN? */
						dp->blk_ofs = 0xFFFFFFFF;	/* It has no LFN. */
					}
					break;
				}
			}
#else		/* Non LFN configuration */
			if (b != DDEM && b != '.' && attr != AM_LFN && (int)((attr & ~AM_ARC) == AM_VOL) == vol) {	/* Is it a valid entry? */
				break;
			}
#endif
		}
		res = dir_next(dp, 0);		/* Next entry */
		if (res != FR_OK) break;
	}

	if (res != FR_OK) dp->sect = 0;		/* Terminate the read operation on error or EOT */
	return res;
}

#endif	/* FF_FS_MINIMIZE <= 1 || FF_USE_LABEL || FF_FS_RPATH >= 2 */


template<typename DISKIO_HW>
FRESULT Driver<DISKIO_HW>::dir_find (DIR* dp)
{
	FRESULT res;
	FATFS *fs = dp->obj.fs;
	BYTE c;
#if FF_USE_LFN
	BYTE a, ord, sum;
#endif

	res = dir_sdi(dp, 0);			/* Rewind directory object */
	if (res != FR_OK) return res;
#if FF_FS_EXFAT
	if (fs->fs_type == FS_EXFAT) {	/* On the exFAT volume */
		BYTE nc;
		UINT di, ni;
		WORD hash = xname_sum(fs->lfnbuf);		/* Hash value of the name to find */

		while ((res = dir_read(dp, static_cast<int>(Directory::READFILE))) == FR_OK) {	/* Read an item */
#if FF_MAX_LFN < 255
			if (fs->dirbuf[XDIR_NumName] > FF_MAX_LFN) continue;		/* Skip comparison if inaccessible object name */
#endif
			if (ld_word(fs->dirbuf + XDIR_NameHash) != hash) continue;	/* Skip comparison if hash mismatched */
			for (nc = fs->dirbuf[XDIR_NumName], di = SZDIRE * 2, ni = 0; nc; nc--, di += 2, ni++) {	/* Compare the name */
				if ((di % SZDIRE) == 0) di += 2;
				if (f_wtoupper(ld_word(fs->dirbuf + di)) != f_wtoupper(fs->lfnbuf[ni])) break;
			}
			if (nc == 0 && !fs->lfnbuf[ni]) break;	/* Name matched? */
		}
		return res;
	}
#endif
	/* On the FAT/FAT32 volume */
#if FF_USE_LFN
	ord = sum = 0xFF; dp->blk_ofs = 0xFFFFFFFF;	/* Reset LFN sequence */
#endif
	do {
		res = move_window(fs, dp->sect);
		if (res != FR_OK) break;
		c = dp->dir[DIR_Name];
		if (c == 0) { res = FR_NO_FILE; break; }	/* Reached to end of table */
#if FF_USE_LFN		/* LFN configuration */
		dp->obj.attr = a = dp->dir[DIR_Attr] & AM_MASK;
		if (c == DDEM || ((a & AM_VOL) && a != AM_LFN)) {	/* An entry without valid data */
			ord = 0xFF; dp->blk_ofs = 0xFFFFFFFF;	/* Reset LFN sequence */
		} else {
			if (a == AM_LFN) {			/* An LFN entry is found */
				if (!(dp->fn[NSFLAG] & NS_NOLFN)) {
					if (c & LLEF) {		/* Is it start of LFN sequence? */
						sum = dp->dir[LDIR_Chksum];
						c &= (BYTE)~LLEF; ord = c;	/* LFN start order */
						dp->blk_ofs = dp->dptr;	/* Start offset of LFN */
					}
					/* Check validity of the LFN entry and compare it with given name */
					ord = (c == ord && sum == dp->dir[LDIR_Chksum] && cmp_lfn(fs->lfnbuf, dp->dir)) ? ord - 1 : 0xFF;
				}
			} else {					/* An SFN entry is found */
				if (ord == 0 && sum == sum_sfn(dp->dir)) break;	/* LFN matched? */
				if (!(dp->fn[NSFLAG] & NS_LOSS) && !memcmp(dp->dir, dp->fn, 11)) break;	/* SFN matched? */
				ord = 0xFF; dp->blk_ofs = 0xFFFFFFFF;	/* Reset LFN sequence */
			}
		}
#else		/* Non LFN configuration */
		dp->obj.attr = dp->dir[DIR_Attr] & AM_MASK;
		if (!(dp->dir[DIR_Attr] & AM_VOL) && !memcmp(dp->dir, dp->fn, 11)) break;	/* Is it a valid entry? */
#endif
		res = dir_next(dp, 0);	/* Next entry */
	} while (res == FR_OK);

	return res;
}




#if !FF_FS_READONLY

template<typename DISKIO_HW>
FRESULT Driver<DISKIO_HW>::dir_register (DIR* dp)
{
	FRESULT res;
	FATFS *fs = dp->obj.fs;
#if FF_USE_LFN		/* LFN configuration */
	UINT n, len, n_ent;
	BYTE sn[12], sum;


	if (dp->fn[NSFLAG] & (NS_DOT | NS_NONAME)) return FR_INVALID_NAME;	/* Check name validity */
	for (len = 0; fs->lfnbuf[len]; len++) ;	/* Get lfn length */

#if FF_FS_EXFAT
	if (fs->fs_type == FS_EXFAT) {	/* On the exFAT volume */
		n_ent = (len + 14) / 15 + 2;	/* Number of entries to allocate (85+C0+C1s) */
		res = dir_alloc(dp, n_ent);		/* Allocate directory entries */
		if (res != FR_OK) return res;
		dp->blk_ofs = dp->dptr - SZDIRE * (n_ent - 1);	/* Set the allocated entry block offset */

		if (dp->obj.stat & 4) {			/* Has the directory been stretched by new allocation? */
			dp->obj.stat &= ~4;
			res = fill_first_frag(&dp->obj);	/* Fill the first fragment on the FAT if needed */
			if (res != FR_OK) return res;
			res = fill_last_frag(&dp->obj, dp->clust, 0xFFFFFFFF);	/* Fill the last fragment on the FAT if needed */
			if (res != FR_OK) return res;
			if (dp->obj.sclust != 0) {		/* Is it a sub-directory? */
				DIR dj;

				res = load_obj_xdir(&dj, &dp->obj);	/* Load the object status */
				if (res != FR_OK) return res;
				dp->obj.objsize += (DWORD)fs->csize * SS(fs);		/* Increase the directory size by cluster size */
				st_qword(fs->dirbuf + XDIR_FileSize, dp->obj.objsize);
				st_qword(fs->dirbuf + XDIR_ValidFileSize, dp->obj.objsize);
				fs->dirbuf[XDIR_GenFlags] = dp->obj.stat | 1;		/* Update the allocation status */
				res = store_xdir(&dj);				/* Store the object status */
				if (res != FR_OK) return res;
			}
		}

		create_xdir(fs->dirbuf, fs->lfnbuf);	/* Create on-memory directory block to be written later */
		return FR_OK;
	}
#endif
	/* On the FAT/FAT32 volume */
	memcpy(sn, dp->fn, 12);
	if (sn[NSFLAG] & NS_LOSS) {			/* When LFN is out of 8.3 format, generate a numbered name */
		dp->fn[NSFLAG] = NS_NOLFN;		/* Find only SFN */
		for (n = 1; n < 100; n++) {
			gen_numname(dp->fn, sn, fs->lfnbuf, n);	/* Generate a numbered name */
			res = dir_find(dp);				/* Check if the name collides with existing SFN */
			if (res != FR_OK) break;
		}
		if (n == 100) return FR_DENIED;		/* Abort if too many collisions */
		if (res != FR_NO_FILE) return res;	/* Abort if the result is other than 'not collided' */
		dp->fn[NSFLAG] = sn[NSFLAG];
	}

	/* Create an SFN with/without LFNs. */
	n_ent = (sn[NSFLAG] & NS_LFN) ? (len + 12) / 13 + 1 : 1;	/* Number of entries to allocate */
	res = dir_alloc(dp, n_ent);		/* Allocate entries */
	if (res == FR_OK && --n_ent) {	/* Set LFN entry if needed */
		res = dir_sdi(dp, dp->dptr - n_ent * SZDIRE);
		if (res == FR_OK) {
			sum = sum_sfn(dp->fn);	/* Checksum value of the SFN tied to the LFN */
			do {					/* Store LFN entries in bottom first */
				res = move_window(fs, dp->sect);
				if (res != FR_OK) break;
				put_lfn(fs->lfnbuf, dp->dir, (BYTE)n_ent, sum);
				fs->wflag = 1;
				res = dir_next(dp, 0);	/* Next entry */
			} while (res == FR_OK && --n_ent);
		}
	}

#else	/* Non LFN configuration */
	res = dir_alloc(dp, 1);		/* Allocate an entry for SFN */

#endif

	/* Set SFN entry */
	if (res == FR_OK) {
		res = move_window(fs, dp->sect);
		if (res == FR_OK) {
			std::memset(dp->dir, 0, SZDIRE);	/* Clean the entry */
			memcpy(dp->dir + DIR_Name, dp->fn, 11);	/* Put SFN */
#if FF_USE_LFN
			dp->dir[DIR_NTres] = dp->fn[NSFLAG] & (NS_BODY | NS_EXT);	/* Put NT flag */
#endif
			fs->wflag = 1;
		}
	}

	return res;
}

#endif /* !FF_FS_READONLY */



#if !FF_FS_READONLY && FF_FS_MINIMIZE == 0

template<typename DISKIO_HW>
FRESULT Driver<DISKIO_HW>::dir_remove (DIR* dp)
{
	FRESULT res;
	FATFS *fs = dp->obj.fs;
#if FF_USE_LFN		/* LFN configuration */
	DWORD last = dp->dptr;

	res = (dp->blk_ofs == 0xFFFFFFFF) ? FR_OK : dir_sdi(dp, dp->blk_ofs);	/* Goto top of the entry block if LFN is exist */
	if (res == FR_OK) {
		do {
			res = move_window(fs, dp->sect);
			if (res != FR_OK) break;
			if (FF_FS_EXFAT && fs->fs_type == FS_EXFAT) {	/* On the exFAT volume */
				dp->dir[XDIR_Type] &= 0x7F;	/* Clear the entry InUse flag. */
			} else {										/* On the FAT/FAT32 volume */
				dp->dir[DIR_Name] = DDEM;	/* Mark the entry 'deleted'. */
			}
			fs->wflag = 1;
			if (dp->dptr >= last) break;	/* If reached last entry then all entries of the object has been deleted. */
			res = dir_next(dp, 0);	/* Next entry */
		} while (res == FR_OK);
		if (res == FR_NO_FILE) res = FR_INT_ERR;
	}
#else			/* Non LFN configuration */

	res = move_window(fs, dp->sect);
	if (res == FR_OK) {
		dp->dir[DIR_Name] = DDEM;	/* Mark the entry 'deleted'.*/
		fs->wflag = 1;
	}
#endif

	return res;
}

#endif /* !FF_FS_READONLY && FF_FS_MINIMIZE == 0 */

template<typename DISKIO_HW>
FRESULT Driver<DISKIO_HW>::follow_path (DIR* dp, const TCHAR* path)
{
	FRESULT res;
	BYTE ns;
	FATFS *fs = dp->obj.fs;


#if FF_FS_RPATH != 0
	if (!IsSeparator(*path) && (FF_STR_VOLUME_ID != 2 || !IsTerminator(*path))) {	/* Without heading separator */
		dp->obj.sclust = fs->cdir;			/* Start at the current directory */
	} else
#endif
	{										/* With heading separator */
		while (IsSeparator(*path)) path++;	/* Strip separators */
		dp->obj.sclust = 0;					/* Start from the root directory */
	}
#if FF_FS_EXFAT
	dp->obj.n_frag = 0;	/* Invalidate last fragment counter of the object */
#if FF_FS_RPATH != 0
	if (fs->fs_type == FS_EXFAT && dp->obj.sclust) {	/* exFAT: Retrieve the sub-directory's status */
		DIR dj;

		dp->obj.c_scl = fs->cdc_scl;
		dp->obj.c_size = fs->cdc_size;
		dp->obj.c_ofs = fs->cdc_ofs;
		res = load_obj_xdir(&dj, &dp->obj);
		if (res != FR_OK) return res;
		dp->obj.objsize = ld_dword(fs->dirbuf + XDIR_FileSize);
		dp->obj.stat = fs->dirbuf[XDIR_GenFlags] & 2;
	}
#endif
#endif

	if ((UINT)*path < ' ') {				/* Null path name is the origin directory itself */
		dp->fn[NSFLAG] = NS_NONAME;
		res = dir_sdi(dp, 0);

	} else {								/* Follow path */
		for (;;) {
			res = create_name(dp, &path);	/* Get a segment name of the path */
			if (res != FR_OK) break;
			res = dir_find(dp);				/* Find an object with the segment name */
			ns = dp->fn[NSFLAG];
			if (res != FR_OK) {				/* Failed to find the object */
				if (res == FR_NO_FILE) {	/* Object is not found */
					if (FF_FS_RPATH && (ns & NS_DOT)) {	/* If dot entry is not exist, stay there */
						if (!(ns & NS_LAST)) continue;	/* Continue to follow if not last segment */
						dp->fn[NSFLAG] = NS_NONAME;
						res = FR_OK;
					} else {							/* Could not find the object */
						if (!(ns & NS_LAST)) res = FR_NO_PATH;	/* Adjust error code if not last segment */
					}
				}
				break;
			}
			if (ns & NS_LAST) break;		/* Last segment matched. Function completed. */
			/* Get into the sub-directory */
			if (!(dp->obj.attr & AM_DIR)) {	/* It is not a sub-directory and cannot follow */
				res = FR_NO_PATH; break;
			}
#if FF_FS_EXFAT
			if (fs->fs_type == FS_EXFAT) {	/* Save containing directory information for next dir */
				dp->obj.c_scl = dp->obj.sclust;
				dp->obj.c_size = ((DWORD)dp->obj.objsize & 0xFFFFFF00) | dp->obj.stat;
				dp->obj.c_ofs = dp->blk_ofs;
				init_alloc_info(fs, &dp->obj);	/* Open next directory */
			} else
#endif
			{
				dp->obj.sclust = ld_clust(fs, fs->win + dp->dptr % SS(fs));	/* Open next directory */
			}
		}
	}

	return res;
}



} // namespace fatfs



#endif // __FF_DRIVER_HIDDEN_IMPL_HPP__