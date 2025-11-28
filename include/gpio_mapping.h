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
 */
void gpio_mapping_update(void);

#endif // gpio_mapping_H
