/**
 * @file pcnt_driver.cpp
 * @brief PCNT driver implementation using ESP32 SDK
 *
 * LAYER 0: Hardware Abstraction Driver
 * Provides pulse counter functionality via ESP32 PCNT peripheral
 */

#include "pcnt_driver.h"
#include <string.h>
#include <driver/pcnt.h>
#include <esp_log.h>

static const char* TAG = "PCNT";

// PCNT configuration state
static uint8_t pcnt_configured[4] = {0};

/* ============================================================================
 * PCNT UNIT INITIALIZATION
 * ============================================================================ */

void pcnt_unit_init(uint8_t unit) {
  if (unit >= 4) return;

  // BUG FIX P0.0: CRITICAL - Don't pause/clear PCNT until AFTER pcnt_unit_config()
  // Calling pause/clear on unconfigured PCNT unit causes "PCNT driver error"
  // Configuration happens later in pcnt_unit_configure(), which will pause/clear/resume

  pcnt_configured[unit] = 0;
  // Do NOT call pcnt_counter_pause/clear here - PCNT not configured yet!
}

/* ============================================================================
 * PCNT UNIT CONFIGURATION
 * ============================================================================ */

void pcnt_unit_configure(uint8_t unit, uint8_t gpio_pin,
                        pcnt_edge_mode_t pos_edge, pcnt_edge_mode_t neg_edge) {
  if (unit >= 4) return;

  // Convert edge modes to PCNT count modes
  // ESP32 PCNT hardware can selectively count rising, falling, or both edges
  // PCNT_COUNT_DIS = ignore this edge (don't count)
  // PCNT_COUNT_INC = increment counter on this edge
  // pos_mode applies to LOW→HIGH transitions (rising edges)
  // neg_mode applies to HIGH→LOW transitions (falling edges)
  pcnt_count_mode_t pos_mode = PCNT_COUNT_DIS;
  pcnt_count_mode_t neg_mode = PCNT_COUNT_DIS;

  // Configure rising edge counting
  if (pos_edge == PCNT_EDGE_RISING) {
    pos_mode = PCNT_COUNT_INC;  // Count on rising edge (LOW→HIGH)
  }

  // Configure falling edge counting
  if (neg_edge == PCNT_EDGE_FALLING) {
    neg_mode = PCNT_COUNT_INC;  // Count on falling edge (HIGH→LOW)
  }

  // DEBUG: Log PCNT configuration (EXPANDED)
  ESP_LOGI(TAG, "Unit%d GPIO%d: pos_edge=%d neg_edge=%d → pos_mode=%d neg_mode=%d",
           unit, gpio_pin, pos_edge, neg_edge, pos_mode, neg_mode);
  ESP_LOGI(TAG, "  PCNT_EDGE_DISABLE=%d, PCNT_EDGE_RISING=%d, PCNT_EDGE_FALLING=%d",
           PCNT_EDGE_DISABLE, PCNT_EDGE_RISING, PCNT_EDGE_FALLING);
  ESP_LOGI(TAG, "  PCNT_COUNT_DIS=%d, PCNT_COUNT_INC=%d",
           PCNT_COUNT_DIS, PCNT_COUNT_INC);

  // Configure PCNT unit
  pcnt_config_t pcnt_config = {
    .pulse_gpio_num = gpio_pin,
    .ctrl_gpio_num = PCNT_PIN_NOT_USED,
    .lctrl_mode = PCNT_MODE_KEEP,
    .hctrl_mode = PCNT_MODE_KEEP,
    .pos_mode = pos_mode,
    .neg_mode = neg_mode,
    .counter_h_lim = 32767,    // Max count (will wrap, we handle in software)
    .counter_l_lim = -32768,   // Min count
    .unit = (pcnt_unit_t)unit,
    .channel = PCNT_CHANNEL_0,
  };

  pcnt_unit_config(&pcnt_config);

  // BUG FIX P0.1: DISABLE filter for high-frequency counting
  // Filter/debounce only for low-frequency (manual button press, noisy environments)
  // At 5 kHz, filter can cause missed pulses - disable completely
  pcnt_filter_disable((pcnt_unit_t)unit);

  // Clear and start counter
  pcnt_counter_pause((pcnt_unit_t)unit);
  pcnt_counter_clear((pcnt_unit_t)unit);
  pcnt_counter_resume((pcnt_unit_t)unit);

  pcnt_configured[unit] = 1;
}

/* ============================================================================
 * PCNT COUNTER OPERATIONS
 * ============================================================================ */

uint32_t pcnt_unit_get_count(uint8_t unit) {
  if (unit >= 4) return 0;

  // Read hardware PCNT counter value
  int16_t count = 0;
  pcnt_get_counter_value((pcnt_unit_t)unit, &count);

  // Return as unsigned (we handle wrap in counter_hw.cpp)
  return (uint32_t)count;
}

void pcnt_unit_clear(uint8_t unit) {
  if (unit >= 4) return;

  // Reset hardware counter
  pcnt_counter_pause((pcnt_unit_t)unit);
  pcnt_counter_clear((pcnt_unit_t)unit);
  pcnt_counter_resume((pcnt_unit_t)unit);
}

void pcnt_unit_set_count(uint8_t unit, uint32_t value) {
  if (unit >= 4) return;

  // ESP32 PCNT can only clear, not set arbitrary values
  // This is a limitation of the hardware
  // We handle absolute values in counter_hw.cpp software layer
  (void)value;  // Unused
}
