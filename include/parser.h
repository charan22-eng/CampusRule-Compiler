/**
 * @file parser.h
 * @brief Recursive-descent parser interface for the CampusRule DSL grammar.
 *
 * Implements syntax analysis for the CampusRule grammar:
 *   <Program>   ::= <Rule>+
 *   <Rule>      ::= 'IF' <Condition> 'THEN' <Action> 'END'
 *   <Condition> ::= <LogicExpr>
 *   <LogicExpr> ::= <CompareExpr> ( ('AND' | 'OR') <CompareExpr> )*
 *   <CompareExpr> ::= <Variable> <RelOp> <Number> | '(' <Condition> ')'
 *   <Action>    ::= ['NOT'] 'ELIGIBLE' ['FOR'] <Category>
 */

#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "ast.h"

/**
 * @brief Parser execution state containing token stream and error tracking.
 */
typedef struct {
    const TokenList *tokens; /**< Input token list produced by the lexer */
    size_t position;         /**< Current token cursor index */
    char *error_msg;         /**< Dynamic error message buffer on syntax failure */
    int error_line;          /**< Line number of syntax error */
    int error_col;           /**< Column number of syntax error */
} Parser;

/**
 * @brief Parses an input TokenList into an Abstract Syntax Tree (ASTNode Program).
 *
 * @param tokens Token stream emitted by lexer_tokenize().
 * @param[out] out_error Pointer to allocated syntax error message on failure.
 * @param[out] out_line Line number of syntax error, if non-NULL.
 * @param[out] out_col Column number of syntax error, if non-NULL.
 * @return Root ASTNode of type AST_NODE_PROGRAM on success, or NULL on error.
 */
ASTNode *parser_parse(const TokenList *tokens, char **out_error, int *out_line, int *out_col);

#endif /* PARSER_H */
