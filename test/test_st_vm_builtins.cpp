/**
 * @file test_st_vm_builtins.cpp
 * @brief Integration Test: VM + Built-in Functions
 *
 * Tests that the VM can execute built-in function calls.
 * Demonstrates bytecode generation and execution.
 *
 * Run: g++ -o test_vm_builtins src/st_vm.cpp src/st_compiler.cpp src/st_builtins.cpp test/test_st_vm_builtins.cpp -Iinclude -lm
 */

#include <stdio.h>
#include "st_types.h"
#include "st_vm.h"
#include "st_builtins.h"

/* Helper: Create bytecode program manually (without compiler) */
st_bytecode_program_t *create_test_program_abs() {
  st_bytecode_program_t *prog = (st_bytecode_program_t *)malloc(sizeof(*prog));
  memset(prog, 0, sizeof(*prog));

  strcpy(prog->name, "Test_ABS");
  prog->var_count = 1;
  prog->enabled = 1;

  // Variable 0: result
  prog->variables[0].int_val = 0;

  // Bytecode: result := ABS(-42)
  // [0] PUSH_INT -42
  prog->instructions[0].opcode = ST_OP_PUSH_INT;
  prog->instructions[0].arg.int_arg = -42;

  // [1] CALL_BUILTIN ABS
  prog->instructions[1].opcode = ST_OP_CALL_BUILTIN;
  prog->instructions[1].arg.int_arg = (int32_t)ST_BUILTIN_ABS;

  // [2] STORE_VAR 0 (result)
  prog->instructions[2].opcode = ST_OP_STORE_VAR;
  prog->instructions[2].arg.var_index = 0;

  // [3] HALT
  prog->instructions[3].opcode = ST_OP_HALT;

  prog->instr_count = 4;
  return prog;
}

/* Create bytecode: result := MIN(10, 5) */
st_bytecode_program_t *create_test_program_min() {
  st_bytecode_program_t *prog = (st_bytecode_program_t *)malloc(sizeof(*prog));
  memset(prog, 0, sizeof(*prog));

  strcpy(prog->name, "Test_MIN");
  prog->var_count = 1;
  prog->enabled = 1;

  // Variable 0: result
  prog->variables[0].int_val = 0;

  // Bytecode: result := MIN(10, 5)
  // [0] PUSH_INT 10
  prog->instructions[0].opcode = ST_OP_PUSH_INT;
  prog->instructions[0].arg.int_arg = 10;

  // [1] PUSH_INT 5
  prog->instructions[1].opcode = ST_OP_PUSH_INT;
  prog->instructions[1].arg.int_arg = 5;

  // [2] CALL_BUILTIN MIN
  prog->instructions[2].opcode = ST_OP_CALL_BUILTIN;
  prog->instructions[2].arg.int_arg = (int32_t)ST_BUILTIN_MIN;

  // [3] STORE_VAR 0 (result)
  prog->instructions[3].opcode = ST_OP_STORE_VAR;
  prog->instructions[3].arg.var_index = 0;

  // [4] HALT
  prog->instructions[4].opcode = ST_OP_HALT;

  prog->instr_count = 5;
  return prog;
}

/* Create bytecode: result := MAX(10, 5) */
st_bytecode_program_t *create_test_program_max() {
  st_bytecode_program_t *prog = (st_bytecode_program_t *)malloc(sizeof(*prog));
  memset(prog, 0, sizeof(*prog));

  strcpy(prog->name, "Test_MAX");
  prog->var_count = 1;
  prog->enabled = 1;

  // Variable 0: result
  prog->variables[0].int_val = 0;

  // Bytecode: result := MAX(10, 5)
  // [0] PUSH_INT 10
  prog->instructions[0].opcode = ST_OP_PUSH_INT;
  prog->instructions[0].arg.int_arg = 10;

  // [1] PUSH_INT 5
  prog->instructions[1].opcode = ST_OP_PUSH_INT;
  prog->instructions[1].arg.int_arg = 5;

  // [2] CALL_BUILTIN MAX
  prog->instructions[2].opcode = ST_OP_CALL_BUILTIN;
  prog->instructions[2].arg.int_arg = (int32_t)ST_BUILTIN_MAX;

  // [3] STORE_VAR 0 (result)
  prog->instructions[3].opcode = ST_OP_STORE_VAR;
  prog->instructions[3].arg.var_index = 0;

  // [4] HALT
  prog->instructions[4].opcode = ST_OP_HALT;

  prog->instr_count = 5;
  return prog;
}

/* Create bytecode: result := INT_TO_BOOL(42) */
st_bytecode_program_t *create_test_program_conversion() {
  st_bytecode_program_t *prog = (st_bytecode_program_t *)malloc(sizeof(*prog));
  memset(prog, 0, sizeof(*prog));

  strcpy(prog->name, "Test_CONVERSION");
  prog->var_count = 1;
  prog->enabled = 1;

  // Variable 0: result
  prog->variables[0].bool_val = 0;

  // Bytecode: result := INT_TO_BOOL(42)
  // [0] PUSH_INT 42
  prog->instructions[0].opcode = ST_OP_PUSH_INT;
  prog->instructions[0].arg.int_arg = 42;

  // [1] CALL_BUILTIN INT_TO_BOOL
  prog->instructions[1].opcode = ST_OP_CALL_BUILTIN;
  prog->instructions[1].arg.int_arg = (int32_t)ST_BUILTIN_INT_TO_BOOL;

  // [2] STORE_VAR 0 (result)
  prog->instructions[2].opcode = ST_OP_STORE_VAR;
  prog->instructions[2].arg.var_index = 0;

  // [3] HALT
  prog->instructions[3].opcode = ST_OP_HALT;

  prog->instr_count = 4;
  return prog;
}

