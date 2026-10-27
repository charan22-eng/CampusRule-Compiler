#include "tac.h"
#include "parser.h"
#include "lexer.h"
#include <assert.h>

static int g_tac_tests_run = 0;
static int g_tac_tests_passed = 0;

#define TEST_ASSERT(cond, msg) do { \
    g_tac_tests_run++; \
    if (cond) { \
        g_tac_tests_passed++; \
    } else { \
        printf("  [FAIL] %s:%d: %s\n", __FILE__, __LINE__, msg); \
    } \
} while(0)

static ASTNode *parse_string(const char *src) {
    TokenList *tokens = lexer_tokenize(src, NULL, NULL, NULL);
    if (!tokens) return NULL;
    ASTNode *ast = parser_parse(tokens, NULL, NULL, NULL);
    token_list_free(tokens);
    return ast;
}

void test_tac_simple_rule(void) {
    const char *src = "IF ATTENDANCE >= 75 THEN\nELIGIBLE EXAM\nEND";
    ASTNode *ast = parse_string(src);
    TEST_ASSERT(ast != NULL, "Parsing should succeed");

    char *tac = tac_generate_string(ast);
    TEST_ASSERT(tac != NULL, "TAC generated");
    TEST_ASSERT(strstr(tac, "Rule 1:") != NULL, "Mentions Rule 1");
    TEST_ASSERT(strstr(tac, "t1 = ATTENDANCE >= 75.0") != NULL, "Assigns t1");
    TEST_ASSERT(strstr(tac, "IF_FALSE t1 GOTO L1") != NULL, "Emits IF_FALSE");
    TEST_ASSERT(strstr(tac, "PRINT \"ELIGIBLE FOR EXAM\"") != NULL, "Emits PRINT");
    TEST_ASSERT(strstr(tac, "L1:") != NULL, "Emits label L1");

    free(tac);
    ast_free(ast);
}

void test_tac_compound_and(void) {
    const char *src = "IF ATTENDANCE >= 75 AND BACKLOGS == 0 THEN\nELIGIBLE EXAM\nEND";
    ASTNode *ast = parse_string(src);
    TEST_ASSERT(ast != NULL, "Parsing should succeed");

    char *tac = tac_generate_string(ast);
    TEST_ASSERT(tac != NULL, "TAC generated");
    TEST_ASSERT(strstr(tac, "t1 = ATTENDANCE >= 75.0") != NULL, "Emits t1");
    TEST_ASSERT(strstr(tac, "t2 = BACKLOGS == 0.0") != NULL, "Emits t2");
    TEST_ASSERT(strstr(tac, "t3 = t1 AND t2") != NULL, "Emits t3 = t1 AND t2");

    free(tac);
    ast_free(ast);
}

void test_tac_multiple_rules(void) {
    const char *src = "IF ATTENDANCE >= 75 THEN\nELIGIBLE EXAM\nEND\n\n"
                      "IF CGPA >= 8 THEN\nELIGIBLE SCHOLARSHIP\nEND";
    ASTNode *ast = parse_string(src);
    TEST_ASSERT(ast != NULL, "Parsing should succeed");

    char *tac = tac_generate_string(ast);
    TEST_ASSERT(tac != NULL, "TAC generated");
    TEST_ASSERT(strstr(tac, "Rule 1:") != NULL, "Contains Rule 1");
    TEST_ASSERT(strstr(tac, "Rule 2:") != NULL, "Contains Rule 2");
    TEST_ASSERT(strstr(tac, "L1:") != NULL, "Contains Label 1");
    TEST_ASSERT(strstr(tac, "L2:") != NULL, "Contains Label 2");

    free(tac);
    ast_free(ast);
}

int run_tac_tests(void) {
    printf("Running TAC Tests...\n");
    g_tac_tests_run = 0;
    g_tac_tests_passed = 0;

    test_tac_simple_rule();
    test_tac_compound_and();
    test_tac_multiple_rules();

    printf("  TAC Tests: %d/%d passed\n", g_tac_tests_passed, g_tac_tests_run);
    return (g_tac_tests_passed == g_tac_tests_run) ? 0 : 1;
}
