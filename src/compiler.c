#include "compiler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chunk.h"
#include "common.h"
#include "object.h"
#include "scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

static void advance();
static uint8_t identifierConstant(Token *name);
static uint8_t parseVariable(const char *errorMessage);
static void defineVariable(uint8_t global);
static void expression();
static bool check(TokenType type);
static void varDeclaration();
static void expressionStatement();
static void consume(TokenType type, const char *message);
static ParseRule *getRule(TokenType type);
static bool match(TokenType type);
static void statement();
static void printStatement();
static void synchronize();
static void declaration();
static uint8_t makeConstant(Value value);
static void emitByte(uint8_t byte);
static void emitBytes(uint8_t byte1, uint8_t byte2);
static ObjFunction *endCompiler();
static void beginScope();
static void block();
static void ifStatement();
static void patchJump(int offset);
static int emitJump(uint8_t instruction);
static void whileStatement();
static void emitLoop(int loopStart);
static void forStatement();
static void funDeclaration();
static void returnStatement();
static void emitReturn();
static void endScope();

Parser parser;
Chunk *compilingChunk;
Compiler *current = NULL;

static void initCompiler(Compiler *compiler, FunctionType functionType) {
  TRACELN("  compiler.initCompiler()");
  compiler->enclosing = current;
  compiler->function = NULL;
  compiler->type = functionType;
  compiler->localCount = 0;
  compiler->scopeDepth = 0;
  compiler->function = newFunction();
  current = compiler;

  if (functionType != TYPE_SCRIPT) {
    current->function->name =
        copyString(parser.previous.start, parser.previous.length);
  }

  Local *local = &current->locals[current->localCount++];
  local->depth = 0;
  local->name.start = "";
  local->name.length = 0;
}

/**
 * Compile source code in to a bytecode chunk for the VM
 */
ObjFunction *compile(const char *source) {
  TRACELN("  compiler.compile()");

  initScanner(source);

  Compiler compiler;
  initCompiler(&compiler, TYPE_SCRIPT);

  parser.hadError = false;
  parser.panicMode = false;

  advance();

  while (!match(TOKEN_EOF)) {
    declaration();
  }

  ObjFunction *function = endCompiler();

  return parser.hadError ? NULL : function;
}

static void declaration() {
  TRACELN("  compiler.declaration()");
  if (match(TOKEN_FUN)) {
    funDeclaration();
  } else if (match(TOKEN_VAR)) {
    varDeclaration();
  } else {
    statement();
  }

  if (parser.panicMode)
    synchronize();
}

static void statement() {
  TRACELN("  compiler.statement()");

  if (match(TOKEN_PRINT)) {
    printStatement();
  } else if (match(TOKEN_FOR)) {
    forStatement();
  } else if (match(TOKEN_IF)) {
    ifStatement();
  } else if (match(TOKEN_RETURN)) {
    returnStatement();
  } else if (match(TOKEN_WHILE)) {
    whileStatement();
  } else if (match(TOKEN_LEFT_BRACE)) {
    beginScope();
    block();
    endScope();
  } else {
    expressionStatement();
  }
}

/**
 * Get the current chunk being compiled
 */
static Chunk *currentChunk() { return &current->function->chunk; }

static void errorAt(Token *token, const char *message) {
  if (parser.panicMode)
    return;

  parser.hadError = true;

  fprintf(stderr, "[line %d] Error", token->line);

  if (token->type == TOKEN_EOF) {
    fprintf(stderr, " at end");
  } else if (token->type == TOKEN_ERROR) {
    // Nothing.
  } else {
    fprintf(stderr, " at '%.*s' (%s)", token->length, token->start,
            tokenTypeToString(token->type));
  }

  fprintf(stderr, ": %s\n", message);
  parser.hadError = true;
}

static void error(const char *message) { errorAt(&parser.previous, message); }

static void errorAtCurrent(const char *message) {
  errorAt(&parser.current, message);
}

static void advance() {
  // TRACELN(ANSI_COLOR_YELLOW "compiler.advance()" ANSI_COLOR_RESET);

  parser.previous = parser.current;

  for (;;) {
    parser.current = scanToken();

    if (parser.current.type != TOKEN_ERROR)
      break;

    errorAtCurrent(parser.current.start);
  }
}

