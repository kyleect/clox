#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "hashtable.h"
#include "object.h"
#include "value.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct {
  ObjClosure *closure;
  uint8_t *ip;
  Value *slots;
} CallFrame;

typedef struct {
  int argc;
  char **argv;

  CallFrame frames[FRAMES_MAX];
  int frameCount;

  Chunk *chunk;
  uint8_t *ip;
  Value stack[STACK_MAX];
  Value *stackTop;
  Table globals;
  Table strings;
  ObjString *initString;
  ObjUpvalue *openUpvalues;
  size_t bytesAllocated;
  size_t nextGC;
  Obj *objects;
  int grayCount;
  int grayCapacity;
  Obj **grayStack;
} VM;

extern VM vm;

typedef enum {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} InterpretResult;

void initVM(int argc, char *argv[]);
void freeVM();
InterpretResult interpret(const char *source);
void push(Value value);
Value pop();

#endif