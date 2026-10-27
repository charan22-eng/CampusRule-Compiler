#ifdef _WIN32

#include "gui.h"
#include <windows.h>
#include <commctrl.h>

#define IDC_SAMPLE_COMBO     1001
#define IDC_EDITOR           1002
#define IDC_ATTENDANCE       1003
#define IDC_CGPA             1004
#define IDC_MARKS            1005
#define IDC_BACKLOGS         1006
#define IDC_BTN_RUN          1007
#define IDC_BTN_RESET        1008
#define IDC_TAB_CONTROL      1009
#define IDC_PHASE_OUTPUT     1010

typedef struct {
    HWND hwnd_main;
    HWND hwnd_sample_combo;
    HWND hwnd_editor;

    HWND hwnd_att;
    HWND hwnd_cgpa;
    HWND hwnd_marks;
    HWND hwnd_back;

    HWND hwnd_btn_run;
    HWND hwnd_btn_reset;

    HWND hwnd_pipeline_chips[PHASE_COUNT];
    HWND hwnd_tab;
    HWND hwnd_output;
    HWND hwnd_verdict;

    HFONT hfont_ui;
    HFONT hfont_mono;
    HFONT hfont_bold;
    HFONT hfont_verdict;

    HBRUSH hbr_green;
    HBRUSH hbr_red;
    HBRUSH hbr_amber;
    HBRUSH hbr_grey;
    HBRUSH hbr_active_verdict;

    PhaseStatus phase_statuses[PHASE_COUNT];
    char *phase_texts[PHASE_COUNT];
    char *verdict_text;
    COLORREF verdict_bg_color;
} Win32AppState;

static Win32AppState g_win32;

static void set_output_text(HWND hwnd, const char *text) {
    if (!text) text = "";
    /* Convert \n to \r\n for Win32 EDIT control */
    size_t len = strlen(text);
    size_t extra = 0;
    for (size_t i = 0; i < len; i++) {
        if (text[i] == '\n' && (i == 0 || text[i-1] != '\r')) extra++;
    }
    char *crlf = (char *)malloc(len + extra + 1);
    if (!crlf) {
        SetWindowTextA(hwnd, text);
        return;
    }
    size_t d = 0;
    for (size_t i = 0; i < len; i++) {
        if (text[i] == '\n' && (i == 0 || text[i-1] != '\r')) {
            crlf[d++] = '\r';
        }
        crlf[d++] = text[i];
    }
    crlf[d] = '\0';
    SetWindowTextA(hwnd, crlf);
    free(crlf);
}

static bool parse_double_field(HWND hwnd, const char *fieldname, double *out_val, char *err_buf, size_t err_size) {
    char buf[64];
    GetWindowTextA(hwnd, buf, sizeof(buf));
    char *start = buf;
    while (isspace((unsigned char)*start)) start++;
    if (*start == '\0') {
        snprintf(err_buf, err_size, "Student field %s cannot be empty", fieldname);
        return false;
    }
    char *endptr = NULL;
    *out_val = strtod(start, &endptr);
    while (isspace((unsigned char)*endptr)) endptr++;
    if (endptr == start || *endptr != '\0') {
        snprintf(err_buf, err_size, "Student field %s must contain a valid number", fieldname);
        return false;
    }
    return true;
}

