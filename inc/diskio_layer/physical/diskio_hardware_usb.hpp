
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

#ifndef __DISKIO_HARDWARE_USB_HPP__
#define __DISKIO_HARDWARE_USB_HPP__

#include <diskio_hardware_base.hpp>
#include <diskio_protocol_usb.hpp>

namespace fatfs {

template<typename DISKIO_PROTOCOL>
class DiskioHardwareUSB : public DiskioHardwareBase
{
public:
    /// @brief Construct a new Diskio Hardware U S B object
    /// @param protocol 
    DiskioHardwareUSB(DISKIO_PROTOCOL &protocol);

    /// @brief 
    void periph_init();

    /// @brief 
    /// @param pdrv 
    /// @return DSTATUS 
    virtual DSTATUS initialize(BYTE pdrv) override;

    /// @brief 
    /// @param pdrv 
    /// @return DSTATUS 
    virtual DSTATUS status(BYTE pdrv) override;

    /// @brief 
    /// @param pdrv 
    /// @param buff 
    /// @param sector 
    /// @param count 
    /// @return DRESULT 
    virtual DRESULT read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count) override;

    /// @brief 
    /// @param pdrv 
    /// @param buff 
    /// @param sector 
    /// @param count 
    /// @return DRESULT 
    virtual DRESULT write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count) override;

    /// @brief 
    /// @param pdrv 
    /// @param cmd 
    /// @param buff 
    /// @return DRESULT 
    virtual DRESULT ioctl (BYTE pdrv, BYTE cmd [[maybe_unused]], void *buff [[maybe_unused]]) override;

private:
    DISKIO_PROTOCOL m_protocol;
};

/// @brief Template c'tor for initialising the DiskioHardwareUSB "DISKIO_PROTOCOL m_periph" member. 
/// Note, if you explicitly specialize the *entire* DiskioHardwareUSB class, this c'tor won't be called.
/// Instead you should explicitly specialize the DiskioHardwareUSB *member functions only*. 
/// Protocol specific functions should go in diskio_protocol_usb.hpp class
/// @tparam DISKIO_PROTOCOL The type to initialize
/// @param protocol Reference to the object we intialize "DISKIO_PROTOCOL m_periph" with
template<typename DISKIO_PROTOCOL>
DiskioHardwareUSB<DISKIO_PROTOCOL>::DiskioHardwareUSB(DISKIO_PROTOCOL &protocol)
:
    m_protocol(protocol)
{

}

// using DiskIO_USB = fatfs::DiskioHardwareUSB<fatfs::DiskioProtocolUSB>;

} // namespace fatfs 

#endif // __DISKIO_HARDWARE_USB_HPP__