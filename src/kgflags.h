/*
 kgflags v0.6.1
 http://github.com/kgabis/kgflags/
 Copyright (c) 2020 Krzysztof Gabis
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.

### About:

 kgflags is a simple to use command-line flag parsing library.

 Visit https://github.com/kgabis/kgflags/tree/master/readme.md for more info.

### Example:

#define KGFLAGS_IMPLEMENTATION
#include "kgflags.h"

int main(int argc, char **argv) {
    const char *to_print = NULL;  // guaranteed to be assigned only if kgflags_parse succeeds
    kgflags_string("to-print", NULL, "String to print.", true, &to_print);
    if (!kgflags_parse(argc, argv)) {
        kgflags_print_errors();
        kgflags_print_usage();
        return 1;
    }
    puts(to_print);
    return 0;
}
*/

#ifndef KGFLAGS_INCLUDE_KGFLAGS_H
#define KGFLAGS_INCLUDE_KGFLAGS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef struct kgflags_string_array {
    char **_items; // private
    int _count; // private
} kgflags_string_array_t;

typedef struct kgflags_int_array {
    char **_items; // private
    int _count; // private
} kgflags_int_array_t;

typedef struct kgflags_double_array {
    char **_items; // private
    int _count; // private
} kgflags_double_array_t;

#ifndef KGFLAGS_MAX_FLAGS
#define KGFLAGS_MAX_FLAGS 256
#endif

#ifndef KGFLAGS_MAX_NON_FLAG_ARGS
#define KGFLAGS_MAX_NON_FLAG_ARGS 512
#endif

#ifndef KGFLAGS_MAX_ERRORS
#define KGFLAGS_MAX_ERRORS 512
#endif

// Functions used to declare flags. If kgflags_parse succeeds values are assigned to out_res/out_arr. Description is optional.
void kgflags_string(const char *name, const char *default_value, const char *description, bool required, const char** out_res);
void kgflags_bool(const char *name, bool default_value, const char *description, bool required, bool *out_res);
void kgflags_int(const char *name, int default_value, const char *description, bool required, int *out_res);
void kgflags_double(const char *name, double default_value, const char *description, bool required, double *out_res);

// Arrays are parsed until first flag argument. e.g. "--arr 1 2 3 --next-flag" -> arr == [1, 2, 3].
void kgflags_string_array(const char *name, const char *description, bool required, kgflags_string_array_t *out_arr);
void kgflags_int_array(const char *name, const char *description, bool required, kgflags_int_array_t *out_arr);
void kgflags_double_array(const char *name, const char *description, bool required, kgflags_double_array_t *out_arr);

// Optionally sets prefix used for flags (such as "--", "-" or "/").
// Default prefix is "--". Should be called *before* calling kgflags_parse.
void kgflags_set_prefix(const char *prefix);

// Parses arguments and assign values to declared flags.
bool kgflags_parse(int argc, char **argv);

// Prints errors that might've occured when declaring flags or during flag parsing.
void kgflags_print_errors(void);

// Prints usage based on flags declared with kgflags_string, kgflags_int etc.
// Can be customized with custom description by calling kgflags_set_custom_description.
// By default it starts with "Usage of ./app:". If custom_description is set with
// kgflags_set_custom_description it will print it instead.
void kgflags_print_usage(void);

// Sets custom description printed by kgflags_print_usage.
// e.g. kgflags_set_custom_description("Usage: ./app [--FLAGS] [file ...]");
void kgflags_set_custom_description(const char *description);

int kgflags_string_array_get_count(const kgflags_string_array_t *arr);
const char* kgflags_string_array_get_item(const kgflags_string_array_t *arr, int at);

// Result is parsed from string every time you get an item.
int kgflags_int_array_get_count(const kgflags_int_array_t *arr);
int kgflags_int_array_get_item(const kgflags_int_array_t *arr, int at);

// Result is parsed from string every time you get an item.
int kgflags_double_array_get_count(const kgflags_double_array_t *arr);
double kgflags_double_array_get_item(const kgflags_double_array_t *arr, int at);

// Returns arguments that don't belong to any flags.
// e.g. if we defined a flag named "file" and call "./app arg0 --file test arg1"
// then non-flag arguments' count is 2 and non-flag[0] is arg0 and non-flag[1] is arg1.
int kgflags_get_non_flag_args_count(void);
const char* kgflags_get_non_flag_arg(int at);

