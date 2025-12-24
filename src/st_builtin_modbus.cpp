/**
 * @file st_builtin_modbus.cpp
 * @brief ST Logic Modbus Master Wrapper Functions
 *
 * Provides Modbus Master functions callable from ST Logic programs.
 */

#include "st_builtin_modbus.h"
#include "modbus_master.h"

/* ============================================================================
 * GLOBAL STATUS VARIABLES
 * ============================================================================ */

int32_t g_mb_last_error = MB_OK;
bool g_mb_success = false;
uint8_t g_mb_request_count = 0;

/* ============================================================================
 * HELPER FUNCTION
 * ============================================================================ */

static bool check_request_limit() {
  if (g_mb_request_count >= g_modbus_master_config.max_requests_per_cycle) {
    g_mb_last_error = MB_MAX_REQUESTS_EXCEEDED;
    g_mb_success = false;
    return false;
  }
  g_mb_request_count++;
  return true;
}

/* ============================================================================
 * ST LOGIC BUILTIN FUNCTIONS
 * ============================================================================ */

st_value_t st_builtin_mb_read_coil(st_value_t slave_id, st_value_t address) {
  st_value_t result;
  result.bool_val = false;

  if (!check_request_limit()) {
    return result;
  }

  bool coil_value = false;
  mb_error_code_t err = modbus_master_read_coil(
    (uint8_t)slave_id.int_val,
    (uint16_t)address.int_val,
    &coil_value
  );

  g_mb_last_error = err;
  g_mb_success = (err == MB_OK);
  result.bool_val = coil_value;

  // Apply inter-frame delay
  if (g_modbus_master_config.inter_frame_delay > 0) {
    delay(g_modbus_master_config.inter_frame_delay);
  }

  return result;
}

st_value_t st_builtin_mb_read_input(st_value_t slave_id, st_value_t address) {
  st_value_t result;
  result.bool_val = false;

  if (!check_request_limit()) {
    return result;
  }

  bool input_value = false;
  mb_error_code_t err = modbus_master_read_input(
    (uint8_t)slave_id.int_val,
    (uint16_t)address.int_val,
    &input_value
  );

  g_mb_last_error = err;
  g_mb_success = (err == MB_OK);
  result.bool_val = input_value;

  if (g_modbus_master_config.inter_frame_delay > 0) {
    delay(g_modbus_master_config.inter_frame_delay);
  }

  return result;
}

st_value_t st_builtin_mb_read_holding(st_value_t slave_id, st_value_t address) {
  st_value_t result;
  result.int_val = 0;

  if (!check_request_limit()) {
    return result;
  }

  uint16_t register_value = 0;
  mb_error_code_t err = modbus_master_read_holding(
    (uint8_t)slave_id.int_val,
    (uint16_t)address.int_val,
    &register_value
  );

  g_mb_last_error = err;
  g_mb_success = (err == MB_OK);
  result.int_val = (int32_t)register_value;

  if (g_modbus_master_config.inter_frame_delay > 0) {
    delay(g_modbus_master_config.inter_frame_delay);
  }

  return result;
}

st_value_t st_builtin_mb_read_input_reg(st_value_t slave_id, st_value_t address) {
  st_value_t result;
  result.int_val = 0;

  if (!check_request_limit()) {
    return result;
  }

  uint16_t register_value = 0;
  mb_error_code_t err = modbus_master_read_input_register(
    (uint8_t)slave_id.int_val,
    (uint16_t)address.int_val,
    &register_value
  );

  g_mb_last_error = err;
  g_mb_success = (err == MB_OK);
  result.int_val = (int32_t)register_value;

  if (g_modbus_master_config.inter_frame_delay > 0) {
    delay(g_modbus_master_config.inter_frame_delay);
  }

  return result;
}

st_value_t st_builtin_mb_write_coil(st_value_t slave_id, st_value_t address, st_value_t value) {
  st_value_t result;
  result.bool_val = false;

  if (!check_request_limit()) {
    return result;
  }

  mb_error_code_t err = modbus_master_write_coil(
    (uint8_t)slave_id.int_val,
    (uint16_t)address.int_val,
    value.bool_val
  );

  g_mb_last_error = err;
  g_mb_success = (err == MB_OK);
  result.bool_val = (err == MB_OK);

  if (g_modbus_master_config.inter_frame_delay > 0) {
    delay(g_modbus_master_config.inter_frame_delay);
  }

  return result;
}

st_value_t st_builtin_mb_write_holding(st_value_t slave_id, st_value_t address, st_value_t value) {
  st_value_t result;
  result.bool_val = false;

  if (!check_request_limit()) {
    return result;
  }

  mb_error_code_t err = modbus_master_write_holding(
    (uint8_t)slave_id.int_val,
    (uint16_t)address.int_val,
    (uint16_t)value.int_val
  );

  g_mb_last_error = err;
  g_mb_success = (err == MB_OK);
  result.bool_val = (err == MB_OK);

  if (g_modbus_master_config.inter_frame_delay > 0) {
    delay(g_modbus_master_config.inter_frame_delay);
  }

  return result;
}
