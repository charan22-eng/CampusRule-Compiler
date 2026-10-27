/**
 * @file compiler.h
 * @brief Unified compiler pipeline coordinator and phase output aggregator.
 *
 * Orchestrates the sequential compilation passes of the CampusRule Compiler:
 *   Phase 0: Input source loading & normalization
 *   Phase 1: Lexical analysis (token scanning)
 *   Phase 2: Syntax analysis (recursive-descent parsing)
 *   Phase 3: Abstract Syntax Tree (AST) construction & visualization
 *   Phase 4: Semantic analysis & academic boundary validation
 *   Phase 5: Intermediate Three-Address Code (TAC) generation
 *   Phase 6: Runtime rule evaluation & student eligibility verdict
 */

#ifndef COMPILER_H
#define COMPILER_H

#include "common.h"
#include "token.h"
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "semantic.h"
#include "tac.h"
#include "evaluator.h"

/**
 * @brief Discrete sequential phases in the CampusRule compiler pipeline.
 */
typedef enum {
    PHASE_INPUT = 0,    /**< Source input ingestion */
    PHASE_LEXICAL,      /**< Scanner tokenization */
    PHASE_PARSING,      /**< Grammar syntax validation */
    PHASE_AST,          /**< AST representation and hierarchy */
    PHASE_SEMANTIC,     /**< Contextual constraint checking */
    PHASE_TAC,          /**< Intermediate code generation */
    PHASE_RESULT,       /**< Evaluation verdict & category decision breakdown */
    PHASE_COUNT         /**< Total phase count (7) */
} CompilerPhase;

typedef enum {
    STATUS_OK,
    STATUS_FAILED,
    STATUS_SKIPPED
} PhaseStatus;

typedef struct {
    char *text[PHASE_COUNT];
    PhaseStatus status[PHASE_COUNT];
    char *error;
    char *verdict;
    bool eligible;
    CategoryDecision exam_decision;
    CategoryDecision placement_decision;
    CategoryDecision scholarship_decision;
} CompilerOutput;

typedef struct {
    const char *title;
    const char *code;
} SampleRule;

const char *compiler_phase_name(CompilerPhase phase);
const char *compiler_status_name(PhaseStatus status);

size_t compiler_get_samples_count(void);
const SampleRule *compiler_get_sample(size_t index);

CompilerOutput *compiler_run(const char *source, const StudentData *student);
void compiler_free_output(CompilerOutput *out);

#endif /* COMPILER_H */
