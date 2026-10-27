#include "ast.h"

const char *relop_to_str(RelOp op) {
    switch (op) {
        case OP_GT: return ">";
        case OP_LT: return "<";
        case OP_GE: return ">=";
        case OP_LE: return "<=";
        case OP_EQ: return "==";
        case OP_NE: return "!=";
        default:    return "??";
    }
}

RelOp relop_from_str(const char *str) {
    if (!str) return OP_EQ;
    if (strcmp(str, ">") == 0) return OP_GT;
    if (strcmp(str, "<") == 0) return OP_LT;
    if (strcmp(str, ">=") == 0) return OP_GE;
    if (strcmp(str, "<=") == 0) return OP_LE;
    if (strcmp(str, "==") == 0) return OP_EQ;
    if (strcmp(str, "!=") == 0) return OP_NE;
    return OP_EQ;
}

const char *logicop_to_str(LogicOp op) {
    switch (op) {
        case LOGIC_AND: return "AND";
        case LOGIC_OR:  return "OR";
        default:        return "??";
    }
}

const char *category_to_str(ActionCategory cat) {
    switch (cat) {
        case CATEGORY_EXAM:        return "EXAM";
        case CATEGORY_PLACEMENT:   return "PLACEMENT";
        case CATEGORY_SCHOLARSHIP: return "SCHOLARSHIP";
        default:                   return "UNKNOWN";
    }
}

ASTNode *ast_create_compare(const char *var, RelOp op, double val, int line, int col) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_NODE_COMPARE;
    node->line = line;
    node->col = col;
    node->as.compare.variable = cr_strdup(var ? var : "");
    node->as.compare.op = op;
    node->as.compare.value = val;
    return node;
}

ASTNode *ast_create_logic(LogicOp op, ASTNode *left, ASTNode *right, int line, int col) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_NODE_LOGIC;
    node->line = line;
    node->col = col;
    node->as.logic.op = op;
    node->as.logic.left = left;
    node->as.logic.right = right;
    return node;
}

ASTNode *ast_create_rule(int index, ASTNode *condition, bool is_not, ActionCategory cat, const char *action_text, int line, int col) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_NODE_RULE;
    node->line = line;
    node->col = col;
    node->as.rule.index = index;
    node->as.rule.condition = condition;
    node->as.rule.action.is_not = is_not;
    node->as.rule.action.category = cat;
    node->as.rule.action.action_text = cr_strdup(action_text ? action_text : "");
    return node;
}

ASTNode *ast_create_program(void) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_NODE_PROGRAM;
    node->line = 1;
    node->col = 1;
    node->as.program.capacity = 8;
    node->as.program.count = 0;
    node->as.program.rules = (ASTNode **)malloc(node->as.program.capacity * sizeof(ASTNode *));
    return node;
}

void ast_program_add_rule(ASTNode *program, ASTNode *rule) {
    if (!program || program->type != AST_NODE_PROGRAM || !rule) return;
    if (program->as.program.count >= program->as.program.capacity) {
        size_t new_cap = program->as.program.capacity * 2;
        ASTNode **new_rules = (ASTNode **)realloc(program->as.program.rules, new_cap * sizeof(ASTNode *));
        if (!new_rules) return;
        program->as.program.rules = new_rules;
        program->as.program.capacity = new_cap;
    }
    program->as.program.rules[program->as.program.count++] = rule;
}

void ast_free(ASTNode *node) {
    if (!node) return;
    switch (node->type) {
        case AST_NODE_COMPARE:
            if (node->as.compare.variable) {
                free(node->as.compare.variable);
                node->as.compare.variable = NULL;
            }
            break;
        case AST_NODE_LOGIC:
            ast_free(node->as.logic.left);
            ast_free(node->as.logic.right);
            break;
        case AST_NODE_RULE:
            ast_free(node->as.rule.condition);
            if (node->as.rule.action.action_text) {
                free(node->as.rule.action.action_text);
                node->as.rule.action.action_text = NULL;
            }
            break;
        case AST_NODE_PROGRAM:
            if (node->as.program.rules) {
                for (size_t i = 0; i < node->as.program.count; i++) {
                    ast_free(node->as.program.rules[i]);
                }
                free(node->as.program.rules);
                node->as.program.rules = NULL;
            }
            break;
    }
    free(node);
}

static void condition_to_sb(const ASTNode *cond, StringBuilder *sb) {
    if (!cond) return;
    if (cond->type == AST_NODE_COMPARE) {
        sb_appendf(sb, "%s %s %.1f",
                   cond->as.compare.variable,
                   relop_to_str(cond->as.compare.op),
                   cond->as.compare.value);
    } else if (cond->type == AST_NODE_LOGIC) {
        sb_append(sb, "(");
        condition_to_sb(cond->as.logic.left, sb);
        sb_appendf(sb, " %s ", logicop_to_str(cond->as.logic.op));
        condition_to_sb(cond->as.logic.right, sb);
        sb_append(sb, ")");
    }
}

char *ast_condition_to_string(const ASTNode *cond) {
    StringBuilder sb;
    sb_init(&sb);
    condition_to_sb(cond, &sb);
    char *res = sb_to_string(&sb);
    sb_free(&sb);
    return res;
}

static void tree_to_sb(const ASTNode *node, const char *prefix, StringBuilder *sb) {
    if (!node) return;
    switch (node->type) {
        case AST_NODE_COMPARE:
            sb_appendf(sb, "%s%s %s %.1f\n",
                       prefix,
                       node->as.compare.variable,
                       relop_to_str(node->as.compare.op),
                       node->as.compare.value);
            break;
        case AST_NODE_LOGIC: {
            sb_appendf(sb, "%s%s\n", prefix, logicop_to_str(node->as.logic.op));
            char child_prefix[256];
            snprintf(child_prefix, sizeof(child_prefix), "%s  ", prefix);
            tree_to_sb(node->as.logic.left, child_prefix, sb);
            tree_to_sb(node->as.logic.right, child_prefix, sb);
            break;
        }
        case AST_NODE_RULE: {
            sb_appendf(sb, "%sRule %d (IF)\n", prefix, node->as.rule.index);
            sb_appendf(sb, "%s  Condition\n", prefix);
            char cond_prefix[256];
            snprintf(cond_prefix, sizeof(cond_prefix), "%s    ", prefix);
            tree_to_sb(node->as.rule.condition, cond_prefix, sb);
            sb_appendf(sb, "%s  Action: %s\n", prefix, node->as.rule.action.action_text);
            break;
        }
        case AST_NODE_PROGRAM: {
            sb_appendf(sb, "Program (%zu rule(s))\n", node->as.program.count);
            for (size_t i = 0; i < node->as.program.count; i++) {
                tree_to_sb(node->as.program.rules[i], "  ", sb);
            }
            break;
        }
    }
}

char *ast_tree_to_string(const ASTNode *node) {
    StringBuilder sb;
    sb_init(&sb);
    tree_to_sb(node, "", &sb);
    char *res = sb_to_string(&sb);
    sb_free(&sb);
    return res;
}
