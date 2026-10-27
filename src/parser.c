#include "parser.h"

static const Token *current(Parser *p) {
    if (p->position < p->tokens->count) {
        return &p->tokens->items[p->position];
    }
    return &p->tokens->items[p->tokens->count - 1]; /* EOF token */
}

static char *describe_token(const Token *t) {
    if (!t || t->type == TOKEN_EOF) {
        return cr_strdup("end of input");
    }
    const char *type_name = token_type_to_string(t->type);
    if (t->value && strcmp(type_name, t->value) == 0) {
        return cr_strdup(type_name);
    }
    size_t len = strlen(type_name) + (t->value ? strlen(t->value) : 0) + 8;
    char *buf = (char *)malloc(len);
    if (buf) {
        snprintf(buf, len, "%s '%s'", type_name, t->value ? t->value : "");
    }
    return buf;
}

static void set_parser_error(Parser *p, const char *msg) {
    if (p->error_msg) return; /* Keep first error */
    const Token *cur = current(p);
    char *desc = describe_token(cur);

    size_t len = strlen(msg) + (desc ? strlen(desc) : 0) + 128;
    p->error_msg = (char *)malloc(len);
    if (p->error_msg) {
        snprintf(p->error_msg, len, "Syntax Error (token %zu): %s but found %s",
                 p->position + 1, msg, desc ? desc : "");
    }
    p->error_line = cur->line;
    p->error_col = cur->col;
    free(desc);
}

static const Token *consume(Parser *p, CRTokenType expected, const char *expected_desc) {
    const Token *cur = current(p);
    if (cur->type != expected) {
        char err[128];
        snprintf(err, sizeof(err), "Expected %s", expected_desc ? expected_desc : token_type_to_string(expected));
        set_parser_error(p, err);
        return NULL;
    }
    p->position++;
    return cur;
}

static ASTNode *parse_expression(Parser *p);

static ASTNode *parse_compare(Parser *p) {
    const Token *var_tok = current(p);
    if (var_tok->type != TOKEN_IDENTIFIER) {
        set_parser_error(p, "Expected variable name");
        return NULL;
    }
    p->position++;

    const Token *op_tok = current(p);
    RelOp op;
    switch (op_tok->type) {
        case TOKEN_GREATER:       op = OP_GT; break;
        case TOKEN_LESS:          op = OP_LT; break;
        case TOKEN_GREATER_EQUAL: op = OP_GE; break;
        case TOKEN_LESS_EQUAL:    op = OP_LE; break;
        case TOKEN_EQUAL:         op = OP_EQ; break;
        case TOKEN_NOT_EQUAL:     op = OP_NE; break;
        default:
            set_parser_error(p, "Expected a comparison operator (>, <, >=, <=, ==, !=)");
            return NULL;
    }
    p->position++;

    const Token *num_tok = current(p);
    if (num_tok->type != TOKEN_NUMBER) {
        set_parser_error(p, "Expected numeric literal");
        return NULL;
    }
    p->position++;

    return ast_create_compare(var_tok->value, op, num_tok->num_value, var_tok->line, var_tok->col);
}

static ASTNode *parse_primary(Parser *p) {
    const Token *cur = current(p);
    if (cur->type == TOKEN_LPAREN) {
        p->position++;
        ASTNode *expr = parse_expression(p);
        if (!expr) return NULL;
        if (!consume(p, TOKEN_RPAREN, "')'")) {
            ast_free(expr);
            return NULL;
        }
        return expr;
    }
    return parse_compare(p);
}

static ASTNode *parse_and(Parser *p) {
    ASTNode *left = parse_primary(p);
    if (!left) return NULL;

    while (current(p)->type == TOKEN_AND) {
        const Token *and_tok = current(p);
        p->position++;
        ASTNode *right = parse_primary(p);
        if (!right) {
            ast_free(left);
            return NULL;
        }
        left = ast_create_logic(LOGIC_AND, left, right, and_tok->line, and_tok->col);
    }
    return left;
}

