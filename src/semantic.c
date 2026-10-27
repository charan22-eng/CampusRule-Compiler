#include "semantic.h"

typedef struct {
    const char *name;
    double min_val;
    double max_val;
    bool is_integer;
} VarRule;

static const VarRule ALLOWED_VARS[] = {
    { VAR_ATTENDANCE, RANGE_ATTENDANCE_MIN, RANGE_ATTENDANCE_MAX, false },
    { VAR_CGPA,       RANGE_CGPA_MIN,       RANGE_CGPA_MAX,       false },
    { VAR_MARKS,      RANGE_MARKS_MIN,      RANGE_MARKS_MAX,      false },
    { VAR_BACKLOGS,   RANGE_BACKLOGS_MIN,   RANGE_BACKLOGS_MAX,   true  }
};
static const size_t ALLOWED_VARS_COUNT = sizeof(ALLOWED_VARS) / sizeof(ALLOWED_VARS[0]);

static const VarRule *find_var_rule(const char *name) {
    if (!name) return NULL;
    for (size_t i = 0; i < ALLOWED_VARS_COUNT; i++) {
        if (CR_STRCASECMP(ALLOWED_VARS[i].name, name) == 0) {
            return &ALLOWED_VARS[i];
        }
    }
    return NULL;
}

static void add_error(SemanticErrors *errs, const char *msg) {
    if (!errs) return;
    if (errs->count >= errs->capacity) {
        size_t new_cap = errs->capacity == 0 ? 8 : errs->capacity * 2;
        char **new_items = (char **)realloc(errs->errors, new_cap * sizeof(char *));
        if (!new_items) return;
        errs->errors = new_items;
        errs->capacity = new_cap;
    }
    errs->errors[errs->count++] = cr_strdup(msg);
}

static void check_condition(const ASTNode *cond, int rule_idx, StringBuilder *out, SemanticErrors *errs) {
    if (!cond) return;
    if (cond->type == AST_NODE_LOGIC) {
        check_condition(cond->as.logic.left, rule_idx, out, errs);
        check_condition(cond->as.logic.right, rule_idx, out, errs);
        return;
    }

    if (cond->type == AST_NODE_COMPARE) {
        const char *var = cond->as.compare.variable;
        double val = cond->as.compare.value;
        const char *op_str = relop_to_str(cond->as.compare.op);

        const VarRule *rule = find_var_rule(var);
        if (!rule) {
            char msg[256];
            snprintf(msg, sizeof(msg), "Rule %d: Unknown variable '%s' (allowed: [ATTENDANCE, CGPA, MARKS, BACKLOGS])",
                     rule_idx, var);
            add_error(errs, msg);
            sb_appendf(out, "  [ERROR] Unknown variable '%s' (allowed: [ATTENDANCE, CGPA, MARKS, BACKLOGS])\n", var);
            return;
        }

        if (val < rule->min_val || val > rule->max_val) {
            char msg[256];
            snprintf(msg, sizeof(msg), "Rule %d: %s = %.1f is outside the valid range [%d, %d]",
                     rule_idx, var, val, (int)rule->min_val, (int)rule->max_val);
            add_error(errs, msg);
            sb_appendf(out, "  [ERROR] %s = %.1f is outside the valid range [%d, %d]\n",
                       var, val, (int)rule->min_val, (int)rule->max_val);
            return;
        }

        if (rule->is_integer && floor(val) != val) {
            char msg[256];
            snprintf(msg, sizeof(msg), "Rule %d: %s must be a whole number, found %.1f",
                     rule_idx, var, val);
            add_error(errs, msg);
            sb_appendf(out, "  [ERROR] %s must be a whole number, found %.1f\n", var, val);
            return;
        }

        sb_appendf(out, "  [OK]    %s %s %.1f   (valid range %d-%d)\n",
                   var, op_str, val, (int)rule->min_val, (int)rule->max_val);
    }
}

