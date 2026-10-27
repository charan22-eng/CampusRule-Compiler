/**
 * @file lexer.h
 * @brief Lexical analyzer (scanner) interface for the CampusRule DSL.
 *
 * Implements deterministic finite-automaton (DFA) based tokenization of rule source code,
 * supporting case-insensitive keywords, relational operators, parenthesized groupings,
 * floating-point/integer numeric literals, and precise line/column coordinate tracking.
 */

#ifndef LEXER_H
#define LEXER_H

#include "token.h"

/**
 * @brief Scanner state maintaining character position and line tracking.
 */
typedef struct {
    const char *source; /**< Null-terminated source code string */
    size_t length;      /**< Length of the source code */
    size_t pos;         /**< Current character scan offset */
    int line;           /**< 1-based source line counter */
    int col;            /**< 1-based source column counter */
} Lexer;

/**
 * @brief Scans DSL source code into a linear TokenList stream.
 *
 * @param source Null-terminated UTF-8/ASCII rule source code.
 * @param[out] out_error Pointer to allocated error message string if lexical error occurs.
 * @param[out] out_line Line number of lexical error (1-based), if non-NULL.
 * @param[out] out_col Column number of lexical error (1-based), if non-NULL.
 * @return Dynamically allocated TokenList on success, or NULL on lexical failure.
 */
TokenList *lexer_tokenize(const char *source, char **out_error, int *out_line, int *out_col);

#endif /* LEXER_H */
