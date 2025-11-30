/**
 * @file st_logic_config.h
 * @brief Structured Text Logic Mode Configuration
 *
 * Configuration for logic programs and Modbus register bindings.
 * Supports 4 independent logic programs with register I/O.
 */

#ifndef ST_LOGIC_CONFIG_H
#define ST_LOGIC_CONFIG_H

#include "st_types.h"

/* ============================================================================
 * VARIABLE BINDING (ST variable ↔ Modbus register)
 * ============================================================================ */

typedef struct {
  uint8_t st_var_index;       // Which ST variable (0-31)
  uint16_t modbus_register;   // Modbus holding register address (0-159)
  uint8_t is_input;           // VAR_INPUT? (read from register)
  uint8_t is_output;          // VAR_OUTPUT? (write to register)
} st_var_binding_t;

/* ============================================================================
 * LOGIC PROGRAM CONFIGURATION
 * ============================================================================ */

typedef struct {
  // Program identification
  char name[32];              // "Logic1", "Logic2", etc.
  uint8_t enabled;            // Is this program enabled?

  // Source code storage
  char source_code[5000];     // ST source code (max 5KB per program)
  uint32_t source_size;

  // Compiled bytecode
  st_bytecode_program_t bytecode; // Compiled and ready to execute
  uint8_t compiled;           // Is bytecode valid?

  // Register bindings (which Modbus registers map to which variables)
  st_var_binding_t var_bindings[32]; // Max 32 variable bindings
  uint8_t binding_count;

  // Execution statistics
  uint32_t execution_count;   // Number of times executed
  uint32_t error_count;       // Number of execution errors
  uint32_t last_execution_ms; // Last execution time (milliseconds)
  char last_error[128];       // Last error message

} st_logic_program_config_t;

/* ============================================================================
 * GLOBAL LOGIC ENGINE STATE
 * ============================================================================ */

typedef struct {
  // 4 independent logic programs
  st_logic_program_config_t programs[4];

  // Global settings
  uint8_t enabled;            // Logic mode enabled/disabled globally
  uint32_t execution_interval_ms; // How often to run programs (10ms default)
  uint32_t last_run_time;     // Timestamp of last execution

} st_logic_engine_state_t;

/* ============================================================================
 * FUNCTIONS
 * ============================================================================ */

/**
 * @brief Initialize logic engine state
 * @param state Logic engine state
 */
void st_logic_init(st_logic_engine_state_t *state);

/**
 * @brief Upload ST source code for a program
 * @param state Logic engine state
 * @param program_id Program ID (0-3)
 * @param source ST source code
 * @param source_size Size of source code
 * @return true if successful
 */
bool st_logic_upload(st_logic_engine_state_t *state, uint8_t program_id,
                      const char *source, uint32_t source_size);

/**
 * @brief Compile and prepare logic program for execution
 * @param state Logic engine state
 * @param program_id Program ID (0-3)
 * @return true if successful
 */
bool st_logic_compile(st_logic_engine_state_t *state, uint8_t program_id);

/**
 * @brief Set variable binding (ST variable ↔ Modbus register)
 * @param state Logic engine state
 * @param program_id Program ID (0-3)
 * @param st_var_index ST variable index (0-31)
 * @param modbus_reg Modbus holding register address
 * @param is_input Is this an input (read from register)?
 * @param is_output Is this an output (write to register)?
 * @return true if successful
 */
bool st_logic_bind_variable(st_logic_engine_state_t *state, uint8_t program_id,
                             uint8_t st_var_index, uint16_t modbus_reg,
                             uint8_t is_input, uint8_t is_output);

/**
 * @brief Enable/disable a logic program
 * @param state Logic engine state
 * @param program_id Program ID (0-3)
 * @param enabled true to enable, false to disable
 * @return true if successful
 */
bool st_logic_set_enabled(st_logic_engine_state_t *state, uint8_t program_id, uint8_t enabled);

/**
 * @brief Delete/clear a logic program
 * @param state Logic engine state
 * @param program_id Program ID (0-3)
 * @return true if successful
 */
bool st_logic_delete(st_logic_engine_state_t *state, uint8_t program_id);

/**
 * @brief Get program info
 * @param state Logic engine state
 * @param program_id Program ID (0-3)
 * @return Program configuration (NULL if invalid ID)
 */
st_logic_program_config_t *st_logic_get_program(st_logic_engine_state_t *state, uint8_t program_id);

#endif // ST_LOGIC_CONFIG_H
