#include <stdio.h>
#include <stdlib.h>

extern int run_lexer_tests(void);
extern int run_parser_tests(void);
extern int run_ast_tests(void);
extern int run_semantic_tests(void);
extern int run_tac_tests(void);
extern int run_evaluator_tests(void);

int main(void) {
    printf("====================================================\n");
    printf("  CampusRule Compiler - Automated Test Suite (C11)  \n");
    printf("====================================================\n\n");

    int failures = 0;
    failures += run_lexer_tests();
    printf("\n");
    failures += run_parser_tests();
    printf("\n");
    failures += run_ast_tests();
    printf("\n");
    failures += run_semantic_tests();
    printf("\n");
    failures += run_tac_tests();
    printf("\n");
    failures += run_evaluator_tests();
    printf("\n");

    printf("====================================================\n");
    if (failures == 0) {
        printf("  ALL TEST SUITES PASSED SUCCESSFULLY! (0 failures)\n");
    } else {
        printf("  TEST FAILURES DETECTED: %d suite(s) failed.\n", failures);
    }
    printf("====================================================\n");

    return (failures == 0) ? 0 : 1;
}
