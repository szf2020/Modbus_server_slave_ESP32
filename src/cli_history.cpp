/**
 * @file cli_history.cpp
 * @brief CLI command history and navigation implementation (LAYER 7)
 *
 * Circular buffer for storing command history with navigation support
 */

#include "cli_history.h"
#include <string.h>

/* ============================================================================
 * HISTORY BUFFER
 * ============================================================================ */

static char cli_history_buffer[CLI_HISTORY_SIZE][CLI_HISTORY_LINE_LENGTH];
static uint8_t cli_history_count = 0;      // Number of valid entries
static uint8_t cli_history_head = 0;       // Next write position
static int8_t cli_history_nav_pos = -1;   // Navigation position (-1 = not navigating)

/* ============================================================================
 * INITIALIZATION
 * ============================================================================ */

void cli_history_init(void) {
  memset(cli_history_buffer, 0, sizeof(cli_history_buffer));
  cli_history_count = 0;
  cli_history_head = 0;
  cli_history_nav_pos = -1;
}

/* ============================================================================
 * ADD TO HISTORY
 * ============================================================================ */

void cli_history_add(const char* command) {
  if (!command || strlen(command) == 0) {
    return;  // Don't store empty commands
  }

  // Store command in circular buffer
  strncpy(cli_history_buffer[cli_history_head], command, CLI_HISTORY_LINE_LENGTH - 1);
  cli_history_buffer[cli_history_head][CLI_HISTORY_LINE_LENGTH - 1] = '\0';

  // Update counters
  cli_history_head = (cli_history_head + 1) % CLI_HISTORY_SIZE;
  if (cli_history_count < CLI_HISTORY_SIZE) {
    cli_history_count++;
  }

  // Reset navigation when new command added
  cli_history_nav_pos = -1;
}

/* ============================================================================
 * NAVIGATION
 * ============================================================================ */

const char* cli_history_get_prev(void) {
  if (cli_history_count == 0) {
    return NULL;  // No history
  }

  // First press of up arrow: start from most recent
  if (cli_history_nav_pos == -1) {
    cli_history_nav_pos = cli_history_count - 1;
  } else if (cli_history_nav_pos > 0) {
    // Go further back
    cli_history_nav_pos--;
  } else {
    // Already at oldest, stay there
    cli_history_nav_pos = 0;
  }

  // Calculate buffer index for this position
  // Formula: index = (head - count + nav_pos + SIZE) % SIZE
  uint8_t index = (cli_history_head - cli_history_count + cli_history_nav_pos + CLI_HISTORY_SIZE) % CLI_HISTORY_SIZE;
  return cli_history_buffer[index];
}

const char* cli_history_get_next(void) {
  if (cli_history_count == 0 || cli_history_nav_pos == -1) {
    return NULL;  // Not navigating
  }

  // Move forward in history (towards newest)
  if (cli_history_nav_pos < cli_history_count - 1) {
    cli_history_nav_pos++;
  } else {
    // At newest, go back to "not navigating" state
    cli_history_nav_pos = -1;
    return "";  // Return empty string to clear line
  }

  // Calculate buffer index for this position
  uint8_t index = (cli_history_head - cli_history_count + cli_history_nav_pos + CLI_HISTORY_SIZE) % CLI_HISTORY_SIZE;
  return cli_history_buffer[index];
}

void cli_history_reset_nav(void) {
  cli_history_nav_pos = -1;
}
