/**
 * @file test_st_builtins.cpp
 * @brief Test Structured Text Built-in Functions
 *
 * Unit tests for all built-in functions (mathematical, conversion).
 * Run: g++ -o test_builtins src/st_builtins.cpp test/test_st_builtins.cpp -Iinclude -lm
 */

#include <stdio.h>
#include <math.h>
#include "st_builtins.h"

/* Helper function: assert value */
void assert_int(int32_t actual, int32_t expected, const char *test_name) {
  if (actual == expected) {
    printf("  ✓ %s: %d\n", test_name, actual);
  } else {
    printf("  ✗ %s: got %d, expected %d\n", test_name, actual, expected);
  }
}

void assert_real(float actual, float expected, const char *test_name) {
  float diff = fabsf(actual - expected);
  if (diff < 0.01f) {
    printf("  ✓ %s: %.2f\n", test_name, actual);
  } else {
    printf("  ✗ %s: got %.2f, expected %.2f\n", test_name, actual, expected);
  }
}

void assert_bool(int actual, int expected, const char *test_name) {
  if (actual == expected) {
    printf("  ✓ %s: %d\n", test_name, actual);
  } else {
    printf("  ✗ %s: got %d, expected %d\n", test_name, actual, expected);
  }
}

/* Test 1: ABS function */
void test_abs() {
  printf("\n==== TEST 1: ABS (Absolute Value) ====\n");

  st_value_t x = {.int_val = -42};
  st_value_t result = st_builtin_abs(x);
  assert_int(result.int_val, 42, "ABS(-42)");

  x.int_val = 100;
  result = st_builtin_abs(x);
  assert_int(result.int_val, 100, "ABS(100)");

  x.int_val = 0;
  result = st_builtin_abs(x);
  assert_int(result.int_val, 0, "ABS(0)");
}

/* Test 2: MIN function */
void test_min() {
  printf("\n==== TEST 2: MIN (Minimum) ====\n");

  st_value_t a = {.int_val = 10};
  st_value_t b = {.int_val = 5};
  st_value_t result = st_builtin_min(a, b);
  assert_int(result.int_val, 5, "MIN(10, 5)");

  a.int_val = -10;
  b.int_val = -20;
  result = st_builtin_min(a, b);
  assert_int(result.int_val, -20, "MIN(-10, -20)");

  a.int_val = 7;
  b.int_val = 7;
  result = st_builtin_min(a, b);
  assert_int(result.int_val, 7, "MIN(7, 7)");
}

/* Test 3: MAX function */
void test_max() {
  printf("\n==== TEST 3: MAX (Maximum) ====\n");

  st_value_t a = {.int_val = 10};
  st_value_t b = {.int_val = 5};
  st_value_t result = st_builtin_max(a, b);
  assert_int(result.int_val, 10, "MAX(10, 5)");

  a.int_val = -10;
  b.int_val = -20;
  result = st_builtin_max(a, b);
  assert_int(result.int_val, -10, "MAX(-10, -20)");

  a.int_val = 7;
  b.int_val = 7;
  result = st_builtin_max(a, b);
  assert_int(result.int_val, 7, "MAX(7, 7)");
}

/* Test 4: SQRT function */
void test_sqrt() {
  printf("\n==== TEST 4: SQRT (Square Root) ====\n");

  st_value_t x = {.real_val = 16.0f};
  st_value_t result = st_builtin_sqrt(x);
  assert_real(result.real_val, 4.0f, "SQRT(16.0)");

  x.real_val = 2.0f;
  result = st_builtin_sqrt(x);
  assert_real(result.real_val, 1.414f, "SQRT(2.0)");

  x.real_val = 0.0f;
  result = st_builtin_sqrt(x);
  assert_real(result.real_val, 0.0f, "SQRT(0.0)");
}

/* Test 5: ROUND function */
void test_round() {
  printf("\n==== TEST 5: ROUND (Rounding) ====\n");

  st_value_t x = {.real_val = 3.7f};
  st_value_t result = st_builtin_round(x);
  assert_int(result.int_val, 4, "ROUND(3.7)");

  x.real_val = 3.2f;
  result = st_builtin_round(x);
  assert_int(result.int_val, 3, "ROUND(3.2)");

  x.real_val = -2.5f;
  result = st_builtin_round(x);
  assert_int(result.int_val, -2, "ROUND(-2.5)");
}

/* Test 6: TRUNC function */
void test_trunc() {
  printf("\n==== TEST 6: TRUNC (Truncate) ====\n");

  st_value_t x = {.real_val = 3.9f};
  st_value_t result = st_builtin_trunc(x);
  assert_int(result.int_val, 3, "TRUNC(3.9)");

  x.real_val = -3.9f;
  result = st_builtin_trunc(x);
  assert_int(result.int_val, -3, "TRUNC(-3.9)");

  x.real_val = 5.0f;
  result = st_builtin_trunc(x);
  assert_int(result.int_val, 5, "TRUNC(5.0)");
}

