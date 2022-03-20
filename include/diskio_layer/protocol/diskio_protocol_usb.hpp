
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

#ifndef __DISKIO_PROTOCOL_USB_HPP__
#define __DISKIO_PROTOCOL_USB_HPP__


#if defined(X86_UNIT_TESTING_ONLY)
	// This file should contain CMSIS bit definitions
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
class DiskioProtocolUSB
{
public:
    // @brief Construct a new Driver Serial Interface object
	DiskioProtocolUSB()
	{
	}
};

} // namespace fatfs

#endif // __DISKIO_PROTOCOL_USB_HPP__