static void parsePrecedence(Precedence precedence) {
  TRACELN("  compiler.parsePrecedence(%d)", precedence);

  advance();
  ParseFn prefixRule = getRule(parser.previous.type)->prefix;
  if (prefixRule == NULL) {
    error("Expect expression.");
    return;
  }

  bool canAssign = precedence <= PREC_ASSIGNMENT;
  prefixRule(canAssign);

  while (precedence <= getRule(parser.current.type)->precedence) {
    advance();
    ParseFn infixRule = getRule(parser.previous.type)->infix;
    infixRule(canAssign);
  }

  if (canAssign && match(TOKEN_EQUAL)) {
    error("Invalid assignment target.");
  }

  TRACELN("  compiler.parsePrecedence() end");
}

static uint8_t identifierConstant(Token *name) {
  return makeConstant(OBJ_VAL(copyString(name->start, name->length)));
}

static bool identifiersEqual(Token *a, Token *b) {
  if (a->length != b->length)
    return false;
  return memcmp(a->start, b->start, a->length) == 0;
}

static int resolveLocal(Compiler *compiler, Token *name) {
  for (int i = compiler->localCount - 1; i >= 0; i--) {
    Local *local = &compiler->locals[i];
    if (identifiersEqual(name, &local->name)) {
      if (local->depth == -1) {
        error("Can't read local variable in its own initializer");
      }

      TRACELN("  compiler.resolveLocal() -> Found local %d", i);

      return i;
    }
  }

  TRACELN("  compiler.resolveLocal() -> Could not find local");

  return -1;
}

static void addLocal(Token name) {
  if (current->localCount == UINT8_COUNT) {
    error("Too many local variables in function.");
    return;
  }

  Local *local = &current->locals[current->localCount++];
  local->name = name;
  local->depth = -1;
}

static void declareVariable() {
  if (current->scopeDepth == 0)
    return;

  Token *name = &parser.previous;
  TRACELN("  compiler.declareVariable() -> %s", AS_CSTRING(OBJ_VAL(name)));
  addLocal(*name);
}

static void and_(bool canAssign) {
  int endJump = emitJump(OP_JUMP_IF_FALSE);

  emitByte(OP_POP);
  parsePrecedence(PREC_AND);

  patchJump(endJump);
}

static uint8_t parseVariable(const char *errorMessage) {
  TRACELN("  compiler.parseVariable()");

  consume(TOKEN_IDENTIFIER, errorMessage);

  declareVariable();
  if (current->scopeDepth > 0)
    return 0;

  return identifierConstant(&parser.previous);
}

static void markInitialized() {
  if (current->scopeDepth == 0)
    return;
  current->locals[current->localCount - 1].depth = current->scopeDepth;
}

static void defineVariable(uint8_t global) {
  TRACELN("  compiler.defineVariable()");

  if (current->scopeDepth > 0) {
    markInitialized();
    return;
  }

  emitBytes(OP_DEFINE_GLOBAL, global);
}

static void consume(TokenType type, const char *message) {
  TRACELN("  compiler.consume(%s)", tokenTypeToString(type));

  if (parser.current.type == type) {
    TRACELN("  compiler.consume matches = true");
    advance();
    return;
  }

  errorAtCurrent(message);
}

