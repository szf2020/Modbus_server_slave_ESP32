/**
 * @file main.cpp
 * @brief Main entry point for ESP32 Modbus RTU Server
 *
 * Arduino setup() and loop() only. All subsystems are called from here.
 * No business logic in this file.
 */

#include <Arduino.h>
#include "constants.h"
#include "types.h"
#include "version.h"
#include "heartbeat.h"
#include "modbus_server.h"
#include "counter_engine.h"
#include "timer_engine.h"
#include "cli_shell.h"
#include "debug.h"

// ============================================================================
// SETUP
// ============================================================================

void setup() {
  // Initialize serial ports
  Serial.begin(SERIAL_BAUD_DEBUG);      // USB debug (UART0)
  delay(1000);  // Wait for serial monitor

  debug_printf("=== %s v%s ===\n", PROJECT_NAME, PROJECT_VERSION);
  debug_printf("Build: %s %s\n", BUILD_DATE, BUILD_TIME);

  // Initialize hardware drivers
  // TODO: gpio_driver_init()
  // TODO: uart_driver_init()
  // TODO: nvs_driver_init()

  // Initialize subsystems
  // TODO: config_load_from_nvs()
  // TODO: modbus_server_init()
  // TODO: counter_engine_init()
  // TODO: timer_engine_init()
  // TODO: cli_shell_init()
  // TODO: heartbeat_init()

  debug_printf("Setup complete.\n");
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
  // CLI takes priority (blocks other subsystems when active)
  if (/* CLI is active */) {
    cli_shell_loop();
  } else {
    // Normal operation: Modbus + features
    // TODO: modbus_server_loop();
    // TODO: counter_engine_loop();
    // TODO: timer_engine_loop();
    // TODO: heartbeat_loop();
  }

  // Small delay to prevent watchdog timeout
  delay(1);
}
