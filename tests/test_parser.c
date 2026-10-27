#include "parser.h"
#include "lexer.h"
#include <assert.h>

static int g_parser_tests_run = 0;
static int g_parser_tests_passed = 0;

#define TEST_ASSERT(cond, msg) do { \
    g_parser_tests_run++; \
    if (cond) { \
        g_parser_tests_passed++; \
    } else { \
        printf("  [FAIL] %s:%d: %s\n", __FILE__, __LINE__, msg); \
    } \
} while(0)

void test_parser_single_rule(void) {
    const char *src = "IF ATTENDANCE >= 75 THEN\nELIGIBLE EXAM\nEND";
    char *err = NULL;
    TokenList *tokens = lexer_tokenize(src, &err, NULL, NULL);
    TEST_ASSERT(tokens != NULL, "Lexing single rule should succeed");

    char *parse_err = NULL;
    ASTNode *ast = parser_parse(tokens, &parse_err, NULL, NULL);
    TEST_ASSERT(ast != NULL, "Parsing single rule should succeed");
    TEST_ASSERT(parse_err == NULL, "No parse error");

    if (ast) {
        TEST_ASSERT(ast->type == AST_NODE_PROGRAM, "AST root is PROGRAM");
        TEST_ASSERT(ast->as.program.count == 1, "Program has 1 rule");
        const ASTNode *rule = ast->as.program.rules[0];
        TEST_ASSERT(rule->type == AST_NODE_RULE, "First node is RULE");
        TEST_ASSERT(rule->as.rule.action.category == CATEGORY_EXAM, "Action is EXAM");
        TEST_ASSERT(!rule->as.rule.action.is_not, "Action is positive");
        ast_free(ast);
    }
    token_list_free(tokens);
}

void test_parser_compound_and(void) {
    const char *src = "IF ATTENDANCE >= 75 AND BACKLOGS == 0 THEN\nELIGIBLE EXAM\nEND";
    TokenList *tokens = lexer_tokenize(src, NULL, NULL, NULL);
    ASTNode *ast = parser_parse(tokens, NULL, NULL, NULL);

    TEST_ASSERT(ast != NULL, "Parsing compound AND should succeed");
    if (ast) {
        const ASTNode *rule = ast->as.program.rules[0];
        TEST_ASSERT(rule->as.rule.condition->type == AST_NODE_LOGIC, "Condition is LOGIC");
        TEST_ASSERT(rule->as.rule.condition->as.logic.op == LOGIC_AND, "Logic op is AND");
        ast_free(ast);
    }
    token_list_free(tokens);
}

void test_parser_scholarship_rule(void) {
    const char *src = "IF CGPA >= 8.5 AND MARKS >= 80 THEN\nELIGIBLE SCHOLARSHIP\nEND";
    TokenList *tokens = lexer_tokenize(src, NULL, NULL, NULL);
    ASTNode *ast = parser_parse(tokens, NULL, NULL, NULL);

    TEST_ASSERT(ast != NULL, "Parsing scholarship rule should succeed");
    if (ast) {
        const ASTNode *rule = ast->as.program.rules[0];
        TEST_ASSERT(rule->as.rule.action.category == CATEGORY_SCHOLARSHIP, "Category is SCHOLARSHIP");
        ast_free(ast);
    }
    token_list_free(tokens);
}

void test_parser_multiple_rules(void) {
    const char *src = "IF ATTENDANCE >= 75 THEN\nELIGIBLE EXAM\nEND\n\n"
                      "IF CGPA >= 7 THEN\nELIGIBLE PLACEMENT\nEND";
    TokenList *tokens = lexer_tokenize(src, NULL, NULL, NULL);
    ASTNode *ast = parser_parse(tokens, NULL, NULL, NULL);

    TEST_ASSERT(ast != NULL, "Parsing multiple rules should succeed");
    if (ast) {
        TEST_ASSERT(ast->as.program.count == 2, "Program should have 2 rules");
        ast_free(ast);
    }
    token_list_free(tokens);
}

void test_parser_parentheses(void) {
    const char *src = "IF (ATTENDANCE >= 75 OR CGPA >= 8.0) AND BACKLOGS == 0 THEN\nELIGIBLE EXAM\nEND";
    TokenList *tokens = lexer_tokenize(src, NULL, NULL, NULL);
    ASTNode *ast = parser_parse(tokens, NULL, NULL, NULL);

    TEST_ASSERT(ast != NULL, "Parsing parentheses should succeed");
    if (ast) {
        const ASTNode *rule = ast->as.program.rules[0];
        TEST_ASSERT(rule->as.rule.condition->type == AST_NODE_LOGIC, "Root condition is LOGIC");
        TEST_ASSERT(rule->as.rule.condition->as.logic.op == LOGIC_AND, "Top logic op is AND");
        ast_free(ast);
    }
    token_list_free(tokens);
}

void test_parser_missing_then(void) {
    const char *src = "IF ATTENDANCE >= 75\nELIGIBLE EXAM\nEND";
    TokenList *tokens = lexer_tokenize(src, NULL, NULL, NULL);
    char *err = NULL;
    ASTNode *ast = parser_parse(tokens, &err, NULL, NULL);

    TEST_ASSERT(ast == NULL, "Parsing without THEN should fail");
    TEST_ASSERT(err != NULL, "Error message should be set");
    if (err) {
        TEST_ASSERT(strstr(err, "Expected THEN") != NULL, "Error should report missing THEN");
        free(err);
    }
    token_list_free(tokens);
}

void test_parser_empty_input(void) {
    const char *src = "   \n\t  ";
    TokenList *tokens = lexer_tokenize(src, NULL, NULL, NULL);
    char *err = NULL;
    ASTNode *ast = parser_parse(tokens, &err, NULL, NULL);

    TEST_ASSERT(ast == NULL, "Parsing empty input should fail");
    TEST_ASSERT(err != NULL, "Error message should be set");
    if (err) {
        TEST_ASSERT(strstr(err, "Empty input") != NULL, "Error should mention empty input");
        free(err);
    }
    token_list_free(tokens);
}

int run_parser_tests(void) {
    printf("Running Parser Tests...\n");
    g_parser_tests_run = 0;
    g_parser_tests_passed = 0;

    test_parser_single_rule();
    test_parser_compound_and();
    test_parser_scholarship_rule();
    test_parser_multiple_rules();
    test_parser_parentheses();
    test_parser_missing_then();
    test_parser_empty_input();

    printf("  Parser Tests: %d/%d passed\n", g_parser_tests_passed, g_parser_tests_run);
    return (g_parser_tests_passed == g_parser_tests_run) ? 0 : 1;
}
