/**
 * @file st_builtin_persist.h
 * @brief ST Logic persistence built-in functions (v4.0+)
 *
 * Implements SAVE() and LOAD() functions for ST Logic programs.
 *
 * Usage from ST code:
 *   SAVE();  (* Save all persistent register groups to NVS *)
 *   LOAD();  (* Restore all groups from NVS *)
 *
 * Note: In v4.0, SAVE() and LOAD() save/restore ALL groups.
 * Future versions may support SAVE("group_name") syntax.
 */

#ifndef ST_BUILTIN_PERSIST_H
#define ST_BUILTIN_PERSIST_H

#include "st_types.h"

/**
 * @brief SAVE() - Save all persistent register groups to NVS
 *
 * @return 0 on success, -1 on failure, -2 if rate limited
 *
 * Rate limiting: Max 1 save per 5 seconds to protect flash wear.
 *
 * Steps:
 * 1. Check rate limit (max 1 save / 5 seconds)
 * 2. Snapshot all group register values (registers_persist_save_all_groups)
 * 3. Write entire config to NVS (config_save_to_nvs)
 * 4. Return status
 */
st_value_t st_builtin_persist_save(void);

/**
 * @brief LOAD() - Restore all persistent register groups from NVS
 *
 * @return 0 on success, -1 on failure
 *
 * Steps:
 * 1. Read config from NVS (config_load_from_nvs)
 * 2. Restore all group register values (registers_persist_restore_all_groups)
 * 3. Return status
 */
st_value_t st_builtin_persist_load(void);

#endif // ST_BUILTIN_PERSIST_H
