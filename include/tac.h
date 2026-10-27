/**
 * @file tac.h
 * @brief Three-Address Code (TAC) intermediate representation generator.
 *
 * Translates high-level AST conditional expressions and rule action side effects
 * into linearized intermediate code instructions featuring temporary variables (t1, t2, ...),
 * conditional branch guards (IF_FALSE tN GOTO LM), and label-delineated blocks.
 */

#ifndef TAC_H
#define TAC_H

#include "ast.h"

/**
 * @brief Opcodes for Three-Address Code intermediate instructions.
 */
typedef enum {
    TAC_ASSIGN_COMPARE, /**< Temporary assignment of comparison: t1 = VAR >= CONST */
    TAC_ASSIGN_LOGIC,   /**< Temporary assignment of boolean logic: t3 = t1 AND t2 */
    TAC_IF_FALSE_GOTO,  /**< Conditional guard jump: IF_FALSE tN GOTO LM */
    TAC_PRINT,          /**< Side effect action execution: PRINT "ACTION_TEXT" */
    TAC_LABEL           /**< Branch target destination: LM: */
} TACOpType;

/**
 * @brief Single linearized Three-Address Code instruction record.
 */
typedef struct {
    TACOpType type;     /**< Instruction opcode */
    char *dest;         /**< Target temporary variable (e.g., "t1") or label ("L1") */
    char *arg1;         /**< Left operand (e.g., "ATTENDANCE" or "t1") */
    char *op;           /**< Operator symbol (e.g., ">=", "AND") */
    char *arg2;         /**< Right operand (e.g., "75.0" or "t2") */
    char *extra;        /**< Additional payload, such as rule action string */
} TACInstruction;

/**
 * @brief Complete sequence of generated TAC instructions with text rendering.
 */
typedef struct {
    TACInstruction *instructions; /**< Dynamic array of TAC instruction records */
    size_t count;                 /**< Number of generated instructions */
    size_t capacity;              /**< Storage buffer capacity */
    char *text_output;            /**< Linearized formatted text output */
} TACProgram;

/**
 * @brief Generates a structured TACProgram from an AST program.
 */
TACProgram *tac_generate_program(const ASTNode *program);

/**
 * @brief Generates linearized Three-Address Code formatted text directly.
 */
char *tac_generate_string(const ASTNode *program);

/**
 * @brief Frees all dynamic memory associated with a TACProgram.
 */
void tac_program_free(TACProgram *tac);

#endif /* TAC_H */
