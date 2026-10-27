#include "compiler.h"

static const char *PHASE_NAMES[PHASE_COUNT] = {
    "Input",
    "Lexical",
    "Parsing",
    "AST",
    "Semantic",
    "TAC",
    "Result"
};

static const SampleRule SAMPLES[] = {
    {
        "1. Simple exam rule",
        "IF ATTENDANCE >= 75 THEN\nELIGIBLE EXAM\nEND"
    },
    {
        "2. Compound AND",
        "IF ATTENDANCE >= 75 AND BACKLOGS == 0 THEN\nELIGIBLE EXAM\nEND"
    },
    {
        "3. Multiple rules (Exam + Placement + Scholarship)",
        "IF ATTENDANCE >= 75 AND BACKLOGS == 0 THEN\nELIGIBLE EXAM\nEND\n\n"
        "IF CGPA >= 7 AND MARKS >= 60 AND BACKLOGS == 0 THEN\nELIGIBLE PLACEMENT\nEND\n\n"
        "IF CGPA >= 8.5 AND MARKS >= 80 THEN\nELIGIBLE SCHOLARSHIP\nEND\n\n"
        "IF ATTENDANCE < 75 OR BACKLOGS > 0 THEN\nNOT ELIGIBLE EXAM\nEND"
    },
    {
        "4. Semantic error (ATTENDANCE = 120)",
        "IF ATTENDANCE >= 120 AND CGPA > 12 THEN\nELIGIBLE EXAM\nEND"
    },
    {
        "5. Syntax error (missing THEN)",
        "IF ATTENDANCE >= 75\nELIGIBLE EXAM\nEND"
    },
    {
        "6. Lexical error (invalid character)",
        "IF ATTENDANCE >= 75 & BACKLOGS == 0 THEN\nELIGIBLE EXAM\nEND"
    },
    {
        "7. Negative rule (NOT ELIGIBLE)",
        "IF ATTENDANCE < 75 OR BACKLOGS > 0 THEN\nNOT ELIGIBLE EXAM\nEND"
    },
    {
        "8. Parentheses expression",
        "IF (ATTENDANCE >= 75 OR CGPA >= 8.0) AND BACKLOGS == 0 THEN\nELIGIBLE EXAM\nEND"
    },
    {
        "9. Conflicting rules (Positive & Negative)",
        "IF ATTENDANCE >= 75 THEN\nELIGIBLE EXAM\nEND\n\n"
        "IF BACKLOGS > 0 THEN\nNOT ELIGIBLE EXAM\nEND"
    }
};

static const size_t SAMPLES_COUNT = sizeof(SAMPLES) / sizeof(SAMPLES[0]);

const char *compiler_phase_name(CompilerPhase phase) {
    if (phase >= 0 && phase < PHASE_COUNT) {
        return PHASE_NAMES[phase];
    }
    return "Unknown";
}

const char *compiler_status_name(PhaseStatus status) {
    switch (status) {
        case STATUS_OK:      return "OK";
        case STATUS_FAILED:  return "FAILED";
        case STATUS_SKIPPED: return "SKIPPED";
        default:             return "UNKNOWN";
    }
}

size_t compiler_get_samples_count(void) {
    return SAMPLES_COUNT;
}

const SampleRule *compiler_get_sample(size_t index) {
    if (index < SAMPLES_COUNT) {
        return &SAMPLES[index];
    }
    return NULL;
}

static CompilerOutput *compiler_output_create(void) {
    CompilerOutput *out = (CompilerOutput *)malloc(sizeof(CompilerOutput));
    if (!out) return NULL;

    for (int p = 0; p < PHASE_COUNT; p++) {
        out->text[p] = cr_strdup("(skipped: an earlier phase failed)");
        out->status[p] = STATUS_SKIPPED;
    }
    out->error = NULL;
    out->verdict = cr_strdup("");
    out->eligible = false;
    out->exam_decision = DECISION_NO_DECISION;
    out->placement_decision = DECISION_NO_DECISION;
    out->scholarship_decision = DECISION_NO_DECISION;
    return out;
}

static void set_phase_ok(CompilerOutput *out, CompilerPhase phase, const char *body) {
    if (out->text[phase]) free(out->text[phase]);
    out->text[phase] = cr_strdup(body ? body : "");
    out->status[phase] = STATUS_OK;
}

static void set_phase_fail(CompilerOutput *out, CompilerPhase phase, const char *body, const char *err) {
    if (out->text[phase]) free(out->text[phase]);
    out->text[phase] = cr_strdup(body ? body : "");
    out->status[phase] = STATUS_FAILED;
    if (out->error) free(out->error);
    out->error = cr_strdup(err ? err : "");
}

