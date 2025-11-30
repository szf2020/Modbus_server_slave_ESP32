/**
 * @file test_st_vm.cpp
 * @brief Test Structured Text Virtual Machine
 *
 * End-to-end tests: Lexer → Parser → Compiler → VM Execution
 * Run: g++ -o test_vm src/st_lexer.cpp src/st_parser.cpp src/st_compiler.cpp src/st_vm.cpp test/test_st_vm.cpp -Iinclude -lm
 */

#include <stdio.h>
#include <string.h>
#include "st_types.h"
#include "st_lexer.h"
#include "st_parser.h"
#include "st_compiler.h"
#include "st_vm.h"

/* Helper function: compile and run ST code */
st_bytecode_program_t *st_compile(const char *code) {
  st_parser_t parser;
  st_parser_init(&parser, code);
  st_program_t *program = st_parser_parse_program(&parser);
  if (!program) {
    printf("Parse error: %s\n", parser.error_msg);
    return NULL;
  }

  st_compiler_t compiler;
  st_compiler_init(&compiler);
  st_bytecode_program_t *bytecode = st_compiler_compile(&compiler, program);
  if (!bytecode) {
    printf("Compile error: %s\n", compiler.error_msg);
    st_program_free(program);
    return NULL;
  }

  st_program_free(program);
  return bytecode;
}

/* Test 1: Simple assignment and variable access */
void test_vm_assignment() {
  printf("==== TEST 1: Assignment & Variables ====\n");
  const char *code = "VAR x: INT; y: INT; END_VAR x := 5; y := x;";
  printf("Code: %s\n\n", code);

  st_bytecode_program_t *bytecode = st_compile(code);
  if (!bytecode) return;

  st_bytecode_print(bytecode);

  st_vm_t vm;
  st_vm_init(&vm, bytecode);
  st_vm_run(&vm, 0);

  printf("After execution:\n");
  st_vm_print_variables(&vm);

  printf("Result: x=%d, y=%d\n", vm.variables[0].int_val, vm.variables[1].int_val);
  printf("Expected: x=5, y=5\n");
  printf("%s\n\n", (vm.variables[0].int_val == 5 && vm.variables[1].int_val == 5) ? "✓ PASS" : "✗ FAIL");

  free(bytecode);
}

/* Test 2: Arithmetic operations */
void test_vm_arithmetic() {
  printf("==== TEST 2: Arithmetic Operations ====\n");
  const char *code = "VAR a: INT; b: INT; result: INT; END_VAR a := 10; b := 3; result := a + b;";
  printf("Code: %s\n\n", code);

  st_bytecode_program_t *bytecode = st_compile(code);
  if (!bytecode) return;

  st_vm_t vm;
  st_vm_init(&vm, bytecode);
  st_vm_run(&vm, 0);

  printf("After execution:\n");
  st_vm_print_variables(&vm);

  printf("Result: a=%d, b=%d, result=%d\n", vm.variables[0].int_val, vm.variables[1].int_val, vm.variables[2].int_val);
  printf("Expected: a=10, b=3, result=13\n");
  printf("%s\n\n", (vm.variables[2].int_val == 13) ? "✓ PASS" : "✗ FAIL");

  free(bytecode);
}

/* Test 3: Comparison and IF statement */
void test_vm_if_statement() {
  printf("==== TEST 3: IF Statement ====\n");
  const char *code = "VAR x: INT; result: INT; END_VAR x := 15; IF x > 10 THEN result := 1; ELSE result := 0; END_IF;";
  printf("Code: %s\n\n", code);

  st_bytecode_program_t *bytecode = st_compile(code);
  if (!bytecode) return;

  st_bytecode_print(bytecode);

  st_vm_t vm;
  st_vm_init(&vm, bytecode);
  st_vm_run(&vm, 0);

  printf("After execution:\n");
  st_vm_print_variables(&vm);

  printf("Result: x=%d, result=%d\n", vm.variables[0].int_val, vm.variables[1].int_val);
  printf("Expected: x=15, result=1 (condition true)\n");
  printf("%s\n\n", (vm.variables[1].int_val == 1) ? "✓ PASS" : "✗ FAIL");

  free(bytecode);
}

/* Test 4: IF with false condition */
void test_vm_if_false() {
  printf("==== TEST 4: IF Statement (False Condition) ====\n");
  const char *code = "VAR x: INT; result: INT; END_VAR x := 5; IF x > 10 THEN result := 1; ELSE result := 0; END_IF;";
  printf("Code: %s\n\n", code);

  st_bytecode_program_t *bytecode = st_compile(code);
  if (!bytecode) return;

  st_vm_t vm;
  st_vm_init(&vm, bytecode);
  st_vm_run(&vm, 0);

  printf("After execution:\n");
  st_vm_print_variables(&vm);

  printf("Result: x=%d, result=%d\n", vm.variables[0].int_val, vm.variables[1].int_val);
  printf("Expected: x=5, result=0 (condition false)\n");
  printf("%s\n\n", (vm.variables[1].int_val == 0) ? "✓ PASS" : "✗ FAIL");

  free(bytecode);
}

