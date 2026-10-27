#include "lexer.h"
#include <assert.h>

static int g_lexer_tests_run = 0;
static int g_lexer_tests_passed = 0;

#define TEST_ASSERT(cond, msg) do { \
    g_lexer_tests_run++; \
    if (cond) { \
        g_lexer_tests_passed++; \
    } else { \
        printf("  [FAIL] %s:%d: %s\n", __FILE__, __LINE__, msg); \
    } \
} while(0)

void test_lexer_keywords(void) {
    const char *src = "IF THEN END ELIGIBLE NOT AND OR EXAM PLACEMENT SCHOLARSHIP";
    char *err = NULL;
    int line = 0, col = 0;
    TokenList *tokens = lexer_tokenize(src, &err, &line, &col);

    TEST_ASSERT(tokens != NULL, "Tokens should not be NULL");
    if (tokens) {
        TEST_ASSERT(tokens->count == 11, "Should have 10 keywords + 1 EOF");
        TEST_ASSERT(tokens->items[0].type == TOKEN_IF, "Token 0 should be IF");
        TEST_ASSERT(tokens->items[1].type == TOKEN_THEN, "Token 1 should be THEN");
        TEST_ASSERT(tokens->items[2].type == TOKEN_END, "Token 2 should be END");
        TEST_ASSERT(tokens->items[3].type == TOKEN_ELIGIBLE, "Token 3 should be ELIGIBLE");
        TEST_ASSERT(tokens->items[4].type == TOKEN_NOT, "Token 4 should be NOT");
        TEST_ASSERT(tokens->items[5].type == TOKEN_AND, "Token 5 should be AND");
        TEST_ASSERT(tokens->items[6].type == TOKEN_OR, "Token 6 should be OR");
        TEST_ASSERT(tokens->items[7].type == TOKEN_EXAM, "Token 7 should be EXAM");
        TEST_ASSERT(tokens->items[8].type == TOKEN_PLACEMENT, "Token 8 should be PLACEMENT");
        TEST_ASSERT(tokens->items[9].type == TOKEN_SCHOLARSHIP, "Token 9 should be SCHOLARSHIP");
        TEST_ASSERT(tokens->items[10].type == TOKEN_EOF, "Token 10 should be EOF");
        token_list_free(tokens);
    }
}

void test_lexer_operators(void) {
    const char *src = "> < >= <= == != ( )";
    char *err = NULL;
    int line = 0, col = 0;
    TokenList *tokens = lexer_tokenize(src, &err, &line, &col);

    TEST_ASSERT(tokens != NULL, "Tokens should not be NULL");
    if (tokens) {
        TEST_ASSERT(tokens->count == 9, "Should have 8 operators/delimiters + 1 EOF");
        TEST_ASSERT(tokens->items[0].type == TOKEN_GREATER, "Op 0 should be >");
        TEST_ASSERT(tokens->items[1].type == TOKEN_LESS, "Op 1 should be <");
        TEST_ASSERT(tokens->items[2].type == TOKEN_GREATER_EQUAL, "Op 2 should be >=");
        TEST_ASSERT(tokens->items[3].type == TOKEN_LESS_EQUAL, "Op 3 should be <=");
        TEST_ASSERT(tokens->items[4].type == TOKEN_EQUAL, "Op 4 should be ==");
        TEST_ASSERT(tokens->items[5].type == TOKEN_NOT_EQUAL, "Op 5 should be !=");
        TEST_ASSERT(tokens->items[6].type == TOKEN_LPAREN, "Op 6 should be (");
        TEST_ASSERT(tokens->items[7].type == TOKEN_RPAREN, "Op 7 should be )");
        token_list_free(tokens);
    }
}

