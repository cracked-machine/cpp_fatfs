
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

#ifndef __DISKIO_HARDWARE_MMC_HPP__
#define __DISKIO_HARDWARE_MMC_HPP__

#include <diskio_hardware_base.hpp>
#include <diskio_protocol_spi.hpp>
#include <diskio_protocol_sdio.hpp>




namespace fatfs {

/// @brief Template class for all low-level MMC Disk IO implementations
/// @tparam DISKIO_PROTOCOL 
template<typename DISKIO_PROTOCOL>
class DiskioHardwareMMC : public DiskioHardwareBase
{
public:

    // MMC/SD command
    const BYTE CMD0	    = 0;		    // GO_IDLE_STATE
    const BYTE CMD1	    = 1;		    // SEND_OP_COND (MMC)
    const BYTE ACMD41	= 0x80+41;	    // SEND_OP_COND (SDC)
    const BYTE CMD8	    = 8;		    // SEND_IF_COND
    const BYTE CMD9	    = 9;		    // SEND_CSD
    const BYTE CMD10	= 10;		    // SEND_CID
    const BYTE CMD12	= 12;		    // STOP_TRANSMISSION
    const BYTE ACMD13   = 0x80+13;	    // SD_STATUS (SDC)
    const BYTE CMD16	= 16;		    // SET_BLOCKLEN
    const BYTE CMD17	= 17;		    // READ_SINGLE_BLOCK
    const BYTE CMD18	= 18;		    // READ_MULTIPLE_BLOCK
    const BYTE CMD23	= 23;		    // SET_BLOCK_COUNT  = MMC)
    const BYTE ACMD23	= 0x80+23;	    // SET_WR_BLK_ERASE_COUNT  = SDC)
    const BYTE CMD24	= 24;		    // WRITE_BLOCK
    const BYTE CMD25	= 25;		    // WRITE_MULTIPLE_BLOCK
    const BYTE CMD32	= 32;		    // ERASE_ER_BLK_START
    const BYTE CMD33	= 33;		    // ERASE_ER_BLK_END
    const BYTE CMD38	= 38;		    // ERASE
    const BYTE CMD55	= 55;		    // APP_CMD
    const BYTE CMD58	= 58;		    // READ_OCR

    // MMC card type flags (MMC_GET_TYPE)
    const BYTE CT_MMC	= 0x01;		    // MMC ver 3
    const BYTE CT_SD1	= 0x02;		    // SD ver 1
    const BYTE CT_SD2	= 0x04;		    // SD ver 2
    const BYTE CT_SDC	= CT_SD1|CT_SD2;// SD
    const BYTE CT_BLOCK	= 0x08;		    // Block addressing

    const BYTE R0       = 0x00;         
    const BYTE R1       = 0x01;

    /// @brief Construct a new Diskio Hardware M M C object
    /// @param periph_interface 
    explicit DiskioHardwareMMC(const DISKIO_PROTOCOL &periph_interface);

    /// @brief 
    /// @param pdrv 
    /// @return DSTATUS 
    DSTATUS initialize(BYTE pdrv) override;

    /// @brief 
    /// @param pdrv 
    /// @return DSTATUS 
    DSTATUS status(BYTE pdrv) override;

    /// @brief 
    /// @param pdrv 
    /// @param buff 
    /// @param sector 
    /// @param count 
    /// @return DRESULT 
    DRESULT read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count) override;

    /// @brief 
    /// @param pdrv 
    /// @param buff 
    /// @param sector 
    /// @param count 
    /// @return DRESULT 
    DRESULT write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count) override;

    /// @brief 
    /// @param pdrv 
    /// @param cmd 
    /// @param buff 
    /// @return DRESULT 
    DRESULT ioctl (BYTE pdrv, BYTE cmd [[maybe_unused]], void *buff [[maybe_unused]]) override;

private:
    DISKIO_PROTOCOL m_protocol;
};

/// @brief Template c'tor for generic initialization of the DiskioHardwareMMC "DISKIO_PROTOCOL m_periph_interface" member. 
/// Note, if you explicitly specialize the *entire* DiskioHardwareMMC class, this c'tor won't be called.
/// Instead you should explicitly specialize the member functions only. 
/// Protocol specific functions should go in the diskio_protocol_spi/diskio_protocol_sdio class.
/// @tparam DISKIO_PROTOCOL The type to initialize
/// @param protocol_interface Reference to the object we intialize "DISKIO_PROTOCOL m_periph_interface" with
template<typename DISKIO_PROTOCOL>
USED_API DiskioHardwareMMC<DISKIO_PROTOCOL>::DiskioHardwareMMC(const DISKIO_PROTOCOL &protocol)
:
    m_protocol(protocol)
{

}

// #if defined(ENABLE_MMC_SPI)
// /// @brief alias for the SPI-specialised Diskio MMC layer
// using DiskIO_MMC_SPI = fatfs::DiskioHardwareMMC<fatfs::DiskioProtocolSPI>;
// #endif 

// #if defined(ENABLE_MMC_SDIO)
// /// @brief alias for the SDIO-specialised Diskio MMC layer
// using DiskIO_MMC_SDIO = fatfs::DiskioHardwareMMC<fatfs::DiskioProtocolSDIO>;
// #endif

} // namespace fatfs 

#endif // __DISKIO_HARDWARE_MMC_HPP__