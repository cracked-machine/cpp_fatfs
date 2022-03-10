### cpp_fatfs
A C++ port of the fatfs library

### Architecture

![](doc/cpp_fatfs-BlockDiagram.png)

### CMake

```
add_subdirectory(cpp_fatfs)

```

### Usage

```
#include <diskio.hpp>

fatfs::DiskioMMC diskio;
fatfs::Driver fatfs_handle(diskio);
fatfs::FATFS fs;
fatfs::FIL fil;
char sd_path[4];          /* uSD device logical drive path */
fatfs::FRESULT fres;
fres = fatfs_handle.f_mount(&fs, (fatfs::TCHAR const*)sd_path, 1); //1=mount now
fres = fatfs_handle.f_open(&fil, (fatfs::TCHAR const*)sd_path, 1);

std::array<char, 100> read_buff;
fatfs::UINT bytes_read {0};
fres = fatfs_handle.f_read(&fil, read_buff.data(), read_buff.size(), &bytes_read);

std::array<char, 100> write_buff;
fatfs::UINT bytes_written {0};
fres = fatfs_handle.f_write(&fil, write_buff.data(), write_buff.size(), &bytes_written);

if (fres != fatfs::FRESULT::FR_OK) {
    // something bad happened
}	
```

