
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


#ifndef X86_UNIT_TESTING_ONLY
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wvolatile"
		#include <stm32g0xx_ll_gpio.h>
        #include <stm32g0xx_ll_spi.h>
        #include <stm32g0xx_ll_bus.h>
	#pragma GCC diagnostic pop
	

#endif

#include <spi_utils.hpp>
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
        std::pair<GPIO_TypeDef*, uint32_t>  sck_gpio,    
        std::pair<GPIO_TypeDef*, uint32_t>  mosi_gpio,   
        std::pair<GPIO_TypeDef*, uint32_t>  miso_gpio,    
        std::pair<GPIO_TypeDef*, uint32_t>  cs_gpio,          
        uint32_t rcc_spi_clk
    )  
	:   m_spi(spi), m_sck_gpio(sck_gpio), m_mosi_gpio(mosi_gpio), 
		m_miso_gpio(miso_gpio), m_cs_gpio(cs_gpio), m_rcc_spi_clk(rcc_spi_clk)
	{}
	bool setup_spi()
	{


        stm32::spi::enable_spi(m_spi, false);
#ifndef X86_UNIT_TESTING_ONLY

		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wvolatile" 

        LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA | LL_IOP_GRP1_PERIPH_GPIOB | LL_IOP_GRP1_PERIPH_GPIOD);
        
        // Enable GPIO (SPI_SCK)
        LL_GPIO_SetPinSpeed(m_sck_gpio.first, m_sck_gpio.second, LL_GPIO_SPEED_FREQ_VERY_HIGH);
        LL_GPIO_SetPinOutputType(m_sck_gpio.first, m_sck_gpio.second, LL_GPIO_OUTPUT_PUSHPULL);
        LL_GPIO_SetPinPull(m_sck_gpio.first, m_sck_gpio.second, LL_GPIO_PULL_UP);
        LL_GPIO_SetAFPin_8_15(m_sck_gpio.first, m_sck_gpio.second, LL_GPIO_AF_1);
        LL_GPIO_SetPinMode(m_sck_gpio.first, m_sck_gpio.second, LL_GPIO_MODE_ALTERNATE);  

        // Enable GPIO (SPI_MOSI)
        LL_GPIO_SetPinSpeed(m_mosi_gpio.first, m_mosi_gpio.second, GPIO_OSPEEDR_OSPEED0); // medium output speed
        LL_GPIO_SetPinOutputType(m_mosi_gpio.first, m_mosi_gpio.second, LL_GPIO_OUTPUT_PUSHPULL);
        LL_GPIO_SetPinPull(m_mosi_gpio.first, m_mosi_gpio.second, LL_GPIO_PULL_UP);
        LL_GPIO_SetAFPin_0_7(m_mosi_gpio.first, m_mosi_gpio.second, LL_GPIO_AF_1);
        LL_GPIO_SetPinMode(m_mosi_gpio.first, m_mosi_gpio.second, LL_GPIO_MODE_ALTERNATE);

        // Enable GPIO (SPI_MISO)
        LL_GPIO_SetPinSpeed(m_miso_gpio.first, m_miso_gpio.second, GPIO_OSPEEDR_OSPEED0); // medium output speed
        LL_GPIO_SetPinOutputType(m_miso_gpio.first, m_miso_gpio.second, LL_GPIO_OUTPUT_PUSHPULL);
        LL_GPIO_SetPinPull(m_miso_gpio.first, m_miso_gpio.second, LL_GPIO_PULL_UP); // must be PUP for init
        LL_GPIO_SetAFPin_0_7(m_miso_gpio.first, m_miso_gpio.second, LL_GPIO_AF_1);
        LL_GPIO_SetPinMode(m_miso_gpio.first, m_miso_gpio.second, LL_GPIO_MODE_ALTERNATE);

        // Enable GPIO (CS)
        LL_GPIO_SetPinSpeed(m_cs_gpio.first, m_cs_gpio.second, LL_GPIO_SPEED_FREQ_VERY_HIGH); // medium output speed
        LL_GPIO_SetPinOutputType(m_cs_gpio.first, m_cs_gpio.second, LL_GPIO_OUTPUT_PUSHPULL);
        LL_GPIO_SetPinPull(m_cs_gpio.first, m_cs_gpio.second, LL_GPIO_PULL_UP);
        LL_GPIO_SetPinMode(m_cs_gpio.first, m_cs_gpio.second, LL_GPIO_MODE_OUTPUT);    
        #pragma GCC diagnostic pop  // ignored "-Wvolatile" 
#endif // X86_UNIT_TESTING_ONLY
    		
		 
    
        RCC->APBENR1 = RCC->APBENR1 | m_rcc_spi_clk;
        //m_spi->CR1 |= ((SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE) | (SPI_CR1_MSTR | SPI_CR1_SSI) | SPI_CR1_SSM | SPI_CR1_BR_1);
        
        // Enable software NSS management
        m_spi->CR1 = m_spi->CR1 | SPI_CR1_SSI;
        m_spi->CR1 = m_spi->CR1 | SPI_CR1_SSM;
        m_spi->CR1 = m_spi->CR1 & ~ SPI_CR2_NSSP;

        // Set the default prescaler to 8. e.g. set the baudrate
        m_spi->CR1 = m_spi->CR1 | (SPI_CR1_BR_1);    

        // Enable SPI Master mode
        m_spi->CR1 = m_spi->CR1 | SPI_CR1_MSTR;
        
        stm32::spi::enable_spi(m_spi);
        
        // check for Mode Faults in the configuration	
        if (m_spi->SR & SPI_SR_MODF)
        {
            return false;
        }
        return true;
	}

	SPI_TypeDef * spi_handle() { return m_spi; }
	std::pair<GPIO_TypeDef*, uint32_t> cs_gpio() { return m_cs_gpio; }
	std::pair<GPIO_TypeDef*, uint32_t> mosi_gpio() { return m_mosi_gpio; }
	std::pair<GPIO_TypeDef*, uint32_t> miso_gpio() { return m_miso_gpio; }
	std::pair<GPIO_TypeDef*, uint32_t> sck_gpio() { return m_sck_gpio; }
    uint32_t rcc_spi_clk() { return m_rcc_spi_clk; }

    void set_cs_low() { m_cs_gpio.first->BRR = m_cs_gpio.second; }
    void set_cs_high() { m_cs_gpio.first->BSRR = m_cs_gpio.second; }
    void toggle_cs() 
    {   
        // read the ODR state of this GPIO port
        uint32_t odr_reg = m_cs_gpio.first->ODR;
        // reset/set the BSRR using the ODR and the pin number (second)
        m_cs_gpio.first->BSRR = m_cs_gpio.first->BSRR 
            | (((odr_reg & m_cs_gpio.second) << 16U) | (~odr_reg & m_cs_gpio.second));
    }   

private:
	// @brief The SPI peripheral
	SPI_TypeDef *m_spi;
	std::pair<GPIO_TypeDef*, uint16_t>  m_sck_gpio;    
	std::pair<GPIO_TypeDef*, uint16_t>  m_mosi_gpio;   
	std::pair<GPIO_TypeDef*, uint16_t>  m_miso_gpio;    
	std::pair<GPIO_TypeDef*, uint16_t>  m_cs_gpio;
    // @brief Used to enable the SPI clock for CS, MOSI, MISO and SCK pins 
    uint32_t m_rcc_spi_clk;
};

#endif // #if defined(ENABLE_MMC_SPI)

} // namespace fatfs

#endif // __DISKIO_PROTOCOL_SPI_HPP__