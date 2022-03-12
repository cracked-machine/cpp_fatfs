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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wvolatile" 
#include <stm32g0xx_ll_gpio.h>
#pragma GCC diagnostic pop  // ignored "-Wvolatile"

namespace fatfs {

#if defined(ENABLE_MMC_SPI)


template<>
DiskioHardwareBase::DSTATUS DiskioHardwareMMC<DiskioProtocolSPI>::initialize(BYTE pdrv [[maybe_unused]]) 
{
    // get STM32 SPI peripheral ready
    if (!m_protocol.setup_spi())
    {
        // SPI config error!
        while(true)
        {}        
    }

    DSTATUS res = 0;

	// BYTE n, cmd, ty, ocr[4];

    // drive #0 only 
	if (pdrv != 0) return STA_NOINIT;		
	
    // Is card inserted?
	if (Stat & STA_NODISK) return Stat;	

    // send some bytes to trigger 74+ clock pulses from SCLK between 100-400KHz
	// stm32::spi::set_prescaler(m_protocol.get_spi_handle(), (SPI_CR1_BR_2 | SPI_CR1_BR_1));
    // set CS high
    m_protocol.set_cs_high();
    for (uint8_t n = 10; n; n--) 
    {
        // send a byte
        stm32::spi::transmit_byte(m_protocol.get_spi_handle(), 0xF0);
        // check the data has left the SPI FIFO before sending the next
        while (!stm32::spi::wait_for_txe_flag(m_protocol.get_spi_handle(), 10));
        while (!stm32::spi::wait_for_bsy_flag(m_protocol.get_spi_handle(), 10));
        
    }
    // set CS low
    m_protocol.set_cs_low();

    return res;
}

template<>
DiskioHardwareBase::DSTATUS DiskioHardwareMMC<DiskioProtocolSPI>::status(BYTE pdrv [[maybe_unused]]) 
{
    // get STM32 SPI peripheral ready
    m_protocol.setup_spi();
    DSTATUS res = 0;
    return res;
}

template<>
DiskioHardwareBase::DRESULT DiskioHardwareMMC<DiskioProtocolSPI>::read(
    BYTE pdrv [[maybe_unused]], 
    BYTE* buff [[maybe_unused]], 
    LBA_t sector [[maybe_unused]], 
    UINT count [[maybe_unused]]) 
{
    // get STM32 SPI peripheral ready
    m_protocol.setup_spi();    
    return DRESULT::RES_OK;
}

template<>
DiskioHardwareBase::DRESULT DiskioHardwareMMC<DiskioProtocolSPI>::write(
    BYTE pdrv [[maybe_unused]], 
    const BYTE* buff [[maybe_unused]], 
    LBA_t sector [[maybe_unused]], 
    UINT count [[maybe_unused]]) 
{
    // get STM32 SPI peripheral ready
    m_protocol.setup_spi();    
    return DRESULT::RES_OK;
}

template<>
DiskioHardwareBase::DRESULT DiskioHardwareMMC<DiskioProtocolSPI>::ioctl (
    BYTE pdrv [[maybe_unused]], 
    BYTE cmd [[maybe_unused]], 
    void *buff [[maybe_unused]])
{
    // get STM32 SPI peripheral ready
    m_protocol.setup_spi();    
    return DRESULT::RES_OK;
}

#endif // #if defined(ENABLE_MMC_SPI)

} // namespace fatfs 
