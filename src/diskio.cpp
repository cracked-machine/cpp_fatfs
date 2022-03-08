// MIT License

// Copyright (c) 2DRESULT::RES_OK21 Chris Sutton

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

#include <diskio.hpp>


namespace fatfs {

Diskio::DSTATUS Diskio::disk_initialize(BYTE pdrv [[maybe_unused]]) 
{
    DSTATUS res = 0;
    return res;
}

Diskio::DSTATUS Diskio::disk_status(BYTE pdrv [[maybe_unused]]) 
{
    DSTATUS res = 0;
    return res;
}

Diskio::DRESULT Diskio::disk_read(BYTE pdrv [[maybe_unused]], BYTE* buff [[maybe_unused]], LBA_t sector [[maybe_unused]], UINT count [[maybe_unused]]) 
{
    return DRESULT::RES_OK;
}

Diskio::DRESULT Diskio::disk_write(BYTE pdrv [[maybe_unused]], const BYTE* buff [[maybe_unused]], LBA_t sector [[maybe_unused]], UINT count [[maybe_unused]]) 
{
    return DRESULT::RES_OK;
}

Diskio::DRESULT Diskio::disk_ioctl (BYTE pdrv [[maybe_unused]], BYTE cmd [[maybe_unused]], void *buff [[maybe_unused]])
{
    return DRESULT::RES_OK;
}


} // namespace fatfs