#ifdef __cplusplus
}
#endif

#endif /* KGFLAGS_INCLUDE_KGFLAGS_H */

#ifdef KGFLAGS_IMPLEMENTATION
#undef KGFLAGS_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>

typedef enum _kgflags_flag_kind {
    KGFLAGS_FLAG_KIND_NONE,
    KGFLAGS_FLAG_KIND_STRING,
    KGFLAGS_FLAG_KIND_BOOL,
    KGFLAGS_FLAG_KIND_INT,
    KGFLAGS_FLAG_KIND_DOUBLE,
    KGFLAGS_FLAG_KIND_STRING_ARRAY,
    KGFLAGS_FLAG_KIND_INT_ARRAY,
    KGFLAGS_FLAG_KIND_DOUBLE_ARRAY,
} _kgflags_flag_kind_t;

typedef struct _kgflags_flag {
    const char *name;
    const char *description;
    union {
        const char *string_value;
        bool bool_value;
        int int_value;
        double double_value;
    } default_value;
    union {
        const char **string_value;
        bool *bool_value;
        int *int_value;
        double *double_value;
        kgflags_string_array_t *string_array;
        kgflags_int_array_t *int_array;
        kgflags_double_array_t *double_array;
    } result;
    bool assigned;
    bool error;
    bool required;
    _kgflags_flag_kind_t kind;
} _kgflags_flag_t;

typedef enum _kgflags_error_kind {
    KGFLAGS_ERROR_KIND_NONE,
    KGFLAGS_ERROR_KIND_MISSING_VALUE,
    KGFLAGS_ERROR_KIND_UNKNOWN_FLAG,
    KGFLAGS_ERROR_KIND_UNASSIGNED_FLAG,
    KGFLAGS_ERROR_KIND_INVALID_INT,
    KGFLAGS_ERROR_KIND_INVALID_DOUBLE,
    KGFLAGS_ERROR_KIND_TOO_MANY_FLAGS,
    KGFLAGS_ERROR_KIND_TOO_MANY_NON_FLAG_ARGS,
    KGFLAGS_ERROR_KIND_MULTIPLE_ASSIGNMENT,
    KGFLAGS_ERROR_KIND_DUPLICATE_FLAG,
    KGFLAGS_ERROR_KIND_PREFIX_NO,
} _kgflags_error_kind_t;

typedef struct _kgflags_error {
    const char *flag_name;
    const char *arg;
    _kgflags_error_kind_t kind;
} _kgflags_error_t;

static bool _kgflags_is_flag(const char* arg);
static const char* _kgflags_get_flag_name(const char* arg);
static void _kgflags_add_flag(_kgflags_flag_t arg);
static _kgflags_flag_t* _kgflags_get_flag(const char* name, bool *out_prefix_no);
static int _kgflags_parse_int(const char *str, bool *out_ok);
static double _kgflags_parse_double(const char *str, bool *out_ok);
static void _kgflags_add_error(_kgflags_error_kind_t kind, const char *flag, const char *arg);
static void _kgflags_assign_default_values(void);
static bool _kgflags_add_non_flag_arg(const char* arg);
static const char* _kgflags_consume_arg(void);
static const char* _kgflags_peek_arg(void);
static void _kgflags_parse_flag(_kgflags_flag_t *flag, bool prefix_no);

static struct {
    int flags_count;
    _kgflags_flag_t flags[KGFLAGS_MAX_FLAGS];

    int non_flag_count;
    const char* non_flag_args[KGFLAGS_MAX_NON_FLAG_ARGS];

    int errors_count;
    _kgflags_error_t errors[KGFLAGS_MAX_ERRORS];

    const char *flag_prefix;

    int arg_cursor;
    int argc;
    char **argv;

    const char *custom_description;
} _kgflags_g;

void kgflags_string(const char *name, const char *default_value, const char *description, bool required, const char** out_res) {
    *out_res = NULL;

    _kgflags_flag_t flag;
    memset(&flag, 0, sizeof(_kgflags_flag_t));
    flag.kind = KGFLAGS_FLAG_KIND_STRING;
    flag.name = name;
    flag.default_value.string_value = default_value;
    flag.description = description;
    flag.required = required;
    flag.result.string_value = out_res;
    flag.assigned = false;
    _kgflags_add_flag(flag);
}

