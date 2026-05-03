# Tests

The language's E2E tests are file based:

| Example File    | Description              |
| --------------- | ------------------------ |
| `test.lox`      | Source test file         |
| `test.lox.out`  | Expected `stdout` output |
| `test.lox.err`  | Expected `stderr` output |
| `test.lox.exit` | Expected exit code       |
| `test.lox.in`   | Text sent to `stdin`     |
| `test.lox.env`  | Environment Variables    |

## Writing A Test

1. Create a new file in the [`tests`](./) directory: `./tests/new_test.lox`
2. Add test lox code
3. Implement feature being tested
4. [Update snapshots](#update-snapshots) which will create `./tests/new_test.lox.out`, `./tests/new_test.lox.err`, `./tests/new_test.lox.exit` files
5. Validate new snapshots. Confirm no other snapshots updated.
6. Commit snapshots if everything is verified

## Run Tests

The tests are run using the [tests.sh](../scripts/tests.sh) script.

```shell
./scripts/tests.sh
```

### Filtering Tests

```shell
./scripts/tests.sh pattern
```

### Verbose

Output additional information when running tests.

```shell
./scripts/tests.sh --verbose
```

## Update Snapshots

Run all tests and update any changed snapshot files.

```shell
./scripts/tests.sh --update
```
