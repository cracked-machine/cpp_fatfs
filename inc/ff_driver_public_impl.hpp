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


/// @brief The implementations of the private templated functions from fatfs::Driver

#ifndef __FF_DRIVER_PUBLIC_IMPL_HPP__
#define __FF_DRIVER_PUBLIC_IMPL_HPP__

namespace fatfs
{


/*---------------------------------------------------------------------------

   Public Functions (FatFs API)

----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Mount/Unmount a Logical Drive                                         */
/*-----------------------------------------------------------------------*/
template<typename IOTYPE>
FRESULT Driver<IOTYPE>::f_mount (
	FATFS* fs,			/* Pointer to the filesystem object to be registered (NULL:unmount)*/
	const TCHAR* path,	/* Logical drive number to be mounted/unmounted */
	BYTE opt			/* Mount option: 0=Do not mount (delayed mount), 1=Mount immediately */
)
{
	FATFS *cfs;
	int vol;
	FRESULT res;
	const TCHAR *rp = path;


	/* Get logical drive number */
	vol = get_ldnumber(&rp);
	if (vol < 0) return FR_INVALID_DRIVE;
	cfs = FatFs[vol];					/* Pointer to fs object */

	if (cfs) 
	{
		#if FF_FS_LOCK != 0
				clear_lock(cfs);
		#endif

		// Discard sync object of the current volume 
		#if FF_FS_REENTRANT	
				if (!ff_del_syncobj(cfs->sobj)) return FR_INT_ERR;
		#endif
		// Clear old fs object
		cfs->fs_type = 0;				
	}

	if (fs) 
	{
		// Clear new fs object 
		fs->fs_type = 0;				
		// Create sync object for the new volume 
		#if FF_FS_REENTRANT						
				if (!ff_cre_syncobj((BYTE)vol, &fs->sobj)) return FR_INT_ERR;
		#endif
	}
	// Register new fs object 
	FatFs[vol] = fs;					

	// Do not mount now, it will be mounted later
	if (opt == 0) return FR_OK;			

	// Force mount the volume, without write-protection
	res = mount_volume(&path, &fs, 0);	

	LEAVE_FF(fs, res);
}




/*-----------------------------------------------------------------------*/
/* Open or Create a File                                                 */
/*-----------------------------------------------------------------------*/
template<typename IOTYPE>
FRESULT Driver<IOTYPE>::f_open (
	FIL* fp,			/* Pointer to the blank file object */
	const TCHAR* path,	/* Pointer to the file name */
	BYTE mode			/* Access mode and open mode flags */
)
{
	FRESULT res;
	DIR dj;
	FATFS *fs;
	#if !FF_FS_READONLY
		DWORD cl, bcs, clst, tm;
		LBA_t sc;
		FSIZE_t ofs;
	#endif
	DEF_NAMBUF


	if (!fp) return FR_INVALID_OBJECT;

	/* Get logical drive number */
	mode &= FF_FS_READONLY ? FA_READ : FA_READ | FA_WRITE | FA_CREATE_ALWAYS | FA_CREATE_NEW | FA_OPEN_ALWAYS | FA_OPEN_APPEND;
	res = mount_volume(&path, &fs, mode);
	if (res == FR_OK) {
		dj.obj.fs = fs;
		INIT_NAMBUF(fs);
		res = follow_path(&dj, path);	/* Follow the file path */
#if !FF_FS_READONLY	/* Read/Write configuration */
		if (res == FR_OK) {
			if (dj.fn[NSFLAG] & NS_NONAME) {	/* Origin directory itself? */
				res = FR_INVALID_NAME;
			}
#if FF_FS_LOCK != 0
			else {
				res = chk_lock(&dj, (mode & ~FA_READ) ? 1 : 0);		/* Check if the file can be used */
			}
#endif
		}
		/* Create or Open a file */
		if (mode & (FA_CREATE_ALWAYS | FA_OPEN_ALWAYS | FA_CREATE_NEW)) {
			if (res != FR_OK) {					/* No file, create new */
				if (res == FR_NO_FILE) {		/* There is no file to open, create a new entry */
#if FF_FS_LOCK != 0
					res = enq_lock() ? dir_register(&dj) : FR_TOO_MANY_OPEN_FILES;
#else
					res = dir_register(&dj);
#endif
				}
				mode |= FA_CREATE_ALWAYS;		/* File is created */
			}
			else {								/* Any object with the same name is already existing */
				if (dj.obj.attr & (AM_RDO | AM_DIR)) {	/* Cannot overwrite it (R/O or DIR) */
					res = FR_DENIED;
				} else {
					if (mode & FA_CREATE_NEW) res = FR_EXIST;	/* Cannot create as new file */
				}
			}
			if (res == FR_OK && (mode & FA_CREATE_ALWAYS)) {	/* Truncate the file if overwrite mode */
#if FF_FS_EXFAT
				if (fs->fs_type == FS_EXFAT) {
					/* Get current allocation info */
					fp->obj.fs = fs;
					init_alloc_info(fs, &fp->obj);
					/* Set directory entry block initial state */
					std::memset(fs->dirbuf + 2, 0, 30);	/* Clear 85 entry except for NumSec */
					std::memset(fs->dirbuf + 38, 0, 26);	/* Clear C0 entry except for NumName and NameHash */
					fs->dirbuf[XDIR_Attr] = AM_ARC;
					st_dword(fs->dirbuf + XDIR_CrtTime, GET_FATTIME());
					fs->dirbuf[XDIR_GenFlags] = 1;
					res = store_xdir(&dj);
					if (res == FR_OK && fp->obj.sclust != 0) {	/* Remove the cluster chain if exist */
						res = remove_chain(&fp->obj, fp->obj.sclust, 0);
						fs->last_clst = fp->obj.sclust - 1;		/* Reuse the cluster hole */
					}
				} else
#endif
				{
					/* Set directory entry initial state */
					tm = GET_FATTIME();					/* Set created time */
					st_dword(dj.dir + DIR_CrtTime, tm);
					st_dword(dj.dir + DIR_ModTime, tm);
					cl = ld_clust(fs, dj.dir);			/* Get current cluster chain */
					dj.dir[DIR_Attr] = AM_ARC;			/* Reset attribute */
					st_clust(fs, dj.dir, 0);			/* Reset file allocation info */
					st_dword(dj.dir + DIR_FileSize, 0);
					fs->wflag = 1;
					if (cl != 0) {						/* Remove the cluster chain if exist */
						sc = fs->winsect;
						res = remove_chain(&dj.obj, cl, 0);
						if (res == FR_OK) {
							res = move_window(fs, sc);
							fs->last_clst = cl - 1;		/* Reuse the cluster hole */
						}
					}
				}
			}
		}
		else {	/* Open an existing file */
			if (res == FR_OK) {					/* Is the object exsiting? */
				if (dj.obj.attr & AM_DIR) {		/* File open against a directory */
					res = FR_NO_FILE;
				} else {
					if ((mode & FA_WRITE) && (dj.obj.attr & AM_RDO)) { /* Write mode open against R/O file */
						res = FR_DENIED;
					}
				}
			}
		}
		if (res == FR_OK) {
			if (mode & FA_CREATE_ALWAYS) mode |= FA_MODIFIED;	/* Set file change flag if created or overwritten */
			fp->dir_sect = fs->winsect;			/* Pointer to the directory entry */
			fp->dir_ptr = dj.dir;
#if FF_FS_LOCK != 0
			fp->obj.lockid = inc_lock(&dj, (mode & ~FA_READ) ? 1 : 0);	/* Lock the file for this session */
			if (fp->obj.lockid == 0) res = FR_INT_ERR;
#endif
		}
#else		/* R/O configuration */
		if (res == FR_OK) {
			if (dj.fn[NSFLAG] & NS_NONAME) {	/* Is it origin directory itself? */
				res = FR_INVALID_NAME;
			} else {
				if (dj.obj.attr & AM_DIR) {		/* Is it a directory? */
					res = FR_NO_FILE;
				}
			}
		}
#endif

		if (res == FR_OK) {
#if FF_FS_EXFAT
			if (fs->fs_type == FS_EXFAT) {
				fp->obj.c_scl = dj.obj.sclust;							/* Get containing directory info */
				fp->obj.c_size = ((DWORD)dj.obj.objsize & 0xFFFFFF00) | dj.obj.stat;
				fp->obj.c_ofs = dj.blk_ofs;
				init_alloc_info(fs, &fp->obj);
			} else
#endif
			{
				fp->obj.sclust = ld_clust(fs, dj.dir);					/* Get object allocation info */
				fp->obj.objsize = ld_dword(dj.dir + DIR_FileSize);
			}
#if FF_USE_FASTSEEK
			fp->cltbl = 0;		/* Disable fast seek mode */
#endif
			fp->obj.fs = fs;	/* Validate the file object */
			fp->obj.id = fs->id;
			fp->flag = mode;	/* Set file access mode */
			fp->err = 0;		/* Clear error flag */
			fp->sect = 0;		/* Invalidate current data sector */
			fp->fptr = 0;		/* Set file pointer top of the file */
#if !FF_FS_READONLY
#if !FF_FS_TINY
			std::memset(fp->buf, 0, sizeof fp->buf);	/* Clear sector buffer */
#endif
			if ((mode & FA_SEEKEND) && fp->obj.objsize > 0) {	/* Seek to end of file if FA_OPEN_APPEND is specified */
				fp->fptr = fp->obj.objsize;			/* Offset to seek */
				bcs = (DWORD)fs->csize * SS(fs);	/* Cluster size in byte */
				clst = fp->obj.sclust;				/* Follow the cluster chain */
				for (ofs = fp->obj.objsize; res == FR_OK && ofs > bcs; ofs -= bcs) {
					clst = get_fat(&fp->obj, clst);
					if (clst <= 1) res = FR_INT_ERR;
					if (clst == 0xFFFFFFFF) res = FR_DISK_ERR;
				}
				fp->clust = clst;
				if (res == FR_OK && ofs % SS(fs)) {	/* Fill sector buffer if not on the sector boundary */
					sc = clst2sect(fs, clst);
					if (sc == 0) {
						res = FR_INT_ERR;
					} else {
						fp->sect = sc + (DWORD)(ofs / SS(fs));
#if !FF_FS_TINY
						if (m_diskio->read(fs->pdrv, fp->buf, fp->sect, 1) != DiskioHardwareBase::DRESULT::RES_OK) res = FR_DISK_ERR;
#endif
					}
				}
#if FF_FS_LOCK != 0
				if (res != FR_OK) dec_lock(fp->obj.lockid); /* Decrement file open counter if seek failed */
#endif
			}
#endif
		}

		FREE_NAMBUF();
	}

	if (res != FR_OK) fp->obj.fs = 0;	/* Invalidate file object on error */

	LEAVE_FF(fs, res);
}




/*-----------------------------------------------------------------------*/
/* Read File                                                             */
/*-----------------------------------------------------------------------*/
template<typename IOTYPE>
FRESULT Driver<IOTYPE>::f_read (
	FIL* fp, 	/* Open file to be read */
	void* buff,	/* Data buffer to store the read data */
	UINT btr,	/* Number of bytes to read */
	UINT* br	/* Number of bytes read */
)
{
	FRESULT res;
	FATFS *fs;
	DWORD clst;
	LBA_t sect;
	FSIZE_t remain;
	UINT rcnt, cc, csect;
	BYTE *rbuff = (BYTE*)buff;


	*br = 0;	/* Clear read byte counter */
	res = validate(&fp->obj, &fs);				/* Check validity of the file object */
	if (res != FR_OK || (res = (FRESULT)fp->err) != FR_OK) LEAVE_FF(fs, res);	/* Check validity */
	if (!(fp->flag & FA_READ)) LEAVE_FF(fs, FR_DENIED); /* Check access mode */
	remain = fp->obj.objsize - fp->fptr;
	if (btr > remain) btr = (UINT)remain;		/* Truncate btr by remaining bytes */

	for ( ; btr > 0; btr -= rcnt, *br += rcnt, rbuff += rcnt, fp->fptr += rcnt) {	/* Repeat until btr bytes read */
		if (fp->fptr % SS(fs) == 0) {			/* On the sector boundary? */
			csect = (UINT)(fp->fptr / SS(fs) & (fs->csize - 1));	/* Sector offset in the cluster */
			if (csect == 0) {					/* On the cluster boundary? */
				if (fp->fptr == 0) {			/* On the top of the file? */
					clst = fp->obj.sclust;		/* Follow cluster chain from the origin */
				} else {						/* Middle or end of the file */
#if FF_USE_FASTSEEK
					if (fp->cltbl) {
						clst = clmt_clust(fp, fp->fptr);	/* Get cluster# from the CLMT */
					} else
#endif
					{
						clst = get_fat(&fp->obj, fp->clust);	/* Follow cluster chain on the FAT */
					}
				}
				if (clst < 2) ABORT(fs, FR_INT_ERR);
				if (clst == 0xFFFFFFFF) ABORT(fs, FR_DISK_ERR);
				fp->clust = clst;				/* Update current cluster */
			}
			sect = clst2sect(fs, fp->clust);	/* Get current sector */
			if (sect == 0) ABORT(fs, FR_INT_ERR);
			sect += csect;
			cc = btr / SS(fs);					/* When remaining bytes >= sector size, */
			if (cc > 0) {						/* Read maximum contiguous sectors directly */
				if (csect + cc > fs->csize) {	/* Clip at cluster boundary */
					cc = fs->csize - csect;
				}
				if (m_diskio->read(fs->pdrv, rbuff, sect, cc) != DiskioHardwareBase::DRESULT::RES_OK) ABORT(fs, FR_DISK_ERR);
#if !FF_FS_READONLY && FF_FS_MINIMIZE <= 2		/* Replace one of the read sectors with cached data if it contains a dirty sector */
#if FF_FS_TINY
				if (fs->wflag && fs->winsect - sect < cc) {
					memcpy(rbuff + ((fs->winsect - sect) * SS(fs)), fs->win, SS(fs));
				}
#else
				if ((fp->flag & FA_DIRTY) && fp->sect - sect < cc) {
					memcpy(rbuff + ((fp->sect - sect) * SS(fs)), fp->buf, SS(fs));
				}
#endif
#endif
				rcnt = SS(fs) * cc;				/* Number of bytes transferred */
				continue;
			}
#if !FF_FS_TINY
			if (fp->sect != sect) {			/* Load data sector if not in cache */
#if !FF_FS_READONLY
				if (fp->flag & FA_DIRTY) {		/* Write-back dirty sector cache */
					if (m_diskio->write(fs->pdrv, fp->buf, fp->sect, 1) != DiskioHardwareBase::DRESULT::RES_OK) ABORT(fs, FR_DISK_ERR);
					fp->flag &= (BYTE)~FA_DIRTY;
				}
#endif
				if (m_diskio->read(fs->pdrv, fp->buf, sect, 1) != DiskioHardwareBase::DRESULT::RES_OK)	ABORT(fs, FR_DISK_ERR);	/* Fill sector cache */
			}
#endif
			fp->sect = sect;
		}
		rcnt = SS(fs) - (UINT)fp->fptr % SS(fs);	/* Number of bytes remains in the sector */
		if (rcnt > btr) rcnt = btr;					/* Clip it by btr if needed */
#if FF_FS_TINY
		if (move_window(fs, fp->sect) != FR_OK) ABORT(fs, FR_DISK_ERR);	/* Move sector window */
		memcpy(rbuff, fs->win + fp->fptr % SS(fs), rcnt);	/* Extract partial sector */
#else
		memcpy(rbuff, fp->buf + fp->fptr % SS(fs), rcnt);	/* Extract partial sector */
#endif
	}

	LEAVE_FF(fs, FR_OK);
}




#if !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Write File                                                            */
/*-----------------------------------------------------------------------*/
template<typename IOTYPE>
FRESULT Driver<IOTYPE>::f_write (
	FIL* fp,			/* Open file to be written */
	const void* buff,	/* Data to be written */
	UINT btw,			/* Number of bytes to write */
	UINT* bw			/* Number of bytes written */
)
{
	FRESULT res;
	FATFS *fs;
	DWORD clst;
	LBA_t sect;
	UINT wcnt, cc, csect;
	const BYTE *wbuff = (const BYTE*)buff;


	*bw = 0;	/* Clear write byte counter */
	res = validate(&fp->obj, &fs);			/* Check validity of the file object */
	if (res != FR_OK || (res = (FRESULT)fp->err) != FR_OK) LEAVE_FF(fs, res);	/* Check validity */
	if (!(fp->flag & FA_WRITE)) LEAVE_FF(fs, FR_DENIED);	/* Check access mode */

	/* Check fptr wrap-around (file size cannot reach 4 GiB at FAT volume) */
	if ((!FF_FS_EXFAT || fs->fs_type != FS_EXFAT) && (DWORD)(fp->fptr + btw) < (DWORD)fp->fptr) {
		btw = (UINT)(0xFFFFFFFF - (DWORD)fp->fptr);
	}

	for ( ; btw > 0; btw -= wcnt, *bw += wcnt, wbuff += wcnt, fp->fptr += wcnt, fp->obj.objsize = (fp->fptr > fp->obj.objsize) ? fp->fptr : fp->obj.objsize) {	/* Repeat until all data written */
		if (fp->fptr % SS(fs) == 0) {		/* On the sector boundary? */
			csect = (UINT)(fp->fptr / SS(fs)) & (fs->csize - 1);	/* Sector offset in the cluster */
			if (csect == 0) {				/* On the cluster boundary? */
				if (fp->fptr == 0) {		/* On the top of the file? */
					clst = fp->obj.sclust;	/* Follow from the origin */
					if (clst == 0) {		/* If no cluster is allocated, */
						clst = create_chain(&fp->obj, 0);	/* create a new cluster chain */
					}
				} else {					/* On the middle or end of the file */
#if FF_USE_FASTSEEK
					if (fp->cltbl) {
						clst = clmt_clust(fp, fp->fptr);	/* Get cluster# from the CLMT */
					} else
#endif
					{
						clst = create_chain(&fp->obj, fp->clust);	/* Follow or stretch cluster chain on the FAT */
					}
				}
				if (clst == 0) break;		/* Could not allocate a new cluster (disk full) */
				if (clst == 1) ABORT(fs, FR_INT_ERR);
				if (clst == 0xFFFFFFFF) ABORT(fs, FR_DISK_ERR);
				fp->clust = clst;			/* Update current cluster */
				if (fp->obj.sclust == 0) fp->obj.sclust = clst;	/* Set start cluster if the first write */
			}
#if FF_FS_TINY
			if (fs->winsect == fp->sect && sync_window(fs) != FR_OK) ABORT(fs, FR_DISK_ERR);	/* Write-back sector cache */
#else
			if (fp->flag & FA_DIRTY) {		/* Write-back sector cache */
				if (m_diskio->write(fs->pdrv, fp->buf, fp->sect, 1) != DiskioHardwareBase::DRESULT::RES_OK) ABORT(fs, FR_DISK_ERR);
				fp->flag &= (BYTE)~FA_DIRTY;
			}
#endif
			sect = clst2sect(fs, fp->clust);	/* Get current sector */
			if (sect == 0) ABORT(fs, FR_INT_ERR);
			sect += csect;
			cc = btw / SS(fs);				/* When remaining bytes >= sector size, */
			if (cc > 0) {					/* Write maximum contiguous sectors directly */
				if (csect + cc > fs->csize) {	/* Clip at cluster boundary */
					cc = fs->csize - csect;
				}
				if (m_diskio->write(fs->pdrv, wbuff, sect, cc) != DiskioHardwareBase::DRESULT::RES_OK) ABORT(fs, FR_DISK_ERR);
#if FF_FS_MINIMIZE <= 2
#if FF_FS_TINY
				if (fs->winsect - sect < cc) {	/* Refill sector cache if it gets invalidated by the direct write */
					memcpy(fs->win, wbuff + ((fs->winsect - sect) * SS(fs)), SS(fs));
					fs->wflag = 0;
				}
#else
				if (fp->sect - sect < cc) { /* Refill sector cache if it gets invalidated by the direct write */
					memcpy(fp->buf, wbuff + ((fp->sect - sect) * SS(fs)), SS(fs));
					fp->flag &= (BYTE)~FA_DIRTY;
				}
#endif
#endif
				wcnt = SS(fs) * cc;		/* Number of bytes transferred */
				continue;
			}
#if FF_FS_TINY
			if (fp->fptr >= fp->obj.objsize) {	/* Avoid silly cache filling on the growing edge */
				if (sync_window(fs) != FR_OK) ABORT(fs, FR_DISK_ERR);
				fs->winsect = sect;
			}
#else
			if (fp->sect != sect && 		/* Fill sector cache with file data */
				fp->fptr < fp->obj.objsize &&
				m_diskio->read(fs->pdrv, fp->buf, sect, 1) != DiskioHardwareBase::DRESULT::RES_OK) {
					ABORT(fs, FR_DISK_ERR);
			}
#endif
			fp->sect = sect;
		}
		wcnt = SS(fs) - (UINT)fp->fptr % SS(fs);	/* Number of bytes remains in the sector */
		if (wcnt > btw) wcnt = btw;					/* Clip it by btw if needed */
#if FF_FS_TINY
		if (move_window(fs, fp->sect) != FR_OK) ABORT(fs, FR_DISK_ERR);	/* Move sector window */
		memcpy(fs->win + fp->fptr % SS(fs), wbuff, wcnt);	/* Fit data to the sector */
		fs->wflag = 1;
#else
		memcpy(fp->buf + fp->fptr % SS(fs), wbuff, wcnt);	/* Fit data to the sector */
		fp->flag |= FA_DIRTY;
#endif
	}

	fp->flag |= FA_MODIFIED;				/* Set file change flag */

	LEAVE_FF(fs, FR_OK);
}




/*-----------------------------------------------------------------------*/
/* Synchronize the File                                                  */
/*-----------------------------------------------------------------------*/
template<typename IOTYPE>
FRESULT Driver<IOTYPE>::f_sync (
	FIL* fp		/* Open file to be synced */
)
{
	FRESULT res;
	FATFS *fs;
	DWORD tm;
	BYTE *dir;


	res = validate(&fp->obj, &fs);	/* Check validity of the file object */
	if (res == FR_OK) {
		if (fp->flag & FA_MODIFIED) {	/* Is there any change to the file? */
#if !FF_FS_TINY
			if (fp->flag & FA_DIRTY) {	/* Write-back cached data if needed */
				if (m_diskio->write(fs->pdrv, fp->buf, fp->sect, 1) != DiskioHardwareBase::DRESULT::RES_OK) LEAVE_FF(fs, FR_DISK_ERR);
				fp->flag &= (BYTE)~FA_DIRTY;
			}
#endif
			/* Update the directory entry */
			tm = GET_FATTIME();				/* Modified time */
#if FF_FS_EXFAT
			if (fs->fs_type == FS_EXFAT) {
				res = fill_first_frag(&fp->obj);	/* Fill first fragment on the FAT if needed */
				if (res == FR_OK) {
					res = fill_last_frag(&fp->obj, fp->clust, 0xFFFFFFFF);	/* Fill last fragment on the FAT if needed */
				}
				if (res == FR_OK) {
					DIR dj;
					DEF_NAMBUF

					INIT_NAMBUF(fs);
					res = load_obj_xdir(&dj, &fp->obj);	/* Load directory entry block */
					if (res == FR_OK) {
						fs->dirbuf[XDIR_Attr] |= AM_ARC;				/* Set archive attribute to indicate that the file has been changed */
						fs->dirbuf[XDIR_GenFlags] = fp->obj.stat | 1;	/* Update file allocation information */
						st_dword(fs->dirbuf + XDIR_FstClus, fp->obj.sclust);		/* Update start cluster */
						st_qword(fs->dirbuf + XDIR_FileSize, fp->obj.objsize);		/* Update file size */
						st_qword(fs->dirbuf + XDIR_ValidFileSize, fp->obj.objsize);	/* (FatFs does not support Valid File Size feature) */
						st_dword(fs->dirbuf + XDIR_ModTime, tm);		/* Update modified time */
						fs->dirbuf[XDIR_ModTime10] = 0;
						st_dword(fs->dirbuf + XDIR_AccTime, 0);
						res = store_xdir(&dj);	/* Restore it to the directory */
						if (res == FR_OK) {
							res = sync_fs(fs);
							fp->flag &= (BYTE)~FA_MODIFIED;
						}
					}
					FREE_NAMBUF();
				}
			} else
#endif
			{
				res = move_window(fs, fp->dir_sect);
				if (res == FR_OK) {
					dir = fp->dir_ptr;
					dir[DIR_Attr] |= AM_ARC;						/* Set archive attribute to indicate that the file has been changed */
					st_clust(fp->obj.fs, dir, fp->obj.sclust);		/* Update file allocation information  */
					st_dword(dir + DIR_FileSize, (DWORD)fp->obj.objsize);	/* Update file size */
					st_dword(dir + DIR_ModTime, tm);				/* Update modified time */
					st_word(dir + DIR_LstAccDate, 0);
					fs->wflag = 1;
					res = sync_fs(fs);					/* Restore it to the directory */
					fp->flag &= (BYTE)~FA_MODIFIED;
				}
			}
		}
	}

	LEAVE_FF(fs, res);
}

#endif /* !FF_FS_READONLY */




/*-----------------------------------------------------------------------*/
/* Close File                                                            */
/*-----------------------------------------------------------------------*/
template<typename IOTYPE>
FRESULT Driver<IOTYPE>::f_close (
	FIL* fp		/* Open file to be closed */
)
{
	FRESULT res;
	FATFS *fs;

#if !FF_FS_READONLY
	res = f_sync(fp);					/* Flush cached data */
	if (res == FR_OK)
#endif
	{
		res = validate(&fp->obj, &fs);	/* Lock volume */
		if (res == FR_OK) {
#if FF_FS_LOCK != 0
			res = dec_lock(fp->obj.lockid);		/* Decrement file open counter */
			if (res == FR_OK) fp->obj.fs = 0;	/* Invalidate file object */
#else
			fp->obj.fs = 0;	/* Invalidate file object */
#endif
#if FF_FS_REENTRANT
			unlock_fs(fs, FR_OK);		/* Unlock volume */
#endif
		}
	}
	return res;
}




#if FF_FS_RPATH >= 1
/*-----------------------------------------------------------------------*/
/* Change Current Directory or Current Drive, Get Current Directory      */
/*-----------------------------------------------------------------------*/
template<typename IOTYPE>
FRESULT Driver<IOTYPE>::f_chdrive (
	const TCHAR* path		/* Drive number to set */
)
{
	int vol;


	/* Get logical drive number */
	vol = get_ldnumber(&path);
	if (vol < 0) return FR_INVALID_DRIVE;
	CurrVol = (BYTE)vol;	/* Set it as current volume */

	return FR_OK;
}


template<typename IOTYPE>
FRESULT Driver<IOTYPE>::f_chdir (
	const TCHAR* path	/* Pointer to the directory path */
)
{
#if FF_STR_VOLUME_ID == 2
	UINT i;
#endif
	FRESULT res;
	DIR dj;
	FATFS *fs;
	DEF_NAMBUF


	/* Get logical drive */
	res = mount_volume(&path, &fs, 0);
	if (res == FR_OK) {
		dj.obj.fs = fs;
		INIT_NAMBUF(fs);
		res = follow_path(&dj, path);		/* Follow the path */
		if (res == FR_OK) {					/* Follow completed */
			if (dj.fn[NSFLAG] & NS_NONAME) {	/* Is it the start directory itself? */
				fs->cdir = dj.obj.sclust;
#if FF_FS_EXFAT
				if (fs->fs_type == FS_EXFAT) {
					fs->cdc_scl = dj.obj.c_scl;
					fs->cdc_size = dj.obj.c_size;
					fs->cdc_ofs = dj.obj.c_ofs;
				}
#endif
			} else {
				if (dj.obj.attr & AM_DIR) {	/* It is a sub-directory */
#if FF_FS_EXFAT
					if (fs->fs_type == FS_EXFAT) {
						fs->cdir = ld_dword(fs->dirbuf + XDIR_FstClus);		/* Sub-directory cluster */
						fs->cdc_scl = dj.obj.sclust;						/* Save containing directory information */
						fs->cdc_size = ((DWORD)dj.obj.objsize & 0xFFFFFF00) | dj.obj.stat;
						fs->cdc_ofs = dj.blk_ofs;
					} else
#endif
					{
						fs->cdir = ld_clust(fs, dj.dir);					/* Sub-directory cluster */
					}
				} else {
					res = FR_NO_PATH;		/* Reached but a file */
				}
			}
		}
		FREE_NAMBUF();
		if (res == FR_NO_FILE) res = FR_NO_PATH;
#if FF_STR_VOLUME_ID == 2	/* Also current drive is changed if in Unix style volume ID */
		if (res == FR_OK) {
			for (i = FF_VOLUMES - 1; i && fs != FatFs[i]; i--) ;	/* Set current drive */
			CurrVol = (BYTE)i;
		}
#endif
	}

	LEAVE_FF(fs, res);
}


#if FF_FS_RPATH >= 2
template<typename IOTYPE>
FRESULT Driver<IOTYPE>::f_getcwd (
	TCHAR* buff,	/* Pointer to the directory path */
	UINT len		/* Size of buff in unit of TCHAR */
)
{
	FRESULT res;
	DIR dj;
	FATFS *fs;
	UINT i, n;
	DWORD ccl;
	TCHAR *tp = buff;
#if FF_VOLUMES >= 2
	UINT vl;
#if FF_STR_VOLUME_ID
	const char *vp;
#endif
#endif
	FILINFO fno;
	DEF_NAMBUF


	/* Get logical drive */
	buff[0] = 0;	/* Set null string to get current volume */
	res = mount_volume((const TCHAR**)&buff, &fs, 0);	/* Get current volume */
	if (res == FR_OK) {
		dj.obj.fs = fs;
		INIT_NAMBUF(fs);

		/* Follow parent directories and create the path */
		i = len;			/* Bottom of buffer (directory stack base) */
		if (!FF_FS_EXFAT || fs->fs_type != FS_EXFAT) {	/* (Cannot do getcwd on exFAT and returns root path) */
			dj.obj.sclust = fs->cdir;				/* Start to follow upper directory from current directory */
			while ((ccl = dj.obj.sclust) != 0) {	/* Repeat while current directory is a sub-directory */
				res = dir_sdi(&dj, 1 * SZDIRE);	/* Get parent directory */
				if (res != FR_OK) break;
				res = move_window(fs, dj.sect);
				if (res != FR_OK) break;
				dj.obj.sclust = ld_clust(fs, dj.dir);	/* Goto parent directory */
				res = dir_sdi(&dj, 0);
				if (res != FR_OK) break;
				do {							/* Find the entry links to the child directory */
					res = dir_read(&dj, static_cast<int>(Directory::READFILE));
					if (res != FR_OK) break;
					if (ccl == ld_clust(fs, dj.dir)) break;	/* Found the entry */
					res = dir_next(&dj, 0);
				} while (res == FR_OK);
				if (res == FR_NO_FILE) res = FR_INT_ERR;/* It cannot be 'not found'. */
				if (res != FR_OK) break;
				get_fileinfo(&dj, &fno);		/* Get the directory name and push it to the buffer */
				for (n = 0; fno.fname[n]; n++) ;	/* Name length */
				if (i < n + 1) {	/* Insufficient space to store the path name? */
					res = FR_NOT_ENOUGH_CORE; break;
				}
				while (n) buff[--i] = fno.fname[--n];	/* Stack the name */
				buff[--i] = '/';
			}
		}
		if (res == FR_OK) {
			if (i == len) buff[--i] = '/';	/* Is it the root-directory? */
#if FF_VOLUMES >= 2			/* Put drive prefix */
			vl = 0;
#if FF_STR_VOLUME_ID >= 1	/* String volume ID */
			for (n = 0, vp = (const char*)VolumeStr[CurrVol]; vp[n]; n++) ;
			if (i >= n + 2) {
				if (FF_STR_VOLUME_ID == 2) *tp++ = (TCHAR)'/';
				for (vl = 0; vl < n; *tp++ = (TCHAR)vp[vl], vl++) ;
				if (FF_STR_VOLUME_ID == 1) *tp++ = (TCHAR)':';
				vl++;
			}
#else						/* Numeric volume ID */
			if (i >= 3) {
				*tp++ = (TCHAR)'0' + CurrVol;
				*tp++ = (TCHAR)':';
				vl = 2;
			}
#endif
			if (vl == 0) res = FR_NOT_ENOUGH_CORE;
#endif
			/* Add current directory path */
			if (res == FR_OK) {
				do *tp++ = buff[i++]; while (i < len);	/* Copy stacked path string */
			}
		}
		FREE_NAMBUF();
	}

	*tp = 0;
	LEAVE_FF(fs, res);
}

#endif /* FF_FS_RPATH >= 2 */
#endif /* FF_FS_RPATH >= 1 */



#if FF_FS_MINIMIZE <= 2
/*-----------------------------------------------------------------------*/
/* Seek File Read/Write Pointer                                          */
/*-----------------------------------------------------------------------*/
template<typename IOTYPE>
FRESULT Driver<IOTYPE>::f_lseek (
	FIL* fp,		/* Pointer to the file object */
	FSIZE_t ofs		/* File pointer from top of file */
)
{
	FRESULT res;
	FATFS *fs;
	DWORD clst, bcs;
	LBA_t nsect;
	FSIZE_t ifptr;
#if FF_USE_FASTSEEK
	DWORD cl, pcl, ncl, tcl, tlen, ulen;
	DWORD *tbl;
	LBA_t dsc;
#endif

	res = validate(&fp->obj, &fs);		/* Check validity of the file object */
	if (res == FR_OK) res = (FRESULT)fp->err;
#if FF_FS_EXFAT && !FF_FS_READONLY
	if (res == FR_OK && fs->fs_type == FS_EXFAT) {
		res = fill_last_frag(&fp->obj, fp->clust, 0xFFFFFFFF);	/* Fill last fragment on the FAT if needed */
	}
#endif
	if (res != FR_OK) LEAVE_FF(fs, res);

#if FF_USE_FASTSEEK
	if (fp->cltbl) {	/* Fast seek */
		if (ofs == CREATE_LINKMAP) {	/* Create CLMT */
			tbl = fp->cltbl;
			tlen = *tbl++; ulen = 2;	/* Given table size and required table size */
			cl = fp->obj.sclust;		/* Origin of the chain */
			if (cl != 0) {
				do {
					/* Get a fragment */
					tcl = cl; ncl = 0; ulen += 2;	/* Top, length and used items */
					do {
						pcl = cl; ncl++;
						cl = get_fat(&fp->obj, cl);
						if (cl <= 1) ABORT(fs, FR_INT_ERR);
						if (cl == 0xFFFFFFFF) ABORT(fs, FR_DISK_ERR);
					} while (cl == pcl + 1);
					if (ulen <= tlen) {		/* Store the length and top of the fragment */
						*tbl++ = ncl; *tbl++ = tcl;
					}
				} while (cl < fs->n_fatent);	/* Repeat until end of chain */
			}
			*fp->cltbl = ulen;	/* Number of items used */
			if (ulen <= tlen) {
				*tbl = 0;		/* Terminate table */
			} else {
				res = FR_NOT_ENOUGH_CORE;	/* Given table size is smaller than required */
			}
		} else {						/* Fast seek */
			if (ofs > fp->obj.objsize) ofs = fp->obj.objsize;	/* Clip offset at the file size */
			fp->fptr = ofs;				/* Set file pointer */
			if (ofs > 0) {
				fp->clust = clmt_clust(fp, ofs - 1);
				dsc = clst2sect(fs, fp->clust);
				if (dsc == 0) ABORT(fs, FR_INT_ERR);
				dsc += (DWORD)((ofs - 1) / SS(fs)) & (fs->csize - 1);
				if (fp->fptr % SS(fs) && dsc != fp->sect) {	/* Refill sector cache if needed */
#if !FF_FS_TINY
#if !FF_FS_READONLY
					if (fp->flag & FA_DIRTY) {		/* Write-back dirty sector cache */
						if (m_diskio->write(fs->pdrv, fp->buf, fp->sect, 1) != DiskioHardwareBase::DRESULT::RES_OK) ABORT(fs, FR_DISK_ERR);
						fp->flag &= (BYTE)~FA_DIRTY;
					}
#endif
					if (m_diskio->read(fs->pdrv, fp->buf, dsc, 1) != DiskioHardwareBase::DRESULT::RES_OK) ABORT(fs, FR_DISK_ERR);	/* Load current sector */
#endif
					fp->sect = dsc;
				}
			}
		}
	} else
#endif

	/* Normal Seek */
	{
#if FF_FS_EXFAT
		if (fs->fs_type != FS_EXFAT && ofs >= 0x100000000) ofs = 0xFFFFFFFF;	/* Clip at 4 GiB - 1 if at FATxx */
#endif
		if (ofs > fp->obj.objsize && (FF_FS_READONLY || !(fp->flag & FA_WRITE))) {	/* In read-only mode, clip offset with the file size */
			ofs = fp->obj.objsize;
		}
		ifptr = fp->fptr;
		fp->fptr = nsect = 0;
		if (ofs > 0) {
			bcs = (DWORD)fs->csize * SS(fs);	/* Cluster size (byte) */
			if (ifptr > 0 &&
				(ofs - 1) / bcs >= (ifptr - 1) / bcs) {	/* When seek to same or following cluster, */
				fp->fptr = (ifptr - 1) & ~(FSIZE_t)(bcs - 1);	/* start from the current cluster */
				ofs -= fp->fptr;
				clst = fp->clust;
			} else {									/* When seek to back cluster, */
				clst = fp->obj.sclust;					/* start from the first cluster */
#if !FF_FS_READONLY
				if (clst == 0) {						/* If no cluster chain, create a new chain */
					clst = create_chain(&fp->obj, 0);
					if (clst == 1) ABORT(fs, FR_INT_ERR);
					if (clst == 0xFFFFFFFF) ABORT(fs, FR_DISK_ERR);
					fp->obj.sclust = clst;
				}
#endif
				fp->clust = clst;
			}
			if (clst != 0) {
				while (ofs > bcs) {						/* Cluster following loop */
					ofs -= bcs; fp->fptr += bcs;
#if !FF_FS_READONLY
					if (fp->flag & FA_WRITE) {			/* Check if in write mode or not */
						if (FF_FS_EXFAT && fp->fptr > fp->obj.objsize) {	/* No FAT chain object needs correct objsize to generate FAT value */
							fp->obj.objsize = fp->fptr;
							fp->flag |= FA_MODIFIED;
						}
						clst = create_chain(&fp->obj, clst);	/* Follow chain with forceed stretch */
						if (clst == 0) {				/* Clip file size in case of disk full */
							ofs = 0; break;
						}
					} else
#endif
					{
						clst = get_fat(&fp->obj, clst);	/* Follow cluster chain if not in write mode */
					}
					if (clst == 0xFFFFFFFF) ABORT(fs, FR_DISK_ERR);
					if (clst <= 1 || clst >= fs->n_fatent) ABORT(fs, FR_INT_ERR);
					fp->clust = clst;
				}
				fp->fptr += ofs;
				if (ofs % SS(fs)) {
					nsect = clst2sect(fs, clst);	/* Current sector */
					if (nsect == 0) ABORT(fs, FR_INT_ERR);
					nsect += (DWORD)(ofs / SS(fs));
				}
			}
		}
		if (!FF_FS_READONLY && fp->fptr > fp->obj.objsize) {	/* Set file change flag if the file size is extended */
			fp->obj.objsize = fp->fptr;
			fp->flag |= FA_MODIFIED;
		}
		if (fp->fptr % SS(fs) && nsect != fp->sect) {	/* Fill sector cache if needed */
#if !FF_FS_TINY
#if !FF_FS_READONLY
			if (fp->flag & FA_DIRTY) {			/* Write-back dirty sector cache */
				if (m_diskio->write(fs->pdrv, fp->buf, fp->sect, 1) != DiskioHardwareBase::DRESULT::RES_OK) ABORT(fs, FR_DISK_ERR);
				fp->flag &= (BYTE)~FA_DIRTY;
			}
#endif
			if (m_diskio->read(fs->pdrv, fp->buf, nsect, 1) != DiskioHardwareBase::DRESULT::RES_OK) ABORT(fs, FR_DISK_ERR);	/* Fill sector cache */
#endif
			fp->sect = nsect;
		}
	}

	LEAVE_FF(fs, res);
}



#if FF_FS_MINIMIZE <= 1
/*-----------------------------------------------------------------------*/
/* Create a Directory Object                                             */
/*-----------------------------------------------------------------------*/
template<typename IOTYPE>
FRESULT Driver<IOTYPE>::f_opendir (
	DIR* dp,			/* Pointer to directory object to create */
	const TCHAR* path	/* Pointer to the directory path */
)
{
	FRESULT res;
	FATFS *fs;
	DEF_NAMBUF


	if (!dp) return FR_INVALID_OBJECT;

	/* Get logical drive */
	res = mount_volume(&path, &fs, 0);
	if (res == FR_OK) {
		dp->obj.fs = fs;
		INIT_NAMBUF(fs);
		res = follow_path(dp, path);			/* Follow the path to the directory */
		if (res == FR_OK) {						/* Follow completed */
			if (!(dp->fn[NSFLAG] & NS_NONAME)) {	/* It is not the origin directory itself */
				if (dp->obj.attr & AM_DIR) {		/* This object is a sub-directory */
#if FF_FS_EXFAT
					if (fs->fs_type == FS_EXFAT) {
						dp->obj.c_scl = dp->obj.sclust;							/* Get containing directory inforamation */
						dp->obj.c_size = ((DWORD)dp->obj.objsize & 0xFFFFFF00) | dp->obj.stat;
						dp->obj.c_ofs = dp->blk_ofs;
						init_alloc_info(fs, &dp->obj);	/* Get object allocation info */
					} else
#endif
					{
						dp->obj.sclust = ld_clust(fs, dp->dir);	/* Get object allocation info */
					}
				} else {						/* This object is a file */
					res = FR_NO_PATH;
				}
			}
			if (res == FR_OK) {
				dp->obj.id = fs->id;
				res = dir_sdi(dp, 0);			/* Rewind directory */
#if FF_FS_LOCK != 0
				if (res == FR_OK) {
					if (dp->obj.sclust != 0) {
						dp->obj.lockid = inc_lock(dp, 0);	/* Lock the sub directory */
						if (!dp->obj.lockid) res = FR_TOO_MANY_OPEN_FILES;
					} else {
						dp->obj.lockid = 0;	/* Root directory need not to be locked */
					}
				}
#endif
			}
		}
		FREE_NAMBUF();
		if (res == FR_NO_FILE) res = FR_NO_PATH;
	}
	if (res != FR_OK) dp->obj.fs = 0;		/* Invalidate the directory object if function faild */

	LEAVE_FF(fs, res);
}




/*-----------------------------------------------------------------------*/
/* Close Directory                                                       */
/*-----------------------------------------------------------------------*/
template<typename IOTYPE>
FRESULT Driver<IOTYPE>::f_closedir (
	DIR *dp		/* Pointer to the directory object to be closed */
)
{
	FRESULT res;
	FATFS *fs;


	res = validate(&dp->obj, &fs);	/* Check validity of the file object */
	if (res == FR_OK) {
#if FF_FS_LOCK != 0
		if (dp->obj.lockid) res = dec_lock(dp->obj.lockid);	/* Decrement sub-directory open counter */
		if (res == FR_OK) dp->obj.fs = 0;	/* Invalidate directory object */
#else
		dp->obj.fs = 0;	/* Invalidate directory object */
#endif
#if FF_FS_REENTRANT
		unlock_fs(fs, FR_OK);		/* Unlock volume */
#endif
	}
	return res;
}




/*-----------------------------------------------------------------------*/
/* Read Directory Entries in Sequence                                    */
/*-----------------------------------------------------------------------*/
template<typename IOTYPE>
FRESULT Driver<IOTYPE>::f_readdir (
	DIR* dp,			/* Pointer to the open directory object */
	FILINFO* fno		/* Pointer to file information to return */
)
{
	FRESULT res;
	FATFS *fs;
	DEF_NAMBUF


	res = validate(&dp->obj, &fs);	/* Check validity of the directory object */
	if (res == FR_OK) {
		if (!fno) {
			res = dir_sdi(dp, 0);			/* Rewind the directory object */
		} else {
			INIT_NAMBUF(fs);
			res = dir_read(dp, static_cast<int>(Directory::READFILE));		/* Read an item */
			if (res == FR_NO_FILE) res = FR_OK;	/* Ignore end of directory */
			if (res == FR_OK) {				/* A valid entry is found */
				get_fileinfo(dp, fno);		/* Get the object information */
				res = dir_next(dp, 0);		/* Increment index for next */
				if (res == FR_NO_FILE) res = FR_OK;	/* Ignore end of directory now */
			}
			FREE_NAMBUF();
		}
	}
	LEAVE_FF(fs, res);
}



#if FF_USE_FIND
/*-----------------------------------------------------------------------*/
/* Find Next File                                                        */
/*-----------------------------------------------------------------------*/
template<typename IOTYPE>
FRESULT Driver<IOTYPE>::f_findnext (
	DIR* dp,		/* Pointer to the open directory object */
	FILINFO* fno	/* Pointer to the file information structure */
)
{
	FRESULT res;


	for (;;) {
		res = f_readdir(dp, fno);		/* Get a directory item */
		if (res != FR_OK || !fno || !fno->fname[0]) break;	/* Terminate if any error or end of directory */
		if (pattern_match(dp->pat, fno->fname, 0, FIND_RECURS)) break;		/* Test for the file name */
#if FF_USE_LFN && FF_USE_FIND == 2
		if (pattern_match(dp->pat, fno->altname, 0, FIND_RECURS)) break;	/* Test for alternative name if exist */
#endif
	}
	return res;
}



/*-----------------------------------------------------------------------*/
/* Find First File                                                       */
/*-----------------------------------------------------------------------*/
template<typename IOTYPE>
FRESULT Driver<IOTYPE>::f_findfirst (
	DIR* dp,				/* Pointer to the blank directory object */
	FILINFO* fno,			/* Pointer to the file information structure */
	const TCHAR* path,		/* Pointer to the directory to open */
	const TCHAR* pattern	/* Pointer to the matching pattern */
)
{
	FRESULT res;


	dp->pat = pattern;		/* Save pointer to pattern string */
	res = f_opendir(dp, path);		/* Open the target directory */
	if (res == FR_OK) {
		res = f_findnext(dp, fno);	/* Find the first item */
	}
	return res;
}

#endif	/* FF_USE_FIND */



#if FF_FS_MINIMIZE == 0
/*-----------------------------------------------------------------------*/
/* Get File Status                                                       */
/*-----------------------------------------------------------------------*/
template<typename IOTYPE>
FRESULT Driver<IOTYPE>::f_stat (
	const TCHAR* path,	/* Pointer to the file path */
	FILINFO* fno		/* Pointer to file information to return */
)
{
	FRESULT res;
	DIR dj;
	DEF_NAMBUF


	/* Get logical drive */
	res = mount_volume(&path, &dj.obj.fs, 0);
	if (res == FR_OK) {
		INIT_NAMBUF(dj.obj.fs);
		res = follow_path(&dj, path);	/* Follow the file path */
		if (res == FR_OK) {				/* Follow completed */
			if (dj.fn[NSFLAG] & NS_NONAME) {	/* It is origin directory */
				res = FR_INVALID_NAME;
			} else {							/* Found an object */
				if (fno) get_fileinfo(&dj, fno);
			}
		}
		FREE_NAMBUF();
	}

	LEAVE_FF(dj.obj.fs, res);
}



#if !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Get Number of Free Clusters                                           */
/*-----------------------------------------------------------------------*/
template<typename IOTYPE>
FRESULT Driver<IOTYPE>::f_getfree (
	const TCHAR* path,	/* Logical drive number */
	DWORD* nclst,		/* Pointer to a variable to return number of free clusters */
	FATFS** fatfs		/* Pointer to return pointer to corresponding filesystem object */
)
{
	FRESULT res;
	FATFS *fs;
	DWORD nfree, clst, stat;
	LBA_t sect;
	UINT i;
	FFOBJID obj;


	/* Get logical drive */
	res = mount_volume(&path, &fs, 0);
	if (res == FR_OK) {
		*fatfs = fs;				/* Return ptr to the fs object */
		/* If free_clst is valid, return it without full FAT scan */
		if (fs->free_clst <= fs->n_fatent - 2) {
			*nclst = fs->free_clst;
		} else {
			/* Scan FAT to obtain number of free clusters */
			nfree = 0;
			if (fs->fs_type == FS_FAT12) {	/* FAT12: Scan bit field FAT entries */
				clst = 2; obj.fs = fs;
				do {
					stat = get_fat(&obj, clst);
					if (stat == 0xFFFFFFFF) { res = FR_DISK_ERR; break; }
					if (stat == 1) { res = FR_INT_ERR; break; }
					if (stat == 0) nfree++;
				} while (++clst < fs->n_fatent);
			} else {
#if FF_FS_EXFAT
				if (fs->fs_type == FS_EXFAT) {	/* exFAT: Scan allocation bitmap */
					BYTE bm;
					UINT b;

					clst = fs->n_fatent - 2;	/* Number of clusters */
					sect = fs->bitbase;			/* Bitmap sector */
					i = 0;						/* Offset in the sector */
					do {	/* Counts numbuer of bits with zero in the bitmap */
						if (i == 0) {
							res = move_window(fs, sect++);
							if (res != FR_OK) break;
						}
						for (b = 8, bm = fs->win[i]; b && clst; b--, clst--) {
							if (!(bm & 1)) nfree++;
							bm >>= 1;
						}
						i = (i + 1) % SS(fs);
					} while (clst);
				} else
#endif
				{	/* FAT16/32: Scan WORD/DWORD FAT entries */
					clst = fs->n_fatent;	/* Number of entries */
					sect = fs->fatbase;		/* Top of the FAT */
					i = 0;					/* Offset in the sector */
					do {	/* Counts numbuer of entries with zero in the FAT */
						if (i == 0) {
							res = move_window(fs, sect++);
							if (res != FR_OK) break;
						}
						if (fs->fs_type == FS_FAT16) {
							if (ld_word(fs->win + i) == 0) nfree++;
							i += 2;
						} else {
							if ((ld_dword(fs->win + i) & 0x0FFFFFFF) == 0) nfree++;
							i += 4;
						}
						i %= SS(fs);
					} while (--clst);
				}
			}
			if (res == FR_OK) {		/* Update parameters if succeeded */
				*nclst = nfree;			/* Return the free clusters */
				fs->free_clst = nfree;	/* Now free_clst is valid */
				fs->fsi_flag |= 1;		/* FAT32: FSInfo is to be updated */
			}
		}
	}

	LEAVE_FF(fs, res);
}




/*-----------------------------------------------------------------------*/
/* Truncate File                                                         */
/*-----------------------------------------------------------------------*/
template<typename IOTYPE>
FRESULT Driver<IOTYPE>::f_truncate (
	FIL* fp		/* Pointer to the file object */
)
{
	FRESULT res;
	FATFS *fs;
	DWORD ncl;


	res = validate(&fp->obj, &fs);	/* Check validity of the file object */
	if (res != FR_OK || (res = (FRESULT)fp->err) != FR_OK) LEAVE_FF(fs, res);
	if (!(fp->flag & FA_WRITE)) LEAVE_FF(fs, FR_DENIED);	/* Check access mode */

	if (fp->fptr < fp->obj.objsize) {	/* Process when fptr is not on the eof */
		if (fp->fptr == 0) {	/* When set file size to zero, remove entire cluster chain */
			res = remove_chain(&fp->obj, fp->obj.sclust, 0);
			fp->obj.sclust = 0;
		} else {				/* When truncate a part of the file, remove remaining clusters */
			ncl = get_fat(&fp->obj, fp->clust);
			res = FR_OK;
			if (ncl == 0xFFFFFFFF) res = FR_DISK_ERR;
			if (ncl == 1) res = FR_INT_ERR;
			if (res == FR_OK && ncl < fs->n_fatent) {
				res = remove_chain(&fp->obj, ncl, fp->clust);
			}
		}
		fp->obj.objsize = fp->fptr;	/* Set file size to current read/write point */
		fp->flag |= FA_MODIFIED;
#if !FF_FS_TINY
		if (res == FR_OK && (fp->flag & FA_DIRTY)) {
			if (m_diskio->write(fs->pdrv, fp->buf, fp->sect, 1) != DiskioHardwareBase::DRESULT::RES_OK) {
				res = FR_DISK_ERR;
			} else {
				fp->flag &= (BYTE)~FA_DIRTY;
			}
		}
#endif
		if (res != FR_OK) ABORT(fs, res);
	}

	LEAVE_FF(fs, res);
}




/*-----------------------------------------------------------------------*/
/* Delete a File/Directory                                               */
/*-----------------------------------------------------------------------*/
template<typename IOTYPE>
FRESULT Driver<IOTYPE>::f_unlink (
	const TCHAR* path		/* Pointer to the file or directory path */
)
{
	FRESULT res;
	DIR dj, sdj;
	DWORD dclst = 0;
	FATFS *fs;
#if FF_FS_EXFAT
	FFOBJID obj;
#endif
	DEF_NAMBUF


	/* Get logical drive */
	res = mount_volume(&path, &fs, FA_WRITE);
	if (res == FR_OK) {
		dj.obj.fs = fs;
		INIT_NAMBUF(fs);
		res = follow_path(&dj, path);		/* Follow the file path */
		if (FF_FS_RPATH && res == FR_OK && (dj.fn[NSFLAG] & NS_DOT)) {
			res = FR_INVALID_NAME;			/* Cannot remove dot entry */
		}
#if FF_FS_LOCK != 0
		if (res == FR_OK) res = chk_lock(&dj, 2);	/* Check if it is an open object */
#endif
		if (res == FR_OK) {					/* The object is accessible */
			if (dj.fn[NSFLAG] & NS_NONAME) {
				res = FR_INVALID_NAME;		/* Cannot remove the origin directory */
			} else {
				if (dj.obj.attr & AM_RDO) {
					res = FR_DENIED;		/* Cannot remove R/O object */
				}
			}
			if (res == FR_OK) {
#if FF_FS_EXFAT
				obj.fs = fs;
				if (fs->fs_type == FS_EXFAT) {
					init_alloc_info(fs, &obj);
					dclst = obj.sclust;
				} else
#endif
				{
					dclst = ld_clust(fs, dj.dir);
				}
				if (dj.obj.attr & AM_DIR) {			/* Is it a sub-directory? */
#if FF_FS_RPATH != 0
					if (dclst == fs->cdir) {	 	/* Is it the current directory? */
						res = FR_DENIED;
					} else
#endif
					{
						sdj.obj.fs = fs;			/* Open the sub-directory */
						sdj.obj.sclust = dclst;
#if FF_FS_EXFAT
						if (fs->fs_type == FS_EXFAT) {
							sdj.obj.objsize = obj.objsize;
							sdj.obj.stat = obj.stat;
						}
#endif
						res = dir_sdi(&sdj, 0);
						if (res == FR_OK) {
							res = dir_read(&sdj, static_cast<int>(Directory::READFILE));			/* Test if the directory is empty */
							if (res == FR_OK) res = FR_DENIED;	/* Not empty? */
							if (res == FR_NO_FILE) res = FR_OK;	/* Empty? */
						}
					}
				}
			}
			if (res == FR_OK) {
				res = dir_remove(&dj);			/* Remove the directory entry */
				if (res == FR_OK && dclst != 0) {	/* Remove the cluster chain if exist */
#if FF_FS_EXFAT
					res = remove_chain(&obj, dclst, 0);
#else
					res = remove_chain(&dj.obj, dclst, 0);
#endif
				}
				if (res == FR_OK) res = sync_fs(fs);
			}
		}
		FREE_NAMBUF();
	}

	LEAVE_FF(fs, res);
}




/*-----------------------------------------------------------------------*/
/* Create a Directory                                                    */
/*-----------------------------------------------------------------------*/
template<typename IOTYPE>
FRESULT Driver<IOTYPE>::f_mkdir (
	const TCHAR* path		/* Pointer to the directory path */
)
{
	FRESULT res;
	DIR dj;
	FFOBJID sobj;
	FATFS *fs;
	DWORD dcl, pcl, tm;
	DEF_NAMBUF


	res = mount_volume(&path, &fs, FA_WRITE);	/* Get logical drive */
	if (res == FR_OK) {
		dj.obj.fs = fs;
		INIT_NAMBUF(fs);
		res = follow_path(&dj, path);			/* Follow the file path */
		if (res == FR_OK) res = FR_EXIST;		/* Name collision? */
		if (FF_FS_RPATH && res == FR_NO_FILE && (dj.fn[NSFLAG] & NS_DOT)) {	/* Invalid name? */
			res = FR_INVALID_NAME;
		}
		if (res == FR_NO_FILE) {				/* It is clear to create a new directory */
			sobj.fs = fs;						/* New object id to create a new chain */
			dcl = create_chain(&sobj, 0);		/* Allocate a cluster for the new directory */
			res = FR_OK;
			if (dcl == 0) res = FR_DENIED;		/* No space to allocate a new cluster? */
			if (dcl == 1) res = FR_INT_ERR;		/* Any insanity? */
			if (dcl == 0xFFFFFFFF) res = FR_DISK_ERR;	/* Disk error? */
			tm = GET_FATTIME();
			if (res == FR_OK) {
				res = dir_clear(fs, dcl);		/* Clean up the new table */
				if (res == FR_OK) {
					if (!FF_FS_EXFAT || fs->fs_type != FS_EXFAT) {	/* Create dot entries (FAT only) */
						std::memset(fs->win + DIR_Name, ' ', 11);	/* Create "." entry */
						fs->win[DIR_Name] = '.';
						fs->win[DIR_Attr] = AM_DIR;
						st_dword(fs->win + DIR_ModTime, tm);
						st_clust(fs, fs->win, dcl);
						memcpy(fs->win + SZDIRE, fs->win, SZDIRE);	/* Create ".." entry */
						fs->win[SZDIRE + 1] = '.'; pcl = dj.obj.sclust;
						st_clust(fs, fs->win + SZDIRE, pcl);
						fs->wflag = 1;
					}
					res = dir_register(&dj);	/* Register the object to the parent directoy */
				}
			}
			if (res == FR_OK) {
#if FF_FS_EXFAT
				if (fs->fs_type == FS_EXFAT) {	/* Initialize directory entry block */
					st_dword(fs->dirbuf + XDIR_ModTime, tm);	/* Created time */
					st_dword(fs->dirbuf + XDIR_FstClus, dcl);	/* Table start cluster */
					st_dword(fs->dirbuf + XDIR_FileSize, (DWORD)fs->csize * SS(fs));	/* Directory size needs to be valid */
					st_dword(fs->dirbuf + XDIR_ValidFileSize, (DWORD)fs->csize * SS(fs));
					fs->dirbuf[XDIR_GenFlags] = 3;				/* Initialize the object flag */
					fs->dirbuf[XDIR_Attr] = AM_DIR;				/* Attribute */
					res = store_xdir(&dj);
				} else
#endif
				{
					st_dword(dj.dir + DIR_ModTime, tm);	/* Created time */
					st_clust(fs, dj.dir, dcl);			/* Table start cluster */
					dj.dir[DIR_Attr] = AM_DIR;			/* Attribute */
					fs->wflag = 1;
				}
				if (res == FR_OK) {
					res = sync_fs(fs);
				}
			} else {
				remove_chain(&sobj, dcl, 0);		/* Could not register, remove the allocated cluster */
			}
		}
		FREE_NAMBUF();
	}

	LEAVE_FF(fs, res);
}




/*-----------------------------------------------------------------------*/
/* Rename a File/Directory                                               */
/*-----------------------------------------------------------------------*/
template<typename IOTYPE>
FRESULT Driver<IOTYPE>::f_rename (
	const TCHAR* path_old,	/* Pointer to the object name to be renamed */
	const TCHAR* path_new	/* Pointer to the new name */
)
{
	FRESULT res;
	DIR djo, djn;
	FATFS *fs;
	BYTE buf[FF_FS_EXFAT ? SZDIRE * 2 : SZDIRE], *dir;
	LBA_t sect;
	DEF_NAMBUF


	get_ldnumber(&path_new);						/* Snip the drive number of new name off */
	res = mount_volume(&path_old, &fs, FA_WRITE);	/* Get logical drive of the old object */
	if (res == FR_OK) {
		djo.obj.fs = fs;
		INIT_NAMBUF(fs);
		res = follow_path(&djo, path_old);			/* Check old object */
		if (res == FR_OK && (djo.fn[NSFLAG] & (NS_DOT | NS_NONAME))) res = FR_INVALID_NAME;	/* Check validity of name */
#if FF_FS_LOCK != 0
		if (res == FR_OK) {
			res = chk_lock(&djo, 2);
		}
#endif
		if (res == FR_OK) {					/* Object to be renamed is found */
#if FF_FS_EXFAT
			if (fs->fs_type == FS_EXFAT) {	/* At exFAT volume */
				BYTE nf, nn;
				WORD nh;

				memcpy(buf, fs->dirbuf, SZDIRE * 2);	/* Save 85+C0 entry of old object */
				memcpy(&djn, &djo, sizeof djo);
				res = follow_path(&djn, path_new);		/* Make sure if new object name is not in use */
				if (res == FR_OK) {						/* Is new name already in use by any other object? */
					res = (djn.obj.sclust == djo.obj.sclust && djn.dptr == djo.dptr) ? FR_NO_FILE : FR_EXIST;
				}
				if (res == FR_NO_FILE) { 				/* It is a valid path and no name collision */
					res = dir_register(&djn);			/* Register the new entry */
					if (res == FR_OK) {
						nf = fs->dirbuf[XDIR_NumSec]; nn = fs->dirbuf[XDIR_NumName];
						nh = ld_word(fs->dirbuf + XDIR_NameHash);
						memcpy(fs->dirbuf, buf, SZDIRE * 2);	/* Restore 85+C0 entry */
						fs->dirbuf[XDIR_NumSec] = nf; fs->dirbuf[XDIR_NumName] = nn;
						st_word(fs->dirbuf + XDIR_NameHash, nh);
						if (!(fs->dirbuf[XDIR_Attr] & AM_DIR)) fs->dirbuf[XDIR_Attr] |= AM_ARC;	/* Set archive attribute if it is a file */
/* Start of critical section where an interruption can cause a cross-link */
						res = store_xdir(&djn);
					}
				}
			} else
#endif
			{	/* At FAT/FAT32 volume */
				memcpy(buf, djo.dir, SZDIRE);			/* Save directory entry of the object */
				memcpy(&djn, &djo, sizeof (DIR));		/* Duplicate the directory object */
				res = follow_path(&djn, path_new);		/* Make sure if new object name is not in use */
				if (res == FR_OK) {						/* Is new name already in use by any other object? */
					res = (djn.obj.sclust == djo.obj.sclust && djn.dptr == djo.dptr) ? FR_NO_FILE : FR_EXIST;
				}
				if (res == FR_NO_FILE) { 				/* It is a valid path and no name collision */
					res = dir_register(&djn);			/* Register the new entry */
					if (res == FR_OK) {
						dir = djn.dir;					/* Copy directory entry of the object except name */
						memcpy(dir + 13, buf + 13, SZDIRE - 13);
						dir[DIR_Attr] = buf[DIR_Attr];
						if (!(dir[DIR_Attr] & AM_DIR)) dir[DIR_Attr] |= AM_ARC;	/* Set archive attribute if it is a file */
						fs->wflag = 1;
						if ((dir[DIR_Attr] & AM_DIR) && djo.obj.sclust != djn.obj.sclust) {	/* Update .. entry in the sub-directory if needed */
							sect = clst2sect(fs, ld_clust(fs, dir));
							if (sect == 0) {
								res = FR_INT_ERR;
							} else {
/* Start of critical section where an interruption can cause a cross-link */
								res = move_window(fs, sect);
								dir = fs->win + SZDIRE * 1;	/* Ptr to .. entry */
								if (res == FR_OK && dir[1] == '.') {
									st_clust(fs, dir, djn.obj.sclust);
									fs->wflag = 1;
								}
							}
						}
					}
				}
			}
			if (res == FR_OK) {
				res = dir_remove(&djo);		/* Remove old entry */
				if (res == FR_OK) {
					res = sync_fs(fs);
				}
			}
/* End of the critical section */
		}
		FREE_NAMBUF();
	}

	LEAVE_FF(fs, res);
}

#endif /* !FF_FS_READONLY */
#endif /* FF_FS_MINIMIZE == 0 */
#endif /* FF_FS_MINIMIZE <= 1 */
#endif /* FF_FS_MINIMIZE <= 2 */



#if FF_USE_CHMOD && !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Change Attribute                                                      */
/*-----------------------------------------------------------------------*/
template<typename IOTYPE>
FRESULT Driver<IOTYPE>::f_chmod (
	const TCHAR* path,	/* Pointer to the file path */
	BYTE attr,			/* Attribute bits */
	BYTE mask			/* Attribute mask to change */
)
{
	FRESULT res;
	DIR dj;
	FATFS *fs;
	DEF_NAMBUF


	res = mount_volume(&path, &fs, FA_WRITE);	/* Get logical drive */
	if (res == FR_OK) {
		dj.obj.fs = fs;
		INIT_NAMBUF(fs);
		res = follow_path(&dj, path);	/* Follow the file path */
		if (res == FR_OK && (dj.fn[NSFLAG] & (NS_DOT | NS_NONAME))) res = FR_INVALID_NAME;	/* Check object validity */
		if (res == FR_OK) {
			mask &= AM_RDO|AM_HID|AM_SYS|AM_ARC;	/* Valid attribute mask */
#if FF_FS_EXFAT
			if (fs->fs_type == FS_EXFAT) {
				fs->dirbuf[XDIR_Attr] = (attr & mask) | (fs->dirbuf[XDIR_Attr] & (BYTE)~mask);	/* Apply attribute change */
				res = store_xdir(&dj);
			} else
#endif
			{
				dj.dir[DIR_Attr] = (attr & mask) | (dj.dir[DIR_Attr] & (BYTE)~mask);	/* Apply attribute change */
				fs->wflag = 1;
			}
			if (res == FR_OK) {
				res = sync_fs(fs);
			}
		}
		FREE_NAMBUF();
	}

	LEAVE_FF(fs, res);
}




/*-----------------------------------------------------------------------*/
/* Change Timestamp                                                      */
/*-----------------------------------------------------------------------*/
template<typename IOTYPE>
FRESULT Driver<IOTYPE>::f_utime (
	const TCHAR* path,	/* Pointer to the file/directory name */
	const FILINFO* fno	/* Pointer to the timestamp to be set */
)
{
	FRESULT res;
	DIR dj;
	FATFS *fs;
	DEF_NAMBUF


	res = mount_volume(&path, &fs, FA_WRITE);	/* Get logical drive */
	if (res == FR_OK) {
		dj.obj.fs = fs;
		INIT_NAMBUF(fs);
		res = follow_path(&dj, path);	/* Follow the file path */
		if (res == FR_OK && (dj.fn[NSFLAG] & (NS_DOT | NS_NONAME))) res = FR_INVALID_NAME;	/* Check object validity */
		if (res == FR_OK) {
#if FF_FS_EXFAT
			if (fs->fs_type == FS_EXFAT) {
				st_dword(fs->dirbuf + XDIR_ModTime, (DWORD)fno->fdate << 16 | fno->ftime);
				res = store_xdir(&dj);
			} else
#endif
			{
				st_dword(dj.dir + DIR_ModTime, (DWORD)fno->fdate << 16 | fno->ftime);
				fs->wflag = 1;
			}
			if (res == FR_OK) {
				res = sync_fs(fs);
			}
		}
		FREE_NAMBUF();
	}

	LEAVE_FF(fs, res);
}

#endif	/* FF_USE_CHMOD && !FF_FS_READONLY */



#if FF_USE_LABEL
/*-----------------------------------------------------------------------*/
/* Get Volume Label                                                      */
/*-----------------------------------------------------------------------*/
template<typename IOTYPE>
FRESULT Driver<IOTYPE>::f_getlabel (
	const TCHAR* path,	/* Logical drive number */
	TCHAR* label,		/* Buffer to store the volume label */
	DWORD* vsn			/* Variable to store the volume serial number */
)
{
	FRESULT res;
	DIR dj;
	FATFS *fs;
	UINT si, di;
	WCHAR wc;

	/* Get logical drive */
	res = mount_volume(&path, &fs, 0);

	/* Get volume label */
	if (res == FR_OK && label) {
		dj.obj.fs = fs; dj.obj.sclust = 0;	/* Open root directory */
		res = dir_sdi(&dj, 0);
		if (res == FR_OK) {
		 	res = dir_read(&dj,static_cast<int>(Directory::READLABEL));		/* Find a volume label entry */
		 	if (res == FR_OK) {
#if FF_FS_EXFAT
				if (fs->fs_type == FS_EXFAT) {
					WCHAR hs;
					UINT nw;

					for (si = di = hs = 0; si < dj.dir[XDIR_NumLabel]; si++) {	/* Extract volume label from 83 entry */
						wc = ld_word(dj.dir + XDIR_Label + si * 2);
						if (hs == 0 && IsSurrogate(wc)) {	/* Is the code a surrogate? */
							hs = wc; continue;
						}
						nw = put_utf((DWORD)hs << 16 | wc, &label[di], 4);	/* Store it in API encoding */
						if (nw == 0) { di = 0; break; }		/* Encode error? */
						di += nw;
						hs = 0;
					}
					if (hs != 0) di = 0;	/* Broken surrogate pair? */
					label[di] = 0;
				} else
#endif
				{
					si = di = 0;		/* Extract volume label from AM_VOL entry */
					while (si < 11) {
						wc = dj.dir[si++];
#if FF_USE_LFN && FF_LFN_UNICODE >= 1 	/* Unicode output */
						if (dbc_1st((BYTE)wc) && si < 11) wc = wc << 8 | dj.dir[si++];	/* Is it a DBC? */
						wc = f_oem2uni(wc, CODEPAGE);		/* Convert it into Unicode */
						if (wc == 0) { di = 0; break; }		/* Invalid char in current code page? */
						di += put_utf(wc, &label[di], 4);	/* Store it in Unicode */
#else									/* ANSI/OEM output */
						label[di++] = (TCHAR)wc;
#endif
					}
					do {				/* Truncate trailing spaces */
						label[di] = 0;
						if (di == 0) break;
					} while (label[--di] == ' ');
				}
			}
		}
		if (res == FR_NO_FILE) {	/* No label entry and return nul string */
			label[0] = 0;
			res = FR_OK;
		}
	}

	/* Get volume serial number */
	if (res == FR_OK && vsn) {
		res = move_window(fs, fs->volbase);
		if (res == FR_OK) {
			switch (fs->fs_type) {
			case FS_EXFAT:
				di = BPB_VolIDEx;
				break;

			case FS_FAT32:
				di = BS_VolID32;
				break;

			default:
				di = BS_VolID;
			}
			*vsn = ld_dword(fs->win + di);
		}
	}

	LEAVE_FF(fs, res);
}



#if !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Set Volume Label                                                      */
/*-----------------------------------------------------------------------*/
template<typename IOTYPE>
FRESULT Driver<IOTYPE>::f_setlabel (
	const TCHAR* label	/* Volume label to set with heading logical drive number */
)
{
	FRESULT res;
	DIR dj;
	FATFS *fs;
	BYTE dirvn[22];
	UINT di;
	WCHAR wc;
	static const char badchr[18] = "+.,;=[]" "/*:<>|\\\"\?\x7F";	/* [0..16] for FAT, [7..16] for exFAT */
#if FF_USE_LFN
	DWORD dc;
#endif

	/* Get logical drive */
	res = mount_volume(&label, &fs, FA_WRITE);
	if (res != FR_OK) LEAVE_FF(fs, res);

#if FF_FS_EXFAT
	if (fs->fs_type == FS_EXFAT) {	/* On the exFAT volume */
		std::memset(dirvn, 0, 22);
		di = 0;
		while ((UINT)*label >= ' ') {	/* Create volume label */
			dc = tchar2uni(&label);	/* Get a Unicode character */
			if (dc >= 0x10000) {
				if (dc == 0xFFFFFFFF || di >= 10) {	/* Wrong surrogate or buffer overflow */
					dc = 0;
				} else {
					st_word(dirvn + di * 2, (WCHAR)(dc >> 16)); di++;
				}
			}
			if (dc == 0 || strchr(&badchr[7], (int)dc) || di >= 11) {	/* Check validity of the volume label */
				LEAVE_FF(fs, FR_INVALID_NAME);
			}
			st_word(dirvn + di * 2, (WCHAR)dc); di++;
		}
	} else
#endif
	{	/* On the FAT/FAT32 volume */
		std::memset(dirvn, ' ', 11);
		di = 0;
		while ((UINT)*label >= ' ') {	/* Create volume label */
#if FF_USE_LFN
			dc = tchar2uni(&label);
			wc = (dc < 0x10000) ? f_uni2oem(f_wtoupper(dc), CODEPAGE) : 0;
#else									/* ANSI/OEM input */
			wc = (BYTE)*label++;
			if (dbc_1st((BYTE)wc)) wc = dbc_2nd((BYTE)*label) ? wc << 8 | (BYTE)*label++ : 0;
			if (std::islower(wc)) wc -= 0x20;		/* To upper ASCII characters */
#if FF_CODE_PAGE == 0
			if (ExCvt && wc >= 0x80) wc = ExCvt[wc - 0x80];	/* To upper extended characters (SBCS cfg) */
#elif FF_CODE_PAGE < 900
			if (wc >= 0x80) wc = ExCvt[wc - 0x80];	/* To upper extended characters (SBCS cfg) */
#endif
#endif
			if (wc == 0 || strchr(&badchr[0], (int)wc) || di >= (UINT)((wc >= 0x100) ? 10 : 11)) {	/* Reject invalid characters for volume label */
				LEAVE_FF(fs, FR_INVALID_NAME);
			}
			if (wc >= 0x100) dirvn[di++] = (BYTE)(wc >> 8);
			dirvn[di++] = (BYTE)wc;
		}
		if (dirvn[0] == DDEM) LEAVE_FF(fs, FR_INVALID_NAME);	/* Reject illegal name (heading DDEM) */
		while (di && dirvn[di - 1] == ' ') di--;				/* Snip trailing spaces */
	}

	/* Set volume label */
	dj.obj.fs = fs; dj.obj.sclust = 0;	/* Open root directory */
	res = dir_sdi(&dj, 0);
	if (res == FR_OK) {
		res = dir_read(&dj,static_cast<int>(Directory::READLABEL));	/* Get volume label entry */
		if (res == FR_OK) {
			if (FF_FS_EXFAT && fs->fs_type == FS_EXFAT) {
				dj.dir[XDIR_NumLabel] = (BYTE)di;	/* Change the volume label */
				memcpy(dj.dir + XDIR_Label, dirvn, 22);
			} else {
				if (di != 0) {
					memcpy(dj.dir, dirvn, 11);	/* Change the volume label */
				} else {
					dj.dir[DIR_Name] = DDEM;	/* Remove the volume label */
				}
			}
			fs->wflag = 1;
			res = sync_fs(fs);
		} else {			/* No volume label entry or an error */
			if (res == FR_NO_FILE) {
				res = FR_OK;
				if (di != 0) {	/* Create a volume label entry */
					res = dir_alloc(&dj, 1);	/* Allocate an entry */
					if (res == FR_OK) {
						std::memset(dj.dir, 0, SZDIRE);	/* Clean the entry */
						if (FF_FS_EXFAT && fs->fs_type == FS_EXFAT) {
							dj.dir[XDIR_Type] = ET_VLABEL;	/* Create volume label entry */
							dj.dir[XDIR_NumLabel] = (BYTE)di;
							memcpy(dj.dir + XDIR_Label, dirvn, 22);
						} else {
							dj.dir[DIR_Attr] = AM_VOL;		/* Create volume label entry */
							memcpy(dj.dir, dirvn, 11);
						}
						fs->wflag = 1;
						res = sync_fs(fs);
					}
				}
			}
		}
	}

	LEAVE_FF(fs, res);
}

#endif /* !FF_FS_READONLY */
#endif /* FF_USE_LABEL */



#if FF_USE_EXPAND && !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Allocate a Contiguous Blocks to the File                              */
/*-----------------------------------------------------------------------*/
template<typename IOTYPE>
FRESULT Driver<IOTYPE>::f_expand (
	FIL* fp,		/* Pointer to the file object */
	FSIZE_t fsz,	/* File size to be expanded to */
	BYTE opt		/* Operation mode 0:Find and prepare or 1:Find and allocate */
)
{
	FRESULT res;
	FATFS *fs;
	DWORD n, clst, stcl, scl, ncl, tcl, lclst;


	res = validate(&fp->obj, &fs);		/* Check validity of the file object */
	if (res != FR_OK || (res = (FRESULT)fp->err) != FR_OK) LEAVE_FF(fs, res);
	if (fsz == 0 || fp->obj.objsize != 0 || !(fp->flag & FA_WRITE)) LEAVE_FF(fs, FR_DENIED);
#if FF_FS_EXFAT
	if (fs->fs_type != FS_EXFAT && fsz >= 0x100000000) LEAVE_FF(fs, FR_DENIED);	/* Check if in size limit */
#endif
	n = (DWORD)fs->csize * SS(fs);	/* Cluster size */
	tcl = (DWORD)(fsz / n) + ((fsz & (n - 1)) ? 1 : 0);	/* Number of clusters required */
	stcl = fs->last_clst; lclst = 0;
	if (stcl < 2 || stcl >= fs->n_fatent) stcl = 2;

#if FF_FS_EXFAT
	if (fs->fs_type == FS_EXFAT) {
		scl = find_bitmap(fs, stcl, tcl);			/* Find a contiguous cluster block */
		if (scl == 0) res = FR_DENIED;				/* No contiguous cluster block was found */
		if (scl == 0xFFFFFFFF) res = FR_DISK_ERR;
		if (res == FR_OK) {	/* A contiguous free area is found */
			if (opt) {		/* Allocate it now */
				res = change_bitmap(fs, scl, tcl, 1);	/* Mark the cluster block 'in use' */
				lclst = scl + tcl - 1;
			} else {		/* Set it as suggested point for next allocation */
				lclst = scl - 1;
			}
		}
	} else
#endif
	{
		scl = clst = stcl; ncl = 0;
		for (;;) {	/* Find a contiguous cluster block */
			n = get_fat(&fp->obj, clst);
			if (++clst >= fs->n_fatent) clst = 2;
			if (n == 1) { res = FR_INT_ERR; break; }
			if (n == 0xFFFFFFFF) { res = FR_DISK_ERR; break; }
			if (n == 0) {	/* Is it a free cluster? */
				if (++ncl == tcl) break;	/* Break if a contiguous cluster block is found */
			} else {
				scl = clst; ncl = 0;		/* Not a free cluster */
			}
			if (clst == stcl) { res = FR_DENIED; break; }	/* No contiguous cluster? */
		}
		if (res == FR_OK) {	/* A contiguous free area is found */
			if (opt) {		/* Allocate it now */
				for (clst = scl, n = tcl; n; clst++, n--) {	/* Create a cluster chain on the FAT */
					res = put_fat(fs, clst, (n == 1) ? 0xFFFFFFFF : clst + 1);
					if (res != FR_OK) break;
					lclst = clst;
				}
			} else {		/* Set it as suggested point for next allocation */
				lclst = scl - 1;
			}
		}
	}

	if (res == FR_OK) {
		fs->last_clst = lclst;		/* Set suggested start cluster to start next */
		if (opt) {	/* Is it allocated now? */
			fp->obj.sclust = scl;		/* Update object allocation information */
			fp->obj.objsize = fsz;
			if (FF_FS_EXFAT) fp->obj.stat = 2;	/* Set status 'contiguous chain' */
			fp->flag |= FA_MODIFIED;
			if (fs->free_clst <= fs->n_fatent - 2) {	/* Update FSINFO */
				fs->free_clst -= tcl;
				fs->fsi_flag |= 1;
			}
		}
	}

	LEAVE_FF(fs, res);
}

#endif /* FF_USE_EXPAND && !FF_FS_READONLY */



#if FF_USE_FORWARD
/*-----------------------------------------------------------------------*/
/* Forward Data to the Stream Directly                                   */
/*-----------------------------------------------------------------------*/
template<typename IOTYPE>
FRESULT Driver<IOTYPE>::f_forward (
	FIL* fp, 						/* Pointer to the file object */
	UINT (*func)(const BYTE*,UINT),	/* Pointer to the streaming function */
	UINT btf,						/* Number of bytes to forward */
	UINT* bf						/* Pointer to number of bytes forwarded */
)
{
	FRESULT res;
	FATFS *fs;
	DWORD clst;
	LBA_t sect;
	FSIZE_t remain;
	UINT rcnt, csect;
	BYTE *dbuf;


	*bf = 0;	/* Clear transfer byte counter */
	res = validate(&fp->obj, &fs);		/* Check validity of the file object */
	if (res != FR_OK || (res = (FRESULT)fp->err) != FR_OK) LEAVE_FF(fs, res);
	if (!(fp->flag & FA_READ)) LEAVE_FF(fs, FR_DENIED);	/* Check access mode */

	remain = fp->obj.objsize - fp->fptr;
	if (btf > remain) btf = (UINT)remain;			/* Truncate btf by remaining bytes */

	for ( ; btf > 0 && (*func)(0, 0); fp->fptr += rcnt, *bf += rcnt, btf -= rcnt) {	/* Repeat until all data transferred or stream goes busy */
		csect = (UINT)(fp->fptr / SS(fs) & (fs->csize - 1));	/* Sector offset in the cluster */
		if (fp->fptr % SS(fs) == 0) {				/* On the sector boundary? */
			if (csect == 0) {						/* On the cluster boundary? */
				clst = (fp->fptr == 0) ?			/* On the top of the file? */
					fp->obj.sclust : get_fat(&fp->obj, fp->clust);
				if (clst <= 1) ABORT(fs, FR_INT_ERR);
				if (clst == 0xFFFFFFFF) ABORT(fs, FR_DISK_ERR);
				fp->clust = clst;					/* Update current cluster */
			}
		}
		sect = clst2sect(fs, fp->clust);			/* Get current data sector */
		if (sect == 0) ABORT(fs, FR_INT_ERR);
		sect += csect;
#if FF_FS_TINY
		if (move_window(fs, sect) != FR_OK) ABORT(fs, FR_DISK_ERR);	/* Move sector window to the file data */
		dbuf = fs->win;
#else
		if (fp->sect != sect) {		/* Fill sector cache with file data */
#if !FF_FS_READONLY
			if (fp->flag & FA_DIRTY) {		/* Write-back dirty sector cache */
				if (m_diskio->write(fs->pdrv, fp->buf, fp->sect, 1) != DiskioHardwareBase::DRESULT::RES_OK) ABORT(fs, FR_DISK_ERR);
				fp->flag &= (BYTE)~FA_DIRTY;
			}
#endif
			if (m_diskio->read(fs->pdrv, fp->buf, sect, 1) != DiskioHardwareBase::DRESULT::RES_OK) ABORT(fs, FR_DISK_ERR);
		}
		dbuf = fp->buf;
#endif
		fp->sect = sect;
		rcnt = SS(fs) - (UINT)fp->fptr % SS(fs);	/* Number of bytes remains in the sector */
		if (rcnt > btf) rcnt = btf;					/* Clip it by btr if needed */
		rcnt = (*func)(dbuf + ((UINT)fp->fptr % SS(fs)), rcnt);	/* Forward the file data */
		if (rcnt == 0) ABORT(fs, FR_INT_ERR);
	}

	LEAVE_FF(fs, FR_OK);
}
#endif /* FF_USE_FORWARD */



#if !FF_FS_READONLY && FF_USE_MKFS
/*-----------------------------------------------------------------------*/
/* Create FAT/exFAT volume (with sub-functions)                          */
/*-----------------------------------------------------------------------*/

#define N_SEC_TRACK 63			/* Sectors per track for determination of drive CHS */
#define	GPT_ALIGN	0x100000	/* Alignment of partitions in GPT [byte] (>=128KB) */
#define GPT_ITEMS	128			/* Number of GPT table size (>=128, sector aligned) */


/* Create partitions on the physical drive in format of MBR or GPT */
template<typename IOTYPE>
FRESULT Driver<IOTYPE>::create_partition (
	BYTE drv,			/* Physical drive number */
	const LBA_t plst[],	/* Partition list */
	BYTE sys,			/* System ID (for only MBR, temp setting) */
	BYTE* buf			/* Working buffer for a sector */
)
{
	UINT i, cy;
	LBA_t sz_drv;
	DWORD sz_drv32, nxt_alloc32, sz_part32;
	BYTE *pte;
	BYTE hd, n_hd, sc, n_sc;

	/* Get physical drive size */
	if (m_diskio->ioctl(drv, DiskioHardwareBase::GET_SECTOR_COUNT, &sz_drv) != DiskioHardwareBase::DRESULT::RES_OK) return FR_DISK_ERR;

#if FF_LBA64
	if (sz_drv >= FF_MIN_GPT) {	/* Create partitions in GPT format */
		WORD ss;
		UINT sz_ptbl, pi, si, ofs;
		DWORD bcc, rnd, align;
		QWORD nxt_alloc, sz_part, sz_pool, top_bpt;
		static const BYTE gpt_mbr[16] = {0x00, 0x00, 0x02, 0x00, 0xEE, 0xFE, 0xFF, 0x00, 0x01, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF};

#if FF_MAX_SS != FF_MIN_SS
		if (m_diskio->ioctl(drv, GET_SECTOR_SIZE, &ss) != DiskioHardwareBase::DRESULT::RES_OK) return FR_DISK_ERR;	/* Get sector size */
		if (ss > FF_MAX_SS || ss < FF_MIN_SS || (ss & (ss - 1))) return FR_DISK_ERR;
#else
		ss = FF_MAX_SS;
#endif
		rnd = (DWORD)sz_drv + GET_FATTIME();	/* Random seed */
		align = GPT_ALIGN / ss;				/* Partition alignment for GPT [sector] */
		sz_ptbl = GPT_ITEMS * SZ_GPTE / ss;	/* Size of partition table [sector] */
		top_bpt = sz_drv - sz_ptbl - 1;		/* Backup partiiton table start sector */
		nxt_alloc = 2 + sz_ptbl;			/* First allocatable sector */
		sz_pool = top_bpt - nxt_alloc;		/* Size of allocatable area */
		bcc = 0xFFFFFFFF; sz_part = 1;
		pi = si = 0;	/* partition table index, size table index */
		do {
			if (pi * SZ_GPTE % ss == 0) std::memset(buf, 0, ss);	/* Clean the buffer if needed */
			if (sz_part != 0) {				/* Is the size table not termintated? */
				nxt_alloc = (nxt_alloc + align - 1) & ((QWORD)0 - align);	/* Align partition start */
				sz_part = plst[si++];		/* Get a partition size */
				if (sz_part <= 100) {		/* Is the size in percentage? */
					sz_part = sz_pool * sz_part / 100;
					sz_part = (sz_part + align - 1) & ((QWORD)0 - align);	/* Align partition end (only if in percentage) */
				}
				if (nxt_alloc + sz_part > top_bpt) {	/* Clip the size at end of the pool */
					sz_part = (nxt_alloc < top_bpt) ? top_bpt - nxt_alloc : 0;
				}
			}
			if (sz_part != 0) {				/* Add a partition? */
				ofs = pi * SZ_GPTE % ss;
				memcpy(buf + ofs + GPTE_PtGuid, GUID_MS_Basic, 16);	/* Set partition GUID (Microsoft Basic Data) */
				rnd = make_rand(rnd, buf + ofs + GPTE_UpGuid, 16);	/* Set unique partition GUID */
				st_qword(buf + ofs + GPTE_FstLba, nxt_alloc);		/* Set partition start sector */
				st_qword(buf + ofs + GPTE_LstLba, nxt_alloc + sz_part - 1);	/* Set partition end sector */
				nxt_alloc += sz_part;								/* Next allocatable sector */
			}
			if ((pi + 1) * SZ_GPTE % ss == 0) {		/* Write the buffer if it is filled up */
				for (i = 0; i < ss; bcc = crc32(bcc, buf[i++])) ;	/* Calculate table check sum */
				if (m_diskio->write(drv, buf, 2 + pi * SZ_GPTE / ss, 1) != DiskioHardwareBase::DRESULT::RES_OK) return FR_DISK_ERR;		/* Write to primary table */
				if (m_diskio->write(drv, buf, top_bpt + pi * SZ_GPTE / ss, 1) != DiskioHardwareBase::DRESULT::RES_OK) return FR_DISK_ERR;	/* Write to secondary table */
			}
		} while (++pi < GPT_ITEMS);

		/* Create primary GPT header */
		std::memset(buf, 0, ss);
		memcpy(buf + GPTH_Sign, "EFI PART" "\0\0\1\0" "\x5C\0\0", 16);	/* Signature, version (1.0) and size (92) */
		st_dword(buf + GPTH_PtBcc, ~bcc);			/* Table check sum */
		st_qword(buf + GPTH_CurLba, 1);				/* LBA of this header */
		st_qword(buf + GPTH_BakLba, sz_drv - 1);	/* LBA of secondary header */
		st_qword(buf + GPTH_FstLba, 2 + sz_ptbl);	/* LBA of first allocatable sector */
		st_qword(buf + GPTH_LstLba, top_bpt - 1);	/* LBA of last allocatable sector */
		st_dword(buf + GPTH_PteSize, SZ_GPTE);		/* Size of a table entry */
		st_dword(buf + GPTH_PtNum, GPT_ITEMS);		/* Number of table entries */
		st_dword(buf + GPTH_PtOfs, 2);				/* LBA of this table */
		rnd = make_rand(rnd, buf + GPTH_DskGuid, 16);	/* Disk GUID */
		for (i = 0, bcc= 0xFFFFFFFF; i < 92; bcc = crc32(bcc, buf[i++])) ;	/* Calculate header check sum */
		st_dword(buf + GPTH_Bcc, ~bcc);				/* Header check sum */
		if (m_diskio->write(drv, buf, 1, 1) != DiskioHardwareBase::DRESULT::RES_OK) return FR_DISK_ERR;

		/* Create secondary GPT header */
		st_qword(buf + GPTH_CurLba, sz_drv - 1);	/* LBA of this header */
		st_qword(buf + GPTH_BakLba, 1);				/* LBA of primary header */
		st_qword(buf + GPTH_PtOfs, top_bpt);		/* LBA of this table */
		st_dword(buf + GPTH_Bcc, 0);
		for (i = 0, bcc= 0xFFFFFFFF; i < 92; bcc = crc32(bcc, buf[i++])) ;	/* Calculate header check sum */
		st_dword(buf + GPTH_Bcc, ~bcc);				/* Header check sum */
		if (m_diskio->write(drv, buf, sz_drv - 1, 1) != DiskioHardwareBase::DRESULT::RES_OK) return FR_DISK_ERR;

		/* Create protective MBR */
		std::memset(buf, 0, ss);
		memcpy(buf + MBR_Table, gpt_mbr, 16);		/* Create a GPT partition */
		st_word(buf + BS_55AA, 0xAA55);
		if (m_diskio->write(drv, buf, 0, 1) != DiskioHardwareBase::DRESULT::RES_OK) return FR_DISK_ERR;

	} else
#endif
	{	/* Create partitions in MBR format */
		sz_drv32 = (DWORD)sz_drv;
		n_sc = N_SEC_TRACK;				/* Determine drive CHS without any consideration of the drive geometry */
		for (n_hd = 8; n_hd != 0 && sz_drv32 / n_hd / n_sc > 1024; n_hd *= 2) ;
		if (n_hd == 0) n_hd = 255;		/* Number of heads needs to be <256 */

		std::memset(buf, 0, FF_MAX_SS);		/* Clear MBR */
		pte = buf + MBR_Table;	/* Partition table in the MBR */
		for (i = 0, nxt_alloc32 = n_sc; i < 4 && nxt_alloc32 != 0 && nxt_alloc32 < sz_drv32; i++, nxt_alloc32 += sz_part32) {
			sz_part32 = (DWORD)plst[i];	/* Get partition size */
			if (sz_part32 <= 100) sz_part32 = (sz_part32 == 100) ? sz_drv32 : sz_drv32 / 100 * sz_part32;	/* Size in percentage? */
			if (nxt_alloc32 + sz_part32 > sz_drv32 || nxt_alloc32 + sz_part32 < nxt_alloc32) sz_part32 = sz_drv32 - nxt_alloc32;	/* Clip at drive size */
			if (sz_part32 == 0) break;	/* End of table or no sector to allocate? */

			st_dword(pte + PTE_StLba, nxt_alloc32);	/* Start LBA */
			st_dword(pte + PTE_SizLba, sz_part32);	/* Number of sectors */
			pte[PTE_System] = sys;					/* System type */

			cy = (UINT)(nxt_alloc32 / n_sc / n_hd);	/* Start cylinder */
			hd = (BYTE)(nxt_alloc32 / n_sc % n_hd);	/* Start head */
			sc = (BYTE)(nxt_alloc32 % n_sc + 1);	/* Start sector */
			pte[PTE_StHead] = hd;
			pte[PTE_StSec] = (BYTE)((cy >> 2 & 0xC0) | sc);
			pte[PTE_StCyl] = (BYTE)cy;

			cy = (UINT)((nxt_alloc32 + sz_part32 - 1) / n_sc / n_hd);	/* End cylinder */
			hd = (BYTE)((nxt_alloc32 + sz_part32 - 1) / n_sc % n_hd);	/* End head */
			sc = (BYTE)((nxt_alloc32 + sz_part32 - 1) % n_sc + 1);		/* End sector */
			pte[PTE_EdHead] = hd;
			pte[PTE_EdSec] = (BYTE)((cy >> 2 & 0xC0) | sc);
			pte[PTE_EdCyl] = (BYTE)cy;

			pte += SZ_PTE;		/* Next entry */
		}

		st_word(buf + BS_55AA, 0xAA55);		/* MBR signature */
		if (m_diskio->write(drv, buf, 0, 1) != DiskioHardwareBase::DRESULT::RES_OK) return FR_DISK_ERR;	/* Write it to the MBR */
	}

	return FR_OK;
}


template<typename IOTYPE>
FRESULT Driver<IOTYPE>::f_mkfs (
	const TCHAR* path,		/* Logical drive number */
	const MKFS_PARM* opt,	/* Format options */
	void* work,				/* Pointer to working buffer (null: use heap memory) */
	UINT len				/* Size of working buffer [byte] */
)
{
	static const WORD cst[] = {1, 4, 16, 64, 256, 512, 0};	/* Cluster size boundary for FAT volume (4Ks unit) */
	static const WORD cst32[] = {1, 2, 4, 8, 16, 32, 0};	/* Cluster size boundary for FAT32 volume (128Ks unit) */
	static const MKFS_PARM defopt = {FM_ANY, 0, 0, 0, 0};	/* Default parameter */
	BYTE fsopt, fsty, sys, *buf, *pte, pdrv, ipart;
	WORD ss;	/* Sector size */
	DWORD sz_buf, sz_blk, n_clst, pau, nsect, n, vsn;
	LBA_t sz_vol, b_vol, b_fat, b_data;		/* Size of volume, Base LBA of volume, fat, data */
	LBA_t sect, lba[2];
	DWORD sz_rsv, sz_fat, sz_dir, sz_au;	/* Size of reserved, fat, dir, data, cluster */
	UINT n_fat, n_root, i;					/* Index, Number of FATs and Number of roor dir entries */
	int vol;
	DiskioHardwareBase::DSTATUS ds;
	FRESULT fr;


	/* Check mounted drive and clear work area */
	vol = get_ldnumber(&path);					/* Get target logical drive */
	if (vol < 0) return FR_INVALID_DRIVE;
	if (FatFs[vol]) FatFs[vol]->fs_type = 0;	/* Clear the fs object if mounted */
	pdrv = LD2PD(vol);			/* Physical drive */
	ipart = LD2PT(vol);			/* Partition (0:create as new, 1..:get from partition table) */
	if (!opt) opt = &defopt;	/* Use default parameter if it is not given */

	/* Get physical drive status (sz_drv, sz_blk, ss) */
	ds = m_diskio->initialize(pdrv);
	if (ds & DiskioHardwareBase::STA_NOINIT) return FR_NOT_READY;
	if (ds & DiskioHardwareBase::STA_PROTECT) return FR_WRITE_PROTECTED;
	sz_blk = opt->align;
	if (sz_blk == 0 && m_diskio->ioctl(pdrv, DiskioHardwareBase::GET_BLOCK_SIZE, &sz_blk) != DiskioHardwareBase::DRESULT::RES_OK) sz_blk = 1;
 	if (sz_blk == 0 || sz_blk > 0x8000 || (sz_blk & (sz_blk - 1))) sz_blk = 1;
#if FF_MAX_SS != FF_MIN_SS
	if (m_diskio->ioctl(pdrv, GET_SECTOR_SIZE, &ss) != DiskioHardwareBase::DRESULT::RES_OK) return FR_DISK_ERR;
	if (ss > FF_MAX_SS || ss < FF_MIN_SS || (ss & (ss - 1))) return FR_DISK_ERR;
#else
	ss = FF_MAX_SS;
#endif
	/* Options for FAT sub-type and FAT parameters */
	fsopt = opt->fmt & (FM_ANY | FM_SFD);
	n_fat = (opt->n_fat >= 1 && opt->n_fat <= 2) ? opt->n_fat : 1;
	n_root = (opt->n_root >= 1 && opt->n_root <= 32768 && (opt->n_root % (ss / SZDIRE)) == 0) ? opt->n_root : 512;
	sz_au = (opt->au_size <= 0x1000000 && (opt->au_size & (opt->au_size - 1)) == 0) ? opt->au_size : 0;
	sz_au /= ss;	/* Byte --> Sector */

	/* Get working buffer */
	sz_buf = len / ss;		/* Size of working buffer [sector] */
	if (sz_buf == 0) return FR_NOT_ENOUGH_CORE;
	buf = (BYTE*)work;		/* Working buffer */
#if FF_USE_LFN == 3
	if (!buf) buf = ff_memalloc(sz_buf * ss);	/* Use heap memory for working buffer */
#endif
	if (!buf) return FR_NOT_ENOUGH_CORE;

	/* Determine where the volume to be located (b_vol, sz_vol) */
	b_vol = sz_vol = 0;
	if (FF_MULTI_PARTITION && ipart != 0) {	/* Is the volume associated with any specific partition? */
		/* Get partition location from the existing partition table */
		if (m_diskio->read(pdrv, buf, 0, 1) != DiskioHardwareBase::DRESULT::RES_OK) LEAVE_MKFS(FR_DISK_ERR);	/* Load MBR */
		if (ld_word(buf + BS_55AA) != 0xAA55) LEAVE_MKFS(FR_MKFS_ABORTED);	/* Check if MBR is valid */
#if FF_LBA64
		if (buf[MBR_Table + PTE_System] == 0xEE) {	/* GPT protective MBR? */
			DWORD n_ent, ofs;
			QWORD pt_lba;

			/* Get the partition location from GPT */
			if (m_diskio->read(pdrv, buf, 1, 1) != DiskioHardwareBase::DRESULT::RES_OK) LEAVE_MKFS(FR_DISK_ERR);	/* Load GPT header sector (next to MBR) */
			if (!test_gpt_header(buf)) LEAVE_MKFS(FR_MKFS_ABORTED);	/* Check if GPT header is valid */
			n_ent = ld_dword(buf + GPTH_PtNum);		/* Number of entries */
			pt_lba = ld_qword(buf + GPTH_PtOfs);	/* Table start sector */
			ofs = i = 0;
			while (n_ent) {		/* Find MS Basic partition with order of ipart */
				if (ofs == 0 && m_diskio->read(pdrv, buf, pt_lba++, 1) != DiskioHardwareBase::DRESULT::RES_OK) LEAVE_MKFS(FR_DISK_ERR);	/* Get PT sector */
				if (!memcmp(buf + ofs + GPTE_PtGuid, GUID_MS_Basic, 16) && ++i == ipart) {	/* MS basic data partition? */
					b_vol = ld_qword(buf + ofs + GPTE_FstLba);
					sz_vol = ld_qword(buf + ofs + GPTE_LstLba) - b_vol + 1;
					break;
				}
				n_ent--; ofs = (ofs + SZ_GPTE) % ss;	/* Next entry */
			}
			if (n_ent == 0) LEAVE_MKFS(FR_MKFS_ABORTED);	/* Partition not found */
			fsopt |= 0x80;	/* Partitioning is in GPT */
		} else
#endif
		{	/* Get the partition location from MBR partition table */
			pte = buf + (MBR_Table + (ipart - 1) * SZ_PTE);
			if (ipart > 4 || pte[PTE_System] == 0) LEAVE_MKFS(FR_MKFS_ABORTED);	/* No partition? */
			b_vol = ld_dword(pte + PTE_StLba);		/* Get volume start sector */
			sz_vol = ld_dword(pte + PTE_SizLba);	/* Get volume size */
		}
	} else {	/* The volume is associated with a physical drive */
		if (m_diskio->ioctl(pdrv, DiskioHardwareBase::GET_SECTOR_COUNT, &sz_vol) != DiskioHardwareBase::DRESULT::RES_OK) LEAVE_MKFS(FR_DISK_ERR);
		if (!(fsopt & FM_SFD)) {	/* To be partitioned? */
			/* Create a single-partition on the drive in this function */
#if FF_LBA64
			if (sz_vol >= FF_MIN_GPT) {	/* Which partition type to create, MBR or GPT? */
				fsopt |= 0x80;		/* Partitioning is in GPT */
				b_vol = GPT_ALIGN / ss; sz_vol -= b_vol + GPT_ITEMS * SZ_GPTE / ss + 1;	/* Estimated partition offset and size */
			} else
#endif
			{	/* Partitioning is in MBR */
				if (sz_vol > N_SEC_TRACK) {
					b_vol = N_SEC_TRACK; sz_vol -= b_vol;	/* Estimated partition offset and size */
				}
			}
		}
	}
	if (sz_vol < 128) LEAVE_MKFS(FR_MKFS_ABORTED);	/* Check if volume size is >=128s */

	/* Now start to create an FAT volume at b_vol and sz_vol */

	do {	/* Pre-determine the FAT type */
		if (FF_FS_EXFAT && (fsopt & FM_EXFAT)) {	/* exFAT possible? */
			if ((fsopt & FM_ANY) == FM_EXFAT || sz_vol >= 0x4000000 || sz_au > 128) {	/* exFAT only, vol >= 64MS or sz_au > 128S ? */
				fsty = FS_EXFAT; break;
			}
		}
#if FF_LBA64
		if (sz_vol >= 0x100000000) LEAVE_MKFS(FR_MKFS_ABORTED);	/* Too large volume for FAT/FAT32 */
#endif
		if (sz_au > 128) sz_au = 128;	/* Invalid AU for FAT/FAT32? */
		if (fsopt & FM_FAT32) {	/* FAT32 possible? */
			if (!(fsopt & FM_FAT)) {	/* no-FAT? */
				fsty = FS_FAT32; break;
			}
		}
		if (!(fsopt & FM_FAT)) LEAVE_MKFS(FR_INVALID_PARAMETER);	/* no-FAT? */
		fsty = FS_FAT16;
	} while (0);

	vsn = (DWORD)sz_vol + GET_FATTIME();	/* VSN generated from current time and partitiion size */

#if FF_FS_EXFAT
	if (fsty == FS_EXFAT) {	/* Create an exFAT volume */
		DWORD szb_bit, szb_case, sum, nbit, clu, clen[3];
		WCHAR ch, si;
		UINT j, st;

		if (sz_vol < 0x1000) LEAVE_MKFS(FR_MKFS_ABORTED);	/* Too small volume for exFAT? */
#if FF_USE_TRIM
		lba[0] = b_vol; lba[1] = b_vol + sz_vol - 1;	/* Inform storage device that the volume area may be erased */
		m_diskio->ioctl(pdrv, DiskioHardwareBase::CTRL_TRIM, lba);
#endif
		/* Determine FAT location, data location and number of clusters */
		if (sz_au == 0) {	/* AU auto-selection */
			sz_au = 8;
			if (sz_vol >= 0x80000) sz_au = 64;		/* >= 512Ks */
			if (sz_vol >= 0x4000000) sz_au = 256;	/* >= 64Ms */
		}
		b_fat = b_vol + 32;										/* FAT start at offset 32 */
		sz_fat = (DWORD)((sz_vol / sz_au + 2) * 4 + ss - 1) / ss;	/* Number of FAT sectors */
		b_data = (b_fat + sz_fat + sz_blk - 1) & ~((LBA_t)sz_blk - 1);	/* Align data area to the erase block boundary */
		if (b_data - b_vol >= sz_vol / 2) LEAVE_MKFS(FR_MKFS_ABORTED);	/* Too small volume? */
		n_clst = (DWORD)(sz_vol - (b_data - b_vol)) / sz_au;	/* Number of clusters */
		if (n_clst <16) LEAVE_MKFS(FR_MKFS_ABORTED);			/* Too few clusters? */
		if (n_clst > MAX_EXFAT) LEAVE_MKFS(FR_MKFS_ABORTED);	/* Too many clusters? */

		szb_bit = (n_clst + 7) / 8;								/* Size of allocation bitmap */
		clen[0] = (szb_bit + sz_au * ss - 1) / (sz_au * ss);	/* Number of allocation bitmap clusters */

		/* Create a compressed up-case table */
		sect = b_data + sz_au * clen[0];	/* Table start sector */
		sum = 0;							/* Table checksum to be stored in the 82 entry */
		st = 0; si = 0; i = 0; j = 0; szb_case = 0;
		do {
			switch (st) {
			case 0:
				ch = (WCHAR)f_wtoupper(si);	/* Get an up-case char */
				if (ch != si) {
					si++; break;		/* Store the up-case char if exist */
				}
				for (j = 1; (WCHAR)(si + j) && (WCHAR)(si + j) == f_wtoupper((WCHAR)(si + j)); j++) ;	/* Get run length of no-case block */
				if (j >= 128) {
					ch = 0xFFFF; st = 2; break;	/* Compress the no-case block if run is >= 128 chars */
				}
				st = 1;			/* Do not compress short run */
				/* FALLTHROUGH */
			case 1:
				ch = si++;		/* Fill the short run */
				if (--j == 0) st = 0;
				break;

			default:
				ch = (WCHAR)j; si += (WCHAR)j;	/* Number of chars to skip */
				st = 0;
			}
			sum = xsum32(buf[i + 0] = (BYTE)ch, sum);	/* Put it into the write buffer */
			sum = xsum32(buf[i + 1] = (BYTE)(ch >> 8), sum);
			i += 2; szb_case += 2;
			if (si == 0 || i == sz_buf * ss) {		/* Write buffered data when buffer full or end of process */
				n = (i + ss - 1) / ss;
				if (m_diskio->write(pdrv, buf, sect, n) != DiskioHardwareBase::DRESULT::RES_OK) LEAVE_MKFS(FR_DISK_ERR);
				sect += n; i = 0;
			}
		} while (si);
		clen[1] = (szb_case + sz_au * ss - 1) / (sz_au * ss);	/* Number of up-case table clusters */
		clen[2] = 1;	/* Number of root dir clusters */

		/* Initialize the allocation bitmap */
		sect = b_data; nsect = (szb_bit + ss - 1) / ss;	/* Start of bitmap and number of bitmap sectors */
		nbit = clen[0] + clen[1] + clen[2];				/* Number of clusters in-use by system (bitmap, up-case and root-dir) */
		do {
			std::memset(buf, 0, sz_buf * ss);				/* Initialize bitmap buffer */
			for (i = 0; nbit != 0 && i / 8 < sz_buf * ss; buf[i / 8] |= 1 << (i % 8), i++, nbit--) ;	/* Mark used clusters */
			n = (nsect > sz_buf) ? sz_buf : nsect;		/* Write the buffered data */
			if (m_diskio->write(pdrv, buf, sect, n) != DiskioHardwareBase::DRESULT::RES_OK) LEAVE_MKFS(FR_DISK_ERR);
			sect += n; nsect -= n;
		} while (nsect);

		/* Initialize the FAT */
		sect = b_fat; nsect = sz_fat;	/* Start of FAT and number of FAT sectors */
		j = nbit = clu = 0;
		do {
			std::memset(buf, 0, sz_buf * ss); i = 0;	/* Clear work area and reset write offset */
			if (clu == 0) {	/* Initialize FAT [0] and FAT[1] */
				st_dword(buf + i, 0xFFFFFFF8); i += 4; clu++;
				st_dword(buf + i, 0xFFFFFFFF); i += 4; clu++;
			}
			do {			/* Create chains of bitmap, up-case and root dir */
				while (nbit != 0 && i < sz_buf * ss) {	/* Create a chain */
					st_dword(buf + i, (nbit > 1) ? clu + 1 : 0xFFFFFFFF);
					i += 4; clu++; nbit--;
				}
				if (nbit == 0 && j < 3) nbit = clen[j++];	/* Get next chain length */
			} while (nbit != 0 && i < sz_buf * ss);
			n = (nsect > sz_buf) ? sz_buf : nsect;	/* Write the buffered data */
			if (m_diskio->write(pdrv, buf, sect, n) != DiskioHardwareBase::DRESULT::RES_OK) LEAVE_MKFS(FR_DISK_ERR);
			sect += n; nsect -= n;
		} while (nsect);

		/* Initialize the root directory */
		std::memset(buf, 0, sz_buf * ss);
		buf[SZDIRE * 0 + 0] = ET_VLABEL;				/* Volume label entry (no label) */
		buf[SZDIRE * 1 + 0] = ET_BITMAP;				/* Bitmap entry */
		st_dword(buf + SZDIRE * 1 + 20, 2);				/*  cluster */
		st_dword(buf + SZDIRE * 1 + 24, szb_bit);		/*  size */
		buf[SZDIRE * 2 + 0] = ET_UPCASE;				/* Up-case table entry */
		st_dword(buf + SZDIRE * 2 + 4, sum);			/*  sum */
		st_dword(buf + SZDIRE * 2 + 20, 2 + clen[0]);	/*  cluster */
		st_dword(buf + SZDIRE * 2 + 24, szb_case);		/*  size */
		sect = b_data + sz_au * (clen[0] + clen[1]); nsect = sz_au;	/* Start of the root directory and number of sectors */
		do {	/* Fill root directory sectors */
			n = (nsect > sz_buf) ? sz_buf : nsect;
			if (m_diskio->write(pdrv, buf, sect, n) != DiskioHardwareBase::DRESULT::RES_OK) LEAVE_MKFS(FR_DISK_ERR);
			std::memset(buf, 0, ss);	/* Rest of entries are filled with zero */
			sect += n; nsect -= n;
		} while (nsect);

		/* Create two set of the exFAT VBR blocks */
		sect = b_vol;
		for (n = 0; n < 2; n++) {
			/* Main record (+0) */
			std::memset(buf, 0, ss);
			memcpy(buf + BS_JmpBoot, "\xEB\x76\x90" "EXFAT   ", 11);	/* Boot jump code (x86), OEM name */
			st_qword(buf + BPB_VolOfsEx, b_vol);					/* Volume offset in the physical drive [sector] */
			st_qword(buf + BPB_TotSecEx, sz_vol);					/* Volume size [sector] */
			st_dword(buf + BPB_FatOfsEx, (DWORD)(b_fat - b_vol));	/* FAT offset [sector] */
			st_dword(buf + BPB_FatSzEx, sz_fat);					/* FAT size [sector] */
			st_dword(buf + BPB_DataOfsEx, (DWORD)(b_data - b_vol));	/* Data offset [sector] */
			st_dword(buf + BPB_NumClusEx, n_clst);					/* Number of clusters */
			st_dword(buf + BPB_RootClusEx, 2 + clen[0] + clen[1]);	/* Root dir cluster # */
			st_dword(buf + BPB_VolIDEx, vsn);						/* VSN */
			st_word(buf + BPB_FSVerEx, 0x100);						/* Filesystem version (1.00) */
			for (buf[BPB_BytsPerSecEx] = 0, i = ss; i >>= 1; buf[BPB_BytsPerSecEx]++) ;	/* Log2 of sector size [byte] */
			for (buf[BPB_SecPerClusEx] = 0, i = sz_au; i >>= 1; buf[BPB_SecPerClusEx]++) ;	/* Log2 of cluster size [sector] */
			buf[BPB_NumFATsEx] = 1;					/* Number of FATs */
			buf[BPB_DrvNumEx] = 0x80;				/* Drive number (for int13) */
			st_word(buf + BS_BootCodeEx, 0xFEEB);	/* Boot code (x86) */
			st_word(buf + BS_55AA, 0xAA55);			/* Signature (placed here regardless of sector size) */
			for (i = sum = 0; i < ss; i++) {		/* VBR checksum */
				if (i != BPB_VolFlagEx && i != BPB_VolFlagEx + 1 && i != BPB_PercInUseEx) sum = xsum32(buf[i], sum);
			}
			if (m_diskio->write(pdrv, buf, sect++, 1) != DiskioHardwareBase::DRESULT::RES_OK) LEAVE_MKFS(FR_DISK_ERR);
			/* Extended bootstrap record (+1..+8) */
			std::memset(buf, 0, ss);
			st_word(buf + ss - 2, 0xAA55);	/* Signature (placed at end of sector) */
			for (j = 1; j < 9; j++) {
				for (i = 0; i < ss; sum = xsum32(buf[i++], sum)) ;	/* VBR checksum */
				if (m_diskio->write(pdrv, buf, sect++, 1) != DiskioHardwareBase::DRESULT::RES_OK) LEAVE_MKFS(FR_DISK_ERR);
			}
			/* OEM/Reserved record (+9..+10) */
			std::memset(buf, 0, ss);
			for ( ; j < 11; j++) {
				for (i = 0; i < ss; sum = xsum32(buf[i++], sum)) ;	/* VBR checksum */
				if (m_diskio->write(pdrv, buf, sect++, 1) != DiskioHardwareBase::DRESULT::RES_OK) LEAVE_MKFS(FR_DISK_ERR);
			}
			/* Sum record (+11) */
			for (i = 0; i < ss; i += 4) st_dword(buf + i, sum);		/* Fill with checksum value */
			if (m_diskio->write(pdrv, buf, sect++, 1) != DiskioHardwareBase::DRESULT::RES_OK) LEAVE_MKFS(FR_DISK_ERR);
		}

	} else
#endif	/* FF_FS_EXFAT */
	{	/* Create an FAT/FAT32 volume */
		do {
			pau = sz_au;
			/* Pre-determine number of clusters and FAT sub-type */
			if (fsty == FS_FAT32) {	/* FAT32 volume */
				if (pau == 0) {	/* AU auto-selection */
					n = (DWORD)sz_vol / 0x20000;	/* Volume size in unit of 128KS */
					for (i = 0, pau = 1; cst32[i] && cst32[i] <= n; i++, pau <<= 1) ;	/* Get from table */
				}
				n_clst = (DWORD)sz_vol / pau;	/* Number of clusters */
				sz_fat = (n_clst * 4 + 8 + ss - 1) / ss;	/* FAT size [sector] */
				sz_rsv = 32;	/* Number of reserved sectors */
				sz_dir = 0;		/* No static directory */
				if (n_clst <= MAX_FAT16 || n_clst > MAX_FAT32) LEAVE_MKFS(FR_MKFS_ABORTED);
			} else {				/* FAT volume */
				if (pau == 0) {	/* au auto-selection */
					n = (DWORD)sz_vol / 0x1000;	/* Volume size in unit of 4KS */
					for (i = 0, pau = 1; cst[i] && cst[i] <= n; i++, pau <<= 1) ;	/* Get from table */
				}
				n_clst = (DWORD)sz_vol / pau;
				if (n_clst > MAX_FAT12) {
					n = n_clst * 2 + 4;		/* FAT size [byte] */
				} else {
					fsty = FS_FAT12;
					n = (n_clst * 3 + 1) / 2 + 3;	/* FAT size [byte] */
				}
				sz_fat = (n + ss - 1) / ss;		/* FAT size [sector] */
				sz_rsv = 1;						/* Number of reserved sectors */
				sz_dir = (DWORD)n_root * SZDIRE / ss;	/* Root dir size [sector] */
			}
			b_fat = b_vol + sz_rsv;						/* FAT base */
			b_data = b_fat + sz_fat * n_fat + sz_dir;	/* Data base */

			/* Align data area to erase block boundary (for flash memory media) */
			n = (DWORD)(((b_data + sz_blk - 1) & ~(sz_blk - 1)) - b_data);	/* Sectors to next nearest from current data base */
			if (fsty == FS_FAT32) {		/* FAT32: Move FAT */
				sz_rsv += n; b_fat += n;
			} else {					/* FAT: Expand FAT */
				if (n % n_fat) {	/* Adjust fractional error if needed */
					n--; sz_rsv++; b_fat++;
				}
				sz_fat += n / n_fat;
			}

			/* Determine number of clusters and final check of validity of the FAT sub-type */
			if (sz_vol < b_data + pau * 16 - b_vol) LEAVE_MKFS(FR_MKFS_ABORTED);	/* Too small volume? */
			n_clst = ((DWORD)sz_vol - sz_rsv - sz_fat * n_fat - sz_dir) / pau;
			if (fsty == FS_FAT32) {
				if (n_clst <= MAX_FAT16) {	/* Too few clusters for FAT32? */
					if (sz_au == 0 && (sz_au = pau / 2) != 0) continue;	/* Adjust cluster size and retry */
					LEAVE_MKFS(FR_MKFS_ABORTED);
				}
			}
			if (fsty == FS_FAT16) {
				if (n_clst > MAX_FAT16) {	/* Too many clusters for FAT16 */
					if (sz_au == 0 && (pau * 2) <= 64) {
						sz_au = pau * 2; continue;	/* Adjust cluster size and retry */
					}
					if ((fsopt & FM_FAT32)) {
						fsty = FS_FAT32; continue;	/* Switch type to FAT32 and retry */
					}
					if (sz_au == 0 && (sz_au = pau * 2) <= 128) continue;	/* Adjust cluster size and retry */
					LEAVE_MKFS(FR_MKFS_ABORTED);
				}
				if  (n_clst <= MAX_FAT12) {	/* Too few clusters for FAT16 */
					if (sz_au == 0 && (sz_au = pau * 2) <= 128) continue;	/* Adjust cluster size and retry */
					LEAVE_MKFS(FR_MKFS_ABORTED);
				}
			}
			if (fsty == FS_FAT12 && n_clst > MAX_FAT12) LEAVE_MKFS(FR_MKFS_ABORTED);	/* Too many clusters for FAT12 */

			/* Ok, it is the valid cluster configuration */
			break;
		} while (1);

#if FF_USE_TRIM
		lba[0] = b_vol; lba[1] = b_vol + sz_vol - 1;	/* Inform storage device that the volume area may be erased */
		m_diskio->ioctl(pdrv, DiskioHardwareBase::CTRL_TRIM, lba);
#endif
		/* Create FAT VBR */
		std::memset(buf, 0, ss);
		memcpy(buf + BS_JmpBoot, "\xEB\xFE\x90" "MSDOS5.0", 11);	/* Boot jump code (x86), OEM name */
		st_word(buf + BPB_BytsPerSec, ss);				/* Sector size [byte] */
		buf[BPB_SecPerClus] = (BYTE)pau;				/* Cluster size [sector] */
		st_word(buf + BPB_RsvdSecCnt, (WORD)sz_rsv);	/* Size of reserved area */
		buf[BPB_NumFATs] = (BYTE)n_fat;					/* Number of FATs */
		st_word(buf + BPB_RootEntCnt, (WORD)((fsty == FS_FAT32) ? 0 : n_root));	/* Number of root directory entries */
		if (sz_vol < 0x10000) {
			st_word(buf + BPB_TotSec16, (WORD)sz_vol);	/* Volume size in 16-bit LBA */
		} else {
			st_dword(buf + BPB_TotSec32, (DWORD)sz_vol);	/* Volume size in 32-bit LBA */
		}
		buf[BPB_Media] = 0xF8;							/* Media descriptor byte */
		st_word(buf + BPB_SecPerTrk, 63);				/* Number of sectors per track (for int13) */
		st_word(buf + BPB_NumHeads, 255);				/* Number of heads (for int13) */
		st_dword(buf + BPB_HiddSec, (DWORD)b_vol);		/* Volume offset in the physical drive [sector] */
		if (fsty == FS_FAT32) {
			st_dword(buf + BS_VolID32, vsn);			/* VSN */
			st_dword(buf + BPB_FATSz32, sz_fat);		/* FAT size [sector] */
			st_dword(buf + BPB_RootClus32, 2);			/* Root directory cluster # (2) */
			st_word(buf + BPB_FSInfo32, 1);				/* Offset of FSINFO sector (VBR + 1) */
			st_word(buf + BPB_BkBootSec32, 6);			/* Offset of backup VBR (VBR + 6) */
			buf[BS_DrvNum32] = 0x80;					/* Drive number (for int13) */
			buf[BS_BootSig32] = 0x29;					/* Extended boot signature */
			memcpy(buf + BS_VolLab32, "NO NAME    " "FAT32   ", 19);	/* Volume label, FAT signature */
		} else {
			st_dword(buf + BS_VolID, vsn);				/* VSN */
			st_word(buf + BPB_FATSz16, (WORD)sz_fat);	/* FAT size [sector] */
			buf[BS_DrvNum] = 0x80;						/* Drive number (for int13) */
			buf[BS_BootSig] = 0x29;						/* Extended boot signature */
			memcpy(buf + BS_VolLab, "NO NAME    " "FAT     ", 19);	/* Volume label, FAT signature */
		}
		st_word(buf + BS_55AA, 0xAA55);					/* Signature (offset is fixed here regardless of sector size) */
		if (m_diskio->write(pdrv, buf, b_vol, 1) != DiskioHardwareBase::DRESULT::RES_OK) LEAVE_MKFS(FR_DISK_ERR);	/* Write it to the VBR sector */

		/* Create FSINFO record if needed */
		if (fsty == FS_FAT32) {
			m_diskio->write(pdrv, buf, b_vol + 6, 1);		/* Write backup VBR (VBR + 6) */
			std::memset(buf, 0, ss);
			st_dword(buf + FSI_LeadSig, 0x41615252);
			st_dword(buf + FSI_StrucSig, 0x61417272);
			st_dword(buf + FSI_Free_Count, n_clst - 1);	/* Number of free clusters */
			st_dword(buf + FSI_Nxt_Free, 2);			/* Last allocated cluster# */
			st_word(buf + BS_55AA, 0xAA55);
			m_diskio->write(pdrv, buf, b_vol + 7, 1);		/* Write backup FSINFO (VBR + 7) */
			m_diskio->write(pdrv, buf, b_vol + 1, 1);		/* Write original FSINFO (VBR + 1) */
		}

		/* Initialize FAT area */
		std::memset(buf, 0, sz_buf * ss);
		sect = b_fat;		/* FAT start sector */
		for (i = 0; i < n_fat; i++) {			/* Initialize FATs each */
			if (fsty == FS_FAT32) {
				st_dword(buf + 0, 0xFFFFFFF8);	/* FAT[0] */
				st_dword(buf + 4, 0xFFFFFFFF);	/* FAT[1] */
				st_dword(buf + 8, 0x0FFFFFFF);	/* FAT[2] (root directory) */
			} else {
				st_dword(buf + 0, (fsty == FS_FAT12) ? 0xFFFFF8 : 0xFFFFFFF8);	/* FAT[0] and FAT[1] */
			}
			nsect = sz_fat;		/* Number of FAT sectors */
			do {	/* Fill FAT sectors */
				n = (nsect > sz_buf) ? sz_buf : nsect;
				if (m_diskio->write(pdrv, buf, sect, (UINT)n) != DiskioHardwareBase::DRESULT::RES_OK) LEAVE_MKFS(FR_DISK_ERR);
				std::memset(buf, 0, ss);	/* Rest of FAT all are cleared */
				sect += n; nsect -= n;
			} while (nsect);
		}

		/* Initialize root directory (fill with zero) */
		nsect = (fsty == FS_FAT32) ? pau : sz_dir;	/* Number of root directory sectors */
		do {
			n = (nsect > sz_buf) ? sz_buf : nsect;
			if (m_diskio->write(pdrv, buf, sect, (UINT)n) != DiskioHardwareBase::DRESULT::RES_OK) LEAVE_MKFS(FR_DISK_ERR);
			sect += n; nsect -= n;
		} while (nsect);
	}

	/* A FAT volume has been created here */

	/* Determine system ID in the MBR partition table */
	if (FF_FS_EXFAT && fsty == FS_EXFAT) {
		sys = 0x07;			/* exFAT */
	} else {
		if (fsty == FS_FAT32) {
			sys = 0x0C;		/* FAT32X */
		} else {
			if (sz_vol >= 0x10000) {
				sys = 0x06;	/* FAT12/16 (large) */
			} else {
				sys = (fsty == FS_FAT16) ? 0x04 : 0x01;	/* FAT16 : FAT12 */
			}
		}
	}

	/* Update partition information */
	if (FF_MULTI_PARTITION && ipart != 0) {	/* Volume is in the existing partition */
		if (!FF_LBA64 || !(fsopt & 0x80)) {
			/* Update system ID in the partition table */
			if (m_diskio->read(pdrv, buf, 0, 1) != DiskioHardwareBase::DRESULT::RES_OK) LEAVE_MKFS(FR_DISK_ERR);	/* Read the MBR */
			buf[MBR_Table + (ipart - 1) * SZ_PTE + PTE_System] = sys;			/* Set system ID */
			if (m_diskio->write(pdrv, buf, 0, 1) != DiskioHardwareBase::DRESULT::RES_OK) LEAVE_MKFS(FR_DISK_ERR);	/* Write it back to the MBR */
		}
	} else {								/* Volume as a new single partition */
		if (!(fsopt & FM_SFD)) {			/* Create partition table if not in SFD */
			lba[0] = sz_vol; lba[1] = 0;
			fr = create_partition(pdrv, lba, sys, buf);
			if (fr != FR_OK) LEAVE_MKFS(fr);
		}
	}

	if (m_diskio->ioctl(pdrv, DiskioHardwareBase::CTRL_SYNC, 0) != DiskioHardwareBase::DRESULT::RES_OK) LEAVE_MKFS(FR_DISK_ERR);

	LEAVE_MKFS(FR_OK);
}




#if FF_MULTI_PARTITION
/*-----------------------------------------------------------------------*/
/* Create Partition Table on the Physical Drive                          */
/*-----------------------------------------------------------------------*/
template<typename IOTYPE>
FRESULT Driver<IOTYPE>::f_fdisk (
	BYTE pdrv,			/* Physical drive number */
	const LBA_t ptbl[],	/* Pointer to the size table for each partitions */
	void* work			/* Pointer to the working buffer (null: use heap memory) */
)
{
	BYTE *buf = (BYTE*)work;
	DiskioHardwareBase::DSTATUS stat;


	stat = m_diskio->initialize(pdrv);
	if (stat & DiskioHardwareBase::STA_NOINIT) return FR_NOT_READY;
	if (stat & DiskioHardwareBase::STA_PROTECT) return FR_WRITE_PROTECTED;
#if FF_USE_LFN == 3
	if (!buf) buf = ff_memalloc(FF_MAX_SS);	/* Use heap memory for working buffer */
#endif
	if (!buf) return FR_NOT_ENOUGH_CORE;

	LEAVE_MKFS(create_partition(pdrv, ptbl, 0x07, buf));
}

#endif /* FF_MULTI_PARTITION */
#endif /* !FF_FS_READONLY && FF_USE_MKFS */




#if FF_USE_STRFUNC
#if FF_USE_LFN && FF_LFN_UNICODE && (FF_STRF_ENCODE < 0 || FF_STRF_ENCODE > 3)
#error Wrong FF_STRF_ENCODE setting
#endif
/*-----------------------------------------------------------------------*/
/* Get a String from the File                                            */
/*-----------------------------------------------------------------------*/
template<typename IOTYPE>
TCHAR* Driver<IOTYPE>::f_gets (
	TCHAR* buff,	/* Pointer to the buffer to store read string */
	int len,		/* Size of string buffer (items) */
	FIL* fp			/* Pointer to the file object */
)
{
	int nc = 0;
	TCHAR *p = buff;
	BYTE s[4];
	UINT rc;
	DWORD dc;
#if FF_USE_LFN && FF_LFN_UNICODE && FF_STRF_ENCODE <= 2
	WCHAR wc;
#endif
#if FF_USE_LFN && FF_LFN_UNICODE && FF_STRF_ENCODE == 3
	UINT ct;
#endif

#if FF_USE_LFN && FF_LFN_UNICODE			/* With code conversion (Unicode API) */
	/* Make a room for the character and terminator  */
	if (FF_LFN_UNICODE == 1) len -= (FF_STRF_ENCODE == 0) ? 1 : 2;
	if (FF_LFN_UNICODE == 2) len -= (FF_STRF_ENCODE == 0) ? 3 : 4;
	if (FF_LFN_UNICODE == 3) len -= 1;
	while (nc < len) {
#if FF_STRF_ENCODE == 0				/* Read a character in ANSI/OEM */
		f_read(fp, s, 1, &rc);		/* Get a code unit */
		if (rc != 1) break;			/* EOF? */
		wc = s[0];
		if (dbc_1st((BYTE)wc)) {	/* DBC 1st byte? */
			f_read(fp, s, 1, &rc);	/* Get 2nd byte */
			if (rc != 1 || !dbc_2nd(s[0])) continue;	/* Wrong code? */
			wc = wc << 8 | s[0];
		}
		dc = ff_oem2uni(wc, CODEPAGE);	/* Convert ANSI/OEM into Unicode */
		if (dc == 0) continue;		/* Conversion error? */
#elif FF_STRF_ENCODE == 1 || FF_STRF_ENCODE == 2 	/* Read a character in UTF-16LE/BE */
		f_read(fp, s, 2, &rc);		/* Get a code unit */
		if (rc != 2) break;			/* EOF? */
		dc = (FF_STRF_ENCODE == 1) ? ld_word(s) : s[0] << 8 | s[1];
		if (IsSurrogateL(dc)) continue;	/* Broken surrogate pair? */
		if (IsSurrogateH(dc)) {		/* High surrogate? */
			f_read(fp, s, 2, &rc);	/* Get low surrogate */
			if (rc != 2) break;		/* EOF? */
			wc = (FF_STRF_ENCODE == 1) ? ld_word(s) : s[0] << 8 | s[1];
			if (!IsSurrogateL(wc)) continue;	/* Broken surrogate pair? */
			dc = ((dc & 0x3FF) + 0x40) << 10 | (wc & 0x3FF);	/* Merge surrogate pair */
		}
#else	/* Read a character in UTF-8 */
		f_read(fp, s, 1, &rc);		/* Get a code unit */
		if (rc != 1) break;			/* EOF? */
		dc = s[0];
		if (dc >= 0x80) {			/* Multi-byte sequence? */
			ct = 0;
			if ((dc & 0xE0) == 0xC0) { dc &= 0x1F; ct = 1; }	/* 2-byte sequence? */
			if ((dc & 0xF0) == 0xE0) { dc &= 0x0F; ct = 2; }	/* 3-byte sequence? */
			if ((dc & 0xF8) == 0xF0) { dc &= 0x07; ct = 3; }	/* 4-byte sequence? */
			if (ct == 0) continue;
			f_read(fp, s, ct, &rc);	/* Get trailing bytes */
			if (rc != ct) break;
			rc = 0;
			do {	/* Merge the byte sequence */
				if ((s[rc] & 0xC0) != 0x80) break;
				dc = dc << 6 | (s[rc] & 0x3F);
			} while (++rc < ct);
			if (rc != ct || dc < 0x80 || IsSurrogate(dc) || dc >= 0x110000) continue;	/* Wrong encoding? */
		}
#endif
		/* A code point is avaialble in dc to be output */

		if (FF_USE_STRFUNC == 2 && dc == '\r') continue;	/* Strip \r off if needed */
#if FF_LFN_UNICODE == 1	|| FF_LFN_UNICODE == 3	/* Output it in UTF-16/32 encoding */
		if (FF_LFN_UNICODE == 1 && dc >= 0x10000) {	/* Out of BMP at UTF-16? */
			*p++ = (TCHAR)(0xD800 | ((dc >> 10) - 0x40)); nc++;	/* Make and output high surrogate */
			dc = 0xDC00 | (dc & 0x3FF);		/* Make low surrogate */
		}
		*p++ = (TCHAR)dc; nc++;
		if (dc == '\n') break;	/* End of line? */
#elif FF_LFN_UNICODE == 2		/* Output it in UTF-8 encoding */
		if (dc < 0x80) {	/* Single byte? */
			*p++ = (TCHAR)dc;
			nc++;
			if (dc == '\n') break;	/* End of line? */
		} else {
			if (dc < 0x800) {		/* 2-byte sequence? */
				*p++ = (TCHAR)(0xC0 | (dc >> 6 & 0x1F));
				*p++ = (TCHAR)(0x80 | (dc >> 0 & 0x3F));
				nc += 2;
			} else {
				if (dc < 0x10000) {	/* 3-byte sequence? */
					*p++ = (TCHAR)(0xE0 | (dc >> 12 & 0x0F));
					*p++ = (TCHAR)(0x80 | (dc >> 6 & 0x3F));
					*p++ = (TCHAR)(0x80 | (dc >> 0 & 0x3F));
					nc += 3;
				} else {			/* 4-byte sequence? */
					*p++ = (TCHAR)(0xF0 | (dc >> 18 & 0x07));
					*p++ = (TCHAR)(0x80 | (dc >> 12 & 0x3F));
					*p++ = (TCHAR)(0x80 | (dc >> 6 & 0x3F));
					*p++ = (TCHAR)(0x80 | (dc >> 0 & 0x3F));
					nc += 4;
				}
			}
		}
#endif
	}

#else			/* Byte-by-byte read without any conversion (ANSI/OEM API) */
	len -= 1;	/* Make a room for the terminator */
	while (nc < len) {
		f_read(fp, s, 1, &rc);	/* Get a byte */
		if (rc != 1) break;		/* EOF? */
		dc = s[0];
		if (FF_USE_STRFUNC == 2 && dc == '\r') continue;
		*p++ = (TCHAR)dc; nc++;
		if (dc == '\n') break;
	}
#endif

	*p = 0;		/* Terminate the string */
	return nc ? buff : 0;	/* When no data read due to EOF or error, return with error. */
}




#if !FF_FS_READONLY
#include <stdarg.h>
#define SZ_PUTC_BUF	64
#define SZ_NUM_BUF	32

/*-----------------------------------------------------------------------*/
/* Put a Character to the File (with sub-functions)                      */
/*-----------------------------------------------------------------------*/

/* Output buffer and work area */

// typedef struct {
// 	FIL *fp;		/* Ptr to the writing file */
// 	int idx, nchr;	/* Write index of buf[] (-1:error), number of encoding units written */
// #if FF_USE_LFN && FF_LFN_UNICODE == 1
// 	WCHAR hs;
// #elif FF_USE_LFN && FF_LFN_UNICODE == 2
// 	BYTE bs[4];
// 	UINT wi, ct;
// #endif
// 	BYTE buf[SZ_PUTC_BUF];	/* Write buffer */
// } putbuff;


/* Buffered file write with code conversion */
template<typename IOTYPE>
void Driver<IOTYPE>::putc_bfd (putbuff* pb, TCHAR c)
{
	UINT n;
	int i, nc;
	#if FF_USE_LFN && FF_LFN_UNICODE
		WCHAR hs, wc;
		#if FF_LFN_UNICODE == 2
			DWORD dc;
			const TCHAR *tp;
		#endif
	#endif

	if (FF_USE_STRFUNC == 2 && c == '\n') {	 /* LF -> CRLF conversion */
		putc_bfd(pb, '\r');
	}

	i = pb->idx;			/* Write index of pb->buf[] */
	if (i < 0) return;		/* In write error? */
	nc = pb->nchr;			/* Write unit counter */

#if FF_USE_LFN && FF_LFN_UNICODE
#if FF_LFN_UNICODE == 1		/* UTF-16 input */
	if (IsSurrogateH(c)) {	/* High surrogate? */
		pb->hs = c; return;	/* Save it for next */
	}
	hs = pb->hs; pb->hs = 0;
	if (hs != 0) {			/* There is a leading high surrogate */
		if (!IsSurrogateL(c)) hs = 0;	/* Discard high surrogate if not a surrogate pair */
	} else {
		if (IsSurrogateL(c)) return;	/* Discard stray low surrogate */
	}
	wc = c;
#elif FF_LFN_UNICODE == 2	/* UTF-8 input */
	for (;;) {
		if (pb->ct == 0) {	/* Out of multi-byte sequence? */
			pb->bs[pb->wi = 0] = (BYTE)c;	/* Save 1st byte */
			if ((BYTE)c < 0x80) break;					/* Single byte? */
			if (((BYTE)c & 0xE0) == 0xC0) pb->ct = 1;	/* 2-byte sequence? */
			if (((BYTE)c & 0xF0) == 0xE0) pb->ct = 2;	/* 3-byte sequence? */
			if (((BYTE)c & 0xF1) == 0xF0) pb->ct = 3;	/* 4-byte sequence? */
			return;
		} else {				/* In the multi-byte sequence */
			if (((BYTE)c & 0xC0) != 0x80) {	/* Broken sequence? */
				pb->ct = 0; continue;
			}
			pb->bs[++pb->wi] = (BYTE)c;	/* Save the trailing byte */
			if (--pb->ct == 0) break;	/* End of multi-byte sequence? */
			return;
		}
	}
	tp = (const TCHAR*)pb->bs;
	dc = tchar2uni(&tp);	/* UTF-8 ==> UTF-16 */
	if (dc == 0xFFFFFFFF) return;	/* Wrong code? */
	wc = (WCHAR)dc;
	hs = (WCHAR)(dc >> 16);
#elif FF_LFN_UNICODE == 3	/* UTF-32 input */
	if (IsSurrogate(c) || c >= 0x110000) return;	/* Discard invalid code */
	if (c >= 0x10000) {		/* Out of BMP? */
		hs = (WCHAR)(0xD800 | ((c >> 10) - 0x40)); 	/* Make high surrogate */
		wc = 0xDC00 | (c & 0x3FF);					/* Make low surrogate */
	} else {
		hs = 0;
		wc = (WCHAR)c;
	}
#endif
	/* A code point in UTF-16 is available in hs and wc */

#if FF_STRF_ENCODE == 1		/* Write a code point in UTF-16LE */
	if (hs != 0) {	/* Surrogate pair? */
		st_word(&pb->buf[i], hs);
		i += 2;
		nc++;
	}
	st_word(&pb->buf[i], wc);
	i += 2;
#elif FF_STRF_ENCODE == 2	/* Write a code point in UTF-16BE */
	if (hs != 0) {	/* Surrogate pair? */
		pb->buf[i++] = (BYTE)(hs >> 8);
		pb->buf[i++] = (BYTE)hs;
		nc++;
	}
	pb->buf[i++] = (BYTE)(wc >> 8);
	pb->buf[i++] = (BYTE)wc;
#elif FF_STRF_ENCODE == 3	/* Write a code point in UTF-8 */
	if (hs != 0) {	/* 4-byte sequence? */
		nc += 3;
		hs = (hs & 0x3FF) + 0x40;
		pb->buf[i++] = (BYTE)(0xF0 | hs >> 8);
		pb->buf[i++] = (BYTE)(0x80 | (hs >> 2 & 0x3F));
		pb->buf[i++] = (BYTE)(0x80 | (hs & 3) << 4 | (wc >> 6 & 0x0F));
		pb->buf[i++] = (BYTE)(0x80 | (wc & 0x3F));
	} else {
		if (wc < 0x80) {	/* Single byte? */
			pb->buf[i++] = (BYTE)wc;
		} else {
			if (wc < 0x800) {	/* 2-byte sequence? */
				nc += 1;
				pb->buf[i++] = (BYTE)(0xC0 | wc >> 6);
			} else {			/* 3-byte sequence */
				nc += 2;
				pb->buf[i++] = (BYTE)(0xE0 | wc >> 12);
				pb->buf[i++] = (BYTE)(0x80 | (wc >> 6 & 0x3F));
			}
			pb->buf[i++] = (BYTE)(0x80 | (wc & 0x3F));
		}
	}
#else						/* Write a code point in ANSI/OEM */
	if (hs != 0) return;
	wc = ff_uni2oem(wc, CODEPAGE);	/* UTF-16 ==> ANSI/OEM */
	if (wc == 0) return;
	if (wc >= 0x100) {
		pb->buf[i++] = (BYTE)(wc >> 8); nc++;
	}
	pb->buf[i++] = (BYTE)wc;
#endif

#else							/* ANSI/OEM input (without re-encoding) */
	pb->buf[i++] = (BYTE)c;
#endif

	if (i >= (int)(sizeof pb->buf) - 4) {	/* Write buffered characters to the file */
		f_write(pb->fp, pb->buf, (UINT)i, &n);
		i = (n == (UINT)i) ? 0 : -1;
	}
	pb->idx = i;
	pb->nchr = nc + 1;
}


/* Flush remaining characters in the buffer */
template<typename IOTYPE>
int Driver<IOTYPE>::putc_flush (putbuff* pb)
{
	UINT nw;

	if (   pb->idx >= 0	/* Flush buffered characters to the file */
		&& f_write(pb->fp, pb->buf, (UINT)pb->idx, &nw) == FR_OK
		&& (UINT)pb->idx == nw) return pb->nchr;
	return -1;
}


/* Initialize write buffer */
template<typename IOTYPE>
void Driver<IOTYPE>::putc_init (putbuff* pb, FIL* fp)
{
	std::memset(pb, 0, sizeof (putbuff));
	pb->fp = fp;
}


template<typename IOTYPE>
int Driver<IOTYPE>::f_putc (
	TCHAR c,	/* A character to be output */
	FIL* fp		/* Pointer to the file object */
)
{
	putbuff pb;


	putc_init(&pb, fp);
	putc_bfd(&pb, c);	/* Put the character */
	return putc_flush(&pb);
}




/*-----------------------------------------------------------------------*/
/* Put a String to the File                                              */
/*-----------------------------------------------------------------------*/
template<typename IOTYPE>
int Driver<IOTYPE>::f_puts (
	const TCHAR* str,	/* Pointer to the string to be output */
	FIL* fp				/* Pointer to the file object */
)
{
	putbuff pb;


	putc_init(&pb, fp);
	while (*str) putc_bfd(&pb, *str++);		/* Put the string */
	return putc_flush(&pb);
}




/*-----------------------------------------------------------------------*/
/* Put a Formatted String to the File (with sub-functions)               */
/*-----------------------------------------------------------------------*/
#if FF_PRINT_FLOAT && FF_INTDEF == 2

template<typename IOTYPE>
int Driver<IOTYPE>::ilog10 (double n)	/* Calculate log10(n) in integer output */
{
	int rv = 0;

	while (n >= 10) {	/* Decimate digit in right shift */
		if (n >= 100000) {
			n /= 100000; rv += 5;
		} else {
			n /= 10; rv++;
		}
	}
	while (n < 1) {		/* Decimate digit in left shift */
		if (n < 0.00001) {
			n *= 100000; rv -= 5;
		} else {
			n *= 10; rv--;
		}
	}
	return rv;
}

template<typename IOTYPE>
double Driver<IOTYPE>::i10x (int n)	/* Calculate 10^n in integer input */
{
	double rv = 1;

	while (n > 0) {		/* Left shift */
		if (n >= 5) {
			rv *= 100000; n -= 5;
		} else {
			rv *= 10; n--;
		}
	}
	while (n < 0) {		/* Right shift */
		if (n <= -5) {
			rv /= 100000; n += 5;
		} else {
			rv /= 10; n++;
		}
	}
	return rv;
}

// https://isocpp.org/wiki/faq/newbie#floating-point-arith
template<typename IOTYPE>
bool Driver<IOTYPE>::isEqual(double x, double y)
{
  return std::abs(x - y) <= std::numeric_limits<double>::epsilon() * std::abs(x);
  // see Knuth section 4.2.2 pages 217-218
}

template<typename IOTYPE>
void Driver<IOTYPE>::ftoa (
	char* buf,	/* Buffer to output the floating point string */
	double val,	/* Value to output */
	int prec,	/* Number of fractional digits */
	TCHAR fmt	/* Notation */
)
{
	int d;
	int e = 0, m = 0;
	char sign = 0;
	double w;
	const char *er = 0;
	const char ds = FF_PRINT_FLOAT == 2 ? ',' : '.';


	if (std::isnan(val)) {			/* Not a number? */
		er = "NaN";
	} else {
		if (prec < 0) prec = 6;	/* Default precision? (6 fractional digits) */
		if (val < 0) {			/* Nagative? */
			val = 0 - val; sign = '-';
		} else {
			sign = '+';
		}
		if (std::isinf(val)) {		/* Infinite? */
			er = "INF";
		} else {
			if (fmt == 'f') {	/* Decimal notation? */
				val += i10x(0 - prec) / 2;	/* Round (nearest) */
				m = ilog10(val);
				if (m < 0) m = 0;
				if (m + prec + 3 >= SZ_NUM_BUF) er = "OV";	/* Buffer overflow? */
			} else {			/* E notation */
				if (!isEqual(val, 0)) {		/* Not a true zero? */
					val += i10x(ilog10(val) - prec) / 2;	/* Round (nearest) */
					e = ilog10(val);
					if (e > 99 || prec + 7 >= SZ_NUM_BUF) {	/* Buffer overflow or E > +99? */
						er = "OV";
					} else {
						if (e < -99) e = -99;
						val /= i10x(e);	/* Normalize */
					}
				}
			}
		}
		if (!er) {	/* Not error condition */
			if (sign == '-') *buf++ = sign;	/* Add a - if negative value */
			do {				/* Put decimal number */
				if (m == -1) *buf++ = ds;	/* Insert a decimal separator when get into fractional part */
				w = i10x(m);				/* Snip the highest digit d */
				d = (int)(val / w); val -= d * w;
				*buf++ = (char)('0' + d);	/* Put the digit */
			} while (--m >= -prec);			/* Output all digits specified by prec */
			if (fmt != 'f') {	/* Put exponent if needed */
				*buf++ = (char)fmt;
				if (e < 0) {
					e = 0 - e; *buf++ = '-';
				} else {
					*buf++ = '+';
				}
				*buf++ = (char)('0' + e / 10);
				*buf++ = (char)('0' + e % 10);
			}
		}
	}
	if (er) {	/* Error condition */
		if (sign) *buf++ = sign;		/* Add sign if needed */
		do *buf++ = *er++; while (*er);	/* Put error symbol */
	}
	*buf = 0;	/* Term */
}
#endif	/* FF_PRINT_FLOAT && FF_INTDEF == 2 */


template<typename IOTYPE>
int Driver<IOTYPE>::f_printf (
	FIL* fp,			/* Pointer to the file object */
	const TCHAR* fmt,	/* Pointer to the format string */
	...					/* Optional arguments... */
)
{
	va_list arp;
	putbuff pb;
	UINT i, j, w, f, r;
	int prec;
#if FF_PRINT_LLI && FF_INTDEF == 2
	QWORD v;
#else
	DWORD v;
#endif
	TCHAR tc, pad, *tp;
	TCHAR nul = 0;
	char d, str[SZ_NUM_BUF];


	putc_init(&pb, fp);

	va_start(arp, fmt);

	for (;;) {
		tc = *fmt++;
		if (tc == 0) break;			/* End of format string */
		if (tc != '%') {			/* Not an escape character (pass-through) */
			putc_bfd(&pb, tc);
			continue;
		}
		f = w = 0; pad = ' '; prec = -1;	/* Initialize parms */
		tc = *fmt++;
		if (tc == '0') {			/* Flag: '0' padded */
			pad = '0'; tc = *fmt++;
		} else if (tc == '-') {		/* Flag: Left aligned */
			f = 2; tc = *fmt++;
		}
		if (tc == '*') {			/* Minimum width from an argument */
			w = va_arg(arp, int);
			tc = *fmt++;
		} else {
			while (std::isdigit(tc)) {	/* Minimum width */
				w = w * 10 + tc - '0';
				tc = *fmt++;
			}
		}
		if (tc == '.') {			/* Precision */
			tc = *fmt++;
			if (tc == '*') {		/* Precision from an argument */
				prec = va_arg(arp, int);
				tc = *fmt++;
			} else {
				prec = 0;
				while (std::isdigit(tc)) {	/* Precision */
					prec = prec * 10 + tc - '0';
					tc = *fmt++;
				}
			}
		}
		if (tc == 'l') {			/* Size: long int */
			f |= 4; tc = *fmt++;
#if FF_PRINT_LLI && FF_INTDEF == 2
			if (tc == 'l') {		/* Size: long long int */
				f |= 8; tc = *fmt++;
			}
#endif
		}
		if (tc == 0) break;			/* End of format string */
		switch (tc) {				/* Atgument type is... */
		case 'b':					/* Unsigned binary */
			r = 2; break;
		case 'o':					/* Unsigned octal */
			r = 8; break;
		case 'd':					/* Signed decimal */
		case 'u':					/* Unsigned decimal */
			r = 10; break;
		case 'x':					/* Unsigned hexdecimal (lower case) */
		case 'X':					/* Unsigned hexdecimal (upper case) */
			r = 16; break;
		case 'c':					/* Character */
			putc_bfd(&pb, (TCHAR)va_arg(arp, int));
			continue;
		case 's':					/* String */
			tp = va_arg(arp, TCHAR*);	/* Get a pointer argument */
			if (!tp) tp = &nul;		/* Null ptr generates a null string */
			for (j = 0; tp[j]; j++) ;	/* j = tcslen(tp) */
			if (prec >= 0 && j > (UINT)prec) j = prec;	/* Limited length of string body */
			for ( ; !(f & 2) && j < w; j++) putc_bfd(&pb, pad);	/* Left pads */
			while (*tp && prec--) putc_bfd(&pb, *tp++);	/* Body */
			while (j++ < w) putc_bfd(&pb, ' ');			/* Right pads */
			continue;
#if FF_PRINT_FLOAT && FF_INTDEF == 2
		case 'f':					/* Floating point (decimal) */
		case 'e':					/* Floating point (e) */
		case 'E':					/* Floating point (E) */
			ftoa(str, va_arg(arp, double), prec, tc);	/* Make a flaoting point string */
			for (j = strlen(str); !(f & 2) && j < w; j++) putc_bfd(&pb, pad);	/* Left pads */
			for (i = 0; str[i]; putc_bfd(&pb, str[i++])) ;	/* Body */
			while (j++ < w) putc_bfd(&pb, ' ');	/* Right pads */
			continue;
#endif
		default:					/* Unknown type (pass-through) */
			putc_bfd(&pb, tc); continue;
		}

		/* Get an integer argument and put it in numeral */
#if FF_PRINT_LLI && FF_INTDEF == 2
		if (f & 8) {	/* long long argument? */
			v = (QWORD)va_arg(arp, long long);
		} else {
			if (f & 4) {	/* long argument? */
				v = (tc == 'd') ? (QWORD)(long long)va_arg(arp, long) : (QWORD)va_arg(arp, unsigned long);
			} else {		/* int/short/char argument */
				v = (tc == 'd') ? (QWORD)(long long)va_arg(arp, int) : (QWORD)va_arg(arp, unsigned int);
			}
		}
		if (tc == 'd' && (v & 0x8000000000000000)) {	/* Negative value? */
			v = 0 - v; f |= 1;
		}
#else
		if (f & 4) {	/* long argument? */
			v = (DWORD)va_arg(arp, long);
		} else {		/* int/short/char argument */
			v = (tc == 'd') ? (DWORD)(long)va_arg(arp, int) : (DWORD)va_arg(arp, unsigned int);
		}
		if (tc == 'd' && (v & 0x80000000)) {	/* Negative value? */
			v = 0 - v; f |= 1;
		}
#endif
		i = 0;
		do {	/* Make an integer number string */
			d = (char)(v % r); v /= r;
			if (d > 9) d += (tc == 'x') ? 0x27 : 0x07;
			str[i++] = d + '0';
		} while (v && i < SZ_NUM_BUF);
		if (f & 1) str[i++] = '-';	/* Sign */
		/* Write it */
		for (j = i; !(f & 2) && j < w; j++) putc_bfd(&pb, pad);	/* Left pads */
		do putc_bfd(&pb, (TCHAR)str[--i]); while (i);	/* Body */
		while (j++ < w) putc_bfd(&pb, ' ');		/* Right pads */
	}

	va_end(arp);

	return putc_flush(&pb);
}

#endif /* !FF_FS_READONLY */
#endif /* FF_USE_STRFUNC */



#if FF_CODE_PAGE == 0
/*-----------------------------------------------------------------------*/
/* Set Active Codepage for the Path Name                                 */
/*-----------------------------------------------------------------------*/
template<typename IOTYPE>
FRESULT Driver<IOTYPE>::f_setcp (
	WORD cp		/* Value to be set as active code page */
)
{
	static const WORD       validcp[22] = {  437,   720,   737,   771,   775,   850,   852,   855,   857,   860,   861,   862,   863,   864,   865,   866,   869,   932,   936,   949,   950, 0};
	static const BYTE* const tables[22] = {Ct437, Ct720, Ct737, Ct771, Ct775, Ct850, Ct852, Ct855, Ct857, Ct860, Ct861, Ct862, Ct863, Ct864, Ct865, Ct866, Ct869, Dc932, Dc936, Dc949, Dc950, 0};
	UINT i;


	for (i = 0; validcp[i] != 0 && validcp[i] != cp; i++) ;	/* Find the code page */
	if (validcp[i] != cp) return FR_INVALID_PARAMETER;	/* Not found? */

	CodePage = cp;
	if (cp >= 900) {	/* DBCS */
		ExCvt = 0;
		DbcTbl = tables[i];
	} else {			/* SBCS */
		ExCvt = tables[i];
		DbcTbl = 0;
	}
	return FR_OK;
}
#endif	/* FF_CODE_PAGE == 0 */



} // namespace fatfs


#endif // __FF_DRIVER_PUBLIC_IMPL_HPP__