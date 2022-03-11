target_sources(${BUILD_NAME} PRIVATE
    src/ff.cpp
    src/ff_common.cpp
    src/ffsystem.cpp
    src/ffunicode.cpp
    src/diskio_base.cpp
    src/diskio_mmc.cpp
    src/diskio_usb.cpp
)

target_include_directories(${BUILD_NAME} PRIVATE 
    inc
)