/* Test 1: ABS function */
void test_vm_builtin_abs() {
  printf("\n==== TEST 1: VM Built-in ABS ====\n");

  st_bytecode_program_t *prog = create_test_program_abs();
  printf("Program: %s\n", prog->name);
  printf("Bytecode:\n");
  printf("  [0] PUSH_INT -42\n");
  printf("  [1] CALL_BUILTIN ABS\n");
  printf("  [2] STORE_VAR 0\n");
  printf("  [3] HALT\n\n");

  st_vm_t vm;
  st_vm_init(&vm, prog);
  bool success = st_vm_run(&vm, 0);

  printf("Execution: %s\n", success ? "SUCCESS" : "FAILED");
  if (vm.error) {
    printf("Error: %s\n", vm.error_msg);
  }
  printf("Result: result = %d\n", vm.variables[0].int_val);
  printf("Expected: 42\n");
  printf("%s\n\n", (vm.variables[0].int_val == 42) ? "✓ PASS" : "✗ FAIL");

  free(prog);
}

/* Test 2: MIN function */
void test_vm_builtin_min() {
  printf("==== TEST 2: VM Built-in MIN ====\n");

  st_bytecode_program_t *prog = create_test_program_min();
  printf("Program: %s\n", prog->name);
  printf("Bytecode:\n");
  printf("  [0] PUSH_INT 10\n");
  printf("  [1] PUSH_INT 5\n");
  printf("  [2] CALL_BUILTIN MIN\n");
  printf("  [3] STORE_VAR 0\n");
  printf("  [4] HALT\n\n");

  st_vm_t vm;
  st_vm_init(&vm, prog);
  bool success = st_vm_run(&vm, 0);

  printf("Execution: %s\n", success ? "SUCCESS" : "FAILED");
  if (vm.error) {
    printf("Error: %s\n", vm.error_msg);
  }
  printf("Result: result = %d\n", vm.variables[0].int_val);
  printf("Expected: 5\n");
  printf("%s\n\n", (vm.variables[0].int_val == 5) ? "✓ PASS" : "✗ FAIL");

  free(prog);
}

/* Test 3: MAX function */
void test_vm_builtin_max() {
  printf("==== TEST 3: VM Built-in MAX ====\n");

  st_bytecode_program_t *prog = create_test_program_max();
  printf("Program: %s\n", prog->name);
  printf("Bytecode:\n");
  printf("  [0] PUSH_INT 10\n");
  printf("  [1] PUSH_INT 5\n");
  printf("  [2] CALL_BUILTIN MAX\n");
  printf("  [3] STORE_VAR 0\n");
  printf("  [4] HALT\n\n");

  st_vm_t vm;
  st_vm_init(&vm, prog);
  bool success = st_vm_run(&vm, 0);

  printf("Execution: %s\n", success ? "SUCCESS" : "FAILED");
  if (vm.error) {
    printf("Error: %s\n", vm.error_msg);
  }
  printf("Result: result = %d\n", vm.variables[0].int_val);
  printf("Expected: 10\n");
  printf("%s\n\n", (vm.variables[0].int_val == 10) ? "✓ PASS" : "✗ FAIL");

  free(prog);
}

/* Test 4: Type conversion */
void test_vm_builtin_conversion() {
  printf("==== TEST 4: VM Built-in Type Conversion ====\n");

  st_bytecode_program_t *prog = create_test_program_conversion();
  printf("Program: %s\n", prog->name);
  printf("Bytecode:\n");
  printf("  [0] PUSH_INT 42\n");
  printf("  [1] CALL_BUILTIN INT_TO_BOOL\n");
  printf("  [2] STORE_VAR 0\n");
  printf("  [3] HALT\n\n");

  st_vm_t vm;
  st_vm_init(&vm, prog);
  bool success = st_vm_run(&vm, 0);

  printf("Execution: %s\n", success ? "SUCCESS" : "FAILED");
  if (vm.error) {
    printf("Error: %s\n", vm.error_msg);
  }
  printf("Result: result = %d (BOOL)\n", vm.variables[0].bool_val);
  printf("Expected: 1 (TRUE)\n");
  printf("%s\n\n", (vm.variables[0].bool_val == 1) ? "✓ PASS" : "✗ FAIL");

  free(prog);
}

/* Main test runner */
int main() {
  printf("\n");
  printf("╔═══════════════════════════════════════════════════════════════════╗\n");
  printf("║   VM + Built-in Functions Integration Tests (Phase 2.3)          ║\n");
  printf("║         Bytecode Execution with Function Calls                   ║\n");
  printf("╚═══════════════════════════════════════════════════════════════════╝\n");

  test_vm_builtin_abs();
  test_vm_builtin_min();
  test_vm_builtin_max();
  test_vm_builtin_conversion();

  printf("╔═══════════════════════════════════════════════════════════════════╗\n");
  printf("║                    All tests completed                           ║\n");
  printf("╚═══════════════════════════════════════════════════════════════════╝\n");
  printf("\n");

  return 0;
}
