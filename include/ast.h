/**
 * @file ast.h
 * @brief Abstract Syntax Tree (AST) node representations and tree manipulation APIs.
 *
 * Defines the hierarchical syntax tree structures representing CampusRule programs,
 * individual conditional rules, logical boolean trees (AND/OR), relational comparisons
 * (variable op value), and action definitions with eligibility states.
 */

#ifndef AST_H
#define AST_H

#include "common.h"

/**
 * @brief Relational comparison operators supported in rule conditions.
 */
typedef enum {
    OP_GT = 0,    /**< Greater than (>) */
    OP_LT,        /**< Less than (<) */
    OP_GE,        /**< Greater than or equal (>=) */
    OP_LE,        /**< Less than or equal (<=) */
    OP_EQ,        /**< Equal to (==) */
    OP_NE         /**< Not equal to (!=) */
} RelOp;

/**
 * @brief Logical binary connectors for compound conditions.
 */
typedef enum {
    LOGIC_AND = 0, /**< Logical conjunction (AND) */
    LOGIC_OR       /**< Logical disjunction (OR) */
} LogicOp;

/**
 * @brief Academic eligibility decision categories.
 */
typedef enum {
    CATEGORY_EXAM = 0,          /**< Semester exam qualification */
    CATEGORY_PLACEMENT,         /**< Campus recruitment placement eligibility */
    CATEGORY_SCHOLARSHIP,       /**< Merit scholarship qualification */
    CATEGORY_UNKNOWN            /**< Unrecognized or malformed category */
} ActionCategory;

typedef enum {
    AST_NODE_COMPARE,
    AST_NODE_LOGIC,
    AST_NODE_RULE,
    AST_NODE_PROGRAM
} ASTNodeType;

typedef struct ASTNode ASTNode;

typedef struct {
    char *variable;
    RelOp op;
    double value;
} ASTCompare;

typedef struct {
    LogicOp op;
    ASTNode *left;
    ASTNode *right;
} ASTLogic;

typedef struct {
    bool is_not;
    ActionCategory category;
    char *action_text;
} RuleAction;

typedef struct {
    int index;
    ASTNode *condition;
    RuleAction action;
} ASTRule;

typedef struct {
    ASTNode **rules;
    size_t count;
    size_t capacity;
} ASTProgram;

struct ASTNode {
    ASTNodeType type;
    int line;
    int col;
    union {
        ASTCompare compare;
        ASTLogic logic;
        ASTRule rule;
        ASTProgram program;
    } as;
};

const char *relop_to_str(RelOp op);
RelOp relop_from_str(const char *str);
const char *logicop_to_str(LogicOp op);
const char *category_to_str(ActionCategory cat);

ASTNode *ast_create_compare(const char *var, RelOp op, double val, int line, int col);
ASTNode *ast_create_logic(LogicOp op, ASTNode *left, ASTNode *right, int line, int col);
ASTNode *ast_create_rule(int index, ASTNode *condition, bool is_not, ActionCategory cat, const char *action_text, int line, int col);
ASTNode *ast_create_program(void);
void ast_program_add_rule(ASTNode *program, ASTNode *rule);
void ast_free(ASTNode *node);

char *ast_tree_to_string(const ASTNode *node);
char *ast_condition_to_string(const ASTNode *cond);

#endif /* AST_H */
