/**
 * @file token.h
 * @brief Token definitions and stream abstractions for the CampusRule DSL compiler.
 *
 * Defines the complete lexical token catalog for the CampusRule domain-specific language,
 * including reserved keywords (IF, THEN, END, ELIGIBLE, etc.), comparison operators,
 * logical connectors, delimiters, literals, and token stream structures.
 */

#ifndef TOKEN_H
#define TOKEN_H

#include "common.h"

/**
 * @brief Lexical token types recognized by the CampusRule scanner.
 * Renamed to CRTokenType to prevent symbol collision with Windows SDK headers.
 */
typedef enum {
    /* Rule Structure Keywords */
    TOKEN_IF,           /**< 'IF' keyword starting a conditional rule */
    TOKEN_THEN,         /**< 'THEN' keyword separating condition from action */
    TOKEN_END,          /**< 'END' keyword terminating a rule block */
    TOKEN_ELIGIBLE,     /**< 'ELIGIBLE' keyword declaring eligibility status */
    TOKEN_NOT,          /**< 'NOT' keyword negating eligibility */
    TOKEN_AND,          /**< 'AND' logical conjunction connector */
    TOKEN_OR,           /**< 'OR' logical disjunction connector */
    TOKEN_EXAM,         /**< 'EXAM' category keyword */
    TOKEN_PLACEMENT,    /**< 'PLACEMENT' category keyword */
    TOKEN_SCHOLARSHIP,  /**< 'SCHOLARSHIP' category keyword */
    TOKEN_FOR,          /**< 'FOR' optional preposition keyword */

    /* Operators */
    TOKEN_GREATER,       /* > */
    TOKEN_LESS,          /* < */
    TOKEN_GREATER_EQUAL, /* >= */
    TOKEN_LESS_EQUAL,    /* <= */
    TOKEN_EQUAL,         /* == */
    TOKEN_NOT_EQUAL,     /* != */

    /* Delimiters */
    TOKEN_LPAREN,        /* ( */
    TOKEN_RPAREN,        /* ) */

    /* Literals / Identifiers */
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,

    /* End of input */
    TOKEN_EOF
} CRTokenType;

typedef struct {
    CRTokenType type;
    char *value;
    double num_value;
    int line;
    int col;
    int pos;
} Token;

typedef struct {
    Token *items;
    size_t count;
    size_t capacity;
} TokenList;

const char *token_type_to_string(CRTokenType type);
Token token_create(CRTokenType type, const char *value, double num_value, int line, int col, int pos);
char *token_format(const Token *t);
void token_free_members(Token *t);

TokenList *token_list_create(void);
void token_list_append(TokenList *list, Token t);
void token_list_free(TokenList *list);

#endif /* TOKEN_H */
