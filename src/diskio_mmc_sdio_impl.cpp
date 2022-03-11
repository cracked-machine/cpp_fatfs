
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

#include <diskio_hardware_mmc.hpp>


namespace fatfs {

#if defined(ENABLE_MMC_SDIO)

static const BYTE ISDIO_READ		= 55;	/* Read data form SD iSDIO register */
static const BYTE ISDIO_WRITE		= 56;	/* Write data to SD iSDIO register */
static const BYTE ISDIO_MRITE		= 57;	/* Masked write data to SD iSDIO register */

template<>
DiskIO_MMC_SDIO::DiskioHardwareMMC(DiskioProtocolSDIO &sdio_interface)
:
    m_periph_interface(sdio_interface)
{

}

template<>
void DiskIO_MMC_SDIO::periph_init()
{
    #if not defined(X86_UNIT_TESTING_ONLY)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wvolatile" 
        // Enable GPIO (SDIO CMD)

        // Enable GPIO (SDIO CLK)
        
        // Enable GPIO (SDIO D0)
        
        // Enable GPIO (SDIO D1)
        
        // Enable GPIO (SDIO D2)
        
        // Enable GPIO (SDIO D3)               

    #pragma GCC diagnostic pop  // ignored "-Wvolatile"  
    #endif // not X86_UNIT_TESTING_ONLY

}    

template<>
DiskIO_MMC_SDIO::DSTATUS DiskIO_MMC_SDIO::initialize(BYTE pdrv [[maybe_unused]]) 
{
    // get STM32 SDIO peripheral ready
    periph_init();
    DSTATUS res = 0;
    return res;
}

template<>
DiskIO_MMC_SDIO::DSTATUS DiskIO_MMC_SDIO::status(BYTE pdrv [[maybe_unused]]) 
{
    // get STM32 SDIO peripheral ready
    periph_init();
    DSTATUS res = 0;
    return res;
}

template<>
DiskIO_MMC_SDIO::DRESULT DiskIO_MMC_SDIO::read(BYTE pdrv [[maybe_unused]], BYTE* buff [[maybe_unused]], LBA_t sector [[maybe_unused]], UINT count [[maybe_unused]]) 
{
    // get STM32 SDIO peripheral ready
    periph_init();    
    return DRESULT::RES_OK;
}

template<>
DiskIO_MMC_SDIO::DRESULT DiskIO_MMC_SDIO::write(BYTE pdrv [[maybe_unused]], const BYTE* buff [[maybe_unused]], LBA_t sector [[maybe_unused]], UINT count [[maybe_unused]]) 
{
    // get STM32 SDIO peripheral ready
    periph_init();    
    return DRESULT::RES_OK;
}

template<>
DiskIO_MMC_SDIO::DRESULT DiskIO_MMC_SDIO::ioctl (BYTE pdrv [[maybe_unused]], BYTE cmd [[maybe_unused]], void *buff [[maybe_unused]])
{
    // get STM32 SDIO peripheral ready
    periph_init();    
    return DRESULT::RES_OK;
}

#endif // #if defined(ENABLE_MMC_SDIO)

} // namespace fatfs 
