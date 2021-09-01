/*
 * Dirent interface for Microsoft Visual Studio
 *
 * Copyright (C) 1998-2019 Toni Ronkko
 * This file is part of dirent.  Dirent may be freely distributed
 * under the MIT license.  For all details and documentation, see
 * https://github.com/tronkko/dirent
 */
#ifndef DIRENT_H
#define DIRENT_H
#ifndef PATH_MAX
#define PATH_MAX 260
#endif
struct dirent {
	/* Always zero */
	long d_ino;

	/* File position within stream */
	long d_off;

	/* Structure size */
	unsigned short d_reclen;

	/* Length of name without \0 */
	size_t d_namlen;

	/* File type */
	int d_type;

	/* File name */
	char d_name[PATH_MAX+1];
};
typedef struct dirent dirent;

typedef struct DIR DIR;

DIR		   *opendir(const char *);
struct dirent *readdir(DIR *);
void rewinddir(DIR* dirp);
int			closedir(DIR *);

#endif
