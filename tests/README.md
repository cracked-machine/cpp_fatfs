## Running Units Tests on X86

Open this project in VSCode to run the unit tests. The build output is linked with the Catch2 library, so to run the unit tests you only need to run the build:
`./build/test_suite`

See `.vscode/tasks.json` for details on the individual toolchain commands.

## CMSIS Mocking

This project downloads the [embedded_utils](https://github.com/cracked-machine/embedded_utils/tree/main/tests) repo which contains a minimal STM32 mocking library.

All files automatically include the `mock.hpp` header during unit testing build. See [CMakeFiles.txt](../CMakeLists.txt#L14):

```
add_definitions(-include ${CMAKE_SOURCE_DIR}/tests/mocks/mock.hpp) 
```