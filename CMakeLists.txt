target_sources(${BUILD_NAME} PRIVATE
    src/ff_driver.cpp
    src/ff_driver_common.cpp
    src/ffsystem.cpp
    src/ffunicode.cpp
    src/diskio_hardware_base.cpp
    src/diskio_mmc_spi_impl.cpp
    src/diskio_mmc_sdio_impl.cpp
    src/diskio_usb_impl.cpp
    
)

target_include_directories(${BUILD_NAME} PRIVATE 
    inc
)