void kgflags_bool(const char *name, bool default_value, const char *description, bool required, bool *out_res) {
    *out_res = false;

    if (strstr(name, "no-") == name) {
        _kgflags_add_error(KGFLAGS_ERROR_KIND_PREFIX_NO, name, NULL);
        return;
    }

    _kgflags_flag_t flag;
    memset(&flag, 0, sizeof(_kgflags_flag_t));
    flag.kind = KGFLAGS_FLAG_KIND_BOOL;
    flag.name = name;
    flag.default_value.bool_value = default_value;
    flag.description = description;
    flag.required = required;
    flag.result.bool_value = out_res;
    flag.assigned = false;
    _kgflags_add_flag(flag);
}

void kgflags_int(const char *name, int default_value, const char *description, bool required, int *out_res) {
    *out_res = 0;

    _kgflags_flag_t flag;
    memset(&flag, 0, sizeof(_kgflags_flag_t));
    flag.kind = KGFLAGS_FLAG_KIND_INT;
    flag.name = name;
    flag.default_value.int_value = default_value;
    flag.description = description;
    flag.required = required;
    flag.result.int_value = out_res;
    flag.assigned = false;
    _kgflags_add_flag(flag);
}

void kgflags_double(const char *name, double default_value, const char *description, bool required, double *out_res) {
    *out_res = 0.0;

    _kgflags_flag_t flag;
    memset(&flag, 0, sizeof(_kgflags_flag_t));
    flag.kind = KGFLAGS_FLAG_KIND_DOUBLE;
    flag.name = name;
    flag.default_value.double_value = default_value;
    flag.description = description;
    flag.required = required;
    flag.result.double_value = out_res;
    flag.assigned = false;
    _kgflags_add_flag(flag);
}

void kgflags_string_array(const char *name, const char *description, bool required, kgflags_string_array_t *out_arr) {
    out_arr->_items = NULL;
    out_arr->_count = 0;

    _kgflags_flag_t flag;
    memset(&flag, 0, sizeof(_kgflags_flag_t));
    flag.kind = KGFLAGS_FLAG_KIND_STRING_ARRAY;
    flag.name = name;
    flag.description = description;
    flag.required = required;
    flag.result.string_array = out_arr;
    flag.assigned = false;
    _kgflags_add_flag(flag);
}

void kgflags_int_array(const char *name, const char *description, bool required, kgflags_int_array_t *out_arr) {
    out_arr->_items = NULL;
    out_arr->_count = 0;

    _kgflags_flag_t flag;
    memset(&flag, 0, sizeof(_kgflags_flag_t));
    flag.kind = KGFLAGS_FLAG_KIND_INT_ARRAY;
    flag.name = name;
    flag.description = description;
    flag.required = required;
    flag.result.int_array = out_arr;
    flag.assigned = false;
    _kgflags_add_flag(flag);
}

void kgflags_double_array(const char *name, const char *description, bool required, kgflags_double_array_t *out_arr) {
    out_arr->_items = NULL;
    out_arr->_count = 0;

    _kgflags_flag_t flag;
    memset(&flag, 0, sizeof(_kgflags_flag_t));
    flag.kind = KGFLAGS_FLAG_KIND_DOUBLE_ARRAY;
    flag.name = name;
    flag.description = description;
    flag.required = required;
    flag.result.double_array = out_arr;
    flag.assigned = false;
    _kgflags_add_flag(flag);
}

void kgflags_set_prefix(const char *prefix) {
    _kgflags_g.flag_prefix = prefix;
}

bool kgflags_parse(int argc, char **argv) {
    _kgflags_g.argc = argc;
    _kgflags_g.argv = argv;
    _kgflags_g.arg_cursor = 1;

    if (_kgflags_g.flag_prefix == NULL) {
        _kgflags_g.flag_prefix = "--";
    }

    if (_kgflags_g.errors_count > 0) {
        return false;
    }

    const char *arg = NULL;
    while ((arg = _kgflags_consume_arg()) != NULL) {
        _kgflags_flag_t *flag = NULL;
        bool is_flag = _kgflags_is_flag(arg);
        bool prefix_no = false;
        if (is_flag) {
            const char *flag_name = _kgflags_get_flag_name(arg);
            flag = _kgflags_get_flag(flag_name, &prefix_no);
            if (flag == NULL) {
                _kgflags_add_error(KGFLAGS_ERROR_KIND_UNKNOWN_FLAG, flag_name, NULL);
                continue;
            }
        } else {
            _kgflags_add_non_flag_arg(arg);
            continue;
        }

        if (flag->assigned) {
            _kgflags_add_error(KGFLAGS_ERROR_KIND_MULTIPLE_ASSIGNMENT, flag->name, NULL);
        }

        _kgflags_parse_flag(flag, prefix_no);
    }

    _kgflags_assign_default_values();

    for (int i = 0; i < _kgflags_g.flags_count; i++) {
        _kgflags_flag_t *flag = &_kgflags_g.flags[i];
        if (flag->required && !flag->assigned && !flag->error) {
            _kgflags_add_error(KGFLAGS_ERROR_KIND_UNASSIGNED_FLAG, flag->name, NULL);
        }
    }

    if (_kgflags_g.errors_count > 0) {
        return false;
    }

    return true;
}

