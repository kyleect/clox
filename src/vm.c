#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "memory.h"
#include "object.h"
#include "string.h"
#include "version.h"
#include "vm.h"

static ObjString *readFile(const char *path);
static void resetStack();
static void runtimeError(const char *format, ...);
static Value peek(int distance);
static InterpretResult run();
static void concatenate();
static bool call(ObjClosure *function, int argCount);

VM vm;

static Value clockNative(int argCount, Value *args) {
  return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

static Value versionNative(int argCount, Value *args) {
  return OBJ_VAL(copyString(CLOX_VERSION, CLOX_VERSION_len));
}

static Value exitNative(int argCount, Value *args) {
  if (argCount != 1) {
    runtimeError("expected 1 arguments but got %d", argCount);
    exit(70); // INTERPRET_RUNTIME_ERROR
  }

  Value exitCode = args[0];

  if (!IS_NUMBER(exitCode)) {
    runtimeError("expected a number argument");
    exit(70); // INTERPRET_RUNTIME_ERROR
  }

  exit(exitCode.as.number);

  return NIL_VAL;
}

static Value randNative(int argCount, Value *args) {
  return NUMBER_VAL((double)rand());
}

static Value rand01Native(int argCount, Value *args) {
  return NUMBER_VAL((double)rand() / (double)RAND_MAX);
}

static Value randBetweenNative(int argCount, Value *args) {
  if (argCount != 2) {
    runtimeError("randomRange(min, max) expects 2 arguments.");
    return NIL_VAL;
  }

  if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1])) {
    runtimeError("randomRange(min, max) arguments must be numbers.");
    return NIL_VAL;
  }

  double min = AS_NUMBER(args[0]);
  double max = AS_NUMBER(args[1]);

  if (min > max) {
    runtimeError("randomRange(min, max) requires min <= max.");
    return NIL_VAL;
  }

  double r = (double)rand() / (double)RAND_MAX; // [0, 1]
  double result = min + r * (max - min);        // [min, max)

  return NUMBER_VAL(ceil(result));
}

static Value ceilNative(int argCount, Value *args) {
  if (argCount != 1) {
    runtimeError("ceil(value) expects 1 arguments.");
    return NIL_VAL;
  }

  Value value = args[0];

  if (!IS_NUMBER(value)) {
    runtimeError("ceil(value) argument must be a number.");
    return NIL_VAL;
  }

  return NUMBER_VAL(ceil(AS_NUMBER(value)));
}

static Value readFileToStringNative(int argCount, Value *args) {
  if (argCount != 1) {
    runtimeError("ceil(value) expects 1 arguments.");
    return NIL_VAL;
  }

  Value value = args[0];

  if (!IS_STRING(value)) {
    runtimeError("ceil(value) argument must be a number.");
    return NIL_VAL;
  }

  ObjString *path = AS_STRING(value);
  ObjString *contents = readFile(path->chars);

  return OBJ_VAL(contents);
}

static Value getEnvNative(int argCount, Value *args) {
  if (argCount != 1) {
    runtimeError("getenv(name) expects 1 argument.");
    exit(70); // INTERPRET_RUNTIME_ERROR
  }

  Value name = args[0];

  if (!IS_STRING(name)) {
    runtimeError("getenv(name) argument must be a string.");
    exit(70); // INTERPRET_RUNTIME_ERROR
  }

  char *name_str = AS_CSTRING(name);

  char *env_value = getenv(name_str);

  if (env_value == NULL) {
    runtimeError("Environment variable not found: '%s'", name_str);
    exit(70); // INTERPRET_RUNTIME_ERROR
  }

  return OBJ_VAL(copyString(env_value, (int)strlen(env_value)));
}

static Value setEnvNative(int argCount, Value *args) {
  if (argCount != 2) {
    runtimeError("setenv(name, value) takes exactly 2 arguments.");
    exit(70); // INTERPRET_RUNTIME_ERROR
  }

  if (!IS_STRING(args[0]) || !IS_STRING(args[1])) {
    runtimeError("setenv(name, value) arguments must be strings.");
    exit(70); // INTERPRET_RUNTIME_ERROR
  }

  ObjString *name = AS_STRING(args[0]);
  ObjString *value = AS_STRING(args[1]);

  // overwrite = 1 means always replace existing value
  int result = setenv(name->chars, value->chars, 1);

  if (result != 0) {
    runtimeError("setenv unable to set environment variable: '%s'",
                 name->chars);
    exit(70); // INTERPRET_RUNTIME_ERROR
  }

  return NIL_VAL; // or you could return true if you prefer
}

InterpretResult interpret(const char *source) {
  TRACELN("vm.interpret()");

  ObjFunction *function = compile(source);
  if (function == NULL)
    return INTERPRET_COMPILE_ERROR;

  push(OBJ_VAL(function));
  ObjClosure *closure = newClosure(function);
  pop();
  push(OBJ_VAL(closure));
  call(closure, 0);

  return run();
}

