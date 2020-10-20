#ifndef _UTIL_LIB
#define _UTIL_LIB

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__BORLANDC__)
#define PATH_SEPARATOR ';'
#define DIR_SEPARATOR '\\'
#else
#define PATH_SEPARATOR ':'
#define DIR_SEPARATOR '/'
#endif

#define PATH_MAX_CHARS 250

struct Mem_buffer {
	char *buffer;
	long size;
	int used;
};

enum buffer_action{
	BUF_GET,
	BUF_RELEASE,
	BUF_FREE
};

void str_upp(char *str);

char * trimwhitespace(char *str);

char *get_filename(char *path);

char *get_dir(char *path);

char *get_ext(char *path);

void strip_ext(char *filename);

int file_exists(char *fname);

int utf8_to_codepoint(char *utf8_s, int *uni_c);

int codepoint_to_utf8(int uni_c, char utf8_s[5]);

int str_utf2cp(char *str, int *cp, int max);

struct Mem_buffer * manage_buffer (long size, enum buffer_action action);

#endif