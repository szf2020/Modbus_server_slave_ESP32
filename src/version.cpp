/**
 * @file version.cpp
 * @brief Version information and changelog implementation
 */

#include "version.h"
#include "debug.h"

void version_print_changelog(void) {
  debug_println("\n=== Changelog ===\n");
  debug_println("v2.0.0 - Structured Text Logic Mode (MAJOR)");
  debug_println("  - IEC 61131-3 Structured Text (ST) language support");
  debug_println("  - ST Lexer (tokenization of all IEC 61131-3 keywords/operators)");
  debug_println("  - ST Parser (recursive descent, AST construction)");
  debug_println("  - Support: IF/THEN/ELSE, FOR, WHILE, REPEAT, CASE statements");
  debug_println("  - Data types: BOOL, INT, DWORD, REAL");
  debug_println("  - Variable declarations: VAR, VAR_INPUT, VAR_OUTPUT, CONST");
  debug_println("  - Full operator support (arithmetic, logical, relational, bitwise)");
  debug_println("  - Bytecode compiler & VM (Phase 2 design)");
  debug_println("  - 4 independent logic programs with Modbus integration");
  debug_println("  - IEC 61131-3 ST-Light profile compliance");
  debug_println("\nv1.0.0 - Initial release");
  debug_println("  - Counter Engine (SW, SW-ISR, HW-PCNT modes)");
  debug_println("  - Timer Engine (One-shot, Monostable, Astable, Input-triggered)");
  debug_println("  - CLI interface (show/set commands)");
  debug_println("  - GPIO driver abstraction");
  debug_println("  - PCNT hardware counter support");
  debug_println("  - Configuration persistence (NVS)");
  debug_println("\nv0.1.0 - Mega2560 port from v3.6.5");
  debug_println("  - Baseline architecture porting complete\n");
}
