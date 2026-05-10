# Extending Lox

These are the documented changes to the language/syntax from the original Lox language.

## Next: 0.2.0

- Modulo operator: `%`
  - New token: `TOKEN_MODULO`
  - New bytecode op: `OP_MODULO`
- Short forms of `and`, `or`: `&&` and `||`
- Nullish coalescing operator (`??`)
  - New tokens: `TOKEN_QUESTION_QUESTION`
  - New bytecode OPs: `OP_JUMP_IF_NOT_NIL`
- Class field declarations
  - Class fields are no longer dynamically defined. They must be declared before use.
  - New bytecode OP: `OP_FIELD`
- Native function: `numberToString(number)`
- Arrays
  - New tokens: `TOKEN_LEFT_BRACKET`, `TOKEN_RIGHT_BRACKET`
  - New bytecode OPs: `OP_ARRAY`, `OP_INDEX_GET`, `OP_INDEX_SET`
  - New native functions:
    - `arrPush(array, value)`
    - `arrPop(array)`
    - `arrInsert(array, index, value)`
  - Update `len` native function to return array lengths as well

## 0.1.0

- No class inheritance
- Native functions added
  - `exit`
  - `__version__`
  - `rand`
  - `rand01`
  - `randBetween`
  - `ceil`
  - `readFileToString`
  - `writeStringToFile`
  - `fileExists`
  - `getenv`
  - `setenv`
  - `len`
  - `typeof`
  - `argc`
  - `argv`
  - `prompt`
  - `stdin`
  - `instanceOf`
  - `parseNumber`
