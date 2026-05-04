# Development

## Prerequisites

- [CMake](https://cmake.org/)
- [Make](https://www.make.com/en)
- [Just](https://just.systems)

### Optional

- lcov for code coverage

## Scripts

See [scripts](../scripts/README.md)

## Adding Functionality

### Create A Native Function

- [ ] 1. Declare a new function in [vm.c](../src/vm.c)
- [ ] 2. Register the native function with `defineNative("function_name", functionNameNative)` in `initVM`
- [ ] 3. Add [tests](../tests/README.md) for the new function
- [ ] 4. Add function to [`clox.tmLanguage.json`](../vsc/syntaxes/clox.tmLanguage.json) under `"name": "entity.name.function.native.clox"`
  - [ ] Call
  - [ ] Call with no args (if applicable)
  - [ ] Call with too many args (if applicable)
  - [ ] Call with incorrect arg types (if applicable)
