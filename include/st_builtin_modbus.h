/**
 * @file st_builtin_modbus.h
 * @brief ST Logic Modbus Master builtin functions
 *
 * Wrapper functions for calling Modbus Master from ST Logic programs.
 */

#ifndef ST_BUILTIN_MODBUS_H
#define ST_BUILTIN_MODBUS_H

#include "st_types.h"

/* ============================================================================
 * ST LOGIC MODBUS FUNCTIONS
 * ============================================================================ */

/**
 * @brief MB_READ_COIL(slave_id, address) → BOOL
 *
 * Reads a single coil from remote slave device.
 * Returns FALSE on error (check mb_last_error).
 */
st_value_t st_builtin_mb_read_coil(st_value_t slave_id, st_value_t address);

/**
 * @brief MB_READ_INPUT(slave_id, address) → BOOL
 *
 * Reads a single discrete input from remote slave device.
 * Returns FALSE on error (check mb_last_error).
 */
st_value_t st_builtin_mb_read_input(st_value_t slave_id, st_value_t address);

/**
 * @brief MB_READ_HOLDING(slave_id, address) → INT
 *
 * Reads a single holding register from remote slave device.
 * Returns 0 on error (check mb_last_error).
 */
st_value_t st_builtin_mb_read_holding(st_value_t slave_id, st_value_t address);

/**
 * @brief MB_READ_INPUT_REG(slave_id, address) → INT
 *
 * Reads a single input register from remote slave device.
 * Returns 0 on error (check mb_last_error).
 */
st_value_t st_builtin_mb_read_input_reg(st_value_t slave_id, st_value_t address);

/**
 * @brief MB_WRITE_COIL(slave_id, address, value) → BOOL
 *
 * Writes a single coil to remote slave device.
 * Returns TRUE on success, FALSE on error.
 */
st_value_t st_builtin_mb_write_coil(st_value_t slave_id, st_value_t address, st_value_t value);

/**
 * @brief MB_WRITE_HOLDING(slave_id, address, value) → BOOL
 *
 * Writes a single holding register to remote slave device.
 * Returns TRUE on success, FALSE on error.
 */
st_value_t st_builtin_mb_write_holding(st_value_t slave_id, st_value_t address, st_value_t value);

/* ============================================================================
 * GLOBAL STATUS VARIABLES (accessible from ST Logic)
 * ============================================================================ */

// These are updated after each Modbus call
extern int32_t g_mb_last_error;   // mb_error_code_t (0=OK, 1=Timeout, etc.)
extern bool g_mb_success;         // TRUE if last operation succeeded
extern uint8_t g_mb_request_count; // Current request count in this execution

#endif // ST_BUILTIN_MODBUS_H
