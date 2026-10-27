#include "gui.h"

#ifdef USE_GTK
#include <gtk/gtk.h>

typedef struct {
    GtkWidget *window;
    GtkWidget *sample_combo;
    GtkWidget *editor_text_view;
    GtkTextBuffer *editor_buffer;

    GtkWidget *entry_attendance;
    GtkWidget *entry_cgpa;
    GtkWidget *entry_marks;
    GtkWidget *entry_backlogs;

    GtkWidget *btn_run;
    GtkWidget *btn_reset;

    GtkWidget *pipeline_chips[PHASE_COUNT];
    GtkWidget *notebook;
    GtkWidget *phase_text_views[PHASE_COUNT];
    GtkTextBuffer *phase_buffers[PHASE_COUNT];

    GtkWidget *verdict_label;
    GtkWidget *verdict_box;
} GtkAppState;

static GtkAppState g_app;

static void apply_css(GtkWidget *widget, const char *css) {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    GtkStyleContext *context = gtk_widget_get_style_context(widget);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(provider);
}

static void update_chip(CompilerPhase phase, PhaseStatus status) {
    const char *color_css = "background-color: #9E9E9E; color: white; font-weight: bold; padding: 4px 10px; border-radius: 4px;";
    if (status == STATUS_OK) {
        color_css = "background-color: #2E7D32; color: white; font-weight: bold; padding: 4px 10px; border-radius: 4px;";
    } else if (status == STATUS_FAILED) {
        color_css = "background-color: #C62828; color: white; font-weight: bold; padding: 4px 10px; border-radius: 4px;";
    }
    apply_css(g_app.pipeline_chips[phase], color_css);
}

static void show_verdict(const char *text, const char *bg_color) {
    gtk_label_set_text(GTK_LABEL(g_app.verdict_label), text);
    char css[256];
    snprintf(css, sizeof(css), "background-color: %s; color: white; font-size: 16px; font-weight: bold; padding: 12px;", bg_color);
    apply_css(g_app.verdict_box, css);
}

static void on_run_clicked(GtkWidget *widget, gpointer data) {
    (void)widget;
    (void)data;

    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(g_app.editor_buffer, &start);
    gtk_text_buffer_get_end_iter(g_app.editor_buffer, &end);
    char *source = gtk_text_buffer_get_text(g_app.editor_buffer, &start, &end, FALSE);

    const char *att_str = gtk_entry_get_text(GTK_ENTRY(g_app.entry_attendance));
    const char *cgpa_str = gtk_entry_get_text(GTK_ENTRY(g_app.entry_cgpa));
    const char *marks_str = gtk_entry_get_text(GTK_ENTRY(g_app.entry_marks));
    const char *back_str = gtk_entry_get_text(GTK_ENTRY(g_app.entry_backlogs));

    char *endptr = NULL;
    StudentData student;
    student.attendance = strtod(att_str, &endptr);
    if (!endptr || *endptr != '\0') {
        show_verdict("Student data fields must contain valid numbers", "#C62828");
        g_free(source);
        return;
    }
    student.cgpa = strtod(cgpa_str, &endptr);
    if (!endptr || *endptr != '\0') {
        show_verdict("Student data fields must contain valid numbers", "#C62828");
        g_free(source);
        return;
    }
    student.marks = strtod(marks_str, &endptr);
    if (!endptr || *endptr != '\0') {
        show_verdict("Student data fields must contain valid numbers", "#C62828");
        g_free(source);
        return;
    }
    student.backlogs = strtod(back_str, &endptr);
    if (!endptr || *endptr != '\0') {
        show_verdict("Student data fields must contain valid numbers", "#C62828");
        g_free(source);
        return;
    }
    student.has_attendance = true;
    student.has_cgpa = true;
    student.has_marks = true;
    student.has_backlogs = true;

    CompilerOutput *out = compiler_run(source, &student);
    g_free(source);
    if (!out) return;

    int select_tab = PHASE_COUNT - 1;
    bool failure_found = false;

    for (int p = 0; p < PHASE_COUNT; p++) {
        gtk_text_buffer_set_text(g_app.phase_buffers[p], out->text[p] ? out->text[p] : "", -1);
        update_chip((CompilerPhase)p, out->status[p]);

        char tab_title[64];
        if (out->status[p] == STATUS_FAILED) {
            snprintf(tab_title, sizeof(tab_title), "%s (error)", compiler_phase_name((CompilerPhase)p));
            if (!failure_found) {
                select_tab = p;
                failure_found = true;
            }
        } else {
            snprintf(tab_title, sizeof(tab_title), "%s", compiler_phase_name((CompilerPhase)p));
        }
        GtkWidget *page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(g_app.notebook), p);
        gtk_notebook_set_tab_label_text(GTK_NOTEBOOK(g_app.notebook), page, tab_title);
    }

    gtk_notebook_set_current_page(GTK_NOTEBOOK(g_app.notebook), select_tab);

    if (out->error) {
        show_verdict(out->error, "#C62828");
    } else {
        show_verdict(out->verdict, out->eligible ? "#2E7D32" : "#EF6C00");
    }

    compiler_free_output(out);
}

