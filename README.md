### cpp_fatfs
A C++ port of the fatfs library

### CMake

```
# link in FF14b static lib
add_subdirectory(cpp_fatfs)
target_link_libraries(${BUILD_NAME} PUBLIC FF14b)
target_include_directories(${BUILD_NAME} PUBLIC
    "${PROJECT_SOURCE_DIR}/cpp_fatfs/inc")
```

### Usage

```
#include <ff.hpp>

fatfs::ff fatfs_driver;
fatfs::FATFS fs;
fatfs::FIL fil;
char sd_path[4];          /* uSD device logical drive path */
fatfs::FRESULT fres;
fres = fatfs_driver.f_mount(&fs, (fatfs::TCHAR const*)sd_path, 1); //1=mount now
fres = fatfs_driver.f_open(&fil, (fatfs::TCHAR const*)sd_path, 1);
 
std::array<char, 100> read_buff;
fatfs::UINT bytes_read {0};
fres = fatfs_driver.f_read(&fil, read_buff.data(), read_buff.size(), &bytes_read);
	
std::array<char, 100> write_buff;
fatfs::UINT bytes_written {0};
fres = fatfs_driver.f_write(&fil, write_buff.data(), write_buff.size(), &bytes_written);
 
if (fres != fatfs::FRESULT::FR_OK) {
	// something bad happened
}	
```