static ASTNode *parse_or(Parser *p) {
    ASTNode *left = parse_and(p);
    if (!left) return NULL;

    while (current(p)->type == TOKEN_OR) {
        const Token *or_tok = current(p);
        p->position++;
        ASTNode *right = parse_and(p);
        if (!right) {
            ast_free(left);
            return NULL;
        }
        left = ast_create_logic(LOGIC_OR, left, right, or_tok->line, or_tok->col);
    }
    return left;
}

static ASTNode *parse_expression(Parser *p) {
    return parse_or(p);
}

static bool parse_action(Parser *p, bool *is_not, ActionCategory *cat, char **action_text) {
    *is_not = false;
    if (current(p)->type == TOKEN_NOT) {
        p->position++;
        *is_not = true;
    }

    if (!consume(p, TOKEN_ELIGIBLE, "ELIGIBLE")) {
        return false;
    }

    /* Optional FOR keyword: ELIGIBLE FOR EXAM or ELIGIBLE EXAM */
    if (current(p)->type == TOKEN_FOR) {
        p->position++;
    }

    const Token *cat_tok = current(p);
    if (cat_tok->type == TOKEN_EXAM) {
        *cat = CATEGORY_EXAM;
        p->position++;
    } else if (cat_tok->type == TOKEN_PLACEMENT) {
        *cat = CATEGORY_PLACEMENT;
        p->position++;
    } else if (cat_tok->type == TOKEN_SCHOLARSHIP) {
        *cat = CATEGORY_SCHOLARSHIP;
        p->position++;
    } else {
        set_parser_error(p, "Expected EXAM, PLACEMENT or SCHOLARSHIP");
        return false;
    }

    char buf[128];
    snprintf(buf, sizeof(buf), "%sELIGIBLE FOR %s",
             *is_not ? "NOT " : "",
             category_to_str(*cat));
    *action_text = cr_strdup(buf);
    return true;
}

static ASTNode *parse_rule(Parser *p, int index) {
    const Token *if_tok = current(p);
    if (!consume(p, TOKEN_IF, "IF")) {
        return NULL;
    }

    ASTNode *condition = parse_expression(p);
    if (!condition) {
        return NULL;
    }

    if (!consume(p, TOKEN_THEN, "THEN")) {
        ast_free(condition);
        return NULL;
    }

    bool is_not = false;
    ActionCategory cat = CATEGORY_UNKNOWN;
    char *action_text = NULL;
    if (!parse_action(p, &is_not, &cat, &action_text)) {
        ast_free(condition);
        return NULL;
    }

    if (!consume(p, TOKEN_END, "END")) {
        ast_free(condition);
        free(action_text);
        return NULL;
    }

    ASTNode *rule = ast_create_rule(index, condition, is_not, cat, action_text, if_tok->line, if_tok->col);
    free(action_text);
    return rule;
}

ASTNode *parser_parse(const TokenList *tokens, char **out_error, int *out_line, int *out_col) {
    if (out_error) *out_error = NULL;
    if (out_line) *out_line = 0;
    if (out_col) *out_col = 0;

    if (!tokens || tokens->count == 0 || tokens->items[0].type == TOKEN_EOF) {
        if (out_error) {
            *out_error = cr_strdup("Syntax Error: Empty input, expected a rule starting with IF");
        }
        if (out_line) *out_line = 1;
        if (out_col) *out_col = 1;
        return NULL;
    }

    Parser p;
    p.tokens = tokens;
    p.position = 0;
    p.error_msg = NULL;
    p.error_line = 0;
    p.error_col = 0;

    ASTNode *program = ast_create_program();
    if (!program) {
        if (out_error) *out_error = cr_strdup("Out of memory");
        return NULL;
    }

    int rule_index = 1;
    while (current(&p)->type != TOKEN_EOF) {
        ASTNode *rule = parse_rule(&p, rule_index++);
        if (!rule) {
            ast_free(program);
            if (out_error) *out_error = p.error_msg;
            else free(p.error_msg);
            if (out_line) *out_line = p.error_line;
            if (out_col) *out_col = p.error_col;
            return NULL;
        }
        ast_program_add_rule(program, rule);
    }

    return program;
}
