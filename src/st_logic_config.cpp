/**
 * @file st_logic_config.cpp
 * @brief Structured Text Logic Configuration Implementation
 */

#include "st_logic_config.h"
#include "st_parser.h"
#include "st_compiler.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* ============================================================================
 * INITIALIZATION
 * ============================================================================ */

void st_logic_init(st_logic_engine_state_t *state) {
  memset(state, 0, sizeof(*state));
  state->enabled = 1;
  state->execution_interval_ms = 10;  // Run every 10ms by default

  // Initialize each program
  for (int i = 0; i < 4; i++) {
    st_logic_program_config_t *prog = &state->programs[i];
    memset(prog, 0, sizeof(*prog));
    snprintf(prog->name, sizeof(prog->name), "Logic%d", i + 1);
    prog->enabled = 0;  // Disabled by default
    prog->compiled = 0;
  }
}

/* ============================================================================
 * PROGRAM UPLOAD & COMPILATION
 * ============================================================================ */

bool st_logic_upload(st_logic_engine_state_t *state, uint8_t program_id,
                      const char *source, uint32_t source_size) {
  if (program_id >= 4) return false;
  if (!source || source_size == 0 || source_size > 5000) return false;

  st_logic_program_config_t *prog = &state->programs[program_id];
  memcpy(prog->source_code, source, source_size);
  prog->source_size = source_size;
  prog->compiled = 0;  // Mark as needing compilation

  return true;
}

bool st_logic_compile(st_logic_engine_state_t *state, uint8_t program_id) {
  if (program_id >= 4) return false;

  st_logic_program_config_t *prog = &state->programs[program_id];

  if (!prog->source_code || prog->source_size == 0) {
    snprintf(prog->last_error, sizeof(prog->last_error), "No source code uploaded");
    return false;
  }

  // Parse the source code
  st_parser_t parser;
  st_parser_init(&parser, prog->source_code);
  st_program_t *program = st_parser_parse_program(&parser);

  if (!program) {
    snprintf(prog->last_error, sizeof(prog->last_error), "Parse error: %s", parser.error_msg);
    return false;
  }

  // Compile to bytecode
  st_compiler_t compiler;
  st_compiler_init(&compiler);
  st_bytecode_program_t *bytecode = st_compiler_compile(&compiler, program);

  if (!bytecode) {
    snprintf(prog->last_error, sizeof(prog->last_error), "Compile error: %s", compiler.error_msg);
    st_program_free(program);
    return false;
  }

  // Store compiled bytecode in config
  memcpy(&prog->bytecode, bytecode, sizeof(*bytecode));
  prog->compiled = 1;
  prog->execution_count = 0;
  prog->error_count = 0;

  st_program_free(program);
  free(bytecode);

  return true;
}

/* ============================================================================
 * VARIABLE BINDING
 * ============================================================================ */

bool st_logic_bind_variable(st_logic_engine_state_t *state, uint8_t program_id,
                             uint8_t st_var_index, uint16_t modbus_reg,
                             uint8_t is_input, uint8_t is_output) {
  if (program_id >= 4) return false;

  st_logic_program_config_t *prog = &state->programs[program_id];

  if (prog->binding_count >= 32) {
    return false;  // Max bindings reached
  }

  // Check for duplicate variable
  for (int i = 0; i < prog->binding_count; i++) {
    if (prog->var_bindings[i].st_var_index == st_var_index) {
      // Update existing binding
      prog->var_bindings[i].modbus_register = modbus_reg;
      prog->var_bindings[i].is_input = is_input;
      prog->var_bindings[i].is_output = is_output;
      return true;
    }
  }

  // Add new binding
  st_var_binding_t *binding = &prog->var_bindings[prog->binding_count++];
  binding->st_var_index = st_var_index;
  binding->modbus_register = modbus_reg;
  binding->is_input = is_input;
  binding->is_output = is_output;

  return true;
}

/* ============================================================================
 * PROGRAM MANAGEMENT
 * ============================================================================ */

bool st_logic_set_enabled(st_logic_engine_state_t *state, uint8_t program_id, uint8_t enabled) {
  if (program_id >= 4) return false;

  st_logic_program_config_t *prog = &state->programs[program_id];
  prog->enabled = (enabled != 0);

  return true;
}

bool st_logic_delete(st_logic_engine_state_t *state, uint8_t program_id) {
  if (program_id >= 4) return false;

  st_logic_program_config_t *prog = &state->programs[program_id];
  memset(prog, 0, sizeof(*prog));
  snprintf(prog->name, sizeof(prog->name), "Logic%d", program_id + 1);
  prog->enabled = 0;
  prog->compiled = 0;

  return true;
}

st_logic_program_config_t *st_logic_get_program(st_logic_engine_state_t *state, uint8_t program_id) {
  if (program_id >= 4) return NULL;
  return &state->programs[program_id];
}
