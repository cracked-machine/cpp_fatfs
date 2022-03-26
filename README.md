[![CMake](https://github.com/cracked-machine/cpp_fatfs/actions/workflows/cmake.yml/badge.svg)](https://github.com/cracked-machine/cpp_fatfs/actions/workflows/cmake.yml)
[![Codecov](https://img.shields.io/codecov/c/github/cracked-machine/cpp_fatfs)](https://app.codecov.io/gh/cracked-machine/cpp_fatfs)


See the [wiki](https://github.com/cracked-machine/cpp_fatfs/wiki) for documentation / reference

See `.vscode/tasks.json` for details on the individual toolchain commands.
#### Running Units Tests on X86

When you run the default CMake build, the output is linked with the Catch2 library. 

To run the testsuite use the command: `./build/test_suite`


#### Adding this library to your STM32 Project

Include this repo into your project as a submodule and add the following line to your top-level CMakeFiles.txt:

`add_subdirectory(cpp_fatfs)`

This assumes your top-level CMakeFiles.txt is already configured for STM32 platform.
### Architecture

The main architecture consists of a public API that uses a lower level disk IO layer. This shows specific implementations of the DiskIO layer for MMC/SD and USB. 

![](docs/cpp_fatfs-BlockDiagram.png)

Details of the STM32 SPI interface (SPI_TypeDef, GPIO ports and pins) are passed as a `DriverInterfaceSPI` object into the `DiskioHardwareMMC` class.  The `DiskioHardwareMMC` object is then passed into the main `Driver` API class. A `FileManager` class can be used to manage the `DiskioHardwareMMC` and `Driver` objects.

![](docs/cpp_fatfs-InitSequence.png)

<!-- @startuml
MainApp -> DriverProtocolSPI ** : create
MainApp -> FileManager ** : create(DriverProtocolSPI)
FileManager -> "DiskioHardwareMMC<DriverProtocolSPI>" ** : create(DriverProtocolSPI)
FileManager -> "Driver<DiskioHardwareMMC<DriverProtocolSPI>>" ** : create(DiskioHardwareMMC<DriverProtocolSPI>)
"Driver<DiskioHardwareMMC<DriverProtocolSPI>>" -> "Driver<DiskioHardwareMMC<DriverProtocolSPI>>" : unique_ptr<DiskioHardwareMMC<DriverProtocolSPI>>
@enduml -->


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
