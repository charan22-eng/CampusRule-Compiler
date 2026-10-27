#include "compiler.h"
#include "gui.h"

static void print_help(const char *prog) {
    printf("CampusRule Compiler — A Domain-Specific Language for College Rule Automation\n");
    printf("Based on Course Based Project Report (IT Department, VNRVJIET)\n\n");
    printf("Usage:\n");
    printf("  %s                      Launch the desktop graphical interface\n", prog);
    printf("  %s --cli                Run all built-in example programs\n", prog);
    printf("  %s --cli <file.cr>      Compile and evaluate a DSL rule file\n", prog);
    printf("  %s --gui                Force graphical interface launch\n", prog);
    printf("  %s --help               Display this help message\n", prog);
    printf("\nStudent Academic Data Options (for --cli <file.cr>):\n");
    printf("  --attendance <float>    Student attendance percentage (0.0 to 100.0, default: 78.0)\n");
    printf("  --cgpa <float>          Student CGPA (0.0 to 10.0, default: 8.2)\n");
    printf("  --marks <float>         Student marks percentage (0.0 to 100.0, default: 82.0)\n");
    printf("  --backlogs <int>        Student backlog count (0 to 50, default: 0)\n");
}

static void run_all_samples_cli(const StudentData *student) {
    for (size_t s = 0; s < compiler_get_samples_count(); s++) {
        const SampleRule *sample = compiler_get_sample(s);
        printf("\n################ %s ################\n", sample->title);

        CompilerOutput *out = compiler_run(sample->code, student);
        if (!out) continue;

        for (int p = 0; p < PHASE_COUNT; p++) {
            const char *pname = compiler_phase_name((CompilerPhase)p);
            const char *sname = compiler_status_name(out->status[p]);
            printf("\n========== %s [%s] ==========\n", pname, sname);
            if (out->status[p] != STATUS_SKIPPED) {
                printf("%s\n", out->text[p] ? out->text[p] : "");
            }
        }

        if (out->error) {
            printf("\n>>> %s\n", out->error);
        } else {
            printf("\n>>> %s\n", out->verdict ? out->verdict : "");
        }

        compiler_free_output(out);
    }
}

static int run_file_cli(const char *filepath, const StudentData *student) {
    FILE *f = fopen(filepath, "rb");
    if (!f) {
        fprintf(stderr, "Error: Could not open file '%s'\n", filepath);
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (fsize < 0) {
        fclose(f);
        fprintf(stderr, "Error: Failed to read file size '%s'\n", filepath);
        return 1;
    }

    char *source = (char *)malloc(fsize + 1);
    if (!source) {
        fclose(f);
        fprintf(stderr, "Error: Out of memory reading '%s'\n", filepath);
        return 1;
    }

    size_t read_bytes = fread(source, 1, fsize, f);
    source[read_bytes] = '\0';
    fclose(f);

    printf("====================================================\n");
    printf("  Compiling: %s\n", filepath);
    printf("====================================================\n");

    CompilerOutput *out = compiler_run(source, student);
    free(source);

    if (!out) {
        fprintf(stderr, "Fatal compiler error\n");
        return 1;
    }

    for (int p = 0; p < PHASE_COUNT; p++) {
        const char *pname = compiler_phase_name((CompilerPhase)p);
        const char *sname = compiler_status_name(out->status[p]);
        printf("\n========== %s [%s] ==========\n", pname, sname);
        if (out->status[p] != STATUS_SKIPPED) {
            printf("%s\n", out->text[p] ? out->text[p] : "");
        }
    }

    int exit_code = 0;
    if (out->error) {
        printf("\n>>> [FAILURE] %s\n", out->error);
        exit_code = 1;
    } else {
        printf("\n>>> [SUCCESS] %s\n", out->verdict ? out->verdict : "");
    }

    compiler_free_output(out);
    return exit_code;
}

int main(int argc, char **argv) {
    bool cli_mode = false;
    bool force_gui = false;
    const char *file_to_run = NULL;

    StudentData student = cr_default_student();

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--cli") == 0) {
            cli_mode = true;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                file_to_run = argv[++i];
            }
        } else if (strcmp(argv[i], "--gui") == 0) {
            force_gui = true;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_help(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
            printf("CampusRule Compiler v1.0.0 (C11 Implementation)\n");
            return 0;
        } else if (strcmp(argv[i], "--attendance") == 0 && i + 1 < argc) {
            student.attendance = atof(argv[++i]);
        } else if (strcmp(argv[i], "--cgpa") == 0 && i + 1 < argc) {
            student.cgpa = atof(argv[++i]);
        } else if (strcmp(argv[i], "--marks") == 0 && i + 1 < argc) {
            student.marks = atof(argv[++i]);
        } else if (strcmp(argv[i], "--backlogs") == 0 && i + 1 < argc) {
            student.backlogs = atof(argv[++i]);
        } else if (argv[i][0] != '-') {
            file_to_run = argv[i];
            cli_mode = true;
        }
    }

    if (force_gui) {
        return gui_run(argc, argv);
    }

    if (cli_mode) {
        if (file_to_run) {
            return run_file_cli(file_to_run, &student);
        } else {
            run_all_samples_cli(&student);
            return 0;
        }
    }

    /* Default behavior: Launch desktop GUI */
    return gui_run(argc, argv);
}
