#include "ast.h"
#include <assert.h>

static int g_ast_tests_run = 0;
static int g_ast_tests_passed = 0;

#define TEST_ASSERT(cond, msg) do { \
    g_ast_tests_run++; \
    if (cond) { \
        g_ast_tests_passed++; \
    } else { \
        printf("  [FAIL] %s:%d: %s\n", __FILE__, __LINE__, msg); \
    } \
} while(0)

void test_ast_node_creation(void) {
    ASTNode *cmp1 = ast_create_compare("ATTENDANCE", OP_GE, 75.0, 1, 4);
    TEST_ASSERT(cmp1 != NULL, "Compare node created");
    TEST_ASSERT(cmp1->type == AST_NODE_COMPARE, "Type is AST_NODE_COMPARE");
    TEST_ASSERT(strcmp(cmp1->as.compare.variable, "ATTENDANCE") == 0, "Var is ATTENDANCE");
    TEST_ASSERT(cmp1->as.compare.op == OP_GE, "Op is OP_GE");
    TEST_ASSERT(cmp1->as.compare.value == 75.0, "Val is 75.0");

    ASTNode *cmp2 = ast_create_compare("BACKLOGS", OP_EQ, 0.0, 1, 25);
    ASTNode *logic = ast_create_logic(LOGIC_AND, cmp1, cmp2, 1, 21);
    TEST_ASSERT(logic != NULL, "Logic node created");
    TEST_ASSERT(logic->type == AST_NODE_LOGIC, "Type is AST_NODE_LOGIC");

    ASTNode *rule = ast_create_rule(1, logic, false, CATEGORY_EXAM, "ELIGIBLE FOR EXAM", 1, 1);
    TEST_ASSERT(rule != NULL, "Rule node created");

    ASTNode *prog = ast_create_program();
    ast_program_add_rule(prog, rule);
    TEST_ASSERT(prog->as.program.count == 1, "Program has 1 rule");

    char *tree = ast_tree_to_string(prog);
    TEST_ASSERT(tree != NULL, "Tree string rendered");
    TEST_ASSERT(strstr(tree, "Program (1 rule(s))") != NULL, "Tree mentions rule count");
    TEST_ASSERT(strstr(tree, "ATTENDANCE >= 75.0") != NULL, "Tree mentions condition");
    TEST_ASSERT(strstr(tree, "Action: ELIGIBLE FOR EXAM") != NULL, "Tree mentions action");

    free(tree);
    ast_free(prog);
}

void test_ast_condition_string(void) {
    ASTNode *c1 = ast_create_compare("CGPA", OP_GE, 8.5, 1, 1);
    ASTNode *c2 = ast_create_compare("MARKS", OP_GE, 80.0, 1, 15);
    ASTNode *logic = ast_create_logic(LOGIC_AND, c1, c2, 1, 10);

    char *txt = ast_condition_to_string(logic);
    TEST_ASSERT(txt != NULL, "Condition string rendered");
    TEST_ASSERT(strstr(txt, "CGPA >= 8.5") != NULL, "Contains CGPA");
    TEST_ASSERT(strstr(txt, "MARKS >= 80.0") != NULL, "Contains MARKS");
    TEST_ASSERT(strstr(txt, "AND") != NULL, "Contains AND");

    free(txt);
    ast_free(logic);
}

int run_ast_tests(void) {
    printf("Running AST Tests...\n");
    g_ast_tests_run = 0;
    g_ast_tests_passed = 0;

    test_ast_node_creation();
    test_ast_condition_string();

    printf("  AST Tests: %d/%d passed\n", g_ast_tests_passed, g_ast_tests_run);
    return (g_ast_tests_passed == g_ast_tests_run) ? 0 : 1;
}
