/**
 * @file semantic.h
 * @brief Semantic analysis and academic constraint validator for the CampusRule DSL.
 *
 * Verifies contextual validity of AST nodes according to college academic policies:
 *   - Variable resolution: only ATTENDANCE, CGPA, MARKS, BACKLOGS allowed.
 *   - Range validation: ATTENDANCE in [0, 100], CGPA in [0, 10], MARKS in [0, 100], BACKLOGS in [0, 50].
 *   - Integer constraints: BACKLOGS must be a whole non-negative number.
 *   - Student data validation: verifies runtime student profile values before evaluation.
 */

#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"

/**
 * @brief Collection of semantic diagnostic error strings.
 */
typedef struct {
    char **errors;    /**< Dynamic array of allocated error messages */
    size_t count;     /**< Total errors recorded */
    size_t capacity;  /**< Buffer capacity */
} SemanticErrors;

/**
 * @brief Structured outcome of semantic analysis pass.
 */
typedef struct {
    char *report;       /**< Full formatted diagnostic report string */
    char *first_error;  /**< Summary text of the first encountered error */
    int error_count;    /**< Total semantic errors detected */
    bool success;       /**< True if zero semantic errors were found */
} SemanticResult;

/**
 * @brief Performs semantic validation on an AST program.
 *
 * @param program AST root node (AST_NODE_PROGRAM).
 * @return SemanticResult containing success flag and diagnostic report.
 */
SemanticResult semantic_analyze(const ASTNode *program);

/**
 * @brief Releases heap memory allocated inside a SemanticResult.
 */
void semantic_result_free(SemanticResult *res);

/**
 * @brief Validates runtime student academic parameters against domain bounds.
 *
 * @param student Pointer to student record.
 * @param[out] out_error Pointer to allocated error message on violation.
 * @return True if all student parameters are physically valid, false otherwise.
 */
bool semantic_validate_student(const StudentData *student, char **out_error);

#endif /* SEMANTIC_H */
