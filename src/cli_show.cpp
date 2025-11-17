/**
 * @file cli_show.cpp
 * @brief CLI `show` command handlers (LAYER 7)
 *
 * Ported from: Mega2560 v3.6.5 cli_shell.cpp (show command handlers)
 * Adapted to: ESP32 modular architecture
 *
 * Responsibility:
 * - Retrieve system state (config, status, register values)
 * - Format for terminal display (tables, lists, etc.)
 * - Print via debug_println()
 *
 * Context: Formatting optimized for 80-char serial terminals
 */

#include "cli_show.h"
#include "counter_engine.h"
#include "counter_config.h"
#include "registers.h"
#include "version.h"
#include "debug.h"
#include <stdio.h>
#include <string.h>

/* ============================================================================
 * SHOW CONFIG
 * ============================================================================ */

void cli_cmd_show_config(void) {
  debug_println("\n=== CONFIGURATION ===");
  debug_println("Version: 1.0.0");
  debug_println("Build: 20251117");
  debug_println("Unit-ID: 1 (SLAVE)");
  debug_println("Baud: 9600");
  debug_println("Server: RUNNING");
  debug_println("Mode: SERVER");
  debug_println("Hostname: esp32-modbus");
  debug_println("=====================\n");

  // Counter configuration block (Mega2560 format)
  bool any_counter = false;
  debug_println("counters");
  for (uint8_t id = 1; id <= 4; id++) {
    CounterConfig cfg;
    if (counter_config_get(id, &cfg)) {
      any_counter = true;
      debug_print("  counter ");
      debug_print_uint(id);
      debug_print(cfg.enabled ? " ENABLED" : " DISABLED");

      // edge mode
      const char* edge_str = "rising";
      if (cfg.edge_type == COUNTER_EDGE_FALLING) edge_str = "falling";
      else if (cfg.edge_type == COUNTER_EDGE_BOTH) edge_str = "both";
      debug_print(" edge=");
      debug_print(edge_str);

      // prescaler and resolution
      debug_print(" prescaler=");
      debug_print_uint(cfg.prescaler);
      debug_print(" res=");
      debug_print_uint(cfg.bit_width);

      // direction
      const char* dir_str = (cfg.direction == COUNTER_DIR_DOWN) ? "down" : "up";
      debug_print(" dir=");
      debug_print(dir_str);

      // scale (format as float)
      debug_print(" scale=");
      debug_print_float(cfg.scale_factor);

      // Register indices
      debug_print(" input-dis=");
      debug_print_uint(cfg.input_dis);
      debug_print(" index-reg=");
      debug_print_uint(cfg.index_reg);

      if (cfg.raw_reg > 0) {
        debug_print(" raw-reg=");
        debug_print_uint(cfg.raw_reg);
      }
      if (cfg.freq_reg > 0) {
        debug_print(" freq-reg=");
        debug_print_uint(cfg.freq_reg);
      }

      debug_print(" overload-reg=");
      if (cfg.overload_reg < 1000) debug_print_uint(cfg.overload_reg);
      else debug_print("n/a");

      debug_print(" ctrl-reg=");
      if (cfg.ctrl_reg < 1000) debug_print_uint(cfg.ctrl_reg);
      else debug_print("n/a");

      // start value
      debug_print(" start=");
      debug_print_uint((uint32_t)cfg.start_value);

      // debounce
      debug_print(" debounce=");
      if (cfg.debounce_enabled && cfg.debounce_ms > 0) {
        debug_print("on/");
        debug_print_uint(cfg.debounce_ms);
        debug_print("ms");
      } else {
        debug_print("off");
      }

      // hw-mode
      debug_print(" hw-mode=");
      if (cfg.hw_mode == COUNTER_HW_SW) {
        if (cfg.interrupt_pin > 0) {
          debug_print("sw-isr");
        } else {
          debug_print("sw");
        }
      } else if (cfg.hw_mode == COUNTER_HW_SW_ISR) {
        debug_print("sw-isr");
      } else if (cfg.hw_mode == COUNTER_HW_PCNT) {
        debug_print("hw");
      } else {
        debug_print("unknown");
      }

      // interrupt pin for ISR mode
      if ((cfg.hw_mode == COUNTER_HW_SW || cfg.hw_mode == COUNTER_HW_SW_ISR) && cfg.interrupt_pin > 0) {
        debug_print(" interrupt-pin=");
        debug_print_uint(cfg.interrupt_pin);
      }

      debug_println("");
    }
  }
  if (!any_counter) {
    debug_println("counters (none enabled)");
  }

  // Counter control block (if any enabled)
  bool show_control = false;
  for (uint8_t id = 1; id <= 4; id++) {
    CounterConfig cfg;
    if (counter_config_get(id, &cfg) && cfg.enabled) {
      show_control = true;
      break;
    }
  }

  if (show_control) {
    debug_println("counters control");
    for (uint8_t id = 1; id <= 4; id++) {
      CounterConfig cfg;
      if (counter_config_get(id, &cfg) && cfg.enabled) {
        debug_print("  counter");
        debug_print_uint(id);
        debug_print(" reset-on-read DISABLE auto-start DISABLE");
        debug_println("");
      }
    }
  }

  debug_println("\n(Full timer configuration not yet displayed)\n");
}

