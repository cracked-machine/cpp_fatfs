
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

#include <catch2/catch_all.hpp>
#include <iostream>
#include <ff_driver.hpp>


TEST_CASE("Testing CPP FATFS", "[cpp_fatfs]")
{
    REQUIRE(true);
}

// enforce code coverage with explicit instances of func templates so that linker does not drop references
#if defined(ENABLE_MMC_SPI)
using DiskioMmcSpi_t = fatfs::DiskioHardwareMMC<fatfs::DiskioProtocolSPI>;
template DiskioMmcSpi_t::DiskioHardwareMMC(const DiskioProtocolSPI &protocol);
template fatfs::Driver<DiskioMmcSpi_t>::Driver(DiskioMmcSpi_t &diskio);
template fatfs::FRESULT fatfs::Driver<DiskioMmcSpi_t>::f_mount (FATFS* fs,  const TCHAR* path, BYTE opt);
template fatfs::FRESULT fatfs::Driver<DiskioMmcSpi_t>::f_open (FIL* fp,	const TCHAR* path,	BYTE mode);
template fatfs::FRESULT fatfs::Driver<DiskioMmcSpi_t>::f_read (FIL* fp, void* buff,	UINT btr, UINT* br);
template fatfs::FRESULT fatfs::Driver<DiskioMmcSpi_t>::f_write (FIL* fp, const void* buff, UINT btw, UINT* bw);
template fatfs::FRESULT fatfs::Driver<DiskioMmcSpi_t>::f_sync (FIL* fp);
template fatfs::FRESULT fatfs::Driver<DiskioMmcSpi_t>::f_close (FIL* fp);
template fatfs::FRESULT fatfs::Driver<DiskioMmcSpi_t>::f_lseek (FIL* fp, FSIZE_t ofs);
template fatfs::FRESULT fatfs::Driver<DiskioMmcSpi_t>::f_opendir (DIR* dp, const TCHAR* path);
template fatfs::FRESULT fatfs::Driver<DiskioMmcSpi_t>::f_closedir (DIR *dp);
template fatfs::FRESULT fatfs::Driver<DiskioMmcSpi_t>::f_readdir (DIR* dp, FILINFO* fno);
template fatfs::FRESULT fatfs::Driver<DiskioMmcSpi_t>::f_stat (const TCHAR* path, FILINFO* fno);
template fatfs::FRESULT fatfs::Driver<DiskioMmcSpi_t>::f_getfree (const TCHAR* path, DWORD* nclst, FATFS** fatfs);
template fatfs::FRESULT fatfs::Driver<DiskioMmcSpi_t>::f_truncate (FIL* fp);
template fatfs::FRESULT fatfs::Driver<DiskioMmcSpi_t>::f_unlink (const TCHAR* path);
template fatfs::FRESULT fatfs::Driver<DiskioMmcSpi_t>::f_mkdir (const TCHAR* path);
template fatfs::FRESULT fatfs::Driver<DiskioMmcSpi_t>::f_rename (const TCHAR* path_old,	const TCHAR* path_new);
template fatfs::FRESULT fatfs::Driver<DiskioMmcSpi_t>::sync_window (FATFS* fs);
template fatfs::FRESULT fatfs::Driver<DiskioMmcSpi_t>::validate (FFOBJID* obj,	FATFS** rfs);
template fatfs::FRESULT fatfs::Driver<DiskioMmcSpi_t>::mount_volume (const TCHAR** path, FATFS** rfs, BYTE mode);
template fatfs::UINT fatfs::Driver<DiskioMmcSpi_t>::check_fs (FATFS* fs, LBA_t sect);
template fatfs::UINT fatfs::Driver<DiskioMmcSpi_t>::find_volume (FATFS* fs, UINT part);
template fatfs::FRESULT fatfs::Driver<DiskioMmcSpi_t>::move_window (FATFS* fs, LBA_t sect);
template fatfs::FRESULT fatfs::Driver<DiskioMmcSpi_t>::sync_fs (FATFS* fs);
template fatfs::DWORD fatfs::Driver<DiskioMmcSpi_t>::get_fat (FFOBJID* obj, DWORD clst);
template fatfs::FRESULT fatfs::Driver<DiskioMmcSpi_t>::put_fat (FATFS* fs,	DWORD clst,	DWORD val);
template fatfs::FRESULT fatfs::Driver<DiskioMmcSpi_t>::remove_chain (FFOBJID* obj,	DWORD clst,	DWORD pclst);
template fatfs::DWORD fatfs::Driver<DiskioMmcSpi_t>::create_chain (FFOBJID* obj, DWORD clst);
template fatfs::FRESULT fatfs::Driver<DiskioMmcSpi_t>::dir_clear (FATFS *fs, DWORD clst);
template fatfs::FRESULT fatfs::Driver<DiskioMmcSpi_t>::dir_sdi (DIR* dp, DWORD ofs);
template fatfs::FRESULT fatfs::Driver<DiskioMmcSpi_t>::dir_next (DIR* dp, int stretch);
template fatfs::FRESULT fatfs::Driver<DiskioMmcSpi_t>::dir_alloc (DIR* dp,	UINT n_ent);
template fatfs::FRESULT fatfs::Driver<DiskioMmcSpi_t>::dir_read (DIR* dp, int vol);
template fatfs::FRESULT fatfs::Driver<DiskioMmcSpi_t>::dir_find (DIR* dp);
template fatfs::FRESULT fatfs::Driver<DiskioMmcSpi_t>::dir_register (DIR* dp);
template fatfs::FRESULT fatfs::Driver<DiskioMmcSpi_t>::dir_remove (DIR* dp);
template fatfs::FRESULT fatfs::Driver<DiskioMmcSpi_t>::follow_path (DIR* dp, const TCHAR* path);
#endif

#if defined(ENABLE_MMC_SDIO)
using DiskioMmcSdio_t = fatfs::DiskioHardwareMMC<fatfs::DiskioProtocolSDIO>;
template DiskioMmcSdio_t::DiskioHardwareMMC(const DiskioProtocolSDIO &protocol);
template fatfs::Driver<DiskioMmcSdio_t>::Driver(DiskioMmcSdio_t &diskio);
#endif