void kgflags_print_errors(void) {
    for (int i = 0; i < _kgflags_g.errors_count; i++) {
        _kgflags_error_t *err = &_kgflags_g.errors[i];
        switch (err->kind) {
            case KGFLAGS_ERROR_KIND_MISSING_VALUE: {
                fprintf(stderr, "Missing value for flag: %s%s\n", _kgflags_g.flag_prefix,  err->flag_name);
                break;
            }
            case KGFLAGS_ERROR_KIND_UNKNOWN_FLAG: {
                fprintf(stderr, "Unrecognized flag: %s%s\n", _kgflags_g.flag_prefix, err->flag_name);
                break;
            }
            case KGFLAGS_ERROR_KIND_UNASSIGNED_FLAG: {
                fprintf(stderr, "Unassigned required flag: %s%s\n", _kgflags_g.flag_prefix, err->flag_name);
                break;
            }
            case KGFLAGS_ERROR_KIND_INVALID_INT: {
                fprintf(stderr, "Invalid value for flag: %s%s (got %s, expected integer)\n", _kgflags_g.flag_prefix, err->flag_name, err->arg);
                break;
            }
            case KGFLAGS_ERROR_KIND_INVALID_DOUBLE: {
                fprintf(stderr, "Invalid value for flag: %s%s (got %s, expected number)\n", _kgflags_g.flag_prefix, err->flag_name, err->arg);
                break;
            }
            case KGFLAGS_ERROR_KIND_TOO_MANY_FLAGS: {
                fprintf(stderr, "Too many flags declared.");
                break;
            }
            case KGFLAGS_ERROR_KIND_TOO_MANY_NON_FLAG_ARGS: {
                fprintf(stderr, "Too many non-flag arguments passed to program.");
                break;
            }
            case KGFLAGS_ERROR_KIND_MULTIPLE_ASSIGNMENT: {
                fprintf(stderr, "Multiple assignment of flag: %s%s\n", _kgflags_g.flag_prefix, err->flag_name);
                break;
            }
            case KGFLAGS_ERROR_KIND_DUPLICATE_FLAG: {
                fprintf(stderr, "Redeclaration of flag: %s%s\n", _kgflags_g.flag_prefix, err->flag_name);
                break;
            }
            case KGFLAGS_ERROR_KIND_PREFIX_NO: {
                fprintf(stderr, "Used \"no-\" prefix when declaring boolean flag: %s%s\n", _kgflags_g.flag_prefix, err->flag_name);
                break;
            }
            default:
                break;
        }
    }
}

