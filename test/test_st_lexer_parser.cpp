/**
 * @file test_st_lexer_parser.cpp
 * @brief Test program for ST Lexer and Parser
 *
 * Demonstrates tokenization and AST building with ST examples.
 * Run: g++ -o test_lexer_parser src/st_lexer.cpp src/st_parser.cpp test/test_st_lexer_parser.cpp -Iinclude
 */

#include <stdio.h>
#include <string.h>
#include "st_types.h"
#include "st_lexer.h"
#include "st_parser.h"

/* Test 1: Lexer - tokenize simple expression */
void test_lexer_simple() {
  printf("==== TEST 1: Lexer - Simple Expression ====\n");
  const char *code = "IF x > 10 THEN y := 1; END_IF;";

  st_lexer_t lexer;
  st_lexer_init(&lexer, code);

  st_token_t token;
  printf("Code: %s\n\n", code);
  printf("Tokens:\n");

  while (st_lexer_next_token(&lexer, &token)) {
    printf("  [%s] = '%s' (line %u, col %u)\n",
           st_token_type_to_string(token.type),
           token.value,
           token.line,
           token.column);

    if (token.type == ST_TOK_EOF) break;
    if (token.type == ST_TOK_ERROR) {
      printf("  ERROR: %s\n", token.value);
      break;
    }
  }
  printf("\n");
}

/* Test 2: Lexer - variable declaration */
void test_lexer_var_declaration() {
  printf("==== TEST 2: Lexer - Variable Declaration ====\n");
  const char *code = "VAR counter: INT := 0; limit: REAL := 3.14; END_VAR";

  st_lexer_t lexer;
  st_lexer_init(&lexer, code);

  st_token_t token;
  printf("Code: %s\n\n", code);
  printf("Tokens:\n");

  while (st_lexer_next_token(&lexer, &token)) {
    if (token.type != ST_TOK_EOF) {
      printf("  [%-15s] '%s'\n", st_token_type_to_string(token.type), token.value);
    }
    if (token.type == ST_TOK_EOF) break;
  }
  printf("\n");
}

/* Test 3: Lexer - numbers and literals */
void test_lexer_numbers() {
  printf("==== TEST 3: Lexer - Numbers and Literals ====\n");
  const char *code = "x := 123; y := 1.5; z := 0xFF; flag := TRUE;";

  st_lexer_t lexer;
  st_lexer_init(&lexer, code);

  st_token_t token;
  printf("Code: %s\n\n", code);
  printf("Tokens:\n");

  while (st_lexer_next_token(&lexer, &token)) {
    if (token.type != ST_TOK_EOF) {
      printf("  [%-15s] '%s'\n", st_token_type_to_string(token.type), token.value);
    }
    if (token.type == ST_TOK_EOF) break;
  }
  printf("\n");
}

/* Test 4: Parser - simple IF statement */
void test_parser_if_statement() {
  printf("==== TEST 4: Parser - Simple IF Statement ====\n");
  const char *code = "VAR x: INT; y: INT; END_VAR IF x > 10 THEN y := 1; ELSE y := 0; END_IF;";

  printf("Code: %s\n\n", code);

  st_parser_t parser;
  st_parser_init(&parser, code);

  st_program_t *program = st_parser_parse_program(&parser);
  if (!program) {
    printf("Parse error: %s\n", parser.error_msg);
    printf("\n");
    return;
  }

  printf("Parsed program: '%s'\n", program->name);
  printf("Variables: %d\n", program->var_count);
  for (int i = 0; i < program->var_count; i++) {
    printf("  - %s (type %d)\n", program->variables[i].name, program->variables[i].type);
  }
  printf("\nAST:\n");
  st_ast_node_print(program->body, 2);

  st_program_free(program);
  printf("\n");
}

