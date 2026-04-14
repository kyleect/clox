# Development

## Prerequisites

- [Just](https://just.systems)
- [CMake](https://cmake.org/)
- [Make](https://www.make.com/en)

## Scripts

### CMake

Run `just cmake` to generate make files for the project.

### Build

Run `just build` to build to project in to `./build` folder.

#### Definitions

Different build definitions can be updated in [CMakeLists.txt](./CMakeLists.txt) run `just cmake && just build`.

##### DEBUG_PRINT_CODE

Setting `DEBUG_PRINT_CODE` will print disassembled bytecode.

##### DEBUG_TRACE_EXECUTION

Setting `DEBUG_TRACE_EXECUTION` will print execution trace logs.

### Test

Run `just test` to run the tests in `./test` folder.

### Clean

Run `just clean` to remove build artifacts.

### Docker Build/Run

Run `just docker-build` to build the docker image `clox`. Run `just docker-run` to run the `clox` REPL inside a `clox` docker container.
