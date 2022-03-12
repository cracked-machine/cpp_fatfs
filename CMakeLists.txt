target_sources(${BUILD_NAME} PRIVATE
    src/ff_driver.cpp
    src/ff_driver_common.cpp
    src/ffsystem.cpp
    src/ffunicode.cpp
    src/diskio_layer/implementation/diskio_mmc_spi.cpp
    src/diskio_layer/implementation/diskio_mmc_sdio.cpp
    src/diskio_layer/implementation/diskio_usb.cpp
)

target_include_directories(${BUILD_NAME} PRIVATE 
    inc
    inc/diskio_layer/physical
    inc/diskio_layer/protocol
)
