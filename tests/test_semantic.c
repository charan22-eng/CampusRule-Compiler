#include "semantic.h"
#include "parser.h"
#include "lexer.h"
#include <assert.h>

static int g_semantic_tests_run = 0;
static int g_semantic_tests_passed = 0;

#define TEST_ASSERT(cond, msg) do { \
    g_semantic_tests_run++; \
    if (cond) { \
        g_semantic_tests_passed++; \
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

void test_semantic_valid_program(void) {
    const char *src = "IF ATTENDANCE >= 75 AND BACKLOGS == 0 THEN\nELIGIBLE EXAM\nEND\n\n"
                      "IF CGPA >= 7.5 AND MARKS >= 60 THEN\nELIGIBLE PLACEMENT\nEND";
    ASTNode *ast = parse_string(src);
    TEST_ASSERT(ast != NULL, "Parsing should succeed");

    SemanticResult res = semantic_analyze(ast);
    TEST_ASSERT(res.success, "Semantic analysis should succeed");
    TEST_ASSERT(res.error_count == 0, "No semantic errors");
    TEST_ASSERT(strstr(res.report, "Semantic Analysis Successful") != NULL, "Report indicates success");

    semantic_result_free(&res);
    ast_free(ast);
}

void test_semantic_unknown_variable(void) {
    const char *src = "IF CREDITS >= 20 THEN\nELIGIBLE EXAM\nEND";
    ASTNode *ast = parse_string(src);
    TEST_ASSERT(ast != NULL, "Parsing should succeed");

    SemanticResult res = semantic_analyze(ast);
    TEST_ASSERT(!res.success, "Semantic analysis should fail for unknown variable");
    TEST_ASSERT(res.error_count > 0, "Errors detected");
    TEST_ASSERT(strstr(res.report, "Unknown variable 'CREDITS'") != NULL, "Report notes unknown variable");

    semantic_result_free(&res);
    ast_free(ast);
}

void test_semantic_out_of_range(void) {
    const char *src = "IF ATTENDANCE >= 120 AND CGPA > 12 THEN\nELIGIBLE EXAM\nEND";
    ASTNode *ast = parse_string(src);
    TEST_ASSERT(ast != NULL, "Parsing should succeed");

    SemanticResult res = semantic_analyze(ast);
    TEST_ASSERT(!res.success, "Semantic analysis should fail for out of range");
    TEST_ASSERT(res.error_count == 2, "Should find 2 semantic errors");
    TEST_ASSERT(strstr(res.report, "ATTENDANCE = 120.0 is outside the valid range [0, 100]") != NULL, "Reports attendance error");
    TEST_ASSERT(strstr(res.report, "CGPA = 12.0 is outside the valid range [0, 10]") != NULL, "Reports CGPA error");

    semantic_result_free(&res);
    ast_free(ast);
}

void test_semantic_fractional_backlogs(void) {
    const char *src = "IF BACKLOGS == 1.5 THEN\nELIGIBLE EXAM\nEND";
    ASTNode *ast = parse_string(src);
    TEST_ASSERT(ast != NULL, "Parsing should succeed");

    SemanticResult res = semantic_analyze(ast);
    TEST_ASSERT(!res.success, "Semantic analysis should fail for fractional backlogs");
    TEST_ASSERT(strstr(res.report, "BACKLOGS must be a whole number") != NULL, "Report notes whole number requirement");

    semantic_result_free(&res);
    ast_free(ast);
}

void test_semantic_duplicate_rule(void) {
    const char *src = "IF ATTENDANCE >= 75 THEN\nELIGIBLE EXAM\nEND\n\n"
                      "IF CGPA >= 8 THEN\nELIGIBLE EXAM\nEND";
    ASTNode *ast = parse_string(src);
    TEST_ASSERT(ast != NULL, "Parsing should succeed");

    SemanticResult res = semantic_analyze(ast);
    TEST_ASSERT(!res.success, "Duplicate rules should be flagged");
    TEST_ASSERT(strstr(res.report, "Duplicate rule") != NULL, "Report notes duplicate rule");

    semantic_result_free(&res);
    ast_free(ast);
}

void test_semantic_student_validation(void) {
    StudentData valid = cr_default_student();
    char *err = NULL;
    TEST_ASSERT(semantic_validate_student(&valid, &err), "Default student is valid");

    StudentData invalid_att = valid;
    invalid_att.attendance = 105.0;
    TEST_ASSERT(!semantic_validate_student(&invalid_att, &err), "Attendance > 100 rejected");
    if (err) free(err);

    StudentData invalid_cgpa = valid;
    invalid_cgpa.cgpa = -1.0;
    TEST_ASSERT(!semantic_validate_student(&invalid_cgpa, &err), "Negative CGPA rejected");
    if (err) free(err);

    StudentData invalid_backlogs = valid;
    invalid_backlogs.backlogs = 2.5;
    TEST_ASSERT(!semantic_validate_student(&invalid_backlogs, &err), "Fractional backlog rejected");
    if (err) free(err);
}

int run_semantic_tests(void) {
    printf("Running Semantic Tests...\n");
    g_semantic_tests_run = 0;
    g_semantic_tests_passed = 0;

    test_semantic_valid_program();
    test_semantic_unknown_variable();
    test_semantic_out_of_range();
    test_semantic_fractional_backlogs();
    test_semantic_duplicate_rule();
    test_semantic_student_validation();

    printf("  Semantic Tests: %d/%d passed\n", g_semantic_tests_passed, g_semantic_tests_run);
    return (g_semantic_tests_passed == g_semantic_tests_run) ? 0 : 1;
}
