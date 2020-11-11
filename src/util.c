#include "util.h"

void str_upp(char *str) { /* upper case the string */
	while (*str= toupper(*str)) str++;
}

char * trimwhitespace(char *str){
	/* Note: This function returns a pointer to a substring of the original string.
	If the given string was allocated dynamically, the caller must not overwrite
	that pointer with the returned value, since the original pointer must be
	deallocated using the same allocator with which it was allocated.  The return
	value must NOT be deallocated using free() etc. 
	Author = Adam Rosenfield
	http://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
	*/
	char *end;

	// Trim leading space
	while(isspace((unsigned char)*str)) str++;

	if(*str == 0)  // All spaces?
	return str;

	// Trim trailing space
	end = str + strlen(str) - 1;
	while(end > str && isspace((unsigned char)*end)) end--;

	// Write new null terminator
	*(end+1) = 0;

	return str;
}

char *get_filename(char *path){
	/* Get the filename with extension from path string.
	This function does not modify the original string,
	but the return string must be used imediatly */
	static char buf[PATH_MAX_CHARS];
	char *ret = NULL;
	
	strncpy(buf, path, PATH_MAX_CHARS);
	ret = strrchr(buf, DIR_SEPARATOR);
	
	if (ret) ret++;
	else {
		//buf[0] = 0;
		ret = buf;
	}
	
	return ret;
}

char *get_dir(char *path){
	/* Get the directory from path string.
	This function does not modify the original string,
	but the return string must be used imediatly */
	static char buf[PATH_MAX_CHARS];
	char *ret = NULL;
	
	strncpy(buf, path, PATH_MAX_CHARS);
	ret = strrchr(buf, DIR_SEPARATOR);
	
	if (ret){
		ret++;
		*ret = 0; /* terminate string */
	}
	else {
		buf[0] = '.';
		buf[1] = DIR_SEPARATOR;
		buf[2] = 0;
	}
	
	return buf;
}

char *get_ext(char *path){
	/* Get the extension of file from path string.
	This function does not modify the original string,
	but the return string must be used imediatly.
	Return string is in lower case*/
	static char buf[PATH_MAX_CHARS];
	char *ret = NULL;
	int i;
	
	
	strncpy(buf, path, PATH_MAX_CHARS);
	ret = strrchr(buf, '.');
	
	if (ret) {
		ret++;
		i = 0;
		while(ret[i]){
			ret[i] = tolower(ret[i]);
			i++;
		}
	}
	else {
		buf[0] = 0;
		ret = buf;
	}
	
	return ret;
}

void strip_ext(char *filename){
	/* Strip extension from filename string.
	Atention: This function modify the original string */
	char *ret = NULL;
	if (filename){
		ret = strrchr(filename, '.');
		if (ret){
			int pos = (int)(ret - filename);
			filename[pos] = 0;
		}
	}
}

int file_exists(char *fname){
	FILE *file;
	if (file = fopen(fname, "r")){
		fclose(file);
		return 1;
	}
	return 0;
}

int utf8_to_codepoint(char *utf8_s, int *uni_c){
	int ofs = 0;
	
	if (!utf8_s || !uni_c) return 0;
	
	char c = utf8_s[ofs];
	if (!c) return 0; /*string end*/
	*uni_c = 0;
	if ( (c & 0x80) == 0 ){
		*uni_c = c;
		ofs++;
	}
	else if ( (c & 0xE0) == 0xC0 ){
		if (!utf8_s[ofs+1]) return 0; /* error -> string end*/
		*uni_c = (utf8_s[ofs] & 0x1F) << 6;
		*uni_c |= (utf8_s[ofs+1] & 0x3F);
		ofs += 2;
	}
	else if ( (c & 0xF0) == 0xE0 ){
		if (!utf8_s[ofs+1] || !utf8_s[ofs+2]) return 0; /* error -> string end*/
		*uni_c = (utf8_s[ofs] & 0xF) << 12;
		*uni_c |= (utf8_s[ofs+1] & 0x3F) << 6;
		*uni_c |= (utf8_s[ofs+2] & 0x3F);
		ofs += 3;
	}
	else if ( (c & 0xF8) == 0xF0 ){
		if (!utf8_s[ofs+1] || !utf8_s[ofs+2] || !utf8_s[ofs+3]) return 0; /* error -> string end*/
		*uni_c = (utf8_s[ofs] & 0x7) << 18;
		*uni_c |= (utf8_s[ofs+1] & 0x3F) << 12;
		*uni_c |= (utf8_s[ofs+2] & 0x3F) << 6;
		*uni_c |= (utf8_s[ofs+3] & 0x3F);
		ofs += 4;
	}

	return ofs;

}

int codepoint_to_utf8(int uni_c, char utf8_s[5]){
	
	if (!utf8_s) return 0;
	
	int len = 0;
	utf8_s[4] = 0;
	if ( 0 <= uni_c && uni_c <= 0x7f ){
		utf8_s[0] = (char)uni_c;
		len++;
	}
	else if ( 0x80 <= uni_c && uni_c <= 0x7ff ){
		utf8_s[0] = ( 0xc0 | (uni_c >> 6) );
		utf8_s[1] = ( 0x80 | (uni_c & 0x3f) );
		len += 2;
	}
	else if ( 0x800 <= uni_c && uni_c <= 0xffff ){
		utf8_s[0] = ( 0xe0 | (uni_c >> 12) );
		utf8_s[1] = ( 0x80 | ((uni_c >> 6) & 0x3f) );
		utf8_s[2] = ( 0x80 | (uni_c & 0x3f) );
		len += 3;
	}
	else if ( 0x10000 <= uni_c && uni_c <= 0x1fffff ){
		utf8_s[0] = ( 0xf0 | (uni_c >> 18) );
		utf8_s[1] = ( 0x80 | ((uni_c >> 12) & 0x3f) );
		utf8_s[2] = ( 0x80 | ((uni_c >> 6) & 0x3f) );
		utf8_s[3] = ( 0x80 | (uni_c & 0x3f) );
		len += 4;
	}
	
	return len;
}

