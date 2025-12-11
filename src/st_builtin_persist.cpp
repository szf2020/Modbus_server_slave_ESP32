/**
 * @file st_builtin_persist.cpp
 * @brief ST Logic persistence built-in functions implementation
 *
 * Implements SAVE() and LOAD() for register persistence from ST programs.
 */

#include "st_builtin_persist.h"
#include "registers_persist.h"
#include "config_save.h"
#include "config_load.h"
#include "config_struct.h"
#include "debug.h"
#include <Arduino.h>

// External reference to global config
extern PersistConfig g_persist_config;

/* ============================================================================
 * SAVE() IMPLEMENTATION
 * ============================================================================ */

st_value_t st_builtin_persist_save(void) {
  static uint32_t last_save_ms = 0;
  st_value_t result;

  // Rate limiting: Max 1 save per 5 seconds (protect flash wear)
  uint32_t now = millis();
  if (now - last_save_ms < 5000) {
    debug_println("SAVE() rate limited (wait 5s between saves)");
    result.int_val = -2;  // Error: Rate limited
    return result;
  }

  // Check if persistence system is enabled
  if (!registers_persist_is_enabled()) {
    debug_println("SAVE() failed: Persistence system disabled");
    result.int_val = -1;
    return result;
  }

  // Step 1: Snapshot all group register values
  debug_println("SAVE(): Snapshotting register groups...");
  bool success = registers_persist_save_all_groups();
  if (!success) {
    debug_println("SAVE() failed: Could not snapshot groups");
    result.int_val = -1;
    return result;
  }

  // Step 2: Save entire config to NVS (includes persist_regs)
  debug_println("SAVE(): Writing to NVS...");
  success = config_save_to_nvs(&g_persist_config);
  if (!success) {
    debug_println("SAVE() failed: NVS write error");
    result.int_val = -1;
    return result;
  }

  // Success!
  last_save_ms = now;
  debug_print("✓ SAVE() completed: ");
  debug_print_uint(g_persist_config.persist_regs.group_count);
  debug_println(" groups saved to NVS");

  result.int_val = 0;  // Success
  return result;
}

/* ============================================================================
 * LOAD() IMPLEMENTATION
 * ============================================================================ */

st_value_t st_builtin_persist_load(void) {
  st_value_t result;

  // Step 1: Load config from NVS
  debug_println("LOAD(): Reading from NVS...");

  PersistConfig temp_config;
  bool success = config_load_from_nvs(&temp_config);
  if (!success) {
    debug_println("LOAD() failed: NVS read error");
    result.int_val = -1;
    return result;
  }

  // Step 2: Copy loaded persist_regs to global config
  // Note: We only restore persist_regs, not the entire config
  // (to avoid overwriting runtime changes to counters, timers, etc.)
  g_persist_config.persist_regs = temp_config.persist_regs;

  // Step 3: Restore all group register values
  debug_println("LOAD(): Restoring register values...");
  success = registers_persist_restore_all_groups();
  if (!success) {
    debug_println("LOAD() failed: Could not restore groups");
    result.int_val = -1;
    return result;
  }

  // Success!
  debug_print("✓ LOAD() completed: ");
  debug_print_uint(g_persist_config.persist_regs.group_count);
  debug_println(" groups restored from NVS");

  result.int_val = 0;  // Success
  return result;
}