void kgflags_print_usage() {
    if (_kgflags_g.custom_description == NULL) {
        fprintf(stderr, "Usage of %s:\n", _kgflags_g.argv[0]);
    } else {
        fprintf(stderr, "%s\n", _kgflags_g.custom_description);
    }

    fprintf(stderr, "Flags:\n");
    for (int i = 0; i < _kgflags_g.flags_count; i++) {
        _kgflags_flag_t *flag = &_kgflags_g.flags[i];
        switch (flag->kind) {
            case KGFLAGS_FLAG_KIND_STRING:
                fprintf(stderr, "\t%s%s\t(string%s\n", _kgflags_g.flag_prefix, flag->name, flag->required ? ")" : ", optional)");
                if (!flag->required) {
                    fprintf(stderr, "\t\tDefault: %s\n", flag->default_value.string_value);
                }
                break;
            case KGFLAGS_FLAG_KIND_BOOL: {
                fprintf(stderr, "\t%s%s, %sno-%s\t(boolean%s\n", _kgflags_g.flag_prefix, flag->name, _kgflags_g.flag_prefix, flag->name,
                    flag->required ? ")" : ", optional)");
                if (!flag->required) {
                    fprintf(stderr, "\t\tDefault: %s\n", flag->default_value.bool_value ? "True" : "False");
                }
                break;
            }
            case KGFLAGS_FLAG_KIND_INT: {
                fprintf(stderr, "\t%s%s\t(integer%s\n", _kgflags_g.flag_prefix, flag->name, flag->required ? ")" : ", optional)");
                if (!flag->required) {
                    fprintf(stderr, "\t\tDefault: %d\n", flag->default_value.int_value);
                }
                break;
            }
            case KGFLAGS_FLAG_KIND_DOUBLE: {
                fprintf(stderr, "\t%s%s\t(float%s\n", _kgflags_g.flag_prefix, flag->name, flag->required ? ")" : ", optional)");
                if (!flag->required) {
                    fprintf(stderr, "\t\tDefault: %1.4g\n", flag->default_value.double_value);
                }
                break;
            }
            case KGFLAGS_FLAG_KIND_STRING_ARRAY: {
                fprintf(stderr, "\t%s%s\t(array of strings%s\n", _kgflags_g.flag_prefix, flag->name, flag->required ? ")" : ", optional)");
                break;
            }
            case KGFLAGS_FLAG_KIND_INT_ARRAY: {
                fprintf(stderr, "\t%s%s\t(array of integers%s\n", _kgflags_g.flag_prefix, flag->name, flag->required ? ")" : ", optional)");
                break;
            }
            case KGFLAGS_FLAG_KIND_DOUBLE_ARRAY: {
                fprintf(stderr, "\t%s%s\t(array of floats%s\n", _kgflags_g.flag_prefix, flag->name, flag->required ? ")" : ", optional)");
                break;
            }
            default:
                break;
        }
        if (flag->description) {
            fprintf(stderr, "\t\t%s\n", flag->description);
        }
        fprintf(stderr, "\n");
    }
}

void kgflags_set_custom_description(const char *description) {
    _kgflags_g.custom_description = description;
}

int kgflags_string_array_get_count(const kgflags_string_array_t *arr) {
    return arr->_count;
}

const char* kgflags_string_array_get_item(const kgflags_string_array_t *arr, int at) {
    if (at < 0 || at >= arr->_count) {
        return NULL;
    }
    return arr->_items[at];
}

int kgflags_int_array_get_count(const kgflags_int_array_t *arr) {
    return arr->_count;
}

int kgflags_int_array_get_item(const kgflags_int_array_t *arr, int at) {
    if (at < 0 || at >= arr->_count) {
        return 0;
    }
    const char *str = arr->_items[at];
    bool ok = false;
    int res = _kgflags_parse_int(str, &ok);
    if (!ok) {
        return 0;
    }
    return res;
}

int kgflags_double_array_get_count(const kgflags_double_array_t *arr) {
    return arr->_count;
}

double kgflags_double_array_get_item(const kgflags_double_array_t *arr, int at) {
    if (at < 0 || at >= arr->_count) {
        return 0.0;
    }
    const char *str = arr->_items[at];
    bool ok = false;
    double res = _kgflags_parse_double(str, &ok);
    if (!ok) {
        return 0.0;
    }
    return res;
}

int kgflags_get_non_flag_args_count(void) {
    return _kgflags_g.non_flag_count;
}

const char* kgflags_get_non_flag_arg(int at) {
    if (at < 0 || at >= _kgflags_g.non_flag_count) {
        return NULL;
    }
    return _kgflags_g.non_flag_args[at];
}

/**************************************************************/
/* INTERNAL FUNCTIONS */
/**************************************************************/

static bool _kgflags_is_flag(const char *arg) {
    return _kgflags_get_flag_name(arg) != NULL;
}

static const char* _kgflags_get_flag_name(const char* arg) {
    unsigned long prefix_len = strlen(_kgflags_g.flag_prefix);
    if (strlen(arg) < prefix_len) {
        return NULL;
    }
    if (strncmp(arg, _kgflags_g.flag_prefix, prefix_len) != 0) {
        return NULL;
    }
    return arg + prefix_len;
}