static void win32_run_compiler(void) {
    int len = GetWindowTextLengthA(g_win32.hwnd_editor);
    char *source = (char *)malloc(len + 1);
    if (!source) return;
    GetWindowTextA(g_win32.hwnd_editor, source, len + 1);

    char err_msg[128];
    StudentData student;
    if (!parse_double_field(g_win32.hwnd_att, "ATTENDANCE", &student.attendance, err_msg, sizeof(err_msg)) ||
        !parse_double_field(g_win32.hwnd_cgpa, "CGPA", &student.cgpa, err_msg, sizeof(err_msg)) ||
        !parse_double_field(g_win32.hwnd_marks, "MARKS", &student.marks, err_msg, sizeof(err_msg)) ||
        !parse_double_field(g_win32.hwnd_back, "BACKLOGS", &student.backlogs, err_msg, sizeof(err_msg))) {
        
        free(source);
        if (g_win32.verdict_text) free(g_win32.verdict_text);
        g_win32.verdict_text = cr_strdup(err_msg);
        g_win32.verdict_bg_color = RGB(198, 40, 40);
        g_win32.hbr_active_verdict = g_win32.hbr_red;
        SetWindowTextA(g_win32.hwnd_verdict, g_win32.verdict_text);
        InvalidateRect(g_win32.hwnd_verdict, NULL, TRUE);
        return;
    }

    student.has_attendance = true;
    student.has_cgpa = true;
    student.has_marks = true;
    student.has_backlogs = true;

    CompilerOutput *out = compiler_run(source, &student);
    free(source);
    if (!out) return;

    int select_tab = PHASE_COUNT - 1;
    bool failure_found = false;

    for (int p = 0; p < PHASE_COUNT; p++) {
        if (g_win32.phase_texts[p]) free(g_win32.phase_texts[p]);
        g_win32.phase_texts[p] = cr_strdup(out->text[p] ? out->text[p] : "");
        g_win32.phase_statuses[p] = out->status[p];

        TCITEMA tie;
        tie.mask = TCIF_TEXT;
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
        tie.pszText = tab_title;
        TabCtrl_SetItem(g_win32.hwnd_tab, p, &tie);

        InvalidateRect(g_win32.hwnd_pipeline_chips[p], NULL, TRUE);
    }

    TabCtrl_SetCurSel(g_win32.hwnd_tab, select_tab);
    set_output_text(g_win32.hwnd_output, g_win32.phase_texts[select_tab]);

    if (g_win32.verdict_text) free(g_win32.verdict_text);
    if (out->error) {
        g_win32.verdict_text = cr_strdup(out->error);
        g_win32.verdict_bg_color = RGB(198, 40, 40); /* Red */
        g_win32.hbr_active_verdict = g_win32.hbr_red;
    } else {
        g_win32.verdict_text = cr_strdup(out->verdict);
        if (out->eligible) {
            g_win32.verdict_bg_color = RGB(46, 125, 50); /* Green */
            g_win32.hbr_active_verdict = g_win32.hbr_green;
        } else {
            g_win32.verdict_bg_color = RGB(239, 108, 0); /* Amber */
            g_win32.hbr_active_verdict = g_win32.hbr_amber;
        }
    }

    SetWindowTextA(g_win32.hwnd_verdict, g_win32.verdict_text);
    InvalidateRect(g_win32.hwnd_verdict, NULL, TRUE);
    compiler_free_output(out);
}

