
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

#ifndef __FATFS_SPI_DEVICE_HPP__
#define __FATFS_SPI_DEVICE_HPP__


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

// @brief contains pointer to SPI peripheral and associated GPIO ports/pins (as defined in CMSIS)
class DriverInterfaceSPI 
{
public:
    // @brief Construct a new Driver Serial Interface object
    // @param led_spi           The SPI peripheral e.g. SPI2
    // @param lat_port          The CS port e.g. GPIOB
    // @param lat_pin           The CS pin e.g. LL_GPIO_PIN_9
    // @param mosi_port         The MOSI port e.g. GPIOB
    // @param mosi_pin          The MOSI pin e.g. LL_GPIO_PIN_7
    // @param miso_port         The MISO port e.g. GPIOB
    // @param miso_pin          The MISO pin e.g. LL_GPIO_PIN_7    
    // @param sck_port          The serial clock port e.g. GPIOB
    // @param sck_pin           The serial clock pin e.g. LL_GPIO_PIN_8
    // @param rcc_spi_clk       The bit to enable the SPI RCC (RCC_APBENR1) for the MOSI/SCK port e.g. LL_APB1_GRP1_PERIPH_SPI2
	DriverInterfaceSPI(
        SPI_TypeDef *led_spi, 
        std::pair<GPIO_TypeDef*, uint16_t>  cs_gpio,    
        std::pair<GPIO_TypeDef*, uint16_t>  mosi_gpio,   
        std::pair<GPIO_TypeDef*, uint16_t>  miso_gpio,    
        std::pair<GPIO_TypeDef*, uint16_t>  sck_gpio,          
        uint32_t rcc_spi_clk
    )  
	:   m_spi(led_spi), 
        m_cs_port(cs_gpio.first), m_cs_pin(cs_gpio.second),           	// init cs port+pin
        m_mosi_port(mosi_gpio.first), m_mosi_pin(mosi_gpio.second),     // init mosi port+pin 
        m_miso_port(miso_gpio.first), m_miso_pin(miso_gpio.second),     // init mosi port+pin 
        m_sck_port(sck_gpio.first), m_sck_pin(sck_gpio.second),         // init sck port+pin
        m_rcc_spi_clk(rcc_spi_clk)
	{
	}

	SPI_TypeDef * get_spi_handle() { return m_spi; }
	GPIO_TypeDef* get_cs_port() { return m_cs_port; }
	uint16_t get_cs_pin() { return m_cs_pin; }
	GPIO_TypeDef* get_mosi_port() { return m_mosi_port; }
	uint16_t get_mosi_pin() { return m_mosi_pin; }
	GPIO_TypeDef* get_miso_port() { return m_miso_port; }
	uint16_t get_miso_pin() { return m_miso_pin; }    
	GPIO_TypeDef* get_sck_port() { return m_sck_port; }
	uint16_t get_sck_pin() { return m_sck_pin; }    
    uint32_t get_rcc_spi_clk() { return m_rcc_spi_clk; }
private:
	// @brief The SPI peripheral
	SPI_TypeDef *m_spi;
	// @brief The latch GPIO port object
	GPIO_TypeDef* m_cs_port;
	// @brief The latch GPIO pin
	uint16_t m_cs_pin;
	// @brief The MOSI GPIO port object
	GPIO_TypeDef* m_mosi_port;
	// @brief The MOSI GPIO pin
	uint16_t m_mosi_pin;
	// @brief The MISO GPIO port object
	GPIO_TypeDef* m_miso_port;
	// @brief The MISO GPIO pin
	uint16_t m_miso_pin;
    // @brief SPI Clock GPIO port
    GPIO_TypeDef* m_sck_port;   
    // @brief SPI Clock GPIO pin
    uint16_t m_sck_pin;    
    // @brief Used to enable the SPI clock for CS, MOSI, MISO and SCK pins 
    uint32_t m_rcc_spi_clk;
};

} // namespace tlc5955

#endif // __FATFS_SPI_DEVICE_HPP__