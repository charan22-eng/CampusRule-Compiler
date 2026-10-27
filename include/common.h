/**
 * @file common.h
 * @brief Core utilities, shared definitions, dynamic StringBuilder, and student data model.
 *
 * Provides cross-platform string manipulation helpers, dynamic string buffers for
 * compiler phase dumps, academic domain constants (variables, categories), and the
 * StudentData structure used for rule evaluation.
 */

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include <stdarg.h>

#ifdef _WIN32
    #define CR_STRCASECMP _stricmp
    #define CR_STRNCASECMP _strnicmp
#else
    #include <strings.h>
    #define CR_STRCASECMP strcasecmp
    #define CR_STRNCASECMP strncasecmp
#endif

/**
 * @brief Safe duplicate of a C string with NULL guard.
 */
static inline char *cr_strdup(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char *copy = (char *)malloc(len + 1);
    if (copy) {
        memcpy(copy, s, len + 1);
    }
    return copy;
}

static inline char *cr_strndup(const char *s, size_t n) {
    if (!s) return NULL;
    size_t len = strlen(s);
    if (len > n) len = n;
    char *copy = (char *)malloc(len + 1);
    if (copy) {
        memcpy(copy, s, len);
        copy[len] = '\0';
    }
    return copy;
}

/* String Builder utility for formatted text output across compiler phases */
typedef struct {
    char *buffer;
    size_t length;
    size_t capacity;
} StringBuilder;

static inline void sb_init(StringBuilder *sb) {
    sb->capacity = 256;
    sb->length = 0;
    sb->buffer = (char *)malloc(sb->capacity);
    if (sb->buffer) {
        sb->buffer[0] = '\0';
    }
}

static inline void sb_append(StringBuilder *sb, const char *str) {
    if (!str || !sb->buffer) return;
    size_t slen = strlen(str);
    if (sb->length + slen + 1 > sb->capacity) {
        size_t new_cap = (sb->capacity * 2 > sb->length + slen + 1) 
                         ? sb->capacity * 2 
                         : sb->length + slen + 256;
        char *new_buf = (char *)realloc(sb->buffer, new_cap);
        if (!new_buf) return;
        sb->buffer = new_buf;
        sb->capacity = new_cap;
    }
    memcpy(sb->buffer + sb->length, str, slen);
    sb->length += slen;
    sb->buffer[sb->length] = '\0';
}

static inline void sb_appendf(StringBuilder *sb, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    va_list args_copy;
    va_copy(args_copy, args);
    int needed = vsnprintf(NULL, 0, fmt, args_copy);
    va_end(args_copy);

    if (needed > 0) {
        if (sb->length + (size_t)needed + 1 > sb->capacity) {
            size_t new_cap = (sb->capacity * 2 > sb->length + (size_t)needed + 1)
                             ? sb->capacity * 2
                             : sb->length + (size_t)needed + 256;
            char *new_buf = (char *)realloc(sb->buffer, new_cap);
            if (!new_buf) {
                va_end(args);
                return;
            }
            sb->buffer = new_buf;
            sb->capacity = new_cap;
        }
        vsnprintf(sb->buffer + sb->length, sb->capacity - sb->length, fmt, args);
        sb->length += (size_t)needed;
    }
    va_end(args);
}

static inline char *sb_to_string(StringBuilder *sb) {
    if (!sb->buffer) return cr_strdup("");
    return cr_strdup(sb->buffer);
}

static inline void sb_free(StringBuilder *sb) {
    if (sb->buffer) {
        free(sb->buffer);
        sb->buffer = NULL;
    }
    sb->length = 0;
    sb->capacity = 0;
}

/* Centralized Academic Rule Constraints */
#define VAR_ATTENDANCE "ATTENDANCE"
#define VAR_CGPA       "CGPA"
#define VAR_MARKS      "MARKS"
#define VAR_BACKLOGS   "BACKLOGS"

#define RANGE_ATTENDANCE_MIN 0.0
#define RANGE_ATTENDANCE_MAX 100.0

#define RANGE_CGPA_MIN 0.0
#define RANGE_CGPA_MAX 10.0

#define RANGE_MARKS_MIN 0.0
#define RANGE_MARKS_MAX 100.0

#define RANGE_BACKLOGS_MIN 0.0
#define RANGE_BACKLOGS_MAX 50.0

/* Student academic input representation */
typedef struct {
    double attendance;
    double cgpa;
    double marks;
    double backlogs;
    bool has_attendance;
    bool has_cgpa;
    bool has_marks;
    bool has_backlogs;
} StudentData;

static inline StudentData cr_default_student(void) {
    StudentData s;
    s.attendance = 78.0;
    s.cgpa = 8.2;
    s.marks = 82.0;
    s.backlogs = 0.0;
    s.has_attendance = true;
    s.has_cgpa = true;
    s.has_marks = true;
    s.has_backlogs = true;
    return s;
}

#endif /* COMMON_H */