static void _kgflags_add_flag(_kgflags_flag_t flag) {
    if (_kgflags_get_flag(flag.name, NULL) != NULL) {
        _kgflags_add_error(KGFLAGS_ERROR_KIND_DUPLICATE_FLAG, flag.name, NULL);
        return;
    }
    if (_kgflags_g.flags_count >= KGFLAGS_MAX_FLAGS) {
        _kgflags_add_error(KGFLAGS_ERROR_KIND_TOO_MANY_FLAGS, NULL, NULL);
        return;
    }
    _kgflags_g.flags[_kgflags_g.flags_count] = flag;
    _kgflags_g.flags_count++;
}

static _kgflags_flag_t* _kgflags_get_flag(const char* name, bool *out_prefix_no) {
    if (out_prefix_no) {
        *out_prefix_no = false;
    }
    for (int i = 0; i < _kgflags_g.flags_count; i++) {
        _kgflags_flag_t *flag = &_kgflags_g.flags[i];
        if (strcmp(name, flag->name) == 0) {
            return flag;
        }
        if (flag->kind == KGFLAGS_FLAG_KIND_BOOL && strstr(name, "no-") == name) {
            if (strcmp(name + 3, flag->name) == 0) {
                if (out_prefix_no) {
                    *out_prefix_no = true;
                }
                return flag;
            }
        }
    }
    return NULL;
}

static int _kgflags_parse_int(const char *str, bool *out_ok) {
    *out_ok = false;
    char *end = NULL;
    long res_l = strtol(str, &end, 10);
    if (end == str || *end != '\0' || res_l > INT_MAX || res_l < INT_MIN
    || ((res_l == LONG_MIN || res_l == LONG_MAX) && ERANGE == errno)) {
        *out_ok = false;
        return 0;
    }
    *out_ok = true;
    return (int)res_l;
}

static double _kgflags_parse_double(const char *str, bool *out_ok) {
    *out_ok = false;
    char *end = NULL;
    double res = strtod(str, &end);
    if (end == str || *end != '\0'
    || ((res == -HUGE_VAL || res == +HUGE_VAL) && ERANGE == errno)) {
        *out_ok = false;
        return 0.0;
    }
    *out_ok = true;
    return res;
}

static void _kgflags_add_error(_kgflags_error_kind_t kind, const char *flag_name, const char *arg) {
    _kgflags_error_t err;
    err.kind = kind;
    err.flag_name = flag_name;
    err.arg = arg;
    if (_kgflags_g.errors_count >= KGFLAGS_MAX_ERRORS) {
        return;
    }
    _kgflags_g.errors[_kgflags_g.errors_count] = err;
    _kgflags_g.errors_count++;
}


static void _kgflags_assign_default_values() {
    for (int i = 0; i < _kgflags_g.flags_count; i++) {
        _kgflags_flag_t *flag = &_kgflags_g.flags[i];
        if (flag->assigned || flag->required) {
            continue;
        }
        switch (flag->kind) {
            case KGFLAGS_FLAG_KIND_STRING: {
                *flag->result.string_value = flag->default_value.string_value;
                flag->assigned = true;
                break;
            }
            case KGFLAGS_FLAG_KIND_BOOL: {
                *flag->result.bool_value = flag->default_value.bool_value;
                flag->assigned = true;
                break;
            }
            case KGFLAGS_FLAG_KIND_INT: {
                *flag->result.int_value = flag->default_value.int_value;
                flag->assigned = true;
                break;
            }
            case KGFLAGS_FLAG_KIND_DOUBLE: {
                *flag->result.double_value = flag->default_value.double_value;
                flag->assigned = true;
                break;
            }
            default:
                break;
        }
    }
}

static bool _kgflags_add_non_flag_arg(const char* arg) {
    if (_kgflags_g.non_flag_count >= KGFLAGS_MAX_NON_FLAG_ARGS) {
        _kgflags_add_error(KGFLAGS_ERROR_KIND_TOO_MANY_NON_FLAG_ARGS, NULL, NULL);
        return false;
    }
    _kgflags_g.non_flag_args[_kgflags_g.non_flag_count] = arg;
    _kgflags_g.non_flag_count++;
    return true;
}

static const char* _kgflags_consume_arg() {
    if (_kgflags_g.arg_cursor >= _kgflags_g.argc) {
        return NULL;
    }
    const char *res = _kgflags_g.argv[_kgflags_g.arg_cursor];
    _kgflags_g.arg_cursor++;
    return res;
}

static const char* _kgflags_peek_arg() {
    if (_kgflags_g.arg_cursor >= _kgflags_g.argc) {
        return NULL;
    }
    return _kgflags_g.argv[_kgflags_g.arg_cursor];
}

