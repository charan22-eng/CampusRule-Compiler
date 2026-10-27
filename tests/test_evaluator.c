#include "evaluator.h"
#include "parser.h"
#include "lexer.h"
#include <assert.h>

static int g_eval_tests_run = 0;
static int g_eval_tests_passed = 0;

#define TEST_ASSERT(cond, msg) do { \
    g_eval_tests_run++; \
    if (cond) { \
        g_eval_tests_passed++; \
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

void test_evaluator_relational_operators(void) {
    StudentData s = cr_default_student(); /* Attendance=78.0, CGPA=8.2, MARKS=82.0, Backlogs=0.0 */

    /* > */
    ASTNode *ast_gt = parse_string("IF ATTENDANCE > 75 THEN\nELIGIBLE EXAM\nEND");
    EvaluationResult res_gt = evaluator_evaluate(ast_gt, &s);
    TEST_ASSERT(res_gt.fired_count == 1, "78.0 > 75.0 should fire");
    evaluation_result_free(&res_gt);
    ast_free(ast_gt);

    /* < */
    ASTNode *ast_lt = parse_string("IF ATTENDANCE < 75 THEN\nNOT ELIGIBLE EXAM\nEND");
    EvaluationResult res_lt = evaluator_evaluate(ast_lt, &s);
    TEST_ASSERT(res_lt.fired_count == 0, "78.0 < 75.0 should not fire");
    evaluation_result_free(&res_lt);
    ast_free(ast_lt);

    /* >= */
    ASTNode *ast_ge = parse_string("IF ATTENDANCE >= 78 THEN\nELIGIBLE EXAM\nEND");
    EvaluationResult res_ge = evaluator_evaluate(ast_ge, &s);
    TEST_ASSERT(res_ge.fired_count == 1, "78.0 >= 78.0 should fire");
    evaluation_result_free(&res_ge);
    ast_free(ast_ge);

    /* <= */
    ASTNode *ast_le = parse_string("IF BACKLOGS <= 0 THEN\nELIGIBLE EXAM\nEND");
    EvaluationResult res_le = evaluator_evaluate(ast_le, &s);
    TEST_ASSERT(res_le.fired_count == 1, "0.0 <= 0.0 should fire");
    evaluation_result_free(&res_le);
    ast_free(ast_le);

    /* == */
    ASTNode *ast_eq = parse_string("IF BACKLOGS == 0 THEN\nELIGIBLE EXAM\nEND");
    EvaluationResult res_eq = evaluator_evaluate(ast_eq, &s);
    TEST_ASSERT(res_eq.fired_count == 1, "0.0 == 0.0 should fire");
    evaluation_result_free(&res_eq);
    ast_free(ast_eq);

    /* != */
    ASTNode *ast_ne = parse_string("IF BACKLOGS != 0 THEN\nNOT ELIGIBLE EXAM\nEND");
    EvaluationResult res_ne = evaluator_evaluate(ast_ne, &s);
    TEST_ASSERT(res_ne.fired_count == 0, "0.0 != 0.0 should not fire");
    evaluation_result_free(&res_ne);
    ast_free(ast_ne);
}

void test_evaluator_multi_category_independence(void) {
    StudentData s = cr_default_student(); /* Attendance=78, CGPA=8.2, Marks=82, Backlogs=0 */
    const char *src = "IF ATTENDANCE >= 75 AND BACKLOGS == 0 THEN\nELIGIBLE EXAM\nEND\n\n"
                      "IF CGPA >= 7 AND MARKS >= 60 AND BACKLOGS == 0 THEN\nELIGIBLE PLACEMENT\nEND\n\n"
                      "IF CGPA >= 9.0 THEN\nELIGIBLE SCHOLARSHIP\nEND";

    ASTNode *ast = parse_string(src);
    TEST_ASSERT(ast != NULL, "Parsing should succeed");

    EvaluationResult res = evaluator_evaluate(ast, &s);
    TEST_ASSERT(res.fired_count == 2, "Rule 1 and Rule 2 should fire, Rule 3 shouldn't");
    TEST_ASSERT(res.exam_decision == DECISION_ELIGIBLE, "Exam is ELIGIBLE");
    TEST_ASSERT(res.placement_decision == DECISION_ELIGIBLE, "Placement is ELIGIBLE");
    TEST_ASSERT(res.scholarship_decision == DECISION_NO_DECISION, "Scholarship is NO DECISION");
    TEST_ASSERT(res.eligible, "Overall student eligible is true");

    evaluation_result_free(&res);
    ast_free(ast);
}

void test_evaluator_conflict_detection(void) {
    StudentData s = cr_default_student();
    /* Contradictory rules for EXAM */
    const char *src = "IF ATTENDANCE >= 75 THEN\nELIGIBLE EXAM\nEND\n\n"
                      "IF MARKS >= 80 THEN\nNOT ELIGIBLE EXAM\nEND";

    ASTNode *ast = parse_string(src);
    EvaluationResult res = evaluator_evaluate(ast, &s);

    TEST_ASSERT(res.exam_decision == DECISION_CONFLICT, "Both positive and negative fired -> CONFLICT");
    TEST_ASSERT(!res.eligible, "Conflict causes overall eligible to be false");

    evaluation_result_free(&res);
    ast_free(ast);
}

void test_evaluator_no_decision(void) {
    StudentData s = cr_default_student();
    const char *src = "IF CGPA >= 9.5 THEN\nELIGIBLE SCHOLARSHIP\nEND";

    ASTNode *ast = parse_string(src);
    EvaluationResult res = evaluator_evaluate(ast, &s);

    TEST_ASSERT(res.fired_count == 0, "No rules fired");
    TEST_ASSERT(res.exam_decision == DECISION_NO_DECISION, "Exam is NO DECISION");
    TEST_ASSERT(res.scholarship_decision == DECISION_NO_DECISION, "Scholarship is NO DECISION");
    TEST_ASSERT(strcmp(res.verdict, "NOT ELIGIBLE") == 0, "Verdict matches default when 0 rules fired");

    evaluation_result_free(&res);
    ast_free(ast);
}

int run_evaluator_tests(void) {
    printf("Running Evaluator Tests...\n");
    g_eval_tests_run = 0;
    g_eval_tests_passed = 0;

    test_evaluator_relational_operators();
    test_evaluator_multi_category_independence();
    test_evaluator_conflict_detection();
    test_evaluator_no_decision();

    printf("  Evaluator Tests: %d/%d passed\n", g_eval_tests_passed, g_eval_tests_run);
    return (g_eval_tests_passed == g_eval_tests_run) ? 0 : 1;
}
