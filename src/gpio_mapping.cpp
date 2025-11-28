/**
 * @file gpio_mapping.cpp
 * @brief GPIO STATIC mapping - sync GPIO pins with Modbus registers/coils
 *
 * LAYER 4: Register/Coil Storage
 * Responsibility: Synchronize GPIO pin states with Modbus data
 */

#include "gpio_mapping.h"
#include "config_struct.h"
#include "gpio_driver.h"
#include "registers.h"

/**
 * @brief Update all GPIO STATIC mappings
 *
 * This function is called once per main loop iteration.
 * It synchronizes GPIO pins with Modbus registers/coils:
 * - INPUT mode: Read GPIO pin → write to discrete input
 * - OUTPUT mode: Read coil → write to GPIO pin
 */
void gpio_mapping_update(void) {
  for (uint8_t i = 0; i < g_persist_config.gpio_map_count; i++) {
    const GPIOMapping* map = &g_persist_config.gpio_maps[i];

    // Skip mappings associated with counters/timers (handled by their engines)
    if (map->associated_counter != 0xff || map->associated_timer != 0xff) {
      continue;
    }

    if (map->is_input) {
      // INPUT mode: GPIO pin → discrete input
      if (map->input_reg != 65535) {
        uint8_t level = gpio_read(map->gpio_pin);
        registers_set_discrete_input(map->input_reg, level);
      }
    } else {
      // OUTPUT mode: Coil → GPIO pin
      if (map->coil_reg != 65535) {
        uint8_t value = registers_get_coil(map->coil_reg);
        gpio_write(map->gpio_pin, value);
      }
    }
  }
}

