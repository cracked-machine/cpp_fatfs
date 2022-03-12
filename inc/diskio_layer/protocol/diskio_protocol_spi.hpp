
// MIT License

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

#ifndef __DISKIO_PROTOCOL_SPI_HPP__
#define __DISKIO_PROTOCOL_SPI_HPP__


#if defined(X86_UNIT_TESTING_ONLY)
	// only used when unit testing on x86
    #include <mock_cmsis.hpp>
	#include <iostream>
#else
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wvolatile"
		#include <stm32g0xx_ll_gpio.h>
	#pragma GCC diagnostic pop
	#include <spi_utils.hpp>
#endif

#include <stdint.h>
#include <utility>

namespace fatfs
{

#if defined(ENABLE_MMC_SPI)	

// @brief Class containing pointers to the STM32 peripheral, 
// GPIO ports and pins numbers for use with SPI protocol.
class DiskioProtocolSPI 
{
public:
	/// @brief Construct a new Diskio object for the SPI Protocol
	/// @param spi 
	/// @param cs_gpio 
	/// @param mosi_gpio 
	/// @param miso_gpio 
	/// @param sck_gpio 
	/// @param rcc_spi_clk 
	DiskioProtocolSPI(
        SPI_TypeDef *spi, 
        std::pair<GPIO_TypeDef*, uint32_t>  cs_gpio,    
        std::pair<GPIO_TypeDef*, uint32_t>  mosi_gpio,   
        std::pair<GPIO_TypeDef*, uint32_t>  miso_gpio,    
        std::pair<GPIO_TypeDef*, uint32_t>  sck_gpio,          
        uint32_t rcc_spi_clk
    )  
	:   m_spi(spi), m_cs_gpio(cs_gpio), m_mosi_gpio(mosi_gpio), 
		m_miso_gpio(miso_gpio), m_sck_gpio(sck_gpio), m_rcc_spi_clk(rcc_spi_clk)
	{}

	SPI_TypeDef * get_spi_handle() { return m_spi; }
	std::pair<GPIO_TypeDef*, uint32_t> get_cs_gpio() { return m_cs_gpio; }
	std::pair<GPIO_TypeDef*, uint32_t> get_mosi_gpio() { return m_mosi_gpio; }
	std::pair<GPIO_TypeDef*, uint32_t> get_miso_gpio() { return m_miso_gpio; }
	std::pair<GPIO_TypeDef*, uint32_t> get_sck_gpio() { return m_sck_gpio; }
    uint32_t get_rcc_spi_clk() { return m_rcc_spi_clk; }

private:
	// @brief The SPI peripheral
	SPI_TypeDef *m_spi;
	std::pair<GPIO_TypeDef*, uint16_t>  m_cs_gpio;    
	std::pair<GPIO_TypeDef*, uint16_t>  m_mosi_gpio;   
	std::pair<GPIO_TypeDef*, uint16_t>  m_miso_gpio;    
	std::pair<GPIO_TypeDef*, uint16_t>  m_sck_gpio;
    // @brief Used to enable the SPI clock for CS, MOSI, MISO and SCK pins 
    uint32_t m_rcc_spi_clk;
};

#endif // #if defined(ENABLE_MMC_SPI)

} // namespace fatfs

#endif // __DISKIO_PROTOCOL_SPI_HPP__