/* ============================================================================
 * SHOW COUNTERS
 * ============================================================================ */

void cli_cmd_show_counters(void) {
  debug_println("\n=== COUNTER STATUS ===\n");
  debug_println("ID   Mode     Enabled  Value        Hz");
  debug_println("--   ----     -------  -----------  ------");

  for (uint8_t id = 1; id <= 4; id++) {
    CounterConfig cfg;
    if (!counter_config_get(id, &cfg)) continue;

    char mode_str[10];
    if (cfg.hw_mode == COUNTER_HW_SW) strcpy(mode_str, "SW");
    else if (cfg.hw_mode == COUNTER_HW_SW_ISR) strcpy(mode_str, "ISR");
    else if (cfg.hw_mode == COUNTER_HW_PCNT) strcpy(mode_str, "HW");
    else strcpy(mode_str, "???");

    uint64_t value = counter_engine_get_value(id);

    debug_print_uint(id);
    debug_print("    ");
    debug_print(mode_str);
    debug_print("       ");
    if (cfg.enabled) {
      debug_print("Yes");
    } else {
      debug_print("No");
    }
    debug_print("      ");
    debug_print_uint((uint32_t)value);  // Simplified: truncate to 32-bit
    debug_println("");
  }

  debug_println("");
}

/* ============================================================================
 * SHOW TIMERS
 * ============================================================================ */

void cli_cmd_show_timers(void) {
  debug_println("\n=== TIMER STATUS ===\n");

  bool any = false;
  for (uint8_t id = 1; id <= 4; id++) {
    // TODO: Implement when timer engine is ported
    // For now, show basic placeholder
  }

  if (!any) {
    debug_println("(Timer functionality not yet fully ported)");
  }
  debug_println("");
}

/* ============================================================================
 * SHOW REGISTERS
 * ============================================================================ */

void cli_cmd_show_registers(uint16_t start, uint16_t count) {
  debug_println("\n=== HOLDING REGISTERS (0..159) ===\n");

  // Show registers in groups of 8 (Mega2560 format)
  for (uint16_t base = 0; base < HOLDING_REGS_SIZE && base < 160; base += 8) {
    debug_print_uint(base);
    debug_print(": ");

    for (uint8_t i = 0; i < 8 && (base + i) < HOLDING_REGS_SIZE; i++) {
      uint16_t value = registers_get_holding_register(base + i);
      debug_print_uint(value);
      debug_print("\t");
    }
    debug_println("");
  }

  debug_println("");
}

/* ============================================================================
 * SHOW COILS
 * ============================================================================ */

void cli_cmd_show_coils(void) {
  debug_println("\n=== COILS (0..63) ===\n");

  // Show coils in groups of 16 (Mega2560 format)
  for (uint16_t base = 0; base < (COILS_SIZE * 8) && base < 64; base += 16) {
    debug_print_uint(base);
    debug_print(": ");

    for (uint8_t i = 0; i < 16 && (base + i) < (COILS_SIZE * 8); i++) {
      bool state = registers_get_coil(base + i);
      debug_print(state ? "1" : "0");
      debug_print(" ");
    }
    debug_println("");
  }

  debug_println("");
}

/* ============================================================================
 * SHOW INPUTS
 * ============================================================================ */

void cli_cmd_show_inputs(void) {
  debug_println("\n=== DISCRETE INPUTS (0..255) ===\n");

  // Show discrete inputs in groups of 16 (Mega2560 format)
  for (uint16_t base = 0; base < (DISCRETE_INPUTS_SIZE * 8) && base < 256; base += 16) {
    debug_print_uint(base);
    debug_print(": ");

    for (uint8_t i = 0; i < 16 && (base + i) < (DISCRETE_INPUTS_SIZE * 8); i++) {
      bool state = registers_get_discrete_input(base + i);
      debug_print(state ? "1" : "0");
      debug_print(" ");
    }
    debug_println("");
  }

  debug_println("");
}

/* ============================================================================
 * SHOW VERSION
 * ============================================================================ */

void cli_cmd_show_version(void) {
  debug_println("\n=== FIRMWARE VERSION ===\n");
  debug_print("Version: ");
  debug_println(PROJECT_VERSION);
  debug_println("Target:  ESP32-WROOM-32");
  debug_println("Project: Modbus RTU Server");
  debug_println("");
}

/* ============================================================================
 * SHOW GPIO
 * ============================================================================ */

void cli_cmd_show_gpio(void) {
  debug_println("\n=== GPIO MAPPING ===\n");
  debug_println("UART1 (Modbus):");
  debug_println("  GPIO4  - RX");
  debug_println("  GPIO5  - TX");
  debug_println("  GPIO15 - RS485 DIR");
  debug_println("");
  debug_println("PCNT Counters:");
  debug_println("  GPIO19 - Counter 1 (PCNT Unit 0)");
  debug_println("  GPIO25 - Counter 2 (PCNT Unit 1)");
  debug_println("  GPIO27 - Counter 3 (PCNT Unit 2)");
  debug_println("  GPIO33 - Counter 4 (PCNT Unit 3)");
  debug_println("");
}
