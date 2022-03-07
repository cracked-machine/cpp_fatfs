

target_sources(${BUILD_NAME} PRIVATE

    # elm-chan.org fatfs c library
    source/ff.cpp
    source/ffsystem.cpp
    source/ffunicode.cpp
)

target_include_directories(${BUILD_NAME} PRIVATE 
    # elm-chan.org fatfs c library
    source
)