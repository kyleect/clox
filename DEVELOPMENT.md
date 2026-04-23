# Development

## Prerequisites

- [CMake](https://cmake.org/)
- [Make](https://www.make.com/en)

## Optional

- [Just](https://just.systems)

## Scripts

### CMake

Run `./scripts/cmake.sh` or `just cmake` to generate make files for the project.

### Build

Run `./scripts/build.sh` or `just build` to build to project in to `./build` folder.

#### Definitions

Different build definitions can be updated in [CMakeLists.txt](./CMakeLists.txt) run `just cmake && just build`.

##### DEBUG_PRINT_CODE

Setting `DEBUG_PRINT_CODE` will print disassembled bytecode.

##### DEBUG_TRACE_EXECUTION

Setting `DEBUG_TRACE_EXECUTION` will print execution trace logs.

### Test

Run `./scripts/test.sh` or `just test` to run the tests in `./test` folder.

#### Filtering Tests

```sh
./scripts/test.sh PATTERN
```

### Test (Update Snapshots)

Run `./scripts/test.sh --update` or `just test-update` to run the tests and update the snapshot files.

#### Filtering Updates

```sh
./scripts/test.sh PATTERN --update
```

### Clean

Run `./scripts/clean.sh` or `just clean` to remove build artifacts.

### Docker Build/Run

Run `just docker-build` to build the docker image `clox`. Run `just docker-run` to run the `clox` REPL inside a `clox` docker container.

## Language Version

The current language version is defined in the [VERSION.txt](./VERSION.txt) file.

### Generating The C File

Generate/update the [`src/version.c`](./src/version.c) file.

```sh
./scripts/generate_version_c.sh
just generate_version_c
```
