#include "evaluator.h"

const char *decision_to_string(CategoryDecision d) {
    switch (d) {
        case DECISION_ELIGIBLE:     return "ELIGIBLE";
        case DECISION_NOT_ELIGIBLE: return "NOT ELIGIBLE";
        case DECISION_CONFLICT:     return "CONFLICT";
        case DECISION_NO_DECISION:  return "NO DECISION";
        default:                    return "UNKNOWN";
    }
}

static double get_student_var(const StudentData *student, const char *var_name) {
    if (!student || !var_name) return 0.0;
    if (CR_STRCASECMP(var_name, VAR_ATTENDANCE) == 0) return student->attendance;
    if (CR_STRCASECMP(var_name, VAR_CGPA) == 0)       return student->cgpa;
    if (CR_STRCASECMP(var_name, VAR_MARKS) == 0)      return student->marks;
    if (CR_STRCASECMP(var_name, VAR_BACKLOGS) == 0)   return student->backlogs;
    return 0.0;
}

static bool eval_condition(const ASTNode *cond, const StudentData *student) {
    if (!cond) return false;

    if (cond->type == AST_NODE_LOGIC) {
        bool a = eval_condition(cond->as.logic.left, student);
        bool b = eval_condition(cond->as.logic.right, student);
        return (cond->as.logic.op == LOGIC_AND) ? (a && b) : (a || b);
    }

    if (cond->type == AST_NODE_COMPARE) {
        double actual = get_student_var(student, cond->as.compare.variable);
        double threshold = cond->as.compare.value;
        const double eps = 1e-9;

        switch (cond->as.compare.op) {
            case OP_GT: return actual > threshold + eps;
            case OP_LT: return actual < threshold - eps;
            case OP_GE: return actual >= threshold - eps;
            case OP_LE: return actual <= threshold + eps;
            case OP_EQ: return fabs(actual - threshold) <= eps;
            case OP_NE: return fabs(actual - threshold) > eps;
            default:    return false;
        }
    }

    return false;
}