SemanticResult semantic_analyze(const ASTNode *program) {
    SemanticResult res;
    res.report = NULL;
    res.first_error = NULL;
    res.error_count = 0;
    res.success = false;

    if (!program || program->type != AST_NODE_PROGRAM) {
        res.report = cr_strdup("Semantic Error: Invalid AST");
        res.first_error = cr_strdup("Invalid AST");
        res.error_count = 1;
        return res;
    }

    StringBuilder out;
    sb_init(&out);

    SemanticErrors errs = { NULL, 0, 0 };

    /* Seen actions array to detect duplicate rules */
    char **seen_actions = NULL;
    size_t seen_count = 0;

    for (size_t i = 0; i < program->as.program.count; i++) {
        const ASTNode *rule = program->as.program.rules[i];
        if (!rule || rule->type != AST_NODE_RULE) continue;

        int idx = rule->as.rule.index;
        const char *act = rule->as.rule.action.action_text;

        sb_appendf(&out, "Rule %d: %s\n", idx, act);
        check_condition(rule->as.rule.condition, idx, &out, &errs);

        /* Duplicate rule check */
        bool duplicate = false;
        for (size_t s = 0; s < seen_count; s++) {
            if (strcmp(seen_actions[s], act) == 0) {
                duplicate = true;
                break;
            }
        }

        if (duplicate) {
            char msg[256];
            snprintf(msg, sizeof(msg), "Rule %d: Duplicate rule: '%s' is already defined by an earlier rule",
                     idx, act);
            add_error(&errs, msg);
            sb_appendf(&out, "  [ERROR] Duplicate rule: '%s' is already defined by an earlier rule\n", act);
        } else {
            char **new_seen = (char **)realloc(seen_actions, (seen_count + 1) * sizeof(char *));
            if (new_seen) {
                seen_actions = new_seen;
                seen_actions[seen_count++] = cr_strdup(act);
            }
        }

        sb_append(&out, "\n");
    }

    /* Free seen actions */
    for (size_t s = 0; s < seen_count; s++) {
        free(seen_actions[s]);
    }
    free(seen_actions);

    if (errs.count == 0) {
        sb_append(&out, "Semantic Analysis Successful");
        res.success = true;
        res.error_count = 0;
    } else {
        sb_appendf(&out, "%zu semantic error(s) found", errs.count);
        res.success = false;
        res.error_count = (int)errs.count;
        if (errs.count > 0) {
            res.first_error = cr_strdup(errs.errors[0]);
        }
    }

    res.report = sb_to_string(&out);
    sb_free(&out);

    /* Free collected error strings */
    for (size_t e = 0; e < errs.count; e++) {
        free(errs.errors[e]);
    }
    free(errs.errors);

    return res;
}

void semantic_result_free(SemanticResult *res) {
    if (!res) return;
    if (res->report) {
        free(res->report);
        res->report = NULL;
    }
    if (res->first_error) {
        free(res->first_error);
        res->first_error = NULL;
    }
    res->error_count = 0;
    res->success = false;
}

bool semantic_validate_student(const StudentData *student, char **out_error) {
    if (out_error) *out_error = NULL;

    if (!student) {
        if (out_error) *out_error = cr_strdup("Student data is missing");
        return false;
    }

    char err[256];

    /* ATTENDANCE */
    if (isnan(student->attendance) || isinf(student->attendance) ||
        student->attendance < RANGE_ATTENDANCE_MIN || student->attendance > RANGE_ATTENDANCE_MAX) {
        snprintf(err, sizeof(err), "Student ATTENDANCE (%.1f) must be between %.0f and %.0f",
                 student->attendance, RANGE_ATTENDANCE_MIN, RANGE_ATTENDANCE_MAX);
        if (out_error) *out_error = cr_strdup(err);
        return false;
    }

    /* CGPA */
    if (isnan(student->cgpa) || isinf(student->cgpa) ||
        student->cgpa < RANGE_CGPA_MIN || student->cgpa > RANGE_CGPA_MAX) {
        snprintf(err, sizeof(err), "Student CGPA (%.2f) must be between %.0f and %.0f",
                 student->cgpa, RANGE_CGPA_MIN, RANGE_CGPA_MAX);
        if (out_error) *out_error = cr_strdup(err);
        return false;
    }

    /* MARKS */
    if (isnan(student->marks) || isinf(student->marks) ||
        student->marks < RANGE_MARKS_MIN || student->marks > RANGE_MARKS_MAX) {
        snprintf(err, sizeof(err), "Student MARKS (%.1f) must be between %.0f and %.0f",
                 student->marks, RANGE_MARKS_MIN, RANGE_MARKS_MAX);
        if (out_error) *out_error = cr_strdup(err);
        return false;
    }

    /* BACKLOGS */
    if (isnan(student->backlogs) || isinf(student->backlogs) ||
        student->backlogs < RANGE_BACKLOGS_MIN || student->backlogs > RANGE_BACKLOGS_MAX) {
        snprintf(err, sizeof(err), "Student BACKLOGS (%.1f) must be between %.0f and %.0f",
                 student->backlogs, RANGE_BACKLOGS_MIN, RANGE_BACKLOGS_MAX);
        if (out_error) *out_error = cr_strdup(err);
        return false;
    }

    if (floor(student->backlogs) != student->backlogs) {
        snprintf(err, sizeof(err), "Student BACKLOGS (%.1f) must be an integer", student->backlogs);
        if (out_error) *out_error = cr_strdup(err);
        return false;
    }

    return true;
}
