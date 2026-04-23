#include <readline/history.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chunk.h"
#include "common.h"
#include "debug.h"
#include "version.h"
#include "vm.h"

static void repl();
static char *readFile(const char *path);
static void runFile(const char *path);

int main(int argc, const char *argv[]) {
  initVM();

  if (argc == 1) {
    repl();
  } else if (argc == 2) {
    runFile(argv[1]);
  } else {
    fprintf(stderr, "Usage: clox [path]\n");
    exit(64);
  }

  freeVM();

  return 0;
}

static void repl() {
  fprintf(stderr, "");
  fprintf(stderr, "Clox v%s", VERSION_txt);
  for (;;) {
    char *line = readline("> ");

    if (line == NULL) {
      printf("\n");
      break;
    }

    if (*line)
      add_history(line);

    if (strcmp(line, "exit") == 0)
      exit(0);

    interpret(line);
    free(line);
  }
}

static char *readFile(const char *path) {
  FILE *file = fopen(path, "rb");

  fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file);
  rewind(file);

  char *buffer = (char *)malloc(fileSize + 1);
  size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
  buffer[bytesRead] = '\0';

  fclose(file);
  return buffer;
}

static void runFile(const char *path) {
  char *source = readFile(path);
  InterpretResult result = interpret(source);
  free(source);

  if (result == INTERPRET_COMPILE_ERROR) {
    exit(65);
  }

  if (result == INTERPRET_RUNTIME_ERROR)
    exit(70);
}