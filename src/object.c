#include <stdio.h>
#include <string.h>

#include "hashtable.h"
#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, objectType)                                         \
  (type *)allocateObject(sizeof(type), objectType)

static Obj *allocateObject(size_t size, ObjType type) {
  Obj *object = (Obj *)reallocate(NULL, 0, size);
  object->type = type;

  object->next = vm.objects;
  vm.objects = object;

  return object;
}

ObjClosure *newClosure(ObjFunction *function) {
  ObjClosure *closure = ALLOCATE_OBJ(ObjClosure, OBJ_CLOSURE);
  closure->function = function;
  return closure;
}

ObjFunction *newFunction() {
  ObjFunction *function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
  function->arity = 0;
  function->name = NULL;
  initChunk(&function->chunk);
  return function;
}

ObjNative *newNative(NativeFn function) {
  ObjNative *native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
  native->function = function;
  return native;
}

static ObjString *allocateString(char *chars, int length, uint32_t hash) {
  ObjString *string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
  string->length = length;
  string->chars = chars;
  string->hash = hash;
  tableSet(&vm.strings, string, NIL_VAL);
  return string;
}

static uint32_t hashString(const char *key, int length) {
  uint32_t hash = 2166136261u;
  for (int i = 0; i < length; i++) {
    hash ^= (uint8_t)key[i];
    hash *= 16777619;
  }
  return hash;
}

ObjString *takeString(char *chars, int length) {
  uint32_t hash = hashString(chars, length);

  ObjString *interned = tableFindString(&vm.strings, chars, length, hash);
  if (interned != NULL) {
    FREE_ARRAY(char, chars, length + 1);
    return interned;
  }

  return allocateString(chars, length, hash);
}

ObjString *copyString(const char *chars, int length) {
  uint32_t hash = hashString(chars, length);

  ObjString *interned = tableFindString(&vm.strings, chars, length, hash);
  if (interned != NULL) {
    TRACELN("object.copyString() found interned string");
    return interned;
  } else {
    TRACELN("object.copyString() not interned string");
  }

  char *heapChars = ALLOCATE(char, length + 1);
  memcpy(heapChars, chars, length);
  heapChars[length] = '\0';
  return allocateString(heapChars, length, hash);
}

static void printFunction(ObjFunction *function) {
  if (function->name == NULL) {
    printf("<script>");
    return;
  }

  printf("<fn %s>", function->name->chars);
}

void objectToString(Value value, char *buffer, size_t size) {
  switch (OBJ_TYPE(value)) {
  case OBJ_STRING:
    snprintf(buffer, size, "%s", AS_CSTRING(value));
    break;
  case OBJ_FUNCTION: {
    ObjFunction *function = AS_FUNCTION(value);

    if (function->name == NULL) {
      printf("<script>");
      return;
    }

    snprintf(buffer, size, "<fn %s>", function->name->chars);
    break;
  }
  case OBJ_CLOSURE: {
    ObjClosure *closure = AS_CLOSURE(value);

    if (closure->function->name == NULL) {
      printf("<script>");
      return;
    }

    snprintf(buffer, size, "<fn %s>", closure->function->name->chars);
    break;
  }
  case OBJ_NATIVE: {
    snprintf(buffer, size, "<fn native>");
    break;
  }
  }
}

void objectTypeToString(ObjType type, char *buffer, size_t size) {
  switch (type) {
  case OBJ_STRING:
    snprintf(buffer, size, "string");
    break;
  case OBJ_FUNCTION: {
    snprintf(buffer, size, "function");
    break;
  }
  case OBJ_CLOSURE: {
    snprintf(buffer, size, "function");
    break;
  }
  case OBJ_NATIVE: {
    snprintf(buffer, size, "function");
    break;
  }
  }
}

void printObject(Value value) {
  switch (OBJ_TYPE(value)) {
  case OBJ_FUNCTION:
    printFunction(AS_FUNCTION(value));
    break;
  case OBJ_NATIVE:
    printf("<native fn>");
    break;
  case OBJ_STRING:
    printf("%s", AS_CSTRING(value));
    break;
  case OBJ_CLOSURE:
    printFunction(AS_CLOSURE(value)->function);
    break;
  }
}

void printObjectToErr(Value value) {
  switch (OBJ_TYPE(value)) {
  case OBJ_STRING:
    fprintf(stderr, "%s", AS_CSTRING(value));
    break;
  case OBJ_FUNCTION:
    if (AS_FUNCTION(value)->name == NULL) {
      fprintf(stderr, "<script>");
      return;
    }

    fprintf(stderr, "<fn %s>", AS_FUNCTION(value)->name->chars);
    break;
  case OBJ_CLOSURE:
    fprintf(stderr, "<fn %s>", AS_CLOSURE(value)->function->name->chars);
    break;
  case OBJ_NATIVE:
    fprintf(stderr, "<fn native>");
    break;
  }
}