static void resetStack() {
  vm.stackTop = vm.stack;
  vm.frameCount = 0;
}

static void runtimeError(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  for (int i = vm.frameCount - 1; i >= 0; i--) {
    CallFrame *frame = &vm.frames[i];
    ObjFunction *function = frame->closure->function;
    size_t instruction = frame->ip - function->chunk.code - 1;

    fprintf(stderr, "[line %d] in ", function->chunk.lines[instruction]);

    if (function->name == NULL) {
      fprintf(stderr, "script\n");
    } else {
      fprintf(stderr, "%s()\n", function->name->chars);
    }
  }

  resetStack();
}

static void defineNative(const char *name, NativeFn function) {
  push(OBJ_VAL(copyString(name, (int)strlen(name))));
  push(OBJ_VAL(newNative(function)));
  tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
  pop();
  pop();
}

void initVM() {
  TRACELN("vm.initVM()");
  srand(time(NULL));
  resetStack();
  vm.objects = NULL;
  initTable(&vm.globals);
  initTable(&vm.strings);

  defineNative("clock", clockNative);
  defineNative("__version__", versionNative);
  defineNative("exit", exitNative);
  defineNative("rand", randNative);
  defineNative("rand01", rand01Native);
  defineNative("randBetween", randBetweenNative);
  defineNative("ceil", ceilNative);
  defineNative("readFileToString", readFileToStringNative);
  defineNative("getenv", getEnvNative);
  defineNative("setenv", setEnvNative);
}

void freeVM() {
  TRACELN("vm.freeVM()");
  freeTable(&vm.globals);
  freeTable(&vm.strings);
  freeObjects();
}

void push(Value value) {
  *vm.stackTop = value;
  vm.stackTop++;
}

Value pop() {
  vm.stackTop--;
  return *vm.stackTop;
}

static Value peek(int distance) { return vm.stackTop[-1 - distance]; }

static bool call(ObjClosure *closure, int argCount) {
  if (argCount != closure->function->arity) {
    runtimeError("Expected %d arguments but got %d.", closure->function->arity,
                 argCount);
    return false;
  }

  if (vm.frameCount == FRAMES_MAX) {
    runtimeError("Stack overflow.");
    return false;
  }

  CallFrame *frame = &vm.frames[vm.frameCount++];
  frame->closure = closure;
  frame->ip = closure->function->chunk.code;
  frame->slots = vm.stackTop - argCount - 1;
  return true;
}

static bool callValue(Value callee, int argCount) {
  if (IS_OBJ(callee)) {
    switch (OBJ_TYPE(callee)) {
    case OBJ_CLOSURE:
      return call(AS_CLOSURE(callee), argCount);
    case OBJ_NATIVE: {
      NativeFn native = AS_NATIVE(callee);
      Value result = native(argCount, vm.stackTop - argCount);
      vm.stackTop -= argCount + 1;
      push(result);
      return true;
    }
    default:
      break; // Non-callable object type.
    }
  }
  runtimeError("Can only call functions and classes.");
  return false;
}

