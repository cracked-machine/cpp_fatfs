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

#ifndef __DISKIO_HARDWARE_BASE_HPP__
#define __DISKIO_HARDWARE_BASE_HPP__

#include <ff_types.hpp>	


namespace fatfs {


class DiskioHardwareBase {

public:

    DiskioHardwareBase() = default;

    using DSTATUS = BYTE;

    /* Results of Disk Functions */
    enum DRESULT {
        RES_OK = 0,		/* 0: Successful */
        RES_ERROR,		/* 1: R/W Error */
        RES_WRPRT,		/* 2: Write Protected */
        RES_NOTRDY,		/* 3: Not Ready */
        RES_PARERR		/* 4: Invalid Parameter */
    };

    static const BYTE STA_NOINIT        = 0x01;	/* Drive not initialized */
    static const BYTE STA_NODISK	    = 0x02;	/* No medium in the drive */
    static const BYTE STA_PROTECT       = 0x04;	/* Write protected */

    static const BYTE CTRL_SYNC			= 0;	/* Complete pending write process (needed at FF_FS_READONLY == 0) */
    static const BYTE GET_SECTOR_COUNT	= 1;	/* Get media size (needed at FF_USE_MKFS == 1) */
    static const BYTE GET_SECTOR_SIZE	= 2;	/* Get sector size (needed at FF_MAX_SS != FF_MIN_SS) */
    static const BYTE GET_BLOCK_SIZE	= 3;	/* Get erase block size (needed at FF_USE_MKFS == 1) */
    static const BYTE CTRL_TRIM			= 4;	/* Inform device that the data on the block of sectors is no longer used (needed at FF_USE_TRIM == 1) */

    static const BYTE CTRL_POWER		= 5;	/* Get/Set power status */
    static const BYTE CTRL_LOCK			= 6;	/* Lock/Unlock media removal */
    static const BYTE CTRL_EJECT		= 7;	/* Eject media */
    static const BYTE CTRL_FORMAT		= 8;	/* Create physical format on the media */

    static const BYTE MMC_GET_TYPE		= 10;	/* Get card type */
    static const BYTE MMC_GET_CSD		= 11;	/* Get CSD */
    static const BYTE MMC_GET_CID		= 12;	/* Get CID */
    static const BYTE MMC_GET_OCR	    = 13;	/* Get OCR */
    static const BYTE MMC_GET_SDSTAT	= 14;	/* Get SD status */


    /* ATA/CF specific ioctl command */
    static const BYTE ATA_GET_REV		= 20;	/* Get F/W revision */
    static const BYTE ATA_GET_MODEL		= 21;	/* Get model name */
    static const BYTE ATA_GET_SN		= 22;	/* Get serial number */
    
    virtual DSTATUS initialize(BYTE pdrv);
    virtual DSTATUS status(BYTE pdrv);
    virtual DRESULT read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count);
    virtual DRESULT write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count);
    virtual DRESULT ioctl (BYTE pdrv, BYTE cmd [[maybe_unused]], void *buff [[maybe_unused]]);
};

} // namespace fatfs


#endif // __DISKIO_HARDWARE_BASE_HPP__