CompilerOutput *compiler_run(const char *source, const StudentData *student) {
    CompilerOutput *out = compiler_output_create();
    if (!out) return NULL;

    set_phase_ok(out, PHASE_INPUT, source);

    /* 1. Lexical Analysis */
    char *lex_error = NULL;
    int lex_line = 0, lex_col = 0;
    TokenList *tokens = lexer_tokenize(source, &lex_error, &lex_line, &lex_col);

    if (!tokens) {
        set_phase_fail(out, PHASE_LEXICAL, lex_error, lex_error);
        free(lex_error);
        return out;
    }

    StringBuilder lex_sb;
    sb_init(&lex_sb);
    for (size_t i = 0; i < tokens->count; i++) {
        char *tok_str = token_format(&tokens->items[i]);
        sb_appendf(&lex_sb, "%2zu. %s\n", i + 1, tok_str);
        free(tok_str);
    }
    char *lex_text = sb_to_string(&lex_sb);
    sb_free(&lex_sb);
    set_phase_ok(out, PHASE_LEXICAL, lex_text);
    free(lex_text);

    /* 2. Syntax Analysis / Parsing */
    char *parse_error = NULL;
    int parse_line = 0, parse_col = 0;
    ASTNode *ast = parser_parse(tokens, &parse_error, &parse_line, &parse_col);

    if (!ast) {
        set_phase_fail(out, PHASE_PARSING, parse_error, parse_error);
        free(parse_error);
        token_list_free(tokens);
        return out;
    }

    char parse_success[128];
    snprintf(parse_success, sizeof(parse_success), "Parsing Successful: %zu rule(s) recognised",
             ast->as.program.count);
    set_phase_ok(out, PHASE_PARSING, parse_success);

    /* 3. AST Construction & Inspection */
    char *ast_text = ast_tree_to_string(ast);
    set_phase_ok(out, PHASE_AST, ast_text);
    free(ast_text);

    /* 4. Semantic Analysis */
    SemanticResult sem_res = semantic_analyze(ast);

    /* Also check student data validity */
    char *student_err = NULL;
    bool student_valid = semantic_validate_student(student, &student_err);

    if (!sem_res.success || !student_valid) {
        StringBuilder sem_err_sb;
        sb_init(&sem_err_sb);
        sb_append(&sem_err_sb, sem_res.report ? sem_res.report : "");

        if (!student_valid) {
            sb_appendf(&sem_err_sb, "\n[ERROR] %s\n", student_err);
        }

        char *combined_report = sb_to_string(&sem_err_sb);
        sb_free(&sem_err_sb);

        char banner_err[256];
        if (!sem_res.success) {
            int more = sem_res.error_count - 1;
            if (more > 0) {
                snprintf(banner_err, sizeof(banner_err), "Semantic Error: %s (+%d more)",
                         sem_res.first_error ? sem_res.first_error : "Unknown", more);
            } else {
                snprintf(banner_err, sizeof(banner_err), "Semantic Error: %s",
                         sem_res.first_error ? sem_res.first_error : "Unknown");
            }
        } else {
            snprintf(banner_err, sizeof(banner_err), "Student Data Error: %s", student_err);
        }

        set_phase_fail(out, PHASE_SEMANTIC, combined_report, banner_err);
        free(combined_report);
        if (student_err) free(student_err);
        semantic_result_free(&sem_res);
        ast_free(ast);
        token_list_free(tokens);
        return out;
    }

    if (student_err) free(student_err);
    set_phase_ok(out, PHASE_SEMANTIC, sem_res.report);
    semantic_result_free(&sem_res);

    /* 5. Three-Address Code Generation */
    char *tac_text = tac_generate_string(ast);
    set_phase_ok(out, PHASE_TAC, tac_text);
    free(tac_text);

    /* 6. Rule Evaluation */
    EvaluationResult eval_res = evaluator_evaluate(ast, student);
    set_phase_ok(out, PHASE_RESULT, eval_res.trace);

    if (out->verdict) free(out->verdict);
    out->verdict = cr_strdup(eval_res.verdict);
    out->eligible = eval_res.eligible;
    out->exam_decision = eval_res.exam_decision;
    out->placement_decision = eval_res.placement_decision;
    out->scholarship_decision = eval_res.scholarship_decision;

    evaluation_result_free(&eval_res);
    ast_free(ast);
    token_list_free(tokens);

    return out;
}

void compiler_free_output(CompilerOutput *out) {
    if (!out) return;
    for (int p = 0; p < PHASE_COUNT; p++) {
        if (out->text[p]) {
            free(out->text[p]);
            out->text[p] = NULL;
        }
    }
    if (out->error) {
        free(out->error);
        out->error = NULL;
    }
    if (out->verdict) {
        free(out->verdict);
        out->verdict = NULL;
    }
    free(out);
}
