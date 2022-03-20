target_sources(${BUILD_NAME} PRIVATE
    ff_driver.cpp
    ff_driver_common.cpp
    ffsystem.cpp
    ffunicode.cpp
    diskio_layer/implementation/diskio_mmc_spi.cpp
    diskio_layer/implementation/diskio_mmc_sdio.cpp
    diskio_layer/implementation/diskio_usb.cpp
)

target_include_directories(${BUILD_NAME} PRIVATE 
    ../include
    ../include/diskio_layer/physical
    ../include/diskio_layer/protocol
)