static void _kgflags_parse_flag(_kgflags_flag_t *flag, bool prefix_no) {
    switch (flag->kind) {
        case KGFLAGS_FLAG_KIND_STRING: {
            const char *val = _kgflags_consume_arg();
            if (!val) {
                flag->error = true;
                _kgflags_add_error(KGFLAGS_ERROR_KIND_MISSING_VALUE, flag->name, NULL);
                return;
            }
            *flag->result.string_value = val;
            flag->assigned = true;
            break;
        }
        case KGFLAGS_FLAG_KIND_BOOL: {
            *flag->result.bool_value = !prefix_no;
            flag->assigned = true;
            break;
        }
        case KGFLAGS_FLAG_KIND_INT: {
            const char *val = _kgflags_consume_arg();
            if (!val) {
                flag->error = true;
                _kgflags_add_error(KGFLAGS_ERROR_KIND_MISSING_VALUE, flag->name, NULL);
                return;
            }
            bool ok = false;
            int int_val = _kgflags_parse_int(val, &ok);
            if (!ok) {
                flag->error = true;
                _kgflags_add_error(KGFLAGS_ERROR_KIND_INVALID_INT, flag->name, val);
                return;
            }
            *flag->result.int_value = int_val;
            flag->assigned = true;
            break;
        }
        case KGFLAGS_FLAG_KIND_DOUBLE: {
            const char *val = _kgflags_consume_arg();
            if (!val) {
                flag->error = true;
                _kgflags_add_error(KGFLAGS_ERROR_KIND_MISSING_VALUE, flag->name, NULL);
                return;
            }
            bool ok = false;
            double double_val = _kgflags_parse_double(val, &ok);
            if (!ok) {
                flag->error = true;
                _kgflags_add_error(KGFLAGS_ERROR_KIND_INVALID_DOUBLE, flag->name, val);
                return;
            }
            *flag->result.double_value = double_val;
            flag->assigned = true;
            break;
        }
        case KGFLAGS_FLAG_KIND_STRING_ARRAY: {
            int initial_cursor = _kgflags_g.arg_cursor;
            int count = 0;
            while (true) {
                const char *val = _kgflags_peek_arg();
                if (val == NULL || _kgflags_is_flag(val)) {
                    break;
                }
                _kgflags_consume_arg();
                count++;
            }
            kgflags_string_array_t *arr = flag->result.string_array;
            arr->_items = _kgflags_g.argv + initial_cursor;
            arr->_count = count;
            flag->assigned = true;
            break;
        }
        case KGFLAGS_FLAG_KIND_INT_ARRAY: {
            int initial_cursor = _kgflags_g.arg_cursor;
            int count = 0;
            bool all_args_ok = true;
            while (true) {
                const char *val = _kgflags_peek_arg();
                if (val == NULL || _kgflags_is_flag(val)) {
                    break;
                }
                _kgflags_consume_arg();
                bool ok = false;
                _kgflags_parse_int(val, &ok);
                if (!ok) {
                    flag->error = true;
                    _kgflags_add_error(KGFLAGS_ERROR_KIND_INVALID_INT, flag->name, val);
                    all_args_ok = false;
                }
                count++;
            }
            kgflags_int_array_t *arr = flag->result.int_array;
            if (all_args_ok) {
                arr->_items = _kgflags_g.argv + initial_cursor;
                arr->_count = count;
            }
            flag->assigned = true;
            break;
        }
        case KGFLAGS_FLAG_KIND_DOUBLE_ARRAY: {
            int initial_cursor = _kgflags_g.arg_cursor;
            int count = 0;
            bool all_args_ok = true;
            while (true) {
                const char *val = _kgflags_peek_arg();
                if (val == NULL || _kgflags_is_flag(val)) {
                    break;
                }
                _kgflags_consume_arg();
                bool ok = false;
                _kgflags_parse_double(val, &ok);
                if (!ok) {
                    flag->error = true;
                    _kgflags_add_error(KGFLAGS_ERROR_KIND_INVALID_DOUBLE, flag->name, val);
                    all_args_ok = false;
                }
                count++;
            }
            kgflags_double_array_t *arr = flag->result.double_array;
            if (all_args_ok) {
                arr->_items = _kgflags_g.argv + initial_cursor;
                arr->_count = count;
            }
            flag->assigned = true;
            break;
        }
        default:
            break;
    }
}

#endif
