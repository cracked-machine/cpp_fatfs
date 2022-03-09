target_sources(${BUILD_NAME} PRIVATE
    src/ff.cpp
    src/ffsystem.cpp
    src/ffunicode.cpp
    src/diskio.cpp
)

target_include_directories(${BUILD_NAME} PRIVATE 
    inc
)

