#include "tac.h"

typedef struct {
    int temp_counter;
    int label_counter;
    StringBuilder sb;
    TACInstruction *instructions;
    size_t count;
    size_t capacity;
} TACBuilder;

static void tac_builder_init(TACBuilder *tb) {
    tb->temp_counter = 1;
    tb->label_counter = 1;
    sb_init(&tb->sb);
    tb->capacity = 32;
    tb->count = 0;
    tb->instructions = (TACInstruction *)malloc(tb->capacity * sizeof(TACInstruction));
    if (!tb->instructions) {
        tb->capacity = 0;
    }
}

static void add_instr(TACBuilder *tb, TACOpType type, const char *dest, const char *arg1,
                      const char *op, const char *arg2, const char *extra) {
    if (tb->count >= tb->capacity) {
        size_t new_cap = (tb->capacity == 0) ? 16 : tb->capacity * 2;
        TACInstruction *new_items = (TACInstruction *)realloc(tb->instructions, new_cap * sizeof(TACInstruction));
        if (!new_items) return;
        tb->instructions = new_items;
        tb->capacity = new_cap;
    }
    if (!tb->instructions) return;
    TACInstruction *ins = &tb->instructions[tb->count++];
    ins->type = type;
    ins->dest = cr_strdup(dest);
    ins->arg1 = cr_strdup(arg1);
    ins->op = cr_strdup(op);
    ins->arg2 = cr_strdup(arg2);
    ins->extra = cr_strdup(extra);
}

static char *gen_condition(const ASTNode *cond, TACBuilder *tb) {
    if (!cond) return cr_strdup("t0");

    if (cond->type == AST_NODE_COMPARE) {
        char temp_name[32];
        snprintf(temp_name, sizeof(temp_name), "t%d", tb->temp_counter++);

        char val_str[64];
        snprintf(val_str, sizeof(val_str), "%.1f", cond->as.compare.value);

        const char *var = cond->as.compare.variable;
        const char *op_str = relop_to_str(cond->as.compare.op);

        sb_appendf(&tb->sb, "    %s = %s %s %.1f\n", temp_name, var, op_str, cond->as.compare.value);
        add_instr(tb, TAC_ASSIGN_COMPARE, temp_name, var, op_str, val_str, NULL);

        return cr_strdup(temp_name);
    }

    if (cond->type == AST_NODE_LOGIC) {
        char *a = gen_condition(cond->as.logic.left, tb);
        char *b = gen_condition(cond->as.logic.right, tb);

        char temp_name[32];
        snprintf(temp_name, sizeof(temp_name), "t%d", tb->temp_counter++);

        const char *lop = logicop_to_str(cond->as.logic.op);

        sb_appendf(&tb->sb, "    %s = %s %s %s\n", temp_name, a, lop, b);
        add_instr(tb, TAC_ASSIGN_LOGIC, temp_name, a, lop, b, NULL);

        free(a);
        free(b);
        return cr_strdup(temp_name);
    }

    return cr_strdup("t0");
}

TACProgram *tac_generate_program(const ASTNode *program) {
    if (!program || program->type != AST_NODE_PROGRAM) return NULL;

    TACBuilder tb;
    tac_builder_init(&tb);

    for (size_t i = 0; i < program->as.program.count; i++) {
        const ASTNode *rule = program->as.program.rules[i];
        if (!rule || rule->type != AST_NODE_RULE) continue;

        sb_appendf(&tb.sb, "Rule %d:\n", rule->as.rule.index);

        char *result_temp = gen_condition(rule->as.rule.condition, &tb);

        char skip_label[32];
        snprintf(skip_label, sizeof(skip_label), "L%d", tb.label_counter++);

        sb_appendf(&tb.sb, "    IF_FALSE %s GOTO %s\n", result_temp, skip_label);
        add_instr(&tb, TAC_IF_FALSE_GOTO, skip_label, result_temp, NULL, NULL, NULL);

        sb_appendf(&tb.sb, "    PRINT \"%s\"\n", rule->as.rule.action.action_text);
        add_instr(&tb, TAC_PRINT, NULL, NULL, NULL, NULL, rule->as.rule.action.action_text);

        sb_appendf(&tb.sb, "%s:\n\n", skip_label);
        add_instr(&tb, TAC_LABEL, skip_label, NULL, NULL, NULL, NULL);

        free(result_temp);
    }

    TACProgram *tac = (TACProgram *)malloc(sizeof(TACProgram));
    if (!tac) {
        sb_free(&tb.sb);
        return NULL;
    }
    tac->instructions = tb.instructions;
    tac->count = tb.count;
    tac->capacity = tb.capacity;
    tac->text_output = sb_to_string(&tb.sb);
    sb_free(&tb.sb);

    return tac;
}

char *tac_generate_string(const ASTNode *program) {
    TACProgram *tac = tac_generate_program(program);
    if (!tac) return cr_strdup("");
    char *res = cr_strdup(tac->text_output);
    tac_program_free(tac);
    return res;
}

void tac_program_free(TACProgram *tac) {
    if (!tac) return;
    if (tac->instructions) {
        for (size_t i = 0; i < tac->count; i++) {
            free(tac->instructions[i].dest);
            free(tac->instructions[i].arg1);
            free(tac->instructions[i].op);
            free(tac->instructions[i].arg2);
            free(tac->instructions[i].extra);
        }
        free(tac->instructions);
        tac->instructions = NULL;
    }
    if (tac->text_output) {
        free(tac->text_output);
        tac->text_output = NULL;
    }
    free(tac);
}
