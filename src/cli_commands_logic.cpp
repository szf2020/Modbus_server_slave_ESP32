/**
 * @file cli_commands_logic.cpp
 * @brief CLI Commands for Structured Text Logic Mode (Fase 4)
 *
 * Commands for managing logic programs:
 *   set logic <id> upload <code>
 *   set logic <id> enabled:true|false
 *   set logic <id> delete
 *   set logic <id> bind <var_index> <register> [input|output|both]
 *   show logic <id>
 *   show logic all
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Forward declarations - from existing CLI infrastructure */
extern void debug_println(const char *msg);
extern void debug_printf(const char *fmt, ...);

/* ============================================================================
 * COMMAND HANDLERS
 * ============================================================================ */

/**
 * @brief set logic <id> upload "<st_code>"
 *
 * Upload ST source code for a logic program
 *
 * Example:
 *   set logic 1 upload "VAR x: INT; END_VAR IF x > 10 THEN x := 1; END_IF;"
 */
int cli_cmd_set_logic_upload(st_logic_engine_state_t *logic_state, uint8_t program_id,
                             const char *source_code) {
  if (program_id >= 4) {
    debug_println("ERROR: Invalid program ID (0-3)");
    return -1;
  }

  if (!source_code || strlen(source_code) == 0) {
    debug_println("ERROR: Source code cannot be empty");
    return -1;
  }

  // Upload source
  if (!st_logic_upload(logic_state, program_id, source_code, strlen(source_code))) {
    debug_println("ERROR: Failed to upload source code");
    return -1;
  }

  // Try to compile immediately
  if (!st_logic_compile(logic_state, program_id)) {
    st_logic_program_config_t *prog = st_logic_get_program(logic_state, program_id);
    debug_printf("INFO: Compile error: %s\n", prog->last_error);
    return -1;
  }

  debug_printf("✓ Logic%d compiled successfully (%d bytes, %d instructions)\n",
               program_id + 1, strlen(source_code), logic_state->programs[program_id].bytecode.instr_count);

  return 0;
}

/**
 * @brief set logic <id> enabled:true|false
 *
 * Enable or disable a logic program
 *
 * Example:
 *   set logic 1 enabled:true
 */
int cli_cmd_set_logic_enabled(st_logic_engine_state_t *logic_state, uint8_t program_id,
                              uint8_t enabled) {
  if (program_id >= 4) {
    debug_println("ERROR: Invalid program ID (0-3)");
    return -1;
  }

  st_logic_program_config_t *prog = st_logic_get_program(logic_state, program_id);
  if (!prog->compiled) {
    debug_println("ERROR: Program not compiled. Upload source code first.");
    return -1;
  }

  if (!st_logic_set_enabled(logic_state, program_id, enabled)) {
    debug_println("ERROR: Failed to set enabled state");
    return -1;
  }

  debug_printf("✓ Logic%d %s\n", program_id + 1, enabled ? "ENABLED" : "DISABLED");
  return 0;
}

/**
 * @brief set logic <id> delete
 *
 * Delete a logic program
 *
 * Example:
 *   set logic 1 delete
 */
int cli_cmd_set_logic_delete(st_logic_engine_state_t *logic_state, uint8_t program_id) {
  if (program_id >= 4) {
    debug_println("ERROR: Invalid program ID (0-3)");
    return -1;
  }

  if (!st_logic_delete(logic_state, program_id)) {
    debug_println("ERROR: Failed to delete program");
    return -1;
  }

  debug_printf("✓ Logic%d deleted\n", program_id + 1);
  return 0;
}

/**
 * @brief set logic <id> bind <var_idx> <register> [input|output|both]
 *
 * Bind ST variable to Modbus register
 *
 * Example:
 *   set logic 1 bind 0 100 input   # ST var[0] reads from HR#100
 *   set logic 1 bind 1 101 output  # ST var[1] writes to HR#101
 */
int cli_cmd_set_logic_bind(st_logic_engine_state_t *logic_state, uint8_t program_id,
                           uint8_t var_index, uint16_t modbus_reg, const char *direction) {
  if (program_id >= 4) {
    debug_println("ERROR: Invalid program ID (0-3)");
    return -1;
  }

  if (var_index >= 32) {
    debug_println("ERROR: Invalid variable index (0-31)");
    return -1;
  }

  if (modbus_reg >= 160) {
    debug_println("ERROR: Invalid Modbus register (0-159)");
    return -1;
  }

  uint8_t is_input = 0, is_output = 0;

  if (strcmp(direction, "input") == 0) {
    is_input = 1;
  } else if (strcmp(direction, "output") == 0) {
    is_output = 1;
  } else if (strcmp(direction, "both") == 0 || strcmp(direction, "inout") == 0) {
    is_input = 1;
    is_output = 1;
  } else {
    debug_println("ERROR: Direction must be 'input', 'output', or 'both'");
    return -1;
  }

  if (!st_logic_bind_variable(logic_state, program_id, var_index, modbus_reg, is_input, is_output)) {
    debug_println("ERROR: Failed to bind variable");
    return -1;
  }

  debug_printf("✓ Logic%d: var[%d] %s Modbus HR#%d\n",
               program_id + 1, var_index,
               (is_input && is_output) ? "↔" : (is_input) ? "←" : "→",
               modbus_reg);

  return 0;
}

/* ============================================================================
 * SHOW COMMANDS
 * ============================================================================ */

/**
 * @brief show logic <id>
 *
 * Show detailed info about a logic program
 */
int cli_cmd_show_logic_program(st_logic_engine_state_t *logic_state, uint8_t program_id) {
  if (program_id >= 4) {
    debug_println("ERROR: Invalid program ID (0-3)");
    return -1;
  }

  st_logic_print_program(logic_state, program_id);
  return 0;
}

/**
 * @brief show logic all
 *
 * Show status of all logic programs
 */
int cli_cmd_show_logic_all(st_logic_engine_state_t *logic_state) {
  st_logic_print_status(logic_state);
  return 0;
}

/**
 * @brief show logic stats
 *
 * Show execution statistics for logic mode
 */
int cli_cmd_show_logic_stats(st_logic_engine_state_t *logic_state) {
  printf("\n=== Logic Engine Statistics ===\n");

  uint32_t total_executions = 0;
  uint32_t total_errors = 0;
  uint32_t enabled_count = 0;
  uint32_t compiled_count = 0;

  for (int i = 0; i < 4; i++) {
    st_logic_program_config_t *prog = &logic_state->programs[i];
    total_executions += prog->execution_count;
    total_errors += prog->error_count;
    if (prog->enabled) enabled_count++;
    if (prog->compiled) compiled_count++;
  }

  printf("Programs Compiled: %d/4\n", compiled_count);
  printf("Programs Enabled: %d/4\n", enabled_count);
  printf("Total Executions: %u\n", total_executions);
  printf("Total Errors: %u\n", total_errors);

  if (total_executions > 0) {
    float error_rate = (float)total_errors / total_executions * 100.0f;
    printf("Error Rate: %.2f%%\n", error_rate);
  }

  printf("\n");
  return 0;
}