/* Test 5: Nested IF statements */
void test_vm_nested_if() {
  printf("==== TEST 5: Nested IF Statements ====\n");
  const char *code =
    "VAR x: INT; result: INT; END_VAR "
    "x := 15; "
    "IF x > 10 THEN "
    "  IF x > 20 THEN result := 2; ELSE result := 1; END_IF; "
    "ELSE "
    "  result := 0; "
    "END_IF;";
  printf("Code: %s\n\n", code);

  st_bytecode_program_t *bytecode = st_compile(code);
  if (!bytecode) return;

  st_vm_t vm;
  st_vm_init(&vm, bytecode);
  st_vm_run(&vm, 0);

  printf("After execution:\n");
  st_vm_print_variables(&vm);

  printf("Result: x=%d, result=%d\n", vm.variables[0].int_val, vm.variables[1].int_val);
  printf("Expected: x=15, result=1 (15 > 10 but not > 20)\n");
  printf("%s\n\n", (vm.variables[1].int_val == 1) ? "✓ PASS" : "✗ FAIL");

  free(bytecode);
}

/* Test 6: Logical AND operation */
void test_vm_logical_and() {
  printf("==== TEST 6: Logical AND ====\n");
  const char *code = "VAR a: INT; b: INT; result: INT; END_VAR a := 10; b := 5; IF a > 5 AND b > 3 THEN result := 1; ELSE result := 0; END_IF;";
  printf("Code: %s\n\n", code);

  st_bytecode_program_t *bytecode = st_compile(code);
  if (!bytecode) return;

  st_vm_t vm;
  st_vm_init(&vm, bytecode);
  st_vm_run(&vm, 0);

  printf("After execution:\n");
  st_vm_print_variables(&vm);

  printf("Result: a=%d, b=%d, result=%d\n", vm.variables[0].int_val, vm.variables[1].int_val, vm.variables[2].int_val);
  printf("Expected: result=1 (both conditions true)\n");
  printf("%s\n\n", (vm.variables[2].int_val == 1) ? "✓ PASS" : "✗ FAIL");

  free(bytecode);
}

/* Test 7: WHILE loop */
void test_vm_while_loop() {
  printf("==== TEST 7: WHILE Loop ====\n");
  const char *code = "VAR count: INT; sum: INT; END_VAR count := 0; sum := 0; WHILE count < 5 DO sum := sum + 1; count := count + 1; END_WHILE;";
  printf("Code: %s\n\n", code);

  st_bytecode_program_t *bytecode = st_compile(code);
  if (!bytecode) return;

  st_vm_t vm;
  st_vm_init(&vm, bytecode);
  st_vm_run(&vm, 100);  // Max 100 steps to prevent infinite loops

  printf("After execution:\n");
  st_vm_print_variables(&vm);

  printf("Result: count=%d, sum=%d\n", vm.variables[0].int_val, vm.variables[1].int_val);
  printf("Expected: count=5, sum=5\n");
  printf("%s\n\n", (vm.variables[0].int_val == 5 && vm.variables[1].int_val == 5) ? "✓ PASS" : "✗ FAIL");

  free(bytecode);
}

/* Test 8: REPEAT loop */
void test_vm_repeat_loop() {
  printf("==== TEST 8: REPEAT Loop ====\n");
  const char *code = "VAR i: INT; END_VAR i := 0; REPEAT i := i + 1; UNTIL i >= 3 END_REPEAT;";
  printf("Code: %s\n\n", code);

  st_bytecode_program_t *bytecode = st_compile(code);
  if (!bytecode) return;

  st_vm_t vm;
  st_vm_init(&vm, bytecode);
  st_vm_run(&vm, 100);

  printf("After execution:\n");
  st_vm_print_variables(&vm);

  printf("Result: i=%d\n", vm.variables[0].int_val);
  printf("Expected: i=3 (executed 3 times: 0→1→2→3)\n");
  printf("%s\n\n", (vm.variables[0].int_val == 3) ? "✓ PASS" : "✗ FAIL");

  free(bytecode);
}

/* Main test runner */
int main() {
  printf("\n");
  printf("╔═══════════════════════════════════════════════════════════════════╗\n");
  printf("║  Structured Text Virtual Machine Execution Tests (Phase 2.2)     ║\n");
  printf("║         Lexer → Parser → Compiler → VM Execution                 ║\n");
  printf("╚═══════════════════════════════════════════════════════════════════╝\n");
  printf("\n");

  test_vm_assignment();
  test_vm_arithmetic();
  test_vm_if_statement();
  test_vm_if_false();
  test_vm_nested_if();
  test_vm_logical_and();
  test_vm_while_loop();
  test_vm_repeat_loop();

  printf("╔═══════════════════════════════════════════════════════════════════╗\n");
  printf("║                    All tests completed                           ║\n");
  printf("╚═══════════════════════════════════════════════════════════════════╝\n");
  printf("\n");

  return 0;
}
