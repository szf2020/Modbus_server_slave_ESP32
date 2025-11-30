/**
 * @file st_logic_engine.cpp
 * @brief Structured Text Logic Engine Implementation
 *
 * Main execution loop integrating ST bytecode with Modbus I/O.
 */

#include "st_logic_engine.h"
#include "st_compiler.h"
#include "st_parser.h"
#include "st_vm.h"
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * INPUT/OUTPUT OPERATIONS (Modbus ↔ ST Variables)
 * ============================================================================ */

bool st_logic_read_inputs(st_logic_engine_state_t *state, uint8_t program_id,
                           uint16_t *holding_regs) {
  st_logic_program_config_t *prog = st_logic_get_program(state, program_id);
  if (!prog || !prog->compiled) return false;

  // For each variable binding marked as INPUT
  for (int i = 0; i < prog->binding_count; i++) {
    st_var_binding_t *binding = &prog->var_bindings[i];

    if (!binding->is_input) continue;  // Skip non-input bindings

    if (binding->modbus_register >= 160) continue;  // Bounds check

    // Read from Modbus register, store to ST variable
    // Note: Simplified - just copy int value. Real implementation would handle type conversion
    prog->bytecode.variables[binding->st_var_index].int_val = holding_regs[binding->modbus_register];
  }

  return true;
}

bool st_logic_write_outputs(st_logic_engine_state_t *state, uint8_t program_id,
                             uint16_t *holding_regs) {
  st_logic_program_config_t *prog = st_logic_get_program(state, program_id);
  if (!prog || !prog->compiled) return false;

  // For each variable binding marked as OUTPUT
  for (int i = 0; i < prog->binding_count; i++) {
    st_var_binding_t *binding = &prog->var_bindings[i];

    if (!binding->is_output) continue;  // Skip non-output bindings

    if (binding->modbus_register >= 160) continue;  // Bounds check

    // Write from ST variable to Modbus register
    // Note: Simplified - just copy int value. Real implementation would handle type conversion
    holding_regs[binding->modbus_register] = prog->bytecode.variables[binding->st_var_index].int_val;
  }

  return true;
}

/* ============================================================================
 * PROGRAM EXECUTION
 * ============================================================================ */

bool st_logic_execute_program(st_logic_engine_state_t *state, uint8_t program_id) {
  st_logic_program_config_t *prog = st_logic_get_program(state, program_id);
  if (!prog || !prog->compiled || !prog->enabled) return false;

  // Create VM and initialize with bytecode
  st_vm_t vm;
  st_vm_init(&vm, &prog->bytecode);

  // Execute until halt or error (max 10000 steps for safety)
  bool success = st_vm_run(&vm, 10000);

  // Copy variables back from VM to program config
  memcpy(prog->bytecode.variables, vm.variables, vm.var_count * sizeof(st_value_t));

  // Update statistics
  prog->execution_count++;
  if (!success || vm.error) {
    prog->error_count++;
    snprintf(prog->last_error, sizeof(prog->last_error), "%s", vm.error_msg);
    return false;
  }

  prog->last_execution_ms = 0;  // TODO: Add timestamp
  return true;
}

/* ============================================================================
 * MAIN LOGIC ENGINE LOOP
 * ============================================================================ */

bool st_logic_engine_loop(st_logic_engine_state_t *state,
                           uint16_t *holding_regs, uint16_t *input_regs) {
  if (!state || !state->enabled) return true;  // Logic mode disabled

  bool all_success = true;

  // Execute each program in sequence
  for (int prog_id = 0; prog_id < 4; prog_id++) {
    st_logic_program_config_t *prog = &state->programs[prog_id];

    if (!prog->enabled || !prog->compiled) continue;

    // Phase 1: Read inputs from Modbus
    st_logic_read_inputs(state, prog_id, holding_regs);

    // Phase 2: Execute program bytecode
    bool success = st_logic_execute_program(state, prog_id);
    if (!success) {
      all_success = false;
      // Continue executing other programs despite error
    }

    // Phase 3: Write outputs to Modbus
    st_logic_write_outputs(state, prog_id, holding_regs);
  }

  return all_success;
}

/* ============================================================================
 * DEBUGGING & STATUS
 * ============================================================================ */

void st_logic_print_status(st_logic_engine_state_t *state) {
  printf("\n=== Logic Engine Status ===\n");
  printf("Enabled: %s\n", state->enabled ? "YES" : "NO");
  printf("Execution Interval: %ums\n", state->execution_interval_ms);
  printf("\nPrograms:\n");

  for (int i = 0; i < 4; i++) {
    st_logic_program_config_t *prog = &state->programs[i];
    printf("  %s:\n", prog->name);
    printf("    Enabled: %s\n", prog->enabled ? "YES" : "NO");
    printf("    Compiled: %s\n", prog->compiled ? "YES" : "NO");
    printf("    Source: %d bytes\n", prog->source_size);
    printf("    Executions: %u (errors: %u)\n", prog->execution_count, prog->error_count);
    if (prog->error_count > 0) {
      printf("    Last error: %s\n", prog->last_error);
    }
  }

  printf("\n");
}

void st_logic_print_program(st_logic_engine_state_t *state, uint8_t program_id) {
  st_logic_program_config_t *prog = st_logic_get_program(state, program_id);
  if (!prog) {
    printf("Invalid program ID: %d\n", program_id);
    return;
  }

  printf("\n=== Logic Program: %s ===\n", prog->name);
  printf("Enabled: %s\n", prog->enabled ? "YES" : "NO");
  printf("Compiled: %s\n", prog->compiled ? "YES" : "NO");
  printf("Source Code: %d bytes\n", prog->source_size);

  if (prog->source_size > 0) {
    printf("\nSource:\n");
    // Print first 500 chars of source
    int chars = (prog->source_size > 500) ? 500 : prog->source_size;
    printf("%.*s\n", chars, prog->source_code);
    if (prog->source_size > 500) {
      printf("... (%d more bytes)\n", prog->source_size - 500);
    }
  }

  printf("\nVariable Bindings: %d\n", prog->binding_count);
  for (int i = 0; i < prog->binding_count; i++) {
    st_var_binding_t *binding = &prog->var_bindings[i];
    printf("  [%d] ST var[%d] %s Modbus HR#%d\n",
           i, binding->st_var_index,
           (binding->is_input && binding->is_output) ? "↔" :
           (binding->is_input) ? "←" : "→",
           binding->modbus_register);
  }

  printf("\nStatistics:\n");
  printf("  Executions: %u\n", prog->execution_count);
  printf("  Errors: %u\n", prog->error_count);

  if (prog->compiled) {
    printf("\nCompiled Bytecode: %d instructions\n", prog->bytecode.instr_count);
  }

  if (prog->error_count > 0) {
    printf("\nLast Error: %s\n", prog->last_error);
  }

  printf("\n");
}
