/**
 * @file cli_shell.cpp
 * @brief CLI shell implementation - serial I/O and command loop (LAYER 7)
 *
 * Handles:
 * - Non-blocking serial input reading
 * - Command line buffering
 * - Prompt display
 * - Command execution via cli_parser
 * - Command history navigation (up/down arrows)
 * - Remote echo control
 */

#include "cli_shell.h"
#include "cli_parser.h"
#include "cli_history.h"
#include "debug.h"
#include <Arduino.h>
#include <string.h>

/* ============================================================================
 * INPUT BUFFER & STATE
 * ============================================================================ */

#define CLI_INPUT_BUFFER_SIZE 256

static char cli_input_buffer[CLI_INPUT_BUFFER_SIZE];
static uint16_t cli_input_pos = 0;          // Length of input
static uint16_t cli_cursor_pos = 0;         // Cursor position (0 = start)
static uint8_t cli_initialized = 0;
static uint8_t cli_remote_echo_enabled = 1;  // Default: echo enabled
static uint8_t cli_escape_seq_state = 0;    // For parsing arrow keys

/* ============================================================================
 * INITIALIZATION
 * ============================================================================ */

void cli_shell_init(void) {
  if (cli_initialized) return;

  cli_history_init();
  debug_println("\nModbus CLI Ready. Type 'help' for commands.\n");
  debug_print("> ");

  cli_input_pos = 0;
  cli_cursor_pos = 0;
  memset(cli_input_buffer, 0, CLI_INPUT_BUFFER_SIZE);
  cli_escape_seq_state = 0;
  cli_initialized = 1;
}

/* ============================================================================
 * REMOTE ECHO CONTROL
 * ============================================================================ */

void cli_shell_set_remote_echo(uint8_t enabled) {
  cli_remote_echo_enabled = enabled ? 1 : 0;
}

uint8_t cli_shell_get_remote_echo(void) {
  return cli_remote_echo_enabled;
}

/* ============================================================================
 * HELPER FUNCTIONS
 * ============================================================================ */

static void cli_redraw_line(void) {
  // Move to start of line after prompt
  Serial.write('\r');  // Carriage return
  Serial.print("> ");

  // Clear rest of line with spaces
  for (uint16_t i = 0; i < CLI_INPUT_BUFFER_SIZE - 10; i++) {
    Serial.write(' ');
  }

  // Go back to start of input area
  Serial.write('\r');
  Serial.print("> ");

  // Display buffer content
  if (cli_input_pos > 0) {
    for (uint16_t i = 0; i < cli_input_pos; i++) {
      Serial.write(cli_input_buffer[i]);
    }
  }

  // Position cursor at correct location
  if (cli_cursor_pos < cli_input_pos) {
    // Move cursor back to position
    uint16_t move_back = cli_input_pos - cli_cursor_pos;
    for (uint16_t i = 0; i < move_back; i++) {
      Serial.write('\b');  // Backspace
    }
  }

  Serial.flush();  // Force output to terminal immediately
}

static void cli_insert_char_at_cursor(char c) {
  if (cli_input_pos >= CLI_INPUT_BUFFER_SIZE - 1) {
    return;  // Buffer full
  }

  if (!cli_remote_echo_enabled) {
    // Simple case: no echo, just add to buffer
    if (cli_cursor_pos < cli_input_pos) {
      // Shift characters forward
      for (uint16_t i = cli_input_pos; i > cli_cursor_pos; i--) {
        cli_input_buffer[i] = cli_input_buffer[i - 1];
      }
    }
    cli_input_buffer[cli_cursor_pos] = c;
    cli_input_pos++;
    cli_cursor_pos++;
    return;
  }

  // Echo enabled: need to redraw
  if (cli_cursor_pos == cli_input_pos) {
    // Simple case: cursor at end, just append
    cli_input_buffer[cli_cursor_pos] = c;
    cli_input_pos++;
    cli_cursor_pos++;
    Serial.write(c);
    Serial.flush();
  } else {
    // Complex case: cursor in middle, need to redraw
    // Shift characters forward
    for (uint16_t i = cli_input_pos; i > cli_cursor_pos; i--) {
      cli_input_buffer[i] = cli_input_buffer[i - 1];
    }
    cli_input_buffer[cli_cursor_pos] = c;
    cli_input_pos++;
    cli_cursor_pos++;

    // Redraw from current position to end
    for (uint16_t i = cli_cursor_pos - 1; i < cli_input_pos; i++) {
      Serial.write(cli_input_buffer[i]);
    }

    // Move cursor back to right position
    uint16_t move_back = cli_input_pos - cli_cursor_pos;
    for (uint16_t i = 0; i < move_back; i++) {
      Serial.write('\b');
    }
    Serial.flush();
  }
}

/* ============================================================================
 * MAIN LOOP
 * ============================================================================ */

