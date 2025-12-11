/**
 * @file st_builtins.cpp
 * @brief Structured Text Built-in Functions Implementation
 *
 * IEC 61131-3 standard mathematical and conversion functions.
 */

#include "st_builtins.h"
#include "st_builtin_persist.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * MATHEMATICAL FUNCTIONS
 * ============================================================================ */

st_value_t st_builtin_abs(st_value_t x) {
  st_value_t result;
  result.int_val = (x.int_val < 0) ? -x.int_val : x.int_val;
  return result;
}

st_value_t st_builtin_min(st_value_t a, st_value_t b) {
  st_value_t result;
  result.int_val = (a.int_val < b.int_val) ? a.int_val : b.int_val;
  return result;
}

st_value_t st_builtin_max(st_value_t a, st_value_t b) {
  st_value_t result;
  result.int_val = (a.int_val > b.int_val) ? a.int_val : b.int_val;
  return result;
}

st_value_t st_builtin_sqrt(st_value_t x) {
  st_value_t result;
  result.real_val = (float)sqrtf((float)x.real_val);
  return result;
}

st_value_t st_builtin_round(st_value_t x) {
  st_value_t result;
  result.int_val = (int32_t)roundf(x.real_val);
  return result;
}

st_value_t st_builtin_trunc(st_value_t x) {
  st_value_t result;
  result.int_val = (int32_t)truncf(x.real_val);
  return result;
}

st_value_t st_builtin_floor(st_value_t x) {
  st_value_t result;
  result.int_val = (int32_t)floorf(x.real_val);
  return result;
}

st_value_t st_builtin_ceil(st_value_t x) {
  st_value_t result;
  result.int_val = (int32_t)ceilf(x.real_val);
  return result;
}

/* ============================================================================
 * TYPE CONVERSION FUNCTIONS
 * ============================================================================ */

st_value_t st_builtin_int_to_real(st_value_t i) {
  st_value_t result;
  result.real_val = (float)i.int_val;
  return result;
}

st_value_t st_builtin_real_to_int(st_value_t r) {
  st_value_t result;
  result.int_val = (int32_t)r.real_val;
  return result;
}

st_value_t st_builtin_bool_to_int(st_value_t b) {
  st_value_t result;
  result.int_val = (b.bool_val != 0) ? 1 : 0;
  return result;
}

st_value_t st_builtin_int_to_bool(st_value_t i) {
  st_value_t result;
  result.bool_val = (i.int_val != 0);
  return result;
}

st_value_t st_builtin_dword_to_int(st_value_t d) {
  st_value_t result;
  // Clamp to INT range if overflow
  if (d.dword_val > 2147483647) {
    result.int_val = 2147483647;
  } else {
    result.int_val = (int32_t)d.dword_val;
  }
  return result;
}

st_value_t st_builtin_int_to_dword(st_value_t i) {
  st_value_t result;
  // Unsigned conversion: negative → wrap around
  result.dword_val = (uint32_t)i.int_val;
  return result;
}

/* ============================================================================
 * DISPATCHER FUNCTION
 * ============================================================================ */

