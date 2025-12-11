/**
 * @file gpio_mapping.h
 * @brief GPIO STATIC mapping - sync GPIO pins with Modbus registers/coils
 *
 * LAYER 4: Register/Coil Storage
 * Responsibility: Synchronize GPIO pin states with Modbus data
 *
 * Two modes:
 * - INPUT mode: GPIO pin → discrete input (Modbus master reads GPIO state)
 * - OUTPUT mode: Coil → GPIO pin (Modbus master controls GPIO output)
 */

#ifndef gpio_mapping_H
#define gpio_mapping_H

#include <stdint.h>
#include "types.h"

/**
 * @brief Update all GPIO STATIC mappings
 *
 * This function should be called once per main loop iteration.
 * It synchronizes GPIO pins with Modbus registers/coils:
 * - INPUT mode: Read GPIO pin → write to discrete input
 * - OUTPUT mode: Read coil → write to GPIO pin
 *
 * DEPRECATED: Use gpio_mapping_read_before_st_logic() and gpio_mapping_write_after_st_logic()
 * separately to avoid INPUT overwriting OUTPUT in the same loop iteration.
 */
void gpio_mapping_update(void);

/**
 * @brief Read all INPUT mappings BEFORE ST logic execution
 *
 * Call this BEFORE st_logic_engine_loop() to provide fresh inputs.
 * Only processes INPUT mappings (Modbus → ST variables, GPIO → discrete inputs).
 */
void gpio_mapping_read_before_st_logic(void);

/**
 * @brief Write all OUTPUT mappings AFTER ST logic execution
 *
 * Call this AFTER st_logic_engine_loop() to push results to Modbus.
 * Only processes OUTPUT mappings (ST variables → Modbus, coils → GPIO).
 */
void gpio_mapping_write_after_st_logic(void);

#endif // gpio_mapping_H