static bool isFalsey(Value value) {
  return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate() {
  ObjString *b = AS_STRING(pop());
  ObjString *a = AS_STRING(pop());

  int length = a->length + b->length;
  char *chars = ALLOCATE(char, length + 1);
  memcpy(chars, a->chars, a->length);
  memcpy(chars + a->length, b->chars, b->length);
  chars[length] = '\0';

  ObjString *result = takeString(chars, length);
  push(OBJ_VAL(result));
}

static ObjString *readFile(const char *path) {
  FILE *file = fopen(path, "rb"); // "rb" = binary mode (safe for all files)
  if (!file)
    return NULL;

  // Go to end to get file size
  if (fseek(file, 0, SEEK_END) != 0) {
    fclose(file);
    return NULL;
  }

  long size = ftell(file);
  if (size < 0) {
    fclose(file);
    return NULL;
  }

  rewind(file); // Go back to start

  // Allocate buffer (+1 for null terminator)
  char *buffer = (char *)malloc(size + 1);
  if (!buffer) {
    fclose(file);
    return NULL;
  }

  size_t bytesRead = fread(buffer, 1, size, file);
  if (bytesRead != (size_t)size) {
    free(buffer);
    fclose(file);
    return NULL;
  }

  buffer[size] = '\0'; // Null-terminate so it's a valid C string

  fclose(file);

  return takeString(buffer, size);
}

static InterpretResult run() {
  CallFrame *f = &vm.frames[vm.frameCount - 1];
#define READ_BYTE() (*f->ip++)
#define READ_CONSTANT()                                                        \
  (f->closure->function->chunk.constants.values[READ_BYTE()])
#define READ_SHORT() (f->ip += 2, (uint16_t)((f->ip[-2] << 8) | f->ip[-1]))
#define READ_STRING() AS_STRING(READ_CONSTANT())
#define BINARY_OP(valueType, op)                                               \
  do {                                                                         \
    if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {                          \
      runtimeError("Operands must be numbers.");                               \
      return INTERPRET_RUNTIME_ERROR;                                          \
    }                                                                          \
    double b = AS_NUMBER(pop());                                               \
    double a = AS_NUMBER(pop());                                               \
    push(valueType(a op b));                                                   \
  } while (false)

  TRACELN("vm.run()");

  for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
    fprintf(stderr, "          ");
    for (Value *slot = vm.stack; slot < vm.stackTop; slot++) {
      fprintf(stderr, "[ ");
      printValueToErr(*slot);
      fprintf(stderr, " ]");
    }

    fprintf(stderr, "\n");

    disassembleInstruction(&f->closure->function->chunk,
                           (int)(f->ip - f->closure->function->chunk.code));
#endif

    uint8_t instruction;
    switch (instruction = READ_BYTE()) {
    case OP_CONSTANT: {
      Value constant = READ_CONSTANT();
      push(constant);
      break;
    }
    case OP_NIL:
      push(NIL_VAL);
      break;
    case OP_TRUE:
      push(BOOL_VAL(true));
      break;
    case OP_FALSE:
      push(BOOL_VAL(false));
      break;
    case OP_ADD: {
      if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
        concatenate();
      } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
        double b = AS_NUMBER(pop());
        double a = AS_NUMBER(pop());
        push(NUMBER_VAL(a + b));
      } else {
        runtimeError("Operands must be two numbers or two strings.");
        return INTERPRET_RUNTIME_ERROR;
      }
      break;
    }
    case OP_SUBTRACT:
      BINARY_OP(NUMBER_VAL, -);
      break;
    case OP_MULTIPLY:
      BINARY_OP(NUMBER_VAL, *);
      break;
    case OP_DIVIDE:
      BINARY_OP(NUMBER_VAL, /);
      break;
    case OP_NOT:
      push(BOOL_VAL(isFalsey(pop())));
      break;
    case OP_NEGATE:
      if (!IS_NUMBER(peek(0))) {
        runtimeError("Operand must be a number.");
        return INTERPRET_RUNTIME_ERROR;
      }
      push(NUMBER_VAL(-AS_NUMBER(pop())));
      break;
    case OP_PRINT: {
      printValue(pop());
      printf("\n");
      break;
    }
    case OP_RETURN: {
      Value result = pop();
      vm.frameCount--;
      if (vm.frameCount == 0) {
        pop();
        return INTERPRET_OK;
      }

      vm.stackTop = f->slots;
      push(result);
      f = &vm.frames[vm.frameCount - 1];
      break;
    }
    case OP_EQUAL: {
      Value b = pop();
      Value a = pop();
      push(BOOL_VAL(valuesEqual(a, b)));
      break;
    }
    case OP_GREATER:
      BINARY_OP(BOOL_VAL, >);
      break;
    case OP_LESS:
      BINARY_OP(BOOL_VAL, <);
      break;
    case OP_POP: {
      pop();
      break;
    }
    case OP_GET_LOCAL: {
      uint8_t slot = READ_BYTE();
      push(f->slots[slot]);
      break;
    }
    case OP_SET_LOCAL: {
      uint8_t slot = READ_BYTE();
      f->slots[slot] = peek(0);
      break;
    }
    case OP_GET_GLOBAL: {
      ObjString *name = READ_STRING();
      Value value;
      if (!tableGet(&vm.globals, name, &value)) {
        runtimeError("Undefined variable '%s'.", name->chars);
        return INTERPRET_RUNTIME_ERROR;
      }
      push(value);
      break;
    }
    case OP_SET_GLOBAL: {
      ObjString *name = READ_STRING();

      if (tableSet(&vm.globals, name, peek(0))) {
        tableDelete(&vm.globals, name);
        runtimeError("Undefined variable %s", name->chars);
        return INTERPRET_RUNTIME_ERROR;
      }

      break;
    }
    case OP_DEFINE_GLOBAL: {
      ObjString *name = READ_STRING();
      tableSet(&vm.globals, name, peek(0));
      pop();
      break;
    }
    case OP_JUMP_IF_FALSE: {
      uint16_t offset = READ_SHORT();
      if (isFalsey(peek(0)))
        f->ip += offset;
      break;
    }
    case OP_JUMP: {
      uint16_t offset = READ_SHORT();
      f->ip += offset;
      break;
    }
    case OP_LOOP: {
      uint16_t offset = READ_SHORT();
      f->ip -= offset;
      break;
    }
    case OP_CALL: {
      int argCount = READ_BYTE();
      if (!callValue(peek(argCount), argCount)) {
        return INTERPRET_RUNTIME_ERROR;
      }
      f = &vm.frames[vm.frameCount - 1];
      break;
    }
    case OP_CLOSURE: {
      ObjFunction *function = AS_FUNCTION(READ_CONSTANT());
      ObjClosure *closure = newClosure(function);
      push(OBJ_VAL(closure));
      break;
    }
    }
  }
#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP
}
