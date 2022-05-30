#ifndef _UTIL_LIB
#define _UTIL_LIB

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__BORLANDC__)
#define PATH_SEPARATOR ';'
#define DIR_SEPARATOR '\\'
#else
#define PATH_SEPARATOR ':'
#define DIR_SEPARATOR '/'
#endif

#define PATH_MAX_CHARS 512

/* ATTENTION: this source code file is not fully portable.
It use POSIX libraries that are not part of the C standard.
Therefore, not all compilers and platafforms can make this file._
Please, check the compatibility first. */

#include <time.h>
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)|| defined(__CYGWIN__) || defined(__MINGW32__) || defined(__BORLANDC__)
#include <direct.h>
#endif

/* POSIX libs*/
#ifndef _MSC_VER
#include <dirent.h>
#else
#include "_dirent.h"
#endif
#include <sys/stat.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
/*-----------*/

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

struct Mem_buffer * manage_buffer (long size, enum buffer_action action, int idx);

struct Mem_buffer *  load_file_reuse(char *path, long *fsize);

char * load_file(char *path, long *fsize);

char * try_load_dflt(char *path, char *dflt);

int miss_file (char *path, char *dflt);

const char * operating_system();

int opener(const char *url);

const char * dflt_fonts_dir ();

int contextual_codepoint (int prev, int curr, int next);

int dir_check(char *path); /* verify if directory exists */

char * dir_full(char *path); /* get full path of a folder */

int dir_change( char *path); /* change current directory */

int dir_make (char *path);

int dir_miss (char* path); /* try to create a folder, if not exists */

void matrix4_mul(float *mat_a, float *mat_b, float *mat_r);

int ray_plane(double ray_o[3], double ray_dir[3],
	double plane[4], double point[3]);
	
int invert_4matrix(float *m, float *m_inv);

#endif