static void on_sample_changed(GtkComboBox *combo, gpointer data) {
    (void)data;
    gint idx = gtk_combo_box_get_active(combo);
    const SampleRule *s = compiler_get_sample((size_t)idx);
    if (s) {
        gtk_text_buffer_set_text(g_app.editor_buffer, s->code, -1);
        on_run_clicked(NULL, NULL);
    }
}

static void on_reset_clicked(GtkWidget *widget, gpointer data) {
    (void)widget;
    (void)data;
    StudentData def = cr_default_student();
    char buf[32];
    snprintf(buf, sizeof(buf), "%.1f", def.attendance);
    gtk_entry_set_text(GTK_ENTRY(g_app.entry_attendance), buf);
    snprintf(buf, sizeof(buf), "%.1f", def.cgpa);
    gtk_entry_set_text(GTK_ENTRY(g_app.entry_cgpa), buf);
    snprintf(buf, sizeof(buf), "%.1f", def.marks);
    gtk_entry_set_text(GTK_ENTRY(g_app.entry_marks), buf);
    snprintf(buf, sizeof(buf), "%.1f", def.backlogs);
    gtk_entry_set_text(GTK_ENTRY(g_app.entry_backlogs), buf);

    gtk_combo_box_set_active(GTK_COMBO_BOX(g_app.sample_combo), 0);
}