void cli_shell_loop(void) {
  // Non-blocking serial input reading
  while (Serial.available() > 0) {
    int c = Serial.read();

    // ========================================================================
    // ESCAPE SEQUENCE PARSING FOR ARROW KEYS
    // ========================================================================

    if (cli_escape_seq_state == 1) {
      // We received ESC, now expecting [
      if (c == '[') {
        cli_escape_seq_state = 2;
        continue;
      } else {
        // Invalid sequence, reset state and process this character
        cli_escape_seq_state = 0;
        // Fall through to normal character processing
      }
    } else if (cli_escape_seq_state == 2) {
      // We received ESC[, now expecting arrow key (A, B, C, D)
      cli_escape_seq_state = 0;

      if (c == 'A') {
        // Up arrow - get previous command
        const char* prev_cmd = cli_history_get_prev();
        if (prev_cmd != NULL && prev_cmd[0] != '\0') {
          cli_redraw_line();  // Clear current line
          memset(cli_input_buffer, 0, CLI_INPUT_BUFFER_SIZE);
          strncpy(cli_input_buffer, prev_cmd, CLI_INPUT_BUFFER_SIZE - 1);
          cli_input_pos = strlen(cli_input_buffer);
          cli_cursor_pos = cli_input_pos;  // Cursor at end
          cli_redraw_line();
        }
        continue;
      } else if (c == 'B') {
        // Down arrow - get next command
        const char* next_cmd = cli_history_get_next();
        if (next_cmd != NULL) {
          cli_redraw_line();  // Clear current line
          memset(cli_input_buffer, 0, CLI_INPUT_BUFFER_SIZE);
          if (next_cmd[0] != '\0') {
            strncpy(cli_input_buffer, next_cmd, CLI_INPUT_BUFFER_SIZE - 1);
            cli_input_pos = strlen(cli_input_buffer);
          } else {
            cli_input_pos = 0;
          }
          cli_cursor_pos = cli_input_pos;  // Cursor at end
          cli_redraw_line();
        }
        continue;
      } else if (c == 'C') {
        // Right arrow - move cursor forward
        if (cli_cursor_pos < cli_input_pos) {
          cli_cursor_pos++;
        }
        // Always send the control sequence back
        Serial.write(0x1B);
        Serial.write('[');
        Serial.write('C');
        Serial.flush();
        continue;
      } else if (c == 'D') {
        // Left arrow - move cursor backward
        if (cli_cursor_pos > 0) {
          cli_cursor_pos--;
        }
        // Always send the control sequence back
        Serial.write(0x1B);
        Serial.write('[');
        Serial.write('D');
        Serial.flush();
        continue;
      }
      // For any other character, fall through to normal processing
    }

    // Check if this is start of escape sequence
    if (c == 0x1B) {
      cli_escape_seq_state = 1;
      continue;
    }

    // ========================================================================
    // NORMAL CHARACTER PROCESSING
    // ========================================================================

    if (c == '\r' || c == '\n') {
      // End of line - execute command
      cli_input_buffer[cli_input_pos] = '\0';

      if (cli_remote_echo_enabled) {
        debug_println("");  // Echo newline
      } else {
        Serial.write('\n');  // Just newline, no echo
      }

      if (cli_input_pos > 0) {
        // Store in history and execute
        cli_history_add(cli_input_buffer);
        cli_history_reset_nav();
        cli_parser_execute(cli_input_buffer);
      }

      // Reset buffer and show new prompt
      cli_input_pos = 0;
      cli_cursor_pos = 0;
      memset(cli_input_buffer, 0, CLI_INPUT_BUFFER_SIZE);
      debug_print("> ");
    }
    else if (c == 0x08 || c == 0x7F) {
      // Backspace - delete character before cursor
      if (cli_cursor_pos > 0) {
        cli_cursor_pos--;

        // Shift characters back
        for (uint16_t i = cli_cursor_pos; i < cli_input_pos - 1; i++) {
          cli_input_buffer[i] = cli_input_buffer[i + 1];
        }
        cli_input_pos--;
        cli_input_buffer[cli_input_pos] = '\0';

        if (cli_remote_echo_enabled) {
          // Send backspace to terminal
          Serial.write('\b');
          Serial.write(' ');
          Serial.write('\b');

          // If there are characters after cursor, redraw them
          if (cli_cursor_pos < cli_input_pos) {
            for (uint16_t i = cli_cursor_pos; i < cli_input_pos; i++) {
              Serial.write(cli_input_buffer[i]);
            }
            Serial.write(' ');  // Clear the last position

            // Move cursor back to original position
            uint16_t move_back = cli_input_pos - cli_cursor_pos + 1;
            for (uint16_t i = 0; i < move_back; i++) {
              Serial.write('\b');
            }
          }
          Serial.flush();
        }
      }
    }
    else if (c >= 32 && c < 127) {
      // Printable character - insert at cursor position
      cli_insert_char_at_cursor(c);
    }
    // Ignore all other characters (control codes, etc.)
  }
}