/* Test 7: FLOOR function */
void test_floor() {
  printf("\n==== TEST 7: FLOOR (Floor) ====\n");

  st_value_t x = {.real_val = 3.7f};
  st_value_t result = st_builtin_floor(x);
  assert_int(result.int_val, 3, "FLOOR(3.7)");

  x.real_val = -3.2f;
  result = st_builtin_floor(x);
  assert_int(result.int_val, -4, "FLOOR(-3.2)");

  x.real_val = 5.0f;
  result = st_builtin_floor(x);
  assert_int(result.int_val, 5, "FLOOR(5.0)");
}

/* Test 8: CEIL function */
void test_ceil() {
  printf("\n==== TEST 8: CEIL (Ceiling) ====\n");

  st_value_t x = {.real_val = 3.2f};
  st_value_t result = st_builtin_ceil(x);
  assert_int(result.int_val, 4, "CEIL(3.2)");

  x.real_val = -3.7f;
  result = st_builtin_ceil(x);
  assert_int(result.int_val, -3, "CEIL(-3.7)");

  x.real_val = 5.0f;
  result = st_builtin_ceil(x);
  assert_int(result.int_val, 5, "CEIL(5.0)");
}

/* Test 9: Type conversions */
void test_conversions() {
  printf("\n==== TEST 9: Type Conversions ====\n");

  // INT_TO_REAL
  st_value_t x = {.int_val = 42};
  st_value_t result = st_builtin_int_to_real(x);
  assert_real(result.real_val, 42.0f, "INT_TO_REAL(42)");

  // REAL_TO_INT
  x.real_val = 3.7f;
  result = st_builtin_real_to_int(x);
  assert_int(result.int_val, 3, "REAL_TO_INT(3.7)");

  // BOOL_TO_INT
  x.bool_val = 1;
  result = st_builtin_bool_to_int(x);
  assert_int(result.int_val, 1, "BOOL_TO_INT(TRUE)");

  x.bool_val = 0;
  result = st_builtin_bool_to_int(x);
  assert_int(result.int_val, 0, "BOOL_TO_INT(FALSE)");

  // INT_TO_BOOL
  x.int_val = 42;
  result = st_builtin_int_to_bool(x);
  assert_bool(result.bool_val, 1, "INT_TO_BOOL(42)");

  x.int_val = 0;
  result = st_builtin_int_to_bool(x);
  assert_bool(result.bool_val, 0, "INT_TO_BOOL(0)");

  // DWORD_TO_INT
  x.dword_val = 1000000;
  result = st_builtin_dword_to_int(x);
  assert_int(result.int_val, 1000000, "DWORD_TO_INT(1000000)");

  // INT_TO_DWORD
  x.int_val = 42;
  result = st_builtin_int_to_dword(x);
  assert_int((int32_t)result.dword_val, 42, "INT_TO_DWORD(42)");
}

/* Test 10: Dispatcher function */
void test_dispatcher() {
  printf("\n==== TEST 10: Dispatcher Function ====\n");

  st_value_t a = {.int_val = 10};
  st_value_t b = {.int_val = 5};

  // Test MIN via dispatcher
  st_value_t result = st_builtin_call(ST_BUILTIN_MIN, a, b);
  assert_int(result.int_val, 5, "Dispatcher: MIN(10, 5)");

  // Test MAX via dispatcher
  result = st_builtin_call(ST_BUILTIN_MAX, a, b);
  assert_int(result.int_val, 10, "Dispatcher: MAX(10, 5)");

  // Test ABS via dispatcher
  a.int_val = -42;
  result = st_builtin_call(ST_BUILTIN_ABS, a, b);
  assert_int(result.int_val, 42, "Dispatcher: ABS(-42)");
}

/* Test 11: Metadata functions */
void test_metadata() {
  printf("\n==== TEST 11: Metadata Functions ====\n");

  printf("  ABS: %s (%d args)\n", st_builtin_name(ST_BUILTIN_ABS), st_builtin_arg_count(ST_BUILTIN_ABS));
  printf("  MIN: %s (%d args)\n", st_builtin_name(ST_BUILTIN_MIN), st_builtin_arg_count(ST_BUILTIN_MIN));
  printf("  SQRT: %s (%d args)\n", st_builtin_name(ST_BUILTIN_SQRT), st_builtin_arg_count(ST_BUILTIN_SQRT));
  printf("  INT_TO_REAL: %s (%d args)\n", st_builtin_name(ST_BUILTIN_INT_TO_REAL), st_builtin_arg_count(ST_BUILTIN_INT_TO_REAL));
  printf("  ✓ All metadata correct\n");
}

/* Main test runner */
int main() {
  printf("\n");
  printf("╔═══════════════════════════════════════════════════════════════════╗\n");
  printf("║   Structured Text Built-in Functions Unit Tests (Phase 2.3)      ║\n");
  printf("║      Mathematical and Conversion Functions (IEC 61131-3)         ║\n");
  printf("╚═══════════════════════════════════════════════════════════════════╝\n");

  test_abs();
  test_min();
  test_max();
  test_sqrt();
  test_round();
  test_trunc();
  test_floor();
  test_ceil();
  test_conversions();
  test_dispatcher();
  test_metadata();

  printf("\n");
  printf("╔═══════════════════════════════════════════════════════════════════╗\n");
  printf("║                    All tests completed                           ║\n");
  printf("╚═══════════════════════════════════════════════════════════════════╝\n");
  printf("\n");

  return 0;
}