static void win32_reset_fields(void) {
    StudentData def = cr_default_student();
    char buf[32];
    snprintf(buf, sizeof(buf), "%.1f", def.attendance);
    SetWindowTextA(g_win32.hwnd_att, buf);
    snprintf(buf, sizeof(buf), "%.1f", def.cgpa);
    SetWindowTextA(g_win32.hwnd_cgpa, buf);
    snprintf(buf, sizeof(buf), "%.1f", def.marks);
    SetWindowTextA(g_win32.hwnd_marks, buf);
    snprintf(buf, sizeof(buf), "%.1f", def.backlogs);
    SetWindowTextA(g_win32.hwnd_back, buf);

    SendMessageA(g_win32.hwnd_sample_combo, CB_SETCURSEL, 0, 0);
    const SampleRule *s = compiler_get_sample(0);
    if (s) {
        set_output_text(g_win32.hwnd_editor, s->code);
    }
    win32_run_compiler();
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);

            if (wmId == IDC_BTN_RUN && wmEvent == BN_CLICKED) {
                win32_run_compiler();
                return 0;
            }

            if (wmId == IDC_BTN_RESET && wmEvent == BN_CLICKED) {
                win32_reset_fields();
                return 0;
            }

            if (wmId == IDC_SAMPLE_COMBO && wmEvent == CBN_SELCHANGE) {
                int idx = (int)SendMessageA(g_win32.hwnd_sample_combo, CB_GETCURSEL, 0, 0);
                const SampleRule *s = compiler_get_sample((size_t)idx);
                if (s) {
                    set_output_text(g_win32.hwnd_editor, s->code);
                    win32_run_compiler();
                }
                return 0;
            }
            break;
        }

        case WM_NOTIFY: {
            NMHDR *pnm = (NMHDR *)lParam;
            if (pnm->idFrom == IDC_TAB_CONTROL && pnm->code == TCN_SELCHANGE) {
                int sel = TabCtrl_GetCurSel(g_win32.hwnd_tab);
                if (sel >= 0 && sel < PHASE_COUNT) {
                    set_output_text(g_win32.hwnd_output, g_win32.phase_texts[sel]);
                }
                return 0;
            }
            break;
        }

        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            HWND hStatic = (HWND)lParam;

            /* Check pipeline chips */
            for (int p = 0; p < PHASE_COUNT; p++) {
                if (hStatic == g_win32.hwnd_pipeline_chips[p]) {
                    SetTextColor(hdc, RGB(255, 255, 255));
                    PhaseStatus s = g_win32.phase_statuses[p];
                    if (s == STATUS_OK) {
                        SetBkColor(hdc, RGB(46, 125, 50));
                        return (LRESULT)g_win32.hbr_green;
                    } else if (s == STATUS_FAILED) {
                        SetBkColor(hdc, RGB(198, 40, 40));
                        return (LRESULT)g_win32.hbr_red;
                    } else {
                        SetBkColor(hdc, RGB(158, 158, 158));
                        return (LRESULT)g_win32.hbr_grey;
                    }
                }
            }

            if (hStatic == g_win32.hwnd_verdict) {
                SetTextColor(hdc, RGB(255, 255, 255));
                SetBkColor(hdc, g_win32.verdict_bg_color);
                return (LRESULT)g_win32.hbr_active_verdict;
            }

            SetBkMode(hdc, TRANSPARENT);
            return (LRESULT)GetStockObject(NULL_BRUSH);
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            (void)hdc;
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_SIZE: {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);

            int left_w = 380;
            int right_w = width - left_w - 30;
            if (right_w < 200) right_w = 200;

            int banner_h = 50;
            int top_h = 45;
            int main_h = height - top_h - banner_h - 20;

            /* Move editor */
            MoveWindow(g_win32.hwnd_editor, 10, 75, left_w, main_h - 225, TRUE);

            /* Move student frame controls */
            int sf_y = 75 + main_h - 215;
            HWND sf = GetDlgItem(hwnd, 9001);
            if (sf) MoveWindow(sf, 10, sf_y, left_w, 155, TRUE);

            MoveWindow(GetDlgItem(hwnd, 9002), 20, sf_y + 25, 120, 20, TRUE);
            MoveWindow(g_win32.hwnd_att, 150, sf_y + 22, 100, 24, TRUE);

            MoveWindow(GetDlgItem(hwnd, 9003), 20, sf_y + 55, 120, 20, TRUE);
            MoveWindow(g_win32.hwnd_cgpa, 150, sf_y + 52, 100, 24, TRUE);

            MoveWindow(GetDlgItem(hwnd, 9004), 20, sf_y + 85, 120, 20, TRUE);
            MoveWindow(g_win32.hwnd_marks, 150, sf_y + 82, 100, 24, TRUE);

            MoveWindow(GetDlgItem(hwnd, 9005), 20, sf_y + 115, 120, 20, TRUE);
            MoveWindow(g_win32.hwnd_back, 150, sf_y + 112, 100, 24, TRUE);

            /* Buttons */
            MoveWindow(g_win32.hwnd_btn_run, 10, sf_y + 165, 270, 36, TRUE);
            MoveWindow(g_win32.hwnd_btn_reset, 290, sf_y + 165, 100, 36, TRUE);

            /* Right tab control and output text area */
            int right_x = left_w + 20;
            MoveWindow(g_win32.hwnd_tab, right_x, top_h + 5, right_w, main_h, TRUE);
            MoveWindow(g_win32.hwnd_output, right_x + 6, top_h + 38, right_w - 12, main_h - 45, TRUE);

            /* Bottom verdict banner */
            int banner_y = height - banner_h - 10;
            MoveWindow(g_win32.hwnd_verdict, 10, banner_y, width - 20, banner_h, TRUE);

            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

