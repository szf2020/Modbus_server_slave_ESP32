/**
 * @file st_stateful.cpp
 * @brief Stateful Storage Implementation
 *
 * Manages persistent state for ST function blocks (timers, edges, counters).
 */

#include "st_stateful.h"
#include <string.h>

/* ============================================================================
 * STORAGE INITIALIZATION
 * ============================================================================ */

void st_stateful_init(st_stateful_storage_t* storage) {
  if (!storage) return;

  // Clear all memory
  memset(storage, 0, sizeof(st_stateful_storage_t));

  // Reset counters
  storage->timer_count = 0;
  storage->edge_count = 0;
  storage->counter_count = 0;
  storage->latch_count = 0;  // v4.7.3: SR/RS latches

  // Mark as initialized
  storage->initialized = true;
}

void st_stateful_reset(st_stateful_storage_t* storage) {
  if (!storage || !storage->initialized) return;

  // Reset all timers
  for (uint8_t i = 0; i < storage->timer_count; i++) {
    storage->timers[i].Q = false;
    storage->timers[i].ET = 0;
    storage->timers[i].running = false;
    storage->timers[i].last_IN = false;
  }

  // Reset all edge detectors
  for (uint8_t i = 0; i < storage->edge_count; i++) {
    storage->edges[i].last_state = false;
  }

  // Reset all counters
  for (uint8_t i = 0; i < storage->counter_count; i++) {
    storage->counters[i].CV = 0;
    storage->counters[i].Q = false;
    storage->counters[i].QU = false;
    storage->counters[i].QD = false;
    storage->counters[i].last_CU = false;
    storage->counters[i].last_CD = false;
    storage->counters[i].last_RESET = false;
    storage->counters[i].last_LOAD = false;
  }

  // Reset all latches (v4.7.3)
  for (uint8_t i = 0; i < storage->latch_count; i++) {
    storage->latches[i].Q = false;
  }
}

/* ============================================================================
 * TIMER ALLOCATION
 * ============================================================================ */

st_timer_instance_t* st_stateful_alloc_timer(st_stateful_storage_t* storage, st_timer_type_t type) {
  if (!storage || !storage->initialized) return NULL;
  if (storage->timer_count >= ST_MAX_TIMER_INSTANCES) return NULL;

  // Get next slot
  st_timer_instance_t* timer = &storage->timers[storage->timer_count];
  storage->timer_count++;

  // Initialize timer
  memset(timer, 0, sizeof(st_timer_instance_t));
  timer->type = type;
  timer->Q = false;
  timer->ET = 0;
  timer->running = false;
  timer->last_IN = false;

  return timer;
}

st_timer_instance_t* st_stateful_get_timer(st_stateful_storage_t* storage, uint8_t instance_id) {
  if (!storage || !storage->initialized) return NULL;
  if (instance_id >= storage->timer_count) return NULL;
  return &storage->timers[instance_id];
}

/* ============================================================================
 * EDGE DETECTOR ALLOCATION
 * ============================================================================ */

st_edge_instance_t* st_stateful_alloc_edge(st_stateful_storage_t* storage, st_edge_type_t type) {
  if (!storage || !storage->initialized) return NULL;
  if (storage->edge_count >= ST_MAX_EDGE_INSTANCES) return NULL;

  // Get next slot
  st_edge_instance_t* edge = &storage->edges[storage->edge_count];
  storage->edge_count++;

  // Initialize edge detector
  memset(edge, 0, sizeof(st_edge_instance_t));
  edge->type = type;
  edge->last_state = false;

  return edge;
}

st_edge_instance_t* st_stateful_get_edge(st_stateful_storage_t* storage, uint8_t instance_id) {
  if (!storage || !storage->initialized) return NULL;
  if (instance_id >= storage->edge_count) return NULL;
  return &storage->edges[instance_id];
}

/* ============================================================================
 * COUNTER ALLOCATION
 * ============================================================================ */

st_counter_instance_t* st_stateful_alloc_counter(st_stateful_storage_t* storage, st_counter_type_t type) {
  if (!storage || !storage->initialized) return NULL;
  if (storage->counter_count >= ST_MAX_COUNTER_INSTANCES) return NULL;

  // Get next slot
  st_counter_instance_t* counter = &storage->counters[storage->counter_count];
  storage->counter_count++;

  // Initialize counter
  memset(counter, 0, sizeof(st_counter_instance_t));
  counter->type = type;
  counter->CV = 0;
  counter->PV = 0;
  counter->Q = false;
  counter->QU = false;
  counter->QD = false;
  counter->last_CU = false;
  counter->last_CD = false;
  counter->last_RESET = false;
  counter->last_LOAD = false;

  return counter;
}

st_counter_instance_t* st_stateful_get_counter(st_stateful_storage_t* storage, uint8_t instance_id) {
  if (!storage || !storage->initialized) return NULL;
  if (instance_id >= storage->counter_count) return NULL;
  return &storage->counters[instance_id];
}

/* ============================================================================
 * LATCH ALLOCATION
 * ============================================================================ */

st_latch_instance_t* st_stateful_alloc_latch(st_stateful_storage_t* storage, st_latch_type_t type) {
  if (!storage || !storage->initialized) return NULL;
  if (storage->latch_count >= ST_MAX_LATCH_INSTANCES) return NULL;

  // Get next slot
  st_latch_instance_t* latch = &storage->latches[storage->latch_count];
  storage->latch_count++;

  // Initialize latch
  memset(latch, 0, sizeof(st_latch_instance_t));
  latch->type = type;
  latch->Q = false;

  return latch;
}

st_latch_instance_t* st_stateful_get_latch(st_stateful_storage_t* storage, uint8_t instance_id) {
  if (!storage || !storage->initialized) return NULL;
  if (instance_id >= storage->latch_count) return NULL;
  return &storage->latches[instance_id];
}