void test_lexer_numbers_and_identifiers(void) {
    const char *src = "ATTENDANCE >= 75 AND CGPA >= 8.2";
    char *err = NULL;
    int line = 0, col = 0;
    TokenList *tokens = lexer_tokenize(src, &err, &line, &col);

    TEST_ASSERT(tokens != NULL, "Tokens should not be NULL");
    if (tokens) {
        TEST_ASSERT(tokens->items[0].type == TOKEN_IDENTIFIER, "Token 0 should be IDENTIFIER");
        TEST_ASSERT(strcmp(tokens->items[0].value, "ATTENDANCE") == 0, "Ident value should be ATTENDANCE");
        TEST_ASSERT(tokens->items[2].type == TOKEN_NUMBER, "Token 2 should be NUMBER");
        TEST_ASSERT(tokens->items[2].num_value == 75.0, "Token 2 should be 75.0");
        TEST_ASSERT(tokens->items[6].type == TOKEN_NUMBER, "Token 6 should be NUMBER");
        TEST_ASSERT(fabs(tokens->items[6].num_value - 8.2) < 1e-6, "Token 6 should be 8.2");
        token_list_free(tokens);
    }
}

void test_lexer_case_insensitivity(void) {
    const char *src = "if attendance >= 75 then eligible exam end";
    char *err = NULL;
    int line = 0, col = 0;
    TokenList *tokens = lexer_tokenize(src, &err, &line, &col);

    TEST_ASSERT(tokens != NULL, "Lowercase rule should be tokenized");
    if (tokens) {
        TEST_ASSERT(tokens->items[0].type == TOKEN_IF, "Token 0 should be IF");
        TEST_ASSERT(tokens->items[1].type == TOKEN_IDENTIFIER, "Token 1 should be IDENTIFIER");
        TEST_ASSERT(strcmp(tokens->items[1].value, "ATTENDANCE") == 0, "Identifier should be normalized to uppercase");
        TEST_ASSERT(tokens->items[2].type == TOKEN_GREATER_EQUAL, "Token 2 should be GREATER_EQUAL");
        TEST_ASSERT(tokens->items[3].type == TOKEN_NUMBER, "Token 3 should be NUMBER");
        TEST_ASSERT(tokens->items[4].type == TOKEN_THEN, "Token 4 should be THEN");
        token_list_free(tokens);
    }
}

void test_lexer_invalid_character(void) {
    const char *src = "IF ATTENDANCE >= 75 & BACKLOGS == 0 THEN";
    char *err = NULL;
    int line = 0, col = 0;
    TokenList *tokens = lexer_tokenize(src, &err, &line, &col);

    TEST_ASSERT(tokens == NULL, "Should fail on invalid character '&'");
    TEST_ASSERT(err != NULL, "Error message should be set");
    if (err) {
        TEST_ASSERT(strstr(err, "Invalid character '&'") != NULL, "Error message should mention '&'");
        free(err);
    }
}

void test_lexer_single_equal_error(void) {
    const char *src = "IF ATTENDANCE = 75 THEN";
    char *err = NULL;
    int line = 0, col = 0;
    TokenList *tokens = lexer_tokenize(src, &err, &line, &col);

    TEST_ASSERT(tokens == NULL, "Should fail on single '='");
    TEST_ASSERT(err != NULL, "Error message should be set");
    if (err) {
        TEST_ASSERT(strstr(err, "Invalid character '='") != NULL, "Error should report '='");
        free(err);
    }
}

int run_lexer_tests(void) {
    printf("Running Lexer Tests...\n");
    g_lexer_tests_run = 0;
    g_lexer_tests_passed = 0;

    test_lexer_keywords();
    test_lexer_operators();
    test_lexer_numbers_and_identifiers();
    test_lexer_case_insensitivity();
    test_lexer_invalid_character();
    test_lexer_single_equal_error();

    printf("  Lexer Tests: %d/%d passed\n", g_lexer_tests_passed, g_lexer_tests_run);
    return (g_lexer_tests_passed == g_lexer_tests_run) ? 0 : 1;
}
