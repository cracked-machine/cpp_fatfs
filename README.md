
### Architecture

The main architecture consists of a public API that uses a lower level disk IO layer. This shows specific implementations of the DiskIO layer for MMC/SD and USB. 

![](doc/cpp_fatfs-BlockDiagram.png)

Details of the STM32 SPI interface (SPI_TypeDef, GPIO ports and pins) are passed as a `DriverInterfaceSPI` object into the `DiskioHardwareMMC` class.  The `DiskioHardwareMMC` object is then passed into the main `Driver` API class. A `FileManager` class can be used to manage the `DiskioHardwareMMC` and `Driver` objects.

![](doc/cpp_fatfs-InitSequence.png)

<!-- @startuml
MainApp -> DriverInterfaceSPI ** : create
MainApp -> FileManager ** : create(DriverInterfaceSPI)
FileManager -> DiskioHardwareMMC ** : create(DriverInterfaceSPI)
FileManager -> "Driver<DiskioHardwareMMC>" ** : create(DiskioHardwareMMC)
"Driver<DiskioHardwareMMC>" -> "Driver<DiskioHardwareMMC>" : unique_ptr<DiskioHardwareMMC>
@enduml -->

### CMake

```
add_subdirectory(cpp_fatfs)

```

### Usage

```
#include <diskio.hpp>

fatfs::DiskioHardwareMMC diskio;
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

