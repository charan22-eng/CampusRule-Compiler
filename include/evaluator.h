/**
 * @file evaluator.h
 * @brief Dynamic rule evaluation engine and multi-category decision aggregator.
 *
 * Traverses the AST condition trees against runtime StudentData, computes boolean
 * satisfiability, evaluates rules independently per academic category (EXAM,
 * PLACEMENT, SCHOLARSHIP), tracks triggered actions, detects conflicting outcomes,
 * and formats detailed evaluation execution traces.
 */

#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "ast.h"

/**
 * @brief Decision outcome state for an academic eligibility category.
 */
typedef enum {
    DECISION_ELIGIBLE,      /**< Qualifying conditions met (positive outcome) */
    DECISION_NOT_ELIGIBLE,  /**< Explicit disqualify rule fired (negative outcome) */
    DECISION_CONFLICT,      /**< Both positive and negative rules triggered */
    DECISION_NO_DECISION    /**< No rules fired affecting this category */
} CategoryDecision;

/**
 * @brief Complete result of student evaluation against an AST program.
 */
typedef struct {
    CategoryDecision exam_decision;         /**< Final status for semester exams */
    CategoryDecision placement_decision;    /**< Final status for campus placements */
    CategoryDecision scholarship_decision;  /**< Final status for merit scholarships */

    char **fired_actions;                   /**< List of action strings triggered */
    size_t fired_count;                     /**< Number of triggered actions */

    char *trace;                            /**< Full step-by-step evaluation trace */
    char *verdict;                          /**< Consolidated human-readable verdict */
    bool eligible;                          /**< True if at least one positive rule fired with no conflict */
} EvaluationResult;

/**
 * @brief Converts a CategoryDecision enum to its display string representation.
 */
const char *decision_to_string(CategoryDecision d);

/**
 * @brief Evaluates an AST program against given student academic data.
 *
 * @param program AST root (AST_NODE_PROGRAM).
 * @param student Pointer to student parameters (attendance, cgpa, marks, backlogs).
 * @return EvaluationResult containing per-category decisions and trace string.
 */
EvaluationResult evaluator_evaluate(const ASTNode *program, const StudentData *student);

/**
 * @brief Deallocates all memory held by an EvaluationResult structure.
 */
void evaluation_result_free(EvaluationResult *res);

#endif /* EVALUATOR_H */
