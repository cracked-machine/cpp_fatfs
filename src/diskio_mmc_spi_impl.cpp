
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

#if defined(ENABLE_MMC_SPI)

template<>
DiskIO_MMC_SPI::DiskioHardwareMMC(DiskioProtocolSPI &spi_interface)
:
    m_periph_interface(spi_interface)
{

}

template<>
void DiskIO_MMC_SPI::periph_init()
{
    #if not defined(X86_UNIT_TESTING_ONLY)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wvolatile" 
        // Enable GPIO (SPI_MOSI)
        LL_GPIO_SetPinSpeed(m_periph_interface.get_mosi_port(), m_periph_interface.get_mosi_pin(), GPIO_OSPEEDR_OSPEED0);
        LL_GPIO_SetPinOutputType(m_periph_interface.get_mosi_port(), m_periph_interface.get_mosi_pin(), LL_GPIO_OUTPUT_PUSHPULL);
        LL_GPIO_SetPinPull(m_periph_interface.get_mosi_port(), m_periph_interface.get_mosi_pin(), LL_GPIO_PULL_DOWN);
        LL_GPIO_SetAFPin_0_7(m_periph_interface.get_mosi_port(), m_periph_interface.get_mosi_pin(), LL_GPIO_AF_1);
        LL_GPIO_SetPinMode(m_periph_interface.get_mosi_port(), m_periph_interface.get_mosi_pin(), LL_GPIO_MODE_ALTERNATE);

        // Enable GPIO (SPI_MISO)
        LL_GPIO_SetPinSpeed(m_periph_interface.get_miso_port(), m_periph_interface.get_miso_pin(), GPIO_OSPEEDR_OSPEED0);
        LL_GPIO_SetPinOutputType(m_periph_interface.get_miso_port(), m_periph_interface.get_miso_pin(), LL_GPIO_OUTPUT_PUSHPULL);
        LL_GPIO_SetPinPull(m_periph_interface.get_miso_port(), m_periph_interface.get_miso_pin(), LL_GPIO_PULL_DOWN);
        LL_GPIO_SetAFPin_0_7(m_periph_interface.get_miso_port(), m_periph_interface.get_miso_pin(), LL_GPIO_AF_1);
        LL_GPIO_SetPinMode(m_periph_interface.get_miso_port(), m_periph_interface.get_miso_pin(), LL_GPIO_MODE_ALTERNATE);


        // Enable GPIO (SPI_SCK)
        LL_GPIO_SetPinSpeed(m_periph_interface.get_sck_port(), m_periph_interface.get_sck_pin(), LL_GPIO_SPEED_FREQ_VERY_HIGH);
        LL_GPIO_SetPinOutputType(m_periph_interface.get_sck_port(), m_periph_interface.get_sck_pin(), LL_GPIO_OUTPUT_PUSHPULL);
        LL_GPIO_SetPinPull(m_periph_interface.get_sck_port(), m_periph_interface.get_sck_pin(), LL_GPIO_PULL_DOWN);
        LL_GPIO_SetAFPin_8_15(m_periph_interface.get_sck_port(), m_periph_interface.get_sck_pin(), LL_GPIO_AF_1);
        LL_GPIO_SetPinMode(m_periph_interface.get_sck_port(), m_periph_interface.get_sck_pin(), LL_GPIO_MODE_ALTERNATE);        

        m_periph_interface.get_spi_handle()->CR1 = 0;
        m_periph_interface.get_spi_handle()->CR1 |=    
            ((SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE) | (SPI_CR1_MSTR | SPI_CR1_SSI) | SPI_CR1_SSM | SPI_CR1_BR_1);

        CLEAR_BIT(m_periph_interface.get_spi_handle()->CR2, SPI_CR2_NSSP);

               

    #pragma GCC diagnostic pop  // ignored "-Wvolatile"  
    #endif // not X86_UNIT_TESTING_ONLY

}    

template<>
DiskIO_MMC_SPI::DSTATUS DiskIO_MMC_SPI::initialize(BYTE pdrv [[maybe_unused]]) 
{
    // get STM32 SPI peripheral ready
    periph_init();
    DSTATUS res = 0;
    return res;
}

template<>
DiskIO_MMC_SPI::DSTATUS DiskIO_MMC_SPI::status(BYTE pdrv [[maybe_unused]]) 
{
    // get STM32 SPI peripheral ready
    periph_init();
    DSTATUS res = 0;
    return res;
}

template<>
DiskIO_MMC_SPI::DRESULT DiskIO_MMC_SPI::read(BYTE pdrv [[maybe_unused]], BYTE* buff [[maybe_unused]], LBA_t sector [[maybe_unused]], UINT count [[maybe_unused]]) 
{
    // get STM32 SPI peripheral ready
    periph_init();    
    return DRESULT::RES_OK;
}

template<>
DiskIO_MMC_SPI::DRESULT DiskIO_MMC_SPI::write(BYTE pdrv [[maybe_unused]], const BYTE* buff [[maybe_unused]], LBA_t sector [[maybe_unused]], UINT count [[maybe_unused]]) 
{
    // get STM32 SPI peripheral ready
    periph_init();    
    return DRESULT::RES_OK;
}

template<>
DiskIO_MMC_SPI::DRESULT DiskIO_MMC_SPI::ioctl (BYTE pdrv [[maybe_unused]], BYTE cmd [[maybe_unused]], void *buff [[maybe_unused]])
{
    // get STM32 SPI peripheral ready
    periph_init();    
    return DRESULT::RES_OK;
}

#endif // #if defined(ENABLE_MMC_SPI)

} // namespace fatfs 
