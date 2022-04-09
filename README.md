[![CMake](https://github.com/cracked-machine/cpp_fatfs/actions/workflows/cmake.yml/badge.svg)](https://github.com/cracked-machine/cpp_fatfs/actions/workflows/cmake.yml)
[![Codecov](https://img.shields.io/codecov/c/github/cracked-machine/cpp_fatfs)](https://app.codecov.io/gh/cracked-machine/cpp_fatfs)


See the [wiki](https://github.com/cracked-machine/cpp_fatfs/wiki) for documentation / reference

See [readme](tests) for information on unit testing/mocking.

#### Adding this library to your STM32 Project

There are two ways to add this library to your project's CMakeLists.txt:

1. Implicitly include the [external.cmake](cmake/external.cmake):

```
set(BUILD_NAME "MyProject")
add_executable(${BUILD_NAME} "")
include(cmake/external.cmake)
```

2. Explicitly add [embedded_utils](https://github.com/cracked-machine/embedded_utils.git) to your project as a ubmodule and add the subdirectory:

```
add_subdirectory(extern/embedded_utils)
```
