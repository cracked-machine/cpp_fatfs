
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

// C++ Port of the original source code are subject to MIT License

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

#include <diskio_base.hpp>



namespace fatfs {

DiskioBase::DSTATUS DiskioBase::initialize(BYTE pdrv [[maybe_unused]]) 
{
    DSTATUS res = 0;
    return res;
}

DiskioBase::DSTATUS DiskioBase::status(BYTE pdrv [[maybe_unused]]) 
{
    DSTATUS res = 0;
    return res;
}

DiskioBase::DRESULT DiskioBase::read(BYTE pdrv [[maybe_unused]], BYTE* buff [[maybe_unused]], LBA_t sector [[maybe_unused]], UINT count [[maybe_unused]]) 
{
    return DRESULT::RES_OK;
}

DiskioBase::DRESULT DiskioBase::write(BYTE pdrv [[maybe_unused]], const BYTE* buff [[maybe_unused]], LBA_t sector [[maybe_unused]], UINT count [[maybe_unused]]) 
{
    return DRESULT::RES_OK;
}

DiskioBase::DRESULT DiskioBase::ioctl (BYTE pdrv [[maybe_unused]], BYTE cmd [[maybe_unused]], void *buff [[maybe_unused]])
{
    return DRESULT::RES_OK;
}


} // namespace fatfs