/* Test 5: Parser - FOR loop */
void test_parser_for_loop() {
  printf("==== TEST 5: Parser - FOR Loop ====\n");
  const char *code = "VAR i: INT; sum: INT; END_VAR FOR i := 1 TO 10 DO sum := sum + i; END_FOR;";

  printf("Code: %s\n\n", code);

  st_parser_t parser;
  st_parser_init(&parser, code);

  st_program_t *program = st_parser_parse_program(&parser);
  if (!program) {
    printf("Parse error: %s\n", parser.error_msg);
    printf("\n");
    return;
  }

  printf("Parsed program: '%s'\n", program->name);
  printf("Variables: %d\n", program->var_count);
  printf("\nAST:\n");
  st_ast_node_print(program->body, 2);

  st_program_free(program);
  printf("\n");
}

/* Test 6: Parser - WHILE loop */
void test_parser_while_loop() {
  printf("==== TEST 6: Parser - WHILE Loop ====\n");
  const char *code = "VAR count: INT; END_VAR WHILE count < 100 DO count := count + 1; END_WHILE;";

  printf("Code: %s\n\n", code);

  st_parser_t parser;
  st_parser_init(&parser, code);

  st_program_t *program = st_parser_parse_program(&parser);
  if (!program) {
    printf("Parse error: %s\n", parser.error_msg);
    printf("\n");
    return;
  }

  printf("Parsed program: '%s'\n", program->name);
  printf("\nAST (simplified):\n");
  st_ast_node_print(program->body, 2);

  st_program_free(program);
  printf("\n");
}

/* Test 7: Parser - Complex expression */
void test_parser_expression() {
  printf("==== TEST 7: Parser - Complex Expression ====\n");
  const char *code = "VAR result: INT; a: INT; b: INT; END_VAR result := (a + b) * 2 > 10 AND a <> 0;";

  printf("Code: %s\n\n", code);

  st_parser_t parser;
  st_parser_init(&parser, code);

  st_program_t *program = st_parser_parse_program(&parser);
  if (!program) {
    printf("Parse error: %s\n", parser.error_msg);
    printf("\n");
    return;
  }

  printf("Parsed program: '%s'\n", program->name);
  printf("Variables: %d\n", program->var_count);
  printf("\nAST:\n");
  st_ast_node_print(program->body, 2);

  st_program_free(program);
  printf("\n");
}

/* Test 8: Parser - Real-world example */
void test_parser_realworld() {
  printf("==== TEST 8: Parser - Real-world Example ====\n");
  const char *code =
    "VAR\n"
    "  counter: INT := 0;\n"
    "  max_value: INT := 100;\n"
    "  enabled: BOOL;\n"
    "END_VAR\n"
    "IF enabled THEN\n"
    "  IF counter < max_value THEN\n"
    "    counter := counter + 1;\n"
    "  ELSE\n"
    "    counter := 0;\n"
    "  END_IF;\n"
    "END_IF;\n";

  printf("Code:\n%s\n\n", code);

  st_parser_t parser;
  st_parser_init(&parser, code);

  st_program_t *program = st_parser_parse_program(&parser);
  if (!program) {
    printf("Parse error: %s\n", parser.error_msg);
    printf("\n");
    return;
  }

  printf("SUCCESS: Parsed complex nested IF statement\n");
  printf("Variables: %d\n", program->var_count);
  for (int i = 0; i < program->var_count; i++) {
    printf("  - %s\n", program->variables[i].name);
  }
  printf("\nAST structure present: %s\n", program->body ? "Yes" : "No");

  st_program_free(program);
  printf("\n");
}

/* Main test runner */
int main() {
  printf("\n");
  printf("╔═══════════════════════════════════════════════════════════════════╗\n");
  printf("║     Structured Text (IEC 61131-3) Lexer & Parser Tests           ║\n");
  printf("║                    ST-Light Profile Demo                         ║\n");
  printf("╚═══════════════════════════════════════════════════════════════════╝\n");
  printf("\n");

  test_lexer_simple();
  test_lexer_var_declaration();
  test_lexer_numbers();
  test_parser_if_statement();
  test_parser_for_loop();
  test_parser_while_loop();
  test_parser_expression();
  test_parser_realworld();

  printf("╔═══════════════════════════════════════════════════════════════════╗\n");
  printf("║                    All tests completed                           ║\n");
  printf("╚═══════════════════════════════════════════════════════════════════╝\n");
  printf("\n");

  return 0;
}
