
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

#ifndef __DISKIO_PROTOCOL_SDIO_HPP__
#define __DISKIO_PROTOCOL_SDIO_HPP__


#if defined(X86_UNIT_TESTING_ONLY)
	// only used when unit testing on x86
    #include <mock_cmsis.hpp>
	#include <iostream>
#else
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wvolatile"
		#include <stm32g0xx_ll_gpio.h>
	#pragma GCC diagnostic pop
#endif

#include <stdint.h>
#include <utility>

namespace fatfs
{

#if defined(ENABLE_MMC_SDIO)

// @brief contains pointer to SPI peripheral and associated GPIO ports/pins (as defined in CMSIS)
class DiskioProtocolSDIO
{
public:
    // @brief Construct a new Driver Serial Interface object
	DiskioProtocolSDIO(
        SDIO_TypeDef *sdio,    
        std::pair<GPIO_TypeDef*, uint16_t>  cmd_gpio,   
        std::pair<GPIO_TypeDef*, uint16_t>  clk_gpio,   
		std::pair<GPIO_TypeDef*, uint16_t>  d0_gpio,    
		std::pair<GPIO_TypeDef*, uint16_t>  d1_gpio,    
		std::pair<GPIO_TypeDef*, uint16_t>  d2_gpio,    
		std::pair<GPIO_TypeDef*, uint16_t>  d3_gpio,           
        uint32_t rcc_spi_clk
    )  
	:   m_sdio(sdio), 
        m_cmd_port(cmd_gpio.first), m_cmd_pin(cmd_gpio.second),     // init cmd port+pin
        m_clk_port(clk_gpio.first), m_clk_pin(clk_gpio.second),     // init clk port+pin
        m_d0_port(d0_gpio.first), m_d0_pin(d0_gpio.second),         // init d0 port+pin		
        m_d1_port(d1_gpio.first), m_d1_pin(d1_gpio.second),         // init d1 port+pin		
        m_d2_port(d2_gpio.first), m_d2_pin(d2_gpio.second),         // init d2 port+pin		
		m_d3_port(d3_gpio.first), m_d3_pin(d3_gpio.second),         // init d3 port+pin
        m_rcc_spi_clk(rcc_spi_clk)
	{
	}

	SDIO_TypeDef * get_sdio_handle() { return m_sdio; }

	GPIO_TypeDef* get_cmd_port() { return m_cmd_port; }
	uint16_t get_cmd_pin() { return m_cmd_pin; }

	GPIO_TypeDef* get_clk_port() { return m_clk_port; }
	uint16_t get_clk_pin() { return m_clk_pin; }	

	GPIO_TypeDef* get_d0_port() { return m_d0_port; }
	uint16_t get_d0_pin() { return m_d0_pin; }

	GPIO_TypeDef* get_d1_port() { return m_d1_port; }
	uint16_t get_d1_pin() { return m_d1_pin; }	

	GPIO_TypeDef* get_d2_port() { return m_d2_port; }
	uint16_t get_d2_pin() { return m_d2_pin; }

	GPIO_TypeDef* get_d3_port() { return m_d3_port; }
	uint16_t get_d3_pin() { return m_d3_pin; }	

    uint32_t get_rcc_spi_clk() { return m_rcc_spi_clk; }
private:
	// @brief The SPI peripheral
	SDIO_TypeDef *m_sdio;

	// @brief The cmd GPIO port object
	GPIO_TypeDef* m_cmd_port;
	// @brief The cmd GPIO pin
	uint16_t m_cmd_pin;

	// @brief The clk GPIO port object
	GPIO_TypeDef* m_clk_port;
	// @brief The clk GPIO pin
	uint16_t m_clk_pin;	

	// @brief The d0 GPIO port object
	GPIO_TypeDef* m_d0_port;
	// @brief The d0 GPIO pin
	uint16_t m_d0_pin;	

	// @brief The d1 GPIO port object
	GPIO_TypeDef* m_d1_port;
	// @brief The d1 GPIO pin
	uint16_t m_d1_pin;	

	// @brief The d2 GPIO port object
	GPIO_TypeDef* m_d2_port;
	// @brief The d2 GPIO pin
	uint16_t m_d2_pin;

	// @brief The d3 GPIO port object
	GPIO_TypeDef* m_d3_port;
	// @brief The d3 GPIO pin
	uint16_t m_d3_pin;	

    // @brief Used to enable the SPI clock for CS, MOSI, MISO and SCK pins 
    uint32_t m_rcc_spi_clk;
};

#endif // #if defined(ENABLE_MMC_SDIO)

} // namespace fatfs

#endif // __DISKIO_PROTOCOL_SDIO_HPP__