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
 * GLOBAL STATE
 * ============================================================================ */

// Global logic engine state - single instance for all 4 programs
static st_logic_engine_state_t g_logic_state;

// Global parser and compiler to avoid stack overflow
// (These are large structures, don't allocate on stack)
static st_parser_t g_parser;
static st_compiler_t g_compiler;

/**
 * @brief Get pointer to global logic engine state
 */
st_logic_engine_state_t *st_logic_get_state(void) {
  return &g_logic_state;
}

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

  // Parse the source code (use global parser to avoid stack overflow)
  st_parser_init(&g_parser, prog->source_code);
  st_program_t *program = st_parser_parse_program(&g_parser);

  if (!program) {
    snprintf(prog->last_error, sizeof(prog->last_error), "Parse error: %s", g_parser.error_msg);
    return false;
  }

  // Compile to bytecode (use global compiler to avoid stack overflow)
  st_compiler_init(&g_compiler);
  st_bytecode_program_t *bytecode = st_compiler_compile(&g_compiler, program);

  if (!bytecode) {
    snprintf(prog->last_error, sizeof(prog->last_error), "Compile error: %s", g_compiler.error_msg);
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
 * VARIABLE BINDING (DEPRECATED)
 *
 * NOTE: Variable binding is now handled by the unified VariableMapping system.
 * Instead of using st_logic_bind_variable(), create a VariableMapping entry:
 *
 * Example:
 *   VariableMapping map = {
 *     .source_type = MAPPING_SOURCE_ST_VAR,
 *     .st_program_id = 0,
 *     .st_var_index = 2,
 *     .is_input = 0,  // OUTPUT mode
 *     .coil_reg = 100  // Write to HR#100
 *   };
 *   g_persist_config.var_maps[g_persist_config.var_map_count++] = map;
 *   config_save();
 *
 * This eliminates code duplication and provides a single place to manage
 * all variable bindings (GPIO + ST).
 * ============================================================================ */

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
