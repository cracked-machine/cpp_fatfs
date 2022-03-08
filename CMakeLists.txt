# build as static lib
add_library(FF14b STATIC "")
target_sources(FF14b PRIVATE
    src/ff.cpp
    src/ffsystem.cpp
    src/ffunicode.cpp
    src/diskio.cpp
)

target_include_directories(FF14b PRIVATE 
    inc
)