int gui_gtk_main(int argc, char **argv) {
    gtk_init(&argc, &argv);

    g_app.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(g_app.window), "CampusRule Compiler — Student Eligibility");
    gtk_window_set_default_size(GTK_WINDOW(g_app.window), 1180, 720);
    g_signal_connect(g_app.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *root_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(root_vbox), 10);
    gtk_container_add(GTK_CONTAINER(g_app.window), root_vbox);

    /* --- Top: Pipeline chips --- */
    GtkWidget *pipeline_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget *pipe_lbl = gtk_label_new("Pipeline:");
    gtk_box_pack_start(GTK_BOX(pipeline_box), pipe_lbl, FALSE, FALSE, 0);

    for (int p = 0; p < PHASE_COUNT; p++) {
        g_app.pipeline_chips[p] = gtk_label_new(compiler_phase_name((CompilerPhase)p));
        update_chip((CompilerPhase)p, STATUS_SKIPPED);
        gtk_box_pack_start(GTK_BOX(pipeline_box), g_app.pipeline_chips[p], FALSE, FALSE, 0);
    }
    gtk_box_pack_start(GTK_BOX(root_vbox), pipeline_box, FALSE, FALSE, 0);

    /* --- Center: Paned left & center --- */
    GtkWidget *hpaned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(root_vbox), hpaned, TRUE, TRUE, 0);

    /* Left panel: editor & student fields */
    GtkWidget *left_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_size_request(left_box, 380, -1);
    gtk_paned_pack1(GTK_PANED(hpaned), left_box, FALSE, FALSE);

    GtkWidget *lbl_rule = gtk_label_new("Rule input (what the user types):");
    gtk_box_pack_start(GTK_BOX(left_box), lbl_rule, FALSE, FALSE, 0);

    /* Sample selector */
    g_app.sample_combo = gtk_combo_box_text_new();
    for (size_t s = 0; s < compiler_get_samples_count(); s++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(g_app.sample_combo), compiler_get_sample(s)->title);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(g_app.sample_combo), 0);
    g_signal_connect(g_app.sample_combo, "changed", G_CALLBACK(on_sample_changed), NULL);
    gtk_box_pack_start(GTK_BOX(left_box), g_app.sample_combo, FALSE, FALSE, 0);

    /* Rule editor */
    GtkWidget *editor_scroll = gtk_scrolled_window_new(NULL, NULL);
    g_app.editor_text_view = gtk_text_view_new();
    g_app.editor_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(g_app.editor_text_view));
    apply_css(g_app.editor_text_view, "font-family: monospace; font-size: 13px;");
    gtk_container_add(GTK_CONTAINER(editor_scroll), g_app.editor_text_view);
    gtk_box_pack_start(GTK_BOX(left_box), editor_scroll, TRUE, TRUE, 0);

    /* Student data frame */
    GtkWidget *frame_student = gtk_frame_new("Student academic data");
    GtkWidget *grid_student = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid_student), 4);
    gtk_grid_set_column_spacing(GTK_GRID(grid_student), 6);
    gtk_container_set_border_width(GTK_CONTAINER(grid_student), 6);
    gtk_container_add(GTK_CONTAINER(frame_student), grid_student);

    StudentData def = cr_default_student();
    char buf[32];

    gtk_grid_attach(GTK_GRID(grid_student), gtk_label_new("ATTENDANCE:"), 0, 0, 1, 1);
    g_app.entry_attendance = gtk_entry_new();
    snprintf(buf, sizeof(buf), "%.1f", def.attendance);
    gtk_entry_set_text(GTK_ENTRY(g_app.entry_attendance), buf);
    gtk_grid_attach(GTK_GRID(grid_student), g_app.entry_attendance, 1, 0, 1, 1);

    gtk_grid_attach(GTK_GRID(grid_student), gtk_label_new("CGPA:"), 0, 1, 1, 1);
    g_app.entry_cgpa = gtk_entry_new();
    snprintf(buf, sizeof(buf), "%.1f", def.cgpa);
    gtk_entry_set_text(GTK_ENTRY(g_app.entry_cgpa), buf);
    gtk_grid_attach(GTK_GRID(grid_student), g_app.entry_cgpa, 1, 1, 1, 1);

    gtk_grid_attach(GTK_GRID(grid_student), gtk_label_new("MARKS:"), 0, 2, 1, 1);
    g_app.entry_marks = gtk_entry_new();
    snprintf(buf, sizeof(buf), "%.1f", def.marks);
    gtk_entry_set_text(GTK_ENTRY(g_app.entry_marks), buf);
    gtk_grid_attach(GTK_GRID(grid_student), g_app.entry_marks, 1, 2, 1, 1);

    gtk_grid_attach(GTK_GRID(grid_student), gtk_label_new("BACKLOGS:"), 0, 3, 1, 1);
    g_app.entry_backlogs = gtk_entry_new();
    snprintf(buf, sizeof(buf), "%.1f", def.backlogs);
    gtk_entry_set_text(GTK_ENTRY(g_app.entry_backlogs), buf);
    gtk_grid_attach(GTK_GRID(grid_student), g_app.entry_backlogs, 1, 3, 1, 1);

    gtk_box_pack_start(GTK_BOX(left_box), frame_student, FALSE, FALSE, 0);

    /* Action buttons */
    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    g_app.btn_run = gtk_button_new_with_label("Compile & Run");
    apply_css(g_app.btn_run, "font-weight: bold; font-size: 14px; padding: 6px;");
    g_signal_connect(g_app.btn_run, "clicked", G_CALLBACK(on_run_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(btn_box), g_app.btn_run, TRUE, TRUE, 0);

    g_app.btn_reset = gtk_button_new_with_label("Reset");
    g_signal_connect(g_app.btn_reset, "clicked", G_CALLBACK(on_reset_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(btn_box), g_app.btn_reset, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(left_box), btn_box, FALSE, FALSE, 0);

    /* Right/Center panel: Phase inspection notebook */
    g_app.notebook = gtk_notebook_new();
    gtk_paned_pack2(GTK_PANED(hpaned), g_app.notebook, TRUE, TRUE);

    for (int p = 0; p < PHASE_COUNT; p++) {
        GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
        g_app.phase_text_views[p] = gtk_text_view_new();
        gtk_text_view_set_editable(GTK_TEXT_VIEW(g_app.phase_text_views[p]), FALSE);
        g_app.phase_buffers[p] = gtk_text_view_get_buffer(GTK_TEXT_VIEW(g_app.phase_text_views[p]));
        apply_css(g_app.phase_text_views[p], "font-family: monospace; font-size: 13px;");
        gtk_container_add(GTK_CONTAINER(scroll), g_app.phase_text_views[p]);

        GtkWidget *lbl = gtk_label_new(compiler_phase_name((CompilerPhase)p));
        gtk_notebook_append_page(GTK_NOTEBOOK(g_app.notebook), scroll, lbl);
    }

    /* --- Bottom: Verdict Banner --- */
    g_app.verdict_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    g_app.verdict_label = gtk_label_new("Ready");
    gtk_widget_set_halign(g_app.verdict_label, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(g_app.verdict_box), g_app.verdict_label, TRUE, TRUE, 0);
    show_verdict("Ready", "#9E9E9E");
    gtk_box_pack_start(GTK_BOX(root_vbox), g_app.verdict_box, FALSE, FALSE, 0);

    /* Initialize with sample 0 */
    gtk_text_buffer_set_text(g_app.editor_buffer, compiler_get_sample(0)->code, -1);
    gtk_widget_show_all(g_app.window);
    on_run_clicked(NULL, NULL);

    gtk_main();
    return 0;
}
#endif