EvaluationResult evaluator_evaluate(const ASTNode *program, const StudentData *student) {
    EvaluationResult res;
    res.exam_decision = DECISION_NO_DECISION;
    res.placement_decision = DECISION_NO_DECISION;
    res.scholarship_decision = DECISION_NO_DECISION;
    res.fired_actions = NULL;
    res.fired_count = 0;
    res.trace = NULL;
    res.verdict = NULL;
    res.eligible = false;

    if (!program || program->type != AST_NODE_PROGRAM || !student) {
        res.verdict = cr_strdup("NOT ELIGIBLE");
        res.trace = cr_strdup("Error: Invalid program or student data");
        return res;
    }

    StringBuilder tr;
    sb_init(&tr);
    sb_appendf(&tr, "Student data: {ATTENDANCE=%.1f, CGPA=%.1f, MARKS=%.1f, BACKLOGS=%.1f}\n\n",
               student->attendance, student->cgpa, student->marks, student->backlogs);

    size_t fired_cap = 8;
    res.fired_actions = (char **)malloc(fired_cap * sizeof(char *));

    /* Track decisions per category */
    int exam_pos = 0, exam_neg = 0;
    int place_pos = 0, place_neg = 0;
    int schol_pos = 0, schol_neg = 0;

    for (size_t i = 0; i < program->as.program.count; i++) {
        const ASTNode *rule = program->as.program.rules[i];
        if (!rule || rule->type != AST_NODE_RULE) continue;

        bool result = eval_condition(rule->as.rule.condition, student);
        char *cond_txt = ast_condition_to_string(rule->as.rule.condition);

        sb_appendf(&tr, "Rule %d: %s -> %s\n",
                   rule->as.rule.index,
                   cond_txt ? cond_txt : "",
                   result ? "TRUE" : "FALSE");
        free(cond_txt);

        if (result) {
            const char *act = rule->as.rule.action.action_text;
            sb_appendf(&tr, "        => %s\n", act);

            if (res.fired_count >= fired_cap) {
                fired_cap *= 2;
                res.fired_actions = (char **)realloc(res.fired_actions, fired_cap * sizeof(char *));
            }
            res.fired_actions[res.fired_count++] = cr_strdup(act);

            /* Track positive vs negative actions per category */
            bool is_not = rule->as.rule.action.is_not;
            switch (rule->as.rule.action.category) {
                case CATEGORY_EXAM:
                    if (is_not) exam_neg++; else exam_pos++;
                    break;
                case CATEGORY_PLACEMENT:
                    if (is_not) place_neg++; else place_pos++;
                    break;
                case CATEGORY_SCHOLARSHIP:
                    if (is_not) schol_neg++; else schol_pos++;
                    break;
                default:
                    break;
            }
        }
    }

    /* Apply deterministic eligibility policy per category */
    if (exam_pos > 0 && exam_neg > 0)       res.exam_decision = DECISION_CONFLICT;
    else if (exam_pos > 0)                  res.exam_decision = DECISION_ELIGIBLE;
    else if (exam_neg > 0)                  res.exam_decision = DECISION_NOT_ELIGIBLE;
    else                                    res.exam_decision = DECISION_NO_DECISION;

    if (place_pos > 0 && place_neg > 0)     res.placement_decision = DECISION_CONFLICT;
    else if (place_pos > 0)                 res.placement_decision = DECISION_ELIGIBLE;
    else if (place_neg > 0)                 res.placement_decision = DECISION_NOT_ELIGIBLE;
    else                                    res.placement_decision = DECISION_NO_DECISION;

    if (schol_pos > 0 && schol_neg > 0)     res.scholarship_decision = DECISION_CONFLICT;
    else if (schol_pos > 0)                 res.scholarship_decision = DECISION_ELIGIBLE;
    else if (schol_neg > 0)                 res.scholarship_decision = DECISION_NOT_ELIGIBLE;
    else                                    res.scholarship_decision = DECISION_NO_DECISION;

    /* Build verdict string */
    if (res.fired_count == 0) {
        res.verdict = cr_strdup("NOT ELIGIBLE");
    } else {
        StringBuilder vb;
        sb_init(&vb);
        for (size_t f = 0; f < res.fired_count; f++) {
            if (f > 0) sb_append(&vb, " | ");
            sb_append(&vb, res.fired_actions[f]);
        }
        res.verdict = sb_to_string(&vb);
        sb_free(&vb);
    }

    /* Overall eligible flag: true if any category is ELIGIBLE and none in CONFLICT */
    res.eligible = (res.exam_decision == DECISION_ELIGIBLE ||
                    res.placement_decision == DECISION_ELIGIBLE ||
                    res.scholarship_decision == DECISION_ELIGIBLE) &&
                   (res.exam_decision != DECISION_CONFLICT &&
                    res.placement_decision != DECISION_CONFLICT &&
                    res.scholarship_decision != DECISION_CONFLICT);

    sb_appendf(&tr, "\nFINAL RESULT: %s\n", res.verdict);
    sb_appendf(&tr, "\nCATEGORY BREAKDOWN:\n");
    sb_appendf(&tr, "  - EXAM:        %s\n", decision_to_string(res.exam_decision));
    sb_appendf(&tr, "  - PLACEMENT:   %s\n", decision_to_string(res.placement_decision));
    sb_appendf(&tr, "  - SCHOLARSHIP: %s\n", decision_to_string(res.scholarship_decision));

    res.trace = sb_to_string(&tr);
    sb_free(&tr);

    return res;
}

void evaluation_result_free(EvaluationResult *res) {
    if (!res) return;
    if (res->fired_actions) {
        for (size_t i = 0; i < res->fired_count; i++) {
            free(res->fired_actions[i]);
        }
        free(res->fired_actions);
        res->fired_actions = NULL;
    }
    if (res->trace) {
        free(res->trace);
        res->trace = NULL;
    }
    if (res->verdict) {
        free(res->verdict);
        res->verdict = NULL;
    }
    res->fired_count = 0;
}
