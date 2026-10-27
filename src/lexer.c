#include "lexer.h"

static void set_error(char **out_error, int *out_line, int *out_col,
                      const char *msg, int line, int col) {
    if (out_error) {
        *out_error = cr_strdup(msg);
    }
    if (out_line) *out_line = line;
    if (out_col) *out_col = col;
}

TokenList *lexer_tokenize(const char *source, char **out_error, int *out_line, int *out_col) {
    if (out_error) *out_error = NULL;
    if (out_line) *out_line = 0;
    if (out_col) *out_col = 0;

    if (!source) {
        set_error(out_error, out_line, out_col, "Source code is NULL", 1, 1);
        return NULL;
    }

    TokenList *tokens = token_list_create();
    if (!tokens) {
        set_error(out_error, out_line, out_col, "Memory allocation failure", 1, 1);
        return NULL;
    }

    size_t length = strlen(source);
    size_t pos = 0;
    int line = 1;
    int col = 1;

    while (pos < length) {
        char c = source[pos];

        if (isspace((unsigned char)c)) {
            if (c == '\n') {
                line++;
                col = 1;
            } else {
                col++;
            }
            pos++;
            continue;
        }

        /* Identifiers and Keywords */
        if (isalpha((unsigned char)c) || c == '_') {
            size_t start_pos = pos;
            int start_col = col;
            int start_line = line;

            while (pos < length && (isalnum((unsigned char)source[pos]) || source[pos] == '_')) {
                pos++;
                col++;
            }

            size_t word_len = pos - start_pos;
            char *word = (char *)malloc(word_len + 1);
            if (!word) {
                token_list_free(tokens);
                set_error(out_error, out_line, out_col, "Out of memory", line, col);
                return NULL;
            }

            for (size_t i = 0; i < word_len; i++) {
                word[i] = (char)toupper((unsigned char)source[start_pos + i]);
            }
            word[word_len] = '\0';

            CRTokenType type;
            if (strcmp(word, "IF") == 0) {
                type = TOKEN_IF;
            } else if (strcmp(word, "THEN") == 0) {
                type = TOKEN_THEN;
            } else if (strcmp(word, "END") == 0) {
                type = TOKEN_END;
            } else if (strcmp(word, "ELIGIBLE") == 0) {
                type = TOKEN_ELIGIBLE;
            } else if (strcmp(word, "EXAM") == 0) {
                type = TOKEN_EXAM;
            } else if (strcmp(word, "PLACEMENT") == 0) {
                type = TOKEN_PLACEMENT;
            } else if (strcmp(word, "SCHOLARSHIP") == 0) {
                type = TOKEN_SCHOLARSHIP;
            } else if (strcmp(word, "NOT") == 0) {
                type = TOKEN_NOT;
            } else if (strcmp(word, "AND") == 0) {
                type = TOKEN_AND;
            } else if (strcmp(word, "OR") == 0) {
                type = TOKEN_OR;
            } else if (strcmp(word, "FOR") == 0) {
                type = TOKEN_FOR;
            } else {
                type = TOKEN_IDENTIFIER;
            }

            Token tok = token_create(type, word, 0.0, start_line, start_col, (int)(start_pos + 1));
            token_list_append(tokens, tok);
            free(word);
            continue;
        }

        /* Numbers: integers and floating point */
        if (isdigit((unsigned char)c)) {
            size_t start_pos = pos;
            int start_col = col;
            int start_line = line;
            bool dot_seen = false;

            while (pos < length) {
                char ch = source[pos];
                if (isdigit((unsigned char)ch)) {
                    pos++;
                    col++;
                } else if (ch == '.' && !dot_seen && pos + 1 < length && isdigit((unsigned char)source[pos + 1])) {
                    dot_seen = true;
                    pos++;
                    col++;
                } else {
                    break;
                }
            }

            size_t num_len = pos - start_pos;
            char *num_str = cr_strndup(source + start_pos, num_len);
            char *endptr = NULL;
            double val = strtod(num_str, &endptr);

            if (endptr && *endptr != '\0') {
                char err_buf[128];
                snprintf(err_buf, sizeof(err_buf), "Syntax Error: Malformed number '%s'", num_str);
                set_error(out_error, out_line, out_col, err_buf, start_line, start_col);
                free(num_str);
                token_list_free(tokens);
                return NULL;
            }

            Token tok = token_create(TOKEN_NUMBER, num_str, val, start_line, start_col, (int)(start_pos + 1));
            token_list_append(tokens, tok);
            free(num_str);
            continue;
        }

        /* Operators & Delimiters */
        char next = (pos + 1 < length) ? source[pos + 1] : '\0';
        int cur_pos = (int)(pos + 1);
        int cur_line = line;
        int cur_col = col;

        if (c == '>') {
            if (next == '=') {
                token_list_append(tokens, token_create(TOKEN_GREATER_EQUAL, ">=", 0.0, cur_line, cur_col, cur_pos));
                pos += 2;
                col += 2;
            } else {
                token_list_append(tokens, token_create(TOKEN_GREATER, ">", 0.0, cur_line, cur_col, cur_pos));
                pos += 1;
                col += 1;
            }
            continue;
        }

        if (c == '<') {
            if (next == '=') {
                token_list_append(tokens, token_create(TOKEN_LESS_EQUAL, "<=", 0.0, cur_line, cur_col, cur_pos));
                pos += 2;
                col += 2;
            } else {
                token_list_append(tokens, token_create(TOKEN_LESS, "<", 0.0, cur_line, cur_col, cur_pos));
                pos += 1;
                col += 1;
            }
            continue;
        }

        if (c == '=') {
            if (next == '=') {
                token_list_append(tokens, token_create(TOKEN_EQUAL, "==", 0.0, cur_line, cur_col, cur_pos));
                pos += 2;
                col += 2;
                continue;
            } else {
                char err_buf[128];
                snprintf(err_buf, sizeof(err_buf), "Lexical Error: Invalid character '=' at position %d", cur_pos);
                set_error(out_error, out_line, out_col, err_buf, cur_line, cur_col);
                token_list_free(tokens);
                return NULL;
            }
        }

        if (c == '!') {
            if (next == '=') {
                token_list_append(tokens, token_create(TOKEN_NOT_EQUAL, "!=", 0.0, cur_line, cur_col, cur_pos));
                pos += 2;
                col += 2;
                continue;
            } else {
                char err_buf[128];
                snprintf(err_buf, sizeof(err_buf), "Lexical Error: Invalid character '!' at position %d", cur_pos);
                set_error(out_error, out_line, out_col, err_buf, cur_line, cur_col);
                token_list_free(tokens);
                return NULL;
            }
        }

        if (c == '(') {
            token_list_append(tokens, token_create(TOKEN_LPAREN, "(", 0.0, cur_line, cur_col, cur_pos));
            pos++;
            col++;
            continue;
        }

        if (c == ')') {
            token_list_append(tokens, token_create(TOKEN_RPAREN, ")", 0.0, cur_line, cur_col, cur_pos));
            pos++;
            col++;
            continue;
        }

        /* Any other character is invalid */
        char err_buf[128];
        snprintf(err_buf, sizeof(err_buf), "Lexical Error: Invalid character '%c' at position %d", c, cur_pos);
        set_error(out_error, out_line, out_col, err_buf, cur_line, cur_col);
        token_list_free(tokens);
        return NULL;
    }

    /* Append EOF token */
    Token eof_tok = token_create(TOKEN_EOF, "", 0.0, line, col, (int)(pos + 1));
    token_list_append(tokens, eof_tok);

    return tokens;
}
