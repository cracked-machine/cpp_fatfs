## Running Units Tests on X86

Open this project in VSCode to run the unit tests. The build output is linked with the Catch2 library, so to run the unit tests you only need to run the build:
`./build/test_suite`

See `.vscode/tasks.json` for details on the individual toolchain commands.

## CMSIS Mocking

This project downloads the [embedded_utils](https://github.com/cracked-machine/embedded_utils/tree/main/tests) repo which contains a minimal STM32 mocking library.

Building this library project directly will define `STM32G0B1xx` which means header files can include the mocked version of `stm32g0xx.h` header. The library will use the STM32 version of `stm32g0xx.h` when built as a submodule.