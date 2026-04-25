#include <getopt.h>
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

const char *help_message = "Usage: clox [-h] [-v] [path]\n";

const char *short_options = "hv";
static struct option long_options[] = {{"help", no_argument, 0, 'h'},
                                       {"version", no_argument, 0, 'v'},
                                       {0, 0, 0, 0}};

int main(int argc, char *argv[]) {
  int opt;
  int long_index = 0;

  while ((opt = getopt_long(argc, argv, short_options, long_options,
                            &long_index)) != -1) {
    switch (opt) {
    case 'h':
      printf("%s\n", help_message);
      return 0;

    case 'v':
      printf("%s\n", CLOX_VERSION);
      return 0;
    }
  }

  if (optind >= argc) {
    initVM();
    runFile("stdlib/stdlib.lox");
    repl();
    freeVM();
  } else if (optind == argc - 1) {
    initVM();
    runFile("stdlib/stdlib.lox");
    runFile(argv[optind]);
    freeVM();
  } else {
    fprintf(stderr, "%s\n", help_message);
    exit(64);
  }

  return 0;
}

static void repl() {
  fprintf(stderr, "========================================\n");
  fprintf(stderr, "REPL %30s clox\n\n", CLOX_VERSION);
  fprintf(stderr, "Enter code or type 'exit' to quit.\n\n");
  // fprintf(stderr, "========================================\n\n");
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

    InterpretResult result = interpret(line);

    switch (result) {
    case INTERPRET_COMPILE_ERROR:
      fprintf(stderr, "Compiler Error!\n");
      break;
    case INTERPRET_RUNTIME_ERROR:
      fprintf(stderr, "Runtime Error!\n");
      break;
    default:
      break;
    }

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