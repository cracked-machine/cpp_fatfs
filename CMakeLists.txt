target_sources(${BUILD_NAME} PRIVATE
    src/fatfs.cpp
)

target_include_directories(${BUILD_NAME} PRIVATE 
    inc
)

add_subdirectory(lib/ff14b)