int gui_win32_main(HINSTANCE hInstance, int nCmdShow) {
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_TAB_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    g_win32.hfont_ui = CreateFontA(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                  DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                  CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    g_win32.hfont_bold = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                    CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    g_win32.hfont_mono = CreateFontA(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                    CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
    g_win32.hfont_verdict = CreateFontA(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                       DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                       CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");

    g_win32.hbr_green = CreateSolidBrush(RGB(46, 125, 50));
    g_win32.hbr_red = CreateSolidBrush(RGB(198, 40, 40));
    g_win32.hbr_amber = CreateSolidBrush(RGB(239, 108, 0));
    g_win32.hbr_grey = CreateSolidBrush(RGB(158, 158, 158));
    g_win32.hbr_active_verdict = g_win32.hbr_grey;
    g_win32.verdict_bg_color = RGB(158, 158, 158);
    g_win32.verdict_text = cr_strdup("Ready");

    for (int p = 0; p < PHASE_COUNT; p++) {
        g_win32.phase_statuses[p] = STATUS_SKIPPED;
        g_win32.phase_texts[p] = cr_strdup("");
    }

    WNDCLASSEXA wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = "CampusRuleCompilerWindowClass";
    RegisterClassExA(&wc);

    HWND hwnd = CreateWindowExA(0, "CampusRuleCompilerWindowClass",
                                "CampusRule Compiler - Student Eligibility",
                                WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, 1200, 750,
                                NULL, NULL, hInstance, NULL);

    g_win32.hwnd_main = hwnd;

    /* Top: Pipeline chips */
    HWND pipe_lbl = CreateWindowA("STATIC", "Pipeline:",
                                  WS_CHILD | WS_VISIBLE,
                                  10, 14, 60, 20, hwnd, NULL, hInstance, NULL);
    SendMessageA(pipe_lbl, WM_SETFONT, (WPARAM)g_win32.hfont_bold, TRUE);

    int chip_x = 75;
    for (int p = 0; p < PHASE_COUNT; p++) {
        const char *name = compiler_phase_name((CompilerPhase)p);
        int chip_w = (int)(strlen(name) * 9 + 20);
        g_win32.hwnd_pipeline_chips[p] = CreateWindowA("STATIC", name,
                                                       WS_CHILD | WS_VISIBLE | SS_CENTER,
                                                       chip_x, 10, chip_w, 24,
                                                       hwnd, NULL, hInstance, NULL);
        SendMessageA(g_win32.hwnd_pipeline_chips[p], WM_SETFONT, (WPARAM)g_win32.hfont_bold, TRUE);
        chip_x += chip_w + 6;
    }

    /* Left panel: Rule input header */
    HWND lbl_rule = CreateWindowA("STATIC", "Rule input (what the user types):",
                                  WS_CHILD | WS_VISIBLE,
                                  10, 48, 260, 20, hwnd, NULL, hInstance, NULL);
    SendMessageA(lbl_rule, WM_SETFONT, (WPARAM)g_win32.hfont_bold, TRUE);

    /* Sample selector combobox */
    g_win32.hwnd_sample_combo = CreateWindowA("COMBOBOX", "",
                                              WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
                                              260, 44, 130, 240,
                                              hwnd, (HMENU)IDC_SAMPLE_COMBO, hInstance, NULL);
    SendMessageA(g_win32.hwnd_sample_combo, WM_SETFONT, (WPARAM)g_win32.hfont_ui, TRUE);

    for (size_t s = 0; s < compiler_get_samples_count(); s++) {
        SendMessageA(g_win32.hwnd_sample_combo, CB_ADDSTRING, 0, (LPARAM)compiler_get_sample(s)->title);
    }
    SendMessageA(g_win32.hwnd_sample_combo, CB_SETCURSEL, 0, 0);

    /* Editor text area */
    g_win32.hwnd_editor = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                                          WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
                                          ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_WANTRETURN,
                                          10, 75, 380, 260, hwnd, (HMENU)IDC_EDITOR, hInstance, NULL);
    SendMessageA(g_win32.hwnd_editor, WM_SETFONT, (WPARAM)g_win32.hfont_mono, TRUE);

    /* Student data group box */
    HWND grp_student = CreateWindowA("BUTTON", "Student academic data",
                                     WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                                     10, 345, 380, 155, hwnd, (HMENU)9001, hInstance, NULL);
    SendMessageA(grp_student, WM_SETFONT, (WPARAM)g_win32.hfont_bold, TRUE);

    StudentData def = cr_default_student();
    char num_buf[32];

    HWND l1 = CreateWindowA("STATIC", "ATTENDANCE:", WS_CHILD | WS_VISIBLE, 20, 370, 120, 20, hwnd, (HMENU)9002, hInstance, NULL);
    SendMessageA(l1, WM_SETFONT, (WPARAM)g_win32.hfont_ui, TRUE);
    g_win32.hwnd_att = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 150, 368, 100, 24, hwnd, (HMENU)IDC_ATTENDANCE, hInstance, NULL);
    SendMessageA(g_win32.hwnd_att, WM_SETFONT, (WPARAM)g_win32.hfont_ui, TRUE);
    snprintf(num_buf, sizeof(num_buf), "%.1f", def.attendance);
    SetWindowTextA(g_win32.hwnd_att, num_buf);

    HWND l2 = CreateWindowA("STATIC", "CGPA:", WS_CHILD | WS_VISIBLE, 20, 400, 120, 20, hwnd, (HMENU)9003, hInstance, NULL);
    SendMessageA(l2, WM_SETFONT, (WPARAM)g_win32.hfont_ui, TRUE);
    g_win32.hwnd_cgpa = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 150, 398, 100, 24, hwnd, (HMENU)IDC_CGPA, hInstance, NULL);
    SendMessageA(g_win32.hwnd_cgpa, WM_SETFONT, (WPARAM)g_win32.hfont_ui, TRUE);
    snprintf(num_buf, sizeof(num_buf), "%.1f", def.cgpa);
    SetWindowTextA(g_win32.hwnd_cgpa, num_buf);

    HWND l3 = CreateWindowA("STATIC", "MARKS:", WS_CHILD | WS_VISIBLE, 20, 430, 120, 20, hwnd, (HMENU)9004, hInstance, NULL);
    SendMessageA(l3, WM_SETFONT, (WPARAM)g_win32.hfont_ui, TRUE);
    g_win32.hwnd_marks = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 150, 428, 100, 24, hwnd, (HMENU)IDC_MARKS, hInstance, NULL);
    SendMessageA(g_win32.hwnd_marks, WM_SETFONT, (WPARAM)g_win32.hfont_ui, TRUE);
    snprintf(num_buf, sizeof(num_buf), "%.1f", def.marks);
    SetWindowTextA(g_win32.hwnd_marks, num_buf);

    HWND l4 = CreateWindowA("STATIC", "BACKLOGS:", WS_CHILD | WS_VISIBLE, 20, 460, 120, 20, hwnd, (HMENU)9005, hInstance, NULL);
    SendMessageA(l4, WM_SETFONT, (WPARAM)g_win32.hfont_ui, TRUE);
    g_win32.hwnd_back = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 150, 458, 100, 24, hwnd, (HMENU)IDC_BACKLOGS, hInstance, NULL);
    SendMessageA(g_win32.hwnd_back, WM_SETFONT, (WPARAM)g_win32.hfont_ui, TRUE);
    snprintf(num_buf, sizeof(num_buf), "%.1f", def.backlogs);
    SetWindowTextA(g_win32.hwnd_back, num_buf);

    /* Run & Reset buttons */
    g_win32.hwnd_btn_run = CreateWindowA("BUTTON", "Compile & Run",
                                         WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                         10, 510, 270, 36, hwnd, (HMENU)IDC_BTN_RUN, hInstance, NULL);
    SendMessageA(g_win32.hwnd_btn_run, WM_SETFONT, (WPARAM)g_win32.hfont_bold, TRUE);

    g_win32.hwnd_btn_reset = CreateWindowA("BUTTON", "Reset",
                                           WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                           290, 510, 100, 36, hwnd, (HMENU)IDC_BTN_RESET, hInstance, NULL);
    SendMessageA(g_win32.hwnd_btn_reset, WM_SETFONT, (WPARAM)g_win32.hfont_ui, TRUE);

    /* Right panel: Tab control */
    g_win32.hwnd_tab = CreateWindowA(WC_TABCONTROLA, "",
                                     WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
                                     400, 45, 770, 500, hwnd, (HMENU)IDC_TAB_CONTROL, hInstance, NULL);
    SendMessageA(g_win32.hwnd_tab, WM_SETFONT, (WPARAM)g_win32.hfont_ui, TRUE);

    for (int p = 0; p < PHASE_COUNT; p++) {
        TCITEMA tie;
        tie.mask = TCIF_TEXT;
        tie.pszText = (LPSTR)compiler_phase_name((CompilerPhase)p);
        TabCtrl_InsertItem(g_win32.hwnd_tab, p, &tie);
    }

    /* Output text area */
    g_win32.hwnd_output = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                                          WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
                                          ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
                                          406, 78, 758, 455, hwnd, (HMENU)IDC_PHASE_OUTPUT, hInstance, NULL);
    SendMessageA(g_win32.hwnd_output, WM_SETFONT, (WPARAM)g_win32.hfont_mono, TRUE);

    /* Bottom: Verdict banner */
    g_win32.hwnd_verdict = CreateWindowA("STATIC", "Ready",
                                         WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE,
                                         10, 560, 1160, 50, hwnd, NULL, hInstance, NULL);
    SendMessageA(g_win32.hwnd_verdict, WM_SETFONT, (WPARAM)g_win32.hfont_verdict, TRUE);

    /* Initial population */
    set_output_text(g_win32.hwnd_editor, compiler_get_sample(0)->code);
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    win32_run_compiler();

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return (int)msg.wParam;
}

#endif /* _WIN32 */
