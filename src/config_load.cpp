/**
 * @file config_load.cpp
 * @brief Configuration load from NVS with CRC validation
 *
 * LAYER 6: Persistence
 * Responsibility: Load configuration from NVS and validate integrity
 */

#include "config_load.h"
#include "config_save.h"
#include "debug.h"
#include <nvs_flash.h>
#include <nvs.h>
#include <cstddef>
#include <cstring>

// NVS key for storing config
#define NVS_CONFIG_KEY "modbus_cfg"
#define NVS_NAMESPACE  "modbus"

/**
 * @brief Initialize configuration with factory defaults
 */
static void config_init_defaults(PersistConfig* cfg) {
  memset(cfg, 0, sizeof(PersistConfig));
  cfg->schema_version = 1;
  cfg->slave_id = 1;
  cfg->baudrate = 9600;

  // Initialize all GPIO mappings as unused
  for (uint8_t i = 0; i < 8; i++) {
    cfg->var_maps[i].input_reg = 65535;
    cfg->var_maps[i].coil_reg = 65535;
    cfg->var_maps[i].associated_counter = 0xff;
    cfg->var_maps[i].associated_timer = 0xff;
  }
}

bool config_load_from_nvs(PersistConfig* out) {
  if (out == NULL) {
    debug_println("ERROR: config_load_from_nvs - NULL output");
    return false;
  }

  // Open NVS
  nvs_handle_t handle;
  esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
  if (err == ESP_ERR_NVS_NOT_FOUND) {
    debug_println("CONFIG LOAD: NVS namespace not found, using defaults");
    config_init_defaults(out);
    return true;
  } else if (err != ESP_OK) {
    debug_print("ERROR: NVS open failed: ");
    debug_print_uint(err);
    debug_println(", using defaults");
    config_init_defaults(out);
    return true;
  }

  // Read config blob from NVS
  size_t required_size = sizeof(PersistConfig);
  err = nvs_get_blob(handle, NVS_CONFIG_KEY, out, &required_size);
  nvs_close(handle);

  if (err == ESP_ERR_NVS_NOT_FOUND) {
    debug_println("CONFIG LOAD: Config key not found, using defaults");
    config_init_defaults(out);
    return true;
  } else if (err != ESP_OK) {
    debug_print("ERROR: NVS get_blob failed: ");
    debug_print_uint(err);
    debug_println(", using defaults");
    config_init_defaults(out);
    return true;
  }

  // Validate CRC
  uint16_t stored_crc = out->crc16;
  uint16_t calculated_crc = config_calculate_crc16(out);

  if (stored_crc != calculated_crc) {
    debug_print("ERROR: CRC mismatch (stored=");
    debug_print_uint(stored_crc);
    debug_print(", calculated=");
    debug_print_uint(calculated_crc);
    debug_println("), using defaults");
    config_init_defaults(out);
    return true;
  }

  // Print summary
  debug_print("CONFIG LOADED: schema=");
  debug_print_uint(out->schema_version);
  debug_print(", slave_id=");
  debug_print_uint(out->slave_id);
  debug_print(", baudrate=");
  debug_print_uint(out->baudrate);
  debug_print(", var_maps=");
  debug_print_uint(out->var_map_count);
  debug_print(", static_regs=");
  debug_print_uint(out->static_reg_count);
  debug_print(", static_coils=");
  debug_print_uint(out->static_coil_count);
  debug_print(", CRC=");
  debug_print_uint(calculated_crc);
  debug_println(" OK");

  return true;
}
