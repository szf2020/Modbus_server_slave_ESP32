/**
 * @file test_st_compiler.cpp
 * @brief Test Structured Text Bytecode Compiler
 *
 * Tests compilation of ST AST to bytecode.
 * Run: g++ -o test_compiler src/st_lexer.cpp src/st_parser.cpp src/st_compiler.cpp test/test_st_compiler.cpp -Iinclude
 */

#include <stdio.h>
#include <string.h>
#include "st_types.h"
#include "st_lexer.h"
#include "st_parser.h"
#include "st_compiler.h"

/* Test 1: Compile simple assignment */
void test_compile_assignment() {
  printf("==== TEST 1: Compile Simple Assignment ====\n");
  const char *code = "VAR x: INT; y: INT; END_VAR x := 5; y := x + 1;";

  printf("Code: %s\n\n", code);

  st_parser_t parser;
  st_parser_init(&parser, code);
  st_program_t *program = st_parser_parse_program(&parser);

  if (!program) {
    printf("Parse error: %s\n\n", parser.error_msg);
    return;
  }

  st_compiler_t compiler;
  st_compiler_init(&compiler);
  st_bytecode_program_t *bytecode = st_compiler_compile(&compiler, program);

  if (!bytecode) {
    printf("Compile error: %s\n\n", compiler.error_msg);
    st_program_free(program);
    return;
  }

  printf("SUCCESS: Compiled %d instructions\n", bytecode->instr_count);
  st_bytecode_print(bytecode);

  st_program_free(program);
  free(bytecode);
  printf("\n");
}

/* Test 2: Compile IF statement */
void test_compile_if() {
  printf("==== TEST 2: Compile IF Statement ====\n");
  const char *code = "VAR x: INT; y: INT; END_VAR IF x > 10 THEN y := 1; ELSE y := 0; END_IF;";

  printf("Code: %s\n\n", code);

  st_parser_t parser;
  st_parser_init(&parser, code);
  st_program_t *program = st_parser_parse_program(&parser);

  if (!program) {
    printf("Parse error: %s\n\n", parser.error_msg);
    return;
  }

  st_compiler_t compiler;
  st_compiler_init(&compiler);
  st_bytecode_program_t *bytecode = st_compiler_compile(&compiler, program);

  if (!bytecode) {
    printf("Compile error: %s\n\n", compiler.error_msg);
    st_program_free(program);
    return;
  }

  printf("SUCCESS: Compiled %d instructions\n", bytecode->instr_count);
  st_bytecode_print(bytecode);

  st_program_free(program);
  free(bytecode);
  printf("\n");
}

/* Test 3: Compile FOR loop */
void test_compile_for() {
  printf("==== TEST 3: Compile FOR Loop ====\n");
  const char *code = "VAR i: INT; sum: INT; END_VAR FOR i := 1 TO 10 DO sum := sum + i; END_FOR;";

  printf("Code: %s\n\n", code);

  st_parser_t parser;
  st_parser_init(&parser, code);
  st_program_t *program = st_parser_parse_program(&parser);

  if (!program) {
    printf("Parse error: %s\n\n", parser.error_msg);
    return;
  }

  st_compiler_t compiler;
  st_compiler_init(&compiler);
  st_bytecode_program_t *bytecode = st_compiler_compile(&compiler, program);

  if (!bytecode) {
    printf("Compile error: %s\n\n", compiler.error_msg);
    st_program_free(program);
    return;
  }

  printf("SUCCESS: Compiled %d instructions\n", bytecode->instr_count);
  st_bytecode_print(bytecode);

  st_program_free(program);
  free(bytecode);
  printf("\n");
}

/* Test 4: Compile WHILE loop */
void test_compile_while() {
  printf("==== TEST 4: Compile WHILE Loop ====\n");
  const char *code = "VAR count: INT; END_VAR WHILE count < 100 DO count := count + 1; END_WHILE;";

  printf("Code: %s\n\n", code);

  st_parser_t parser;
  st_parser_init(&parser, code);
  st_program_t *program = st_parser_parse_program(&parser);

  if (!program) {
    printf("Parse error: %s\n\n", parser.error_msg);
    return;
  }

  st_compiler_t compiler;
  st_compiler_init(&compiler);
  st_bytecode_program_t *bytecode = st_compiler_compile(&compiler, program);

  if (!bytecode) {
    printf("Compile error: %s\n\n", compiler.error_msg);
    st_program_free(program);
    return;
  }

  printf("SUCCESS: Compiled %d instructions\n", bytecode->instr_count);
  st_bytecode_print(bytecode);

  st_program_free(program);
  free(bytecode);
  printf("\n");
}

/* Test 5: Compile REPEAT loop */
void test_compile_repeat() {
  printf("==== TEST 5: Compile REPEAT Loop ====\n");
  const char *code = "VAR i: INT; END_VAR REPEAT i := i + 1; UNTIL i > 5 END_REPEAT;";

  printf("Code: %s\n\n", code);

  st_parser_t parser;
  st_parser_init(&parser, code);
  st_program_t *program = st_parser_parse_program(&parser);

  if (!program) {
    printf("Parse error: %s\n\n", parser.error_msg);
    return;
  }

  st_compiler_t compiler;
  st_compiler_init(&compiler);
  st_bytecode_program_t *bytecode = st_compiler_compile(&compiler, program);

  if (!bytecode) {
    printf("Compile error: %s\n\n", compiler.error_msg);
    st_program_free(program);
    return;
  }

  printf("SUCCESS: Compiled %d instructions\n", bytecode->instr_count);
  st_bytecode_print(bytecode);

  st_program_free(program);
  free(bytecode);
  printf("\n");
}

/* Test 6: Complex expression */
void test_compile_complex_expr() {
  printf("==== TEST 6: Compile Complex Expression ====\n");
  const char *code = "VAR a: INT; b: INT; result: INT; END_VAR result := (a + b) * 2 > 10 AND a <> 0;";

  printf("Code: %s\n\n", code);

  st_parser_t parser;
  st_parser_init(&parser, code);
  st_program_t *program = st_parser_parse_program(&parser);

  if (!program) {
    printf("Parse error: %s\n\n", parser.error_msg);
    return;
  }

  st_compiler_t compiler;
  st_compiler_init(&compiler);
  st_bytecode_program_t *bytecode = st_compiler_compile(&compiler, program);

  if (!bytecode) {
    printf("Compile error: %s\n\n", compiler.error_msg);
    st_program_free(program);
    return;
  }

  printf("SUCCESS: Compiled %d instructions\n", bytecode->instr_count);
  st_bytecode_print(bytecode);

  st_program_free(program);
  free(bytecode);
  printf("\n");
}

/* Main test runner */
int main() {
  printf("\n");
  printf("╔═══════════════════════════════════════════════════════════════════╗\n");
  printf("║     Structured Text (IEC 61131-3) Bytecode Compiler Tests        ║\n");
  printf("║                       Phase 2.1 Demo                             ║\n");
  printf("╚═══════════════════════════════════════════════════════════════════╝\n");
  printf("\n");

  test_compile_assignment();
  test_compile_if();
  test_compile_for();
  test_compile_while();
  test_compile_repeat();
  test_compile_complex_expr();

  printf("╔═══════════════════════════════════════════════════════════════════╗\n");
  printf("║                    All tests completed                           ║\n");
  printf("╚═══════════════════════════════════════════════════════════════════╝\n");
  printf("\n");

  return 0;
}