st_value_t st_builtin_call(st_builtin_func_t func_id, st_value_t arg1, st_value_t arg2) {
  st_value_t result = {0};

  switch (func_id) {
    // Mathematical
    case ST_BUILTIN_ABS:
      result = st_builtin_abs(arg1);
      break;

    case ST_BUILTIN_MIN:
      result = st_builtin_min(arg1, arg2);
      break;

    case ST_BUILTIN_MAX:
      result = st_builtin_max(arg1, arg2);
      break;

    case ST_BUILTIN_SUM:
      // Alias for addition
      result.int_val = arg1.int_val + arg2.int_val;
      break;

    case ST_BUILTIN_SQRT:
      result = st_builtin_sqrt(arg1);
      break;

    case ST_BUILTIN_ROUND:
      result = st_builtin_round(arg1);
      break;

    case ST_BUILTIN_TRUNC:
      result = st_builtin_trunc(arg1);
      break;

    case ST_BUILTIN_FLOOR:
      result = st_builtin_floor(arg1);
      break;

    case ST_BUILTIN_CEIL:
      result = st_builtin_ceil(arg1);
      break;

    // Type conversions
    case ST_BUILTIN_INT_TO_REAL:
      result = st_builtin_int_to_real(arg1);
      break;

    case ST_BUILTIN_REAL_TO_INT:
      result = st_builtin_real_to_int(arg1);
      break;

    case ST_BUILTIN_BOOL_TO_INT:
      result = st_builtin_bool_to_int(arg1);
      break;

    case ST_BUILTIN_INT_TO_BOOL:
      result = st_builtin_int_to_bool(arg1);
      break;

    case ST_BUILTIN_DWORD_TO_INT:
      result = st_builtin_dword_to_int(arg1);
      break;

    case ST_BUILTIN_INT_TO_DWORD:
      result = st_builtin_int_to_dword(arg1);
      break;

    // Persistence (v4.0+)
    case ST_BUILTIN_PERSIST_SAVE:
      result = st_builtin_persist_save();
      break;

    case ST_BUILTIN_PERSIST_LOAD:
      result = st_builtin_persist_load();
      break;

    default:
      // Unknown function - return zero
      break;
  }

  return result;
}

/* ============================================================================
 * METADATA FUNCTIONS
 * ============================================================================ */

const char *st_builtin_name(st_builtin_func_t func_id) {
  switch (func_id) {
    case ST_BUILTIN_ABS:           return "ABS";
    case ST_BUILTIN_MIN:           return "MIN";
    case ST_BUILTIN_MAX:           return "MAX";
    case ST_BUILTIN_SUM:           return "SUM";
    case ST_BUILTIN_SQRT:          return "SQRT";
    case ST_BUILTIN_ROUND:         return "ROUND";
    case ST_BUILTIN_TRUNC:         return "TRUNC";
    case ST_BUILTIN_FLOOR:         return "FLOOR";
    case ST_BUILTIN_CEIL:          return "CEIL";
    case ST_BUILTIN_INT_TO_REAL:   return "INT_TO_REAL";
    case ST_BUILTIN_REAL_TO_INT:   return "REAL_TO_INT";
    case ST_BUILTIN_BOOL_TO_INT:   return "BOOL_TO_INT";
    case ST_BUILTIN_INT_TO_BOOL:   return "INT_TO_BOOL";
    case ST_BUILTIN_DWORD_TO_INT:  return "DWORD_TO_INT";
    case ST_BUILTIN_INT_TO_DWORD:  return "INT_TO_DWORD";
    case ST_BUILTIN_PERSIST_SAVE:  return "SAVE";
    case ST_BUILTIN_PERSIST_LOAD:  return "LOAD";
    default:                       return "UNKNOWN";
  }
}

uint8_t st_builtin_arg_count(st_builtin_func_t func_id) {
  switch (func_id) {
    // 1-argument functions
    case ST_BUILTIN_ABS:
    case ST_BUILTIN_SQRT:
    case ST_BUILTIN_ROUND:
    case ST_BUILTIN_TRUNC:
    case ST_BUILTIN_FLOOR:
    case ST_BUILTIN_CEIL:
    case ST_BUILTIN_INT_TO_REAL:
    case ST_BUILTIN_REAL_TO_INT:
    case ST_BUILTIN_BOOL_TO_INT:
    case ST_BUILTIN_INT_TO_BOOL:
    case ST_BUILTIN_DWORD_TO_INT:
    case ST_BUILTIN_INT_TO_DWORD:
      return 1;

    // 2-argument functions
    case ST_BUILTIN_MIN:
    case ST_BUILTIN_MAX:
    case ST_BUILTIN_SUM:
      return 2;

    // 0-argument functions (v4.0+)
    case ST_BUILTIN_PERSIST_SAVE:
    case ST_BUILTIN_PERSIST_LOAD:
      return 0;

    default:
      return 0;
  }
}
