target_sources(${BUILD_NAME} PRIVATE
    catch_cpp_fatfs.cpp
    # cloned using external.cmake
    ${CMAKE_BINARY_DIR}/embedded_utils/src/spi_utils.cpp
    ${CMAKE_BINARY_DIR}/embedded_utils/src/timer_manager.cpp
)

target_include_directories(${BUILD_NAME} PRIVATE 
    .
    # cloned using external.cmake
    ${CMAKE_BINARY_DIR}/stm32_interrupt_managers/include
    ${CMAKE_BINARY_DIR}/embedded_utils/include
)