int str_utf2cp(char *str, int *cp, int max){
	int ofs = 0, str_start = 0, code_p, size = 0;
	
	while ((ofs = utf8_to_codepoint(str + str_start, &code_p)) && (size < max)){
		str_start += ofs;
		cp[size] = code_p;
		size++;
	}
	return size;
}

char *str_replace(char *orig, char *rep, char *with) {
	/*
	Author: jmucchiello
	http://stackoverflow.com/questions/779875/what-is-the-function-to-replace-string-in-c
	*/
	/* You must free the result if result is non-NULL. */
	char *result; /* the return string */
	char *ins;    /* the next insert point */
	char *tmp;    /* varies */
	int len_rep;  /* length of rep (the string to remove) */
	int len_with; /* length of with (the string to replace rep with) */
	int len_front; /* distance between rep and end of last rep */
	int count;    /* number of replacements */

	/* sanity checks and initialization */
	if (!orig || !rep)
		return NULL;
	len_rep = strlen(rep);
	if (len_rep == 0)
		return NULL; /* empty rep causes infinite loop during count */
	if (!with)
		with = "";
	len_with = strlen(with);

	/* count the number of replacements needed */
	ins = orig;
	for (count = 0; tmp = strstr(ins, rep); ++count) {
		ins = tmp + len_rep;
	}

	tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

	if (!result)
		return NULL;

	/*
	first time through the loop, all the variable are set correctly
	from here on, tmp points to the end of the result string
	ins points to the next occurrence of rep in orig
	orig points to the remainder of orig after "end of rep"
	*/
	while (count--) {
		ins = strstr(orig, rep);
		len_front = ins - orig;
		tmp = strncpy(tmp, orig, len_front) + len_front;
		tmp = strcpy(tmp, with) + len_with;
		orig += len_front + len_rep; /* move to next "end of rep" */
	}
	strcpy(tmp, orig);
	return result;
}

struct Mem_buffer * manage_buffer (long size, enum buffer_action action){
	static struct Mem_buffer buf = {.buffer = NULL, .size = 0, .used = 0};
	
	if (action == BUF_GET){
		if (size <= 0) return NULL;
		if (buf.used != 0) return NULL;
		
		if (buf.buffer == NULL){
			if (buf.buffer = malloc (size)){
				buf.size = size;
			}
			else {
				buf.size = 0;
				buf.used = 0;
				return NULL;
			}
		}
		else if (size > buf.size){
			if (buf.buffer = realloc (buf.buffer, size)){
				buf.size = size;
			}
			else {
				buf.size = 0;
				buf.used = 0;
				return NULL;
			}
		}
		buf.used = 1;
	}
	else if (action == BUF_RELEASE){
		buf.used = 0;
	}
	else if (action == BUF_FREE){
		if (buf.buffer != NULL) free(buf.buffer);
		buf.buffer = NULL;
		buf.size = 0;
		buf.used = 0;
		return NULL;
	}
	
	
	return &buf;
}

struct Mem_buffer *  load_file_reuse(char *path, long *fsize){
	FILE *file;
	
	*fsize = 0;
	file = fopen(path, "rb");
	if(file == NULL){
		return NULL;
	}
	
	fseek(file, 0, SEEK_END);
	*fsize = ftell(file); /* size of file*/
	fseek(file, 0, SEEK_SET);  //same as rewind(f);
	//printf("file size = %d\n", fsize);
	
	//char *buf = malloc(*fsize + 1);
	struct Mem_buffer *buf = manage_buffer(*fsize + 1, BUF_GET);
	if (!buf){
		*fsize = 0;
		fclose(file);
		return NULL;
	}
	fread(buf->buffer, *fsize, 1, file);
	fclose(file);
	buf->buffer[*fsize] = 0;
	return buf;
}

char * load_file(char *path, long *fsize){
	FILE *file;
	
	*fsize = 0;
	file = fopen(path, "rb");
	if(file == NULL){
		return NULL;
	}
	
	fseek(file, 0, SEEK_END);
	*fsize = ftell(file); /* size of file*/
	fseek(file, 0, SEEK_SET);  //same as rewind(f);
	//printf("file size = %d\n", fsize);
	
	char *buf = malloc(*fsize + 1);
	if (!buf){
		*fsize = 0;
		fclose(file);
		return NULL;
	}
	fread(buf, *fsize, 1, file);
	fclose(file);
	buf[*fsize] = 0;
	return buf;
}

char * try_load_dflt(char *path, char *dflt){
	long fsize;
	
	char *buf = load_file(path, &fsize);
	
	if (buf == NULL && dflt != NULL) {
		fsize = strlen(dflt) + 1;
		buf = calloc(fsize, 1);
		if (buf) memcpy (buf, dflt, fsize);
	}
	
	return buf;
}