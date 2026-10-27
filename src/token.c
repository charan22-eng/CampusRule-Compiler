#include "token.h"

const char *token_type_to_string(CRTokenType type) {
    switch (type) {
        case TOKEN_IF:            return "IF";
        case TOKEN_THEN:          return "THEN";
        case TOKEN_END:           return "END";
        case TOKEN_ELIGIBLE:      return "ELIGIBLE";
        case TOKEN_NOT:           return "NOT";
        case TOKEN_AND:           return "AND";
        case TOKEN_OR:            return "OR";
        case TOKEN_EXAM:          return "EXAM";
        case TOKEN_PLACEMENT:     return "PLACEMENT";
        case TOKEN_SCHOLARSHIP:   return "SCHOLARSHIP";
        case TOKEN_FOR:           return "FOR";
        case TOKEN_GREATER:       return "GREATER";
        case TOKEN_LESS:          return "LESS";
        case TOKEN_GREATER_EQUAL: return "GREATER_EQUAL";
        case TOKEN_LESS_EQUAL:    return "LESS_EQUAL";
        case TOKEN_EQUAL:         return "EQUAL";
        case TOKEN_NOT_EQUAL:     return "NOT_EQUAL";
        case TOKEN_LPAREN:        return "LPAREN";
        case TOKEN_RPAREN:        return "RPAREN";
        case TOKEN_IDENTIFIER:    return "IDENTIFIER";
        case TOKEN_NUMBER:        return "NUMBER";
        case TOKEN_EOF:           return "EOF";
        default:                  return "UNKNOWN";
    }
}

Token token_create(CRTokenType type, const char *value, double num_value, int line, int col, int pos) {
    Token t;
    t.type = type;
    t.value = cr_strdup(value ? value : "");
    t.num_value = num_value;
    t.line = line;
    t.col = col;
    t.pos = pos;
    return t;
}

char *token_format(const Token *t) {
    if (!t) return cr_strdup("");
    const char *name = token_type_to_string(t->type);
    const char *val = t->value ? t->value : "";
    size_t len = strlen(name) + strlen(val) + 16;
    char *buf = (char *)malloc(len);
    if (!buf) return cr_strdup("");
    snprintf(buf, len, "%s '%s'", name, val);
    return buf;
}

void token_free_members(Token *t) {
    if (t && t->value) {
        free(t->value);
        t->value = NULL;
    }
}

TokenList *token_list_create(void) {
    TokenList *list = (TokenList *)malloc(sizeof(TokenList));
    if (!list) return NULL;
    list->capacity = 32;
    list->count = 0;
    list->items = (Token *)malloc(list->capacity * sizeof(Token));
    return list;
}

void token_list_append(TokenList *list, Token t) {
    if (!list) return;
    if (list->count >= list->capacity) {
        size_t new_cap = list->capacity * 2;
        Token *new_items = (Token *)realloc(list->items, new_cap * sizeof(Token));
        if (!new_items) return;
        list->items = new_items;
        list->capacity = new_cap;
    }
    list->items[list->count++] = t;
}

void token_list_free(TokenList *list) {
    if (!list) return;
    if (list->items) {
        for (size_t i = 0; i < list->count; i++) {
            token_free_members(&list->items[i]);
        }
        free(list->items);
        list->items = NULL;
    }
    free(list);
}