static uint8_t argumentList() {
  TRACELN("  compiler.argumentList()");

  uint8_t argCount = 0;

  if (!check(TOKEN_RIGHT_PAREN)) {
    do {
      expression();
      if (argCount == 255) {
        error("Can't have more than 255 arguments.");
      }
      argCount++;
    } while (match(TOKEN_COMMA));
  }

  consume(TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");

  return argCount;
}

static bool check(TokenType type) { return parser.current.type == type; }

static bool match(TokenType type) {
  if (!check(type))
    return false;
  advance();
  return true;
}

static void expression() {
  TRACELN("  compiler.expression()");

  parsePrecedence(PREC_ASSIGNMENT);
}

static void block() {
  TRACELN("  compiler.block()");

  while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
    declaration();
  }

  consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

static void function(FunctionType type) {
  TRACELN("  compiler.function()");
  Compiler compiler;
  initCompiler(&compiler, type);
  beginScope();

  consume(TOKEN_LEFT_PAREN, "Expect '(' after function name.");

  if (!check(TOKEN_RIGHT_PAREN)) {
    do {
      current->function->arity++;
      if (current->function->arity > 255) {
        errorAtCurrent("Can't have more than 255 parameters.");
      }

      uint8_t constant = parseVariable("Expect parameter name.");

      defineVariable(constant);
    } while (match(TOKEN_COMMA));
  }

  consume(TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
  consume(TOKEN_LEFT_BRACE, "Expect '{' before function body.");
  block();

  TRACELN("  compiler.function() end");

  ObjFunction *function = endCompiler();

  emitBytes(OP_CLOSURE, makeConstant(OBJ_VAL(function)));
}

static void funDeclaration() {
  TRACELN("  compiler.funDeclaration()");

  uint8_t global = parseVariable("Expected function name");
  markInitialized();
  function(TYPE_FUNCTION);
  defineVariable(global);
}

static void varDeclaration() {
  TRACELN("  compiler.varDeclaration()");

  uint8_t global = parseVariable("Expect variable name.");

  if (match(TOKEN_EQUAL)) {
    expression();
  } else {
    emitByte(OP_NIL);
  }
  consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");

  defineVariable(global);
}

static void expressionStatement() {
  TRACELN("  compiler.expressionStatement()");

  expression();
  consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
  emitByte(OP_POP);
}

static void forStatement() {
  TRACELN("  compiler.forStatement()");

  beginScope();

  consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");

  if (match(TOKEN_SEMICOLON)) {
    // No initializer.
  } else if (match(TOKEN_VAR)) {
    varDeclaration();
  } else {
    expressionStatement();
  }

  int loopStart = currentChunk()->count;

  int exitJump = -1;
  if (!match(TOKEN_SEMICOLON)) {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");

    // Jump out of the loop if the condition is false.
    exitJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP); // Condition.
  }

  if (!match(TOKEN_RIGHT_PAREN)) {
    int bodyJump = emitJump(OP_JUMP);
    int incrementStart = currentChunk()->count;
    expression();
    emitByte(OP_POP);
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

    emitLoop(loopStart);
    loopStart = incrementStart;
    patchJump(bodyJump);
  }

  statement();
  emitLoop(loopStart);

  if (exitJump != -1) {
    patchJump(exitJump);
    emitByte(OP_POP); // Condition.
  }

  endScope();
}

static void ifStatement() {
  TRACELN("  compiler.ifStatement()");

  consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

  int thenJump = emitJump(OP_JUMP_IF_FALSE);
  emitByte(OP_POP);
  statement();

  int elseJump = emitJump(OP_JUMP);

  patchJump(thenJump);
  emitByte(OP_POP);

  if (match(TOKEN_ELSE))
    statement();

  patchJump(elseJump);
}

static void printStatement() {
  TRACELN("  compiler.printStatement()");

  expression();
  consume(TOKEN_SEMICOLON, "Expect ';' after value.");
  emitByte(OP_PRINT);
}

static void returnStatement() {
  TRACELN("  compiler.returnStatement()");

  if (current->type == TYPE_SCRIPT) {
    error("Can't return from top-level code.");
  }

  if (match(TOKEN_SEMICOLON)) {
    emitReturn();
  } else {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after return value.");
    emitByte(OP_RETURN);
  }
}

static void whileStatement() {
  TRACELN("  compiler.whileStatement()");

  int loopStart = currentChunk()->count;

  consume(TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

  int exitJump = emitJump(OP_JUMP_IF_FALSE);
  emitByte(OP_POP);
  statement();

  emitLoop(loopStart);

  patchJump(exitJump);
  emitByte(OP_POP);
}

static void synchronize() {
  parser.panicMode = false;

  while (parser.current.type != TOKEN_EOF) {
    if (parser.previous.type == TOKEN_SEMICOLON)
      return;
    switch (parser.current.type) {
    case TOKEN_CLASS:
    case TOKEN_FUN:
    case TOKEN_VAR:
    case TOKEN_FOR:
    case TOKEN_IF:
    case TOKEN_WHILE:
    case TOKEN_PRINT:
    case TOKEN_RETURN:
      return;

    default:; // Do nothing.
    }

    advance();
  }
}

static void grouping(bool canAssign) {
  TRACELN("  compiler.grouping()");

  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void emitByte(uint8_t byte) {
  // #ifdef DEBUG_PRINT_CODE
  //   TRACELN("  compiler.emitByte() -> %s", disassembleByte(byte));
  // #endif
  writeChunk(currentChunk(), byte, parser.previous.line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2) {
  emitByte(byte1);
  emitByte(byte2);
}

static void emitLoop(int loopStart) {
  emitByte(OP_LOOP);

  int offset = currentChunk()->count - loopStart + 2;
  if (offset > UINT16_MAX)
    error("Loop body too large.");

  emitByte((offset >> 8) & 0xff);
  emitByte(offset & 0xff);
}

static int emitJump(uint8_t instruction) {
  TRACELN("  compiler.emitJump()");

  emitByte(instruction);
  emitBytes(0xff, 0xff);
  return currentChunk()->count - 2;
}

static void binary(bool canAssign) {
  TRACELN("  compiler.binary()");

  TokenType operatorType = parser.previous.type;
  ParseRule *rule = getRule(operatorType);
  parsePrecedence((Precedence)(rule->precedence + 1));

  switch (operatorType) {
  case TOKEN_BANG_EQUAL:
    emitBytes(OP_EQUAL, OP_NOT);
    break;
  case TOKEN_GREATER:
    emitByte(OP_GREATER);
    break;
  case TOKEN_GREATER_EQUAL:
    emitBytes(OP_LESS, OP_NOT);
    break;
  case TOKEN_LESS:
    emitByte(OP_LESS);
    break;
  case TOKEN_LESS_EQUAL:
    emitBytes(OP_GREATER, OP_NOT);
    break;
  case TOKEN_EQUAL_EQUAL:
    emitByte(OP_EQUAL);
    break;
  case TOKEN_PLUS:
    emitByte(OP_ADD);
    break;
  case TOKEN_MINUS:
    emitByte(OP_SUBTRACT);
    break;
  case TOKEN_STAR:
    emitByte(OP_MULTIPLY);
    break;
  case TOKEN_SLASH:
    emitByte(OP_DIVIDE);
    break;
  default:
    return; // Unreachable.
  }
}

static void call(bool canAssign) {
  TRACELN("  compiler.call()");

  uint8_t argCount = argumentList();
  emitBytes(OP_CALL, argCount);
}

static void literal(bool canAssign) {
  switch (parser.previous.type) {
  case TOKEN_FALSE:
    TRACELN("  compiler.literal() -> false");
    emitByte(OP_FALSE);
    break;
  case TOKEN_NIL:
    TRACELN("  compiler.literal() -> nil");
    emitByte(OP_NIL);
    break;
  case TOKEN_TRUE:
    TRACELN("  compiler.literal() -> true");
    emitByte(OP_TRUE);
    break;
  default:
    return; // Unreachable.
  }
}

static void unary(bool canAssign) {
  TRACELN("  compiler.unary()");

  TokenType operatorType = parser.previous.type;

  parsePrecedence(PREC_UNARY);

  switch (operatorType) {
  case TOKEN_BANG:
    emitByte(OP_NOT);
    break;
  case TOKEN_MINUS:
    emitByte(OP_NEGATE);
    break;
  default:
    return;
  }
}

static uint8_t makeConstant(Value value) {
  int constant = addConstantToChunk(currentChunk(), value);

  if (constant > UINT8_MAX) {
    error("Too many constants in one chunk.");
    return 0;
  }

  if (IS_STRING(value)) {
    TRACELN("  compiler.makeConstant() -> %d = %s", constant,
            AS_CSTRING(value));
  } else {
    TRACELN("  compiler.makeConstant(%d) -> %d", value.type, constant);
  }

  return (uint8_t)constant;
}

static void emitConstant(Value value) {
  TRACELN("  compiler.emitConstant()");

  emitBytes(OP_CONSTANT, makeConstant(value));
}

static void patchJump(int offset) {
  int jump = currentChunk()->count - offset - 2;

  if (jump > UINT16_MAX) {
    error("Too much code to jump over");
  }

  currentChunk()->code[offset] = (jump >> 8) & 0xff;
  currentChunk()->code[offset + 1] = jump & 0xff;
}

static void number(bool canAssign) {
  TRACELN("  compiler.number()");

  double value = strtod(parser.previous.start, NULL);
  emitConstant(NUMBER_VAL(value));
}

static void or_(bool canAssign) {
  int elseJump = emitJump(OP_JUMP_IF_FALSE);
  int endJump = emitJump(OP_JUMP);

  patchJump(elseJump);
  emitByte(OP_POP);

  parsePrecedence(PREC_OR);
  patchJump(endJump);
}

static void string(bool canAssign) {
  TRACELN("  compiler.string()");

  const char *chars = parser.previous.start + 1;
  int length = parser.previous.length - 2;

  emitConstant(OBJ_VAL(copyString(chars, length)));
}

static void namedVariable(Token name, bool canAssign) {
  TRACELN("  compiler.namedVariable()");

  uint8_t getOp, setOp;
  int arg = resolveLocal(current, &name);
  if (arg != -1) {
    TRACELN("  compiler.namedVariable() is local");
    getOp = OP_GET_LOCAL;
    setOp = OP_SET_LOCAL;
  } else {
    TRACELN("  compiler.namedVariable() is global");
    arg = identifierConstant(&name);
    getOp = OP_GET_GLOBAL;
    setOp = OP_SET_GLOBAL;
  }

  if (canAssign && match(TOKEN_EQUAL)) {
    expression();
    emitBytes(setOp, (uint8_t)arg);
  } else {
    emitBytes(getOp, (uint8_t)arg);
  }
}

static void variable(bool canAssign) {
  TRACELN("  compiler.variable()");
  namedVariable(parser.previous, canAssign);
}

ParseRule rules[] = {
    [TOKEN_LEFT_PAREN] = {grouping, call, PREC_CALL},
    [TOKEN_RIGHT_PAREN] = {NULL, NULL, PREC_NONE},
    [TOKEN_LEFT_BRACE] = {NULL, NULL, PREC_NONE},
    [TOKEN_RIGHT_BRACE] = {NULL, NULL, PREC_NONE},
    [TOKEN_COMMA] = {NULL, NULL, PREC_NONE},
    [TOKEN_DOT] = {NULL, NULL, PREC_NONE},
    [TOKEN_MINUS] = {unary, binary, PREC_TERM},
    [TOKEN_PLUS] = {NULL, binary, PREC_TERM},
    [TOKEN_SEMICOLON] = {NULL, NULL, PREC_NONE},
    [TOKEN_SLASH] = {NULL, binary, PREC_FACTOR},
    [TOKEN_STAR] = {NULL, binary, PREC_FACTOR},
    [TOKEN_BANG] = {unary, NULL, PREC_NONE},
    [TOKEN_BANG_EQUAL] = {NULL, binary, PREC_EQUALITY},
    [TOKEN_EQUAL] = {NULL, NULL, PREC_NONE},
    [TOKEN_EQUAL_EQUAL] = {NULL, binary, PREC_EQUALITY},
    [TOKEN_GREATER] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_GREATER_EQUAL] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_LESS] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_LESS_EQUAL] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_IDENTIFIER] = {variable, NULL, PREC_NONE},
    [TOKEN_STRING] = {string, NULL, PREC_NONE},
    [TOKEN_NUMBER] = {number, NULL, PREC_NONE},
    [TOKEN_AND] = {NULL, and_, PREC_AND},
    [TOKEN_CLASS] = {NULL, NULL, PREC_NONE},
    [TOKEN_ELSE] = {NULL, NULL, PREC_NONE},
    [TOKEN_FALSE] = {literal, NULL, PREC_NONE},
    [TOKEN_FOR] = {NULL, NULL, PREC_NONE},
    [TOKEN_FUN] = {NULL, NULL, PREC_NONE},
    [TOKEN_IF] = {NULL, NULL, PREC_NONE},
    [TOKEN_NIL] = {literal, NULL, PREC_NONE},
    [TOKEN_OR] = {NULL, or_, PREC_OR},
    [TOKEN_PRINT] = {NULL, NULL, PREC_NONE},
    [TOKEN_RETURN] = {NULL, NULL, PREC_NONE},
    [TOKEN_SUPER] = {NULL, NULL, PREC_NONE},
    [TOKEN_THIS] = {NULL, NULL, PREC_NONE},
    [TOKEN_TRUE] = {literal, NULL, PREC_NONE},
    [TOKEN_VAR] = {NULL, NULL, PREC_NONE},
    [TOKEN_WHILE] = {NULL, NULL, PREC_NONE},
    [TOKEN_ERROR] = {NULL, NULL, PREC_NONE},
    [TOKEN_EOF] = {NULL, NULL, PREC_NONE},
};

static ParseRule *getRule(TokenType type) { return &rules[type]; }

static void emitReturn() {
  TRACELN("  compiler.emitReturn()");
  emitByte(OP_NIL);
  emitByte(OP_RETURN);
}

static ObjFunction *endCompiler() {
  TRACELN("  compiler.endCompiler()");

  emitReturn();

  ObjFunction *function = current->function;

#ifdef DEBUG_PRINT_CODE
  if (!parser.hadError) {
    disassembleChunk(currentChunk(), function->name != NULL
                                         ? function->name->chars
                                         : "<script>");
  }
#endif

  current = current->enclosing;

  return function;
}

static void beginScope() { current->scopeDepth++; }

static void cleanLocalsInScope() {
  while (current->localCount > 0 &&
         current->locals[current->localCount - 1].depth > current->scopeDepth) {
    emitByte(OP_POP);
    current->localCount--;
  }
}

static void endScope() {
  current->scopeDepth--;

  cleanLocalsInScope();
}
